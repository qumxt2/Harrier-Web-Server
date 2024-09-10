//! \file	units_pressure.h
//! \brief Functions for pressure unit conversions
//!
//! Copyright 2006-2008
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef UNITS_PRESSURE_H
#define UNITS_PRESSURE_H

#include "typedef.h"

// ***************************************************
// * PUBLIC FUNCTIONS
// ***************************************************

sint32 bar_s16d16_to_psi_s16d16( sint32 bar_s16d16 );

sint32 psi_s16d16_to_bar_s16d16( sint32 psi_s16d16 );

sint32 bar_s16d16_to_mbar_s32d0( sint32 bar_s16d16 );

sint32 mbar_s32d0_to_bar_s16d16( sint32 mbar_s32d0 );

sint32 bar_s16d16_to_mpa_s16d16( sint32 bar_s16d16 );

sint32 mpa_s16d16_to_bar_s16d16( sint32 mpa_s16d16 );

#endif // UNITS_PRESSURE_H
