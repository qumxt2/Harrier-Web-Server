// systemTask.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file implements the main system task which polls & updates system information

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************
#include "systemTask.h"
#include "rtos.h"
#include "eeprom.h"
#include "assert.h"
#include "datetime.h"
#include "dvar.h"
#include "dvseg_17G721_run.h"
#include "dvseg_17G721_setup.h"
#include "version.h"
#include "debug.h"
#include "ee_map.h"
#include "in_voltage.h"
#include "in_pressure.h"
#include "stdio.h"
#include "inputDebounceTask.h"
#include "application.h"
#include "alarms.h"
#include "PublishSubscribe.h"
#include "rtcTask.h"
#include "temperature.h"
#include "pumpControlTask.h"
#include "screensTask.h"
#include "modbus_handler.h"
#include "NetworkScreen.h"
#include "modemTask.h"
#include "serial_uart_u1a.h"
#include "AlarmScreenBattery.h"
#include "TankScreen.h"
#include "CountDigit.h"
#include "AlarmScreenControl.h"
#include "debug_app.h"
#include "AdvancedScreen.h"
#include "utilities.h"
#include "volumeTask.h"
#include <math.h>
#include "AlarmScreenTemp.h"
#include "AlarmScreenBattery.h"
#include "AlarmScreenMotorCurrent.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************
#define SYSTEM_TASK_TIMER_EVENT_FLAG		(RTOS_EVENT_FLAG_1)
#define START_IMMEDIATELY					(0x0001)
#define ALWAYS_WAIT_FOR_MATCH				(0x0000)
#define SYSTEM_TASK_FREQ_HZ                 (10)
#define BATT_ALARM_HYSTERESIS               (1000)                      // mV
#define POLLING_INTERVAL                    (10 * SYSTEM_TASK_FREQ_HZ)  // seconds
#define FIRST_UPDATE_PERIOD                 (10)                        // seconds
#define MOVING_AVERAGE_PERIODS              (8)                         // Must be power of 2
#define MOVING_AVERAGE_EMPTY                (0xFFFF)
#define REMOTE_DISABLE_INPUT                (DB_INPUT_2)
#define ANALOG_ALARM_DELAY                  (61 * SYSTEM_TASK_FREQ_HZ)  // seconds
#define REMOTE_UNINITIALIZED                (0xFF)
#define PRESSURE_ALARM_LOCKOUT_TIME         (60 / SYSTEM_TASK_FREQ_HZ)
#define EEPROM_UPDATE_PERIOD                (30)                        // seconds
#define DIGITAL_ALARM_DELAY                 (5 * SYSTEM_TASK_FREQ_HZ)   // seconds
#define TANK_ALARM_HYSTERESIS_GALS          (10)      // one gallon or 10%
#define TANK_ALARM_HYSTERESIS_INCHES        (100)     // one inch
#define TANK_LEVEL_DELAY                    (1 * SYSTEM_TASK_FREQ_HZ)   // 1 second
#define TEN_UV_VOLTS_PER_COUNT              (61)                        // 2.5/4096
#define ADC_TEMP_SERIES_RESISTOR            (10000UL)                   // in ohms
#define ADC_TEMP_INITIAL_VOLTAGE            (330000UL)                  // this is in 10uV, 3.3V
#define TEMP_HYSTERESIS                     (2)                         // degrees C
#define MIN_THERMISTOR_COUNTS               (10)
#define CURRENT_CHECK_INTERVAL              (1 * SYSTEM_TASK_FREQ_HZ) // 1 second

//16 will work out to be 1.6 second window of values to look at
#define NUM_OF_CURRENT_SAMPLES 16       //must be multiple of 2
#define NUM_BITS_FOR_CURRENT_SAMPLES 4  //2^N where N is NUM_OF_CURRENT_SAMPLES
#define CURRENT_FAULTS_ALLOWED 3    //allow 3 seconds of high current before tripping an alarm
#define HIGH_POSITIVE_DC_CURRENT_THRESHOLD_MV  700      //700mV => 27A, work with mV since it will speed up the process .5V is 30A 2.5V is 0A and 4.5V is -30A
#define HIGH_NEGATIVE_DC_CURRENT_THRESHOLD_MV  4300     //4300mV => -27A, work with mV since it will speed up the process .5V is 30A 2.5V is 0A and 4.5V is -30A
#define HIGH_POSITIVE_AC_CURRENT_THRESHOLD_MV  2000     //2000mV => 7.5A peak or 5.3A rms, work with mV since it will speed up the process .5V is 30A 2.5V is 0A and 4.5V is -30A
#define HIGH_NEGATIVE_AC_CURRENT_THRESHOLD_MV  3000     //3000mV => -7.5A peak or 5.3A rms, work with mV since it will speed up the process .5V is 30A 2.5V is 0A and 4.5V is -30A
#define HIGH_POSITIVE_NO_CURRENT_LIMIT  2430     //70mV window around 2500mV, 1A window to determine that current is not flowing
#define HIGH_NEGATIVE_NO_CURRENT_LIMIT  2570     //70mV window around 2500mV, 1A window to determine that current is not flowing
#define NO_CURRENT_MV 2500    //2500mV is 0 current

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************
uint8 gDvarLockoutResID = RTOS_INVALID_ID;

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************
static uint8 gSystemTaskTimerID;
static uint8 gSystemTaskID;
static uint8 gPeriodicUpdateTimerID = RTC_TIMER_INVALID;
static uint32 gPollingCounter = 0;
static uint16 gBatteryHistory[MOVING_AVERAGE_PERIODS];
static uint16 gPressureHistory[MOVING_AVERAGE_PERIODS];
static uint16 gThermistorHistory[MOVING_AVERAGE_PERIODS];
static uint8 gBatteryHistoryIndex = 0;                      // Where the next battery history item will go
static uint8 gPressureHistoryIndex = 0;                     // Where the next pressure history item will go
static uint8 gThermistorVoltageHistoryIndex = 0;            // Where the next thermistor history item will go
static uint8 gEEpromUpdateTimerID = RTC_TIMER_INVALID;
static uint8 gDigitalAlarmDelayCounter = 0;
static uint8 gTankLevelDelayCounter = 0;
static uint8 gModbusCommsTimerID = RTC_TIMER_INVALID;

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************
static sint8 DvarStart ( void );
static void RemoteDisableInput(void);
static void GenericDigitalInputs(db_input_t inputId);
static void PollSystemStatus(void);
static void CheckAlarms(void);
static void CheckDigitalAlarms(void);
static void CheckLinePressureAlarm(void);
static void CheckTankPressureAlarm(void);
static void CheckBatteryAlarm(void);
static void PeriodicUpdateCallback(uint8 id);
static void CheckModbusCommsCallback(uint8 id);
static void LinePressureAlarmCallback(alarm_id_t alarmId, bool alarmIsActive);
static void TankLevelAlarmCallback(alarm_id_t alarmId, bool alarmIsActive);
static void BatteryAlarmCallback(alarm_id_t alarmId, bool alarmIsActive);
static uint16 UpdateMovingAverage(uint16 newValue, uint8* pValueIndex, uint16* valueHistoryArray);
static void CheckPowerSaveMode(void);
static void PeriodicEEpromUpdateCallback(uint8 id);
static void GetTankPressure(void);
static void UpdateCurrentReading(void);
static void CheckMotorCurrent(void);
static void GetmVReadingfor4to20mA(void);
static void CheckTankGallonThresholds(void);
static void CheckTankHeightThresholds(void);
static void CheckAnalogInputAlarm(void);
static void TempProbeDisableInput(void);

// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************

uint8 SystemInit (void)
{
    uint8 status = SYSTEM_TASK_INIT_OK;
    uint8 i;

    // Init the moving average stores
    for (i = 0; i < MOVING_AVERAGE_PERIODS; i++)
    {
        gBatteryHistory[i] = MOVING_AVERAGE_EMPTY;
        gPressureHistory[i] = MOVING_AVERAGE_EMPTY;
        gThermistorHistory[i] = MOVING_AVERAGE_EMPTY;
    }

    // Reserve a semaphore to indicate that DVARs are ready for reading and writing
    gDvarLockoutResID = RtosResourceReserveID();
    if( gDvarLockoutResID == RTOS_INVALID_ID)
    {
        status |= SYSTEM_TASK_INIT_ERROR;
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "SYSTEM TASK INIT ERROR -- DVAR RESOURCE\r\n");
    }

    // Reserve a task timer
	gSystemTaskTimerID = RtosTimerReserveID();
	if (gSystemTaskTimerID == RTOS_INVALID_ID)
	{
        status |= SYSTEM_TASK_INIT_ERROR;
		DEBUG_PRINT_STRING(DBUG_ALWAYS, "SYSTEM TASK INIT ERROR -- TASK TIMER\r\n");
	}

	// Reserve a task ID and queue the task
	gSystemTaskID = RtosTaskCreateStart(SystemTask);
	if (gSystemTaskID == RTOS_INVALID_ID)
	{
        status |= SYSTEM_TASK_INIT_ERROR;
		DEBUG_PRINT_STRING(DBUG_ALWAYS, "SYSTEM TASK INIT ERROR -- TASK ID\r\n");
	}

	return status;
}

// *****************************************************************************
// * PRIVATE FUNCTIONS
// *****************************************************************************

