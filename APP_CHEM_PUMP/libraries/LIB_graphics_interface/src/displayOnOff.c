// displayOnOff.c

// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This function can be used to turn the pixel display of the LCD on and off.  This 
// function does not control power to the LCD.  It simply turns the pixel display
// on and off.  The function will return GRAPHICS_CALL_OK if no errors occurred.  
// The function will return GRAPHICS_CALL_ERROR if an error was detected.   

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisp.h"							// Graphics driver library prototypes
#include "typedef.h"						// Compiler specific type definitions
#include "graphics_interface.h"				// Interface library prototypes

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
uint8 displayOnOff (uint8 displayState)
{
	if (displayState == DISPLAY_ON)
	{
		// Driver library function to turn the pixel display on
		ghw_dispon();
		
		if ( ghw_err() )					// Check for error(s)
		{
			return GRAPHICS_CALL_ERROR;		// Error(s) found
		}

		return GRAPHICS_CALL_OK;			// No error found
	}

	else if(displayState == DISPLAY_OFF)
	{
		// Driver library function to turn the pixel display off
		ghw_dispoff();
		
		if ( ghw_err() )					// Check for error(s)
		{
			return GRAPHICS_CALL_ERROR;		// Error(s) found
		}

		return GRAPHICS_CALL_OK;			// No error found
	}

	else
	{
		return GRAPHICS_CALL_ERROR; 
	}
}
