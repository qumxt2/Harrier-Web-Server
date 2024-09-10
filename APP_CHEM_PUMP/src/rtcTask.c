// rtcTask.c

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The RtcTask is used to implement long running timers for application events

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"
#include "rtos.h"
#include "debug_app.h"
#include "rtcTask.h"
#include "sys/attribs.h"
#include "peripheral/rtcc.h"
#include "define_app.h"
#include "assert_app.h"
#include "utilities.h"

// ******************************************************************************************
// CONSTANTS AND MACROS
// ******************************************************************************************
#define RTC_TIC_EVENT_FLAG                      (RTOS_EVENT_FLAG_1)
#define NUM_RTC_TIMERS                          (20)
#define ALWAYS_WAIT_FOR_MATCH					(0x0000)

// ******************************************************************************************
// STATIC VARIABLES
// ******************************************************************************************
static uint8 gRtcTaskID;
static rtcTimer_t gRtcTimer[NUM_RTC_TIMERS];
static uint32 gRtcUptime;
uint8 gRtcResourceID;

// ******************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// ******************************************************************************************
void RtcTask (void);
static void timerTic(void);
static void timerExpired(uint8 timerID);
static void enableRtcInterrupts(void);
static uint8 getNewTimerId(void);
static void initializeTimers(void);
static bool isActiveTimer(uint8 timerID);

// ******************************************************************************************
// PUBLIC FUNCTIONS
// ******************************************************************************************

uint8 rtcTaskInit (void)
{
	uint8 status = RTC_TASK_INIT_OK;

    // Reserve a resource
    gRtcResourceID = RtosResourceReserveID();
    if (gRtcResourceID == RTOS_INVALID_ID)
    {
        status = RTC_TASK_INIT_ERROR;
    }

	// Reserve a task ID and queue the task
	gRtcTaskID = RtosTaskCreateStart(RtcTask);
	if (gRtcTaskID == RTOS_INVALID_ID)
	{
		status = RTC_TASK_INIT_ERROR;
	}
	
	if (status == RTC_TASK_INIT_ERROR)
	{
		DEBUG_PRINT_STRING(DBUG_ALWAYS, "RTC TASK INIT ERROR\r\n");
	}
	
    initializeTimers();

    return status;
}

uint8 RTC_oneShotTimer(uint32 timeout, timerCb_t cb, uint8 timerID)
{
    (void)K_Resource_Get(gRtcResourceID);

    // Allow an existing timer to be reset if given
    if (!isActiveTimer(timerID))
    {
        timerID = getNewTimerId();
    }

    if (timerID != RTC_TIMER_INVALID)
    {
        gRtcTimer[timerID].isEnabled = TRUE;
        gRtcTimer[timerID].isRunning = TRUE;
        gRtcTimer[timerID].value = 0;
        gRtcTimer[timerID].expiration = timeout;
        gRtcTimer[timerID].cb = cb;
        gRtcTimer[timerID].type = RTC_TIMER_ONE_SHOT;
    }

    (void)K_Resource_Release(gRtcResourceID);
    
    return timerID;
}

uint8 RTC_createTimer(uint32 timeout, timerCb_t cb)
{
    (void)K_Resource_Get(gRtcResourceID);

    const uint8 timerID = getNewTimerId();

    if (timerID != RTC_TIMER_INVALID)
    {
        gRtcTimer[timerID].isEnabled = TRUE;
        gRtcTimer[timerID].isRunning = FALSE;
        gRtcTimer[timerID].value = 0;
        gRtcTimer[timerID].expiration = timeout;
        gRtcTimer[timerID].cb = cb;
        gRtcTimer[timerID].type = RTC_TIMER_RECURRING;
    }

    (void)K_Resource_Release(gRtcResourceID);

    return timerID;
}

uint8 RTC_resetTimer(uint8 timerID, uint32 timeout)
{
    (void)K_Resource_Get(gRtcResourceID);

    if ( (timerID != RTC_TIMER_INVALID) && (timeout > 0) )
    {
        gRtcTimer[timerID].expiration = timeout;
        gRtcTimer[timerID].value = 0;
    }

    (void)K_Resource_Release(gRtcResourceID);

    return timerID;
}

uint8 RTC_stopTimer(uint8 timerID)
{
    (void)K_Resource_Get(gRtcResourceID);

    if (timerID != RTC_TIMER_INVALID)
    {
        gRtcTimer[timerID].isRunning = FALSE;
    }

    (void)K_Resource_Release(gRtcResourceID);

    return timerID;
}

uint8 RTC_startTimer(uint8 timerID)
{
    (void)K_Resource_Get(gRtcResourceID);

    if ( (timerID != RTC_TIMER_INVALID) && (gRtcTimer[timerID].expiration > 0) )
    {
        gRtcTimer[timerID].isRunning = TRUE;
    }

    (void)K_Resource_Release(gRtcResourceID);

    return timerID;
}

uint8 RTC_destroyTimer(uint8 timerID)
{
    (void)K_Resource_Get(gRtcResourceID);

    if (timerID != RTC_TIMER_INVALID)
    {
        gRtcTimer[timerID].isEnabled = FALSE;
    }

    (void)K_Resource_Release(gRtcResourceID);

    return RTC_TIMER_INVALID;
}

