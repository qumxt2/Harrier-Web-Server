// serial_uart_U1A.c

// Copyright 2006-2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// ***************************************************
// * DESCRIPTION
// ***************************************************
// This is meant to work with the PIC32MX795F512L as used on the N100M Informer DCM.
// This file is derived from the PIC32 ADM. 
// Serial port U1A "U1RX" is pin 52, RF2.
// Serial port U1A "U1TX" is pin 53, RF8.
// To the best extent possible all lines that are serial port U1A specific have the
// "U1A" token in them. Therefore, function names were modified to contain this.

#include "string.h"

#include "typedef.h"

#include "p32mx795f512l.h"

#include "uC_peripheral_map.h"

#include "rtos.h"
#include "oscillator.h"

#include "component.h"

#include "serial_uart_U1A.h"
#include "sys/attribs.h"

#include "debug.h"
#include <peripheral/int.h>


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

#define MAX_SERIAL_U1A_INIT_WAIT	(1000)


// ***************************************************
// * STATIC VARIABLES
// ***************************************************
static bool initialized = FALSE;
static uint32 current_baud_rate = 0;

/* Resource indexes */
static uint8 s_SerialResourceIndex = RTOS_INVALID_ID;

// Not completely sure this needs to be this size, but it definitely needs
// to be bigger than the library default. Changed indicies to support >256 byte
// buffer, too.
#define	TX_BUFFER_SIZE (1024)
static uint8 s_TxDMABufferA[TX_BUFFER_SIZE] __attribute__ ((aligned(TX_BUFFER_SIZE)));

static uint16 gByteCnt = 0;
static uint16 gByteIdx = 0;

static bool g_TxInProgress = FALSE;

static uint16 s_RxBufferIn;				/* index to place next character */
static uint16 s_RxBufferOut;			/* index to read out next character */

// Substantially increased this due to SSL/TLS handshake needs (had been experiencing
// buffer overruns with buffers as large as 4096 bytes)
#define	RX_BUFFER_SIZE				(8192)

static uint8 s_RxBuffer[ RX_BUFFER_SIZE ];

static uartTxCB_t localTxCallback = NULL;
static uartRxCB_t localRxCallback = NULL;

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
static inline sint16 Serial_U1A_HardwareInit (UART_OPTIONS_T *options);
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

enum INIT_ERRORS Serial_U1A_Init( UART_OPTIONS_T *options )
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

	if ( Serial_U1A_HardwareInit(options) < 0 )
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
bool Serial_U1A_Rx( uint8 *cPtr )
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
bool Serial_U1A_Tx(uint8 *ptData, uint16 count, uint16 timeout)
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
		IEC0SET = _IEC0_U1ATXIE_MASK;	// Enable Interrupt
		//IFS0CLR = _IFS0_U1ATXIF_MASK;
	}
	
	return FALSE;
}


