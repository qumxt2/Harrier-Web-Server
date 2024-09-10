// PublishSubscribe.c

#include "wolfssl/ssl.h"


// Copyright 2014
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the publish/subscribe interface to the MQTT protocol

// **********************************************************************************************************
// Header files
// **********************************************************************************************************

#include "typedef.h"
#include "stdint.h"
#include "stdio.h"
#include "debug.h"
#include "rtos.h"
#include "modemTask.h"
#include "dvinterface_17G721.h"
#include "dvseg_17G721_setup.h"
#include "dvseg_17G721_run.h"
#include "string.h"
#include "socketModem.h"
#include "PublishSubscribe.h"
#include "MQTTPacket.h"
#include "rtcTask.h"
#include "pumpControlTask.h"
#include "NetworkScreen.h"
#include "alarms.h"
#include "sslHelper.h"
#include "stdlib.h"
#include "systemTask.h"
#include "queue.h"
#include "assert_app.h"
#include "PublishSubscribe.h"
#include "AlarmScreenBattery.h"
#include "utilities.h"
#include "advancedScreen.h"
#include "AlarmScreenMotorCurrent.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

#define MAX_MQTT_MSG_SIZE           200
#define PUBLISH_Q_SIZE              40
#define MAX_TOPIC_STR_SIZE          128
#define CONNACK_TIMEOUT             2
#define MAX_UINT32_STRING_SIZE      12
#define MESSAGE_RETAINED            1
#define MESSAGE_NOT_RETAINED        0

#define PING_REPEAT_INTERVAL        180

#define MAX_BUF                (192u)
#define MAX_USER_PASS_LEN      (24u)

#ifndef FORCE_INSECURE
#define writeSerial(buf, len) (void)SSL_writeBytes((char*)buf, len)
#define readSerialFunc  SSL_readBytes
#define readSerial(buf, maxBytes) readSerialFunc((unsigned char*)buf, maxBytes)
#else
#define writeSerial(buf, len) MODEM_SendString((uint8*)buf, len)
#define readSerialFunc getSocketModemData
#define readSerial(buf, maxBytes) readSerialFunc((uint8*)buf, maxBytes)
#endif

#define API_MQTT_AUTH_MAGIC_NUMBER "98618265897354"
#define API_MQTT_AUTH_REQUEST "GET /api/v1.0/mqttauth/?pump_id=%s&magic_number=" API_MQTT_AUTH_MAGIC_NUMBER " HTTP/1.1\r\n" "Host: " HOST_FQDN "\r\n\r\n"

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

static int16_t gCommsWatchdog = 0;

static bool gPingTimerExpired = FALSE;
static uint8 gPingTimerId = RTC_TIMER_INVALID;

static bool gHaveUserPass = FALSE;
#define MQTT_USERNAME_MAX_SIZE      (16u + 1)
static char gMqttUsername[MQTT_USERNAME_MAX_SIZE];
static char gMqttPassword[MQTT_USERNAME_MAX_SIZE];

static publish_q_t gPublishBuffer[PUBLISH_Q_SIZE];
static queue_t  gPublishQ;

static char gActivationKeyStr[ACTIVATION_KEY_SIZE] = "";
static char gPumpNameStr[PUMP_NAME_SIZE] = "";

static TOPIC_t gTopic[NUMBER_OF_TOPICS] =
{
    { NULL, "ActiveClients" },
    { NULL, "SetPumpStatus" },
    { NULL, "ResetTotalizer" },
    { NULL, "ClearAlarmStatus" },
    { NULL, "PumpStatus" },
    { NULL, "FlowRate" },
    { NULL, "Totalizer" },
    { NULL, "GrandTotalizer" },
    { NULL, "AlarmStatus" },
    { NULL, "SetFlowRate" },
    { NULL, "MeteringMode" },
    { NULL, "DebugEvent" },
    { NULL, "SoftwareVersion" },
    { NULL, "OnTime" },
    { NULL, "OffTime" },
    { NULL, "OnCycles" },
    { NULL, "PumpOnTimeout" },
    { NULL, "SetOnTime" },
    { NULL, "SetOffTime" },
    { NULL, "SetOnCycles" },
    { NULL, "SetPumpOnTimeout" },
    { NULL, "Location" },
    { NULL, "PressureLevel" },
    { NULL, "BatteryVoltage" },
    { NULL, "HighPressureTrigger" },
    { NULL, "LowPressureTrigger" },
    { NULL, "BatteryWarningTrigger" },
    { NULL, "LowBatteryTrigger" },
    { NULL, "SetHighPressureTrigger" },
    { NULL, "SetLowPressureTrigger" },
    { NULL, "SetLowBatteryTrigger" },
    { NULL, "SignalStrength" },
    { NULL, "SystemPublicationPeriod" },
    { NULL, "SetSystemPublicationPeriod" },
    { NULL, "ActivationKey" },
    { NULL, "SetPumpName" },
    { NULL, "SetBatteryWarningTrigger" },
    { NULL, "PowerSaveMode" },
    { NULL, "TankLevelNotifyTrigger" },
    { NULL, "SetTankLevelNotifyTrigger" },
    { NULL, "TankLevelShutoffTrigger" },
    { NULL, "SetTankLevelShutoffTrigger" },    
    { NULL, "TankLevel" },    
    { NULL, "SensorType" },
    { NULL, "TankType" },
    { NULL, "TankLevelVolumeMax" },
    { NULL, "FlowVerifyPercentage" },
    { NULL, "SetFlowVerifyPercentage" },
    { NULL, "FlowVerifyEnable" },
    { NULL, "AnalogInputMode" },
    { NULL, "RawAnalogIn" },
    { NULL, "Temperature" },
    { NULL, "TemperatureControl" },
    { NULL, "SetTemperatureControl" },
    { NULL, "TemperatureSetpoint" },
    { NULL, "SetTemperatureSetpoint" },
    { NULL, "MotorProtection" },
    { NULL, "MotorCurrentMv" },
};


// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void PublishAfterConnect(void);
static void DoOneTimePublication(void);
static void mqttPublish(char* topicStr, char* pMessage, uint8_t length, uint8_t retained);
static void mqttSubscribe(char* topicStr);
static void mqttConnect(void);
static void mqttPing(void);
static void setPumpStatusCallback(uint8_t* pPayload, uint16_t len);
static void clearAlarmStatusCallback(uint8_t* pPayload, uint16_t len);
static void setHighPressureTriggerCallback(uint8_t* pPayload, uint16_t len);
static void setLowPressureTriggerCallback(uint8_t* pPayload, uint16_t len);
static void setBatteryWarningTriggerCallback(uint8_t* pPayload, uint16_t len);
static void setLowBatteryTriggerCallback(uint8_t* pPayload, uint16_t len);
static void setSystemPublicationPeriodCallback(uint8_t* pPayload, uint16_t len);
static void activationKeyCallback(uint8_t* pPayload, uint16_t len);
static void pumpNameCallback(uint8_t* pPayload, uint16_t len);
static void pingTimerCallback(uint8 ignored);
static void pingTimerStart(uint32 seconds);
static void getUsernamePassword(void);
static void mqttEnqueue(TOPIC_ID_t topicId, char* pMessage, uint8_t length, uint8_t retained);
static void setTankLevelShutoffTriggerCallback(uint8_t* pPayload, uint16_t len);
static void setTankLevelNotifyTriggerCallback(uint8_t* pPayload, uint16_t len);
static void setFlowVerifyPercentageCallback(uint8_t* pPayload, uint16_t len);
static void setTemperatureControlCallback(uint8_t* pPayload, uint16_t len);
static void setTemperatureSetpointCallback(uint8_t* pPayload, uint16_t len);

// **********************************************************************************************************
// PUBLISH_init - Initialize the publish/subscribe interface
// **********************************************************************************************************
void PUBLISH_init(void)
{
    assert(Q_init(&gPublishQ, gPublishBuffer, PUBLISH_Q_SIZE, sizeof(publish_q_t)) == Q_SUCCESS);
}

// **********************************************************************************************************
// PUBLISH_start - Start the publish/subscribe interface
// **********************************************************************************************************
void PUBLISH_start(void)
{
    uint16_t i;

    if (gSetup.NetworkMode == NETWORK_MODE_CELLULAR)
    {
        if (MODEM_Connected())
        {
            mqttConnect();

            (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, MqttConnected), TRUE);

            // Keep the network screen in sync
            (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, ConnectionStatus), (DistVarType)CONNECTION_SUCCESS);
            RefreshScreen();

            gTopic[TOPIC_SetPumpStatus].callback = setPumpStatusCallback;
            gTopic[TOPIC_ClearAlarmStatus].callback = clearAlarmStatusCallback;
            gTopic[TOPIC_SetHighPressureTrigger].callback = setHighPressureTriggerCallback;
            gTopic[TOPIC_SetLowPressureTrigger].callback = setLowPressureTriggerCallback;
            gTopic[TOPIC_SetLowBatteryTrigger].callback = setLowBatteryTriggerCallback;
            gTopic[TOPIC_SetSystemPublicationPeriod].callback = setSystemPublicationPeriodCallback;
            gTopic[TOPIC_ActivationKey].callback = activationKeyCallback;
            gTopic[TOPIC_SetPumpName].callback = pumpNameCallback;
            gTopic[TOPIC_SetBatteryWarningTrigger].callback = setBatteryWarningTriggerCallback;
            gTopic[TOPIC_SetTankLevelNotifyTrigger].callback = setTankLevelNotifyTriggerCallback;
            gTopic[TOPIC_SetTankLevelShutoffTrigger].callback = setTankLevelShutoffTriggerCallback;
            gTopic[TOPIC_SetFlowVerifyPercentage].callback = setFlowVerifyPercentageCallback;
            gTopic[TOPIC_SetTemperatureControl].callback = setTemperatureControlCallback;
            gTopic[TOPIC_SetTemperatureSetpoint].callback = setTemperatureSetpointCallback;            

            for (i = 0; i < NUMBER_OF_TOPICS; i++)
            {
                if (gTopic[i].callback != NULL)
                {
                    Subscribe((TOPIC_ID_t)i, gTopic[i].callback);
                }
            }

            // Excpect a CONNACK message after CONNACK_TIMEOUT seconds and a ping every PING_REPEAT_INTERVAL
            gCommsWatchdog = 1;

            PublishAfterConnect();
        }

        // Always start the ping timer even if the connection was not successful. The ping timer will cause
        // future connection retries
        pingTimerStart(CONNACK_TIMEOUT);
    }
}

