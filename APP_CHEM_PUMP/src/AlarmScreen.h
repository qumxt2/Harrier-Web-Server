// AlarmScreen.h

// Copyright 2014 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Alarm Screen

#ifndef ALARM_SCREEN_H
#define ALARM_SCREEN_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
typedef enum
{
    ALARM_CONTROL = 0,
    ALARM_BATTERY,
    ALARM_PRESSURE,
    ALARM_TANK,
    ALARM_TEMP,
    ALARM_MOTOR_CURRENT,
    NUMBER_ALARM_TYPES
} ALARM_TYPES_t;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

INPUT_MODE_t AlarmScreen(INPUT_EVENT_t InputEvent);

//*******************************************************************************************


#endif
