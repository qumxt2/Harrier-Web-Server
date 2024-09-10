// utilities.c

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// The c file for common functions used throughout the project

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

#include "utilities.h"
#include "dvseg_17G721_setup.h"
#include "AdvancedScreen.h"
#include "PublishSubscribe.h"
#include "limits.h"
#include "stdio.h"
#include "inttypes.h"

#define MAX_VOLUME_TO_L         (1134733)       // GAL

// **********************************************************************************************************
// integerDivideRound - divide & round, returns an integer
// **********************************************************************************************************
INT32U integerDivideRound(INT32U dividend, INT32U divisor)
{
    assert(divisor > 0);
    return (dividend + (divisor >> 1)) / divisor;
}

// **********************************************************************************************************
// integerDivideRoundLarge - divide & round, returns an integer.  For cases where dividend is the product of
//                           more than one number that can exceed uint32.  In that case, they need to be
//                           explicity type cast to uint64.  End result is clamped to uint32.
// **********************************************************************************************************
INT32U integerDivideRoundLarge(uint64 dividend, INT32U divisor)
{
    uint64 result;
    
    assert(divisor > 0);    
    result = (dividend + (uint64)(divisor >> 1)) / divisor;

    // Clamp large results to uint32
    if (result > UINT_MAX)
    {
        result = UINT_MAX;
    }

    return ((INT32U)result);
}

// **********************************************************************************************************
// clampValue - clamp a value to a min or max & return clamped value
// **********************************************************************************************************
uint32 clampValue(uint32 val, uint32 max_val, uint32 min_val)
{
    if(val > max_val)
    {
        return max_val;
    }
    else if(val < min_val)
    {
        return min_val;
    }
    else
    {
        return val;
    }
}

// **********************************************************************************************************
// Interpolate - perform linear interpolation between two points using line equation in standard two point form
// **********************************************************************************************************
uint32 Interpolate(uint32 x, uint32 x1, uint32 y1, uint32 x2, uint32 y2)
{
    static sint32 a, b, c = 0;
    static uint32 y = 0;
    
    //y = ((y2 - y1) * (x - x1)/(x2 - x1)) + y1;

    a = (y2 - y1);
    b = (x - x1);
    c = (x2 - x1);
    
    // Force negative values to 0
    if ((a >= 0) && (b >= 0) && (c > 0))
    {
        y = integerDivideRoundLarge((uint64)a*(uint64)b, c) + y1;
    }
    else
    {
        y = 0;
    }
               
    return y;
}

// **********************************************************************************************************
// barToPsi - Convert pressure from bar x 10 to psi
// **********************************************************************************************************
uint32 barToPsi(uint32 pressure)
{
    pressure = bar_s16d16_to_psi_s16d16(IntegerToFixedPoint(pressure,1));// << 16) >> 16;

    pressure = FixedPointToInteger(pressure,1);

    pressure = (pressure+5)/10; //handles a rounding bug with fixed point to integer when using 0 for a decimal
    return pressure;
}

// **********************************************************************************************************
// getLocalPressure - Get the pressure in the local units psi or bar x 10.
// **********************************************************************************************************
uint32 getLocalPressure(uint32 pressure)
{
    if (gSetup.Units == UNITS_METRIC)
    {
        uint32 bar_u16d16 = psi_s16d16_to_bar_s16d16(pressure << 16);

        // Multiply by 10 and add .5 for rounding
        pressure = ((bar_u16d16 * 10)  + 0x8000) >> 16;
    }

    return pressure;
}
// **********************************************************************************************************
// litersToGallons - Convert liters to gallons
// **********************************************************************************************************
INT32U litersToGallons(INT32U liters)
{
    // 3.785 liters per gallon
    // Multiply by 1000 round and divide by 1000 for added resolution

    return integerDivideRound(liters * 1000, L_PER_GALLON_X_1000);
}

// **********************************************************************************************************
// gallonsToLiters - Convert gallons to liters
// **********************************************************************************************************
INT32U gallonsToLiters(INT32U gallons)
{
    INT32U result;
    // 3.785 liters per gallon
    // Multiply by 1000 round and divide by 1000 for added resolution

    // There are unlikely edge cases when volume extends far beyond the max, clamped to UINT_MAX.
    // Prevent those large volumes from overflowing when converted to liters
    if (gallons >= MAX_VOLUME_TO_L)
    {
        result = UINT_MAX;
    }
    else
    {
        result = integerDivideRound(gallons * L_PER_GALLON_X_1000, 1000);
    }
    
    return result;
}

// **********************************************************************************************************
// inchesToCentimeters - Convert inches to centimeters
// **********************************************************************************************************
INT32U inchesToCentimeters(INT32U inches)
{
    // 2.54 centimeters per inch
    // Multiply by 100 round and divide by 100 for added resolution

    return integerDivideRound(inches * CM_PER_INCH_X_100, 100);
}