void SystemTask ( void )
{
    uint8 status;
    uint8 error = FALSE;

    // Need to stall other tasks that are using DVARs early in their startup so that this task has time
    // to set the DVAR values to defaults if the existing values are out of range, the EEPROM map has
    // changed, or the EEPROM is blank.
    status = K_Resource_Get(gDvarLockoutResID);
    if (status != K_OK)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "System task could not lock out DVARs!\n");
    }

    // Create and start the timer
	status = K_Timer_Create(gSystemTaskTimerID, RTOS_NOTIFY_SPECIFIC, gSystemTaskID, SYSTEM_TASK_TIMER_EVENT_FLAG);
	status |= K_Timer_Start(gSystemTaskTimerID, START_IMMEDIATELY, (RtosGetTickFreq()/SYSTEM_TASK_FREQ_HZ));

	if (status != K_OK)
	{
		DEBUG_PRINT_STRING(DBUG_ALWAYS, "SYSTEM TASK TIMER ERROR\r\n");
	}
    
    error = ( DvarStart( ) < 0 );

    assert( !error );

    // DVARs have been initialized, so release everything waiting on them
    (void)K_Resource_Release(gDvarLockoutResID);

    // Startup the network
    SYSTEM_enableModbusOrCellular();
            
    // Pressure monitoring setup
    Init_Pressure_Transducer(gSetup.TankType);

    // Callback setups
    (void)ALARM_RegisterCallback(ALARM_ID_Battery_Shutoff, BatteryAlarmCallback);
    (void)ALARM_RegisterCallback(ALARM_ID_High_Pressure, LinePressureAlarmCallback);
    (void)ALARM_RegisterCallback(ALARM_ID_Low_Pressure, LinePressureAlarmCallback);
    (void)ALARM_RegisterCallback(ALARM_ID_Low_Tank_Shutoff, TankLevelAlarmCallback);
    (void)ALARM_RegisterCallback(ALARM_ID_Low_Tank_Notify, TankLevelAlarmCallback);
    (void)ALARM_RegisterCallback(ALARM_ID_Flow_Accuracy, TankLevelAlarmCallback);
    
    // Initialize to something other than true or false so that we always treat the initial
    // reading as an edge
    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, RemoteDisableActive), REMOTE_UNINITIALIZED);

    // Periodically publish information about the pump. Make the first one occur soon after startup, but long
    // enough for the inputs to stabilize and readings to have been taken. Subsequent ones can be spaced further
    // apart.
    gPeriodicUpdateTimerID = RTC_createTimer(FIRST_UPDATE_PERIOD, PeriodicUpdateCallback);
    (void)RTC_startTimer(gPeriodicUpdateTimerID);
    
    gModbusCommsTimerID = RTC_createTimer(FIRST_UPDATE_PERIOD, CheckModbusCommsCallback);
    if ((gSetup.NetworkMode == NETWORK_MODE_MODBUS) && (gSetup.ModbusCommsEnable == TRUE))
    {
        (void)RTC_startTimer(gModbusCommsTimerID);
    }
    
    // Loop forever
    for (;;)
	{
		status = K_Event_Wait(SYSTEM_TASK_TIMER_EVENT_FLAG, ALWAYS_WAIT_FOR_MATCH, RTOS_CLEAR_EVENT_FLAGS_AFTER);
        
        // Poll digital I/O at the task frequency
        // Need enough start-up delay before checking alarms to allow pump to leave reset mode at start-up when pump is auto set to run
        if(gDigitalAlarmDelayCounter >= DIGITAL_ALARM_DELAY)
        {
            CheckDigitalAlarms();
            RemoteDisableInput();
        }
        else
        {
            gDigitalAlarmDelayCounter++;
        }
        
        // Read and average tank monitor transducer every 0.1 seconds.  Delay 1 second to ignore spikes on transducer at power up.
        if (gTankLevelDelayCounter >= TANK_LEVEL_DELAY)
        {
            GetTankPressure();
            UpdateCurrentReading(); //watch the current sense chip
        }
        else
        {
            gTankLevelDelayCounter++;
        }
          
        //When using the Analog Input get a reading and update the average every .1 seconds
        if (gSetup.AnalogInControl != AIN_OFF)
        {
            GetmVReadingfor4to20mA();
        }
        
        // Process the current reading to make sure controller is okay
        if ((gPollingCounter % CURRENT_CHECK_INTERVAL) == 0)
        {
            // Motor Current Check every 1 second
            CheckMotorCurrent();
        }
        
        // Poll analog I/O and update tank level at the slower, POLLING_INTERVAL
        if (gPollingCounter++ % POLLING_INTERVAL == 0)
        {
            // Update tank level every 10 seconds
            UpdateTankLevel();
            UpdateAnalogInput();
            UpdateAnalogOutput();
            PollSystemStatus();
        }
    }
}

static sint8 DvarStart ( void )
{
    sint8 error = 0;
    sint8 overall_error = FALSE;
    uint16 i;
    bool mResetDefaults = FALSE;

    DEBUG_PRINT_STRING( DBUG_ALWAYS, "\n* DVAR start... " );

    mResetDefaults = DVSEG_17G721_SETUP_MagicNumberMismatch( );
    if( mResetDefaults )
    {
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "EEPROM blank or corrupt!\n" );
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "* >Clearing EEPROM... " );

        for( i = EE_BASE; i < EE_LAST_USED; )
        {
            if ( EEPROM_WriteByte( i, 0x00 ) == EEPROMError_Ok )
            {
                i++;
            }
            else
            {
                (void)K_Task_Wait( 2 );
            }
        }

        DEBUG_PRINT_STRING( DBUG_ALWAYS, "\r\x1b[74C[ \x1b[32mOK\x1b[0m ]\n" );
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "* Initializing DVARs... " );
    }
    
    // Initialize DVARs
    error = DVAR_Init();
    if (error < 0)
    {
        overall_error |= TRUE;
    }

    error = DVSEG_17G721_RUN_Initialize( mResetDefaults );
    if( error < 0 )
    {
        overall_error |= TRUE;
    }

    error = DVSEG_17G721_SETUP_Initialize( mResetDefaults );
    if( error < 0 )
    {
        overall_error |= TRUE;
    }

    error = DVSEG_17G721_LevelChart_Initialize( mResetDefaults );
    if( error < 0 )
    {
        overall_error |= TRUE;
    }
    
    error = DVSEG_17G721_USAGE_Initialize( mResetDefaults);
    if( error < 0 )
    {
        overall_error |= TRUE;
    }

    if( overall_error )
    {
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "\r\x1b[74C[\x1b[31mFAIL\x1b[0m]\n" );
    }
    else
    {
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "\r\x1b[74C[ \x1b[32mOK\x1b[0m ]\n" );
    }

    return overall_error;
}

