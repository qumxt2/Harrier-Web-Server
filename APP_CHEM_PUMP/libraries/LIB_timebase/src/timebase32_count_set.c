//! \file	timebase32_count_set.c
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
void TIMEBASE_32bit_Count_Set( TIMEBASE_ID_t timebase_id, uint32 count )
{
	//uint16 lsw = (uint16)(count & 0xFFFF);
	//uint16 msw = (uint16)(count >> 16);	
	
	switch( timebase_id )
	{
		case TIMEBASE_2:
			//TMR3HLD = msw;
			//TMR2 = lsw;
			TMR2 = count;
			break;
		case TIMEBASE_4:
			//TMR5HLD = msw;
			//TMR4 = lsw;
			TMR4 = count;
			break;

		default:
			break;
	}
}
