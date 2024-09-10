// volumeTask.c

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The volumeTask is used to implement tracking pump volume and counts

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "PublishSubscribe.h"
#include "typedef.h"
#include "rtos.h"
#include "debug_app.h"
#include "volumeTask.h"
#include "io_typedef.h"
#include "out_digital.h"
#include "rtcTask.h"
#include "inputDebounceTask.h"
#include "assert_app.h"
#include "define_app.h"
#include "dvseg_17G721_setup.h"
#include "dvseg_17G721_run.h"
#include "pumpControlTask.h"
#include "screensTask.h"
#include "FlowScreen.h"
#include "AdvancedScreen.h"
#include "utilities.h"
#include "systemTask.h"
#include "limits.h"
#include "AnalogOutFlowScreen.h"
#include "d2a.h"

// ******************************************************************************************
// CONSTANTS AND MACROS
// ******************************************************************************************
#define VOLUME_TASK_TIC_EVENT_FLAG      (RTOS_EVENT_FLAG_1)
#define VOLUME_TASK_TASK_FREQ_HZ        (100u)
#define START_IMMEDIATELY               (0x0001)
#define ALWAYS_WAIT_FOR_MATCH           (0x0000)

#define MSECS_PER_SEC                   (1000u)
#define SECS_PER_MIN                    (60u)
#define MSEC_PER_TIC                    (MSECS_PER_SEC / VOLUME_TASK_TASK_FREQ_HZ)
#define VOLUME_SCALE_FACTOR             (100000UL)
#define CYCLE_TIME                      (VOL_getCycleInterval() * 10u)
#define MAX_ON_TIME                     (5999u)
#define MINS_PER_DAY                    (60UL * 24UL)
#define ARG_NOT_USED                    (0u)
#define BACKUP_TOTALS_FREQUENCY         (900u)
#define OUNCE_PER_GALLON                (128u)
#define CC_PER_GALLON                   (3785u)
#define CAL_VOLUME_SCALE_FACTOR         (1000u)
#define FLOW_MODE                       (0)
#define D2A_FULLSCALE                   (0x0FFF)
#define VARIABLE_SPEED_MIN_RPMS         (10)
#define MAX_DAC_OUTPUT_MV               (2500)  // mV
#define MIN_DAC_OUTPUT_MV               (0)     // mV
#define PERCENT_SCALE_FACTOR            (100)
#define DEFAULT_CYCLE_TIMER             (1000u) // ms

// ******************************************************************************************
// STATIC VARIABLES
// ******************************************************************************************
static uint8 gVolumeTaskID;
static uint8 gVolumeTaskTimerID;
static watcher_t gVolumeWatcher;
static watcher_t gCountWatcher;
static uint32 gPumpRpm = DEFAULT_PUMP_RPM;
uint8 gWatcherResourceID;
uint32 gVolumeModeOnTime;
static uint8 gTotalsBackupTimerID = RTC_TIMER_INVALID;
static float gTenthGalPerStroke = 0.0;
static float gTotal = 0.0;
static float gGrandTotal = 0.0;
static D2A_CHANNEL_t d2aChannel = D2A_MODULE_2_CHA;     // Default production part AD5338ARMZ-1

// ******************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// ******************************************************************************************
void VolumeTask (void);
static void UpdateTotals(void);
static void watchVolume(IOState_t reedSwitchState);
static void watchCounts(IOState_t reedSwitchState);
static void fakeReedCallback(uint8 id);
static uint32 calculateOnTime(void);
static void backupTotalsCallback(uint8 id);
static void resetTotalizerCallback(uint8_t* pPayload, uint16_t len);
static uint32 analogFlowInterpolate(void);

// ******************************************************************************************
// PUBLIC FUNCTIONS
// ******************************************************************************************

