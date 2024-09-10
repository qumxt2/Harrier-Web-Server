/* 
 * File:   pressure_15M669.h
 * Pressure Sensor parameters for Graco p/n 16P289
 * Author: quank1
 *
 * Created on December 11, 2013, 12:00 PM
 */

//! Copyright 2013
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved

#ifndef PRESSURE_15M669_H
#define	PRESSURE_15M669_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "in_pressure.h"

// ***************************************************
// * CONSTANTS
// ***************************************************

const PressureSensor_t pressure_15M669_definition = {
    0x00040000,		// zeroBarOffset_mV_s16d16
    // nominal offset at 0bar = 4mV +- 3mV
    // in mV_S16d16 format, this = 0x00040000

    0x0002F474,		// scaleFactor_bar_per_mV_s16d16
    // nominal output span = 175mV at 7500psi
    // 7500 psi = 517.106797 bar
    // 517.106797 bar / 175 mV = 2.954896 bar/mV
    // in s16d16 format, this = 0x0002F474

    0x02051B57,     // fullScale_bar_s16d16
    // nominal full scale output = 7500psi
    // 7500 psi = 517.106797 bar
    // in s16d16 format, this = 0x02051B57

    0x00040000,		// zero_cal_mVpV_s16d16
    // default zero_cal is 0 mVpV
    // in s16d16 format, this is 0x00000000

    0x00230000,		// sensitivity_cal_mVpV_s16d16
    // default fullScale_cal is 28.571428 mVpV
    // in s16d16 format, this is 0x001C9249
};

#ifdef	__cplusplus
}
#endif

#endif	/* PRESSURE_15M669_H */

