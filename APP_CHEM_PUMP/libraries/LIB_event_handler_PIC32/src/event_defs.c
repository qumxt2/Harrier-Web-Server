// event_handler.c

// Copyright 2010-2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved


// DESCRIPTION
// This file contains all the event handling code


#include "typedef.h"

#ifdef ADM_GRAPHICS_PROCESSOR
	#include "ipcTypedef.h"
#else
	#include "dvar.h"
#endif // ADM_GRAPHICS_PROCESSOR

#include "event_defs.h"


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************


// ***************************************************
// * CONSTANTS
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************


// ***************************************************
// * PRIVATE VARIABLES
// ***************************************************


// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************


// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************
DistVarType EVENT_DEFS_EncodeEventCodeDvar( char *event_code_str, eventtype_t event_type )
{
	if( event_code_str == NULL )
	{
		return 0;
	}

	return (DistVarType)((uint8)event_code_str[0] & 0x7F) +
		   ((DistVarType)((uint8)event_code_str[1] & 0x7F) << 7) +
		   ((DistVarType)((uint8)event_code_str[2] & 0x7F) << 14) +
		   ((DistVarType)((uint8)event_code_str[3] & 0x7F) << 21) +
		   ((DistVarType)((uint8)event_type & 0x0F) << 28);
}

void EVENT_DEFS_DecodeEventCodeDvar( DistVarType event_code, char *event_code_str, eventtype_t *event_type )
{
	if( (event_code_str == NULL) ||
		(event_type == NULL) )
	{
		return;
	}

	event_code_str[0] = (char)(event_code & 0x7F);
	event_code_str[1] = (char)((event_code >> 7) & 0x7F);
	event_code_str[2] = (char)((event_code >> 14) & 0x7F);
	event_code_str[3] = (char)((event_code >> 21) & 0x7F);
	event_code_str[4] = '\0';

	*event_type = (eventtype_t)((event_code >> 28) & 0x0F);
}
