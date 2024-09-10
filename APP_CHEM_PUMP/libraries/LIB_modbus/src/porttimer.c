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
 * File: $Id: porttimer.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "p32mx795f512l.h"
#include "sys/attribs.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
//#include "mbport.h"

#include "timebase.h"

#define MODBUS_T4_INT_PRIOR_2	(2)

/* ----------------------- Static variables ---------------------------------*/
static uint16 gPortTimerTimeout;

/* ----------------------- static functions ---------------------------------*/
//static void prvvTIMERExpiredISR( void );

/* ----------------------- Start implementation -----------------------------*/
bool xMBPortTimersInit(uint16 usTim1Timerout50us)
{
	//INTEnableSystemMultiVectoredInt();

	//usTim1Timerout50us = usTim1Timerout50us * 250/(SYS_FREQ/PRESCALE)
    // SYS_FREQ/PRESCALE/20000 = 250
    
    gPortTimerTimeout = usTim1Timerout50us;
    usTim1Timerout50us = gPortTimerTimeout * 250;

	// Configure Timer
	TIMEBASE_Initialize( TIMEBASE_4 );
	TIMEBASE_Period_Set( TIMEBASE_4, usTim1Timerout50us );
	TIMEBASE_Count_Set( TIMEBASE_4, 0 );
    (void)TIMEBASE_32bit_Prescale_Set ( TIMEBASE_4, 8 );

	// Configure Interrupt
	IPC4CLR = _IPC4_T4IP_MASK;
	IPC4SET = (MODBUS_T4_INT_PRIOR_2  << _IPC4_T4IP_POSITION);
	IEC0CLR = _IEC0_T4IE_MASK;
	IEC0SET = (1 << _IEC0_T4IE_POSITION);

	TIMEBASE_State_Set( TIMEBASE_4, TRUE );

	return TRUE;
}

inline void vMBPortTimersEnable(void)
{
    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
	
	TIMEBASE_State_Set( TIMEBASE_4, TRUE );
    ( void ) xMBPortTimersInit( gPortTimerTimeout );	//Adjust timing for real timeouts
}

inline void vMBPortTimersDisable(void)
{
    /* Disable any pending timers. */
	TIMEBASE_State_Set( TIMEBASE_4, FALSE );
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */

void __ISR(_TIMER_4_VECTOR, ipl4) Timer4Handler(void)
{
    // clear the interrupt flag
	IFS0CLR = _IFS0_T4IF_MASK;
	(void)pxMBPortCBTimerExpired();

	//mPORTAToggleBits(BIT_7);
	//(LATGINV = (uint16)(1<<14));
}
