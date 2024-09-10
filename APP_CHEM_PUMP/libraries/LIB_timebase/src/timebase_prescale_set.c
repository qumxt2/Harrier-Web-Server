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
uint16 TIMEBASE_Prescale_Set( TIMEBASE_ID_t timebase_id, uint16 prescale )
{
	switch( prescale )
	{
		case 1:
			prescale = 0;
			break;
		case 8:
			prescale = 1;
			break;
		case 64:
			prescale = 2;
			break;
		case 256:
			prescale = 3;
			break;
		default:
			prescale = 0xFFFF;
			break;
	}

	if( prescale <= 3 )
	{
		switch( timebase_id )
		{
			//case TIMEBASE_1:
			//	T1CONbits.TCKPS = prescale;
			//	break;
			case TIMEBASE_2:
				T2CONbits.TCKPS = prescale;
				break;
			case TIMEBASE_3:
				T3CONbits.TCKPS = prescale;
				break;
			case TIMEBASE_4:
				T4CONbits.TCKPS = prescale;
				break;
			case TIMEBASE_5:
				T5CONbits.TCKPS = prescale;
				break;
			default:
				break;
		}
	}

	return TIMEBASE_Prescale_Get( timebase_id );
}
