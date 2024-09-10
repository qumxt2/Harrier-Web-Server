// PinScreen.c

// Copyright 2015 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the PIN screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "gdisp.h"
#include "screensTask.h"
#include "CountDigit.h"
#include "PinScreen.h"
#include "screenStuff.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

#define NUM_PIN_DIGITS                  (4u)
#define SELECT_BOX_X                    (5u)
#define SELECT_BOX_Y                    (2u)
#define SELECT_BOX_LENGTH               (8u)
#define DIGIT_BOX_X                     (9u)
#define DIGIT_BOX_Y                     (5u)

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_PIN_SELECT_BOX = 0,
    FOCUS_PIN_ENTRY,
    NUMBER_PIN_ITEMS
} PIN_FOCUS_t;

typedef enum
{
    PIN_DISABLED = 0,
    PIN_ENABLED,
    NUMBER_PIN_ENABLE_CHOICES
} PIN_ENABLE_CHOICES_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t * gPinEntryDigitBox;

static SELECTION_BOX_t* gPinSelectBox;

static char* ppPinSelectBoxTextList[] =
{
    "DISABLED",
    "ENABLED",
};

static INPUT_MODE_t ReturnMode;
static PIN_FOCUS_t pinFocusIndex = FOCUS_PIN_SELECT_BOX;
static uint32 pin = 0;
static BOOLEAN isPinEnabled = FALSE;
static bool* isFocusArray[NUMBER_PIN_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawPinScreen(PIN_FOCUS_t index);
static PIN_FOCUS_t incrementPinFocusIndex(PIN_FOCUS_t focusIndex);
static PIN_FOCUS_t decrementPinFocusIndex(PIN_FOCUS_t focusIndex);

static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// PinScreen - The main handler for the pin screen display
// **********************************************************************************************************

INPUT_MODE_t PinScreen(INPUT_EVENT_t InputEvent)
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

    drawPinScreen(pinFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementPinFocusIndex - Move focus to the next field
// **********************************************************************************************************

static PIN_FOCUS_t incrementPinFocusIndex(PIN_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex < (NUMBER_PIN_ITEMS - 1) )
    {
        focusIndex = focusIndex + 1;
    }
    else
    {
         focusIndex = 0;
    }
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// decrementPinFocusIndex - Move focus to the previous field
// **********************************************************************************************************

static PIN_FOCUS_t decrementPinFocusIndex(PIN_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = (NUMBER_PIN_ITEMS - 1);
    }
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawFlowScreen - Draw the rest of the volume screen
// **********************************************************************************************************

static void drawPinScreen(PIN_FOCUS_t focusIndex)
{
    if (gSetup.ScreenPinEnable)
    {
        gsetcpos(DIGIT_BOX_X - 4, DIGIT_BOX_Y);
        gputs("PIN ");
        (*gPinEntryDigitBox).isHidden = FALSE;
    }
    else
    {
        (*gPinEntryDigitBox).isHidden = TRUE;
    }
    
    drawAllDigitBoxes();
    drawAllSelectBoxes();
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
	
    ReturnMode = INPUT_MODE_PIN_CODE;
    pin = gSetup.ScreenPin;
    isPinEnabled = gSetup.ScreenPinEnable;	

    //Load shared digit box
    gPinEntryDigitBox = &digitBox1;
    (void)LoadCountDigit(&(*gPinEntryDigitBox).countDigit, pin, NUM_PIN_DIGITS, NO_DECIMAL_POINT, DIGIT_BOX_X, DIGIT_BOX_Y, FALSE, FALSE);
    
    //load shared select box
    gPinSelectBox = &selectBox1;
    (void) selectBoxConfigure(gPinSelectBox, 0, NUMBER_PIN_ENABLE_CHOICES, FALSE, FALSE, FALSE, TRUE, SELECT_BOX_X, SELECT_BOX_Y, SELECT_BOX_LENGTH, ppPinSelectBoxTextList); 
    (*gPinSelectBox).index = isPinEnabled;
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    pinFocusIndex = FOCUS_PIN_SELECT_BOX;
    *isFocusArray[FOCUS_PIN_SELECT_BOX] = TRUE;
	
    //The draw screen function will unhide this if is enabled
	(*gPinEntryDigitBox).isHidden = TRUE;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if( (anySelectBoxIsEdit() == FALSE) && (anyDigitBoxIsEdit() == FALSE) )
    {
        ReturnMode = INPUT_MODE_CONFIG;
        clearAllIsFocus();
        hideAllBoxes();
    }
    (void)LoadCountDigit(&(*gPinEntryDigitBox).countDigit, pin, NUM_PIN_DIGITS, NO_DECIMAL_POINT, DIGIT_BOX_X, DIGIT_BOX_Y, FALSE, FALSE);
    clearAllSelectBoxIsEdit();
    (*gPinSelectBox).index = isPinEnabled;
    clearAllDigitBoxIsEdit();   
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( pinFocusIndex )
    {
        case FOCUS_PIN_SELECT_BOX:
            if( (*gPinSelectBox).isEditMode == TRUE )
            {
                isPinEnabled = (*gPinSelectBox).index;
                (*gPinSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, ScreenPinEnable), (DistVarType)isPinEnabled);
            }
            else
            {
                (*gPinSelectBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_PIN_ENTRY:
            if( (*gPinEntryDigitBox).isEditMode == TRUE )
            {
                pin = GetCountDigitValue(&(*gPinEntryDigitBox).countDigit);
                (*gPinEntryDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, ScreenPin), (DistVarType)pin);
            }
            else
            {
                (*gPinEntryDigitBox).countDigit.DigitSelected = COUNT_DIGIT_1000;
                (*gPinEntryDigitBox).isEditMode = TRUE;
            }
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
    if( upEventForDigitBox() == FALSE && upEventForSelectBox() == FALSE)
    {
        if(gSetup.ScreenPinEnable)
        {
            pinFocusIndex = decrementPinFocusIndex(pinFocusIndex);
        }
    }
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
   if( downEventForDigitBox() == FALSE && downEventForSelectBox() == FALSE)
    {
        if(gSetup.ScreenPinEnable)
        {
            pinFocusIndex = incrementPinFocusIndex(pinFocusIndex);
        }
    } 
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_PIN_SELECT_BOX] = &((*gPinSelectBox).isFocus);
    isFocusArray[FOCUS_PIN_ENTRY] = &((*gPinEntryDigitBox).isFocus);
}

