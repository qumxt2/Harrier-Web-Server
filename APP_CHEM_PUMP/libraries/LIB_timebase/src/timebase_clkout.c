//! \file	timebase_clkout.c
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
sint32 TIMEBASE_ClkOUT_Freq_Get( TIMEBASE_ID_t timebase_id )
{
	uint16 prescale = 0;

	sint32 clkin = TIMEBASE_ClkIN_Freq_Get( timebase_id )/2;

	// 32 bit timers have different prescalers, check.
	if( TIMEBASE_32bit_Mode_Get( timebase_id ) )
	{
		prescale = TIMEBASE_32bit_Prescale_Get( timebase_id );
	}
	else
	{
		prescale = TIMEBASE_Prescale_Get( timebase_id );
	}
	
	if( clkin < 0 )
	{
		return -1;
	}
	
	if( prescale == 0 )
	{
		return -1;
	}
	 
	return (clkin / prescale);
}
