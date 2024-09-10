// gcaPortal_component.h

// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This module presents a user-interactive console on the debug serial port 
// associated specifically with the component level of the primary processor on
// the Display Control Module.  This console can be used for troubleshooting 
// purposes as well as for general testing. 


#ifndef GCA_PORTAL_COMPONENT_H
#define GCA_PORTAL_COMPONENT_H


//***********************************************************************************
// HEADER FILES
//***********************************************************************************
#include "typedef.h"					// Compiler specific type definitions

//***********************************************************************************
// * CONSTANTS
//***********************************************************************************
#define DBUG_USBLOAD (DBUG_APP_CATEGORY(2))

//***********************************************************************************
// PUBLIC FUCTION PROTOTYPES
//***********************************************************************************

//***********************************************************************************
// void GCAP_CompInterpreterInit (void)
//***********************************************************************************

	// This function initializes the component level functionality of the GCA
	// portal, specifically for the primary processor on the Display Control
	// Module.  This function should be called at the component level only, included 
	// in ComponentInit().  There is no need to call this function from the 
	// application level. 

	void GCAP_CompInterpreterInit (void);

//***********************************************************************************


#endif 		// GCA_PORTAL_COMPONENT_H
