//! \file	timebase32_period_set.c
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
void TIMEBASE_32bit_Period_Set( TIMEBASE_ID_t timebase_id, uint32 period )
{
	//uint16 lsw = (uint16)(period & 0xFFFF);
	//uint16 msw = (uint16)(period >> 16);
	
	switch( timebase_id )
	{
		case TIMEBASE_2:
			//PR4 = msw;
			//PR5 = lsw;
			PR2 = period;
			break;
		case TIMEBASE_4:
			//PR4 = msw;
			//PR5 = lsw;
			PR4 = period;
			break;
		default:
			break;
	}
}
