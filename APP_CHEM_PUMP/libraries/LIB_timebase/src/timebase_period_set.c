//! \file	timebase_period_set.c
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
void TIMEBASE_Period_Set( TIMEBASE_ID_t timebase_id, uint16 period )
{
	switch( timebase_id )
	{
		//case TIMEBASE_1:
		//	PR1 = period;
		//	break;
		case TIMEBASE_2:
			PR2 = period;
			break;
		case TIMEBASE_3:
			PR3 = period;
			break;
		case TIMEBASE_4:
			PR4 = period;
			break;
		case TIMEBASE_5:
			PR5 = period;
			break;
		default:
			break;
	}
}
