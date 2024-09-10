// mbmap_callbacks.h

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The header file for all of the modbus map callbacks

#ifndef MBMAP_CALLBACKS_H
#define	MBMAP_CALLBACKS_H

// *****************************************************************************
// HEADER FILES
// *****************************************************************************
#include "dvinterface_17G721.h"

// *****************************************************************************
// CONSTANTS
// *****************************************************************************

// *****************************************************************************
// GLOBAL VARIABLES
// *****************************************************************************

// *****************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************
uint32 SetPumpStatus ( bool set, uint32 Val, void *MbMapElement );
uint32 SetPowerSaveMode ( bool set, uint32 Val, void *MbMapElement );
uint32 SetMeteringMode ( bool set, uint32 Val, void *MbMapElement );
uint32 SetOnTime ( bool set, uint32 Val, void *MbMapElement );
uint32 SetOffTime ( bool set, uint32 Val, void *MbMapElement );
uint32 SetOnCycles ( bool set, uint32 Val, void *MbMapElement );
uint32 SetOnTimeout ( bool set, uint32 Val, void *MbMapElement );
uint32 SetPowerSaveMode ( bool set, uint32 Val, void *MbMapElement );
uint32 SetUnits ( bool set, uint32 Val, void *MbMapElement );
uint32 SetKFactor ( bool set, uint32 Val, void *MbMapElement );
uint32 GetDesiredFlowRate ( bool set, uint32 Val, void *MbMapElement );
uint32 SetDesiredFlowRate ( bool set, uint32 Val, void *MbMapElement );
uint32 GetHighPressureTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetHighPressureTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 GetLowPressureTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetLowPressureTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetBatteryWarningTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetBatteryShutoffTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetAlarm1Trigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetAlarm2Trigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetRemoteOffTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetVolumeModeInterval ( bool set, uint32 Val, void *MbMapElement );
uint32 SetSystemPublicationPeriod ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTotalizerReset ( bool set, uint32 Val, void *MbMapElement );
uint32 SetClearAlarmStatus ( bool set, uint32 Val, void *MbMapElement );
uint32 GetSoftwareVersion (bool set, uint32 Val, void *MbMapElement );
uint32 GetPressure_1_Psi ( bool set, uint32 Val, void *MbMapElement );
uint32 GetPressure_1_Offset ( bool set, uint32 Val, void *MbMapElement );
uint32 SetPressure_1_Offset ( bool set, uint32 Val, void *MbMapElement );
uint32 GetPressure_1_Slope ( bool set, uint32 Val, void *MbMapElement );
uint32 SetPressure_1_Slope ( bool set, uint32 Val, void *MbMapElement );
uint32 SetAlarmAction ( bool set, uint32 Val, void *MbMapElement );

uint32 GetTankFullPsiCal ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTankType ( bool set, uint32 Val, void *MbMapElement );
uint32 GetTankLevelNotifyTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTankNotifyTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 GetTankLevelShutoffTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTankShutoffTrigger ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTankLength ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTankHeight ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTankWidth ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTankDiameter ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTankFullPsiCal ( bool set, uint32 Val, void *MbMapElement );
uint32 SetFlowVerifyEnable ( bool set, uint32 Val, void *MbMapElement );
uint32 SetFlowVerifyPercentage ( bool set, uint32 Val, void *MbMapElement );
uint32 GetFlowRateLowmASetpoint ( bool set, uint32 Val, void *MbMapElement );
uint32 SetFlowRateLowmASetpoint ( bool set, uint32 Val, void *MbMapElement );
uint32 GetFlowRateHighmASetpoint ( bool set, uint32 Val, void *MbMapElement );
uint32 SetFlowRateHighmASetpoint ( bool set, uint32 Val, void *MbMapElement );
uint32 SetAnalogInControl ( bool set, uint32 Val, void *MbMapElement );
uint32 SetLowmASetpoint ( bool set, uint32 Val, void *MbMapElement );
uint32 SetHighmASetpoint ( bool set, uint32 Val, void *MbMapElement );

uint32 GetTotal ( bool set, uint32 Val, void *MbMapElement );
uint32 GetGrandTotal ( bool set, uint32 Val, void *MbMapElement );
uint32 GetTankLevel ( bool set, uint32 Val, void *MbMapElement );
uint32 GetAnalogFlowRate ( bool set, uint32 Val, void *MbMapElement );
uint32 GetMaxTankVolume ( bool set, uint32 Val, void *MbMapElement );

uint32 UpperWord ( bool set, uint32 Val, void *MbMapElement );
uint32 LowerWord ( bool set, uint32 Val, void *MbMapElement );
uint32 GetGrandTotalUpper ( bool set, uint32 Val, void *MbMapElement );
uint32 GetGrandTotalLower ( bool set, uint32 Val, void *MbMapElement );
uint32 GetTankLevelUpper ( bool set, uint32 Val, void *MbMapElement );
uint32 GetTankLevelLower ( bool set, uint32 Val, void *MbMapElement );
uint32 GetDesiredFlowRateUpper( bool set, uint32 Val, void *MbMapElement );
uint32 GetDesiredFlowRateLower( bool set, uint32 Val, void *MbMapElement );
uint16 SetDesiredFlowRateU16 ( bool set, uint16 Val, void *MbMapElement );
uint16 SetKFactorU16 ( bool set, uint16 Val, void *MbMapElement );
uint16 SetPumpStatusU16 ( bool set, uint16 Val, void *MbMapElement );
uint32 SetPumpStatusInhibit ( bool set, uint32 Val, void *MbMapElement );
uint32 GetGrandTotalU16 ( bool set, uint32 Val, void *MbMapElement );
uint32 SetModbusCommsEnable ( bool set, uint32 Val, void *MbMapElement );
uint32 SetTankHeightNotifyTrigger (bool set, uint32 Val, void *MbMapElement );
uint32 SetTankHeightShutoffTrigger (bool set, uint32 Val, void *MbMapElement );
uint32 UpdateHearbeatTick (bool set, uint32 Val, void *MbMapElement );
uint32 GetTemperature( bool set, uint32 Val, void *MbMapElement );
uint32 GetTemperatureSetpoint( bool set, uint32 Val, void *MbMapElement );
uint32 SetTemperatureSetpoint( bool set, uint32 Val, void *MbMapElement );
uint32 SetTemperatureControl( bool set, uint32 Val, void *MbMapElement );

#endif	/* MBMAP_CALLBACKS_H */

