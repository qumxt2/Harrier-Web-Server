// ConfigScreen.h

// Copyright 2014 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the header for the configuration menu screen

#ifndef _CONFIGSCREEN_H_
#define _CONFIGSCREEN_H_

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

// Enemeration of select screen modes
typedef enum
{
    SELECT_POSITION_INIT,
    SELECT_POSITION_PUMP = SELECT_POSITION_INIT,
    SELECT_POSITION_PIN_ALARMS,
    SELECT_POSITION_PIN_CODE,
    SELECT_POSITION_NETWORK,
    NUMBER_OF_SELECT_POSITIONS
} SELECT_POSITION_t;

typedef enum
{
    PUMP_MODE_VOLUME = 0,
    PUMP_MODE_TIME,
    PUMP_MODE_CYCLES,
    NUMBER_PUMP_MODES
} PUMP_MODE_t;

// **********************************************************************************************************
// Public functions
// **********************************************************************************************************

INPUT_MODE_t ConfigScreen(INPUT_EVENT_t InputEvent);
void showActiveAlarms(void);

#endif
