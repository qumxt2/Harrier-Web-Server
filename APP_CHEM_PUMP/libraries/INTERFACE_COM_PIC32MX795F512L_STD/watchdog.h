// watchdog.h
 
// Copyright 2006
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// DESCRIPTION

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "typedef.h"


// *************************************************
// * TYPEDEFS
// *************************************************
typedef struct
{
	uint8 errorid;
} watchdog_to_icbp_errormsg_t;

typedef struct
{
	uint8 errorid;
} watchdog_to_ccaportal_errormsg_t;


// *************************************************
// * CONSTANTS
// *************************************************
#define WATCHDOG_INIT_NO_ERROR				(0)
#define WATCHDOG_INIT_ERROR					(1)

// BKC - 6/26/08 - Setting a watchdog error with error ID
// WATCHDOG_HARDWARE_FAILURE_ERROR_ID will result in all other errors being
// ignored, and the red LED staying on constantly.  This error may be cleared
// and any previously queued errors will resume blinking operation.  This
// error does not occupy a spot in the error buffer.
// This modification was a result of fixing bug #2437.
#define WATCHDOG_HARDWARE_FAILURE_ERROR_ID	(0)

#define WATCHDOG_REGISTRATION_ERROR			(0xFF)

#define WATCHDOG_ERRORID_SET				(0)		// return value for "no error"
#define WATCHDOG_ERRORID_CLEARED			(0)		// return value for "no error"
#define WATCHDOG_ERRORID_ALREADY_SET		(1)
#define WATCHDOG_ERRORID_ALREADY_CLEAR		(1)
#define WATCHDOG_ERRORID_INVALID			(2)
#define WATCHDOG_ERRORID_BUFFER_FULL		(3)


// *************************************************
// * FUNCTION PROTOTYPES
// *************************************************
// initialize Watchdog library - the Watchdog timer is automatically
//  enabled when the Watchdog task first runs.
uint8 WatchdogInit	(void);		

// disable the Watchdog
void WatchdogDisable (void);

// enable the Watchdog
void WatchdogEnable (void);

// Ask the Watchdog library for a critical flag ID 
uint8 WatchdogRegisterCriticalFlag (void);

// Tell the Watchdog task to set criticalflagID (cast a vote for watchdog reset)
void WatchdogSetCriticalFlag (uint8 criticalflagID); 

// Set an error condition - possible return values are defined below
//  RED indicator will flash code according to errorid variable.
// RETURN VALUES:
//  - WATCHDOG_ERRORID_SET if successful
//  - WATCHDOG_ERRORID_ALREADY_SET if errorid already exists in fault buffer
//  - WATCHDOG_ERRORID_INVALID if errorid contains any '0's (in decimal
//     representation) with the exception of '0' which will turn the red LED
//     on solid to indicate a hardware error.
//  - WATCHDOG_ERRORID_BUFFER_FULL if unable to set error condition
//     due to full buffer.  
uint8 WatchdogSetError (uint8 errorid);

// Clear an error condition - possible return values are defined below
// RETURN VALUES:
//  - WATCHDOG_ERRORID_CLEARED if successful
//  - WATCHDOG_ERRORID_ALREADY_CLEAR if errorid does not exist in fault buffer
//  - WATCHDOG_ERRORID_INVALID if errorid contains any '0's (in decimal
//     representation) with the exception of '0' which will turn the red LED
//     on solid to indicate a hardware error.
uint8 WatchdogClearError (uint8 errorid);

// Record an error condition, but do not give any visible indication
//  of error.
void WatchdogRecordSilentError (uint8 errorid);


// This function will dump the current value of the critical flags register
//  to the CCA Debug Portal.  There's no harm in using this function at any
//  time, but it is really meant to be used during the startup_pre_datainit
//  function to help troubleshoot the cause of a watchdog reset...
void WatchdogDumpCriticalFlags (void);

#endif // WATCHDOG_H
