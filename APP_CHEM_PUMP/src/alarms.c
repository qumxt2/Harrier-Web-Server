// alarms.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// Implements the logic for activating & clearing alarms

//**********************************************************************************************
// HEADER FILES
// *********************************************************************************************

#include "alarms.h"
#include "string.h"
#include "event_handler.h"
#include "PublishSubscribe.h"
#include "out_digital.h"
#include "pumpControlTask.h"
#include "dvseg_17G721_run.h"
#include "screensTask.h"
#include "AdvancedScreen.h"
#include "dvseg_17G721_setup.h"

// Public definitions
//List is currently maxed at 16 items, caused by uint16's in this file
const char* const ALARM_alarmNames[] = {
    "Unknown",
    "Relay Ctrl Mode",
    "Low tank shutoff",
    "Low tank notify",
    "Analog In",
    "Modbus Comms",
    "Flow Accuracy",
    "Count not achieved",
    "Input 1",
    "Input 2",
    "Temperature",
    "Battery Shutoff",
    "Remote off",
    "High pressure",
    "Low pressure",
    "Over Current",
};

// Private macros
#define MAGIC_NUMBER_ALARM              "ALM"
#define MAGIC_NUMBER_LEN                (3)
#define EVENT_STR_LENGTH                (sizeof(DistVarType) + 1)
#define DEFAULT_ALARM_EVENT_TYPE        (EVENTTYPE_ALARM)

// Prototypes for private functions

static bool EncodeAlarmToEventStr(alarm_id_t alarmId, char* pEventStr);
static bool DecodeEventStrToAlarm(alarm_id_t* pAlarmId, char* pEventStr);
static ALARM_ACTIONS_t getAlarmAction(alarm_id_t alarmID);

// Private types

typedef union EventStrToAlarm {
    char eventStr[EVENT_STR_LENGTH];
    struct {
        char magicNumber[MAGIC_NUMBER_LEN];
        uint8 alarmId;
    };
} alarm_trans_t;

typedef struct {
    ALARM_callback_function_t pCallback;
} alarm_meta_t;

// Private variables

static const char AlarmMagicNumber[] = MAGIC_NUMBER_ALARM;
static alarm_meta_t gAlarmMeta[ALARM_ID_NUM_ALARMS] = {{0}};

// Functions

//----------------------------------------------------------------------------
//! \brief Sound the specified alarm
//!
//! \param alarmId  The ID of the alarm to activate/sound
//!
//! \b Notes:
//!
//! \return TRUE on success, otherwise FALSE
//----------------------------------------------------------------------------
bool ALARM_ActivateAlarm(alarm_id_t alarmId)
{
    bool success = FALSE;
    char eventStr[EVENT_STR_LENGTH] = {0};

    if (getCurrentScreen() == INPUT_MODE_RUN)
    {
        success = EncodeAlarmToEventStr(alarmId, eventStr);
        success = success && EVENT_HANDLER_EventSet(eventStr, DEFAULT_ALARM_EVENT_TYPE, NULL, NULL);
    }
    if(alarmId == ALARM_ID_Hardware_Fault)
    {
        PMP_DisableSafetyRelay();  //make sure the safety relay get reenabled when clearing the Hardware fault alarm
    }

    return success;
}

//----------------------------------------------------------------------------
//! \brief Cancel the specified alarm
//!
//! \param alarmId  The ID of the alarm to deactivate/cancel
//!
//! \b Notes:
//!
//! \return TRUE on success, otherwise FALSE
//----------------------------------------------------------------------------
bool ALARM_CancelAlarm(alarm_id_t alarmId)
{
    bool success = FALSE;
    char eventStr[EVENT_STR_LENGTH] = {0};

    success = EncodeAlarmToEventStr(alarmId, eventStr);
    success = success && EVENT_HANDLER_EventClear(eventStr, DEFAULT_ALARM_EVENT_TYPE);

    if(alarmId == ALARM_ID_Hardware_Fault)// want to kill power to the pump regardless of the screen we are when when the hardware fault is triggered
    {
        PMP_ActivateSafetyRelay();  //make sure the safety relay get reenabled when clearing the Hardware fault alarm
    }
    
    return success;
}

