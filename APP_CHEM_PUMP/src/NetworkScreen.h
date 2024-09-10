// NetworkScreen.h

// Copyright 2014 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the header file for the network screen

#ifndef _NETWORKSCREEN_H_
#define _NETWORKSCREEN_H_

#include "screensTask.h"

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    NETWORK_MODE_NO_NETWORK = 0,
    NETWORK_MODE_CELLULAR,
    NETWORK_MODE_MODBUS,
    NUMBER_NETWORK_MODES
} NETWORK_MODE_t;

// **********************************************************************************************************
// Public functions
// **********************************************************************************************************

INPUT_MODE_t NetworkScreen(INPUT_EVENT_t InputEvent);

#endif
