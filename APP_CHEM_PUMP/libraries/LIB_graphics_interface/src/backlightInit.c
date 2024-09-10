// backlightInit.c

// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This function initializes the hardware necessary for interfacing with the backlight
// of the LCD.  This function should be called at the component level only, included 
// in ComponentInit().  There is no need to call this function at the application level.  


#include "graphics_interface.h"				// Interface library prototypes
#include "maxim_MAX8647.h"


void backlightInit (void)
{
	// Backlight Init is done in chip relavent file maxim_MAX8647.c
	MAX8647_InitializeBacklight(MAX8647_C1|MAX8647_C2|MAX8647_C3, 0);
}
