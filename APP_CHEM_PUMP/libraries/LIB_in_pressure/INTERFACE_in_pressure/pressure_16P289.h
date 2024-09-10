//! \file	pressure_16P289.h
//! \brief Pressure Sensor parameters for Graco p/n 16P289
//!
//! Copyright 2013
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef PRESSURE_16P289_H
#define PRESSURE_16P289_H

#include "in_pressure.h"

// ***************************************************
// * CONSTANTS
// ***************************************************

const PressureSensor_t pressure_16P289_definition = {
    0x00000000,		// zeroBarOffset_mV_s16d16
    // nominal offset at 0bar = 0mV +- 3mV
    // in mV_S16d16 format, this = 0x00000000

    0x00005840,		// scaleFactor_bar_per_mV_s16d16
    // nominal output span = 100mV at 500psi
    // 500 psi = 34.4737865 bar
    // 34.4737865 bar / 100 mV = 0.344737865 bar/mV
    // in s16d16 format, this = 0x00005840

    0x0022794A,     // fullScale_bar_s16d16
    // nominal full scale output = 500psi
    // 500 psi = 34.4737865 bar
    // in s16d16 format, this = 0x0022794A

    0x00000000,		// zero_cal_mVpV_s16d16
    // default zero_cal is 0 mVpV
    // in s16d16 format, this is 0x00000000

    0x00140000,		// sensitivity_cal_mVpV_s16d16
    // default fullScale_cal is 20 mVpV
    // in s16d16 format, this is 0x00140000
};

#endif // PRESSURE_16P289_H
