// drawRectangle.c

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

// This function will draw a rectangular box whose boundaries are defined by the
// parameters in the function call.  The function will return GRAPHICS_CALL_OK if no 
// errors occurred.  The function will return GRAPHICS_CALL_ERROR if an error was detected. 
// Refer to the screen layout image above for details on the orientation of the x and
// y coordinates.  

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisp.h"						// Graphics driver library prototypes
#include "typedef.h"					// Compiler specific type definitions
#include "graphics_interface.h"		// Interface library prototypes

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
uint8 drawRectangle (uint8 xUpperLeft, uint8 yUpperLeft, uint8 xLowerRight, uint8 yLowerRight)
{
	// Check the rectangle area boundaries
	if ( (xUpperLeft > xLowerRight) || (yUpperLeft > yLowerRight) || (xLowerRight > LCD_X_MAX) 
			|| (yLowerRight > LCD_Y_MAX) )
	{
		return GRAPHICS_CALL_ERROR;					// Invalid boundaries
	}

	// Driver library function to draw the rectangular box
	grectangle(xUpperLeft, yUpperLeft, xLowerRight, yLowerRight);

	if ( ghw_err() )					// Check for error(s)
	{
		return GRAPHICS_CALL_ERROR;		// Error(s) found
	}

	return GRAPHICS_CALL_OK;			// No error found

}
