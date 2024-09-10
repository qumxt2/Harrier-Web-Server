// dvinterface_17G721.h

// Copyright 2013
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// DESCRIPTION
 
#ifndef DVINTERFACE_17G721_H
#define DVINTERFACE_17G721_H

/*******************************************************************************
*   HEADERS
*******************************************************************************/

#include "component.h" // so the following check will appropriately pass/fail
#ifdef ADM_GRAPHICS_PROCESSOR
	#include "ipcTypedef.h"
#else
	#include "dvar.h"
#endif // ADM_GRAPHICS_PROCESSOR

/*******************************************************************************
*   SYSCFGID
*******************************************************************************/

#define SCID17G721_CMP (0x10)
#define SCID17G721_APP (4)
#define SCID17G721_PUR (0)
#define VER17G721_MAJOR (1)
#define VER17G721_MINOR (2)
#define VER17G721_BUILD (14)

#define SCID17G721_32 ( ((uint32)SCID17G721_CMP << 24) + \
                        ((uint32)SCID17G721_APP <<  8) + \
                        ((uint32)SCID17G721_PUR <<  0) )


/*******************************************************************************
*   CONSTANTS
*******************************************************************************/
#define NUM_LEVEL_CHART_ROWS    (20)
/*******************************************************************************
*   GLOBAL VARIABLES
*******************************************************************************/
extern uint8 gDvarLockoutResID;

/*******************************************************************************
*   DVAR ADDRESSES
*******************************************************************************/

#define DVA17G721_APP_BASE  (0x00F00000)

#define DVA17G721_gRun_BASE (DVA17G721_APP_BASE)
#define DVA17G721_gRun_LEN (0x100)

#define DVA17G721_gSetup_BASE (DVA17G721_gRun_BASE + DVA17G721_gRun_LEN)
#define DVA17G721_gSetup_LEN (0x100)

#define DVA17G721_gLevelChart_BASE (DVA17G721_gSetup_BASE + DVA17G721_gSetup_LEN)
#define DVA17G721_gLevelChart_LEN (0x100)

#define DVA17G721_gUsage_BASE (DVA17G721_gLevelChart_BASE + DVA17G721_gLevelChart_LEN)
#define DVA17G721_gUsage_LEN  (0x100)

#define DVA17G721_APP_LAST (DVA17G721_gUsage_BASE + DVA17G721_gUsage_LEN)

/*******************************************************************************
*   DVAR ADDRESS MACROS
*******************************************************************************/

