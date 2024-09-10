// AlarmScreenTank.h

// Copyright 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Alarm Tank Screen

#ifndef ALARM_SCREEN_TANK_H
#define ALARM_SCREEN_TANK_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
typedef enum
{
    FLOW_VERIFY_DISABLED = 0,
    FLOW_VERIFY_ENABLED,
    NUMBER_FLOW_VERIFY_ITEMS
} FLOW_VERIFY_ITMES_t;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

INPUT_MODE_t AlarmScreenTank(INPUT_EVENT_t InputEvent);

//*******************************************************************************************


#endif
