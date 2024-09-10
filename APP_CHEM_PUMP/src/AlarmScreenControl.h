// AlarmScreenControl.h

// Copyright 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Alarm Control Screen

#ifndef ALARM_SCREEN_CONTROL_H
#define ALARM_SCREEN_CONTROL_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
typedef enum
{
    LOGIC_DISABLED = 0,
    LOGIC_ACTIVE_HIGH,
    LOGIC_ACTIVE_LOW,
    NUMBER_LOGIC_LEVELS
} LOGIC_LEVELS_t;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************
INPUT_MODE_t AlarmScreenControl(INPUT_EVENT_t InputEvent);

//*******************************************************************************************


#endif
