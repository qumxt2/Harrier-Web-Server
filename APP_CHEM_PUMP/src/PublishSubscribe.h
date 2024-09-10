// PublishSubscribe.h

// Copyright 2014
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// The header file for the publish/subscribe interface

#ifndef _PUBLISHSUBSCRIBE_H_
#define _PUBLISHSUBSCRIBE_H_

#include <stdint.h>
#include "socketModem.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define PUBLISH_PAYLOAD_SIZE        32
#define ACTIVATION_KEY_SIZE         9
#define PUMP_NAME_SIZE              24

typedef void (*TOPIC_CB_t)(uint8_t* pPayload, uint16_t len);

typedef struct
{
    TOPIC_CB_t callback;
    char* str;
}TOPIC_t;

typedef enum
{
    TOPIC_ActiveClients,
    TOPIC_SetPumpStatus,
    TOPIC_ResetTotalizer,
    TOPIC_ClearAlarmStatus,
    TOPIC_PumpStatus,
    TOPIC_FlowRate,
    TOPIC_Totalizer,
    TOPIC_GrandTotalizer,
    TOPIC_AlarmStatus,
    TOPIC_SetFlowRate,
    TOPIC_MeteringMode,
    TOPIC_DebugEvent,
    TOPIC_SoftwareVersion,
    TOPIC_OnTime,
    TOPIC_OffTime,
    TOPIC_OnCycles,
    TOPIC_OnTimeout,
    TOPIC_SetOnTime,
    TOPIC_SetOffTime,
    TOPIC_SetOnCycles,
    TOPIC_SetOnTimeout,
    TOPIC_Location,
    TOPIC_PressureLevel,
    TOPIC_BatteryVoltage,
    TOPIC_HighPressureTrigger,
    TOPIC_LowPressureTrigger,
    TOPIC_BatteryWarningTrigger,
    TOPIC_LowBatteryTrigger,
    TOPIC_SetHighPressureTrigger,
    TOPIC_SetLowPressureTrigger,
    TOPIC_SetLowBatteryTrigger,
    TOPIC_SignalStrength,
    TOPIC_SystemPublicationPeriod,
    TOPIC_SetSystemPublicationPeriod,
    TOPIC_ActivationKey,
    TOPIC_SetPumpName,
    TOPIC_SetBatteryWarningTrigger,
    TOPIC_PowerSaveMode, 
    TOPIC_TankLevelNotifyTrigger,
    TOPIC_SetTankLevelNotifyTrigger,
    TOPIC_TankLevelShutoffTrigger,
    TOPIC_SetTankLevelShutoffTrigger,
    TOPIC_TankLevel,
    TOPIC_SensorType,
    TOPIC_TankType,
    TOPIC_TankLevelVolumeMax,
    TOPIC_FlowVerifyPercentage,
    TOPIC_SetFlowVerifyPercentage,
    TOPIC_FlowVerifyEnable,
    TOPIC_AnalogInputMode,
    TOPIC_RawAnalogIn,
    TOPIC_Temperature,
    TOPIC_TemperatureControl,
    TOPIC_SetTemperatureControl,
    TOPIC_TemperatureSetpoint,
    TOPIC_SetTemperatureSetpoint,
    TOPIC_MotorProtectionSettings,  //the MSB byte will be 0, next byte will be the enabled? next byte will be the voltage setting, LSB byte will be the pump setting
    TOPIC_MotorCurrentMv,
    NUMBER_OF_TOPICS
} TOPIC_ID_t;

typedef struct
{
    TOPIC_ID_t  topicId;
    char        message[PUBLISH_PAYLOAD_SIZE];
    uint8_t     length;
    uint8_t     retained;
} publish_q_t;

// **********************************************************************************************************
// Public functions
// **********************************************************************************************************

void PUBLISH_init(void);
void ServiceSubscriptions(void);
void PUBLISH_start(void);
void Publish(TOPIC_ID_t topicId, uint8_t* pData, uint8_t length);
void PublishUint32(TOPIC_ID_t topicId, uint32_t data);
void PublishInt32(TOPIC_ID_t topicId, int32_t data);
void PublishUintFloat(TOPIC_ID_t topicId, double data);
void Subscribe(TOPIC_ID_t topicId, TOPIC_CB_t callback);
void PublishDebugEvent(char* pString);
void PublishString(TOPIC_ID_t topicId, char* pString);
void PUBLISH_WriteSerial(void);
CONNECTION_STATUS_t PUBLISH_reconnect(void);
bool PUBLISH_HaveUserPass(void);
char* PUBLISH_getActivationKey(void);
char* PUBLISH_getPumpName(void);
void PublishUint32_Retained(TOPIC_ID_t topicId, uint32_t data);

#endif
