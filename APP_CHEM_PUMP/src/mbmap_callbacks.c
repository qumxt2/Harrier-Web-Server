// mbmap_callbacks.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains all of the modbus map callbacks
//
// Note: Write callbacks need to support polling for most customer applications.  They tend to 
// write to registers once a second, so we need to be careful to only do things on change, 
// especially any resets or state changes.

// *****************************************************************************
// HEADER FILES
// *****************************************************************************

#include "dvseg_17G721_run.h"
#include "dvseg_17G721_setup.h"
#include "pumpControlTask.h"
#include "PublishSubscribe.h"
#include "systemTask.h"
#include "alarms.h"
#include "volumeTask.h"
#include "CountDigit.h"
#include "AlarmScreenControl.h"
#include "AdvancedScreen.h"
#include "utilities.h"
#include "AnalogInFlowScreen.h"
#include "modbus.h"
#include "limits.h"
#include "FlowScreen.h"
#include "utilities.h"
#include "AlarmScreenTemp.h"
#include "TankScreen.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************
#define INVALID_VALUE       (0xFFFFFFFF)

// *****************************************************************************
// PUBLIC FUNCTIONS
// *****************************************************************************
uint32 SetPumpStatus ( bool set, uint32 Val, void *MbMapElement )
{    
    if (set)
    {
        if (Val != gRun.PumpStatus)
        {
            if (Val == 1)
            {
                PMP_setRunMode();
            }
            else if (Val == 0)
            {
                PMP_setStandbyMode();
            }
            else
            {
                // Force standby mode
                return 0;
            }
        }
    }
    return gRun.PumpStatus;
}

uint32 SetPumpStatusInhibit ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            // Marathon's inhibit state: 1 = alarm state, so "inhibit" pump (go to standby)
            if (Val == 1)
            {
                PMP_setStandbyMode();
            }
            else if (Val == 0)
            {
                PMP_setRunMode();
            }
            else
            {
                return 0;
            }
        }
        oldVal = Val;
    }
    return gRun.PumpStatus;
}

uint32 SetPowerSaveMode ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.PowerSaveMode;
    }
}

uint32 SetMeteringMode ( bool set, uint32 Val, void *MbMapElement )
{
    const bool keepPumpRunning = PMP_isRunning();

    if (set)
    {
        if (Val != gSetup.MeteringMode)
        {
            PMP_setStandbyMode();
            (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, MeteringMode), Val);
            PMP_resetStates();

            // If pump was running, start it running again
            if (keepPumpRunning)
            {
                PMP_setRunMode();
            }
        }
        return Val;
    }
    else
    {
        return gSetup.MeteringMode;
    }   
}

uint32 SetOnTime ( bool set, uint32 Val, void *MbMapElement )
{
    const bool keepPumpRunning = PMP_isRunning();

    if (set)
    {
        if (Val != gSetup.OnTime)
        {
            PMP_resetStates();

            // If pump was running, start it running again
            if (keepPumpRunning)
            {
                PMP_setRunMode();
            }
        }
        return Val;
    }
    else
    {
        return gSetup.OnTime;
    }
}

uint32 SetOffTime ( bool set, uint32 Val, void *MbMapElement )
{
    const bool keepPumpRunning = PMP_isRunning();

    if (set)
    {
        if (Val != gSetup.OffTime)
        {
            PMP_resetStates();

            // If pump was running, start it running again
            if (keepPumpRunning)
            {
                PMP_setRunMode();
            }
        }
        return Val;
    }
    else
    {
        return gSetup.OffTime;
    }
}

uint32 SetOnCycles ( bool set, uint32 Val, void *MbMapElement )
{
    const bool keepPumpRunning = PMP_isRunning();

    if (set)
    {
        if (Val != gSetup.OnCycles)
        {
            PMP_resetStates();

            // If pump was running, start it running again
            if (keepPumpRunning)
            {
                PMP_setRunMode();
            }
        }
        return Val;
    }
    else
    {
        return gSetup.OnCycles;
    }
}

