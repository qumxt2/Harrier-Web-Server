// serial_uart_u1b.c

// Copyright 2006-2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// ***************************************************
// * DESCRIPTION
// ***************************************************
// This is meant to work with the PIC32MX795F512L as used on the N100M Informer DCM.
// This file is derived from the PIC32 ADM. 
// Serial port U1B "U1RX" is pin 52, RF2.
// Serial port U1B "U1TX" is pin 53, RF8.
// To the best extent possible all lines that are serial port U1B specific have the
// "U1B" token in them. Therefore, function names were modified to contain this.

#include "string.h"

#include "typedef.h"

#include "p32mx795f512l.h"

#include "uC_peripheral_map.h"

#include "rtos.h"
#include "oscillator.h"

#include "component.h"

#include "serial_uart_u1b.h"

#include "sys/attribs.h"

#include "debug.h"

// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************

/// Macro used to signal the RTOS of ISR execution start
#define INTERRUPT_ENTRY() {	K_OS_Intrp_Entry();	}

/// Macro used to signal the RTOS of ISR execution end
#define INTERRUPT_EXIT() {	K_OS_Intrp_Exit(); }

#define MAX_WAIT_NOPS	(500)

#define MAX_SERIAL_U1B_INIT_WAIT	(1000)


// ***************************************************
// * STATIC VARIABLES
// ***************************************************
static bool initialized = FALSE;
static uint32 current_baud_rate = 0;

/* Resource indexes */
static uint8 s_SerialResourceIndex = RTOS_INVALID_ID;

#define	TX_BUFFER_SIZE (256)
static uint8 s_TxDMABufferA[TX_BUFFER_SIZE] __attribute__ ((aligned(TX_BUFFER_SIZE)));

static uint8 gByteCnt = 0;
static uint8 gByteIdx = 0;

static bool g_TxInProgress = FALSE;

static uint16 s_RxBufferIn;				/* index to place next character */
static uint16 s_RxBufferOut;			/* index to read out next character */

#define	RX_BUFFER_SIZE				(128)

static uint8 s_RxBuffer[ RX_BUFFER_SIZE ];

static uartTxCB_t localTxCallback;
static uartRxCB_t localRxCallback;

/*
** We rely on the buffer size being a power of two so that we can do some
** speedy power-of-two tricks for head/tail pointer wrap-around.
*/
#if (((RX_BUFFER_SIZE - 1) & (RX_BUFFER_SIZE)) != 0)
#error	RX_BUFFER_SIZE must be a power of two
#endif

#define DEFAULT_BAUD_RATE			(9600UL)



// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************
static inline sint16 Serial_U1B_HardwareInit (UART_OPTIONS_T *options);
static uint16 GetBRGSettings( uint32 clockRate );


// ***************************************************
// * CONSTANTS
// ***************************************************

#define	XON_CHARACTER				((uint8) 0x11)
#define	SYNC_CHARACTER				((uint8) 0xFF)

#define MAX_UART_BAUD_RATE_DIVISOR	(0xFFFF)
#define DEFAULT_UART_BRD_CALC_DIVISOR	(4)


// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************

enum INIT_ERRORS Serial_U1B_Init( UART_OPTIONS_T *options )
{
    /* Initialize UART */
    static bool firstTime = TRUE;

	if( ! options->txCallback )
	{
		return ERROR_HARDWARE_INIT;
	}

	if( ! options->rxCallback )
	{
		return ERROR_HARDWARE_INIT;
	}

	if( ! options->baudRate )
	{
		return ERROR_INVALID_BAUD;
	}	

	if ( Serial_U1B_HardwareInit(options) < 0 )
	{
		return ERROR_HARDWARE_INIT;
	}
	if( initialized )
	{
		return ERROR_ALREADY_INITIALIZED;
	}
	
	// Added firstTime check to avoid reserving the resource over and over if resetting occurs repeatedly
	if( firstTime )
	{
		/* Reserve and check resource index */
		s_SerialResourceIndex = RtosResourceReserveID();
		if(RTOS_INVALID_ID == s_SerialResourceIndex)
		{
			return ERROR_INVALID_RESOURCE_ID;
		}
		
		firstTime = FALSE;
	}

	localTxCallback = *options->txCallback;
	localRxCallback = *options->rxCallback;	

	initialized = TRUE;

    return NO_ERROR;
}


/*
** Retrieve a character from the receive FIFO of the UART, if any exists.
*/
bool Serial_U1B_Rx( uint8 *cPtr )
{
	// Value to return at end of function
	bool rtn = FALSE;

	// Check for data available
	if (s_RxBufferOut != s_RxBufferIn)
	{
		// Stuff data
		*cPtr = s_RxBuffer[s_RxBufferOut];

		// Increment out index and roll if necessary
		s_RxBufferOut = (s_RxBufferOut + 1) & (RX_BUFFER_SIZE - 1);

		// Indicate that data has been returned
		rtn = TRUE;
	}

	return(rtn);
}

