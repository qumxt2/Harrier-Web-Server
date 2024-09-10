//! \file	timebase.h
//! \brief Module for DCM/ADCM timebases that are used for "high-speed" digital i/o
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_adcm
//@{

//! \addtogroup gca_adcm_timebase		DCM/ADCM timebases
//! \brief Advanced Display Control Module timebases for high-speed
//!        digital I/O
//!
//! \b DESCRIPTION:
//!    This module provides an interface for working with the
//!    high-speed timebases on the DCM/ADCM.

#ifndef TIMEBASE_H
#define TIMEBASE_H

//! \ingroup gca_adcm_timebase
//@{

// Include basic platform types
#include "typedef.h"
	
	
// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************
typedef uint32 mS_u32d0_t;
typedef uint32 mS_u16d16_t;
typedef uint32 uS_u32d0_t;
typedef uint32 uS_u16d16_t;
typedef uint32 nS_u32d0_t;


typedef enum
{
	TIMEBASE_1 = 0,	// Used by RTOS
	TIMEBASE_2,		// can be combined with TIMEBASE_3 to create 32-bit timebase
	TIMEBASE_3,			// when configured as 32-bit timebase, access is thru TIMEBASE_2
	TIMEBASE_4,			// can be combined with TIMEBASE_5 to create 32-bit timebase
	TIMEBASE_5,			// when configured as 32-bit timebase, access is thru TIMEBASE_4

	NUM_TIMEBASES,
	TIMEBASE_INVALID = NUM_TIMEBASES
} TIMEBASE_ID_t;



// ***************************************************
// * MACROS
// ***************************************************


// ***************************************************
// * PUBLIC FUCTION PROTOTYPES
// ***************************************************

///----------------------------------------------------------------------------
//! \brief Initialzation / reset function for TIMEBASE.
//!
//! NOTES:
//!   This function must be called by the application layer for a TIMEBASE
//!   before using any of the functionality of this module on that TIMEBASE
//!
//! \param timebase_id The TIMEBASE to initialize
//-----------------------------------------------------------------------------
void TIMEBASE_Initialize( TIMEBASE_ID_t timebase_id );


///----------------------------------------------------------------------------
//! \brief Get function for TIMEBASE input clock
//!
//! \param timebase_id The TIMEBASE to get the input clock of.
//!
//! \return -1 if unable to determine input clock frequency
//!         otherwise frequency in Hz of TIMEBASE input clock
//-----------------------------------------------------------------------------
sint32 TIMEBASE_ClkIN_Freq_Get( TIMEBASE_ID_t timebase_id );

///----------------------------------------------------------------------------
//! \brief Get function for TIMEBASE prescaler
//!
//! NOTES:
//!   Acceptable values for prescale on DCM/ADCM are: 1, 8, 64 or 256
//!
//! \param timebase_id The TIMEBASE to get the prescaler of
//!
//! \return prescale value for TIMEBASE.
//-----------------------------------------------------------------------------
uint16 TIMEBASE_Prescale_Get( TIMEBASE_ID_t timebase_id );

///----------------------------------------------------------------------------
//! \brief Set function for TIMEBASE prescaler
//!
//! NOTES:
//!   Acceptable values for prescale on DCM/ADCM are: 1, 8, 64 or 256
//!
//! \param timebase_id The TIMEBASE to to set the prescaler of
//! \param prescale The new prescaler value.  Acceptable values are 1, 8, 64, or 256
//!
//! \return actual prescale value for TIMEBASE.  If error check is desired, 
//!    then the return value should be compared against desired value
//----------------------------------------------------------------------------
uint16 TIMEBASE_Prescale_Set( TIMEBASE_ID_t timebase_id, uint16 prescale );

///----------------------------------------------------------------------------
//! \brief Get function for TIMEBASE output clock
//!
//! NOTES:
//!   Output Clock of TIMEBASE is defined as input clock / prescale value
//!
//! \param timebase_id The TIMEBASE get the output clock of.
//!
//! \return -1 if unable to determine out clock frequency
//!         otherwise frequency in Hz of TIMEBASE output clock
//-----------------------------------------------------------------------------
sint32 TIMEBASE_ClkOUT_Freq_Get( TIMEBASE_ID_t timebase_id );