// **********************************************************************************************************
// PUBLISH_HaveMqttUserPass -- Whether we have obtained the MQTT username and password
// **********************************************************************************************************
bool PUBLISH_HaveMqttUserPass(void)
{
    return gHaveUserPass;
}

// **********************************************************************************************************
// PublishAfterConnect -- Topics to publish automatically after a (re)connection
// **********************************************************************************************************

static void PublishAfterConnect(void)
{
    char* pLocation = GetLocation();

    DoOneTimePublication();

    PublishUint32_Retained(TOPIC_ActiveClients, 1);
    PublishUint32(TOPIC_PumpStatus, gRun.PumpStatus);
    if(gSetup.AnalogInControl == AIN_FLOW_RATE)
    {
        PublishUint32(TOPIC_FlowRate, gRun.AnalogFlowRate);
    }
    else
    {
        PublishUint32(TOPIC_FlowRate, gSetup.DesiredFlowRate);
    }
    PublishUint32(TOPIC_AlarmStatus, ALARM_ActiveAlarmBitfield());
    PublishUint32(TOPIC_MeteringMode, gSetup.MeteringMode + 1);

    PublishUint32(TOPIC_OnTime, gSetup.OnTime);
    PublishUint32(TOPIC_OffTime, gSetup.OffTime);
    PublishUint32(TOPIC_OnCycles, gSetup.OnCycles);
    PublishUint32(TOPIC_OnTimeout, gSetup.OnTimeout);
    if (POWER_SAVE_OFF != gSetup.PowerSaveMode)
    {
        PublishUint32(TOPIC_BatteryVoltage, gRun.BatteryMillivolts);
    }

    PublishUint32(TOPIC_HighPressureTrigger, gSetup.HighPressureTrigger);
    PublishUint32(TOPIC_LowPressureTrigger, gSetup.LowPressureTrigger);
    PublishUint32(TOPIC_PressureLevel, gRun.Pressure_1_Psi);
    PublishUint32(TOPIC_TankLevelNotifyTrigger, gSetup.TankLevelNotifyTrigger);
    PublishUint32(TOPIC_TankLevelShutoffTrigger, gSetup.TankLevelShutoffTrigger);
    PublishUint32(TOPIC_TankLevel, gRun.TankLevel);
    PublishUint32(TOPIC_TankLevelVolumeMax, gSetup.MaxTankVolume);
    PublishUint32(TOPIC_TankType, gSetup.TankType + 1);
    PublishUint32(TOPIC_FlowVerifyEnable, gSetup.FlowVerifyEnable);
    PublishUint32(TOPIC_FlowVerifyPercentage, gSetup.FlowVerifyPercentage);
    PublishUint32(TOPIC_PowerSaveMode, gSetup.PowerSaveMode);    
    PublishUint32(TOPIC_BatteryWarningTrigger, gSetup.BatteryWarningTrigger);
    PublishUint32(TOPIC_LowBatteryTrigger, gSetup.BatteryShutoffTrigger);
    PublishUint32(TOPIC_SignalStrength, GetSignalStrength());
    PublishUint32(TOPIC_SystemPublicationPeriod, gSetup.SystemPublicationPeriod);
    PublishInt32(TOPIC_Temperature, (int32_t)gRun.Temperature);
    PublishUint32(TOPIC_TemperatureControl, gSetup.TempControl + 1);
    PublishInt32(TOPIC_TemperatureSetpoint, (int32_t)gSetup.TempSetpoint);
    if (gSetup.MtrProtectionEnabled == MTR_PROTECTION_ON)
    {
        PublishUint32(TOPIC_MotorProtectionSettings, PackageMotorProtectionSettings());
    }
    // Publish the location only if there is one
    if( pLocation[0] != '\0' )
    {
        PublishString(TOPIC_Location, pLocation);
    }

}

// **********************************************************************************************************
// DoOneTimePublication - Publications to do only the first time that a pump connects after startup
// **********************************************************************************************************

static void DoOneTimePublication(void)
{
    static bool alreadyPublished = FALSE;
    char versionStr[] ="XX.XX.XX";

    // Publish only the first time
    if (!alreadyPublished)
    {
        sprintf(versionStr, "%2d.%2d.%2d", VER17G721_MAJOR, VER17G721_MINOR, VER17G721_BUILD);
        Publish(TOPIC_SoftwareVersion, (uint8_t*)versionStr, strlen(versionStr));

        alreadyPublished = TRUE;
    }
}

// **********************************************************************************************************
// Publish - Publish data to a MQTT topic
// **********************************************************************************************************

void Publish(TOPIC_ID_t topicId, uint8_t* pData, uint8_t length)
{
    mqttEnqueue(topicId, (char*)pData, length, MESSAGE_NOT_RETAINED);
}

// **********************************************************************************************************
// PublishDebugEvent - Publish a string to the debug event topic
// **********************************************************************************************************

void PublishDebugEvent(char* pString)
{
    mqttEnqueue(TOPIC_DebugEvent, pString, strlen(pString), MESSAGE_NOT_RETAINED);
}

// **********************************************************************************************************
// PublishUint32 - Publish an uint32_t integer to a topic
// **********************************************************************************************************