uint32 SetOnTimeout ( bool set, uint32 Val, void *MbMapElement )
{
    const bool keepPumpRunning = PMP_isRunning();

    if (set)
    {
        if (Val != gSetup.OnTimeout)
        {
            PMP_resetStates();

            // If pump was running, start it running again
            if (keepPumpRunning)
            {
                PMP_setRunMode();
            }
        }
        return Val;
    }
    else
    {
        return gSetup.OnTimeout;
    }
}

uint32 SetUnits ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.Units;
    }
}

uint32 GetDesiredFlowRate( bool set, uint32 Val, void *MbMapElement )
{
    return getLocalFlowRate(gSetup.DesiredFlowRate);
}

uint32 SetDesiredFlowRate ( bool set, uint32 Val, void *MbMapElement )
{
    const bool keepPumpRunning = PMP_isRunning();

    if (set)
    {
        if(gSetup.AnalogInControl != AIN_FLOW_RATE) //only allow the flow to be set when not in the analog flow control mode
        {    
            // Must convert to gallons before checking to see if it has changed or not
            if (gSetup.Units == UNITS_METRIC)
            {
                Val = Val*10;   //need to have 2 decimal places
                Val = litersToGallons(Val);
            }
            
            // Using DVAR instead of static variable to see if it has changed because kfactor can limit what gets written & if they
            // change the kfactor, they also need to change what they write for desired flow rate & change it back if static variable is used
            
            if (Val != gSetup.DesiredFlowRate)
            {
                // Purposely converted to gallons first so we can limit check in gallons
                if (Val <= MIN_FLOW_RATE_GAL)
                {
                    Val = MIN_FLOW_RATE_GAL;
                }
                else if (Val >= MAX_FLOW_RATE_GAL)
                {
                    Val = MAX_FLOW_RATE_GAL;
                }
                
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, DesiredFlowRate), Val);
                VOL_resetVolumeThreshold();

                // If pump was running, start it running again
                if (keepPumpRunning)
                {
                    PMP_setRunMode();
                }
            }
        }
    }
    return getLocalFlowRate(gSetup.DesiredFlowRate);
}

uint32 GetHighPressureTrigger (bool set, uint32 Val, void *MbMapElement )
{
    return (getLocalPressure(gSetup.HighPressureTrigger));
}

uint32 SetHighPressureTrigger ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            if(gSetup.Units == UNITS_METRIC)
            {
                Val = barToPsi(Val);
            }
            (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, HighPressureTrigger), Val);
        }
        oldVal = Val;
    }
    return (getLocalPressure(gSetup.HighPressureTrigger));
}

uint32 GetLowPressureTrigger (bool set, uint32 Val, void *MbMapElement )
{
    return (getLocalPressure(gSetup.LowPressureTrigger));
}

uint32 SetLowPressureTrigger ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {        
            if(gSetup.Units == UNITS_METRIC)
            {
                Val = barToPsi(Val);
            }
            (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, LowPressureTrigger), (DistVarType)Val);
        }
        oldVal = Val;
    }
    return (getLocalPressure(gSetup.LowPressureTrigger));
}

uint32 SetBatteryWarningTrigger ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.BatteryWarningTrigger;
    }
}

uint32 SetBatteryShutoffTrigger ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.BatteryShutoffTrigger;
    }
}

uint32 SetAlarm1Trigger ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.Alarm1Trigger;
    }
}

uint32 SetAlarm2Trigger ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.Alarm2Trigger;
    }
}

uint32 SetRemoteOffTrigger ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        if (Val != gSetup.RemoteOffTrigger)
        {
            (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, RemoteOffTrigger), Val);

            if(Val == LOGIC_DISABLED)
            {
                (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, RemoteDisableActive), FALSE);
                PMP_setRunMode();
            }
        }
    }
    return gSetup.RemoteOffTrigger;
}

uint32 SetVolumeModeInterval ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.VolumeModeInterval;
    }
}

uint32 SetKFactor ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        if (Val == 0)
        {
            Val = 1;
        }
        
        if (Val > 9999)
        {
            Val = 9999;
        }

        return Val;
    }
    else
    {
        return gSetup.KFactor;
    }
}

uint32 SetTotalizerReset ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            if (TRUE == Val)
            {
                resetTotal();
            }
        }
        oldVal = Val;
    }
    return Val;
}

