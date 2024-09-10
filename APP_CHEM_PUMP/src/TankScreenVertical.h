// TankScreenVertical.h

// Copyright 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the vertical tank screen

#ifndef VERTICAL_SCREEN_H
#define VERTICAL_SCREEN_H

#include "in_pressure.h"
#include "screensTask.h"

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// Constants and macros
// ******************************************************************************************
#define TANK_PERCENT_SCALE_FACTOR   (100)

typedef enum
{
    TANK_STATE_NO = 0,
    TANK_STATE_YES,
    NUMBER_TANK_STATES
} TANK_FULL_STATES_t;

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************
INPUT_MODE_t TankScreenVertical(INPUT_EVENT_t InputEvent);
void UpdateTankLevelVertical(void);

//*******************************************************************************************


#endif
