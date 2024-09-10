//! \file	timebase_prescale_set.c
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

#include "p32mx795f512l.h"

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


// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************


// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************
void TIMEBASE_Initialize( TIMEBASE_ID_t timebase_id )
{
	switch( timebase_id )
	{
		//case TIMEBASE_1:
			// Disable timer 2 (TIMEBASE_1)
			// Disable gated time accumulation
			// Set prescaler to 1:1
			// set timer clock source to internal clock (Fcy)
		//	T1CON = 0x0000;
		//	break;
		case TIMEBASE_2:
			// Disable timer 2 (TIMEBASE_2)
			// Disable gated time accumulation
			// Set prescaler to 1:1
			// set timer clock source to internal clock (Fcy)
			T2CON = 0x0000;
			break;
		case TIMEBASE_3:
			// Disable timer 3 (TIMEBASE_3)
			// Disable gated time accumulation
			// Set prescaler to 1:1
			// set timer clock source to internal clock (Fcy)
			T3CON = 0x0000;
			break;
		case TIMEBASE_4:
			// Disable timer 4 (TIMEBASE_4)
			// Disable gated time accumulation
			// Set prescaler to 1:1
			// set timer clock source to internal clock (Fcy)
			T4CON = 0x0000;
			break;
		case TIMEBASE_5:
			// Disable timer 5 (TIMEBASE_5)
			// Disable gated time accumulation
			// Set prescaler to 1:1
			// set timer clock source to internal clock (Fcy)
			T5CON = 0x0000;
			break;
		default:
			break;
	}
}