uint32 SetClearAlarmStatus ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            if (TRUE == Val)
            {
                (void)ALARM_CancelAll();
            }
        }
        oldVal = Val;
    }
    return Val;
}
 
uint32 UpdateHearbeatTick (bool set, uint32 Val, void *MbMapElement )
{
    // Always update the heartbeat tick count every time this register is written, regardless of what's written
    
    if (set)
    {
        gRun.ModbusHeartbeatTick++;

        // Limited to a 16 bit value because it's a 16 bit register
        if (gRun.ModbusHeartbeatTick >= USHRT_MAX)
        {
            gRun.ModbusHeartbeatTick = 0;
        }
    }
    return gRun.ModbusHeartbeatTick;
}

uint32 GetSoftwareVersion ( bool set, uint32 Val, void *MbMapElement )
{
    uint32 Version = 0;
  
    // returned as xxyyzz where xx.yy.zz = major.minor.build
    Version = (VER17G721_MAJOR * 10000) + (VER17G721_MINOR * 100) + VER17G721_BUILD;

    return Version;
}

uint32 GetPressure_1_Psi ( bool set, uint32 Val, void *MbMapElement )
{
    return (getLocalPressure(gRun.Pressure_1_Psi));
}

uint32 GetPressure_1_Offset ( bool set, uint32 Val, void *MbMapElement )
{
    return (FixedPointToInteger(gSetup.Pressure_1_Offset, DECIMAL_PLACE_TWO));
}

uint32 SetPressure_1_Offset ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            uint32 ValFP = IntegerToFixedPoint(Val, DECIMAL_PLACE_TWO);
            (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, Pressure_1_Offset), ValFP);
        }
        oldVal = Val;
    }
    return (FixedPointToInteger(gSetup.Pressure_1_Offset, DECIMAL_PLACE_TWO));
}

uint32 GetPressure_1_Slope ( bool set, uint32 Val, void *MbMapElement )
{
    return (FixedPointToInteger(gSetup.Pressure_1_Slope, DECIMAL_PLACE_TWO));
}

uint32 SetPressure_1_Slope ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            uint32 ValFP = IntegerToFixedPoint(Val, DECIMAL_PLACE_TWO);
            (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, Pressure_1_Slope), ValFP);
        }
        oldVal = Val;
    }
    return (FixedPointToInteger(gSetup.Pressure_1_Slope, DECIMAL_PLACE_TWO));
}

uint32 SetAlarmAction ( bool set, uint32 Val, void *MbMapElement )
{
    const bool keepPumpRunning = PMP_isRunning();

    if (set)
    {
        if (Val != gSetup.AlarmAction)
        {
            (void)ALARM_CancelAll();

            // If pump was running, start it running again
            if (keepPumpRunning)
            {
                PMP_setRunMode();
            }
        }
        return Val;
    }
    else
    {
        return gSetup.AlarmAction;
    }
}

uint32 GetTankLevelNotifyTrigger ( bool set, uint32 Val, void *MbMapElement )
{
    return (getLocalVolume(gSetup.TankLevelNotifyTrigger));
}

uint32 SetTankNotifyTrigger ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            if (gSetup.Units == UNITS_METRIC)
            {
                // Check inputs before conversion so it doesn't overflow while converting L to G
                if (Val > MAX_VOLUME_L)
                {
                    Val = MAX_VOLUME_L;
                }
            }
            else
            {
                if (Val > MAX_VOLUME_G)
                {
                    Val = MAX_VOLUME_G;
                }
            }
            // Values are sent x10 to match scaling on digit boxes on screens & we x10 again before storing in dvar
            Val = setLocalVolume(Val);
            (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TankLevelNotifyTrigger), Val);
        }
        oldVal = Val;
    }
    return getLocalVolume(gSetup.TankLevelNotifyTrigger);
}

uint32 GetTankLevelShutoffTrigger ( bool set, uint32 Val, void *MbMapElement )
{
    return (getLocalVolume(gSetup.TankLevelShutoffTrigger));    
}

