/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

#include "port.h"
#include "p32mx795f512l.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
//#include "mbport.h"
#include "serial_common.h"
#include "serial_uart_u1a.h"
#include "serial_uart_u1b.h"

/* ----------------------- static functions ---------------------------------*/
//static void prvvUARTTxReadyISR( void );
//static void prvvUARTRxISR( void );

static void tx1aCallback ( void );
static void rx1aCallback ( void );
static void tx1bCallback ( void );
static void rx1bCallback ( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( bool xRxEnable, bool xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
	
	if(xRxEnable == TRUE && xTxEnable == TRUE){
		// uart1a
		//INTEnable(INT_U1ARX, INT_ENABLED); 
		//INTEnable(INT_U1ATX, INT_ENABLED); 
		IEC0bits.U1ARXIE = 1;
		IEC0bits.U1ATXIE = 1;
		
		// uart1b
		//INTEnable(INT_U1BRX, INT_ENABLED); 
		//INTEnable(INT_U1BTX, INT_ENABLED); 
		IEC2bits.U1BRXIE = 1;
		IEC2bits.U1BTXIE = 1;
	}
	else if(xRxEnable == TRUE && xTxEnable == FALSE){
		// uart1a
		//INTEnable(INT_U1ARX, INT_ENABLED); 
		//INTEnable(INT_U1ATX, INT_DISABLED); 
		IEC0bits.U1ARXIE = 1;
		IEC0bits.U1ATXIE = 0;
		// uart1b
		//INTEnable(INT_U1BRX, INT_ENABLED); 
		IEC2bits.U1BRXIE = 1;
		//INTEnable(INT_U1BTX, INT_ENABLED); 
	}
	else if(xRxEnable == FALSE && xTxEnable == TRUE){
		// uart1a
		//INTEnable(INT_U1ARX, INT_DISABLED); 
		//INTEnable(INT_U1ATX, INT_ENABLED); 
		IEC0bits.U1ARXIE = 0;
		IEC0bits.U1ATXIE = 1;	
		IFS0bits.U1ATXIF = 1;
		// uart1b
		//INTEnable(INT_U1BRX, INT_ENABLED); 
		//INTEnable(INT_U1BTX, INT_DISABLED); 
		//IFS2bits.U1BTXIF = 1;
		IEC2bits.U1BRXIE = 1;
		IEC2bits.U1BTXIE = 0;
	}
	else if(xRxEnable == FALSE && xTxEnable == FALSE){
		// uart1a
		//INTEnable(INT_U1ARX, INT_DISABLED); 
		//INTEnable(INT_U1ATX, INT_DISABLED); 
		IEC0bits.U1ARXIE = 0;
		IEC0bits.U1ATXIE = 0;
		// uart1b
		//INTEnable(INT_U1BRX, INT_DISABLED); 
		//INTEnable(INT_U1BTX, INT_DISABLED); 
		IEC2bits.U1BRXIE = 0;
		IEC2bits.U1BTXIE = 0;
	}
	
}

bool
xMBPortSerialInit( uchar ucPORT, uint32 ulBaudRate, uchar ucDataBits, eMBParity eParity, uint8 stopBits )
{
	// Passing address of function to simplify code
	/*lint -e546*/
	UART_OPTIONS_T uart1aOptions = {ulBaudRate, RX_POL_POS, TX_POL_POS, FC_NONE, (uint8)eParity, stopBits, &tx1aCallback, &rx1aCallback};
	 //UART_OPTIONS_T uart1bOptions = {ulBaudRate, RX_POL_POS, TX_POL_POS, FC_NONE, (uint8)eParity, stopBits, &tx1bCallback, &rx1bCallback};
	/*lint +e546*/
/*
	int pbClk;

	pbClk=SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
	if (ucPORT == 1){
		OpenUART1(UART_EN, 		// Module is ON
				  UART_RX_ENABLE | UART_TX_ENABLE,		// Enable TX & RX
				  pbClk/16/ulBaudRate-1);	// 
		ConfigIntUART1(UART_INT_PR2 | UART_RX_INT_EN);
	}
	else if (ucPORT == 2){
		OpenUART2(UART_EN | UART_NO_PAR_8BIT | UART_2STOPBITS, 		// Module is ON
				  UART_RX_ENABLE | UART_TX_ENABLE,		// Enable TX & RX
				  pbClk/16/ulBaudRate-1);	// 
		ConfigIntUART2(UART_INT_PR2 | UART_RX_INT_EN);
	}
*/		
	//INTEnableSystemMultiVectoredInt();

	vMBPortSerialEnable( FALSE, FALSE );
	( void ) Serial_U1A_Init(&uart1aOptions);
//	( void ) Serial_U1B_Init(&uart1bOptions);
	vMBPortSerialEnable( FALSE, FALSE );


    return TRUE;
}

bool
xMBPortSerialPutByte( char ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
	//putcUART2(ucByte);
	( void ) Serial_U1A_Tx((uint8*)&ucByte, 1, 1);
    return TRUE;
}

bool
xMBPortSerialGetByte( char * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */

	( void ) Serial_U1A_Rx((uint8*)pucByte);
    return TRUE;
}

bool
xMBPortSerial1bPutByte( char ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */

//	( void ) Serial_U1B_Tx((uint8*)&ucByte, 1, 1);
    return TRUE;
}

bool
xMBPortSerial1bGetByte( char * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */

//	( void ) Serial_U1B_Rx((uint8*)pucByte);
    return TRUE;
}

void
xMBPortSerialClose( void )
{
	Serial_U1A_Reset( );
//	Serial_U1B_Reset( );
}

void
vMBPortClose( void )
{
	xMBPortSerialClose( );
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */

// UART 2 interrupt handler
// it is set at priority level 2
/*
void __ISR(_UART1_VECTOR, ipl2) IntUart1Handler(void){

	// RX interrupt
	if(mU2RXGetIntFlag())
	{
		// Clear the RX interrupt Flag
	    mU2RXClearIntFlag();
		pxMBFrameCBByteReceived(  );

	}

	// TX interrupt
	if ( mU2TXGetIntFlag() )
	{
		mU2TXClearIntFlag();
		pxMBFrameCBTransmitterEmpty(  );

	}
}

*/
void rx1aCallback ( void ) 
{
	( void ) pxMBFrameCBByteReceived(  );
}

void tx1aCallback ( void ) 
{
	( void ) pxMBFrameCBTransmitterEmpty();
}

void rx1bCallback ( void ) 
{
	( void ) pxMBFrameCBByteReceived1b(  );
}

void tx1bCallback ( void ) 
{
	( void ) pxMBFrameCBTransmitterEmpty1b();
}