#define DVA17G721_SS(seg, mem) \
                                    ( DVA17G721_##seg##_BASE + \
                                      offsetof(DVS17G721_##seg##_t, mem) / 4 )
// example for single var in single struct
// DVAR_SetPointRemote_ACK(
//        DVA17G721_SS(setup,systemGfbEnabled_bool),
//        TRUE,
//        100);

#define DVA17G721_AS(seg, index, mem) \
                                    ( DVA17G721_##seg##_BASE + \
                                      sizeof(DVS17G721_##seg##_t)/4 * (index) + \
                                      offsetof(DVS17G721_##seg##_t, mem) / 4 )
/*
#define DVA17G721_AS(seg, index, mem) \
                                    ( DVA17G721_##seg##_BASE + \
                                      DVA17G721_##seg##_LEN * (index) + \
                                      offsetof(DVS17G721_##seg##_t, mem) / 4 )
*/
// example for single var in array struct
// (potlife timeout in fourth recipe segment)
// DVAR_SetPointRemote_ACK(
//        DVA17G721_AS(recipe,3,potlifeTimeout_min_u32),
//        60,
//        100);


#define DVA17G721_SA(seg, mem, index) \
                                    ( DVA17G721_##seg##_BASE + \
                                      offsetof(DVS17G721_##seg##_t, mem) / 4 + \
                                      (index) )
// example for array var in single struct
// (third usb log period in setup segment)
// DVAR_SetPointRemote_ACK(
//        DVA17G721_SA(setup,usbLogPeriod_sec_u32,2),
//        60,
//        100);

#define DVA17G721_AA(seg, index1, mem, index2) \
                                    ( DVA17G721_##seg##_BASE + \
                                      sizeof(DVS17G721_##seg##_t)/4 * (index1) + \
                                      offsetof(DVS17G721_##seg##_t, mem) / 4 + \
                                      (index2) )
// example for array var in array struct
// (second flush time in first flush segment)
// DVAR_SetPointRemote_ACK(
//        DVA17G721_AA(flush,0,time_s_u32,1),
//        20,
//        100);

typedef struct DVS17G721_run_t
{
    DistVarType Pressure_1_Psi;
    DistVarType BatteryMillivolts;
    DistVarType PumpStatus;
    DistVarType ConnectionStatus;
    DistVarType IpAddress;
    DistVarType CycleProgress;
    DistVarType Total;
    DistVarType GrandTotal;
    DistVarType AlarmBitfield;
    DistVarType MqttConnected;
    DistVarType RemoteDisableActive;
    DistVarType ModemFound;
    DistVarType TankLevel;
    DistVarType Pressure_2_Psi;
    DistVarType TankHeightCalc;
    DistVarType AnalogFlowRate;
    DistVarType AnalogInmV;
    DistVarType refPsiInProcess;
    DistVarType ModbusHeartbeatTick;
    DistVarType TankHeightNotifyTrigger;
    DistVarType TankHeightShutoffTrigger;
    DistVarType TestOverride;
    DistVarType Temperature;
    DistVarType TempProbeDisableActive;
    DistVarType MaxMotorCurrentReadingmV;
    DistVarType AverageMotorCurrentReadingmV;
} DVS17G721_gRun_t;

// The following DVARS will exist in EEPROM backed RAM.
typedef struct DVS17G721_setup_t
{
    DistVarType magicNumber_u32;
    DistVarType MeteringMode;
    DistVarType OnTime;
    DistVarType OffTime;
    DistVarType OnCycles;
    DistVarType OnTimeout;
    DistVarType PowerSaveMode;
    DistVarType Units;
    DistVarType Pressure_1_Offset;
    DistVarType Pressure_1_Slope;
    DistVarType KFactor;
    DistVarType DesiredFlowRate;
    DistVarType TotalBackup;
    DistVarType GrandTotalBackup;
    DistVarType Location; 
    DistVarType SoftwareVersion;
    DistVarType NetworkMode;
    DistVarType ScreenPin;
    DistVarType ScreenPinEnable;
    DistVarType HighPressureTrigger;
    DistVarType LowPressureTrigger;
    DistVarType BatteryWarningTrigger;
    DistVarType BatteryShutoffTrigger;
    DistVarType Alarm1Trigger;
    DistVarType Alarm2Trigger;
    DistVarType RemoteOffTrigger;
    DistVarType SystemPublicationPeriod;
    DistVarType ModbusSlaveID;
    DistVarType ModbusBaudrateIndex;
    DistVarType VolumeModeInterval;
	DistVarType AlarmAction;
    DistVarType TankType;
    DistVarType TankLevelNotifyTrigger;
    DistVarType TankLevelShutoffTrigger;
    DistVarType FluidDensity;
    DistVarType FlowVerifyEnable;
    DistVarType FlowVerifyPercentage;    
    DistVarType Pressure_2_Offset;
    DistVarType Pressure_2_Slope;
    DistVarType TankFillHeight;
    DistVarType TankFillPressure;
    DistVarType TankFillVoltage;
    DistVarType TankFillVolume;
    DistVarType TankSensorHeight;
    DistVarType TankSensorVolume;
    DistVarType TankSensorPosition;
    DistVarType TankSetupComplete;
    DistVarType MaxTankVolume;
    DistVarType PressureVSHeightRadius_x10000;
    DistVarType FlowRateLowmASetpoint;
    DistVarType FlowRateHighmASetpoint;
    DistVarType AnalogInControl;
    DistVarType HighmASetpoint;
    DistVarType LowmASetpoint;
    DistVarType ModbusParity;
    DistVarType ModbusStopBits;
    DistVarType ModbusCommsEnable;
    DistVarType AnalogOutControl;
    DistVarType AoutOutputType;
    DistVarType AoutMinOut;
    DistVarType AoutMaxOut;
    DistVarType TempControl;
    DistVarType TempSetpoint;
    DistVarType MtrProtectionEnabled;
    DistVarType MtrVoltage;
    DistVarType MtrSelection;
    DistVarType MaxMtrCurrentmV;
    DistVarType CycleSwitchEnable;
    DistVarType RpmNameplate;
} DVS17G721_gSetup_t;

// The following DVARS will exist in EEPROM backed RAM.
typedef struct DVS17G721_TankLevel_t
{
    DistVarType TankLevel[NUM_LEVEL_CHART_ROWS];
    DistVarType TankVolume[NUM_LEVEL_CHART_ROWS];
} DVS17G721_gLevelChart_t;

typedef struct DVS17G721_gUsage_t
{
    DistVarType PumpStatusBackup;
} DVS17G721_gUsage_t;

#endif
