//! \file	timebase_period_get.c
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
uint16 TIMEBASE_Period_Get( TIMEBASE_ID_t timebase_id )
{
	uint16 rtnval = 0;

	switch( timebase_id )
	{
		case TIMEBASE_1:
			rtnval = PR1;
			break;
		case TIMEBASE_2:
			rtnval = PR2;
			break;
		case TIMEBASE_3:
			rtnval = PR3;
			break;
		case TIMEBASE_4:
			rtnval = PR4;
			break;
		case TIMEBASE_5:
			rtnval = PR5;
			break;
		default:
			break;
	}

	return rtnval;
}

