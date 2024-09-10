// fillArea.c

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

// This function will fill a rectangular screen area whose boundaries are defined by the
// parameters in the function call with the pattern defined by the final parameter in 
// the function call.  Pre-defined fill patterns are available in graphics_interface.h.  
// Other fill patterns can be created if necessary.  The LSB of the fillPattern word is
// used on even pixel lines.  The MSB of the fillPattern word is used on odd pixel lines. 
// A '1' represents a dark pixel, and a '0' represents a light pixel.  The function will 
// return GRAPHICS_CALL_OK if no errors occurred.  The function will return 
// GRAPHICS_CALL_ERROR if an error was detected.  Refer to the screen layout image above 
// for details on the orientation of the x and y coordinates.  

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisp.h"						// Graphics driver library prototypes
#include "typedef.h"					// Compiler specific type definitions
#include "graphics_interface.h"		// Interface library prototypes


// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
uint8 fillArea (uint8 xUpperLeft, uint8 yUpperLeft, uint8 xLowerRight, 
					uint8 yLowerRight, uint16 fillPattern)
{
	// Check the rectangle area boundaries
	if ( (xUpperLeft > xLowerRight) || (yUpperLeft > yLowerRight) || (xLowerRight > LCD_X_MAX) 
			|| (yLowerRight > LCD_Y_MAX) )
	{
		return GRAPHICS_CALL_ERROR;					// Invalid boundaries
	}

	// Driver library function to clear the screen area
	gfillvp(xUpperLeft, yUpperLeft, xLowerRight, yLowerRight, fillPattern);

	if ( ghw_err() )					// Check for error(s)
	{
		return GRAPHICS_CALL_ERROR;		// Error(s) found
	}

	return GRAPHICS_CALL_OK;			// No error found

}
