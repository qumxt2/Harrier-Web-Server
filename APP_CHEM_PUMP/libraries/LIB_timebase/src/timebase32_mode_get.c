//! \file	timebase32_mode_set.c
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
bool TIMEBASE_32bit_Mode_Get( TIMEBASE_ID_t timebase_id )
{
	bool enabled = FALSE;
	
	switch( timebase_id )
	{
		case TIMEBASE_2:
			enabled = T2CONbits.T32;
			break;

		case TIMEBASE_4:
			enabled = T4CONbits.T32;
			break;

		default:
			break;
	}
	
	return enabled;
}

