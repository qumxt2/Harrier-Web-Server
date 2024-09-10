// backlightOnOff.c

// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The following function turns the LCD backlight on and off with varying levels of 
// brightness.  A parameter value of BACKLIGHT_OFF will turn the backlight completely off. 
// A parameter value of BACKLIGHT_100 will turn the backlight full intensity.  Parameter values 
// between BACKLIGHT_10 and BACKLIGHT_90 will turn the backlight on to varying degrees 
// of brightness (10%, 20%, and so on).  These inputs are defined in graphics_interface.h.
// This function will do nothing if the input parameter is not in the described range.  

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"						// Compiler specific type definitions
#include "graphics_interface.h"				// Interface library prototypes
#include "maxim_MAX8647.h"

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
void setBacklightOnOff( uint8 backlightValue )
{
	if( backlightValue == BACKLIGHT_ON )
	{
		MAX8647_SetBacklightOn();
	}
	
	else if( backlightValue == BACKLIGHT_OFF )
	{
		MAX8647_SetBacklightOff();
	}
}

bool getBacklightOnOff( )
{
	return MAX8647_IsBacklightOn();
}

void setBacklightIntensity( uint8 backlightValue )
{	
	MAX8647_SetBacklightPct( backlightValue );
}

uint8 getBacklightIntensity( )
{	
	return MAX8647_GetBacklightPct( ); 
}