///----------------------------------------------------------------------------
//! \brief Get function for TIMEBASE period register
//!
//! NOTES:
//!   When enabled, the TIMEBASE counter will repeatedly 
//!   count up to the value in the period register.
//!
//! \param timebase_id The TIMEBASE to get the period of.
//!
//! \return value of TIMEBASE period register
//-----------------------------------------------------------------------------
uint16 TIMEBASE_Period_Get( TIMEBASE_ID_t timebase_id );

///----------------------------------------------------------------------------
//! \brief Set function for TIMEBASE Period
//!
//! \param timebase_id The TIMEBASE to set the period of
//! \param count value to set TIMEBASE Period to
//-----------------------------------------------------------------------------
void TIMEBASE_Period_Set( TIMEBASE_ID_t timebase_id, uint16 period );


///----------------------------------------------------------------------------
//! \brief Get function for TIMEBASE counter register
//!
//! NOTES:
//!   When enabled, the TIMEBASE counter will repeatedly 
//!   count up to the value in the period register.
//!
//! \param timebase_id The TIMEBASE to get the counter of
//!
//! \return value of TIMEBASE counter register
//-----------------------------------------------------------------------------
uint16 TIMEBASE_Count_Get( TIMEBASE_ID_t timebase_id );

///----------------------------------------------------------------------------
//! \brief Set function for TIMEBASE Counter
//!
//! \param timebase_id The TIMEBASE to set the counter of
//! \param count value to set TIMEBASE Counter to
//-----------------------------------------------------------------------------
void TIMEBASE_Count_Set( TIMEBASE_ID_t timebase_id, uint16 count );


///----------------------------------------------------------------------------
//! \brief Get function for TIMEBASE state (enabled or disabled)
//!
//! \param timebase_id The TIMEBASE to get the state of.
//!
//! \return TRUE if enabled, otherwise FALSE
//-----------------------------------------------------------------------------
bool TIMEBASE_State_Get( TIMEBASE_ID_t timebase_id );

///----------------------------------------------------------------------------
//! \brief Set function for TIMEBASE state (enabled or disabled)
//!
//! \param timebase_id The TIMEBASE to set the state of
//! \param state TRUE to enable TIMEBASE, FALSE to disable
//-----------------------------------------------------------------------------
void TIMEBASE_State_Set( TIMEBASE_ID_t timebase_id, bool state );


///----------------------------------------------------------------------------
//! \brief Enable or disable linked 32-bit timer operation
//!
//! \param timebase_id The TIMEBASE on which to enable or disable 32-bit operation
//! \param state TRUE to enable 32-bit operation, FALSE to disable
//!
//! NOTES:
//!   - The TIMEBASE ID must be TIMEBASE_2, or TIMEBASE_4.
//!     Calling this function on any other timebase will have no effect
//-----------------------------------------------------------------------------
void TIMEBASE_32bit_Mode_Set( TIMEBASE_ID_t timebase_id, bool enable );

///----------------------------------------------------------------------------
//! \brief Get enabled/disabled state of linked 32-bit timer operation
//!
//! \param timebase_id The TIMEBASE on which to get state of
//!
//! \return TRUE if enabled, otherwise FALSE
//!
//! NOTES:
//!   - The TIMEBASE ID must be TIMEBASE_2, or TIMEBASE_4.
//!     Calling this function on any other timebase will return FALSE
//-----------------------------------------------------------------------------
bool TIMEBASE_32bit_Mode_Get( TIMEBASE_ID_t timebase_id );

///----------------------------------------------------------------------------
//! \brief Get function for TIMEBASE prescaler
//!
//! NOTES:
//!   Acceptable values for prescale on DCM/ADCM are: 1, 8, 64 or 256
//!
//! \param timebase_id The TIMEBASE to get the prescaler of
//!
//! \return prescale value for TIMEBASE.
//-----------------------------------------------------------------------------
uint16 TIMEBASE_32bit_Prescale_Get( TIMEBASE_ID_t timebase_id );

