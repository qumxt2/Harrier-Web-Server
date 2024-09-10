//! \file	psi_s16d16_to_bar_s16d16.h
//! \brief Convert units of psi(in s16d16 format) to
//!        bar(in s16d16 format)
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
sint32 psi_s16d16_to_bar_s16d16( sint32 psi_s16d16 )
{
	uint64 tempval;
	bool neg = FALSE;
	sint64 newrtnval;
	
	// To work around a bug in the compiler (C30-29/29536), signed long-long multiplication
	// will be done with unsigned values and the sign will be handled manually.
	
	if( psi_s16d16 < 0 )
	{
		neg = TRUE;
		tempval = (uint64)((sint64)-psi_s16d16);
	}
	else
	{
		tempval = (uint64)((sint64)psi_s16d16);
	}
	
	// 1 psi = .0689475729317 bar
	// in u0d32 format, this = 0x11A68C53
	tempval = (tempval * 0x11A68C53UL) >> 32;
	
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
