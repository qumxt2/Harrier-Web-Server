//! \file	timebase_2_event.c
//! \brief Module for DCM/ADCM timebases that are used for "high-speed" digital i/o
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This module provides access to the DCM/ADCM timebases used by the
//!    high-speed digital i/o modules on the DCM/ADCM component (Advanced / Display Control Module)


// Include basic platform types
#include "typedef.h"

#include "p32mx795f512l.h"

#include "rtos.h"

#include "timebase.h"


// ***************************************************
// * CONSTANTS
// ***************************************************


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************


// ***************************************************
// * PRIVATE (STATIC) VARIABLES
// ***************************************************
static uint8 timebase_2_task_id = RTOS_INVALID_ID;
static uint8 timebase_2_eventmask = 0;


// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************


// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************
void TIMEBASE_2_Period_RegisterEvent( uint8 task_id, uint8 eventmask )
{
	timebase_2_task_id = task_id;
	timebase_2_eventmask = eventmask;

	if( timebase_2_task_id == RTOS_INVALID_ID )
	{
		IEC0bits.T2IE = 0;
	}
	else
	{
		IPC2bits.T2IP = 5; 	// Priority level 5
		IFS0bits.T2IF = 0;
		IEC0bits.T2IE = 1;
	}
}

// ***************************************************
// * INTERRUPT HANDLERS
// ***************************************************
void _ISR_T2Interrupt( void )
{
	K_OS_Intrp_Entry();

	IFS0bits.T2IF = 0;

	// Send the event signal
	if (timebase_2_task_id != RTOS_INVALID_ID)
	{
		(void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, timebase_2_task_id, timebase_2_eventmask);
	}

	K_OS_Intrp_Exit();
}
