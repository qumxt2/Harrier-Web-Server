// pumpControlTask.c

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The pumpControlTask is used to implement the pump control state machine

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"
#include "rtos.h"
#include "debug_app.h"
#include "pumpControlTask.h"
#include "FSM.h"
#include "out_digital.h"
#include "io_pin.h"
#include "rtcTask.h"
#include "volumeTask.h"
#include "dvseg_17G721_setup.h"
#include "dvseg_17G721_run.h"
#include "PublishSubscribe.h"
#include "assert_app.h"
#include "alarms.h"
#include <stdlib.h>
#include "screensTask.h"
#include "AlarmScreenBattery.h"
#include "FlowScreen.h"
#include "dvseg_17G721_usage.h"
#include "utilities.h"
#include "AdvancedScreen.h"

// ******************************************************************************************
// CONSTANTS AND MACROS
// ******************************************************************************************
#define PUMP_ON_EVENT_FLAG                  (RTOS_EVENT_FLAG_1)
#define PUMP_OFF_EVENT_FLAG                 (RTOS_EVENT_FLAG_2)
#define STANDBY_EVENT_FLAG                  (RTOS_EVENT_FLAG_3)
#define RUN_EVENT_FLAG                      (RTOS_EVENT_FLAG_4)
#define RESTART_EVENT_FLAG                  (RTOS_EVENT_FLAG_5)
#define PARAM_CHANGE_EVENT_FLAG             (RTOS_EVENT_FLAG_6)
#define CALIBRATE_EVENT_FLAG                (RTOS_EVENT_FLAG_7)

#define ALL_PUMP_EVENTS                      (PUMP_ON_EVENT_FLAG | PUMP_OFF_EVENT_FLAG | \
                                             STANDBY_EVENT_FLAG | RUN_EVENT_FLAG | \
                                             RESTART_EVENT_FLAG | PARAM_CHANGE_EVENT_FLAG | \
                                             CALIBRATE_EVENT_FLAG)

#define ALWAYS_WAIT_FOR_MATCH               (0x0000)

#define POWER_SAVE_MIN_PERCENT              (75u)
#define POWER_SAVE_NORMAL_PERCENT           (50u)
#define POWER_SAVE_MAX_PERCENT              (25u)

enum PumpControlSignals
{
    SIG_PUMP_ON = SIG_USER,
    SIG_PUMP_OFF,
    SIG_STANDBY,
    SIG_RUN,
    SIG_RESET,
    SIG_PARAM_CHANGE,
    SIG_PRIME_PUMP,
    SIG_CALIBRATE,
};

// ******************************************************************************************
// STATIC VARIABLES
// ******************************************************************************************
static uint8 gPumpControlTaskID;
static fsm_t gPumpStateMachine;
static uint8 gOnTimerID = RTC_TIMER_INVALID;
static uint8 gOffTimerID = RTC_TIMER_INVALID;
static uint8 gVolumeTimerID = RTC_TIMER_INVALID;
static uint8 gOnTimeoutID = RTC_TIMER_INVALID;
static handler_t gCurrentRunState;
static void (*gfpCalibrationCompleteHandler)() = NULL;
static bool gIsPowersaveMode = FALSE;

// Pump parameters
static uint32 gOnTime;
static uint32 gOffTime;
static uint32 gOnCycles;
static uint32 gDesiredFlowRate;

// ******************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// ******************************************************************************************
void PumpControlTask (void);

// State handlers
static state_t ResetState(fsm_t* pMe, evt_t const *pEvt);
static state_t StandbyState(fsm_t* pMe, evt_t const *pEvt);
static state_t TimeModeOnState(fsm_t* pMe, evt_t const *pEvt);
static state_t TimeModeOffState(fsm_t* pMe, evt_t const *pEvt);
static state_t CycleModeOnState(fsm_t* pMe, evt_t const *pEvt);
static state_t CycleModeOffState(fsm_t* pMe, evt_t const *pEvt);
static state_t VolumeModeOnState(fsm_t* pMe, evt_t const *pEvt);
static state_t VolumeModeOffState(fsm_t* pMe, evt_t const *pEvt);
static state_t CalibrationState(fsm_t* pMe, evt_t const *pEvt);

