// placeBitmap.c

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

// This function will place a bitmap image of any size at any location on the LCD.
// The xPosition and yPosition parameters to the function define the location 
// on the screen where the upper left hand corner of the bitmap image will be placed.  
// The screen itself is defined with a horizontal x-axis and a vertical y-axis, where
// the upper left hand corner of the screen is the origin of the axes.  The x-axis
// ranges from LCD_X_MIN to LCD_X_MAX while the y-axis ranges from LCD_Y_MIN to LCD_Y_MAX.  
// The "data" variable must point to the bitmap structure.  For information on this 
// structure, please refer to the comments in bitmaps.c and bitmaps.h.  
// The function returns a value of GRAPHICS_CALL_OK if no errors are detected.  Otherwise, the 
// function returns a value of GRAPHICS_CALL_ERROR.
// Failing to specifically cast the variable "data" to (void*) in the function call
// may result in compiler warnings.  

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisp.h"						// Graphics driver library prototypes
#include "typedef.h"					// Compiler specific type definitions
#include "graphics_interface.h"			// Interface library prototypes

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
uint8 placeBitmap(uint8 xPosition, uint8 yPosition, void * data)
{

	// Test absolute bitmap placement
	if ( (xPosition > LCD_X_MAX) || (yPosition > LCD_Y_MAX) ) 		
	{			
		return GRAPHICS_CALL_ERROR;		// Coordinates invalid
	}

	// Driver library function to place bitmap
	gputsym( xPosition, yPosition, data);	

	if ( ghw_err() )					// Check for error(s)
	{
		return GRAPHICS_CALL_ERROR;		// Error(s) found
	}

	return GRAPHICS_CALL_OK;			// No error found


}