///----------------------------------------------------------------------------
//! \brief Set function for TIMEBASE prescaler
//!
//! NOTES:
//!   Acceptable values for prescale on DCM/ADCM are: 1, 2, 4, 8, 16, 64 or 256
//!
//! \param timebase_id The TIMEBASE to to set the prescaler of
//! \param prescale The new prescaler value.  Acceptable values are 1, 2, 4, 8, 16, 64 or 256
//!
//! \return actual prescale value for TIMEBASE.  If error check is desired, 
//!    then the return value should be compared against desired value
//----------------------------------------------------------------------------
uint16 TIMEBASE_32bit_Prescale_Set( TIMEBASE_ID_t timebase_id, uint16 prescale );

///----------------------------------------------------------------------------
//! \brief Get function for TIMEBASE counter register
//!
//! NOTES:
//!   When enabled, the TIMEBASE counter will repeatedly 
//!   count up to the value in the period register.
//!
//! \param timebase_id The TIMEBASE to get the counter of
//!
//! \return value of TIMEBASE counter register
//!
//! NOTES:
//!   - The TIMEBASE ID must be TIMEBASE_2,  or TIMEBASE_4.
//!     Calling this function on any other timebase will return a value of 0
//-----------------------------------------------------------------------------
uint32 TIMEBASE_32bit_Count_Get( TIMEBASE_ID_t timebase_id );

///----------------------------------------------------------------------------
//! \brief Set function for TIMEBASE Counter
//!
//! \param timebase_id The TIMEBASE to set the counter of
//! \param count value to set TIMEBASE Counter to
//!
//! NOTES:
//!   - The TIMEBASE ID must be TIMEBASE_2, or TIMEBASE_4.
//!     Calling this function on any other timebase will have no effect
//-----------------------------------------------------------------------------
void TIMEBASE_32bit_Count_Set( TIMEBASE_ID_t timebase_id, uint32 count );


///----------------------------------------------------------------------------
//! \brief Get function for TIMEBASE period register
//!
//! NOTES:
//!   When a TIMEBASE is enabled, the TIMEBASE counter will repeatedly 
//!   count up to the value in the period register.
//!
//! \param timebase_id The TIMEBASE to get the period of.
//!
//! \return value of TIMEBASE period register
//!
//! NOTES:
//!   - The TIMEBASE ID must be TIMEBASE_2, or TIMEBASE_4.
//!     Calling this function on any other timebase will return a value of 0
//-----------------------------------------------------------------------------
uint32 TIMEBASE_32bit_Period_Get( TIMEBASE_ID_t timebase_id );

///----------------------------------------------------------------------------
//! \brief Set function for TIMEBASE Period
//!
//! \param timebase_id The TIMEBASE to set the period of
//! \param count value to set TIMEBASE Period to
//!
//! NOTES:
//!   - The TIMEBASE ID must be TIMEBASE_2, or TIMEBASE_4.
//!     Calling this function on any other timebase will have no effect
//-----------------------------------------------------------------------------
void TIMEBASE_32bit_Period_Set( TIMEBASE_ID_t timebase_id, uint32 period );


///----------------------------------------------------------------------------
//! \brief Register an RTOS Task/Event to be notified on a TIMEBASE #1
//!   period match (when TIMEBASE_1_Count reaches TIMEBASE_1_Period)
//!
//! \param task_id The specific RTOS Task ID to send the event trigger to
//! \param eventmask The event information to send to the RTOS Task
//!
//! NOTES:
//!   - The TIMEBASE ID is incorporated into the function name because of the
//!     way the interrupt handler needs to be linked into the final application.
//!   - To "Unregister" the event, call this function with a task_id of RTOS_INVALID_ID
//-----------------------------------------------------------------------------
void TIMEBASE_2_Period_RegisterEvent( uint8 task_id, uint8 eventmask );

