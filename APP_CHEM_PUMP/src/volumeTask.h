// volumeTask.h

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Volume
// Task.   


#ifndef VOLUME_TASK_TASK_H
#define VOLUME_TASK_TASK_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"
#include "dvinterface_17G721.h"                                // Compiler specific type definitions


// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
#define VOLUME_TASK_INIT_OK                 (0x00)      // Initialization successful
#define VOLUME_TASK_INIT_ERROR              (0xFF)      // Initialization error
#define VOLUME_TASK_TIMER_INVALID           (0xFF)
#define PRESSURE_MONITOR_CYCLE_TIME         (5u)
#define MV_PER_MA                           (100)
#define DEFAULT_PUMP_RPM                    (60u)

typedef void (*volumeCb_t) (uint8 unused);

typedef struct
{
    bool            isEnabled;
    bool            isRunning;
    uint32          value;
    uint32          threshold;
    volumeCb_t      cb;
} watcher_t;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

uint8 volumeTaskInit (void);
void VOL_createVolumeWatcher(uint32 desiredFlowRate, volumeCb_t cb);
void VOL_resetVolumeWatcher(uint32 desiredFlowRate);
void VOL_stopVolumeWatcher(void);
void VOL_startVolumeWatcher(void);
uint32 VOL_getVolumeTimeRemaining(void);
void VOL_createCountWatcher(uint32 desiredFlowRate, volumeCb_t cb);
void VOL_resetCountWatcher(uint32 desiredFlowRate);
void VOL_stopCountWatcher(void);
void VOL_startCountWatcher(void);
uint32 VOL_getCountsRemaining(void);
uint32 VOL_getVolumePercentProgress(void);
uint32 VOL_getCountsPercentProgress(void);
uint32 VOL_getVolumeModeOnTime(void);
void VOL_BackupTotals(void);
void VOL_updateKfactor(uint32 volume);
uint32 VOL_getCycleInterval(void);
void VOL_resetVolumeThreshold(void);

void VOL_fakeReedSwitch(bool enabled);
void resetTotal(void);
void UpdateAnalogInput(void);
void UpdateAnalogOutput(void);
void VOL_updateRpm(uint32 newRpm);

//*******************************************************************************************


#endif
