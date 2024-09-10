// screensTask.h

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Screens Task

#ifndef SCREENS_TASK_H
#define SCREENS_TASK_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"                                // Compiler specific type definitions

// Enumeration of input events
typedef enum
{
    INPUT_EVENT_ENTRY_INIT,
    INPUT_EVENT_RESET,
    INPUT_EVENT_ENTER,
    INPUT_EVENT_UP_ARROW,
    INPUT_EVENT_DOWN_ARROW,
    INPUT_EVENT_RIGHT_ARROW,
    INPUT_EVENT_LEFT_ARROW,
    INPUT_EVENT_PRESS_HOLD_ENTER,
    INPUT_EVENT_BOTH_ARROWS,
    INPUT_EVENT_REFRESH_SCREEN,

    NUMBER_OF_INPUT_EVENTS
} INPUT_EVENT_t;


// Enumeration of input modes
typedef enum
{
    INPUT_MODE_STARTUP,
    INPUT_MODE_CONFIG,
    INPUT_MODE_FLOW,
    INPUT_MODE_TIME,
    INPUT_MODE_CYCLES,
    INPUT_MODE_ALARMS,
    INPUT_MODE_ALARMS_CONTROL,
    INPUT_MODE_ALARMS_BATT,
    INPUT_MODE_ALARMS_PRESS,
    INPUT_MODE_ALARMS_TANK,
    INPUT_MODE_ALARMS_TEMP,
    INPUT_MODE_ALARMS_MOTOR_CURRENT,
    INPUT_MODE_PIN_CODE,
    INPUT_MODE_NETWORK,
    INPUT_MODE_RUN,
    INPUT_MODE_PIN_ENTRY,
    INPUT_MODE_ADVANCED,
    INPUT_MODE_ACTIVATION,
    INPUT_MODE_SNOOP,
    INPUT_MODE_FLUID_PRESS,	
    INPUT_MODE_TANK,
    INPUT_MODE_VERTICAL,
    INPUT_MODE_HORIZONTAL,
    INPUT_MODE_CUSTOM,
    INPUT_MODE_TANK_DEBUG,
    INPUT_MODE_AIN_FLOW,
    INPUT_MODE_AOUT_FLOW,
    NUMBER_OF_INPUT_MODES,
 } INPUT_MODE_t;

typedef enum
{
    VIEWPORT_MAIN = 0,
    NUMBER_OF_VIEWPORTS
} VIEWPORT_t;

typedef INPUT_MODE_t (*screen_t) (INPUT_EVENT_t event);

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
#define DEVELOPMENT_TASK_INIT_OK        (0x00)      // Initialization successful
#define DEVELOPMENT_TASK_INIT_ERROR     (0xFF)      // Initialization error
                    
// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************
void ScreensTask (void);
void ConfigInit(void);
void ClearScreen (void);
void ShowPullDownCharacter(void);
void DrawSelectBox (INT8U PositionX, INT8U PositionY, INT8U Length, INT8U Depth);
void PopulateSelectBox (INT8U Index, char *SelectText);
void SelectSelectBox (INT8U Index);
INT8U screensTaskInit (void);
void GotoScreen(INPUT_MODE_t screenID);
void RefreshScreen(void);
INPUT_MODE_t getCurrentScreen(void);

#endif
