// rtcTask.h

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the RTC
// Task, used to develop component level software

#ifndef RTC_TASK_H
#define RTC_TASK_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"								// Compiler specific type definitions


typedef void (*timerCb_t) (uint8 timerId);

typedef enum
{
    RTC_TIMER_ONE_SHOT = 0,
    RTC_TIMER_RECURRING,
    
}timerType_t;

typedef struct
{
    bool            isEnabled;
    bool            isRunning;
    uint32          value;
    uint32          expiration;
    timerCb_t       cb;
    timerType_t     type;
} rtcTimer_t;

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
#define RTC_TASK_INIT_OK                        (0x00)		// Initialization successful
#define RTC_TASK_INIT_ERROR                     (0xFF)		// Initialization error
#define RTC_TIMER_INVALID                       (0xFF)

					
// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

uint8 rtcTaskInit (void);
uint8 RTC_oneShotTimer(uint32 timeout, timerCb_t cb, uint8 timerID);
uint8 RTC_createTimer(uint32 timeout, timerCb_t cb);
uint8 RTC_resetTimer(uint8 timerID, uint32 timeout);
uint8 RTC_stopTimer(uint8 timerID);
uint8 RTC_startTimer(uint8 timerID);
uint8 RTC_destroyTimer(uint8 timerID);
uint32 RTC_getTimeRemaining(uint8 timerID);
uint32 RTC_getPercentProgress(uint8 timerID);
uint32 RTC_getUptime(void);

//*******************************************************************************************


#endif
