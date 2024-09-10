/* 
 * File:   pressure_420420.h
 * Pressure Sensor parameters for Graco p/n xxxxxx
 * Author: qukls5
 *
 * Created on April 17, 2017
 */

//! Copyright 2017
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved

#ifndef PRESSURE_420420_H
#define	PRESSURE_420420_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "in_pressure.h"

// ***************************************************
// * CONSTANTS
// ***************************************************

const PressureSensor_t pressure_420420_definition = {
    0x01900000,		// zeroBarOffset_mV_s16d16
    // nominal offset at 0bar = 400mV (represents 4mA) +- 1%
    // in mV_S16d16 format, this = 0x01890000

    0x00003728,		// scaleFactor_bar_per_mV_s16d16
    // nominal output span = 2000mV (represents 20mA) at 5psi
    // 5 psi = 0.344738 bar
    // 0.344738 bar / (2V(at 20mA) - 0.4V(at 4mA)) = 0.21546125 bar/V (3.125 PSI/V)
    // in s16d16 format, this = 0x00002C20

    0x00005840,     // fullScale_bar_s16d16
    // nominal full scale output = 5psi
    // 5 psi = 0.344738 bar
    // in s16d16 format, this = 0x00005840

    0x01900000,        // zero_cal_mVpV_s16d16
    // default zero_cal is 400 mV
    // in s16d16 format, this is 0x01900000

    0x00003728,        // sensitivity_cal_mVpV_s16d16
    // default fullScale_cal is 0.21546125 V/bar (3.125 V/PSI)
    // in s16d16 format, this is 0x00003728
};

#ifdef	__cplusplus
}
#endif

#endif	/* PRESSURE_15M669_H */