// Callbacks
static void pumpOnCallback(uint8 id);
static void pumpOffCallback(uint8 id);
static void onTimeoutCallback(uint8 id);
static void SetFlowRateCallback(uint8_t* pPayload, uint16_t len);
static void SetOnTimeCallback(uint8_t* pPayload, uint16_t len);
static void SetOffTimeCallback(uint8_t* pPayload, uint16_t len);
static void SetOnCyclesCallback(uint8_t* pPayload, uint16_t len);
static void SetOnTimeoutCallback(uint8_t* pPayload, uint16_t len);


// Private functions
static void turnPumpOn(void);
static void turnPumpOff(void);
static void turnSafetyRelayOn(void);
static void turnSafetyRelayOff(void);
static void updatePumpParameters(void);

// ******************************************************************************************
// PUBLIC FUNCTIONS
// ******************************************************************************************

uint8 pumpControlTaskInit (void)
{
    uint8 status = PUMP_CONTROL_TASK_INIT_OK;

    // Reserve a task ID and queue the task
    gPumpControlTaskID = RtosTaskCreateStart(PumpControlTask);
    if (gPumpControlTaskID == RTOS_INVALID_ID)
    {
        status = PUMP_CONTROL_TASK_INIT_ERROR;
    }
    
    if (status == PUMP_CONTROL_TASK_INIT_ERROR)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "PUMP CONTROL TASK INIT ERROR\r\n");
    }
    
    return status;
}

void PMP_setRunMode(void)
{
    // Don't let the pump run if there are any alarms active or if the flow 
    // rate is 0
    if (ALARM_ActiveAlarms() || (TRUE == gRun.RemoteDisableActive) ||
        (getLocalFlowRate(gSetup.DesiredFlowRate) == 0) || (TRUE == gRun.TempProbeDisableActive))
    {
        PMP_setStandbyMode();
    }
    else
    {
        (void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, gPumpControlTaskID, RUN_EVENT_FLAG);
        GotoScreen(INPUT_MODE_RUN);

        if ( (TRUE == gIsPowersaveMode) && (POWER_SAVE_OFF != gSetup.PowerSaveMode) )
        {
            (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gRun, PumpStatus), (DistVarType)PUMP_STATUS_Powersave);
        }
        else
        {
            (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gRun, PumpStatus), (DistVarType)PUMP_STATUS_Run);
        }
    }

    PublishUint32(TOPIC_PumpStatus, gRun.PumpStatus);
}

void PMP_setStandbyMode(void)
{
    (void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, gPumpControlTaskID, STANDBY_EVENT_FLAG);

    // Ensure that the correct standby sub-type is published
    if (ALARM_ActiveAlarms())
    {
        (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gRun, PumpStatus), (DistVarType)PUMP_STATUS_Lockout_Alarm);
    }
    else if (gRun.RemoteDisableActive == TRUE)
    {
        (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gRun, PumpStatus), (DistVarType)PUMP_STATUS_Lockout_Remote);
    }
    else if (gRun.TempProbeDisableActive == TRUE)
    {
        (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gRun, PumpStatus), (DistVarType)PUMP_STATUS_Lockout_Temperature);
    }
    else
    {
        (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gRun, PumpStatus), (DistVarType)PUMP_STATUS_Standby);
    }

    PublishUint32(TOPIC_PumpStatus, gRun.PumpStatus);
}

void PMP_resetStates(void)
{
    (void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, gPumpControlTaskID, RESTART_EVENT_FLAG);
}

void PMP_calibrate(void(*pFuncCalibrationComplete)())
{
    (void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, gPumpControlTaskID, CALIBRATE_EVENT_FLAG);
    gfpCalibrationCompleteHandler = pFuncCalibrationComplete;
}

void PMP_primePump(void)
{
    PMP_resetStates();
    PMP_setRunMode();
}

