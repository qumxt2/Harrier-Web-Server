//! \file	timebase_state_set.c
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

#include "oscillator.h"

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
void TIMEBASE_State_Set( TIMEBASE_ID_t timebase_id, bool state )
{
	uint16 ton = 0;

	if( state )
	{
		ton = 1;
	}

	switch( timebase_id )
	{
		//case TIMEBASE_1:
		//	T1CONbits.TON = ton;
		//	break;
		case TIMEBASE_2:
			T2CONbits.TON = ton;
			break;
		case TIMEBASE_3:
			T3CONbits.TON = ton;
			break;
		case TIMEBASE_4:
			T4CONbits.TON = ton;
			break;
		case TIMEBASE_5:
			T5CONbits.TON = ton;
			break;
		default:
			break;
	}
}

