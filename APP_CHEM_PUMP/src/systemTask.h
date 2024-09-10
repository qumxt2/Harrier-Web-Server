// systemTask.h

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The header file for the main system task which polls & updates system information

#ifndef SYSTEMTASK_H
#define SYSTEMTASK_H

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "typedef.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************

#define SYSTEM_RUN_REFRESH_FREQ         (100) // Hz
#define SYSTEM_RUN_CALC_DELAY(ms)       ((ms * SYSTEM_RUN_REFRESH_FREQ) / 1000)

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
#define SYSTEM_TASK_INIT_OK                        (0x00)		// Initialization successful
#define SYSTEM_TASK_INIT_ERROR                     (0xFF)		// Initialization error

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************
uint8 SystemInit(void);
void SystemTask(void);
void SYSTEM_ForcePublicationNow(void);
void SYSTEM_enableModbusOrCellular(void);
void SYSTEM_enableBackupTimer(void);
void SYSTEM_StopCommsTimer(void);
void SYSTEM_StopCommsTimer(void);

#endif // SYSTEMTASK_H