//----------------------------------------------------------------------------
// Transmits an array of characters via DMA RAM.
//
// Note: See the header file for complete documentation of this public method.
//----------------------------------------------------------------------------
bool Serial_U1B_Tx(uint8 *ptData, uint8 count, uint16 timeout)
{
	uint16 waitCount = 0;
	uint16 stopCount = 0;
	
	// Wait for the destination to become availalble
	while(g_TxInProgress)
	{
		if( waitCount < MAX_WAIT_NOPS )
		{
			Nop();
		}
		else
		{
			(void) K_Task_Wait(1);
			
			if( stopCount > timeout)
			{
				return TRUE;
			}
			
			stopCount++;
		}

		waitCount++;
	}

	// Copy	Tx data into the DMA RAM	
	(void)memcpy(s_TxDMABufferA, ptData, count);

	// Set COUNT to the appropriate value, skip write if count is zero.
	if (count)
	{
		g_TxInProgress = TRUE;
		gByteCnt = count;
		gByteIdx = 0;
		IEC2SET = _IEC2_U1BTXIE_MASK;	// Enable Interrupt
	}
	
	return FALSE;
}


void Serial_U1B_Reset( void )
{
	initialized = FALSE;
	current_baud_rate = 0;
	
	g_TxInProgress = FALSE;
	gByteCnt = 0;
	gByteIdx = 0;
}


// *********************************************
// * PRIVATE FUNCTIONS
// *********************************************

/*
** Set up the UART U1B hardware to desired baud rate and mode.
*/
static inline sint16 Serial_U1B_HardwareInit( UART_OPTIONS_T *options )
{
	static sint32 pBus_Fcy = 0;
	sint32 pBus_Fcy_Temp;
	uint16 i;
	
	/* Initialize UART */

	if( ! options->baudRate )
	{
		return -1;
	}
	pBus_Fcy_Temp = PeripheralBusGetFcy();
	// Check if the oscillator library returned a valid value
	if (pBus_Fcy_Temp < 0)
	{
		// nope, the oscillator library doesn't know what's going on,
		return -1;
	}

    if((options->baudRate == current_baud_rate) &&
	   (pBus_Fcy_Temp == pBus_Fcy))
	{
		//return 0;
	}

    U1BSTA = 0;
    U1BMODE = 0;

	TRISCLR_U1B_TX = BITMASK_U1B_TX;		/* UART TX pin RD15 to output */
	TRISSET_U1B_RX = BITMASK_U1B_RX;		/* UART RX pin RD14 to input */

	// In the N100M Informer DCM implementation, CTS and RTS are not used.
	// Those pins are assigned as U1B.

	U1BMODESET = _U1BMODE_BRGH_MASK;	/* BRG Generates 4 clocks per bit period (high-speed mode) */
	// Previous check of negative value allows type cast to unsigned
	
	U1BBRG = GetBRGSettings( options->baudRate );
	
	// Set polarity
	if(options->rxPolarity == RX_POL_NEG) {
		U1BMODESET = (1)<<_U1BMODE_RXINV_POSITION;
	} else {
		U1BMODECLR = (1)<<_U1BMODE_RXINV_POSITION;
	}

	if(options->txPolarity == TX_POL_NEG) {
		U1BSTASET = (1)<<_U1BSTA_UTXINV_POSITION;
	} else {
		U1BSTACLR = (1)<<_U1BSTA_UTXINV_POSITION;
	}

	// Set parity
	if(options->parity == PARITY_NONE)
	{
		U1BMODECLR = _U1BMODE_PDSEL_MASK;
	}
	else if (options->parity == PARITY_ODD)
	{
		U1BMODECLR = _U1BMODE_PDSEL0_MASK;
		U1BMODESET = _U1BMODE_PDSEL1_MASK;
	}
	else if (options->parity == PARITY_EVEN)
	{
		U1BMODECLR = _U1BMODE_PDSEL1_MASK;
		U1BMODESET = _U1BMODE_PDSEL0_MASK;
	}

	// Set stop bits
	if(options->stopBits == 2) {
		U1BMODESET = (1)<<_U1BMODE_STSEL_POSITION;
	} else {
		U1BMODECLR = (1)<<_U1BMODE_STSEL_POSITION;
	}

	// Set priority of UART interrupt
	// If the priority is too high, the I2C communications will drop out.
	IPC12CLR = _IPC12_U1BIP_MASK;
	IPC12SET = (5)<<_IPC12_U1BIP_POSITION;
	// Set subpriority of UART interrupt
	IPC12CLR = _IPC12_U1BIS_MASK;
	IPC12SET = (3)<<_IPC12_U1BIS_POSITION;
	
    U1BMODESET = _U1BMODE_UARTEN_MASK;		/* Enable UART */
    U1BSTASET = _U1BSTA_UTXEN_MASK;			/* Enable transmitter */
    U1BSTASET = _U1BSTA_URXEN_MASK;			/* Enable receiver */
    
    // Interrupts occur when TX buffer is empty
	U1BSTACLR = _U1BSTA_UTXSEL_MASK;
	U1BSTASET = (2)<<_U1BSTA_UTXSEL_POSITION;
	IEC2CLR = _IEC2_U1BTXIE_MASK;			/* Interrupt will be enabled once the Tx function is called */
	IFS2CLR = _IFS2_U1BTXIF_MASK;			/* Reset TX flag */
	
    IFS2CLR = _IFS2_U1BRXIF_MASK;			/* Reset RX flag */
	U1BSTACLR = _U1BSTA_URXISEL_MASK;		/* Interrupts occur when any data is received */
	IEC2SET = _IEC2_U1BRXIE_MASK;			/* Enable interrupts when data is received */
	current_baud_rate = options->baudRate;
	pBus_Fcy = pBus_Fcy_Temp;

	// Send a small SYNC burst, then an XON character,
	// just to make sure that flow controlled terminals
	// are unblocked.
	
	for( i = 0 ; i < 9 ; i++ )
	{
		while( U1BSTAbits.UTXBF )
			;
		U1BTXREG = SYNC_CHARACTER;
	}
	
	U1BTXREG = XON_CHARACTER;
	
	// If the firmware has never been loaded, the TRMT bit will never
	// change to 0b1.  This timeout allows it to continue without failure.
	while( !U1BSTAbits.TRMT && (i < MAX_SERIAL_U1B_INIT_WAIT) )
	{
		(void) K_Task_Wait(1);
		i++;
	}
	
    return 0;
}

