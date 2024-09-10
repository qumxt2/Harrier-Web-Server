// AnalogOutFlowScreen.h

// Copyright 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary  for the analog flow control input

#ifndef AOUT_FLOW_SCREEN_H
#define	AOUT_FLOW_SCREEN_H


#define MA_MIN_SETTING 400
#define MA_MAX_SETTING 2000

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************
typedef enum
{
    OVERRIDE_OFF = 0,
    OVERRIDE_ON,
    NUMBER_OVERRIDE_STATES
} AOUT_OVERRIDE_STATES_t;

// **********************************************************************************************************
// Public functions
// **********************************************************************************************************
INPUT_MODE_t AnalogOutFlowScreen(INPUT_EVENT_t InputEvent);


#endif	/* AOUT_FLOW_SCREEN_H */