void PublishUint32(TOPIC_ID_t topicId, uint32_t data)
{
    char str[MAX_UINT32_STRING_SIZE];

    sprintf(str, "%lu", (uint32)data);
    mqttEnqueue(topicId, str, strlen(str), MESSAGE_NOT_RETAINED);
}

// **********************************************************************************************************
// PublishUint32_ - Publish an uint32_t integer to a topic & retain it on the server
// **********************************************************************************************************

void PublishUint32_Retained(TOPIC_ID_t topicId, uint32_t data)
{
    char str[MAX_UINT32_STRING_SIZE];

    sprintf(str, "%lu", (uint32)data);
    mqttEnqueue(topicId, str, strlen(str), MESSAGE_RETAINED);
}

// **********************************************************************************************************
// PublishIint32 - Publish an sint32_t integer to a topic
// **********************************************************************************************************

void PublishInt32(TOPIC_ID_t topicId, int32_t data)
{
    char str[MAX_UINT32_STRING_SIZE];

    sprintf(str, "%ld", data);
    mqttEnqueue(topicId, str, strlen(str), MESSAGE_NOT_RETAINED);
}

// **********************************************************************************************************
// PublishUintFloat - Publish an float integer to a topic
// **********************************************************************************************************

void PublishUintFloat(TOPIC_ID_t topicId, double data)
{
    char str[MAX_UINT32_STRING_SIZE];

    sprintf(str, "%.4f", data);
    mqttEnqueue(topicId, str, strlen(str), MESSAGE_NOT_RETAINED);
}

// **********************************************************************************************************
// PublishString - Publish a string to a topic
// **********************************************************************************************************

void PublishString(TOPIC_ID_t topicId, char* pString)
{
    mqttEnqueue(topicId, pString, strlen(pString), MESSAGE_NOT_RETAINED);
}

// **********************************************************************************************************
// Subscribe - Subscribe to a MQTT topic
// **********************************************************************************************************

void Subscribe(TOPIC_ID_t topicId, TOPIC_CB_t callback)
{
    gTopic[topicId].callback = callback;

    if (gRun.MqttConnected == TRUE)
    {
        mqttSubscribe(gTopic[topicId].str);
    }
}

// **********************************************************************************************************
// ServiceSubscriptions - Handle an incoming message from a subscribed topic
// **********************************************************************************************************

void ServiceSubscriptions(void)
{
    if (gSetup.NetworkMode == NETWORK_MODE_CELLULAR)
    {
        unsigned char buf[200] = {0};
        int buflen = sizeof(buf);
        int messageType = 0;

        if (!MODEM_Connected() || !gRun.MqttConnected || !PUBLISH_HaveMqttUserPass())
        {
            (void)PUBLISH_reconnect();
        }

        // Check again, in case the above failed
        if (MODEM_Connected() && gRun.MqttConnected && PUBLISH_HaveMqttUserPass())
        {
            while (messageType >= 0)
            {
                messageType = MQTTPacket_read(buf, buflen, readSerialFunc);

                if (messageType == PUBLISH)
                {
                    unsigned char dup;
                    int qos;
                    unsigned char retained;
                    unsigned short msgid;
                    int payloadlen_in;
                    unsigned char* payload_in;
                    MQTTString receivedTopic;
                    uint16_t i;

                    (void)MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
                                                 &payload_in, &payloadlen_in, buf, buflen);

                    // Send PUBACK if QoS is > 0
                    if (qos > 0)
                    {
                        uint16_t len = MQTTSerialize_ack(buf, buflen, PUBACK, dup, msgid);
                        writeSerial((char*)buf, len);
                    }

                    // Loop through the topics and call the appropriate callback
                    for (i = 0; i < NUMBER_OF_TOPICS; i++)
                    {
                        if (memcmp(receivedTopic.lenstring.data, gTopic[i].str, strlen(gTopic[i].str) - 1) == 0)
                        {
                            if (gTopic[i].callback != NULL)
                            {
                                gTopic[i].callback(payload_in, payloadlen_in);
                            }
                            break;
                        }
                    }

                    printf("Message received\n");
                }

                // Track ping responses
                else if (messageType == PINGRESP)
                {
                    gCommsWatchdog--;
                    printf("Pong!");
                }

                // Watch for the connection ack
                else if (messageType == CONNACK)
                {
                    uint8_t sessionPresent;
                    uint8_t connack_rc;

                    (void)MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen);
                    if (connack_rc == 0)
                    {
                        gCommsWatchdog--;
                        printf("MQTT connack received\n");
                    }
                }
                else if (messageType == MQTTPACKET_READ_ERROR)
                {
    #ifndef FORCE_INSECURE
                    // Turns out that this is only a true error when SSL is active, and even then it
                    // might not be the case
                    sint16 errorCode = SSL_isError();

                    if (errorCode != 0)
                    {
                        printf("SSL error: %d", errorCode);
                        (void)PUBLISH_reconnect();
                    }
    #endif
                }

                // Heartbeat ping
                if (gPingTimerExpired)
                {
                    mqttPing();
                    pingTimerStart(PING_REPEAT_INTERVAL);
                }
            }
        }
    }
}

// **********************************************************************************************************
// setPumpStatusCallback - The callback for the SetPumpStatus topic
// **********************************************************************************************************