// Poll system status, and activate alarms as necessary
static void PollSystemStatus(void)
{
    IOrtn_mV_to_psi_s16d16_t rtnval;
    IOrtn_uint16_t batteryAdcResults = {0};
    IOrtn_uint16_t motorCurrentResults = {0};
    IOrtn_uint16_t thermistorAdcResults = {0};
    uint16 batteryMv = 0;
    uint16 motorCurrentMv = 0;
    uint16 Pressure_1_Psi = 0;
    uint16 batteryMvAveraged = 0;
    uint16 Pressure_1_PsiAveraged = 0;
    uint16 voltageAdcAveraged = 0;       
   
    uint32 systemTemp = 0;
    static uint32 voltageADC = 0;
    double temp = 0;
    
    static uint8 avg_cnt = 0;
    static uint32 avg_accum_thermistor_volts = 0;
    uint32 thermistor_volts_averaged = 0;

    // Check the battery
    batteryAdcResults = IN_A2D_Get(BATTERY_ADC_CHANNEL);
    batteryMv = (batteryAdcResults.u16 * BATTERY_ADC_MULTIPLIER / BATTERY_ADC_DIVISOR) + BATTERY_OFFSET_MV;

    // Round small voltages down to zero to clean up the noise when the battery line is floating
    if (batteryMv < MV_PER_VOLT)
    {
        batteryMv = 0;
    }
    batteryMvAveraged = UpdateMovingAverage(batteryMv, &gBatteryHistoryIndex, gBatteryHistory);
    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, BatteryMillivolts), batteryMvAveraged);

    // Check the line pressure
    rtnval = GetPressurePSI(IOHARDWARE_PRESSURESENSOR_A);
    Pressure_1_Psi = (uint16)rtnval.psi_s16d16;
    Pressure_1_PsiAveraged = UpdateMovingAverage(Pressure_1_Psi, &gPressureHistoryIndex, gPressureHistory);
    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, Pressure_1_Psi), Pressure_1_PsiAveraged);
    
    // Check the thermistor
    // Note: Mfg & Creation test benches assume that the controller is always reading temp probe in the background
    thermistorAdcResults = IN_A2D_Get(THERMISTOR_ADC_CHANNEL);
    if ( thermistorAdcResults.u16 > MIN_THERMISTOR_COUNTS)
    {
        voltageADC = thermistorAdcResults.u16 * TEN_UV_VOLTS_PER_COUNT;
        if (avg_cnt < 8)
        {
            avg_cnt++;
            avg_accum_thermistor_volts += voltageADC;
            thermistor_volts_averaged = avg_accum_thermistor_volts / avg_cnt;
        }
        else
        {
            avg_accum_thermistor_volts = (avg_accum_thermistor_volts - (avg_accum_thermistor_volts / avg_cnt)) + voltageADC;
            thermistor_volts_averaged = avg_accum_thermistor_volts / avg_cnt;
        }        

        voltageAdcAveraged = UpdateMovingAverage(voltageADC, &gThermistorVoltageHistoryIndex, gThermistorHistory);
    }
    else
    {
        // Force the voltage to be zero for anything less than 10 counts & consider it disconnected.
        // We only measure down to -40 deg C, which = 202 counts.  1 - 10 counts = -147 deg C to -101 deg C, a range we'll never be in.
        voltageADC = 0;
    }
        
    if (voltageADC > 0)
    {
        systemTemp = (ADC_TEMP_INITIAL_VOLTAGE - voltageADC) * ADC_TEMP_SERIES_RESISTOR;
        systemTemp = systemTemp / voltageADC;

        //system temp is now a resistance plug this into the resistance temp curve to get temperature in °C
        temp = systemTemp;
        temp = log(temp);
        temp = temp * ((double)(-20.15));
        temp = temp + (double)211;

        // convert C to F
        temp = temp*9/5 + 32;
        // Round for positive & negative #'s
        temp += (0.5 - (temp < 0.0));
        temp = (sint16)temp;
        gRun.Temperature = (uint32)temp;
    }
    else
    {
        systemTemp = 0;
        gRun.Temperature = TEMPERATURE_INVALID_DEG_F;
    }
    
    CheckAlarms();
}

// Update the given moving average with the given new value.
// Returns:  uint16 - the current moving average for the history points and the new value
// Not a true moving average, since it will start returning an averaged value if
// at least one data point exists.
static uint16 UpdateMovingAverage(uint16 newValue, uint8* pValueIndex, uint16* valueHistoryArray)
{
    uint32 runningSum = 0;
    uint8 foundCount = 0;
    uint16 movingAverage = 0;
    uint8 i = 0;

    valueHistoryArray[*pValueIndex] = newValue;

    // Increment and wrap index
    *pValueIndex += 1;
    *pValueIndex &= (MOVING_AVERAGE_PERIODS - 1);

    for (i = 0; i < MOVING_AVERAGE_PERIODS; i++)
    {
        if (valueHistoryArray[i] != MOVING_AVERAGE_EMPTY)
        {
            runningSum += valueHistoryArray[i];
            foundCount++;
        }
    }

    if (foundCount)
    {
        movingAverage = runningSum / foundCount;
    }

    return movingAverage;
}

static void CheckAlarms(void)
{
    // Latch to deal with the short polling counter overflow window
    static bool startupDelaySatisfied = FALSE;

    if (gPollingCounter >= ANALOG_ALARM_DELAY)
    {
        startupDelaySatisfied = TRUE;
    }

    // Allow most of the alarm inputs to stabilize before they can cause alarms
    if ( startupDelaySatisfied )
    {
        CheckLinePressureAlarm();
        CheckTankPressureAlarm();
        CheckBatteryAlarm();
        CheckPowerSaveMode();
        CheckAnalogInputAlarm();
        TempProbeDisableInput();
    }
}

static void CheckDigitalAlarms(void)
{
    GenericDigitalInputs(DB_INPUT_3);
    GenericDigitalInputs(DB_INPUT_4);
}

// When one of the inputs associated with the generic digital alarms changes,
// check if that new state indicates an error condition
static void GenericDigitalInputs(db_input_t inputId)
{
    bool activeState;
    alarm_id_t alarmId = ALARM_ID_Unknown;
    LOGIC_LEVELS_t triggerType = LOGIC_DISABLED;
    IOrtn_digital_t state = DB_DigitalStateGet(inputId);

    // Associate the input to the alarm
    switch (inputId)
    {
        case DB_INPUT_3:
            alarmId = ALARM_ID_Input_1;
            triggerType = gSetup.Alarm1Trigger;
            break;

        case DB_INPUT_4:
            alarmId = ALARM_ID_Input_2;
            triggerType = gSetup.Alarm2Trigger;
            break;

        default:
            break;
    }

    if (alarmId != ALARM_ID_Unknown && triggerType != LOGIC_DISABLED)
    {
        activeState = (triggerType == LOGIC_ACTIVE_HIGH);
        if (state.state == activeState)
        {
            (void)ALARM_ActivateAlarm(alarmId);
        }
    }
}