uint32 SetTankHeightNotifyTrigger (bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            if (gSetup.Units == UNITS_METRIC)
            {
                // All values sent via modbus are sent as xxx.xx *100 regardless of units, so no need to x10 here like we do for strap chart
                Val = centimetersToInches(Val);
            }
            if (Val > TANK_HEIGHT_MAX)
            {
                Val = TANK_HEIGHT_MAX;
            }
            else if (Val < TANK_HEIGHT_MIN)
            {
                Val = TANK_HEIGHT_MIN;
            }
            (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, TankHeightNotifyTrigger), (DistVarType)Val);
        }
        oldVal = Val;
    }
    return gRun.TankHeightNotifyTrigger;
}

uint32 SetTankHeightShutoffTrigger (bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            if (gSetup.Units == UNITS_METRIC)
            {
                // All values sent via modbus are sent as xxx.xx *100 regardless of units, so no need to x10 here like we do for strap chart
                Val = centimetersToInches(Val);
            }
            if (Val > TANK_HEIGHT_MAX)
            {
                Val = TANK_HEIGHT_MAX;
            }
            else if (Val < TANK_HEIGHT_MIN)
            {
                Val = TANK_HEIGHT_MIN;
            }
            (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, TankHeightShutoffTrigger), (DistVarType)Val);
        }
        oldVal = Val;
    }
    return gRun.TankHeightShutoffTrigger;
}

uint32 SetTankShutoffTrigger ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            if(gSetup.Units == UNITS_METRIC)
            {
                // Check inputs before conversion so it doesn't overflow while converting L to G
                if (Val > MAX_VOLUME_L)
                {
                    Val = MAX_VOLUME_L;
                }
            }
            else
            {
                if (Val > MAX_VOLUME_G)
                {
                    Val = MAX_VOLUME_G;
                }
            }
            // Values are sent x10 to match scaling on digit boxes on screens & we x10 again before storing in dvar
            Val = setLocalVolume(Val);
            (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TankLevelShutoffTrigger), Val);
        }
        oldVal = Val;
    }
    return getLocalVolume(gSetup.TankLevelShutoffTrigger);    
}

uint32 SetFlowVerifyEnable ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.FlowVerifyEnable;    
    }
}

uint32 SetModbusCommsEnable ( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            if (Val == TRUE)
            {
                gSetup.ModbusCommsEnable = Val;
                SYSTEM_StartCommsTimer();
            }
            else if (Val == FALSE)
            {
                gSetup.ModbusCommsEnable = Val;
                SYSTEM_StopCommsTimer();
            }
        }
        oldVal = Val;
    }
    return gSetup.ModbusCommsEnable;
}

uint32 SetFlowVerifyPercentage ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.FlowVerifyPercentage;    
    }
}

uint32 GetFlowRateLowmASetpoint( bool set, uint32 Val, void *MbMapElement )
{
    return (getLocalFlowRate(gSetup.FlowRateLowmASetpoint));
}

uint32 SetFlowRateLowmASetpoint ( bool set, uint32 Val, void *MbMapElement )
{
    const bool keepPumpRunning = PMP_isRunning();
    
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            setLocalFlowRate(Val, DVA17G721_SS(gSetup, FlowRateLowmASetpoint));
            PMP_resetStates();

            // If pump was running, start it running again
            if (keepPumpRunning)
            {
                PMP_setRunMode();
            }
        }
        oldVal = Val;
    }
    return getLocalFlowRate(gSetup.FlowRateLowmASetpoint);  
}

uint32 GetFlowRateHighmASetpoint( bool set, uint32 Val, void *MbMapElement )
{
    return (getLocalFlowRate(gSetup.FlowRateHighmASetpoint));
}

uint32 SetFlowRateHighmASetpoint ( bool set, uint32 Val, void *MbMapElement )
{
    const bool keepPumpRunning = PMP_isRunning();
    
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    
    if (set)
    {
        if (Val != oldVal)
        {
            setLocalFlowRate(Val, DVA17G721_SS(gSetup, FlowRateHighmASetpoint));
            PMP_resetStates();

            // If pump was running, start it running again
            if (keepPumpRunning)
            {
                PMP_setRunMode();
            }
        }
        oldVal = Val;
    }
    return getLocalFlowRate(gSetup.FlowRateHighmASetpoint); 
}
uint32 SetAnalogInControl ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        return Val;
    }
    else
    {
        return gSetup.AnalogInControl;    
    }
}
uint32 SetLowmASetpoint ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        Val = clampValue(Val, gSetup.HighmASetpoint-1, MA_MIN_SETTING);  // the setpoints cannot be equal, hence the -1 , .01mA
        return Val;
    }
    else
    {
        return gSetup.LowmASetpoint;    
    }
}

