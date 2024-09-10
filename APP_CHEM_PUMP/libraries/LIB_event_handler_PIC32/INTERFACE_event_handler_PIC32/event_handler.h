//! \file	event_handler.h
//! \brief The Event Handler Module header file.
//!
//! Copyright 2010-2011
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_comm				Events
//! \brief Events.
//!
//! \b DESCRIPTION:
//!    This module provides a high-level interface for tracking
//!    GCA events.


#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H


#include "typedef.h"					// Compiler specific type definitions
#include "event_defs.h"


//! \ingroup events
//@{


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************

/// The required function type for an event logging function
typedef void (*LogEventFunction_t)( char *event_code_str, eventtype_t event_type, eventaction_t event_action );

/// The required function type for a callback function that will be called when an event code has been acknowledged
typedef void (*EventAckCallback_t)( char *event_code_str, eventtype_t event_type );

/// The required function type for a function that makes an attempt to clear the condition that caused an event.
/// An EventClearFunction_t must return a boolean value indicating whether or not the condition was successfully cleared.
/// After an event has been acknowledged, the event clear function will be called at rate of 1Hz until the condition has
/// been successfully cleared.
typedef bool (*EventClearFunction_t)( char *event_code_str, eventtype_t event_type );


// ***************************************************
// * MARCROS
// ***************************************************


// ***************************************************
// * PUBLIC FUNCTION PROTOTYPES
// ***************************************************

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! \param log_event_function  The event logging function that will be called
//!                            on any event action (set, ack, clear).
//!
//! \b Notes:
//! \li Must be invoked prior to using the remainder of this interface.
//! \li The parameter \c log_event_function will accept the NULL value if no
//!     logging is desired.
//!
//! \return TRUE on success, otherwise FALSE
//----------------------------------------------------------------------------
bool EVENT_HANDLER_Init( LogEventFunction_t log_event_function );


//----------------------------------------------------------------------------
//! \brief Event Handler Task
//!
//! \b Notes:
//! \li This function \b MUST \b NOT be called directly.  It is an RTOS task that will be created
//!     and started from within the function \c EVENT_HANDLER_Init
//! \li This task must be added to task list in main.c.  Typical task settings are:
//!      - #define	TASK_xx_FUNCTION_NAME	EVENT_HANDLER_Task
//!      - #define	TASK_xx_STACK_SIZE		500
//!      - #define	TASK_xx_PRIORITY		185
//----------------------------------------------------------------------------
void EVENT_HANDLER_Task( void );


//----------------------------------------------------------------------------
//! \brief Set an event.
//!
//! \param	event_code_str          Pointer to an event code string being set.
//! \param	event_type              The type of event being set
//! \param	ack_callback_function   Pointer to a function that will be called when the event gets acknowledged
//! \param  event_clear_function    Pointer to a function that will be called in attempt to clear an event-causing condition
//!
//! \b Notes:
//!	\li	The parameter \c event_code_str must point to string with length of 4.
//! \li If no acknowledge callback is desired, then the parameter \c ack_callback_function may be set to NULL.
//! \li After an event has been acknowledged, the event handler module will attempt to clear the event-causing condition by
//!		calling the \c event_clear_function.  The \c event_clear_function will continue to be called at a rate
//!     of 1Hz until the condition has been successfully cleared (i.e. the clear function returns TRUE).  If no event
//!     clear functionallity is desired, then the parameter \c event_clear_function may be set to NULL.
//! \li This function waits on a resource, therefore it \b MUST \b NOT be called from a context
//!     in which the RTOS is locked out from task switching (e.g. in a DVAR callback function).
//!
//! \return TRUE on success, otherwise FALSE
//----------------------------------------------------------------------------
bool EVENT_HANDLER_EventSet( char *event_code_str, eventtype_t event_type, 
							 EventAckCallback_t ack_callback_function,
							 EventClearFunction_t event_clear_function );


//! \param	auto_ack_seconds		Number of seconds after event is set that event will auto acknowledge
bool EVENT_HANDLER_EventSetAutoAck( char *event_code_str, eventtype_t event_type, 
							 		EventAckCallback_t ack_callback_function,
									EventClearFunction_t event_clear_function,
							 		uint8 auto_ack_seconds );

//----------------------------------------------------------------------------
//! \brief Acknowledge an event.
//!
//! \param	event_code_str          Pointer to an event code string being acknowledged.
//! \param	event_type              The type of event being acknowledged
//!
//! \b Notes:
//!	\li	The parameter \c event_code_str must point to string with length of 4.
//! \li This function waits on a resource, therefore it \b MUST \b NOT be called from a context
//!     in which the RTOS is locked out from task switching (e.g. in a DVAR callback function).
//!
//! \return TRUE on success, otherwise FALSE
//----------------------------------------------------------------------------
bool EVENT_HANDLER_EventAcknowledge( char *event_code_str, eventtype_t event_type );