uint8 volumeTaskInit (void)
{
    uint8 status = VOLUME_TASK_INIT_OK;

    // Reserve a task timer
    gVolumeTaskTimerID = RtosTimerReserveID();
    if (gVolumeTaskTimerID == RTOS_INVALID_ID)
    {
        status = VOLUME_TASK_INIT_ERROR;
    }

    // Reserve a resource
    gWatcherResourceID = RtosResourceReserveID();
    if (gWatcherResourceID == RTOS_INVALID_ID)
    {
        status = VOLUME_TASK_INIT_ERROR;
    }

    // Reserve a task ID and queue the task
    gVolumeTaskID = RtosTaskCreateStart(VolumeTask);
    if (gVolumeTaskID == RTOS_INVALID_ID)
    {
        status = VOLUME_TASK_INIT_ERROR;
    }
    
    if (status == VOLUME_TASK_INIT_ERROR)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "VOLUME TASK INIT ERROR\r\n");
    }

    return status;
}

void VOL_createVolumeWatcher(uint32 desiredFlowRate, volumeCb_t cb)
{
    (void)K_Resource_Get(gWatcherResourceID);

    gVolumeWatcher.isEnabled = TRUE;
    gVolumeWatcher.isRunning = FALSE;
    gVolumeWatcher.value = 0;
    gVolumeWatcher.threshold = calculateOnTime();
    gVolumeWatcher.cb = cb;

    (void)K_Resource_Release(gWatcherResourceID);
}

void VOL_stopVolumeWatcher(void)
{
    (void)K_Resource_Get(gWatcherResourceID);

    gVolumeWatcher.isRunning = FALSE;

    (void)K_Resource_Release(gWatcherResourceID);

}

void VOL_startVolumeWatcher(void)
{
    (void)K_Resource_Get(gWatcherResourceID);

    gVolumeWatcher.isRunning = TRUE;

    (void)K_Resource_Release(gWatcherResourceID);
}

void VOL_resetVolumeWatcher(uint32 desiredFlowRate)
{
    (void)K_Resource_Get(gWatcherResourceID);

    gVolumeWatcher.isRunning = FALSE;
    gVolumeWatcher.value = 0;
    gVolumeWatcher.threshold = calculateOnTime();

    (void)K_Resource_Release(gWatcherResourceID);
}

void VOL_resetVolumeThreshold(void)
{
    (void)K_Resource_Get(gWatcherResourceID);
    
    gVolumeWatcher.threshold = calculateOnTime();

    (void)K_Resource_Release(gWatcherResourceID);
}

uint32 VOL_getVolumeTimeRemaining(void)
{
    (void)K_Resource_Get(gWatcherResourceID);

    uint32 timeRemaining = 0;

    if (gVolumeWatcher.threshold  > gVolumeWatcher.value)
    {
        timeRemaining = gVolumeWatcher.threshold - gVolumeWatcher.value;
    }

    (void)K_Resource_Release(gWatcherResourceID);

    return timeRemaining;
}

uint32 VOL_getVolumePercentProgress(void)
{
    (void)K_Resource_Get(gWatcherResourceID);

    uint32 progress = 0;

    if (gVolumeWatcher.threshold  > 0)
    {
        progress = integerDivideRound(gVolumeWatcher.value * 100, gVolumeWatcher.threshold);
    }

    (void)K_Resource_Release(gWatcherResourceID);

    return progress;
}

uint32 VOL_getVolumeModeOnTime(void)
{
    return gVolumeModeOnTime;
}

void VOL_createCountWatcher(uint32 counts, volumeCb_t cb)
{
    (void)K_Resource_Get(gWatcherResourceID);

    gCountWatcher.isEnabled = TRUE;
    gCountWatcher.isRunning = FALSE;
    gCountWatcher.value = 0;
    gCountWatcher.threshold = counts;
    gCountWatcher.cb = cb;

    (void)K_Resource_Release(gWatcherResourceID);
}

void VOL_stopCountWatcher(void)
{
    (void)K_Resource_Get(gWatcherResourceID);

    gCountWatcher.isRunning = FALSE;

    (void)K_Resource_Release(gWatcherResourceID);
}

