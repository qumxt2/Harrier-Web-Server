// ConfigScreen.c

// Copyright 2014 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the configuration menu screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#include "pumpControlTask.h"
#include "dvseg_17G721_run.h"
#include "dvinterface_17G721.h"
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "screensTask.h"
#include "gdisp.h"
#include "countDigit.h"
#include "graphics_interface.h"
#include "screen_bitmaps.h"
#include "alarms.h"
#include "ConfigScreen.h"
#include "screenStuff.h"

#define MENU_Y_OFFSET   1u

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_PUMP = 0,
    FOCUS_ALARMS,
    FOCUS_PIN_CODE,
    FOCUS_NETWORK,
    FOCUS_ADVANCED,
    FOCUS_FLUID_PRESS,
    FOCUS_TANK_LEVEL,
    NUMBER_CONFIG_ITEMS
} CONFIG_FOCUS_t;

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawConfigMenuOptions(CONFIG_FOCUS_t index);
static CONFIG_FOCUS_t incrementConfigFocusIndex(CONFIG_FOCUS_t focusIndex);
static CONFIG_FOCUS_t decrementConfigFocusIndex(CONFIG_FOCUS_t focusIndex);
static void drawToolIcon(void);

static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static CONFIG_FOCUS_t configFocusIndex = FOCUS_PUMP;
static INPUT_MODE_t ReturnMode;

// **********************************************************************************************************
// ConfigScreen - The main handling function for the configuration screen
// **********************************************************************************************************

INPUT_MODE_t ConfigScreen(INPUT_EVENT_t InputEvent)
{
    void (*processInputEvent[NUMBER_OF_INPUT_EVENTS])(void);
    
    processInputEvent[INPUT_EVENT_ENTRY_INIT] = processInputEntryEvent;
    processInputEvent[INPUT_EVENT_RESET] = processInputResetEvent;
    processInputEvent[INPUT_EVENT_ENTER] = processInputEnterEvent;
    processInputEvent[INPUT_EVENT_UP_ARROW] = processInputUpArrowEvent;
    processInputEvent[INPUT_EVENT_DOWN_ARROW] = processInputDownArrowEvent;
    processInputEvent[INPUT_EVENT_RIGHT_ARROW] = processInputDefaultEvent;
    processInputEvent[INPUT_EVENT_LEFT_ARROW] = processInputDefaultEvent;
	processInputEvent[INPUT_EVENT_PRESS_HOLD_ENTER] = processInputDefaultEvent;
	processInputEvent[INPUT_EVENT_BOTH_ARROWS] = processInputDefaultEvent;
	processInputEvent[INPUT_EVENT_REFRESH_SCREEN] = processInputDefaultEvent;
    
    // Unless something changes it return select mode
    ReturnMode = INPUT_MODE_CONFIG;

    // Process based on input event
    (void)(*processInputEvent[InputEvent])();
    
    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawConfigMenuOptions(configFocusIndex);

    // Place the tool icon on the screen
    drawToolIcon();

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementConfigFocusIndex - Increment the field with the focus
// **********************************************************************************************************

static CONFIG_FOCUS_t incrementConfigFocusIndex(CONFIG_FOCUS_t focusIndex)
{
    if( focusIndex < (NUMBER_CONFIG_ITEMS - 1) )
    {
        return focusIndex + 1;
    }
    else
    {
        return 0;
    }
}

// **********************************************************************************************************
// decrementConfigFocusIndex - Decrement the field with the focus
// **********************************************************************************************************

static CONFIG_FOCUS_t decrementConfigFocusIndex(CONFIG_FOCUS_t focusIndex)
{
    if( focusIndex > 0 )
    {
        return focusIndex - 1;
    }
    else
    {
        return (NUMBER_CONFIG_ITEMS - 1);
    }
}

// **********************************************************************************************************
// drawConfigMenuOptions - Draw the rest of the configuration screen
// **********************************************************************************************************

static void drawConfigMenuOptions(CONFIG_FOCUS_t focusIndex)
{
    INT8U rowNum = MENU_Y_OFFSET;

    gsetcpos(1, rowNum++);
    gputs("PUMP");
    gsetcpos(1, rowNum++);
    gputs("ALARMS");
    gsetcpos(1, rowNum++);
    gputs("PIN CODE");
    gsetcpos(1, rowNum++);
    gputs("NETWORK");
    gsetcpos(1, rowNum++);
    gputs("ADVANCED");
    gsetcpos(1, rowNum++);
    gputs("FLUID PRESS");
    gsetcpos(1, rowNum++);
    gputs("TANK LEVEL");    

    // Draw a box around the whole shebang
    grectangle((ggetfw() * 1) - 3,
               (ggetfh() * MENU_Y_OFFSET) - 3,
               (ggetfw() * 12) + 3,
               (ggetfh() * 8) + 1);

    // Put rectangle around selected entry
    DrawBox(1, (focusIndex) + MENU_Y_OFFSET, 11);
}

// **********************************************************************************************************
// drawToolIcon - Draw the tool icon in the upper right hand corner of the screen
// **********************************************************************************************************

static void drawToolIcon(void)
{
    void* pToolIcon = (void*)&BMP_Screen_Modeicons_32x32[6];

    // Right justify tool icon
    (void)placeBitmap( GDISPW - gfgetfw(pToolIcon) - 1, 0, pToolIcon );
}


//****************************************************************************//
//Fcn: processInputEntryEvent
//
//Desc: This function handles all the Input Entry Event that occurs when the 
// screen first loads
//****************************************************************************//
static void processInputEntryEvent(void)
{
    //reset all the boxes to their default states
    clearAllIsFocus();
    hideAllBoxes();
    clearAllIsEdit();
    ClearScreen();
    configFocusIndex = FOCUS_PUMP;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    ReturnMode = INPUT_MODE_RUN;
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( configFocusIndex )
    {
        case FOCUS_PUMP:
            PMP_resetStates();
            switch( gSetup.MeteringMode )
            {
                case METERING_MODE_VOLUME:
                    ReturnMode = INPUT_MODE_FLOW;
                    break;

                case METERING_MODE_TIME:
                    ReturnMode = INPUT_MODE_TIME;
                    break;

                case METERING_MODE_CYCLES:
                    ReturnMode = INPUT_MODE_CYCLES;
                    break;

                default:
                    break;
            }
            break;

        case FOCUS_ALARMS:
            ReturnMode = INPUT_MODE_ALARMS;
            break;

        case FOCUS_PIN_CODE:
            ReturnMode = INPUT_MODE_PIN_CODE;
            break;

        case FOCUS_NETWORK:
            ReturnMode = INPUT_MODE_NETWORK;
            break;

		case FOCUS_ADVANCED:
			ReturnMode = INPUT_MODE_ADVANCED;
			break;

		case FOCUS_FLUID_PRESS:
			ReturnMode = INPUT_MODE_FLUID_PRESS;
			break;
		
		case FOCUS_TANK_LEVEL:
			ReturnMode = INPUT_MODE_TANK;
			break; 
        default:
            break;
    }
}

//****************************************************************************//
//Fcn: processInputUpArrowEvent
//
//Desc: This function processes the up arrow events
//****************************************************************************//
static void processInputUpArrowEvent(void)
{
    configFocusIndex = decrementConfigFocusIndex(configFocusIndex);
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    configFocusIndex = incrementConfigFocusIndex(configFocusIndex);
}

