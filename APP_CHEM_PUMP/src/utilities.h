// utilities.h

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// The header file for common functions used throughout the project

#ifndef UTILITIES_H
#define	UTILITIES_H

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************
#include "assert_app.h"

// *****************************************************************************
// USEFUL MACROS
// *****************************************************************************
#define delay(x)                    {(void)K_Task_Wait(TICKS_MS(x));}
#define TICKS_MS(x)                 (((uint32)x*1000) / RtosGetTickPeriod())
#define MAX_DEG_F                   (118)
#define MIN_DEG_F                   (-40)

// *****************************************************************************
// Constants and macros
// *****************************************************************************
#define L_PER_GALLON_X_1000         3785

#define L_PER_GALLON_X_1000     3785
#define CM_PER_INCH_X_100       254

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************
INT32U integerDivideRound(INT32U dividend, INT32U divisor);
INT32U integerDivideRoundLarge(uint64 dividend1, INT32U divisor);
uint32 clampValue(uint32 val, uint32 max_val, uint32 min_val);
uint32 Interpolate(uint32 x, uint32 x1, uint32 y1, uint32 x2, uint32 y2);
INT32U litersToGallons(INT32U liters);
INT32U gallonsToLiters(INT32U gallons);
INT32U centimetersToInches(INT32U centimeters);
INT32U inchesToCentimeters(INT32U inches);
INT32 fahrenheitToCelsius(INT32 temperature);
INT32 celsiusToFahrenheit(INT32 temperature);

uint32 barToPsi(uint32 pressure);
uint32 getLocalPressure(uint32 pressure);
uint32 getLocalVolume(uint32 volume);
uint32 setLocalVolume(uint32 volume);
INT32U getLocalFlowRate(INT32U flowRate);
uint32 getLocalTotalizer(uint32 volume);
sint32 getLocalTemperature(uint32 temperature);
sint32 setLocalTemperature(uint32 temperature, uint32 dvarID);


#endif	/* UTILITIES_H */


