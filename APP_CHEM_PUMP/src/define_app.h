// define_app.h

// Copyright 2013
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Description

#ifndef DEFINE_APP_H
#define DEFINE_APP_H

// Includes
#include "graphics_interface.h"
//#include "ee_map.h"
// Defines

#define APP_MAGIC_NUMBER                (EE_MAP_VERSION)

#define DBUG_BOOTLOG (DBUG_ALWAYS)

#define APP_SELECTION_PADDING (1)
#define APP_BACKLIGHT_INTENSITY (100) 	// Percent
#define APP_REDRAW_SCREEN (7200)		// 1 Hour
#define APP_FIRST_EE_ADDR (0x2000)

#define APP_X_MIN (LCD_X_MIN) //does not include padding
#define APP_Y_MIN (LCD_Y_MIN) //does not include padding
#define APP_X_MAX (LCD_X_MAX-26) //26 pixels for softkey bar, pxt
#define APP_Y_MAX (LCD_Y_MAX)

//Controls
#define CONTROLS_SINGLE_HEIGHT (16)

//Controls Position
#define CONTROL_X1			(APP_X_MIN+53)
#define CONTROL_X2			(CONTROL_X1 + 25)
#define CONTROL_Y1			(APP_Y_MIN+4)
#define CONTROL_Y2			(CONTROL_Y1 + CONTROLS_SINGLE_HEIGHT + 9)
#define CONTROL_Y3			(CONTROL_Y2 + CONTROLS_SINGLE_HEIGHT + 9)
#define CONTROL_Y4			(CONTROL_Y3 + CONTROLS_SINGLE_HEIGHT + 9)

#define CONTROL_WIDTH		(59)
#define CONTROL_SHORT		(25)

//Icons positions
#define ICON_X_RUN			(3)
#define ICON_X1				(10)
#define ICON_Y1				(1)
#define ICON_Y2				(ICON_Y1 + 25)
#define ICON_Y3				(ICON_Y2 + 25)
#define ICON_Y4 			(ICON_Y3 + 25)
#define SUBICON_X1			(34)
#define SUBICON_MARGIN_TOP 	(5)

// Title Controls for Run pages
#define TITLE_CONTROLS_SINGLE_HEIGHT (19)

#define TITLE_CONTROL_X1	(APP_X_MIN + 45)
#define TITLE_CONTROL_Y1	(APP_Y_MIN + 1)
#define TITLE_CONTROL_Y2	(TITLE_CONTROL_Y1 + TITLE_CONTROLS_SINGLE_HEIGHT + 15)
#define TITLE_CONTROL_Y3	(TITLE_CONTROL_Y2 + TITLE_CONTROLS_SINGLE_HEIGHT + 14)

#define TITLE_ICON_X1		(10)
#define TITLE_ICON_Y1		(3)
#define TITLE_ICON_Y2		(35)
#define TITLE_ICON_Y3		(67)

#define SCREEN_REFRESH \
	{ \
		EventStatus rVal = EventStatus_Update; \
		timeCounter ++;	\
		if (timeCounter >= APP_REDRAW_SCREEN) { \
			timeCounter = 0; \
			rVal = EventStatus_Redraw; } \
		return rVal; \
	} \

#endif
