//! \file	timebase_to_ms_u32d0.c
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

#include "limits.h"

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
mS_u32d0_t TIMEBASE_Convert_to_milliSec_u32d0( TIMEBASE_ID_t timebase_id, uint32 count )
{
	sint32 clkoutFreq = TIMEBASE_ClkOUT_Freq_Get( timebase_id );
	uint64 calc;
	
	if( clkoutFreq <= 0 )
	{
		return 0;
	}
	
	calc = (1000 * (uint64)count) / (uint32)clkoutFreq;
	
	if( calc > ULONG_MAX )
	{
		calc = ULONG_MAX;
	}
	
	return (mS_u32d0_t)calc;
}