void VOL_startCountWatcher(void)
{
    (void)K_Resource_Get(gWatcherResourceID);

    gCountWatcher.isRunning = TRUE;

    (void)K_Resource_Release(gWatcherResourceID);
}

void VOL_resetCountWatcher(uint32 counts)
{
    (void)K_Resource_Get(gWatcherResourceID);

    gCountWatcher.isRunning = FALSE;
    gCountWatcher.value = 0;
    gCountWatcher.threshold = counts;

    (void)K_Resource_Release(gWatcherResourceID);
}

uint32 VOL_getCountsRemaining(void)
{
    (void)K_Resource_Get(gWatcherResourceID);

    uint32 timeRemaining = 0;

    if (gCountWatcher.threshold  > gCountWatcher.value)
    {
        timeRemaining = gCountWatcher.threshold - gCountWatcher.value;
    }

    (void)K_Resource_Release(gWatcherResourceID);

    return timeRemaining;
}

uint32 VOL_getCountsPercentProgress(void)
{
    (void)K_Resource_Get(gWatcherResourceID);

    uint32 progress = 0;

    if (gCountWatcher.threshold  > 0)
    {
        progress = integerDivideRound(gCountWatcher.value * 100, gCountWatcher.threshold);
    }

    (void)K_Resource_Release(gWatcherResourceID);

    return progress;
}

void VOL_fakeReedSwitch(bool enabled)
{
    static uint8 fakeReedTimerID = RTC_TIMER_INVALID;

    if (fakeReedTimerID == RTC_TIMER_INVALID)
    {
        fakeReedTimerID = RTC_createTimer(1, fakeReedCallback);
    }

    if (enabled)
    {
        (void)RTC_startTimer(fakeReedTimerID);
    }
    else
    {
        (void)RTC_stopTimer(fakeReedTimerID);
    }
}

void VOL_BackupTotals(void)
{
    backupTotalsCallback(gTotalsBackupTimerID);
}

void VOL_updateKfactor(uint32 volume)
{
    uint32 kfactor;
    
    if (gSetup.Units == UNITS_METRIC)
    {
        kfactor = integerDivideRound(volume * CAL_VOLUME_SCALE_FACTOR, CC_PER_GALLON);
    }
    else
    {
        kfactor = integerDivideRound(volume * CAL_VOLUME_SCALE_FACTOR, OUNCE_PER_GALLON);
    }

    if (kfactor == 0)
    {
        kfactor = 1;
    }
    
    if (kfactor > 9999)
    {
        kfactor = 9999;
    }

    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, KFactor), kfactor);
}

uint32 VOL_getCycleInterval(void)
{
    uint32_t retVal = ONE_MIN_INTERVAL_SEC;

    switch (gSetup.VolumeModeInterval)
    {
        case INTERVAL_ONE:
            retVal = ONE_MIN_INTERVAL_SEC;
            break;
 
        case INTERVAL_FIVE:
            retVal = FIVE_MIN_INTERVAL_SEC;
            break;
 
        case INTERVAL_TEN:
            retVal = TEN_MIN_INTERVAL_SEC;
            break;
 
        default:
            break;
    }

    return retVal;
}

// ******************************************************************************************
// PRIVATE FUNCTIONS
// ******************************************************************************************