// Allow pump running to be effectively locked out by grounding digital input 2
static void RemoteDisableInput(void)
{
   if (gSetup.RemoteOffTrigger != LOGIC_DISABLED)
   {
        const bool activeState = (gSetup.RemoteOffTrigger == LOGIC_ACTIVE_HIGH);
        const db_input_t inputId = REMOTE_DISABLE_INPUT;
        IOrtn_digital_t state = DB_DigitalStateGet(inputId);

        if (state.state == activeState)
        {
            // Trigger only on edge
            if (TRUE != gRun.RemoteDisableActive)
            {
                (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, RemoteDisableActive), TRUE);
                if (getCurrentScreen() == INPUT_MODE_RUN)
                {
                    PMP_setStandbyMode();
                }
            }
        }
        else
        {
            // Trigger only on edge
            if (FALSE != gRun.RemoteDisableActive)
            {
                (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, RemoteDisableActive), FALSE);
                if (getCurrentScreen() == INPUT_MODE_RUN)
                {
                    PMP_setRunMode();
                }
            }
        }
   }
}

// Allow pump running to be effectively locked out by a high or low temp probe reading
static void TempProbeDisableInput(void)
{
    sint32 tempSetpoint = (sint32)gSetup.TempSetpoint;
    sint32 temperature = (sint32)gRun.Temperature;

    if (gSetup.TempControl == TEMP_CONTROL_OFF_ABOVE)   // Turn pump off above setpoint, on below
    {
        if (temperature >= tempSetpoint)
        {
            if (TRUE != gRun.TempProbeDisableActive)
            {
                (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, TempProbeDisableActive), TRUE);
                if (getCurrentScreen() == INPUT_MODE_RUN)
                {
                    PMP_setStandbyMode();
                }
                PublishInt32(TOPIC_Temperature, (int32_t)gRun.Temperature);
            }
        }
        else if (temperature < (tempSetpoint) - (sint32)TEMP_HYSTERESIS )
        {
            if (TRUE == gRun.TempProbeDisableActive)
            {
                (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, TempProbeDisableActive), FALSE);
                if (getCurrentScreen() == INPUT_MODE_RUN)
                {
                    PMP_setRunMode();
                }
                PublishInt32(TOPIC_Temperature, (int32_t)gRun.Temperature);
            }
        }
    }
    else if (gSetup.TempControl == TEMP_CONTROL_OFF_BELOW)  // Turn pump off below setpoint, on above
    {
        if (temperature <= tempSetpoint)
        {
            if (TRUE != gRun.TempProbeDisableActive)
            {
                (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, TempProbeDisableActive), TRUE);
                if (getCurrentScreen() == INPUT_MODE_RUN)
                {
                    PMP_setStandbyMode();
                }
                PublishInt32(TOPIC_Temperature, (int32_t)gRun.Temperature);
            }
        }
        else if (temperature > (tempSetpoint) + (sint32)TEMP_HYSTERESIS)
        {
            if (TRUE == gRun.TempProbeDisableActive)
            {
                (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, TempProbeDisableActive), FALSE);
                if (getCurrentScreen() == INPUT_MODE_RUN)
                {
                    PMP_setRunMode();
                }
                PublishInt32(TOPIC_Temperature, (int32_t)gRun.Temperature);
            }
        }
    }
}

static void CheckLinePressureAlarm(void)
{
    static uint32 alarmLockoutTimer = 0;

    if (PMP_isRunning())
    {
        // Only evaluate pressure alarms after running for PRESSURE_ALARM_LOCKOUT_TIME, most accurate when pump is running
        if (alarmLockoutTimer < PRESSURE_ALARM_LOCKOUT_TIME)
        {
            alarmLockoutTimer++;
        }
        else
        {
            if(gRun.Pressure_1_Psi > gSetup.HighPressureTrigger)
            {
                (void)ALARM_ActivateAlarm(ALARM_ID_High_Pressure);
                alarmLockoutTimer = 0;
            }

            if(gRun.Pressure_1_Psi < gSetup.LowPressureTrigger)
            {
                (void)ALARM_ActivateAlarm(ALARM_ID_Low_Pressure);
                alarmLockoutTimer = 0;
            }
        }
    }
    else
    {
        // Reset timer if pump is not running
        alarmLockoutTimer = 0;
    }
}

static void CheckAnalogInputAlarm(void)
{
    uint32 analogInVal = gRun.AnalogInmV;
    
    if(gSetup.AnalogInControl == AIN_FLOW_RATE && gSetup.MeteringMode == METERING_MODE_VOLUME)
    {
        if(analogInVal > 20*MV_PER_MA)
        {
            //trigger alarm if too far out of range
            if(analogInVal > 21*MV_PER_MA)
            {
                (void)ALARM_ActivateAlarm(ALARM_ID_ANALOG_OUT_OF_RANGE);
            }
        }
        else if(analogInVal < 4*MV_PER_MA)
        {
            //trigger alarm if too far out of range
            if(analogInVal < 3*MV_PER_MA)
            {
                (void)ALARM_ActivateAlarm(ALARM_ID_ANALOG_OUT_OF_RANGE);
            }
        }
        else
        {
            if (ALARM_AlarmState(ALARM_ID_ANALOG_OUT_OF_RANGE))
            {
                (void)ALARM_CancelAlarm(ALARM_ID_ANALOG_OUT_OF_RANGE);
            }
        }
    }    
}

static void CheckTankPressureAlarm(void)
{
    // Alarms are evaluated based on height in inches if the height thresholds are set > 0 via modbus.
    // Set both height thresholds to 0 to disable and use gallons.  Default is gallons.
    // The gRun parameters will be 0 on reset or power cycle until a modbus message from the master changes them.
    
    if ((gRun.TankHeightShutoffTrigger > 0) && (gRun.TankHeightNotifyTrigger > 0))
    {
        CheckTankHeightThresholds();
    }
    else
    {
        CheckTankGallonThresholds();
    }
}