static uint16 GetBRGSettings( uint32 clockRate )
{
	uint32 closestDiff;
	uint32 testDivisor;
	uint16 bestSetting = 0;
	
	closestDiff = 2 * PeripheralBusGetFcy();
	
	for( testDivisor = 0 ; testDivisor <= MAX_UART_BAUD_RATE_DIVISOR ;	testDivisor++ )
	{
		// Calculate the peripheral bus clock frequency
		uint32 rate = (uint32)PeripheralBusGetFcy();
		uint32 diff;

		rate = rate / ( (DEFAULT_UART_BRD_CALC_DIVISOR)*(testDivisor + 1) );
		
		if( rate > clockRate )
		{
			diff = rate - clockRate;
		}	
		else
		{
			diff = clockRate - rate;
		}	
		
		if( diff < closestDiff )
		{
			closestDiff = diff;

			bestSetting = testDivisor;
		}
	}

	return bestSetting;
}

/*
** ISR for handling bytes avaialble in the UART U1B hardware Rx buffer.
*/
void __ISR (_UART_1B_VECTOR, ipl6) _U1BRXInterrupt(void)
{
	if( IFS2bits.U1BRXIF )
	{
		while( 1 )
		{
			if( U1BSTAbits.OERR )
			{
				/* Receive buffer overflow... clearing bit should reset the FIFO */
				U1BSTACLR = _U1BSTA_OERR_MASK;
				continue;
			}
	
			if( ! U1BSTAbits.URXDA ) {
				break;
			}

			if( U1BSTAbits.PERR || U1BSTAbits.FERR )
			{
				/* Parity or framing error... drop character */
				int dummy = U1BRXREG;
	
				dummy = dummy;		// this keeps the compiler from complaining
				continue;
			}
	
			// Grab data
			s_RxBuffer[s_RxBufferIn] = U1BRXREG;
	
			// Increment buffer index and roll if necessary.
			s_RxBufferIn = (s_RxBufferIn + 1) & (RX_BUFFER_SIZE - 1);
		}
	
	    // Reset the interrupt flag
	    IFS2CLR = _IFS2_U1BRXIF_MASK;
		localRxCallback();
	}
	
	if( IFS2bits.U1BTXIF )
	{
		// While the transmit buffer is NOT FULL and there are bytes to send...
		while( !U1BSTAbits.UTXBF && (gByteIdx < gByteCnt) )
		{
			// ...Add some bytes
			U1BTXREG = s_TxDMABufferA[gByteIdx++];
		}
		
		// If we have no more data to send, reset the g_TxInProgress flag
		if( gByteIdx == gByteCnt )
		{
			g_TxInProgress = FALSE;
			IEC2CLR = _IEC2_U1BTXIE_MASK;		// Disable Interrupt
		}	
		
		IFS2CLR = _IFS2_U1BTXIF_MASK;		// Clear the TX interrupt flag
		localTxCallback();
	}
}