//----------------------------------------------------------------------------
//! \brief Cancel all active alarms
//!
//! \param None
//!
//! \b Notes:
//!
//! \return TRUE on success, otherwise FALSE
//----------------------------------------------------------------------------
bool ALARM_CancelAll(void)
{
    uint8 idx;
    bool success = TRUE;

    for (idx = ALARM_ID_Unknown + 1; idx < ALARM_ID_NUM_ALARMS; idx++)
    {
        // Don't cancel alarms that aren't active, or else we'll cause
        // needless callbacks to be fired
        if (ALARM_AlarmState(idx))
        {
            success = ALARM_CancelAlarm(idx);
        }

        if (!success)
        {
            break;
        }
    }

    return success;
}

//----------------------------------------------------------------------------
//! \brief Get the state of an alarm
//!
//! \param alarmId  The ID of the alarm to check
//!
//! \b Notes:
//!
//! \return TRUE if the alarm is active/sounding, otherwise FALSE
//----------------------------------------------------------------------------
bool ALARM_AlarmState(alarm_id_t alarmId)
{
    char eventStr[EVENT_STR_LENGTH] = {0};

    (void)EncodeAlarmToEventStr(alarmId, eventStr);

    return EVENT_HANDLER_IsEventActive(eventStr, DEFAULT_ALARM_EVENT_TYPE);
}

//----------------------------------------------------------------------------
//! \brief Alarm bitfield
//!
//! \param None
//!
//! \b Notes: The bitfield is shifted one to the right, so as to match the MQTT
//            expectation. That is, the Unknown alarm is not in the bitfield
//!
//! \return Bitfield of alarms, with a bit set if the corresponding alarm is active
//----------------------------------------------------------------------------
uint16 ALARM_ActiveAlarmBitfield(void)
{
    uint16 activeAlarms = 0;
    uint16 idx = 0;
    bool isActive = FALSE;

    for (idx = ALARM_ID_Unknown + 1; idx < ALARM_ID_NUM_ALARMS; idx++)
    {
        isActive = ALARM_AlarmState(idx);
        activeAlarms &= ~(1u << idx);
        activeAlarms |= isActive << idx;
    }

    // The web portal expects a bitfield that is shifted one relative
    // to what we generate here, so let's match it.
    activeAlarms >>= 1;

    return activeAlarms;
}

// Alarms that are notify only will not be considered active alarms (will not put system into standby)
bool ALARM_ActiveAlarms(void)
{
    bool activeAlarms = FALSE;
    uint16 idx = 0;

    for (idx = ALARM_ID_Unknown + 1; idx < ALARM_ID_NUM_ALARMS; idx++)
    {
        if ((getAlarmAction(idx) == ACTION_ALARM) && ALARM_AlarmState(idx))
        {
            activeAlarms = TRUE;
        }
    }
    
    return activeAlarms;
}

//----------------------------------------------------------------------------
//! \brief Callback for the event handler interface
//!
//! \param TBD
//!
//! \b Notes: 
//!
//! \return None
//----------------------------------------------------------------------------
void ALARM_EventCallback( char *event_code_str, eventtype_t event_type, eventaction_t event_action )
{
    bool alarmState = FALSE;
    alarm_id_t alarmId = ALARM_ID_Unknown;
    uint16 alarmBitfield = 0;
    
    (void)DecodeEventStrToAlarm(&alarmId, event_code_str);

    // The MQTT topic expects a bitfield
    alarmBitfield = ALARM_ActiveAlarmBitfield();
    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, AlarmBitfield), (DistVarType)alarmBitfield);
    PublishUint32(TOPIC_AlarmStatus, alarmBitfield);

    // If any alarms are active, the LED should be on and the pump should be stopped
    if (alarmBitfield != 0)
    {   
        if (getAlarmAction(alarmId) == ACTION_ALARM)
        {
            (void)OUT_Digital_Latch_Set(IOPIN_ALARM_LED, ASSERTED);
            PMP_setStandbyMode();
        }
    }
    else
    {
        (void)OUT_Digital_Latch_Set(IOPIN_ALARM_LED, NOT_ASSERTED);
        PMP_setRunMode();
    }

    // Update the dispaly
    RefreshScreen();

    // Service user-assignable callbacks
    if (alarmId > ALARM_ID_Unknown && gAlarmMeta[alarmId].pCallback)
    {
        alarmState = ALARM_AlarmState(alarmId);
        gAlarmMeta[alarmId].pCallback(alarmId, alarmState);
    }
}