static void CheckTankGallonThresholds(void)
{
    uint32 tankLevel = gRun.TankLevel;
    
    // Check for tank shutoff (Manually Cleared)
    if (tankLevel < (gSetup.TankLevelShutoffTrigger) && (gSetup.TankLevelShutoffTrigger > 0))
    {
        // Clear the notify alarm when shutoff is active & set the shutoff alarm next time through
        if (ALARM_AlarmState(ALARM_ID_Low_Tank_Notify))
        {
            (void)ALARM_CancelAlarm(ALARM_ID_Low_Tank_Notify);
        }
        else if (!ALARM_AlarmState(ALARM_ID_Low_Tank_Shutoff))
        {
            (void)ALARM_ActivateAlarm(ALARM_ID_Low_Tank_Shutoff);
        }
    }
    // Check for tank notify if shutoff hasn't been activated & notify alarm is enabled (trigger > 0)
    else if (!ALARM_AlarmState(ALARM_ID_Low_Tank_Shutoff) && (gSetup.TankLevelNotifyTrigger > 0))
    {	
        // Check for tank notify (Auto Cleared)
        if ((tankLevel <= (gSetup.TankLevelNotifyTrigger)) && (tankLevel >= (gSetup.TankLevelShutoffTrigger)))
        {
            if (!ALARM_AlarmState(ALARM_ID_Low_Tank_Notify))
            {
                (void)ALARM_ActivateAlarm(ALARM_ID_Low_Tank_Notify);
            }
        }
        else if (tankLevel > (gSetup.TankLevelNotifyTrigger) + TANK_ALARM_HYSTERESIS_GALS )
        {
            if (ALARM_AlarmState(ALARM_ID_Low_Tank_Notify))
            {
                (void)ALARM_CancelAlarm(ALARM_ID_Low_Tank_Notify);
            }
        }
    }
}

static void CheckTankHeightThresholds(void)
{
    uint32 tankHeight = gRun.TankHeightCalc;
    
    // Check for tank shutoff (Manually Cleared)
    if (tankHeight < (gRun.TankHeightShutoffTrigger) && (gRun.TankHeightShutoffTrigger > 0))
    {
        // Clear the notify alarm when shutoff is active & set the shutoff alarm next time through
        if (ALARM_AlarmState(ALARM_ID_Low_Tank_Notify))
        {
            (void)ALARM_CancelAlarm(ALARM_ID_Low_Tank_Notify);
        }
        else if (!ALARM_AlarmState(ALARM_ID_Low_Tank_Shutoff))
        {
            (void)ALARM_ActivateAlarm(ALARM_ID_Low_Tank_Shutoff);
        }
    }
    // Check for tank notify if shutoff hasn't been activated & notify alarm is enabled (trigger > 0)
    else if (!ALARM_AlarmState(ALARM_ID_Low_Tank_Shutoff) && (gRun.TankHeightNotifyTrigger > 0))
    {	
        // Check for tank notify (Auto Cleared)
        if ((tankHeight <= (gRun.TankHeightNotifyTrigger)) && (tankHeight >= (gRun.TankHeightShutoffTrigger)))
        {
            if (!ALARM_AlarmState(ALARM_ID_Low_Tank_Notify))
            {
                (void)ALARM_ActivateAlarm(ALARM_ID_Low_Tank_Notify);
            }
        }
        else if (tankHeight > (gRun.TankHeightNotifyTrigger) + TANK_ALARM_HYSTERESIS_INCHES )
        {
            if (ALARM_AlarmState(ALARM_ID_Low_Tank_Notify))
            {
                (void)ALARM_CancelAlarm(ALARM_ID_Low_Tank_Notify);
            }
        }
    }
}

static void CheckBatteryAlarm(void)
{
    if (POWER_SAVE_OFF != gSetup.PowerSaveMode)
    {
        // Check for low battery
        if (gRun.BatteryMillivolts < (gSetup.BatteryShutoffTrigger))
        {
            // Ensure that the voltage is updated immediately so that we don't show a
            // stale, good, voltage when the alarm goes off
            if (!ALARM_AlarmState(ALARM_ID_Battery_Shutoff))
            {
                (void)ALARM_ActivateAlarm(ALARM_ID_Battery_Shutoff);
            }
        }
        // Unlike most alarms, we want to clear this one automatically
        else if (gRun.BatteryMillivolts > (gSetup.BatteryShutoffTrigger) + BATT_ALARM_HYSTERESIS )
        {
            if (ALARM_AlarmState(ALARM_ID_Battery_Shutoff))
            {
                (void)ALARM_CancelAlarm(ALARM_ID_Battery_Shutoff);
            }
        }
        else
        {
            // No action in this branch. This provides the hysteresis. (Present for clarity.)
        }
    }
}

static void CheckPowerSaveMode(void)
{
    if (POWER_SAVE_OFF != gSetup.PowerSaveMode)
    {
        // Check for entering power save mode
        if (gRun.BatteryMillivolts < (gSetup.BatteryWarningTrigger))
        {
            if (gRun.PumpStatus == PUMP_STATUS_Run)
            {
                (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gRun, PumpStatus), PUMP_STATUS_Powersave);
                PMP_updatePowerSaveMode();
            }
        }
        // Clear power save mode
        else if (gRun.BatteryMillivolts > (gSetup.BatteryWarningTrigger  + BATT_ALARM_HYSTERESIS) )
        {
            if (gRun.PumpStatus == PUMP_STATUS_Powersave)
            {
                (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gRun, PumpStatus), PUMP_STATUS_Run);
                PMP_updatePowerSaveMode();
            }
        }
        else
        {
            // No action in this branch. This provides the hysteresis. (Present for clarity.)
        }
    }
}

