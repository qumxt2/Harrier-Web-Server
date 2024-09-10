// TankScreenCustom.h

// Copyright 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the custom tank screen

#ifndef CUSTOM_SCREEN_H
#define CUSTOM_SCREEN_H

#include "in_pressure.h"
#include "screensTask.h"

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_TANK_LEVEL_DIGITS           5

typedef enum
{
    POSITION_ABOVE_TANK = 0,
    POSITION_BELOW_TANK,
    NUMBER_SENSOR_POSITIONS
} SENSOR_POSITIONS_t;

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************
INPUT_MODE_t TankScreenCustom(INPUT_EVENT_t InputEvent);
void UpdateTankLevelCustom(void);

//******************************************************************************


#endif