uint32 SetHighmASetpoint ( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        Val = clampValue(Val, MA_MAX_SETTING, gSetup.LowmASetpoint+1);  // the setpoints cannot be equal, hence the +1 , .01mA
        return Val;
    }
    else
    {
        return gSetup.HighmASetpoint;    
    }
}

uint32 GetTotal ( bool set, uint32 Val, void *MbMapElement )
{
    return(getLocalVolume(gRun.Total));
}
        
uint32 GetGrandTotal ( bool set, uint32 Val, void *MbMapElement )
{
    return(getLocalVolume(gRun.GrandTotal));
}

uint32 GetGrandTotalU16 ( bool set, uint32 Val, void *MbMapElement )
{   
    uint32 grandTotal = (getLocalTotalizer(gRun.GrandTotal));
    grandTotal = (grandTotal % 32767);
    return grandTotal;
}

uint32 GetTankLevel ( bool set, uint32 Val, void *MbMapElement )
{
    return(getLocalVolume(gRun.TankLevel));
}

uint32 GetAnalogFlowRate( bool set, uint32 Val, void *MbMapElement )
{
    return getLocalFlowRate(gRun.AnalogFlowRate);
}

uint32 GetMaxTankVolume( bool set, uint32 Val, void *MbMapElement )
{
    return getLocalVolume(gSetup.MaxTankVolume);
}

uint32 UpperWord ( bool set, uint32 Val, void *MbMapElement )
{
    DVarSearchContext   dvarSearchResult;
    Modbus_MapVar *pModbusMapEl = MbMapElement;
    dvarSearchResult.id = pModbusMapEl->DvarId;
    (void)DVAR_SeekLocalVariable( &dvarSearchResult, dvarSearchResult.id );
    return dvarSearchResult.value >> 16;
}

uint32 LowerWord ( bool set, uint32 Val, void *MbMapElement )
{
    DVarSearchContext   dvarSearchResult;
    Modbus_MapVar *pModbusMapEl = MbMapElement;
    dvarSearchResult.id = pModbusMapEl->DvarId;
    (void)DVAR_SeekLocalVariable( &dvarSearchResult, dvarSearchResult.id );
    return dvarSearchResult.value & 0xFFFF;
}

uint32 GetTemperature( bool set, uint32 Val, void *MbMapElement )
{
    return ((uint32)getLocalTemperature(gRun.Temperature));
}

uint32 GetTemperatureSetpoint( bool set, uint32 Val, void *MbMapElement )
{
    return ((uint32)getLocalTemperature(gSetup.TempSetpoint));
}

uint32 SetTemperatureSetpoint( bool set, uint32 Val, void *MbMapElement )
{
    static uint32 oldVal = INVALID_VALUE;      // Default to an invalid or unlikely value so it will be processed first time through
    sint32 sVal = Val;
 
    if (set)
    {
        if (Val != oldVal)
        {
            if (gSetup.Units == UNITS_METRIC)
            {
                sVal = celsiusToFahrenheit(sVal);
            }
            if ((sVal >= MIN_DEG_F) && (sVal <= MAX_DEG_F))
            {                
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TempSetpoint), (uint32)sVal);
            }
        }
        oldVal = Val;
    }
    return ((uint32)getLocalTemperature(gSetup.TempSetpoint));
}

uint32 SetTemperatureControl( bool set, uint32 Val, void *MbMapElement )
{
    if (set)
    {
        if (Val != gSetup.TempControl)
        {
            // Make sure the "disabled by temperature" is cleared if it was just disabled or set to display only
            if((Val == TEMP_CONTROL_DISABLED) || (Val == TEMP_CONTROL_DISPLAY))
            {
                (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, TempProbeDisableActive), FALSE);
                PMP_setRunMode();
            }
            if(Val < NUMBER_TEMP_CONTROL_MODES)
            {
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TempControl), Val);
            }
        }
    }
    return ((uint32)getLocalTemperature(gSetup.TempControl));
}