void VolumeTask (void)
{
    static uint8 status;
    IOrtn_digital_t ioReedSwitch;

    // Initialize watchers
    gVolumeWatcher.isEnabled = FALSE;
    gVolumeWatcher.cb = NULL;
    gCountWatcher.isEnabled = FALSE;
    gCountWatcher.cb = NULL;

    // Wait for DVARs to be ready
    (void)K_Resource_Wait(gDvarLockoutResID, 0);
    (void)K_Resource_Release(gDvarLockoutResID);
    
    // Default to the RPM from the nameplate if no reed switch is hooked up
    // Note: Must wait for DVARS to be ready
    if(gSetup.CycleSwitchEnable == CYCLE_SW_NO)
    {
        gPumpRpm = gSetup.RpmNameplate;
    }
    
    // Create and start the timer
    status = K_Timer_Create(gVolumeTaskTimerID,RTOS_NOTIFY_SPECIFIC,gVolumeTaskID,VOLUME_TASK_TIC_EVENT_FLAG);
    status |= K_Timer_Start(gVolumeTaskTimerID,START_IMMEDIATELY,(RtosGetTickFreq()/VOLUME_TASK_TASK_FREQ_HZ));

    if (status != K_OK)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "DEVELOPMENT TASK TIMER ERROR\r\n");
    }

    gTotalsBackupTimerID = RTC_createTimer(BACKUP_TOTALS_FREQUENCY, backupTotalsCallback);
    (void)RTC_startTimer(gTotalsBackupTimerID);
    
    // Restore the backup of the totals
    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, Total), gSetup.TotalBackup);                // xxx.xx x100 (saved as x100 to maintain decimal precision for modbus)
    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, GrandTotal), gSetup.GrandTotalBackup);      // xxx.xx x100 (saved as x100 to maintain decimal precision for modbus)
    
    gTotal = (float)gSetup.TotalBackup/100;                                                 // Divide scaled uint32 backup back down for internal float totalizers
    gGrandTotal = (float)gSetup.GrandTotalBackup/100;                                       // Divide scaled uint32 backup back down for internal float totalizers
    
    PublishUintFloat(TOPIC_Totalizer, gTotal);
    PublishUintFloat(TOPIC_GrandTotalizer, gGrandTotal);

    Subscribe(TOPIC_ResetTotalizer, resetTotalizerCallback);

    // Enter the cyclic portion of the task.
    for (;;)
    {
        status = K_Event_Wait(VOLUME_TASK_TIC_EVENT_FLAG,ALWAYS_WAIT_FOR_MATCH,RTOS_CLEAR_EVENT_FLAGS_AFTER);

         ioReedSwitch = DB_DigitalStateGet(DB_INPUT_1);
        
        if (ioReedSwitch.error == IOError_Ok)
        {
            (void)K_Resource_Get(gWatcherResourceID);
            watchVolume(ioReedSwitch.state);
            watchCounts(ioReedSwitch.state);
            (void)K_Resource_Release(gWatcherResourceID);
        }
    }
}

static void watchVolume(IOState_t reedSwitchState)
{
    IOrtn_digital_t pumpOutput;
    static uint32 cycleTimer = 0;
    static IOState_t lastState;

    // Since there is no flow meter and the reed switch doesn't have a high resolution,
    // time is used to determine how long the pump remains on.

    if ( (gVolumeWatcher.isEnabled == TRUE) && (gVolumeWatcher.isRunning == TRUE) )
    {
        gVolumeWatcher.value += MSEC_PER_TIC;

        if (gVolumeWatcher.value >= gVolumeWatcher.threshold)
        {
            assert(gVolumeWatcher.cb != NULL);
            gVolumeWatcher.value = 0;
            gVolumeWatcher.cb(ARG_NOT_USED);
        }
    }

    pumpOutput = OUT_Digital_Latch_Get(IOPIN_OUTPUT_1);
    if ( (pumpOutput.error == IOError_Ok) && (pumpOutput.state == ASSERTED) )
    {
        cycleTimer += MSEC_PER_TIC;
    }
    
    if (gSetup.CycleSwitchEnable == CYCLE_SW_NO)
    {
        uint32 nameplateCycleTimer = DEFAULT_CYCLE_TIMER;

        if ( gSetup.RpmNameplate > 0)
        {
            // Determine how fast to increment totalizer based on RPM.  Defaults to once a second if RPM not > 0, which should never happen.
            nameplateCycleTimer = integerDivideRound(SECS_PER_MIN * MSECS_PER_SEC, gSetup.RpmNameplate);
        }
        
        if (cycleTimer >= nameplateCycleTimer)
        {
            PMP_resetPumpTimeout();
            cycleTimer = 0;
            
            UpdateTotals();
        }
    }
    else if ( (lastState == ASSERTED) && (reedSwitchState == NOT_ASSERTED) && (cycleTimer > 0) )
    {
        const uint32 rpm = integerDivideRound(SECS_PER_MIN * MSECS_PER_SEC, cycleTimer);

        PMP_resetPumpTimeout();

        cycleTimer = 0;

        if(gSetup.AnalogOutControl != AOUT_ON)
        {
            if (rpm > gPumpRpm)
            {
                gPumpRpm++;
                gVolumeWatcher.threshold = calculateOnTime();
                DEBUG_PRINT_STRING(DBUG_PUMP, "RPM - ");
                DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_PUMP, gPumpRpm);
                DEBUG_PRINT_STRING(DBUG_PUMP, "\r\n");
            }

            if (rpm < gPumpRpm)
            {
                gPumpRpm--;
                gVolumeWatcher.threshold = calculateOnTime();
                DEBUG_PRINT_STRING(DBUG_PUMP, "RPM - ");
                DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_PUMP, gPumpRpm);
                DEBUG_PRINT_STRING(DBUG_PUMP, "\r\n");
            }
        }

        UpdateTotals();
    }
    lastState = reedSwitchState;
}

