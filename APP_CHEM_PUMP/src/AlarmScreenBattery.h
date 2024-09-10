// AlarmScreenBattery.h

// Copyright 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Alarm Screen

#ifndef ALARM_SCREEN_BATT_H
#define ALARM_SCREEN_BATT_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
typedef enum
{
    POWER_SAVE_OFF = 0,
    POWER_SAVE_NOTIFY,
    POWER_SAVE_MIN,
    POWER_SAVE_NORMAL,
    POWER_SAVE_MAX,
    NUMBER_POWER_SAVE_MODES
} POWER_SAVE_MODES_t;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

INPUT_MODE_t AlarmScreenBattery(INPUT_EVENT_t InputEvent);

//*******************************************************************************************


#endif
