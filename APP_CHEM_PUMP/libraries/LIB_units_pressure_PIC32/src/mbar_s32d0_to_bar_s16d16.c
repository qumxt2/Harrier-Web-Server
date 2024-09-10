//! \file	mbar_s32d0_to_bar_s16d16.h
//! \brief Convert units of mbar(in s32d0 format) to
//!        bar(in s16d16 format)
//!
//! Copyright 2006-2008
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#include "limits.h"
#include "typedef.h"

#include "units_pressure.h"


// ***************************************************
// * CONSTANTS
// ***************************************************


// ***************************************************
// * PUBLIC FUNCTIONS
// ***************************************************
sint32 mbar_s32d0_to_bar_s16d16( sint32 mbar_s32d0 )
{
	uint64 tempval;
	bool neg = FALSE;
	sint64 newrtnval;
	
	// To work around a bug in the compiler (C30-29/29536), signed long-long multiplication
	// will be done with unsigned values and the sign will be handled manually.
	
	if( mbar_s32d0 < 0 )
	{
		neg = TRUE;
		tempval = (uint64)((sint64)-mbar_s32d0);
	}
	else
	{
		tempval = (uint64)((sint64)mbar_s32d0);
	}
	
	// 1 mbar = .001 bar
	// in u-8d40 format, this = 0x4189374B
	tempval = (tempval * (0x4189374BUL)) >> 24;
	
	if( neg )
	{
		newrtnval = -(sint64)tempval;
	}
	else
	{
		newrtnval = tempval;
	}
	
	if( newrtnval > LONG_MAX )
	{
		newrtnval = LONG_MAX;
	}
	else if( newrtnval < LONG_MIN )
	{
		newrtnval = LONG_MIN;
	}
	
	return (sint32)newrtnval;
}