static void UpdateTotals(void)
{
    if ( (gSetup.MeteringMode == METERING_MODE_VOLUME) || (gSetup.MeteringMode == METERING_MODE_CYCLES) )
    {
        // Use doubles internally to maintain precision, but store and communicate
        // totals as uint32
        gTotal += gTenthGalPerStroke;
        gGrandTotal += gTenthGalPerStroke;
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, Total), (uint32)(gTotal*100 + .5));  // shift to .01 gal and round for improved accuracy on the MODBUS side
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, GrandTotal), (uint32)(gGrandTotal*100 + .5));
    }
}

static void watchCounts(IOState_t reedSwitchState)
{
    static IOState_t lastState;

    if ( (gCountWatcher.isEnabled == TRUE)&& (gCountWatcher.isRunning == TRUE) )
    {
        if ( (lastState == ASSERTED) && (reedSwitchState == NOT_ASSERTED) )
        {
            gCountWatcher.value++;
        }

        lastState = reedSwitchState;

        if (gCountWatcher.value >= gCountWatcher.threshold)
        {
            assert(gCountWatcher.cb != NULL);
            gCountWatcher.value = 0;
            gCountWatcher.cb(ARG_NOT_USED);
        }
    }
}

static void fakeReedCallback(uint8 id)
{
    watchVolume(ASSERTED);
    watchCounts(ASSERTED);

    watchVolume(NOT_ASSERTED);
    watchCounts(NOT_ASSERTED);
}