static void setPumpStatusCallback(uint8_t* pPayload, uint16_t len)
{
    if (pPayload[0] == '1')
    {
        PMP_setRunMode();
    }
    else
    {
        PMP_setStandbyMode();
    }
}

// **********************************************************************************************************
// clearAlarmStatusCallback - The callback for the ClearAlarmStatus topic
// **********************************************************************************************************

static void clearAlarmStatusCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t alarmBitfield = 0;
    uint16 idx = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        // shift by 1 bit to sync web alarm id with firmware alarm id
        alarmBitfield = atoi((char*)pPayload) << 1;
        
        if (alarmBitfield == 0)
        {
            (void)ALARM_CancelAll();
        }
        else
        {
            for (idx = ALARM_ID_Unknown + 1; idx < ALARM_ID_NUM_ALARMS; idx++)
            {
                if (alarmBitfield & (1 << idx))
                {
                    if (!ALARM_AlarmState(idx))
                    {
                        (void)ALARM_ActivateAlarm(idx);
                    }
                }
            }
        }
    }
}

// **********************************************************************************************************
// setHighPressureTriggerCallback - The callback for web-based changes to the high-pressure alarm trigger point
// **********************************************************************************************************

static void setHighPressureTriggerCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);

        (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, HighPressureTrigger), (DistVarType)integerValue);
        PublishUint32(TOPIC_HighPressureTrigger, gSetup.HighPressureTrigger);
    }
}

// **********************************************************************************************************
// setLowPressureTriggerCallback - The callback for web-based changes to the low-pressure alarm trigger point
// **********************************************************************************************************

static void setLowPressureTriggerCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);

        (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, LowPressureTrigger), (DistVarType)integerValue);
        PublishUint32(TOPIC_LowPressureTrigger, gSetup.LowPressureTrigger);
    }
}

// **********************************************************************************************************
// setTankLevelShutoffTriggerCallback - The callback for web-based changes to the low tank alarm trigger point
// **********************************************************************************************************
static void setTankLevelShutoffTriggerCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);
        
        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TankLevelShutoffTrigger), (DistVarType)integerValue);
        PublishUint32(TOPIC_TankLevelShutoffTrigger, gSetup.TankLevelShutoffTrigger);
    }
}

// **********************************************************************************************************
// setTankLevelNotifyCallback - The callback for web-based changes to the low tank alarm trigger point
// **********************************************************************************************************
static void setTankLevelNotifyTriggerCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);

        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TankLevelNotifyTrigger), (DistVarType)integerValue);
        PublishUint32(TOPIC_TankLevelNotifyTrigger, gSetup.TankLevelNotifyTrigger);
    }
}

// **********************************************************************************************************
// setFlowVerifyPercentageCallback - The callback for web-based changes to the flow verify percentage point
// **********************************************************************************************************
static void setFlowVerifyPercentageCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);

        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, FlowVerifyPercentage), (DistVarType)integerValue);
        PublishUint32(TOPIC_FlowVerifyPercentage, gSetup.FlowVerifyPercentage);
    }
}

// **********************************************************************************************************
// setTemperatureControlCallback - The callback for web-based changes to the temperature control select box
// **********************************************************************************************************
static void setTemperatureControlCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);
     
        // Store signed values from the web in unsigned dvars.  The app will cast them back to signed before using them.
        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TempControl), (DistVarType)integerValue);
        
        // The web will store the unsigned dvar in a signed integer & treat it as a signed #
        PublishUint32(TOPIC_TemperatureControl, gSetup.TempControl + 1);
    }
}

// **********************************************************************************************************
// setTemperatureSetpointCallback - The callback for web-based changes to the temperature setpoint
// **********************************************************************************************************
static void setTemperatureSetpointCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);

        // Store signed values from the web in unsigned dvars.  The app will cast them back to signed before using them.
        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TempSetpoint), (DistVarType)integerValue);

        // The web will store the unsigned dvar in a signed integer & treat it as a signed #
        PublishInt32(TOPIC_TemperatureSetpoint, (int32_t)gSetup.TempSetpoint);
    }
}

// **********************************************************************************************************
// setLowBatteryTriggerCallback - The callback for web-based changes to the low-battery alarm trigger point
// **********************************************************************************************************

static void setLowBatteryTriggerCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);

        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, BatteryShutoffTrigger), (DistVarType)integerValue);
        PublishUint32(TOPIC_LowBatteryTrigger, gSetup.BatteryShutoffTrigger);
    }
}

// **********************************************************************************************************
// setBatteryWarningTriggerCallback - The callback for web-based changes to the battery level
// **********************************************************************************************************

static void setBatteryWarningTriggerCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);

        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, BatteryWarningTrigger), (DistVarType)integerValue);
        PublishUint32(TOPIC_BatteryWarningTrigger, gSetup.BatteryWarningTrigger);
    }
}

// **********************************************************************************************************
// setSystemPublicationPeriodCallback - The callback for web-based changes to the system publication period
// **********************************************************************************************************

static void setSystemPublicationPeriodCallback(uint8_t* pPayload, uint16_t len)
{
    uint32_t integerValue = 0;

    // Catch some runaway situations
    if (strlen((char*)pPayload) == len)
    {
        integerValue = atoi((char*)pPayload);

        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, SystemPublicationPeriod), (DistVarType)integerValue);
        PublishUint32(TOPIC_SystemPublicationPeriod, gSetup.SystemPublicationPeriod);

        // Force the new publication period to be picked up immediately
        SYSTEM_ForcePublicationNow();
    }
}

