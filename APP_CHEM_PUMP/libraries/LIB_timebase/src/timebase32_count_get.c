//! \file	timebase32_count_get.c
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
uint32 TIMEBASE_32bit_Count_Get( TIMEBASE_ID_t timebase_id )
{
	// declared volatile because the timer registers need to be read
	// in specific order.  Don't let the compiler "get smart" on us.
	//volatile uint16 rtnval_lsw = 0;
	//volatile uint16 rtnval_msw = 0;
	volatile uint32 rtnval = 0;

	switch( timebase_id )
	{
		case TIMEBASE_2:
			rtnval = TMR2;
			//rtnval_lsw = TMR2;
			//rtnval_msw = TMR3HLD;
			break;
		case TIMEBASE_4:
			rtnval = TMR4;
			//rtnval_lsw = TMR4;
			//rtnval_msw = TMR5HLD;
			break;

		default:
			break;
	}

	//return ((uint32)rtnval_msw << 16) + rtnval_lsw;
	return rtnval;
}