static uint32 calculateOnTime(void)
{
    uint32 desiredFlowRate = gSetup.DesiredFlowRate;
    uint32 flowRateLowmA = gSetup.FlowRateLowmASetpoint;
    uint32 flowRateHighmA = gSetup.FlowRateHighmASetpoint;
    const uint32 maxFlowRate = integerDivideRound(gSetup.KFactor * MINS_PER_DAY * gPumpRpm, VOLUME_SCALE_FACTOR / 10);
    uint32 onTime;
    uint32 minFlowRate = MIN_FLOW_RATE_GAL;
    
    if(gSetup.AnalogInControl == AIN_FLOW_RATE && gSetup.MeteringMode == FLOW_MODE)
    {
        flowRateLowmA = clampValue(flowRateLowmA, maxFlowRate, MIN_FLOW_RATE_GAL);
        flowRateHighmA = clampValue(flowRateHighmA, maxFlowRate, MIN_FLOW_RATE_GAL);
        
        if (gSetup.FlowRateLowmASetpoint != flowRateLowmA)
        {
            (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, FlowRateLowmASetpoint), flowRateLowmA);
            RefreshScreen();
        }   
    
        if (gSetup.FlowRateHighmASetpoint != flowRateHighmA)
        {
            (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, FlowRateHighmASetpoint), flowRateHighmA);
            RefreshScreen();
        }
        
        desiredFlowRate = analogFlowInterpolate();
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, AnalogFlowRate), desiredFlowRate);
    }
    else
    {
        desiredFlowRate = clampValue(desiredFlowRate, maxFlowRate, minFlowRate);
        
        // Update system if a new flow rate is being used
        if (gSetup.DesiredFlowRate != desiredFlowRate)
        {
            (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, DesiredFlowRate), desiredFlowRate);
            PublishUint32(TOPIC_FlowRate, gSetup.DesiredFlowRate);
            RefreshScreen();
        }
    }

    //prevent a divide by 0 error
    if (maxFlowRate > 0)
    {
        onTime = integerDivideRound(desiredFlowRate * CYCLE_TIME, maxFlowRate);
    }
    else
    {
        onTime = MAX_ON_TIME;
    }

    if( onTime > MAX_ON_TIME )
    {
        onTime = MAX_ON_TIME;
    }

    // Adjust on time for power savings mode
    onTime = PMP_adjustForPowerSavingsMode(onTime);
    
    // Update global variables used in the rest of the module
    gVolumeModeOnTime = integerDivideRound(onTime, 10);
    gTenthGalPerStroke = (float)gSetup.KFactor / (float)VOLUME_SCALE_FACTOR;
    
    return onTime * 100;
}

static void backupTotalsCallback(uint8 id)
{
    if (gRun.Total != gSetup.TotalBackup)
    {
        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TotalBackup), (gRun.Total));  // Save with two decimals, xx.xx x100, to maintain precision for modbus
    }

    if (gRun.GrandTotal != gSetup.GrandTotalBackup)
    {
        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, GrandTotalBackup), (gRun.GrandTotal)); // Save with two decimals, xx.xx x100, to maintain precision for modbus
    }

    PublishUintFloat(TOPIC_Totalizer, gTotal);
    PublishUintFloat(TOPIC_GrandTotalizer, gGrandTotal);

    (void)RTC_resetTimer(id, BACKUP_TOTALS_FREQUENCY);
}

static void resetTotalizerCallback(uint8_t* pPayload, uint16_t len)
{
    gTotal = 0.0;
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TotalBackup), 0);
    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, Total), 0);
    PublishUintFloat(TOPIC_Totalizer, 0.0f);
}

void resetTotal(void)
{
    resetTotalizerCallback(NULL, 0);
}

//This function handles the interpolation of the flow rate when using the analog input
static uint32 analogFlowInterpolate(void)
{
    uint32 flowRate = 0;
    uint32 aVal = gRun.AnalogInmV;
    uint32 fRateLow = gSetup.FlowRateLowmASetpoint;
    uint32 fRateHigh = gSetup.FlowRateHighmASetpoint;
    uint32 lowmASetpoint = gSetup.LowmASetpoint;
    uint32 highmASetpoint = gSetup.HighmASetpoint;         
    
    // Clamp to default min & max signal range
    if(aVal > 20*MV_PER_MA)
    {
        aVal = 20*MV_PER_MA;
    }
    else if(aVal < 4*MV_PER_MA)
    {
        aVal = 4*MV_PER_MA;
    }
    
    // User defined signal range can override defaults to enable operation across only a portion of the full range, so clamp to those instead
    aVal = clampValue(aVal, highmASetpoint, lowmASetpoint);
    
    // Increasing Slope
    if(gSetup.FlowRateHighmASetpoint >= gSetup.FlowRateLowmASetpoint)
    {
        flowRate = ((((fRateHigh - fRateLow) * (aVal-lowmASetpoint)) / (highmASetpoint-lowmASetpoint))) + fRateLow;   //slope is (HighmA flow - LowmA flow) / (highmAsetpoint-lowmASetpoint)  ----> slope*(analog Value converted to mA) + 4mA flow
        flowRate = clampValue(flowRate,fRateHigh,fRateLow);
    }
    // Decreasing Slope
    else
    {
        flowRate = fRateLow - ((((fRateLow - fRateHigh) * (aVal-lowmASetpoint)) / (highmASetpoint-lowmASetpoint)));   //slope is (LowmA flow - HighmA flow) / (highmAsetpoint-lowmASetpoint)  ----> slope*(analog Value converted to mA) + 4mA flow
        flowRate = clampValue(flowRate,fRateLow,fRateHigh);
    }
    
    return(flowRate);
}

