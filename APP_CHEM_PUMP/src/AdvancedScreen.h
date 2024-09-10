// AdvancedScreen.h

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Advanced Screen

#ifndef CALIBRATE_SCREEN_H
#define CALIBRATE_SCREEN_H

#include "alarms.h"

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
typedef enum
{
    UNITS_IMPERIAL = 0,
    UNITS_METRIC,
    NUMBER_UNITS_TYPES
} UNIT_TYPES_t;

typedef enum
{
    CAL_STOP = 0,
    CAL_START,
    NUMBER_CAL_STATES
} ADV_STATES_t;

typedef enum
{
    AIN_OFF = 0,
    AIN_FLOW_RATE,
    NUMBER_ANALOG_IN_CONTROL_ITEMS
} Analog_In_Control_t;

typedef enum
{
    AOUT_OFF = 0,
    AOUT_ON,
    NUMBER_ANALOG_OUT_CONTROL_ITEMS
} Analog_Out_Control_t;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

INPUT_MODE_t AdvancedScreen(INPUT_EVENT_t InputEvent);

//*******************************************************************************************


#endif
