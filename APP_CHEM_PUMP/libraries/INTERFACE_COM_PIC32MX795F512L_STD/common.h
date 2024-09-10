// common.h
 
// Copyright 2006
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// DESCRIPTION

#ifndef COMMON_H
#define COMMON_H

// ***************************************************
// * CONSTANTS
// ***************************************************
// This is the first address location that the component level
//  software is allowed to use.  Everything below this address is
//  reserved for common software modules.
#define	COMPONENT_FIRST_EEADDR		(0x1000)

#define FREEPIN_BUTTON_S1				(0x01)		// Pin 1 - RG15
#define FREEPIN_FORCE_SYSTEM_RESET		(0x02)		// Pin 17 - RA0
#define FREEPIN_POWER_GOOD				(0x04)		// Pin 38 - RA1
#define FREEPIN_TOKEN_CS_AND_LOFO		(0x08)		// Pin 39 - RF13 and Pin 40 - RF12

typedef enum {
    ExceptionInfo_Uninitialized = 0,
    ExceptionInfo_AlreadyRead = 0xAEDBCF01,
    ExceptionInfo_NewException = 0xBDEAFC02
} ExceptionInfo_t;

// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************
sint16 __attribute__((nomips16)) CommonInit (void); 		// initialize COMMON level resources and tasks
sint16 __attribute__((nomips16)) CommonInit_FreePins (uint8 freepins_mask); 		// initialize COMMON level resources and tasks
void   CommonStartOS (void) __attribute__ ((noreturn)); 	// Start the Operating System

// Call ExeptionHandlerInit() in "startup_pre_datainit()" to enable exception handler
// This function should only be used for development as calling it adds to the
// application code size.
void ExceptionHandlerInit( void );

// GetExceptionInfo tells the application whether or not the most reset was due to an exception.
// If logging exceptions is desired, the application should call it once at power-up and record its
// return value.
//
// Return values:
//      ExceptionInfo_Uninitialized - An exception has not occurred.
//      ExceptionInfo_AlreadyRead - An exception has occurred but has already been reported
//          to the application by a previous call to GetExceptionInfo. This will happen if an
//          exception happens and is reported, then the processor is reset by a different
//          mechanism and GetExceptionInfo is called again. It will also happen if
//          GetExceptionInfo is called more than once; only the first call will report the
//          exception.
//      ExceptionInfo_NewException - An exception has occurred and caused a reset, and has not
//          been reported by GetExceptionInfo yet.
ExceptionInfo_t GetExceptionInfo( void );

// Get the exception code of the most recent exception.
// If GetExceptionInfo returns ExceptionInfo_Uninitialized this value has no meaning.
uint32 GetExceptionCode( void );

// Get the program memory address of the instruction that caused the most recent exception.
// The reported address can be up to 4 bytes away from the correct address.
// If GetExceptionInfo returns ExceptionInfo_Uninitialized this value has no meaning.
uint32 GetExceptionAddress( void );

#endif // COMMON_H