// **********************************************************************************************************
// activationKeyCallback - The callback for the activation Key
// **********************************************************************************************************

static void activationKeyCallback(uint8_t* pPayload, uint16_t len)
{
    memcpy(gActivationKeyStr, pPayload, ACTIVATION_KEY_SIZE - 1);
    gActivationKeyStr[ACTIVATION_KEY_SIZE - 1] ='\0';
    RefreshScreen();
}

// **********************************************************************************************************
// pumpNameCallback - The callback for the pump name
// **********************************************************************************************************

static void pumpNameCallback(uint8_t* pPayload, uint16_t len)
{
    len++;
    if (len > PUMP_NAME_SIZE)
    {
        len = PUMP_NAME_SIZE;
    }
    
    memcpy(gPumpNameStr, pPayload, len);
    gPumpNameStr[len - 1] ='\0';
    RefreshScreen();
}


// **********************************************************************************************************
// PUBLISH_getActivationKey - Get a pointer to the activation key string
// **********************************************************************************************************

char* PUBLISH_getActivationKey(void)
{
    if (strlen(gActivationKeyStr) > ACTIVATION_KEY_SIZE)
    {
        gActivationKeyStr[0] = '\0';
    }

    return gActivationKeyStr;
}


// **********************************************************************************************************
// PUBLISH_getPumpName - Get a pointer to the pump name string
// **********************************************************************************************************

char* PUBLISH_getPumpName(void)
{
    if (strlen(gActivationKeyStr) > PUMP_NAME_SIZE)
    {
        gActivationKeyStr[0] = '\0';
    }

    return gPumpNameStr;
}


// **********************************************************************************************************
// mqttConnect - Connect to the server using the MQTT connect message
// **********************************************************************************************************

static void mqttConnect(void)
{
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    uint8_t buf[MAX_MQTT_MSG_SIZE];
    uint16_t buflen = sizeof(buf);
    uint16_t len = 0;
    char fullTopicStr[MAX_TOPIC_STR_SIZE] = "";
    char* pUniqueID = GetId();

    // Build the string for the ActiveClients topic
    strcpy(fullTopicStr, gTopic[TOPIC_ActiveClients].str);
    strcat(fullTopicStr, "/");
    strcat(fullTopicStr, pUniqueID);

    data.clientID.cstring = pUniqueID;
    data.username.cstring = gMqttUsername;
    data.password.cstring = gMqttPassword;
    data.keepAliveInterval = PING_REPEAT_INTERVAL * 2;
    data.cleansession = 1;
    data.MQTTVersion = 4;
    data.will.retained = 1;

    // Setup will to publish "0" to ActiveClients when connection times out
    data.will.retained = 1;
    data.willFlag = 1;
    data.will.topicName.cstring = fullTopicStr;
    data.will.message.cstring = "0";

    len = MQTTSerialize_connect(buf, buflen, &data);
    writeSerial((char*)buf, len);
}

// **********************************************************************************************************
// mqttPublish - Publish a message to a MQTT topic
// **********************************************************************************************************

static void mqttPublish(char* topicStr, char* pMessage, uint8_t length, uint8_t retained)
{
    uint8_t buf[MAX_MQTT_MSG_SIZE];
    uint16_t buflen = sizeof(buf);
    MQTTString topic = MQTTString_initializer;
    char fullTopicStr[MAX_TOPIC_STR_SIZE] = "";
    uint16_t len = 0;
    char* pUniqueID = GetId();

    // Don't try to publish if we're not connected
    if (!gRun.MqttConnected)
    {
        return;
    }

    // Build topic string
    strcpy(fullTopicStr, topicStr);
    strcat(fullTopicStr, "/");
    strcat(fullTopicStr, pUniqueID);
    topic.cstring = fullTopicStr;

    len = MQTTSerialize_publish(buf, buflen, 0, 0, retained, 0, topic, (uint8_t*)pMessage, length);
    writeSerial((char*)buf, len);
}

// **********************************************************************************************************
// mqttSubscribe - Subscribe updates on a MQTT topic
// **********************************************************************************************************

static void mqttSubscribe(char* topicStr)
{
    char buf[MAX_MQTT_MSG_SIZE];
    uint16_t buflen = sizeof(buf);
    MQTTString topic = MQTTString_initializer;
    char fullTopicStr[MAX_TOPIC_STR_SIZE] = "";
    uint16_t len = 0;
    uint8_t msgid = 1;
    uint8_t req_qos = 2;
    char* pUniqueID = GetId();

    // Build topic string
    strcpy(fullTopicStr, topicStr);
    strcat(fullTopicStr, "/");
    strcat(fullTopicStr, pUniqueID);
    topic.cstring = fullTopicStr;

    len = MQTTSerialize_subscribe((uint8_t*)buf, buflen, 0, msgid, 1, &topic, (int*)&req_qos);
    writeSerial((char*)buf, len);
}

// **********************************************************************************************************
// mqttSubscribe - Send the MQTT ping message
// **********************************************************************************************************

