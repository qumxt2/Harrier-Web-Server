//! \file	event_defs.h
//! \brief The Event Definitions Module header file.
//!
//! Copyright 2010-2011
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_comm				Events
//! \brief Events.
//!
//! \b DESCRIPTION:
//!    This module provides a high-level definitions for GCA events


#ifndef EVENT_DEFS_H
#define EVENT_DEFS_H


#include "typedef.h"					// Compiler specific type definitions

#ifdef ADM_GRAPHICS_PROCESSOR
	#include "ipcTypedef.h"
#else
	#include "dvar.h"
#endif // ADM_GRAPHICS_PROCESSOR


//! \ingroup events
//@{


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************

/// List of possible event types.
typedef enum
{
	/// An action that is being logged but is not required to be displayed
	EVENTTYPE_RECORD_ONLY = 0,

	/// A parameter that is not immediately critical to the process needs attention to prevent more serious issues in the future
	EVENTTYPE_ADVISORY,

	/// A parameter critical to the process has reached a level requiring attention, but not sufficient to stop the system at this time.
	EVENTTYPE_DEVIATION,

	/// A parameter critical to the process has reached a level requring the system to be halted and the problem to be addressed immediately
	EVENTTYPE_ALARM,

	EVENTTYPE_NUM_TYPES,
	EVENTTYPE_INVALID = EVENTTYPE_NUM_TYPES
} eventtype_t;

/// List of possible event actions (for use with logging function).
typedef enum
{
	/// Event was set
	EVENTACTION_SET = 0,

	/// Event was acknowledged
	EVENTACTION_ACKNOWLEDGE,

	/// Event was cleared
	EVENTACTION_CLEAR,

	EVENTACTION_NUM_ACTIONS,
	EVENTACTION_INVALID = EVENTACTION_NUM_ACTIONS
} eventaction_t;


// ***************************************************
// * MARCROS
// ***************************************************


// ***************************************************
// * PUBLIC FUNCTION PROTOTYPES
// ***************************************************

//----------------------------------------------------------------------------
//! \brief Encode event information into a Distributed Variable
//!
//! \param	event_code_str          Pointer to an event code string to be encoded.
//! \param	event_type              The type of event to be encoded
//!
//! \b Notes:
//!	\li	The parameter \c event_code_str must point to string with length of 4.
//!
//! \return The event encoded into DistVarType format
//----------------------------------------------------------------------------
DistVarType EVENT_DEFS_EncodeEventCodeDvar( char *event_code_str, eventtype_t event_type );

//----------------------------------------------------------------------------
//! \brief Decode event information from a Distributed Variable
//!
//! \param	event_code          	The distributed variable to be decoded
//! \param	event_code_str          Pointer to a string variable that will store decoded event code string.
//! \param	event_type              Pointer to a variable that will store the decoded event type
//!
//! \b Notes:
//!	\li	The parameter \c event_code_str must be sized to store string of length 4 (i.e. >= 5 bytes).
//----------------------------------------------------------------------------
void EVENT_DEFS_DecodeEventCodeDvar( DistVarType event_code, char *event_code_str, eventtype_t *event_type );


//@}

#endif // EVENT_DEFS_H