void UpdateAnalogInput(void)
{
    if(gSetup.AnalogInControl == AIN_FLOW_RATE && gSetup.MeteringMode == FLOW_MODE)
    {
        VOL_resetVolumeThreshold();
    }
}

void UpdateAnalogOutput(void)
{
    uint32 vout = 0;
    uint32 desiredFlowRate = gSetup.DesiredFlowRate;
    uint32 maxOutputVoltage = gSetup.AoutMaxOut;
    uint32 minOutputVoltage = gSetup.AoutMinOut;
    const uint32 maxFlowRate = integerDivideRound(gSetup.KFactor * MINS_PER_DAY * gPumpRpm, VOLUME_SCALE_FACTOR / 10);
    uint32 vdac = 0;
    bool success = TRUE;
    
    if(gRun.TestOverride == FALSE)
    {
        if(gSetup.AnalogOutControl == AOUT_ON && gSetup.MeteringMode == FLOW_MODE)
        {
            if(maxFlowRate > 0)
            {
                if((gSetup.AnalogOutControl == AOUT_ON) && (gSetup.AnalogInControl == AIN_FLOW_RATE))
                {
                    desiredFlowRate = gRun.AnalogFlowRate;
                }
                vout = integerDivideRound(desiredFlowRate * maxOutputVoltage, maxFlowRate);
            }
            else
            {
                vout = 0;
            }
            vout = clampValue(vout, maxOutputVoltage, minOutputVoltage);
        }
        else
        {
            // The max voltage on the analog out screen will always be output on the analog out pins.  Default is 0V, that way the pump runs like normal
            // when analog output mode isn't being used or is turned off for (ie: for calibration).
            vout = maxOutputVoltage;
        }
        // Now that we know how much voltage to output, we need to figure out how much voltage to apply to the dac (2500mV dac output = 10V on analog output circuit)
        // ie: 10.0V = 100.  100/100*2500 = 10V out.
        // ie: 05.0V = 050.  050/100*2500 = 05V out.
        vdac = integerDivideRound(vout * MAX_DAC_OUTPUT_MV, PERCENT_SCALE_FACTOR);

        // Clamps DAC to 0-2500mV, which allows for up to 0-10V analog output
        vdac = clampValue(vdac, (uint32)MAX_DAC_OUTPUT_MV, (uint32)MIN_DAC_OUTPUT_MV);
        
        // We have a mix of DACs with different addresses in the field, AD5338ARMZ-1 & AD5338ARMZ parts, so we try both addresses & save the one that works
        // Production boards will always use D2A_MODULE_2_CHA
        success = D2A_Output_Set(d2aChannel, (D2A_FULLSCALE * vdac) / 2500);
        
        if (!success)
        {
            if(D2A_Output_Set(D2A_MODULE_0_CHA, (D2A_FULLSCALE * vdac) / 2500))
            {
                d2aChannel = D2A_MODULE_0_CHA;
                DEBUG_PRINT_STRING(DBUG_ALWAYS, "DAC channel updated\r\n");
            }
        }
    }
}

void VOL_updateRpm(uint32 newRpm)
{
    gPumpRpm = newRpm;
}

