// SnoopScreen.c

// Copyright 2015 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the modem snoop screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include <stdint.h>
#include "dvseg_17G721_setup.h"
#include "gdisp.h"
#include "screensTask.h"
#include "PublishSubscribe.h"
#include "modemTask.h"

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

// **********************************************************************************************************
// PinScreen - The main handler for the pin screen display
// **********************************************************************************************************

INPUT_MODE_t SnoopScreen(INPUT_EVENT_t InputEvent)
{
    INPUT_MODE_t ReturnMode = INPUT_MODE_SNOOP;

    // Process based on input event
    switch( InputEvent )
    {
        case INPUT_EVENT_ENTRY_INIT:
            ClearScreen();
            MODEM_Snoop(TRUE);
            gselvp(VIEWPORT_MAIN);
            gsetcpos(0, 0);
            break;

        case INPUT_EVENT_RESET:
            ReturnMode = INPUT_MODE_NETWORK;
            MODEM_Snoop(FALSE);
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

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// SNOOP_putc - Display snoop info on screen
// **********************************************************************************************************

void SNOOP_putc(char c)
{
    // Display character as long is it isn't an EOL character
    if ( (c != '\r') && (c != '\n') )
    {
        gputch(c);
    }
}
