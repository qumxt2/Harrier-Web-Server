//! \file	timebase_prescale_get.c
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
uint16 TIMEBASE_Prescale_Get( TIMEBASE_ID_t timebase_id )
{
	uint16 prescale;

	switch( timebase_id )
	{
		//case TIMEBASE_1:
		//	prescale = T1CONbits.TCKPS;
		//	break;

		case TIMEBASE_2:
			prescale = T2CONbits.TCKPS;
			break;

		case TIMEBASE_3:
			prescale = T3CONbits.TCKPS;
			break;

		case TIMEBASE_4:
			prescale = T4CONbits.TCKPS;
			break;

		case TIMEBASE_5:
			prescale = T5CONbits.TCKPS;
			break;

		default:
			prescale = 0xFFFF;
			break;
	}

	switch( prescale )
	{
		case 0:
			prescale = 1;
			break;
		case 1:
			prescale = 8;
			break;
		case 2:
			prescale = 64;
			break;
		case 3:
			prescale = 256;
			break;
		default:
			prescale = 0;
			break;
	}

	return prescale;
}

