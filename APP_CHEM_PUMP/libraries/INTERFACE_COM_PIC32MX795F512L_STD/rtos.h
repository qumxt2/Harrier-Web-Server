//! \file rtos.h
//! \brief The Real-Time Operating System.
//! 
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved


#ifndef RTOS_H
#define RTOS_H

//! \addtogroup gca_rtos
//@{

#include "typedef.h"

#include "cpfuncs.h"


// *************************************************
// * TYPEDEFS
// *************************************************
typedef struct
{
	uint8 errorid;
} rtos_errormsg_t;


// *************************************************
// * CONSTANTS
// *************************************************
/// Return value for successful call to ::RtosInit
#define RTOS_INIT_NO_ERROR		(0x00)
/// Return value for failed call to ::RtosInit
#define RTOS_INIT_ERROR			(0x01)

/// Invalid value for many RTOS functions.  Often used to indicate a failed function call.
#define RTOS_INVALID_ID			(0xFF)

/// Bit mask for RTOS event flag #1
#define RTOS_EVENT_FLAG_1		(0x01)
/// Bit mask for RTOS event flag #2
#define RTOS_EVENT_FLAG_2		(0x02)
/// Bit mask for RTOS event flag #3
#define RTOS_EVENT_FLAG_3		(0x04)
/// Bit mask for RTOS event flag #4
#define RTOS_EVENT_FLAG_4		(0x08)
/// Bit mask for RTOS event flag #5
#define RTOS_EVENT_FLAG_5		(0x10)
/// Bit mask for RTOS event flag #6
#define RTOS_EVENT_FLAG_6		(0x20)
/// Bit mask for RTOS event flag #7
#define RTOS_EVENT_FLAG_7		(0x40)
/// Bit mask for RTOS event flag #8
#define RTOS_EVENT_FLAG_8		(0x80)

#define RTOS_CLEAR_EVENT_FLAGS_NEVER		(0)
#define RTOS_CLEAR_EVENT_FLAGS_BEFORE		(1)
#define RTOS_CLEAR_EVENT_FLAGS_AFTER		(2)
#define RTOS_CLEAR_EVENT_FLAGS_BOTH			(3)

#define RTOS_NOTIFY_SPECIFIC				(0)
#define RTOS_NOTIFY_HIGHEST_PRI				(1)
#define RTOS_NOTIFY_WAITING_HIGHEST_PRI		(2)
#define RTOS_NOTIFY_ALL						(3)
#define RTOS_NOTIFY_ALL_WAITING				(4)
#define RTOS_NOTIFY_ALL_SPECIFIC_PRI		(5)
#define RTOS_NOTIFY_ALL_WAITING_SPEC_PRI	(6)

// *************************************************
// * FUNCTION PROTOTYPES
// *************************************************

//----------------------------------------------------------------------------
//! \brief	Initialize the Real-Time Operating System
//!
//! \return
//!	\li	::RTOS_INIT_NO_ERROR if successful.
//!	\li	otherwise ::RTOS_INIT_ERROR.
//!
//! \note This function does not actually start the RTOS.
//! \sa RtosGo
//----------------------------------------------------------------------------
uint8 RtosInit (void);

//----------------------------------------------------------------------------
//! \brief	Start the Real-Time Operating System
//!
//! \note The RTOS takes control with this function call and never returns. \n
//! \note The RTOS must be initialized before calling this function.
//! \sa RtosInit
//----------------------------------------------------------------------------
void RtosGo (void) __attribute__ ((noreturn));		

//----------------------------------------------------------------------------
//! \brief	Dump the current state of the RTOS to the serial port
//!
//! \note This function is intended to be used in startup_pre_datainit() 
//----------------------------------------------------------------------------
void RtosStateDump (void);

//----------------------------------------------------------------------------
//! \brief	Dump the current state of the RTOS to the CCA Debug Portal
//!
//! \note This function is intended to be used with the \ref gca_portal 
//----------------------------------------------------------------------------
void RtosStateDumpCCAportal (void);

//----------------------------------------------------------------------------
//! \brief	Get the frequency of the RTOS tick
//!
//! \return The RTOS Tick Frequency (in Hertz)
//----------------------------------------------------------------------------
uint16 RtosGetTickFreq (void);

//----------------------------------------------------------------------------
//! \brief	Get the period of an RTOS tick
//!
//! \return The RTOS Tick period (in microseconds)
//----------------------------------------------------------------------------
uint16 RtosGetTickPeriod (void);

// Get TaskID number of task pointed to by task_addr
//   The function will return TaskID if found. 
//   A return value of RTOS_INVALID_ID indicates the task was not found
uint8 RtosTaskGetID (void (*task_addr)(void));

// Create the task pointed to by task_addr
//   The function will return TaskID if succesful. 
//   A return value of RTOS_INVALID_ID indicates an error occured
uint8 RtosTaskCreate (void (*task_addr)(void));

// Create AND Start the task pointed to by task_addr
//   The function will return TaskID if succesful. 
//   A return value of RTOS_INVALID_ID indicates an error occured
uint8 RtosTaskCreateStart (void (*task_addr)(void));


// reserve next available resource ID
//   a return value of RTOS_INVALID_ID indicates error
uint8 RtosResourceReserveID (void);

// reserve next available semaphore ID
//   a return value of RTOS_INVALID_ID indicates error
uint8 RtosSemaphoreReserveID (void);

// reserve next available cyclic timer ID
//   a return value of RTOS_INVALID_ID indicates error
uint8 RtosTimerReserveID (void);

// reserve next available mailbox ID
//   a return value of RTOS_INVALID_ID indicates error
uint8 RtosMailboxReserveID (void);


// This function will tell the RTOS where to send error messages.  
// The RTOS will format these messages using the typedef 'rtos_errormsg_t'
//  a return value equal to mailboxid input parameter indicates success.
//  a return value of RTOS_INVALID_ID indicates failure
void RtosSetErrorHandlerMailboxID (uint8 mailboxid);

// Set the idle task to monitor the stack usage
void RtosIdleTaskSetUsage_StackMonitor( void );

 // Set the idle task to monitor the CPU utilization rate
void RtosIdleTaskSetUsage_CpuUtilization( void );

// Returns a value indicating the number of RTOS ticks that have occurred since
// last power up.
uint64 RtosGetRawTickCount( void );

uint8 GetNumResourcesReserved( void );
uint8 GetNumSemaphoresReserved( void );
uint8 GetNumTimersReserved( void );
uint8 GetNumMailboxesReserved( void );
word16 GetMinimumFreeMessages( void );

//@}

#endif // RTOS_H