// Periodically publish telemetry about the pump
static void PeriodicUpdateCallback(uint8 id)
{
    // Battery
    if (POWER_SAVE_OFF != gSetup.PowerSaveMode)
    {
         PublishUint32(TOPIC_BatteryVoltage, gRun.BatteryMillivolts);
    }
    
    // Pressure, Tank Level, and Temperature
    PublishUint32(TOPIC_PressureLevel, gRun.Pressure_1_Psi);
    PublishUint32(TOPIC_TankLevel, gRun.TankLevel);
    PublishInt32(TOPIC_Temperature, (int32_t)gRun.Temperature);
    if(gSetup.AnalogInControl != AIN_OFF)
    {
        PublishUint32(TOPIC_FlowRate, gRun.AnalogFlowRate);
        //Passing the raw mV reading, there is a 100ohm resistor that the current runs trough to generate this voltage. 
        //So by sending the mV reading we are sending the current in a XX.XX form
        PublishUint32(TOPIC_RawAnalogIn, gRun.AnalogInmV);
        
    }
    
    (void)RTC_resetTimer(id, gSetup.SystemPublicationPeriod);
}

static void CheckModbusCommsCallback(uint8 id)
{
    static uint32 lastTickCount = 0;
    uint32 newTickCount = gRun.ModbusHeartbeatTick;
    
    // Only checked if enabled
    if (gSetup.ModbusCommsEnable != TRUE)
    {
        return;
    }
    
    // Activate an alarm if the tick count hasn't changed
    if (newTickCount == lastTickCount)
    {
        if (!ALARM_AlarmState(ALARM_ID_Modbus_Comms))
        {
            (void)ALARM_ActivateAlarm(ALARM_ID_Modbus_Comms);
        }
    }
    else
    {
        if (ALARM_AlarmState(ALARM_ID_Modbus_Comms))
        {
            (void)ALARM_CancelAlarm(ALARM_ID_Modbus_Comms);
        }
    }
 
    lastTickCount = newTickCount;
    (void)RTC_resetTimer(id, 10);
}

// Callback for battery alarm state changes
static void BatteryAlarmCallback(alarm_id_t alarmId, bool alarmIsActive)
{
    if (POWER_SAVE_OFF != gSetup.PowerSaveMode)
    {
        // Ensure that the voltage is updated immediately so that we don't show a
        // stale voltage when the alarm is activated or cleared
        PublishUint32(TOPIC_BatteryVoltage, gRun.BatteryMillivolts);
    }
}

// Callback for line pressure alarm state changes
static void LinePressureAlarmCallback(alarm_id_t alarmId, bool alarmIsActive)
{
    // Ensure the pressure shown on the web portal is immediately consistent with the alarm state
    PublishUint32(TOPIC_PressureLevel, gRun.Pressure_1_Psi);
}

// Callback for tank level alarm state changes
static void TankLevelAlarmCallback(alarm_id_t alarmId, bool alarmIsActive)
{
    // Ensure the tank level shown on the web portal is immediately consistent with the alarm state
    PublishUint32(TOPIC_TankLevel, gRun.TankLevel);
}

// Force the publication of the telemetry now, which implies forcing the timer to pick up the
// new period value, if any
void SYSTEM_ForcePublicationNow(void)
{
    PeriodicUpdateCallback(gPeriodicUpdateTimerID);
}

// Choose whether to initialize modbus or cellular, depending on the
// system state
void SYSTEM_enableModbusOrCellular(void)
{
    if (gSetup.NetworkMode == NETWORK_MODE_CELLULAR)
    {
        Serial_U1A_Reset();
        MODEM_configureUart();
    }
    if (gSetup.NetworkMode == NETWORK_MODE_MODBUS)
    {
        Serial_U1A_Reset();
        (void)Modbus_Init((uint8)gSetup.ModbusSlaveID, (uint8)gSetup.ModbusParity, (uint8)gSetup.ModbusStopBits);
    }
}

// Periodically update EEprom backups stored in gUsage
// Must wait until all parameters have been restored with backups before creating & starting timer
// or it could overwrite saved parameters.
void SYSTEM_enableBackupTimer(void)
{
    gEEpromUpdateTimerID = RTC_createTimer(EEPROM_UPDATE_PERIOD, PeriodicEEpromUpdateCallback);
    (void)RTC_startTimer(gEEpromUpdateTimerID);
}

static void PeriodicEEpromUpdateCallback(uint8 id)
{
    DVSEG_17G721_USAGE_SaveValues();
    (void)RTC_resetTimer(id, EEPROM_UPDATE_PERIOD);
}

void GetTankPressure(void)
{
    static uint32 avg_cnt = 0;
    static uint32 avg_accum = 0;    
    static uint32 Pressure_2_Psi = 0;
    uint32 Pressure_2_PsiAveraged = 0;
    IOrtn_mV_to_psi_s16d16_t rtnval;    
    
    // Only read tank level sensor when pump isn't running, or is running an off cycle, or if analog output is enabled (pump running continuously)
    if ((!PMP_isRunning()) || (PMP_isOffCycleRunning()) || gSetup.AnalogOutControl == AOUT_ON)
    {
        rtnval = GetPressurePSI(IOHARDWARE_PRESSURESENSOR_B);
        Pressure_2_Psi = (uint32)rtnval.psi_s16d16;

        if (avg_cnt < 200)
        {
            avg_cnt++;
            avg_accum += Pressure_2_Psi;
            Pressure_2_PsiAveraged = avg_accum / avg_cnt;
        }
        else
        {
            avg_accum = (avg_accum - (avg_accum / avg_cnt)) + Pressure_2_Psi;
            Pressure_2_PsiAveraged = avg_accum / avg_cnt;
        }

        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, Pressure_2_Psi), Pressure_2_PsiAveraged);    
    }
}