static void mqttPing(void)
{
    uint8_t buf[MAX_MQTT_MSG_SIZE];
    uint16_t buflen = sizeof(buf);
    uint16_t len = 0;

    if (gCommsWatchdog > 0)
    {
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, MqttConnected), FALSE);
    }
    else
    {
        len = MQTTSerialize_pingreq(buf, buflen);
        writeSerial((char*)buf, len);
        gCommsWatchdog++;
        printf("Ping?");
    }
}

// **********************************************************************************************************
// PUBLISH_reconnect - Execute a reconnection to the server
// **********************************************************************************************************
CONNECTION_STATUS_t PUBLISH_reconnect(void)
{
    CONNECTION_STATUS_t status = CONNECTION_SUCCESS;

    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, MqttConnected), FALSE);

    if (gSetup.NetworkMode == NETWORK_MODE_CELLULAR)
    {
        printf("Connecting...\n");

        status = InitializeSocketModem();

        if (status == CONNECTION_SUCCESS)
        {
            if (!gHaveUserPass)
            {
                getUsernamePassword();
            }
            else
            {
#ifndef FORCE_INSECURE
                status = SSL_Connect(HOST_FQDN, FALSE) ? CONNECTION_SUCCESS : CONNECTION_SSL_ERROR;
#endif
                if (status == CONNECTION_SUCCESS)
                {
                    PUBLISH_start();
                    gCommsWatchdog = 0;
                }
            }
        }
    }

    return status;
}

// **********************************************************************************************************
// PUBLISH_HaveUserPass - Indicate if we have acquired the MQTT username and password yet
// **********************************************************************************************************
bool PUBLISH_HaveUserPass(void)
{
    return gHaveUserPass;
}

// **********************************************************************************************************
// pingTimerCallback - Callback for when the ping timer has expired
// **********************************************************************************************************
static void pingTimerCallback(uint8 ignored)
{
    gPingTimerExpired = TRUE;
}

// **********************************************************************************************************
// Search for string "needle" in the stream buffer. Requires maintaining matchIndexSoFar
// from call to call outside of the function.
//
// Returns whether the full search term was found
// **********************************************************************************************************
static bool streamSearch(const uint8* streamBuf, const uint16 bufLen, const uint8* needle, uint16* pMatchIndexSoFar, uint16* pNeedleIndex)
{
    const uint8 needleLen = strlen((const char*)needle);
    sint16 foundIndex = -1;
    sint16 bufIndex;
    char streamByte = 0;

    for (bufIndex = 0; bufIndex < (bufLen - *pNeedleIndex); bufIndex++)
    {
        streamByte = streamBuf[*pNeedleIndex + bufIndex];
        if (needle[*pMatchIndexSoFar] == streamByte || needle[*pMatchIndexSoFar] == '*')
        {
            // Sequence matching so far
            *pMatchIndexSoFar += 1;
        }
        else if (needle[0] == streamByte)
        {
            // Also check if we're starting the match over. Important when
            // we had a partial match immediately prior.
            *pMatchIndexSoFar = 1;
        }
        else
        {
            // Broke our string of matches
            *pMatchIndexSoFar = 0;
        }

        if (*pMatchIndexSoFar == needleLen)
        {
            // Found all of needle
            foundIndex = bufIndex;
            break;
        }
    }

    if (foundIndex >= 0)
    {
        *pMatchIndexSoFar = 0;

        // Indicate where our successful search ended so that future calls can start their own search from there
        *pNeedleIndex += foundIndex + 1;
    }
    
    return (foundIndex >= 0);
}

// **********************************************************************************************************
// streamExtract - Extract a string from a possibly fragmented stream. Returns true when we think that we have the whole string
// **********************************************************************************************************
static bool streamExtract(const uint8* buf, uint8* outStore, uint16 bufLen, uint16 outStoreSize, uint16* pMatchDepth, uint16* pNeedleIndex)
{
    bool stringDone = FALSE;
    sint16 i;

    for (i = 0; (*pNeedleIndex + i) < bufLen && (*pMatchDepth + i) < (outStoreSize-1); i++ )
    {
        if (buf[*pNeedleIndex + i] == '\n' || buf[*pNeedleIndex + i] == '\r')
        {
            // Found end of string
            outStore[*pMatchDepth + i] = '\0';
            stringDone = TRUE;
            break;
        }
        else
        {
            // Copy to our username buffer, keeping in mind that we might already have a
            // partial username from an earlier fragment
            outStore[*pMatchDepth + i] = buf[*pNeedleIndex + i];
        }
    }

    // Advance the buffer index so that the next search starts where we finished this one
    *pNeedleIndex += i + 1;

    // Concat with our existing extraction the next time we come through unless we've found the whole string
    if (stringDone)
    {
        *pMatchDepth = 0;
    }
    else
    {
        *pMatchDepth += i;
    }

    return stringDone;
}