//----------------------------------------------------------------------------
//! \brief Notify other components when an alarm changes state
//!
//! \param alarmId - the alarm to monitor
//! \param pCallback - the callback function pointer, or NULL to unregister
//!
//! \b Notes: 
//!
//! \return True if successful, false otherwise
//----------------------------------------------------------------------------
bool ALARM_RegisterCallback(alarm_id_t alarmId, ALARM_callback_function_t pCallback)
{
    bool retVal = FALSE;

    if (alarmId > ALARM_ID_Unknown && alarmId < ALARM_ID_NUM_ALARMS)
    {
        gAlarmMeta[alarmId].pCallback = pCallback;

        retVal = TRUE;
    }

    return retVal;
}

// Encode the given alarm ID into an event string for compatibility with the event handler
static bool EncodeAlarmToEventStr(alarm_id_t alarmId, char* pEventStr)
{
    bool success = FALSE;
    alarm_trans_t* pUnion = (alarm_trans_t*)pEventStr;

    if (pEventStr && 
        alarmId > ALARM_ID_Unknown &&
        alarmId < ALARM_ID_NUM_ALARMS)
    {
        // Event strings are always four bytes plus a null
        memset(pEventStr, 0, EVENT_STR_LENGTH);

        memcpy((void*)pUnion->magicNumber, AlarmMagicNumber, MAGIC_NUMBER_LEN);

        // Store the alarm ID as a (probably) printable character
        pUnion->alarmId = (uint8)alarmId;

        success = TRUE;
    }
    
    return success;
}

// Decode the given event string into an alarm ID, or return FALSE if the decoding fails
static bool DecodeEventStrToAlarm(alarm_id_t* pAlarmId, char* pEventStr)
{
    bool success = FALSE;
    alarm_id_t alarmRaw = ALARM_ID_Unknown;
    alarm_trans_t* pUnion = (alarm_trans_t*)pEventStr;

    if (pEventStr)
    {
        if (strncmp((void*)pUnion->magicNumber, AlarmMagicNumber, MAGIC_NUMBER_LEN) == 0)
        {
            alarmRaw = (alarm_id_t)pUnion->alarmId;
        }
    }
    
    if (alarmRaw > ALARM_ID_Unknown && alarmRaw < ALARM_ID_NUM_ALARMS)
    {
        success = TRUE;
        *pAlarmId = alarmRaw;
    }

    return success;
}

ALARM_ACTIONS_t getAlarmAction(alarm_id_t alarmID)
{
    ALARM_ACTIONS_t alarmAction = ACTION_ALARM;

    switch (alarmID)
    {
        case ALARM_ID_Input_1:
        case ALARM_ID_Input_2:
        case ALARM_ID_High_Pressure:
        case ALARM_ID_Low_Pressure:
            alarmAction = gSetup.AlarmAction;                // Some alarms can be NOTIFY or ALARM
            break;

        case ALARM_ID_Count_Not_Achieved:
            if( gSetup.MeteringMode == METERING_MODE_VOLUME)
            {
                alarmAction = gSetup.AlarmAction;            // Allow notify only for flow mode.
            }
            else
            {
                alarmAction = ACTION_ALARM;
            }
            break;
            
        case ALARM_ID_Flow_Accuracy:
        case ALARM_ID_Low_Tank_Notify:
        case ALARM_ID_ANALOG_OUT_OF_RANGE:
        case ALARM_ID_Modbus_Comms:
        case ALARM_ID_Hardware_Fault:                       //The idea behind this is when this is active we will change how the Pump control operates so that it is using the safety relay to contol the pump. This way the end user is not completely dead in the water
            alarmAction = ACTION_NOTIFY;                    // Always notify only
            break;
            
        case ALARM_ID_Low_Tank_Shutoff:
        case ALARM_ID_Over_Current:
            alarmAction = ACTION_ALARM;                     // Always alarm
            break;

        default:
            alarmAction = ACTION_ALARM;                     // All other alarms will always ALARM
            break;
    }
    
    return (alarmAction);
}

