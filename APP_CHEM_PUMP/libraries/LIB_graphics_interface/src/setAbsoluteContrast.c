// setAbsoluteContrast.c

// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This function allows the application to set the absolute value of the LCD contrast
// setting in a range from 0 to +99.  Upon LCD initialization, the default contrast level 
// is set to 48.  This value gives the best appearance at room temperature conditions 
// with average loading.  The contrast level is very sensitive.  Under normal conditions, 
// a value below 35 results in a white screen.  A value above 60 causes the screen to be 
// completely dark.  The application layer may wish to remember the last contrast level 
// set by the user and reset the contrast to that level upon power up in ApplicationInit().
// This function can be used for that purpose.  

// The function takes in a parameter between 0 and +99.  The function returns either the 
// new contrast level or CONTRAST_ADJ_ERROR.  If the returned value is CONTRAST_ADJ_ERROR, 
// either an error has occurred in the function, or the input parameter was outside of 
// the valid range.  
 
// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisp.h"							// Graphics driver library prototypes
#include "typedef.h"						// Compiler specific type definitions
#include "graphics_interface.h"			// Interface library prototypes

#define CONTRAST_LOW_LIMIT		(0)			// Low Contrast Limit
#define CONTRAST_HIGH_LIMIT		(99)		// High Contrast Limit

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
uint8 setAbsoluteContrast (uint8 absoluteValue)
{
	//uint8 oldContrast;						// Variable to store the old contrast setting

	if (absoluteValue <= CONTRAST_HIGH_LIMIT)
	{
		//oldContrast = ghw_cont_set(absoluteValue);
		
			if ( ghw_err() )					// Check for error(s)
			{
				return CONTRAST_ADJ_ERROR;		// Error(s) found
			}

			else 
			{
				return absoluteValue;
			}
	}

	else
	{
		return CONTRAST_ADJ_ERROR; 
	}
}
