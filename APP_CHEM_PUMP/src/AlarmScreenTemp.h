// AlarmScreenTemp.h

// Copyright 2018
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the alarm temp control screen

#ifndef ALARM_SCREEN_TEMP_H
#define ALARM_SCREEN_TEMP_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
#define TEMPERATURE_INVALID_DEG_F     (999)

// Definition of degree symbol in Xtreme font
#define DEGREE_SYMBOL                 (0x1)

typedef enum
{
    TEMP_CONTROL_DISABLED = 0,
    TEMP_CONTROL_DISPLAY,
    TEMP_CONTROL_OFF_ABOVE,     //"ON BELOW"
    TEMP_CONTROL_OFF_BELOW,     //"ON ABOVE"
    NUMBER_TEMP_CONTROL_MODES
} TEMP_CONTROL_MODES_t;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

INPUT_MODE_t AlarmScreenTemp(INPUT_EVENT_t InputEvent);

//*******************************************************************************************


#endif
