//! \file	bar_s16d16_to_mbar_s32d0.h
//! \brief Convert units of bars(in s16d16 format) to
//!        mbar(in s32d0 format)
//!
//! Copyright 2006-2008
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#include "typedef.h"

#include "units_pressure.h"


// ***************************************************
// * CONSTANTS
// ***************************************************


// ***************************************************
// * PUBLIC FUNCTIONS
// ***************************************************
sint32 bar_s16d16_to_mbar_s32d0( sint32 bar_s16d16 )
{
	uint64 tempval;
	bool neg = FALSE;
	sint64 newrtnval;
	
	// To work around a bug in the compiler (C30-29/29536), signed long-long multiplication
	// will be done with unsigned values and the sign will be handled manually.
	
	if( bar_s16d16 < 0 )
	{
		neg = TRUE;
		tempval = (uint64)((sint64)-bar_s16d16);
	}
	else
	{
		tempval = (uint64)((sint64)bar_s16d16);
	}
	
	// 1 bar = 1000 mbar
	// in u16d0 format, this = 0x03E8
	tempval = (tempval * 0x03E8U) >> 16;
	
	if( neg )
	{
		newrtnval = -(sint64)tempval;
	}
	else
	{
		newrtnval = tempval;
	}
	
	return (sint32)newrtnval;
}
