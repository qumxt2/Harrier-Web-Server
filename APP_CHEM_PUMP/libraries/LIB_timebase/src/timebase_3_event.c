//! \file	timebase_3_event.c
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
static uint8 timebase_3_task_id = RTOS_INVALID_ID;
static uint8 timebase_3_eventmask = 0;


// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************


// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************
void TIMEBASE_3_Period_RegisterEvent( uint8 task_id, uint8 eventmask )
{
	timebase_3_task_id = task_id;
	timebase_3_eventmask = eventmask;

	if( timebase_3_task_id == RTOS_INVALID_ID )
	{
		IEC0bits.T3IE = 0;
	}
	else
	{
		IPC3bits.T3IP = 5; 	// Priority level 5
		IFS0bits.T3IF = 0;
		IEC0bits.T3IE = 1;
	}
}

// ***************************************************
// * INTERRUPT HANDLERS
// ***************************************************
void _ISR_T3Interrupt( void )
{
	K_OS_Intrp_Entry();

	IFS0bits.T3IF = 0;

	// Send the event signal
	if (timebase_3_task_id != RTOS_INVALID_ID)
	{
		(void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, timebase_3_task_id, timebase_3_eventmask);
	}

	K_OS_Intrp_Exit();
}
