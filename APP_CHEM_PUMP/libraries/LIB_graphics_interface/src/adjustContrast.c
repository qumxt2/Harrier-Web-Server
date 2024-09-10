// adjustContrast.c

// Copyright 2006-2007
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This function will adjust the contrast setting for the LCD.  Upon LCD initialization, 
// the default contrast level is set to 48.  This value gives the best appearance at
// room temperature conditions with average loading.  The acceptable range of contrast
// values is 0 to +99, but the contrast level is very sensitive.  Under normal conditions, 
// a value below 35 results in a white screen.  A value above 60 causes the screen to be 
// completely dark.  Temperature variations and/or changes in loading may require 
// slight adjustments in the contrast level.  This function allows for those adjustments. 
// The application layer may wish to remember the last contrast level set by the user and 
// reset the contrast to that level upon power up in ApplicationInit().

// The function takes in a parameter of either +1 or -1.  A +1 increments the contrast level
// by 1 from its current setting.  A -1 decrements the contrast level by 1 from its current
// setting.  The function returns either the new contrast level in a range from 0 to +99, 
// or CONTRAST_ADJ_ERROR.  If the returned value is CONTRAST_ADJ_ERROR, an error has 
// occurred in the function. 

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisp.h"							// Graphics driver library prototypes
#include "typedef.h"						// Compiler specific type definitions
#include "graphics_interface.h"				// Interface library prototypes

#define CONTRAST_LOW_LIMIT		(0)			// Low Contrast Limit
#define CONTRAST_HIGH_LIMIT		(99)		// High Contrast Limit

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
uint8 adjustContrast (sint8 relativeValue)
{
	uint8 oldContrast;						// Variable to store the old contrast setting
	uint8 currentContrast;					// Variable to store the current contrast setting
	currentContrast = ghw_cont_change(0);	// Initialized to the current contrast setting  

	if (relativeValue == -1)
	{
		
		if (currentContrast > CONTRAST_LOW_LIMIT)
		{
			
			// Driver library function to set the new contrast level.  The function returns
			// an unsigned 8-bit value representing the old contrast level.  
			oldContrast = ghw_cont_change(relativeValue);
		
			if ( ghw_err() )					// Check for error(s)
			{
				return CONTRAST_ADJ_ERROR;		// Error(s) found
			}

			return oldContrast - 1;
		}
		
		return CONTRAST_LOW_LIMIT;
	}


	else if(relativeValue == 1)
	{
		
		if (currentContrast < CONTRAST_HIGH_LIMIT)
		{
			// Driver library function to set the new contrast level.  The function returns
			// an unsigned 8-bit value representing the old contrast level.  
			oldContrast = ghw_cont_change(relativeValue);

			if ( ghw_err() )					// Check for error(s)
			{
				return CONTRAST_ADJ_ERROR;		// Error(s) found
			}

			return oldContrast + 1;
			
		}
		
		return CONTRAST_HIGH_LIMIT;
	}

	else
	{
		return CONTRAST_ADJ_ERROR; 
	}
}


