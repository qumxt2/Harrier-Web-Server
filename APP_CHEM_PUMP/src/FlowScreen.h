// FlowScreen.h

// Copyright 2014 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the header file for the flow rate pump settings screen

#ifndef _VOLUMESCREEN_H_
#define _VOLUMESCREEN_H_

typedef enum
{
    INTERVAL_ONE = 0,
    INTERVAL_FIVE,
    INTERVAL_TEN,
    NUMBER_INTERVALS
} INTERVAL_INDEX_t;

typedef enum
{
    CYCLE_SW_NO = 0,
    CYCLE_SW_YES,
    NUMBER_CYCLE_SW_ITEMS
} CYCLE_SW_t;

#define ONE_MIN_INTERVAL_SEC            (60 * 1)
#define FIVE_MIN_INTERVAL_SEC           (60 * 5)
#define TEN_MIN_INTERVAL_SEC            (60 * 10)
#define MAX_FLOW_RATE_GAL               (60000)     // 600.00 gallons = 2271.3 L
#define MIN_FLOW_RATE_GAL               (10)        //    .10 gallons =     .4 L
// For limiting screen inputs
#define MAX_FLOW_RATE_L                 (22713)    // liters
#define MIN_FLOW_RATE_L                 (4)        // liters

// **********************************************************************************************************
// Public functions
// **********************************************************************************************************

INPUT_MODE_t FlowScreen(INPUT_EVENT_t InputEvent);
void setLocalFlowRate(INT32U flowRate,  INT32U dvarID);

#endif
