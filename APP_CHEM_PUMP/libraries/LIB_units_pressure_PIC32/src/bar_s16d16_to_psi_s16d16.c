//! \file	bar_s16d16_to_psi_s16d16.h
//! \brief Convert units of bars(in s16d16 format) to
//!        psi(in s16d16 format)
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
sint32 bar_s16d16_to_psi_s16d16( sint32 bar_s16d16 )
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
	
	// 1 bar = 14.50377 psi
	// in u8d24 format, this = 0x0E80F751
	tempval = (tempval * (0x0E80F751UL)) >> 24;
	
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