///----------------------------------------------------------------------------
//! \brief Register an RTOS Task/Event to be notified on a TIMEBASE #2
//!   period match (when TIMEBASE_2_Count reaches TIMEBASE_2_Period)
//!
//! \param task_id The specific RTOS Task ID to send the event trigger to
//! \param eventmask The event information to send to the RTOS Task
//!
//! NOTES:
//!   - The TIMEBASE ID is incorporated into the function name because of the
//!     way the interrupt handler needs to be linked into the final application.
//!   - To "Unregister" the event, call this function with a task_id of RTOS_INVALID_ID
//-----------------------------------------------------------------------------
void TIMEBASE_3_Period_RegisterEvent( uint8 task_id, uint8 eventmask );


///----------------------------------------------------------------------------
//! \brief Convert TIMEBASE count value into milliseconds (mS_u32d0 format)
//!
//! \param timebase_id The TIMEBASE to use for the conversion
//! \param count The count value to convert
//!
//! \return the time in milliseconds based on the settings of the TIMEBASE 
//!   return value is in mS_u32d0 format
//!
//! NOTES:
//!   If the return value is not able to fit into the 32-bit return variable,
//!   then the return value will be coerced to ULONG_MAX.  If there is any possibility
//!   that an overflow will occur, then it is the APPLICATION programmer's responsibility
//!   to compare the return value against ULONG_MAX before accepting the result.
//-----------------------------------------------------------------------------
mS_u32d0_t TIMEBASE_Convert_to_milliSec_u32d0( TIMEBASE_ID_t timebase_id, uint32 count );


///----------------------------------------------------------------------------
//! \brief Convert TIMEBASE count value into milliseconds (mS_u16d16 format)
//!
//! \param timebase_id The TIMEBASE to use for the conversion
//! \param count The count value to convert
//!
//! \return the time in milliseconds based on the settings of the TIMEBASE 
//!   return value is in mS_u16d16 format (most significant 16 bits representing
//!   whole milliseconds, least significant 16 bits representing fractional mS)
//!
//! NOTES:
//!   If the return value is not able to fit into the 32-bit return variable,
//!   then the return value will be coerced to ULONG_MAX.  If there is any possibility
//!   that an overflow will occur, then it is the APPLICATION programmer's responsibility
//!   to compare the return value against ULONG_MAX before accepting the result.
//-----------------------------------------------------------------------------
mS_u16d16_t TIMEBASE_Convert_to_milliSec_u16d16( TIMEBASE_ID_t timebase_id, uint32 count );


///----------------------------------------------------------------------------
//! \brief Convert TIMEBASE count value into milliseconds (uS_u32d0 format)
//!
//! \param timebase_id The TIMEBASE to use for the conversion
//! \param count The count value to convert
//!
//! \return the time in milliseconds based on the settings of the TIMEBASE 
//!   return value is in uS_u32d0 format.
//!
//! NOTES:
//!   If the return value is not able to fit into the 32-bit return variable,
//!   then the return value will be coerced to ULONG_MAX.  If there is any possibility
//!   that an overflow will occur, then it is the APPLICATION programmer's responsibility
//!   to compare the return value against ULONG_MAX before accepting the result.
//-----------------------------------------------------------------------------
uS_u32d0_t TIMEBASE_Convert_to_microSec_u32d0( TIMEBASE_ID_t timebase_id, uint32 count );


///----------------------------------------------------------------------------
//! \brief Convert TIMEBASE count value into nanoseconds (nS_u32d0 format)
//!
//! \param timebase_id The TIMEBASE to use for the conversion
//! \param count The count value to convert
//!
//! \return the time in nanoseconds based on the settings of the TIMEBASE 
//!   return value is in nS_u32d0 format.
//!
//! NOTES:
//!   If the return value is not able to fit into the 32-bit return variable,
//!   then the return value will be coerced to ULONG_MAX.  If there is any possibility
//!   that an overflow will occur, then it is the APPLICATION programmer's responsibility
//!   to compare the return value against ULONG_MAX before accepting the result.
//-----------------------------------------------------------------------------
nS_u32d0_t TIMEBASE_Convert_to_nanoSec_u32d0( TIMEBASE_ID_t timebase_id, uint32 count );


#endif // TIMEBASE_H


//@}

