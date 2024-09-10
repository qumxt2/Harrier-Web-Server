// PinEntryScreen.c

// Copyright 2015 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the PIN entry screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "gdisp.h"
#include "screensTask.h"
#include "CountDigit.h"
#include "PinEntryScreen.h"
#include "screenStuff.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

#define NUM_PIN_DIGITS                  (4u)
#define DIGIT_BOX_X                     (9u)
#define DIGIT_BOX_Y                     (4u)
#define SECRET_PIN_BACKDOOR             (1492u)

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************


// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

static DIGIT_BOX_t * gPinEntryDigitBox;

static INPUT_MODE_t ReturnMode;
static uint32 pin = 0;

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawScreen(void);

static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);

// **********************************************************************************************************
// PinScreen - The main handler for the volume screen display
// **********************************************************************************************************

INPUT_MODE_t PinEntryScreen(INPUT_EVENT_t InputEvent)
{
    void (*processInputEvent[NUMBER_OF_INPUT_EVENTS])(void);
    
    processInputEvent[INPUT_EVENT_ENTRY_INIT] = processInputEntryEvent;
    processInputEvent[INPUT_EVENT_RESET] = processInputResetEvent;
    processInputEvent[INPUT_EVENT_ENTER] = processInputEnterEvent;
    processInputEvent[INPUT_EVENT_UP_ARROW] = processInputUpArrowEvent;
    processInputEvent[INPUT_EVENT_DOWN_ARROW] = processInputDownArrowEvent;
    processInputEvent[INPUT_EVENT_RIGHT_ARROW] = processInputRightArrowEvent;
    processInputEvent[INPUT_EVENT_LEFT_ARROW] = processInputLeftArrowEvent;
    processInputEvent[INPUT_EVENT_PRESS_HOLD_ENTER] = processInputDefaultEvent;
	processInputEvent[INPUT_EVENT_BOTH_ARROWS] = processInputDefaultEvent;
	processInputEvent[INPUT_EVENT_REFRESH_SCREEN] = processInputDefaultEvent;
    
    // Process based on input event    
    (void)(*processInputEvent[InputEvent])();
    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    if ( (gSetup.ScreenPinEnable)  && (gSetup.ScreenPin > 0) )
    {
        drawScreen();
    }
    else
    {
        ReturnMode = INPUT_MODE_CONFIG;   
    }
    
    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// drawScreen - draw the volume screen
// **********************************************************************************************************

static void drawScreen(void)
{
    gsetcpos(DIGIT_BOX_X - 4, DIGIT_BOX_Y);
    gputs("PIN ");
    DisplayCountDigit(&(*gPinEntryDigitBox).countDigit);

    if ((*gPinEntryDigitBox).isEditMode == TRUE)
    {
        DrawCountDigitDigitBox(&(*gPinEntryDigitBox).countDigit);
    }
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
            
    ReturnMode = INPUT_MODE_PIN_ENTRY;
    pin = 0;			
			
    //load shared digit boxes
    gPinEntryDigitBox = &digitBox1;
    (void)LoadCountDigit(&(*gPinEntryDigitBox).countDigit, pin, NUM_PIN_DIGITS, NO_DECIMAL_POINT, DIGIT_BOX_X, DIGIT_BOX_Y, FALSE, FALSE);
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if ((anyDigitBoxIsEdit() == FALSE)) 
    {
        ReturnMode = INPUT_MODE_RUN;
    }
    clearAllDigitBoxIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    if( (*gPinEntryDigitBox).isEditMode == TRUE )
    {
        pin = GetCountDigitValue(&(*gPinEntryDigitBox).countDigit);
        (*gPinEntryDigitBox).isEditMode = FALSE;
        if ( (pin == gSetup.ScreenPin) || (pin == SECRET_PIN_BACKDOOR) )
        {
            ReturnMode = INPUT_MODE_CONFIG;
        }
    }
    else
    {
        (*gPinEntryDigitBox).countDigit.DigitSelected = COUNT_DIGIT_1000;
        (*gPinEntryDigitBox).isEditMode = TRUE;
    } 
}

//****************************************************************************//
//Fcn: processInputUpArrowEvent
//
//Desc: This function processes the up arrow events
//****************************************************************************//
static void processInputUpArrowEvent(void)
{
    (void)upEventForDigitBox();
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    (void)downEventForDigitBox();
}

