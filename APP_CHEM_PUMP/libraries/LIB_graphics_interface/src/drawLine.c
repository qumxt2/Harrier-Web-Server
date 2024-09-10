// drawLine.c

// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// SCREEN LAYOUT
//  				x = LCD_X_MIN   -->   -->   -->   -->   -->   -->   -->   x = LCD_X_MAX
//	y = LCD_Y_MIN	-----------------------------------------------------------------
//					|																|
//			|		|																|
//			|		|																|
//			V		|																|
//					|																|
//					|						 LCD Screen Area						|
//			|		|																|
//			|		|																|
//			V		|																|
//					|																|
//	y = LCD_Y_MAX	-----------------------------------------------------------------
// 

// This function will draw a line defined by its start and end coordinates.  
// The function will return GRAPHICS_CALL_OK if no errors occurred.  The function will 
// return GRAPHICS_CALL_ERROR if an error was detected.  
// Refer to the screen layout image above for details on the orientation of the x and
// y coordinates.  

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisp.h"							// Graphics driver library prototypes
#include "typedef.h"						// Compiler specific type definitions
#include "graphics_interface.h"			// Interface library prototypes

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
uint8 drawLine(uint8 xStart, uint8 yStart, uint8 xEnd, uint8 yEnd)
{
	// Verify coordinates are within the screen area
	if ( (xStart > LCD_X_MAX) || (yStart > LCD_Y_MAX) || (xEnd > LCD_X_MAX) || (yEnd > LCD_Y_MAX) )
	{
		return GRAPHICS_CALL_ERROR;			// Invalid coordinates
	}

	// Driver library function to set the start position for the line
	gmoveto(xStart, yStart);

	// Driver library function to draw the line from the start position to 
	// the end position
	glineto(xEnd, yEnd);

	if ( ghw_err() )						// Check for error(s)
	{
		return GRAPHICS_CALL_ERROR;			// Error(s) found
	}

	return GRAPHICS_CALL_OK;				// No error found
	
}