uint32 RTC_getTimeRemaining(uint8 timerID)
{
    (void)K_Resource_Get(gRtcResourceID);

    uint32 timeRemaining = 0;

    if ( (timerID != RTC_TIMER_INVALID) &&
         (gRtcTimer[timerID].expiration > gRtcTimer[timerID].value) )
    {
        timeRemaining = gRtcTimer[timerID].expiration - gRtcTimer[timerID].value;
    }

    (void)K_Resource_Release(gRtcResourceID);

    return timeRemaining;
}

uint32 RTC_getPercentProgress(uint8 timerID)
{
    (void)K_Resource_Get(gRtcResourceID);

    uint32 progress = 0;

    if ( (timerID != RTC_TIMER_INVALID) && (gRtcTimer[timerID].expiration > 0) && (gRtcTimer[timerID].value > 0) )
    {
        progress = integerDivideRound((gRtcTimer[timerID].value + 1) * 100, gRtcTimer[timerID].expiration);
    }

    (void)K_Resource_Release(gRtcResourceID);

    return progress;
}

// Get the number of seconds the system has been running
uint32 RTC_getUptime(void)
{
    uint32 result = 0;

    (void)K_Resource_Get(gRtcResourceID);
    result = gRtcUptime;
    (void)K_Resource_Release(gRtcResourceID);

    return result;
}

// ******************************************************************************************
// PRIVATE FUNCTIONS
// ******************************************************************************************

void RtcTask (void)
{
	static uint8 status;

    enableRtcInterrupts();
    
	if (status != K_OK)
	{
		DEBUG_PRINT_STRING(DBUG_ALWAYS, "DEVELOPMENT TASK TIMER ERROR\r\n");
	}

    // Enter the cyclic portion of the task.
	for (;;)
	{
        status = K_Event_Wait(RTC_TIC_EVENT_FLAG,ALWAYS_WAIT_FOR_MATCH,RTOS_CLEAR_EVENT_FLAGS_AFTER);

        (void)K_Resource_Get(gRtcResourceID);
        timerTic();
        (void)K_Resource_Release(gRtcResourceID);
    }
}

static void timerTic(void)
{
    uint8 i;

    //DEBUG_PRINT_STRING(DBUG_PUMP, "tic\r\n");

    // Increment timers
    for (i = 0; i < NUM_RTC_TIMERS; i++)
    {
        if ( (gRtcTimer[i].isEnabled == TRUE) && (gRtcTimer[i].isRunning == TRUE) )
        {
             gRtcTimer[i].value++;
        }
    }

    // Check for expired timers
    for (i = 0; i < NUM_RTC_TIMERS; i++)
    {
        if (gRtcTimer[i].value >= gRtcTimer[i].expiration)
        {
            gRtcTimer[i].value = 0;
            timerExpired(i);
        }
    }

    // Increment global uptime counter
    gRtcUptime++;
}

static void timerExpired(uint8 timerID)
{
    if (gRtcTimer[timerID].type == RTC_TIMER_ONE_SHOT)
    {
        gRtcTimer[timerID].isEnabled = FALSE;
    }

    // Intiate callback
    if (gRtcTimer[timerID].cb != NULL)
    {
        gRtcTimer[timerID].cb(timerID);
    }
}

static void initializeTimers(void)
{
    uint8 i;

    for (i = 0; i < NUM_RTC_TIMERS; i++)
    {
        gRtcTimer[i].isEnabled = FALSE;
    }
}

static uint8 getNewTimerId(void)
{
    uint8 i;

    for (i = 0; i < NUM_RTC_TIMERS; i++)
    {
        if (gRtcTimer[i].isEnabled == FALSE)
        {
            // return early if a timer is found
            return i;
        }
    }

    assert_always();
    return RTC_TIMER_INVALID;
}

static bool isActiveTimer(uint8 timerID)
{
    bool retVal = FALSE;

    (void)K_Resource_Get(gRtcResourceID);

    if (timerID < NUM_RTC_TIMERS && gRtcTimer[timerID].isEnabled)
    {
        retVal = TRUE;
    }

    (void)K_Resource_Release(gRtcResourceID);

    return retVal;
}

static void enableRtcInterrupts(void)
{
    // Unlock RTC and SOSC
    SYSKEY = 0xaa996655;
    SYSKEY = 0x556699aa;

   // Enable the secondary oscillator
    OSCCONbits.SOSCEN = 1;
    while (!OSCCONbits.SOSCRDY)
    {
        // If stuck here, oscillator did not start
        // todo - add timeout and error reporting
    }

    // Enable writing to the
    RTCCONbits.RTCWREN = 1;

    // Turn on RTC
    RTCCONbits.ON = 1;

    // Disable RTC interrupt
    IEC1bits.RTCCIE = 0;

    // Clear RTCC interrupt flag
    IFS1bits.RTCCIF = 0;

    // Set interrupt priority to 3
    IPC8bits.RTCCIP = 3;

    // Iterrupt continuously once per second
    RTCALRMbits.AMASK = 1;
    RTCALRMbits.CHIME = 1;
    RTCALRMbits.ALRMEN = 1;

    // Enable RTC interrupt
    IEC1bits.RTCCIE = 1;

    // Disable writing to RTC
    RTCCONbits.RTCWREN = 0;

    // Lock RTC and SOSC
    SYSKEY = 0;
}

void __ISR(_RTCC_VECTOR, ipl4) RtccIsr(void)
{
    // Clear RTCC interrupt flag
    (void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, gRtcTaskID, RTC_TIC_EVENT_FLAG);
    IFS1bits.RTCCIF = 0;
}

