//! \file	timebase_state_get.c
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
bool TIMEBASE_State_Get( TIMEBASE_ID_t timebase_id )
{
	bool state = FALSE;

	switch( timebase_id )
	{
		//case TIMEBASE_1:
		//	state = T1CONbits.TON;
		//	break;

		case TIMEBASE_2:
			state = T2CONbits.TON;
			break;

		case TIMEBASE_3:
			state = T3CONbits.TON;
			break;

		case TIMEBASE_4:
			state = T4CONbits.TON;
			break;

		case TIMEBASE_5:
			state = T5CONbits.TON;
			break;

		default:
			break;
	}

	return state;
}

