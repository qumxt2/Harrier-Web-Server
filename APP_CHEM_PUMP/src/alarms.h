// alarms.h

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for handling the alarms

#ifndef ALARMS_H
#define	ALARMS_H

#include "typedef.h"
#include "event_defs.h"

// Alarm definitions

typedef enum {
    ALARM_ID_Unknown,
    ALARM_ID_Hardware_Fault,
    ALARM_ID_Low_Tank_Shutoff,
    ALARM_ID_Low_Tank_Notify,
    ALARM_ID_ANALOG_OUT_OF_RANGE,
    ALARM_ID_Modbus_Comms,
    ALARM_ID_Flow_Accuracy,
    ALARM_ID_Count_Not_Achieved,
    ALARM_ID_Input_1,
    ALARM_ID_Input_2,
    ALARM_ID_Temperature,
    ALARM_ID_Battery_Shutoff,
    ALARM_ID_Remote_Off, 
    ALARM_ID_High_Pressure,
    ALARM_ID_Low_Pressure,
    ALARM_ID_Over_Current,
    ALARM_ID_NUM_ALARMS,
} alarm_id_t;

extern const char* const ALARM_alarmNames[];

// Types
typedef enum
{
    ACTION_ALARM = 0,
    ACTION_NOTIFY,
    NUMBER_ALARM_ACTIONS
} ALARM_ACTIONS_t;

// Alarm callback will be called with the alarmId and the alarm's new state
typedef void (*ALARM_callback_function_t)( alarm_id_t alarmId, bool alarmIsActive );

// Public functions

bool ALARM_ActivateAlarm(alarm_id_t alarmId);
bool ALARM_CancelAlarm(alarm_id_t alarmId);
bool ALARM_CancelAll(void);
bool ALARM_AlarmState(alarm_id_t alarmId);
uint16 ALARM_ActiveAlarmBitfield(void);
void ALARM_EventCallback( char *event_code_str, eventtype_t event_type, eventaction_t event_action );
bool ALARM_RegisterCallback(alarm_id_t alarmId, ALARM_callback_function_t pCallback);
bool ALARM_ActiveAlarms(void);

#endif	/* ALARMS_H */

