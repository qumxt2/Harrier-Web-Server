// modbus_map_1.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file defines the modbus map

// *****************************************************************************
// HEADER FILES
// *****************************************************************************
#include "dvseg_17G721_run.h"
#include "dvseg_17G721_setup.h"
#include "modbus.h"
#include "rtos.h"
#include "event_handler.h"
#include "mbmap_callbacks.h"

// *****************************************************************************
// CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
// PRIVATE VARIABLES
// *****************************************************************************

// *****************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

Modbus_MapVar ModbusMap_1[] = {

    // ================================= //
    // ID  (401000-401999)               //
    //================================== //

    // Register,	DVAR,                                                Callback,                   Permissions,    Frame Size
    // Run Read Only Block
    { 401000,		DVA17G721_SS( gRun, Pressure_1_Psi ),                GetPressure_1_Psi,          MBR_READ,       MBR_UINT32},
    { 401002,		DVA17G721_SS( gRun, BatteryMillivolts ),			 NULL,                       MBR_READ,       MBR_UINT32},
    { 401004,		DVA17G721_SS( gRun, PumpStatus ),                    NULL,                       MBR_READ,       MBR_UINT32},
    { 401006,		DVA17G721_SS( gRun, CycleProgress ),                 NULL,                       MBR_READ,       MBR_UINT32},
    { 401008,		DVA17G721_SS( gRun, Total ),                         GetTotal,                   MBR_READ,       MBR_UINT32},
    { 401010,		DVA17G721_SS( gRun, GrandTotal ),					 GetGrandTotal,              MBR_READ,       MBR_UINT32},
    { 401012,		DVA17G721_SS( gRun, AlarmBitfield ),			     NULL,                       MBR_READ,       MBR_UINT32},
    { 401014,		DVA17G721_SS( gRun, RemoteDisableActive ),           NULL,                       MBR_READ,       MBR_UINT32},
    { 401016,		DVA17G721_SS( gRun, TankLevel ),                     GetTankLevel,               MBR_READ,       MBR_UINT32},
    { 401018,		DVA17G721_SS( gRun, AnalogFlowRate ),                GetAnalogFlowRate,          MBR_READ,       MBR_UINT32},

    // Setup Read Only Block
    { 401020,       DVA17G721_SS( gSetup, AlarmAction ),                 NULL,                       MBR_READ,       MBR_UINT32},
    { 401022,		DVA17G721_SS( gSetup, MeteringMode ),                NULL,                       MBR_READ,       MBR_UINT32},
    { 401024,		DVA17G721_SS( gSetup, OnTime ),                      NULL,                       MBR_READ,       MBR_UINT32},
    { 401026,		DVA17G721_SS( gSetup, OffTime ),                     NULL,                       MBR_READ,       MBR_UINT32},
    { 401028,		DVA17G721_SS( gSetup, OnCycles ),                    NULL,                       MBR_READ,       MBR_UINT32},
    { 401030,		DVA17G721_SS( gSetup, OnTimeout ),                   NULL,                       MBR_READ,       MBR_UINT32},
    { 401032,		DVA17G721_SS( gSetup, PowerSaveMode ),               NULL,                       MBR_READ,       MBR_UINT32},
    { 401034,		DVA17G721_SS( gSetup, Units ),                       NULL,                       MBR_READ,       MBR_UINT32},
    { 401036,		0,                                                   GetPressure_1_Offset,       MBR_READ,       MBR_UINT32},
    { 401038,		0,                                                   GetPressure_1_Slope,        MBR_READ,       MBR_UINT32},
    { 401040,		DVA17G721_SS( gSetup, KFactor ),                     NULL,                       MBR_READ,       MBR_UINT32},
    { 401042,		0,                                                   GetDesiredFlowRate,         MBR_READ,       MBR_UINT32},
    { 401044,		0,                                                   GetSoftwareVersion,         MBR_READ,       MBR_UINT32},
    { 401046,		0,                                                   GetHighPressureTrigger,     MBR_READ,       MBR_UINT32},
    { 401048,		0,                                                   GetLowPressureTrigger,      MBR_READ,       MBR_UINT32},
    { 401050,		DVA17G721_SS( gSetup, BatteryWarningTrigger ),       NULL,                       MBR_READ,       MBR_UINT32},
    { 401052,		DVA17G721_SS( gSetup, BatteryShutoffTrigger ),       NULL,                       MBR_READ,       MBR_UINT32},
    { 401054,		DVA17G721_SS( gSetup, Alarm1Trigger ),               NULL,                       MBR_READ,       MBR_UINT32},
    { 401056,		DVA17G721_SS( gSetup, Alarm2Trigger ),               NULL,                       MBR_READ,       MBR_UINT32},
    { 401058,		DVA17G721_SS( gSetup, RemoteOffTrigger ),            NULL,                       MBR_READ,       MBR_UINT32},
    { 401060,		DVA17G721_SS( gSetup, VolumeModeInterval ),          NULL,                       MBR_READ,       MBR_UINT32},   
    { 401062,		0,                                                   GetTankLevelNotifyTrigger,  MBR_READ,       MBR_UINT32},    
    { 401064,		0,                                                   GetTankLevelShutoffTrigger, MBR_READ,       MBR_UINT32},        
    { 401066,		DVA17G721_SS( gSetup, FlowVerifyEnable ),            NULL,                       MBR_READ,       MBR_UINT32},
    { 401068,		DVA17G721_SS( gSetup, FlowVerifyPercentage ),        NULL,                       MBR_READ,       MBR_UINT32},
    { 401070,		0,                                                   GetMaxTankVolume,           MBR_READ,       MBR_UINT32},        
    { 401072,		0,                                                   GetFlowRateLowmASetpoint,   MBR_READ,       MBR_UINT32},        
    { 401074,		0,                                                   GetFlowRateHighmASetpoint,  MBR_READ,       MBR_UINT32},
    { 401076,		DVA17G721_SS( gSetup, AnalogInControl ),             NULL,                       MBR_READ,       MBR_UINT32},
    { 401078,		DVA17G721_SS( gSetup, LowmASetpoint ),               NULL,                       MBR_READ,       MBR_UINT32},
    { 401080,		DVA17G721_SS( gSetup, HighmASetpoint ),              NULL,                       MBR_READ,       MBR_UINT32},        
    
    // Run Write Only Block
    { 401082,		DVA17G721_SS( gRun, PumpStatus ),                    SetPumpStatus,              MBR_READ_WRITE,      MBR_UINT32},
    
    // Setup Write Only Block.  The blocks are read/write so that garbage values are not returned when a write block is read
    { 401084,       DVA17G721_SS( gSetup, AlarmAction ),                 SetAlarmAction,             MBR_READ_WRITE,      MBR_UINT32},
    { 401086,		0,                                                   SetMeteringMode,            MBR_READ_WRITE,      MBR_UINT32},
    { 401088,		DVA17G721_SS( gSetup, OnTime ),                      SetOnTime,                  MBR_READ_WRITE,      MBR_UINT32},
    { 401090,		DVA17G721_SS( gSetup, OffTime ),                     SetOffTime,                 MBR_READ_WRITE,      MBR_UINT32},
    { 401092,		DVA17G721_SS( gSetup, OnCycles ),                    SetOnCycles,                MBR_READ_WRITE,      MBR_UINT32},
    { 401094,		DVA17G721_SS( gSetup, OnTimeout ),                   SetOnTimeout,               MBR_READ_WRITE,      MBR_UINT32},
    { 401096,		DVA17G721_SS( gSetup, PowerSaveMode ),               SetPowerSaveMode,           MBR_READ_WRITE,      MBR_UINT32},
    { 401098,		DVA17G721_SS( gSetup, Units ),                       SetUnits,                   MBR_READ_WRITE,      MBR_UINT32},
    { 401100,		0,                                                   SetPressure_1_Offset,       MBR_READ_WRITE,      MBR_UINT32},
    { 401102,		0,                                                   SetPressure_1_Slope,        MBR_READ_WRITE,      MBR_UINT32},
    { 401104,		DVA17G721_SS( gSetup, KFactor ),                     SetKFactor,                 MBR_READ_WRITE,      MBR_UINT32},
    { 401106,		0,                                                   SetDesiredFlowRate,         MBR_READ_WRITE,      MBR_UINT32},
    { 401108,		0,                                                   SetHighPressureTrigger,     MBR_READ_WRITE,      MBR_UINT32},
    { 401110,		0,                                                   SetLowPressureTrigger,      MBR_READ_WRITE,      MBR_UINT32},
    { 401112,		DVA17G721_SS( gSetup, BatteryWarningTrigger ),       SetBatteryWarningTrigger,   MBR_READ_WRITE,      MBR_UINT32},
    { 401114,		DVA17G721_SS( gSetup, BatteryShutoffTrigger ),       SetBatteryShutoffTrigger,   MBR_READ_WRITE,      MBR_UINT32},
    { 401116,		DVA17G721_SS( gSetup, Alarm1Trigger ),               SetAlarm1Trigger,           MBR_READ_WRITE,      MBR_UINT32},
    { 401118,		DVA17G721_SS( gSetup, Alarm2Trigger ),               SetAlarm2Trigger,           MBR_READ_WRITE,      MBR_UINT32},
    { 401120,		0,                                                   SetRemoteOffTrigger,        MBR_READ_WRITE,      MBR_UINT32},
    { 401122,		DVA17G721_SS( gSetup, VolumeModeInterval ),          SetVolumeModeInterval,      MBR_READ_WRITE,      MBR_UINT32},     
    { 401124,		0,                                                   SetTankNotifyTrigger,       MBR_READ_WRITE,      MBR_UINT32},    
    { 401126,		0,                                                   SetTankShutoffTrigger,      MBR_READ_WRITE,      MBR_UINT32},               
    { 401128,		DVA17G721_SS( gSetup, FlowVerifyEnable ),            SetFlowVerifyEnable,        MBR_READ_WRITE,      MBR_UINT32},
    { 401130,		DVA17G721_SS( gSetup, FlowVerifyPercentage ),        SetFlowVerifyPercentage,    MBR_READ_WRITE,      MBR_UINT32},
    { 401132,		0,                                                   SetFlowRateLowmASetpoint,   MBR_READ_WRITE,      MBR_UINT32},        
    { 401134,		0,                                                   SetFlowRateHighmASetpoint,  MBR_READ_WRITE,      MBR_UINT32},
    { 401136,		DVA17G721_SS( gSetup, AnalogInControl ),             SetAnalogInControl,         MBR_READ_WRITE,      MBR_UINT32},
    { 401138,		DVA17G721_SS( gSetup, LowmASetpoint ),               SetLowmASetpoint,           MBR_READ_WRITE,      MBR_UINT32},
    { 401140,		DVA17G721_SS( gSetup, HighmASetpoint ),              SetHighmASetpoint,          MBR_READ_WRITE,      MBR_UINT32},
    
    // Topics
    { 401142,		0,                                                   SetTotalizerReset,          MBR_WRITE,      MBR_UINT32},
    { 401144,		0,                                                   SetClearAlarmStatus,        MBR_WRITE,      MBR_UINT32},    
    
    // 16 Bit Marathon registers & other expansions.  All future expansions will be tacked onto the end & no longer in any certain order.
    { 401146,		DVA17G721_SS( gRun, BatteryMillivolts ),			 UpperWord,                   MBR_READ,        MBR_UINT16},
    { 401147,		DVA17G721_SS( gRun, BatteryMillivolts ),			 LowerWord,                   MBR_READ,        MBR_UINT16},    
    { 401148,		DVA17G721_SS( gRun, AlarmBitfield ),			     UpperWord,                   MBR_READ,        MBR_UINT16},
    { 401149,		DVA17G721_SS( gRun, AlarmBitfield ),			     LowerWord,                   MBR_READ,        MBR_UINT16},
    { 401150,		DVA17G721_SS( gRun, TankHeightCalc ),     	         UpperWord,                   MBR_READ,        MBR_UINT16},
    { 401151,		DVA17G721_SS( gRun, TankHeightCalc ),                LowerWord,                   MBR_READ,        MBR_UINT16},    
    { 401152,		DVA17G721_SS( gSetup, KFactor ),                     UpperWord,                   MBR_READ,        MBR_UINT16},
    { 401153,		DVA17G721_SS( gSetup, KFactor ),                     LowerWord,                   MBR_READ,        MBR_UINT16},
    { 401154,		DVA17G721_SS( gRun, PumpStatus ),                    UpperWord,                   MBR_READ,        MBR_UINT16},
    { 401155,		DVA17G721_SS( gRun, PumpStatus ),                    LowerWord,                   MBR_READ,        MBR_UINT16},
    { 401156,		0,                                                   GetGrandTotalU16,            MBR_READ,        MBR_UINT16},        
    { 401157,       0,                                                   SetDesiredFlowRate,          MBR_READ_WRITE,  MBR_UINT16},
    { 401158,       0,                                                   SetPumpStatusInhibit,        MBR_READ_WRITE,  MBR_UINT16},
    { 401159,       0,                                                   SetClearAlarmStatus,         MBR_READ_WRITE,  MBR_UINT16},
    { 401160,       0,                                                   UpdateHearbeatTick,          MBR_READ_WRITE,  MBR_UINT16},
    { 401161,       0,                                                   SetModbusCommsEnable,        MBR_READ_WRITE,  MBR_UINT16},
    { 401162,       DVA17G721_SS( gRun, TankHeightNotifyTrigger ),       SetTankHeightNotifyTrigger,  MBR_READ_WRITE,  MBR_UINT16},
    { 401163,       DVA17G721_SS( gRun, TankHeightShutoffTrigger ),      SetTankHeightShutoffTrigger, MBR_READ_WRITE,  MBR_UINT16},
    // Analog Output Read
    { 401164,		DVA17G721_SS( gSetup, AnalogOutControl ),            NULL,                        MBR_READ,        MBR_UINT32},    
    { 401166,		DVA17G721_SS( gSetup, AoutMinOut ),                  NULL,                        MBR_READ,        MBR_UINT32},
    { 401168,		DVA17G721_SS( gSetup, AoutMaxOut ),                  NULL,                        MBR_READ,        MBR_UINT32},
    { 401170,		DVA17G721_SS( gSetup, AoutOutputType ),              NULL,                        MBR_READ,        MBR_UINT32},        
    // Analog Output Write
    { 401172,		DVA17G721_SS( gSetup, AnalogOutControl ),            NULL,                        MBR_READ_WRITE,  MBR_UINT32},    
    { 401174,		DVA17G721_SS( gSetup, AoutMinOut ),                  NULL,                        MBR_READ_WRITE,  MBR_UINT32},
    { 401176,		DVA17G721_SS( gSetup, AoutMaxOut ),                  NULL,                        MBR_READ_WRITE,  MBR_UINT32},    
    // Temperature Probe Read
    { 401178,		DVA17G721_SS( gRun, Temperature ),                   GetTemperature,              MBR_READ,        MBR_UINT32},    
    { 401180,		DVA17G721_SS( gSetup, TempControl ),                 NULL,                        MBR_READ,        MBR_UINT32},
    { 401182,		DVA17G721_SS( gSetup, TempSetpoint ),                GetTemperatureSetpoint,      MBR_READ,        MBR_UINT32},    
    // Temperature Probe Write 
    { 401184,		0,                                                   SetTemperatureControl,       MBR_READ_WRITE,  MBR_UINT32},
    { 401186,		0,                                                   SetTemperatureSetpoint,      MBR_READ_WRITE,  MBR_UINT32},    
    // 16 Bit XTO Registers
    { 401188,		0,                                                   GetDesiredFlowRate,          MBR_READ,        MBR_UINT16},
    { 401189,		DVA17G721_SS( gRun, TankLevel ),                     UpperWord,                   MBR_READ,        MBR_UINT16},
    { 401190,		DVA17G721_SS( gRun, TankLevel ),                     LowerWord,                   MBR_READ,        MBR_UINT16},
    { 401191,		DVA17G721_SS( gRun, Pressure_1_Psi ),                GetPressure_1_Psi,           MBR_READ,        MBR_UINT16},
    { 401192,		DVA17G721_SS( gSetup, OnTime ),                      LowerWord,                   MBR_READ,        MBR_UINT16},
    { 401193,		DVA17G721_SS( gSetup, OffTime ),                     LowerWord,                   MBR_READ,        MBR_UINT16},   
    { 401194,		DVA17G721_SS( gSetup, OnTime ),                      SetOnTime,                   MBR_READ_WRITE,  MBR_UINT16}, // 65,535 sec max = 18h, 12m, 15s
    { 401195,		DVA17G721_SS( gSetup, OffTime ),                     SetOffTime,                  MBR_READ_WRITE,  MBR_UINT16}, // 65,535 sec max = 18h, 12m, 15s

    { 0xffffffff, 0, NULL, MBR_READ, MBR_UINT16}
};