void UpdateCurrentReading(void)
{   
    static uint32 readings[NUM_OF_CURRENT_SAMPLES] = {[0 ... (NUM_OF_CURRENT_SAMPLES-1)] = 2500};   //gcc to initialize all elements of an array to a number other than 0
    static uint8 counter = 0;
    uint32 average = 0;
    uint32 max = 0;
    uint32 min = 0;
    uint8 i = 0;
    IOrtn_uint16_t rtnval;
    uint32 mtrCurrentmV;
    
    rtnval = IN_A2D_Get(ADC_CHANNEL_CH3);
    
    mtrCurrentmV = rtnval.u16;
    
    mtrCurrentmV = (mtrCurrentmV * MOTOR_ADC_MULTIPLIER) >> MOTOR_ADC_DIVISOR_BIT_SHIFT;
    
    readings[counter] = mtrCurrentmV;
    
    ++counter;
    counter = counter & (NUM_OF_CURRENT_SAMPLES-1);
    
    max = readings[0];
    min = readings[0];
    average = readings[0];
    i = 1;
    for(; i<NUM_OF_CURRENT_SAMPLES; ++i)    //assuming that a value close to both peaks will be seen so only look for the lowest value or highest positive current
    {
        average += readings[i];
        if(min > readings[i])
        {
            min = readings[i];
        }
        else if( max < readings[i])
        {
            max = readings[i];
        }
    }
    max = (((uint32)NO_CURRENT_MV) << 1) - max; //convert the negative current to a positive on 0current + (max - 0Current)
    average = average >> NUM_BITS_FOR_CURRENT_SAMPLES;   // average the readings
    
    if(max < min)
    {
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, MaxMotorCurrentReadingmV), max);    
    }
    else
    {
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, MaxMotorCurrentReadingmV), min);            
    }
    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, AverageMotorCurrentReadingmV), average);    
    
}

void CheckMotorCurrent(void)
{
    static uint8 faultCounterOn = 0;
    static uint8 faultCounterOff = 0;
    static bool faultPreviousOnCycle = FALSE;
    static bool faultCurrentOnCycle = FALSE;
    uint32 maxCurrentmV = 0;
    uint32 runningCurrentmV = 0;
    uint32 offCurrentmV = 0;
    
    //for AC applications use the max value since average will likely be 0 current
    if((gSetup.PowerSaveMode == POWER_SAVE_OFF) || (gSetup.MtrSelection ==  MTR_VOLTAGE_120VAC) || (gSetup.MtrSelection == MTR_VOLTAGE_230VAC))
    {
        runningCurrentmV = gRun.MaxMotorCurrentReadingmV;
        offCurrentmV = gRun.MaxMotorCurrentReadingmV;
    }
    else
    {
        runningCurrentmV = gRun.AverageMotorCurrentReadingmV;
        offCurrentmV = gRun.AverageMotorCurrentReadingmV;
    }
    
    if(PMP_isOnCycleRunning())  //check to see if the pump is suppose to be on
    {
        if(gSetup.MtrProtectionEnabled == TRUE) //get the proper max current based on either the pump or the controllers max
        {
            maxCurrentmV = gSetup.MaxMtrCurrentmV;
        }
        else
        {
            if(gSetup.PowerSaveMode == POWER_SAVE_OFF)  //Power Save Mode is turned off Assume AC controller
            {
               maxCurrentmV = HIGH_POSITIVE_AC_CURRENT_THRESHOLD_MV;
            }
            else
            {
                maxCurrentmV = HIGH_POSITIVE_DC_CURRENT_THRESHOLD_MV;
            }
            
        }

        faultCounterOff = 0;
        if(runningCurrentmV < maxCurrentmV)
        {
            ++faultCounterOn;
            if(faultCounterOn > CURRENT_FAULTS_ALLOWED)
            {
                faultCounterOn = 0; //reset the counter to prevent multiple faults being being tripped, give the system a chance to respond
                //trip the alarm
                (void)ALARM_ActivateAlarm(ALARM_ID_Over_Current);
                PublishUint32(TOPIC_MotorCurrentMv, runningCurrentmV);
            }
        }
        else
        {
            faultCounterOn = 0;
        }
    }
    else    //pump is suppose to be off
    {
        faultCounterOn = 0;
//        if(offCurrentmV < HIGH_POSITIVE_NO_CURRENT_LIMIT)
//        {
//            ++faultCounterOff;
//            if(faultCounterOff > CURRENT_FAULTS_ALLOWED)
//            {
//                faultCounterOff = 0; //reset the counter to prevent multiple faults being being tripped, give the system a chance to respond
//                (void)ALARM_ActivateAlarm(ALARM_ID_Hardware_Fault);
//            }
//            
//        }
//        else
//        {
//            faultCounterOff = 0;
//        }
        
    }
    
}

void GetmVReadingfor4to20mA(void)
{
    uint8 readingsPowOf2 = 4;
    static uint32 avg_cnt = 0;
    static uint32 avg_accum = 0;    
    static uint32 AnalogInmV = 0;
    uint32 AnalogInmVAveraged = 0;
    IOrtn_mV_s16d16_t rtnval;    
    
    rtnval = IN_Voltage_Pressure_Get_4to20mA(ADC_CHANNEL_CH7);
    
    AnalogInmV = FixedPointToInteger(rtnval.mV_s16d16,DECIMAL_PLACE_NONE);
    
    
    if (avg_cnt < (uint8)(0x01 << readingsPowOf2))
    {
        avg_cnt++;
        avg_accum += AnalogInmV;
        AnalogInmVAveraged = avg_accum / avg_cnt;
    }
    else
    {
        avg_accum = (avg_accum - (avg_accum >> readingsPowOf2)) + AnalogInmV;
        AnalogInmVAveraged = avg_accum >> readingsPowOf2;
    }

    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, AnalogInmV), AnalogInmVAveraged);    
}

void SYSTEM_StopCommsTimer(void)
{
    (void)RTC_stopTimer(gModbusCommsTimerID);
}

void SYSTEM_StartCommsTimer(void)
{
    // Only start the timer if the comms alarm is enabled
    if (gSetup.ModbusCommsEnable == TRUE)
    {
        (void)RTC_startTimer(gModbusCommsTimerID);   
    }
}