uint32 PMP_getCycleProgress(void)
{
    uint32 progress = 0;

    if (gCurrentRunState == (handler_t)TimeModeOnState)
    {
        progress = RTC_getPercentProgress(gOnTimerID);
    }
    
    else if (gCurrentRunState == (handler_t)TimeModeOffState)
    {
        progress = RTC_getPercentProgress(gOffTimerID);
    }
    
    else if (gCurrentRunState == (handler_t)CycleModeOnState)
    {
        progress = VOL_getCountsPercentProgress();
    }
    
    else if (gCurrentRunState == (handler_t)CycleModeOffState)
    {
        progress = RTC_getPercentProgress(gOffTimerID);
    }

    else if (gCurrentRunState == (handler_t)VolumeModeOnState)
    {
        progress = VOL_getVolumePercentProgress();
    }

    else if (gCurrentRunState == (handler_t)VolumeModeOffState)
    {
        const uint32 totalOffTime = VOL_getCycleInterval() - VOL_getVolumeModeOnTime();
        const uint32 remainingTime = RTC_getTimeRemaining(gVolumeTimerID);
        uint32 offTimeElapsed = 0;

        if (totalOffTime > remainingTime)
        {
            offTimeElapsed = totalOffTime - remainingTime;
        }

        if (totalOffTime > 0)
        {
            progress = integerDivideRound(offTimeElapsed * 100, totalOffTime);
        }
        else
        {
            progress = 100;
        }
    }

    return progress;
}

