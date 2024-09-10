// ActivationScreen.c

// Copyright 2015 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the Activation screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include <stdint.h>
#include "dvseg_17G721_setup.h"
#include "gdisp.h"
#include "screensTask.h"
#include "PublishSubscribe.h"
#include <string.h>
#include "screenStuff.h"

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawActivationScreen(void);

// **********************************************************************************************************
// ActivationScreen - The main handler for the activation screen display
// **********************************************************************************************************

INPUT_MODE_t ActivationScreen(INPUT_EVENT_t InputEvent)
{
    INPUT_MODE_t ReturnMode = INPUT_MODE_ACTIVATION;

    // Process based on input event
    switch( InputEvent )
    {
        case INPUT_EVENT_ENTRY_INIT:
            //reset all the boxes to their default states
            clearAllIsFocus();
            hideAllBoxes();
            clearAllIsEdit();
            
            ClearScreen();
            break;

        case INPUT_EVENT_RESET:
            ReturnMode = INPUT_MODE_NETWORK;
            break;

        case INPUT_EVENT_ENTER:
        case INPUT_EVENT_UP_ARROW:
        case INPUT_EVENT_DOWN_ARROW:
        case INPUT_EVENT_RIGHT_ARROW:
        case INPUT_EVENT_LEFT_ARROW:
            break;

        default:
            break;
    }

    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawActivationScreen();
 
    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// drawActivationScreen - Draw the rest of the activation screen
// **********************************************************************************************************

static void drawActivationScreen(void)
{
    char* pActivationStr = PUBLISH_getActivationKey();

    gsetcpos(0, 2);
    gputs("Web Activation Code");

    gsetcpos(0, 4);
    if (strlen(pActivationStr) == ACTIVATION_KEY_SIZE - 1)
    {
        gputs(pActivationStr);
    }
    else
    {
        gputs("Waiting to connect...");
    }

    gsetcpos(0, 6);
    gputs("Press Reset To Exit");
}