//----------------------------------------------------------------------------
//! \brief Acknowledge an event (from within DVAR callback function).
//!
//! \param	oldVal    The old value of the Distributed Variable.
//! \param	newVal    The new value of the Distributed Variable 
//!
//! \b Notes:
//! \li This function is intended to be called from within a Distributed Variable
//!     callback function.
//!
//! \return 0 if \c newVal == \c oldVal, otherwise \c oldVal
//----------------------------------------------------------------------------
DistVarType EVENT_HANDLER_EventAcknowledge_DVCB( DistVarType oldVal, DistVarType newVal );

//----------------------------------------------------------------------------
//! \brief Clear an event.
//!
//! \param	event_code_str          Pointer to an event code string being cleared.
//! \param	event_type              The type of event being cleared
//!
//! \b Notes:
//!	\li	The parameter \c event_code_str must point to string with length of 4.
//! \li This function does not make any attempt to clear the condition that caused the event to be set.
//!     It only the removes the event from the active event list maintained by this module. The application
//!     should ensure that the event-causing condition is no longer in effect before calling this function.
//! \li This function waits on a resource, therefore it \b MUST \b NOT be called from a context
//!     in which the RTOS is locked out from task switching (e.g. in a DVAR callback function).
//!
//! \return TRUE on success, otherwise FALSE
//----------------------------------------------------------------------------
bool EVENT_HANDLER_EventClear( char *event_code_str, eventtype_t event_type );


//----------------------------------------------------------------------------
//! \brief Check if event is active.
//!
//! \param	event_code_str          Pointer to an event code string to be checked.
//! \param	event_type              The type of event to be checked
//!
//! \b Notes:
//!	\li	The parameter \c event_code_str must either point to string with length of 4
//!     or be set to NULL.  If set to NULL, this will have the effect of testing for
//!     ANY active event of type \c event_type.
//! \li The parameter \c event_code_str may use '*' as a wildcard character if the user
//!		only cares about certain characters. For instance, the string "A***" would search
//!		for any event starting with the letter A.
//!	\li	The parameter \c event_type may be set to ::EVENTTYPE_INVALID to check for
//!     \c event_code_str without regard to the event type.
//! \li This function waits on a resource, therefore it \b MUST \b NOT be called from a context
//!     in which the RTOS is locked out from task switching (e.g. in a DVAR callback function).
//!
//! \return TRUE on success, otherwise FALSE
//----------------------------------------------------------------------------
bool EVENT_HANDLER_IsEventActive( char *event_code_str, eventtype_t event_type );

//----------------------------------------------------------------------------
//! \brief Check if all the event codes of a certain type are in a list of events passed into the function
//!
//! \param	**checkForTheseEvents   Array of event codes to be checked
//! \param      sizeOfEventList         Size of the **checkForTheseEvents array
//! \param	event_type              The type of event to be checked
//!
//! \b Notes:
//!         \li This function checks to see if all the active events of a specified
//!             type are found in the input parameter array **checkForTheseEvents
//!         \return TRUE if all active errors are found in input parameter array **checkForTheseEvents
//!                 FALSE if any active error is found that is not in input paramater array **checkForTheseEvents
//----------------------------------------------------------------------------
bool EVENT_HANDLER_isInputEventListOnlyEventsActive(const char **checkForTheseEvents, uint8 sizeOfEventList, eventtype_t eventType);

//----------------------------------------------------------------------------
//! \brief Get the event code that is currently needing acknowledgement from user
//!
//! \b Notes:
//! \li This function waits on a resource, therefore it \b MUST \b NOT be called from a context
//!     in which the RTOS is locked out from task switching (e.g. in a DVAR callback function).
//!
//! \return The event needing acknowledgment (encoded into DistVarType format).
//!         A return value of 0 indicates that there are no events awaiting
//!			acknowledgment.
//----------------------------------------------------------------------------
DistVarType EVENT_HANDLER_GetEventCodeNeedingAck( void );

//----------------------------------------------------------------------------
//! \brief Get the event code that should currently be shown on rolling event display
//!
//! \b Notes:
//! \li This function waits on a resource, therefore it \b MUST \b NOT be called from a context
//!     in which the RTOS is locked out from task switching (e.g. in a DVAR callback function).
//!
//! \return The event needing acknowledgment (encoded into DistVarType format).
//!         A return value of 0 indicates that there are no events needing to
//!			be displayed.
//----------------------------------------------------------------------------
DistVarType EVENT_HANDLER_GetEventCodeRollingDisplay( void );


//----------------------------------------------------------------------------
//! \brief Dump the event lists to the GCA Debug Portal
//!
//! \b Notes:
//! \li This function waits on a resource, therefore it MUST NOT be called from a context in which the
//!     RTOS is locked out from task switching (e.g. in a DVAR callback function).
//----------------------------------------------------------------------------
void EVENT_HANDLER_GCAP_DumpEventLists( void );

//----------------------------------------------------------------------------
//! \brief Returns the number of active events of the given type
//!
//! \param	event_type              The type of event to be checked
//!
//! \return the number of active events
//----------------------------------------------------------------------------

uint8 EVENT_HANDLER_GetActiveEventCount( eventtype_t event_type );

//@}


#endif // EVENT_HANDLER_H