// **********************************************************************************************************
// getUsernamePassword - Retrieve the MQTT username and password from the server
// **********************************************************************************************************
static void getUsernamePassword(void)
{
    // NOTE: This function assumes that a socket connection to port 443 on the server is already open
    const uint8 statusString[] = "HTTP/1.* 200 OK\r\n";
    const uint8 userStartToken[] = "\nu:";
    const uint8 passStartToken[] = "\np:";

    bool success;
    
    sint16 bytesRead = -1;
    uint16 startOffset = 0;
    uint16 matchDepth = 0;
    
    uint8 buf[MAX_BUF] = {0};
    uint8 username[MAX_USER_PASS_LEN + 1] = {0};
    uint8 password[MAX_USER_PASS_LEN + 1] = {0};

    bool foundStatusOk = FALSE;
    bool foundUsernameStartToken = FALSE;
    bool foundPasswordStartToken = FALSE;
    bool foundUsername = FALSE;
    bool foundPassword = FALSE;

    // Start SSL w/SNI
    // (the connection to the server on port 443 must already be established)
#ifndef FORCE_INSECURE
    success = SSL_Connect(HOST_FQDN, TRUE) ? CONNECTION_SUCCESS : CONNECTION_SSL_ERROR;
    if (CONNECTION_SUCCESS != success)
    {
        printf("Error during SSL connection setup\n");
        return;
    }
#endif

    // Make HTTP request to auth endpoint 
    printf("Requesting credentials\n");
    sprintf((char*)buf, API_MQTT_AUTH_REQUEST, GetId());
    
    // This write needs to be synchronous, not queued
    writeSerial(buf, strlen((char*)buf));

    // Get response, but give it a somewhat arbitrary time to happen (or else our first read will return no bytes)
    // This is mostly to deal with cellular network latency.
    delay(5000);

    // We're re-using this buffer
    memset(buf, 0, sizeof(buf));

    // Read until we've found username and password or there is no more data.
    // The "no more data" case also covers the situation where there is some sort of error response
    do
    {
        startOffset = 0;

        // Probably want to back the max read size waaaay off for testing to ensure that we can properly handle
        // fragmented reads.
        bytesRead = readSerial(buf, sizeof(buf));
        
        // See if we got a 200 response (good) or something else (bad)
        if (!foundStatusOk)
        {
            foundStatusOk = streamSearch(buf, bytesRead, statusString, &matchDepth, &startOffset);
        }

        // Look for the start of the username in the (possbly fragmented) stream
        // Not an "else", since we might have just found the previous search goal
        if (foundStatusOk && !foundUsernameStartToken)
        {
            foundUsernameStartToken = streamSearch(buf, bytesRead, userStartToken, &matchDepth, &startOffset);
        }

        // Extract the username from the (possibly fragmented) stream
        // Not an "else", since we might have just found the previous search goal
        if (foundUsernameStartToken && !foundUsername)
        {
            foundUsername = streamExtract(buf, username, bytesRead, sizeof(username), &matchDepth, &startOffset);
        }

        // Look for the start of the password
        if (foundUsername && !foundPasswordStartToken)
        {
            foundPasswordStartToken = streamSearch(buf, bytesRead, passStartToken, &matchDepth, &startOffset);
        }

        // Extract the password from the (possibly fragmented) stream
        // Not an "else", since we might have just found the previous search goal
        if (foundPasswordStartToken && !foundPassword)
        {
            foundPassword = streamExtract(buf, password, bytesRead, sizeof(password), &matchDepth, &startOffset);
        }
    }
    while (!foundPassword && bytesRead > 0);

    // Store u/p in globals
    if (foundUsername && foundPassword)
    {
        strncpy(gMqttUsername, (const char*)username, min(sizeof(gMqttUsername), sizeof(username))); //lint !e506
        strncpy(gMqttPassword, (const char*)password, min(sizeof(gMqttPassword), sizeof(password)));  //lint !e506
        gHaveUserPass = TRUE;
        printf("Got credentials\n");
    }
    else
    {
        printf("Couldn't get credentials!\n");
    }

    // We're done with the HTTPS connection. We'll need to open a new secure connection later, on
    // a different port, to the MQTT server
    SSL_Disconnect();
}


// **********************************************************************************************************
// pingTimerStart - Start the MQTT heartbeat (ping) timer
// **********************************************************************************************************

static void pingTimerStart(uint32 seconds)
{
    gPingTimerExpired = FALSE;
    
    gPingTimerId = RTC_oneShotTimer(seconds, pingTimerCallback, gPingTimerId);
}

// **********************************************************************************************************
// mqttEnqueue - Put bytes into a queue for transmitting to the modem
// **********************************************************************************************************

static void mqttEnqueue(TOPIC_ID_t topicId, char* pMessage, uint8_t length, uint8_t retained)
{
    publish_q_t qEntry;

    assert(length <= PUBLISH_PAYLOAD_SIZE);

    // Don't try to publish if we're not connected
    if (!gRun.MqttConnected)
    {
        return;
    }

    qEntry.topicId = topicId;
    memcpy(qEntry.message, pMessage, length);  //lint !e669 !e670 //assert guards from memory overrun
    qEntry.length = length;
    qEntry.retained = retained;

     assert(Q_put(&gPublishQ, &qEntry) == Q_SUCCESS);
}
// **********************************************************************************************************
// PUBLISH_DequeueSerial - Get bytes from the queue and send to the modem
// **********************************************************************************************************

void PUBLISH_WriteSerial(void)
{
    publish_q_t qEntry;

    // Don't try to publish if we're not connected
    if (!gRun.MqttConnected)
    {
        return;
    }

    // Send up to PUBLISH_BUF_SIZE bytes to the modem
    while (Q_get(&gPublishQ, &qEntry) == Q_SUCCESS)
    {
        mqttPublish(gTopic[qEntry.topicId].str, qEntry.message, qEntry.length, qEntry.retained);
    }
}
