//lcdInit.c

// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This function initializes the LCD hardware.  Call this function on power-up prior to 
// attempting any graphics manipulation.  In the event of a system or board level power 
// reset, this function will need to be called again prior to attempting any graphics 
// manipulation.  This function should be called at the component level only, included 
// in componentInit().  There is no need to call this function at the application level.  
// A return value of LCD_INIT_ERROR indicates that a hardware error was detected by the 
// low level graphics drivers.  A return value of LCD_INIT_OK indicates that no errors 
// were detected.

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisp.h"							// Graphics driver library prototypes
#include "typedef.h"						// Compiler specific type definitions
#include "graphics_interface.h"				// Interface library prototypes

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
sint16 lcdInit (void)
{
	uint8 error;
	
	
	// WARNING!!! - Applying the -12VDC before a stable +3.3VDC is available to the LCD
	// module on pin 21 of the LCD connector will permanently damage the display.

	// By reaching this point, you are assured of a stable +3.3VDC
	
	// Not used in DCM screen. pxt
	//_LATA6 = 0;					// Switch -12VDC to the LCD module
	//_TRISA6 = 0;				// Set RA6 to an output
	
	
	error = ginit();						// Driver library function to intialize 
											// all LCD hardware

	if ( (ghw_err()) || (error) )			// Check for error(s)
	{
		return LCD_INIT_ERROR;				// Error(s) found
	}

	return LCD_INIT_OK;						// No error found

}

