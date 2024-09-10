// main.c

// Copyright 2015-2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// Main - Harrier + Controller

//**********************************************************************************************
// HEADER FILES
// *********************************************************************************************
#include "typedef.h"                    // Compiler specific type definitions
#include "Cpdefine.h"
#include "common.h"                     // Common initialization constants and prototypes 
#include "component.h"                  // Component initialization constants and prototypes 
#include "application.h"                // Application initialization constants and prototypes
#include "debug.h"
#include "version.h"                    // Versioning information accessor prototypes 
#include "syscfgid.h"                   // System Configuration ID prototypes and constants
#include "rtos.h"
#include "watchdog.h"
#include "debug_app.h"
#include "systemTask.h"
#include "eeprom.h"
#include "event_handler.h"
#include "alarms.h"
#include "dvinterface_17G721.h"

//**********************************************************************************************
// CONSTANTS AND MACROS
// *********************************************************************************************

_COPYRIGHT_STR("COPYRIGHT 2006-2017 GRACO INC.");

_MAJOR_VERSION(VER17G721_MAJOR);                            // Set Major Version
_MINOR_VERSION(VER17G721_MINOR);                            // Set Minor Version
_BUILD_VERSION(VER17G721_BUILD);                            // Set Build Version
_BUILD_DATE();                                              // Set Build Date
_BUILD_TIME();                                              // Set Build Time
_SOFTWARE_PARTNO("17G721");                                 // Set Software Part Number

// Component Class set in ComponentInit();

_SYS_SOFTWARE_APP_ID(1);                                    //lint !e19 Set Software App ID 
_SYS_PURPOSE_ID(1);                                         //lint !e19 Set Purpose ID

//**********************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// *********************************************************************************************
void FatalError (sint16 errorcode);         // Fatal Error Trap

// *********************************************************************************************
// TASK DEFINITIONS
// ********************************************************************************************* 

#define NUM_TASKS               21

#define TASK_1_FUNCTION_NAME    RtosIdleTask
#define TASK_1_STACK_SIZE       150//200
#define TASK_1_PRIORITY         TASK_PRIORITY_MAX_NUM
#define TASK_2_FUNCTION_NAME    WatchdogTask
#define TASK_2_STACK_SIZE       150
#define TASK_2_PRIORITY         200
#define TASK_3_FUNCTION_NAME    Serial3ADaemon
#define TASK_3_STACK_SIZE       150
#define TASK_3_PRIORITY         160
#define TASK_4_FUNCTION_NAME    CcaPortalTask
#define TASK_4_STACK_SIZE       900//1000
#define TASK_4_PRIORITY         240
#define TASK_5_FUNCTION_NAME    AutoBroadcastDaemon
#define TASK_5_STACK_SIZE       200
#define TASK_5_PRIORITY         110
#define TASK_6_FUNCTION_NAME    MessageInterpreterDaemon
#define TASK_6_STACK_SIZE       150//200
#define TASK_6_PRIORITY         55
#define TASK_7_FUNCTION_NAME    ICBPDaemon
#define TASK_7_STACK_SIZE       250//300
#define TASK_7_PRIORITY         120
#define TASK_8_FUNCTION_NAME    HeartbeatMonitor
#define TASK_8_STACK_SIZE       150
#define TASK_8_PRIORITY         130
#define TASK_9_FUNCTION_NAME    IDAssertionScript
#define TASK_9_STACK_SIZE       150
#define TASK_9_PRIORITY         140
#define TASK_10_FUNCTION_NAME   CANDrvTxRx
#define TASK_10_STACK_SIZE      200
#define TASK_10_PRIORITY        50
#define TASK_11_FUNCTION_NAME   EEPROMWritingDaemon
#define TASK_11_STACK_SIZE      140
#define TASK_11_PRIORITY        121

// The system task MUST be numbered lower than any tasks that make automatic use
// of DVARs, or else those DVARs might not be initialized properly when the task
// starts. In addition, all tasks making automatic use of DVARs should wait for
// the gDvarResourceReady resource to be available before completing their startup.
#define TASK_12_FUNCTION_NAME   SystemTask
#define TASK_12_STACK_SIZE      800//1000
#define TASK_12_PRIORITY        122

//---------------------------------------------------------------------
//Component Level
// --------------------------------------------------------------------
#define TASK_13_FUNCTION_NAME   ButtonInterfaceTask
#define TASK_13_STACK_SIZE      250
#define TASK_13_PRIORITY        174

// --------------------------------------------------------------------
// Application Level
//---------------------------------------------------------------------
#define TASK_14_FUNCTION_NAME   ScreensTask
#define TASK_14_STACK_SIZE      700//1000
#define TASK_14_PRIORITY        150

#define TASK_15_FUNCTION_NAME   RtcTask
#define TASK_15_STACK_SIZE      800//1000
#define TASK_15_PRIORITY        149

#define TASK_16_FUNCTION_NAME   PumpControlTask
#define TASK_16_STACK_SIZE      450//500
#define TASK_16_PRIORITY        148

#define TASK_17_FUNCTION_NAME   InputDebounceTask
#define TASK_17_STACK_SIZE      450//500
#define TASK_17_PRIORITY        147

#define TASK_18_FUNCTION_NAME   VolumeTask
#define TASK_18_STACK_SIZE      450//500
#define TASK_18_PRIORITY        146

#define TASK_19_FUNCTION_NAME   ModemTask
#define TASK_19_STACK_SIZE      5000//6000
#define TASK_19_PRIORITY        175

#define TASK_20_FUNCTION_NAME   EVENT_HANDLER_Task
#define TASK_20_STACK_SIZE      500//450//500
#define TASK_20_PRIORITY        185

#define TASK_21_FUNCTION_NAME   MODBUS_Task
#define TASK_21_STACK_SIZE      470
#define TASK_21_PRIORITY        198

#include "rtos_gentasks.h" // RTOS task generation information

// *********************************************************************************************
// TASK DEFINITIONS - END
// *********************************************************************************************


//**********************************************************************************************
// PRIVATE FUNCTIONS
// *********************************************************************************************

int main ( void )
{
    sint16 error;

    // Set debug modes
    g_DebugMask |= DBUG_PUMP;

    // Initialize all COMMON hardware and software modules
    error = CommonInit();
    ExceptionHandlerInit();
    if (error < 0)
    {
        FatalError(error);
    }

    // Initialize event handler
    error = EVENT_HANDLER_Init(ALARM_EventCallback);
    if (error < 0)
    {
        FatalError(error);
    } 

    // Initialize EEPROM
    error = EEPROM_Init();
    if (error < 0)
    {
        FatalError(error);
    }

    // Initialize system
    error = SystemInit();
    if (error < 0)
    {
        FatalError(error);
    }
    
    // Initialize all COMPONENT level hardware and software modules
    error = ComponentInit();
    if (error != COMPONENT_INIT_OK)
    {
        FatalError(error);
    }

    // Initialize all APPLICATION level software modules
    error = ApplicationInit();
    if (error != APPLICATION_INIT_OK)
    {
        FatalError(error);
    }

    // Start the Operating System
    CommonStartOS();

    // We should never get here unless the RTOS returns.
    return -1;
}


// *********************************************************************************************

// Fatal Error Trap 

void FatalError (sint16 errorcode)
{
    // Do something based upon errorcode... like print to the GCA Portal?
    for(;;) {}
}