// **********************************************************************************************************
// centimetersToInches - Convert liters to gallons
// **********************************************************************************************************
INT32U centimetersToInches(INT32U centimeters)
{
    // 2.54 centimeters per inch
    // Multiply by 100 round and divide by 100 for added resolution

    return integerDivideRound(centimeters * 100, CM_PER_INCH_X_100);
}

// **********************************************************************************************************
// fahrenheitToCelsius - Convert degrees fahrenheit to degrees Celsius
// **********************************************************************************************************
INT32 fahrenheitToCelsius(INT32 temperature)
{
    double Temperature = temperature;
    
    // C = (F-32)*(5/9))
    Temperature = ((Temperature - 32) * 5/9);
    
    // Round for positive & negative #'s
    Temperature += (0.5 - (Temperature < 0.0));
    
    return ((INT32)Temperature);
}

// **********************************************************************************************************
// celsiusToFahrenheit - Convert degrees celsius to degrees fahrenheit
// **********************************************************************************************************
INT32 celsiusToFahrenheit(INT32 temperature)
{
    double Temperature = temperature;
    // F = 1.8*C + 32 or ((9/5)C + 32)
    Temperature = ((Temperature*9/5) + 32);
    
    // Round for positive & negative #'s
    Temperature += (0.5 - (Temperature < 0.0));
    
    return ((INT32)Temperature);
}

// **********************************************************************************************************
// getLocalVolume - Converts the volume parameter from gallons to the local units.
// **********************************************************************************************************
uint32 getLocalVolume(uint32 volume)
{
    // The volumes are stored as XXX.XX *100 format.
    // They then need to be divided by 10 for use on the display in XXX.X *10 format
    
    if (gSetup.Units == UNITS_METRIC)
    {
        volume = gallonsToLiters(volume);
    }
    // Use the 64-bit version because volume can be as large as UINT_MAX
    return integerDivideRoundLarge(volume, 10);
}

// **********************************************************************************************************
// getLocalVolume - Converts the volume parameter from gallons to the local units.
// **********************************************************************************************************
uint32 getLocalTotalizer(uint32 volume)
{
    // The volumes are stored as XXX.XX *100 format.
    // We are not dividing them down before outputting for modbus to maintain precision
    
    if (gSetup.Units == UNITS_METRIC)
    {
        volume = gallonsToLiters(volume);
    }
    return volume;
}

// **********************************************************************************************************
// setLocalVolume - Convert the volume parameters from local units to gallons before storing in DVARs
//                  Currently used when setting tlm volumes from the tlm screens
//                  DVARs are not set here because the return value is validated differently in each case
// **********************************************************************************************************
uint32 setLocalVolume(uint32 volume)
{
    // Displayed as XX,XXX.X L & X,XXX.X G, multiplied by 10 so we have an extra digit of precision when converting L to G & back to L for display
    // ie: 3.3L = 0.9G = 3.4L so we need 3.3L = 0.87G = 3.3L
    volume *= 10;
    
    if (gSetup.Units == UNITS_METRIC)
    {
        volume = litersToGallons(volume);
    }
    return volume;
}

// **********************************************************************************************************
// getLocalFlowRate - Convert the flow rate to the proper value for display
// **********************************************************************************************************

INT32U getLocalFlowRate(INT32U flowRate)
{
    //Gallons are stored and displayed as XXXX.XX
    //Liters are displayed as XXXXX.X

    if (gSetup.Units == UNITS_METRIC)
    {
        flowRate = gallonsToLiters(flowRate);
        flowRate = integerDivideRound(flowRate, 10);
    }

    return flowRate;
}

// **********************************************************************************************************
// getLocalTemperature - Convert the temperature to the proper value for display
// **********************************************************************************************************

sint32 getLocalTemperature(uint32 temperature)
{
    // The temperature is stored as an unsigned value, so we need to convert it to signed
    // The temperature is stored in deg F
    
    // 1. Cast to signed
    // 2. Convert signed value to celsius if needed
    // 3. Return signed value
    
    sint32 stemp = (sint32)temperature;
    
    if (gSetup.Units == UNITS_METRIC)
    {
        stemp = fahrenheitToCelsius(stemp);
    }
    
    return stemp;
}

sint32 setLocalTemperature(uint32 temperature, uint32 dvarID)
{
    // temperature represents a signed # saved in a uint32
    // cast it to signed for the conversion & store signed value in uint32
    
    sint32 sTemperature = (sint32)temperature;

    if (gSetup.Units == UNITS_METRIC)
    {
        sTemperature = celsiusToFahrenheit(sTemperature);
    }
    
    if (sTemperature > MAX_DEG_F)
    {
        sTemperature = (sint32)MAX_DEG_F;
    }
    else if (sTemperature < MIN_DEG_F)
    {
        sTemperature = (sint32)MIN_DEG_F;
    }
    
    (void)DVAR_SetPointLocal(dvarID, (DistVarType)sTemperature);
    PublishInt32(TOPIC_TemperatureSetpoint, (int32_t)gSetup.TempSetpoint);
}