bool PMP_isRunning(void)
{
    if ( (gPumpStateMachine.state == (handler_t)ResetState) || 
         (gPumpStateMachine.state == (handler_t)StandbyState) ||
         (gPumpStateMachine.state == NULL) )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

bool PMP_isOffCycleRunning(void)
{
    const bool pumpRunning = PMP_isRunning();
    IOrtn_digital_t pumpOutput;
    bool rtnVal = FALSE;
    
    pumpOutput = OUT_Digital_Latch_Get(IOPIN_OUTPUT_1);
    
    if (pumpRunning)
    {
        if ((pumpOutput.error == IOError_Ok) && (pumpOutput.state == NOT_ASSERTED))
        {
            rtnVal = TRUE;
        }
    }
    return rtnVal;
}

bool PMP_isOnCycleRunning(void)
{
    const bool pumpRunning = PMP_isRunning();
    IOrtn_digital_t pumpOutput;
    bool rtnVal = FALSE;
    
    pumpOutput = OUT_Digital_Latch_Get(IOPIN_OUTPUT_1);
    
    if (pumpRunning)
    {
        if ((pumpOutput.error == IOError_Ok) && (pumpOutput.state == ASSERTED))
        {
            rtnVal = TRUE;
        }
    }
    return rtnVal;
}

void PMP_updatePowerSaveMode(void)
{
    const bool keepPumpRunning = PMP_isRunning();

    if (gRun.PumpStatus == PUMP_STATUS_Powersave)
    {
        gIsPowersaveMode = TRUE;
    }
    else
    {
        gIsPowersaveMode = FALSE;     
    }
    
    PMP_resetStates();

    // If pump was running, start it running again
    if (keepPumpRunning)
    {
        PMP_setRunMode();
    }
}

uint32 PMP_adjustForPowerSavingsMode(uint32 param)
{
    if (gIsPowersaveMode == TRUE)
    {
        switch (gSetup.PowerSaveMode)
        {
            case POWER_SAVE_OFF:
            case POWER_SAVE_NOTIFY:
                break;

            case POWER_SAVE_MIN:
                param = integerDivideRound(param * POWER_SAVE_MIN_PERCENT, 100);
                break;

            case POWER_SAVE_NORMAL:
                param = integerDivideRound(param * POWER_SAVE_NORMAL_PERCENT, 100);
                break;

            case POWER_SAVE_MAX:
                param = integerDivideRound(param * POWER_SAVE_MAX_PERCENT, 100);
                break;

            default:
                break;
        }
    }

    return param;
}

void PMP_resetPumpTimeout(void)
{
    (void)RTC_resetTimer(gOnTimeoutID, gSetup.OnTimeout);
}

void PMP_TurnPumpOff(void)
{
    pumpOffCallback(0); //the id parameter is not used, just send 0
}

void PMP_DisableSafetyRelay(void)
{
    turnSafetyRelayOff();
}

void PMP_ActivateSafetyRelay(void)
{
    turnSafetyRelayOn();
}

// ******************************************************************************************
// PRIVATE FUNCTIONS
// ******************************************************************************************

void PumpControlTask (void)
{
    static uint8 events;

    // Wait for DVARs to be ready
    (void)K_Resource_Wait(gDvarLockoutResID, 0);
    (void)K_Resource_Release(gDvarLockoutResID);

    updatePumpParameters();

    // Create timers and volume monitors
    gOnTimerID = RTC_createTimer(gOnTime, pumpOffCallback);
    gOffTimerID = RTC_createTimer(gOffTime, pumpOnCallback);
    gVolumeTimerID = RTC_createTimer(VOL_getCycleInterval(), pumpOnCallback);
    gOnTimeoutID = RTC_createTimer(gSetup.OnTimeout, onTimeoutCallback);
    VOL_createVolumeWatcher(gDesiredFlowRate, pumpOffCallback);
    VOL_createCountWatcher(gOnCycles, pumpOffCallback);

    Subscribe(TOPIC_SetFlowRate, SetFlowRateCallback);
    Subscribe(TOPIC_SetOnTime, SetOnTimeCallback);
    Subscribe(TOPIC_SetOffTime, SetOffTimeCallback);
    Subscribe(TOPIC_SetOnCycles, SetOnCyclesCallback);
    Subscribe(TOPIC_SetOnTimeout, SetOnTimeoutCallback);

    // Initialize the state machine
    FSM_init(&gPumpStateMachine, (handler_t)ResetState);

    // Turn on Safety Relay
    turnSafetyRelayOn();

    // Restore PumpStatus if pump was previously running.  Must wait for DVARs to be ready before using gUsage parameters.
    if (gUsage.PumpStatusBackup != PUMP_STATUS_Standby)
    {
        PMP_setRunMode();
    }
    SYSTEM_enableBackupTimer();

    // Enter the cyclic portion of the task.
    for (;;)
    {
        events = K_Event_Wait(ALL_PUMP_EVENTS, ALWAYS_WAIT_FOR_MATCH,
                              RTOS_CLEAR_EVENT_FLAGS_AFTER);

        if (events == 0)
        {
            // Event error
        }
        
        // Translate the OS event to a state machine event
        else 
        {
            if (events & RESTART_EVENT_FLAG)
            {
               FSM_dispatchSig(&gPumpStateMachine, SIG_RESET);
            }
            
            if (events & PUMP_ON_EVENT_FLAG)
            {
                FSM_dispatchSig(&gPumpStateMachine, SIG_PUMP_ON);
            }
            
            if (events & PUMP_OFF_EVENT_FLAG)
            {
                FSM_dispatchSig(&gPumpStateMachine, SIG_PUMP_OFF);
            }

            if (events & STANDBY_EVENT_FLAG)
            {
               FSM_dispatchSig(&gPumpStateMachine, SIG_STANDBY);
             }
            
            if (events & RUN_EVENT_FLAG)
            {
               FSM_dispatchSig(&gPumpStateMachine, SIG_RUN);
            }
            
            if (events & PARAM_CHANGE_EVENT_FLAG)
            {
               FSM_dispatchSig(&gPumpStateMachine, SIG_PARAM_CHANGE);
            }

            if (events & CALIBRATE_EVENT_FLAG)
            {
               FSM_dispatchSig(&gPumpStateMachine, SIG_CALIBRATE);
            }
        }
    }
} 

static state_t ResetState(fsm_t* pMe, evt_t const *pEvt)
{
    switch(pEvt->sig)
    {
        case SIG_ENTRY:
        case SIG_RESET:
           DEBUG_PRINT_STRING(DBUG_PUMP, "ResetState\r\n");
           updatePumpParameters();
           (void)RTC_resetTimer(gOnTimerID, PMP_adjustForPowerSavingsMode(gOnTime));
           (void)RTC_resetTimer(gOffTimerID, gOffTime);
           (void)RTC_resetTimer(gVolumeTimerID, VOL_getCycleInterval());
           (void)RTC_resetTimer(gOnTimeoutID, gSetup.OnTimeout);
           VOL_resetVolumeWatcher(gDesiredFlowRate);
           VOL_resetCountWatcher(PMP_adjustForPowerSavingsMode(gOnCycles));
           return FSM_Handled();

        case SIG_EXIT:
            return FSM_Handled();

        case SIG_RUN:
            switch (gSetup.MeteringMode)
            {
                case METERING_MODE_VOLUME:
                    return FSM_TRAN(VolumeModeOnState);

                case METERING_MODE_TIME:
                    return FSM_TRAN(TimeModeOnState);

                case METERING_MODE_CYCLES:
                    return FSM_TRAN(CycleModeOnState);
            }
            return FSM_Handled();

        case SIG_CALIBRATE:
            return FSM_TRAN(CalibrationState);
    }
    return FSM_Handled();
}

static state_t StandbyState(fsm_t* pMe, evt_t const *pEvt)
{
    switch(pEvt->sig)
    {
        case SIG_ENTRY:
            DEBUG_PRINT_STRING(DBUG_PUMP, "StandbyState\r\n");
            VOL_BackupTotals();
            return FSM_Handled();

        case SIG_EXIT:
            return FSM_Handled();

        case SIG_RUN:
            return FSM_TRAN(gCurrentRunState);

        case SIG_CALIBRATE:
            return FSM_TRAN(CalibrationState);

        case SIG_RESET:
            return FSM_TRAN(ResetState);
    }
    return FSM_Handled();
}

static state_t TimeModeOnState(fsm_t* pMe, evt_t const *pEvt)
{
    switch(pEvt->sig)
    {
        case SIG_ENTRY:
            DEBUG_PRINT_STRING(DBUG_PUMP, "TimeModeOnState: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_PUMP, RTC_getTimeRemaining(gOnTimerID));
            DEBUG_PRINT_STRING(DBUG_PUMP, "sec\r\n");
            gCurrentRunState = (handler_t)TimeModeOnState;
            turnPumpOn();
            (void)RTC_startTimer(gOnTimerID);
            return FSM_Handled();

        case SIG_EXIT:
            turnPumpOff();
            (void)RTC_stopTimer(gOnTimerID);
            return FSM_Handled();

        case SIG_STANDBY:
            return FSM_TRAN(StandbyState);

         case SIG_PUMP_OFF:
            if (gSetup.OffTime > 0)
            {
                return FSM_TRAN(TimeModeOffState);
            }
            else
            {
                // Stay in on mode if the pump off time is 0
                return FSM_Handled();
            }
  
        case SIG_RESET:
            return FSM_TRAN(ResetState);
    }
    return FSM_Handled();
}

static state_t TimeModeOffState(fsm_t* pMe, evt_t const *pEvt)
{
    switch(pEvt->sig)
    {
        case SIG_ENTRY:
            DEBUG_PRINT_STRING(DBUG_PUMP, "TimeModeOffState: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_PUMP, RTC_getTimeRemaining(gOffTimerID));
            DEBUG_PRINT_STRING(DBUG_PUMP, "sec\r\n");
            gCurrentRunState = (handler_t)TimeModeOffState;
            (void)RTC_startTimer(gOffTimerID);
            return FSM_Handled();

        case SIG_EXIT:
            (void)RTC_stopTimer(gOffTimerID);
            return FSM_Handled();

        case SIG_STANDBY:
            return FSM_TRAN(StandbyState);

        case SIG_PUMP_ON:
            return FSM_TRAN(TimeModeOnState);

        case SIG_RESET:
            return FSM_TRAN(ResetState);
    }
    return FSM_Handled();
}

static state_t CycleModeOnState(fsm_t* pMe, evt_t const *pEvt)
{
    switch(pEvt->sig)
    {
        case SIG_ENTRY:
            DEBUG_PRINT_STRING(DBUG_PUMP, "CycleModeOnState: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_PUMP, VOL_getCountsRemaining());
            DEBUG_PRINT_STRING(DBUG_PUMP, "counts\r\n");
            gCurrentRunState = (handler_t)CycleModeOnState;
            turnPumpOn();
            VOL_startCountWatcher();
            (void)RTC_resetTimer(gOnTimeoutID, gSetup.OnTimeout);
            (void)RTC_startTimer(gOnTimeoutID);
            return FSM_Handled();

        case SIG_EXIT:
            turnPumpOff();
            VOL_stopCountWatcher();
            (void)RTC_stopTimer(gOnTimeoutID);
            return FSM_Handled();

        case SIG_STANDBY:
            return FSM_TRAN(StandbyState);

         case SIG_PUMP_OFF:
            return FSM_TRAN(CycleModeOffState);

        case SIG_RESET:
            return FSM_TRAN(ResetState);
    }
    return FSM_Handled();
}

static state_t CycleModeOffState(fsm_t* pMe, evt_t const *pEvt)
{
    switch(pEvt->sig)
    {
        case SIG_ENTRY:
            DEBUG_PRINT_STRING(DBUG_PUMP, "CycleModeOffState: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_PUMP, RTC_getTimeRemaining(gOffTimerID));
            DEBUG_PRINT_STRING(DBUG_PUMP, "sec\r\n");
            gCurrentRunState = (handler_t)CycleModeOffState;
            (void)RTC_startTimer(gOffTimerID);
            return FSM_Handled();

        case SIG_EXIT:
            turnPumpOff();
            (void)RTC_stopTimer(gOffTimerID);
            return FSM_Handled();

        case SIG_STANDBY:
            return FSM_TRAN(StandbyState);

        case SIG_PUMP_ON:
            return FSM_TRAN(CycleModeOnState);

        case SIG_RESET:
            return FSM_TRAN(ResetState);
    }
    return FSM_Handled();
}

static state_t VolumeModeOnState(fsm_t* pMe, evt_t const *pEvt)
{
    switch(pEvt->sig)
    {
        case SIG_ENTRY:
            DEBUG_PRINT_STRING(DBUG_PUMP, "VolumeModeOnState: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_PUMP, VOL_getVolumeTimeRemaining());
            DEBUG_PRINT_STRING(DBUG_PUMP, "msec\r\n");
            gCurrentRunState = (handler_t)VolumeModeOnState;
            turnPumpOn();
            VOL_startVolumeWatcher();
            (void)RTC_startTimer(gVolumeTimerID);
            if (gSetup.CycleSwitchEnable == CYCLE_SW_YES)
            {
                 // Only use the timeout if there's a cycle switch hooked up
                (void)RTC_startTimer(gOnTimeoutID);
            }
            return FSM_Handled();

        case SIG_EXIT:
            if (gSetup.AnalogOutControl == AOUT_OFF)        // Keep pump on all the time when analog out is ON, except when going to standby
            {
                turnPumpOff();
            }
            (void)RTC_stopTimer(gVolumeTimerID);
            VOL_stopVolumeWatcher();
            (void)RTC_stopTimer(gOnTimeoutID);
            return FSM_Handled();

        case SIG_STANDBY:
            if (gSetup.AnalogOutControl == AOUT_ON)        // Pump needs to be turned off when going to standby if analog out is ON
            {
                turnPumpOff();
            }
            return FSM_TRAN(StandbyState);

         case SIG_PUMP_OFF:
            return FSM_TRAN(VolumeModeOffState);

        case SIG_PUMP_ON:
            // Special case where the off time expires before the on timer.
            // This is a race condition for when the system is constantly running.
            // In this case, stay in pump on mode.
            (void)RTC_resetTimer(gVolumeTimerID, VOL_getCycleInterval());
            VOL_resetVolumeWatcher(gDesiredFlowRate);
            return FSM_TRAN(VolumeModeOnState);

        case SIG_RESET:
            return FSM_TRAN(ResetState);
    }
    return FSM_Handled();
}

static state_t VolumeModeOffState(fsm_t* pMe, evt_t const *pEvt)
{
    switch(pEvt->sig)
    {
        case SIG_ENTRY:
            DEBUG_PRINT_STRING(DBUG_PUMP, "VolumeModeOffState: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_PUMP, RTC_getTimeRemaining(gVolumeTimerID));
            DEBUG_PRINT_STRING(DBUG_PUMP, "sec\r\n");
            gCurrentRunState = (handler_t)VolumeModeOffState;
            (void)RTC_startTimer(gVolumeTimerID);
            if (gSetup.AnalogOutControl == AOUT_ON)        // Pump needs to be turned on for analog out in case it's coming from standby
            {
                turnPumpOn();
            } 
            return FSM_Handled();

        case SIG_EXIT:
            (void)RTC_stopTimer(gVolumeTimerID);
            return FSM_Handled();

        case SIG_STANDBY:
            if (gSetup.AnalogOutControl == AOUT_ON)        // Pump needs to be turned off when going to standby if analog out is ON
            {
                turnPumpOff();
            }            
            return FSM_TRAN(StandbyState);

        case SIG_PUMP_ON:
            return FSM_TRAN(VolumeModeOnState);

        case SIG_RESET:
            return FSM_TRAN(ResetState);
    }
    return FSM_Handled();
}

static state_t CalibrationState(fsm_t* pMe, evt_t const *pEvt)
{
    switch(pEvt->sig)
    {
        case SIG_ENTRY:
            DEBUG_PRINT_STRING(DBUG_PUMP, "CalibrationState: ");
            gCurrentRunState = (handler_t)CalibrationState;
            turnPumpOn();
            VOL_resetCountWatcher(10);
            VOL_startCountWatcher();
            (void)RTC_resetTimer(gOnTimeoutID, gSetup.OnTimeout);
            (void)RTC_startTimer(gOnTimeoutID);
            return FSM_Handled();

        case SIG_EXIT:
            turnPumpOff();
            VOL_stopCountWatcher();
            (void)RTC_stopTimer(gOnTimeoutID);

            // Update the display so it can prompt the user to enter a volume
            if (gfpCalibrationCompleteHandler != NULL)
            {
                gfpCalibrationCompleteHandler();
            }
            return FSM_Handled();

        case SIG_STANDBY:
            return FSM_TRAN(ResetState);

         case SIG_PUMP_OFF:
            return FSM_TRAN(ResetState);

        case SIG_RESET:
            return FSM_TRAN(ResetState);
    }
    return FSM_Handled();
}
static void pumpOnCallback(uint8 id)
{
    (void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, gPumpControlTaskID, PUMP_ON_EVENT_FLAG);
}

static void pumpOffCallback(uint8 id)
{
    (void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, gPumpControlTaskID, PUMP_OFF_EVENT_FLAG);
}

static void onTimeoutCallback(uint8 id)
{
    // Cycle mode timeout additional logic is required here to go to alarm screen
    (void)ALARM_ActivateAlarm(ALARM_ID_Count_Not_Achieved);
}

static void turnPumpOn(void)
{
    (void)OUT_Digital_Latch_Set(IOPIN_OUTPUT_1, ASSERTED);
    if(ALARM_AlarmState(ALARM_ID_Hardware_Fault) == TRUE)   //hardware fault is active, control safety relay to control the motor
    {
        turnSafetyRelayOn();
    }
}

static void turnPumpOff(void)
{
    (void)OUT_Digital_Latch_Set(IOPIN_OUTPUT_1, NOT_ASSERTED);
    if(ALARM_AlarmState(ALARM_ID_Hardware_Fault) == TRUE)   //hardware fault is active, control safety relay to contol the motor
    {
        turnSafetyRelayOff();
    }
}

static void turnSafetyRelayOn(void)
{
    (void)OUT_Digital_Latch_Set(IOPIN_OUTPUT_2, ASSERTED);
}

static void turnSafetyRelayOff(void)
{
    (void)OUT_Digital_Latch_Set(IOPIN_OUTPUT_2, NOT_ASSERTED);
}

static void updatePumpParameters(void)
{
    gOnTime = gSetup.OnTime;
    gOffTime = gSetup.OffTime;
    gOnCycles = gSetup.OnCycles;
    gDesiredFlowRate = gSetup.DesiredFlowRate;
}

// **********************************************************************************************************
// SetOnTimeCallback - The callback for when the on time is changed via the web
// **********************************************************************************************************

static void SetOnTimeCallback(uint8_t* pPayload, uint16_t len)
{
    uint32 onTime;
    const bool keepPumpRunning = PMP_isRunning();

    // Add null terminator and extract flow rate from payload.
    pPayload[len] = '\0';
    onTime = strtoul((char*)pPayload, NULL, 10);
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OnTime), (DistVarType)onTime);
    PMP_resetStates();
    PublishUint32(TOPIC_OnTime, gSetup.OnTime);

    // If pump was running, start it running again
    if (keepPumpRunning)
    {
        PMP_setRunMode();
    }
}

// **********************************************************************************************************
// SetOffTimeCallback - The callback for when the off time is changed via the web
// **********************************************************************************************************

static void SetOffTimeCallback(uint8_t* pPayload, uint16_t len)
{
    uint32 offTime;
    const bool keepPumpRunning = PMP_isRunning();

    // Add null terminator and extract flow rate from payload.
    pPayload[len] = '\0';
    offTime = strtoul((char*)pPayload, NULL, 10);
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OffTime), (DistVarType)offTime);
    PMP_resetStates();
    PublishUint32(TOPIC_OffTime, gSetup.OffTime);

    // If pump was running, start it running again
    if (keepPumpRunning)
    {
        PMP_setRunMode();
    }
}


// **********************************************************************************************************
// SetFlowRateCallback - The callback for when the flow rate is changed via the web
// **********************************************************************************************************

static void SetFlowRateCallback(uint8_t* pPayload, uint16_t len)
{
    uint32 desiredFlowRate;
    const bool keepPumpRunning = PMP_isRunning();

    // Add null terminator and extract flow rate from payload.
    pPayload[len] = '\0';
    desiredFlowRate = strtoul((char*)pPayload, NULL, 10);
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, DesiredFlowRate), (DistVarType)desiredFlowRate);
    PMP_resetStates();
    PublishUint32(TOPIC_FlowRate, gSetup.DesiredFlowRate);

    // If pump was running, start it running again
    if (keepPumpRunning)
    {
        PMP_setRunMode();
    }
}

// **********************************************************************************************************
// SetOnCyclesCallback - Callback for setting the on cycles via the web
// **********************************************************************************************************

static void SetOnCyclesCallback(uint8_t* pPayload, uint16_t len)
{
    uint32 cycleCount;
    const bool keepPumpRunning = PMP_isRunning();

    // Add null terminator and extract flow rate from payload.
    pPayload[len] = '\0';
    cycleCount = strtoul((char*)pPayload, NULL, 10);
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OnCycles), (DistVarType)cycleCount);
    PMP_resetStates();
    PublishUint32(TOPIC_OnCycles, gSetup.OnCycles);

    // If pump was running, start it running again
    if (keepPumpRunning)
    {
        PMP_setRunMode();
    }
}

// **********************************************************************************************************
// SetOnTimeoutCallback - Callback for setting the on timeout via the web
// **********************************************************************************************************

static void SetOnTimeoutCallback(uint8_t* pPayload, uint16_t len)
{
    uint32 onTimeout;
    const bool keepPumpRunning = PMP_isRunning();

    // Add null terminator and extract flow rate from payload.
    pPayload[len] = '\0';
    onTimeout = strtoul((char*)pPayload, NULL, 10);
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OnTimeout), (DistVarType)onTimeout);
    PMP_resetStates();
    PublishUint32(TOPIC_OnTimeout, gSetup.OnTimeout);

    // If pump was running, start it running again
    if (keepPumpRunning)
    {
        PMP_setRunMode();
    }
}