void Serial_U1A_Reset( void )
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
** Set up the UART U1A hardware to desired baud rate and mode.
*/
static inline sint16 Serial_U1A_HardwareInit( UART_OPTIONS_T *options )
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
		return 0;
	}

    U1ASTA = 0;
    U1AMODE = 0;

	TRISCLR_U1A_TX = BITMASK_U1A_TX;		/* UART TX pin RF8 to output */
	TRISSET_U1A_RX = BITMASK_U1A_RX;		/* UART RX pin RF2 to input */

	// In the N100M Informer DCM implementation, CTS and RTS are not used.
	// Those pins are assigned as U1B.
	if(options->flowControl == FC_CTSRTS) {
		TRISSET_U1A_CTS = BITMASK_U1A_CTS;		/* UART CTS pin RD14 to input */
		TRISCLR_U1A_RTS = BITMASK_U1A_RTS;		/* UART RTS pin RD15 to output */
	}

	U1AMODECLR = _U1AMODE_UEN_MASK;
	if(options->flowControl == FC_CTSRTS) {
		/* 0b10 - UxTX, UxRX, UxCTS and UxRTS pins are enabled and used */
		U1AMODESET = (2)<<_U1AMODE_UEN_POSITION;
	} else {
		/* 0b10 - UxTX, UxRX, pins are enabled and used, UxCTS and UxRTS are controlled by PORTx */
		U1AMODESET = (0)<<_U1AMODE_UEN_POSITION;
	}

	U1AMODESET = _U1AMODE_BRGH_MASK;	/* BRG Generates 4 clocks per bit period (high-speed mode) */
	// Previous check of negative value allows type cast to unsigned

	U1ABRG = GetBRGSettings( options->baudRate );

	// Set polarity
	if(options->rxPolarity == RX_POL_NEG) {
		U1AMODESET = (1)<<_U1AMODE_RXINV_POSITION;
	} else {
		U1AMODECLR = (1)<<_U1AMODE_RXINV_POSITION;
	}

	if(options->txPolarity == TX_POL_NEG) {
		U1ASTASET = (1)<<_U1ASTA_UTXINV_POSITION;
	} else {
		U1ASTACLR = (1)<<_U1ASTA_UTXINV_POSITION;
	}

	// Set parity
	//U1AMODECLR = _U1AMODE_PDSEL_MASK;
	//U1AMODESET = (options->parity)<<_U1AMODE_PDSEL_POSITION;

	if(options->parity == PARITY_NONE)
	{
		U1AMODECLR = _U1AMODE_PDSEL_MASK;
	}
	else if (options->parity == PARITY_ODD)
	{
		U1AMODECLR = _U1AMODE_PDSEL0_MASK;
		U1AMODESET = _U1AMODE_PDSEL1_MASK;
	}
	else if (options->parity == PARITY_EVEN)
	{
		U1AMODECLR = _U1AMODE_PDSEL1_MASK;
		U1AMODESET = _U1AMODE_PDSEL0_MASK;
	}

	// Set stop bits
	if(options->stopBits == 2) {
		U1AMODESET = (1)<<_U1AMODE_STSEL_POSITION;
	} else {
		U1AMODECLR = (1)<<_U1AMODE_STSEL_POSITION;
	}


	// Set priority of UART interrupt
	// If the priority is too high, the I2C communications will drop out.
	IPC6CLR = _IPC6_U1AIP_MASK;
	IPC6SET = (5)<<_IPC6_U1AIP_POSITION;
	// Set subpriority of UART interrupt
	IPC6CLR = _IPC6_U1AIS_MASK;
	IPC6SET = (3)<<_IPC6_U1AIS_POSITION;
	
    U1AMODESET = _U1AMODE_UARTEN_MASK;		/* Enable UART */
    U1ASTASET = _U1ASTA_UTXEN_MASK;			/* Enable transmitter */
    U1ASTASET = _U1ASTA_URXEN_MASK;			/* Enable receiver */
    
    // Interrupts occur when TX buffer is empty
	U1ASTACLR = _U1ASTA_UTXSEL_MASK;
	U1ASTASET = (2)<<_U1ASTA_UTXSEL_POSITION;
	IEC0CLR = _IEC0_U1ATXIE_MASK;			/* Interrupt will be enabled once the Tx function is called */
	IFS0CLR = _IFS0_U1ATXIF_MASK;			/* Reset TX flag */
	
    IFS0CLR = _IFS0_U1ARXIF_MASK;			/* Reset RX flag */
	U1ASTACLR = _U1ASTA_URXISEL_MASK;		/* Interrupts occur when any data is received */
	IEC0SET = _IEC0_U1ARXIE_MASK;			/* Enable interrupts when data is received */
	current_baud_rate = options->baudRate;
	pBus_Fcy = pBus_Fcy_Temp;

	// Send a small SYNC burst, then an XON character,
	// just to make sure that flow controlled terminals
	// are unblocked.
	
	for( i = 0 ; i < 9 ; i++ )
	{
		while( U1ASTAbits.UTXBF )
			;
		U1ATXREG = SYNC_CHARACTER;
	}
	
	U1ATXREG = XON_CHARACTER;
	
	// If the firmware has never been loaded, the TRMT bit will never
	// change to 0b1.  This timeout allows it to continue without failure.
	while( !U1ASTAbits.TRMT && (i < MAX_SERIAL_U1A_INIT_WAIT) )
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
** ISR for handling bytes avaialble in the UART U1A hardware Rx buffer.
*/
void __ISR (_UART_1A_VECTOR, ipl6) _U1ARXInterrupt(void)
{
	uint8 counter;

	counter = 10;

	if( IFS0bits.U1ARXIF )
	{
		//while( counter-- )
		
		while( 1 )
		{
			if( U1ASTAbits.OERR )
			{
				/* Receive buffer overflow... clearing bit should reset the FIFO */
				U1ASTACLR = _U1ASTA_OERR_MASK;
				continue;
			}
	
			if( ! U1ASTAbits.URXDA ) {
				break;
			}
	
			if( U1ASTAbits.PERR || U1ASTAbits.FERR )
			{
				/* Parity or framing error... drop character */
				int dummy = U1ARXREG;
	
				dummy = dummy;		// this keeps the compiler from complaining
				continue;
			}
	
			// Grab data
			s_RxBuffer[s_RxBufferIn] = U1ARXREG;
	
			// Increment buffer index and roll if necessary.
			s_RxBufferIn = (s_RxBufferIn + 1) & (RX_BUFFER_SIZE - 1);
		}
	
	    // Reset the interrupt flag
	    IFS0CLR = _IFS0_U1ARXIF_MASK;

        if (localRxCallback != NULL)
        {
            localRxCallback();
        }
	}

	if( IFS0bits.U1ATXIF )
	{
        if (gByteCnt > 200)
        {
            if (localTxCallback != NULL)
            {
                localTxCallback();
            }
        }

		// While the transmit buffer is NOT FULL and there are bytes to send...
		while( !U1ASTAbits.UTXBF && (gByteIdx < gByteCnt) )
		{
			// ...Add some bytes
			U1ATXREG = s_TxDMABufferA[gByteIdx++];
		}
		
		// If we have no more data to send, reset the g_TxInProgress flag
		if( gByteIdx == gByteCnt )
		{
			g_TxInProgress = FALSE;
			IEC0CLR = _IEC0_U1ATXIE_MASK;		// Disable Interrupt
		}	
		
		//mU1ATXClearIntFlag();
		IFS0CLR = _IFS0_U1ATXIF_MASK;		// Clear the TX interrupt flag

        if (localTxCallback != NULL)
        {
            localTxCallback();
        }

	}
}
