// component.h
 
// Copyright 2006-2007
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all of the necessary component level information, including
// function calls and task information.  

#ifndef COMPONENT_H
#define COMPONENT_H

#include "typedef.h"			// Compiler specific type definitions


// ***************************************************
// * CONSTANTS
// ***************************************************

// Possible ComponentInit() function return values
#define NO_ERROR					(0)
#define ERROR						(-1)


//**********************************************************************************************
// CONSTANTS
//**********************************************************************************************
#define COMPONENT_INIT_OK								(0b0000000000000000)
#define COMPONENT_INIT_I2C_ERROR						(0b0000000000000001)
#define COMPONENT_INIT_LCD_ERROR						(0b0000000000000010)
#define COMPONENT_INIT_BACKLIGHT_ERROR					(0b0000000000000100)
#define COMPONENT_INIT_BUTTON_ERROR						(0b0000000000001000)
#define COMPONENT_INIT_RTC_ERROR						(0b0000000000010000)
#define COMPONENT_INIT_TOKEN_LOAD_ERROR					(0b0000000000100000)
#define COMPONENT_INIT_GCAPORTAL_ERROR					(0b0000000001000000)

/// The first EEPROM address location that the application level
/// software is allowed to use.  Everything below this address is
/// reserved for either common or component software modules.
/// \hideinitializer

//#define	APPLICATION_FIRST_EEADDR		(COMPONENT_FIRST_EEADDR	+ 0x1000)
//#define USB_COMPONENT_FIRST_EEADDR		(COMPONENT_FIRST_EEADDR + 0x0800)


// ***************************************************
// * PUBLIC FUNCTIONS
// ***************************************************

//**********************************************************************************************
// sint16 ComponentInit(void)
//**********************************************************************************************

	// The following function will initialize all component level resources and tasks.  This 
	// function should be called from main() prior to ApplicationInit() and after CommonInit().
	// A return value of RETURN_NO_ERROR implies that no error occurred.  Any other return value
	// designates an error.  

	sint16 ComponentInit(void); 

//**********************************************************************************************


#endif // COMPONENT_H
