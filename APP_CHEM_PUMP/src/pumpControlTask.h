// pumpControlTask.h

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Pump Control
// Task.


#ifndef PUMP_CONTROL_TASK_H
#define PUMP_CONTROL_TASK_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"                                // Compiler specific type definitions

typedef enum
{
    METERING_MODE_VOLUME = 0,
    METERING_MODE_TIME,
    METERING_MODE_CYCLES,
    NUMBER_METERING_MODES
} METERING_MODE_t;

typedef enum
{
    PUMP_STATUS_Standby,
    PUMP_STATUS_Run,
    PUMP_STATUS_Lockout_Alarm,
    PUMP_STATUS_Lockout_Remote,
    PUMP_STATUS_Powersave,
    PUMP_STATUS_Lockout_Temperature,
} PUMP_STATUS_t;

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
#define PUMP_CONTROL_TASK_INIT_OK                (0x00)     // Initialization successful
#define PUMP_CONTROL_TASK_INIT_ERROR             (0xFF)     // Initialization error
                    
// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

uint8 pumpControlTaskInit (void);
void PMP_setRunMode(void);
void PMP_setStandbyMode(void);
void PMP_resetStates(void);
uint32 PMP_getCycleProgress(void);
bool PMP_isRunning(void);
bool PMP_isOnCycleRunning(void);
void PMP_primePump(void);
void PMP_calibrate(void(*pFuncCalibrationComplete)());
void PMP_updatePowerSaveMode(void);
uint32 PMP_adjustForPowerSavingsMode(uint32 param);
void PMP_resetPumpTimeout(void);
void PMP_TurnPumpOff(void);
void PMP_DisableSafetyRelay(void);
void PMP_ActivateSafetyRelay(void);
//*******************************************************************************************


#endif
