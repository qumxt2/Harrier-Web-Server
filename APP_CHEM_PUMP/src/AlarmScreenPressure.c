// AlarmScreenPressure.c

#include "stdio.h"


// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the alarm pressure screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "CountDigit.h"
#include "AlarmScreenPressure.h"
#include "screensTask.h"
#include "AdvancedScreen.h"
#include "screenStuff.h"
#include "utilities.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_PRESSURE_DIGITS         5u

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************
typedef enum
{
    FOCUS_HIGH_PRESSURE = 0,            
    FOCUS_LOW_PRESSURE,
    NUMBER_ALARM_PRESS_ITEMS
} ALARM_PRESS_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t* gHighPressureDigitBox;
static DIGIT_BOX_t* gLowPressureDigitBox;

static INPUT_MODE_t ReturnMode;
static ALARM_PRESS_FOCUS_t alarmFocusIndex = FOCUS_HIGH_PRESSURE;
static uint32 highPressure = 0;
static uint32 lowPressure = 0;
static bool* isFocusArray[NUMBER_ALARM_PRESS_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawAlarmScreenPressure(ALARM_PRESS_FOCUS_t index);
static ALARM_PRESS_FOCUS_t incrementAlarmFocusIndex(ALARM_PRESS_FOCUS_t focusIndex);
static ALARM_PRESS_FOCUS_t decrementAlarmFocusIndex(ALARM_PRESS_FOCUS_t focusIndex);
uint32 refreshHighPressureDigitBox(void);
uint32 refreshLowPressureDigitBox(void);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// AlarmScreenPressure - The main handler for the alarm pressure screen display
// **********************************************************************************************************
INPUT_MODE_t AlarmScreenPressure(INPUT_EVENT_t InputEvent)
{
    ReturnMode = INPUT_MODE_ALARMS_PRESS;

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

    drawAlarmScreenPressure(alarmFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementAlarmFocusIndex - Move focus to the next field
// **********************************************************************************************************
static ALARM_PRESS_FOCUS_t incrementAlarmFocusIndex(ALARM_PRESS_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex < (NUMBER_ALARM_PRESS_ITEMS - 1) )
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
// decrementAlarmFocusIndex - Move focus to the previous field
// **********************************************************************************************************
static ALARM_PRESS_FOCUS_t decrementAlarmFocusIndex(ALARM_PRESS_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = NUMBER_ALARM_PRESS_ITEMS - 1;
    }
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawAlarmScreenPressure - Draw the rest of the alarm pressure screen
// **********************************************************************************************************
static void drawAlarmScreenPressure(ALARM_PRESS_FOCUS_t focusIndex)
{
    gsetcpos(0, 1);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("HIGH PRESSURE            BAR");
    }
    else
    {
        gputs("HIGH PRESSURE            PSI");
    }

    gsetcpos(0, 3);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("LOW PRESSURE             BAR");
    }
    else
    {
        gputs("LOW PRESSURE             PSI");
    }

    drawAllDigitBoxes();
}

// **********************************************************************************************************
// refreshHighPressureDigitBox - Update the digit box and return the localized value for pressure
// **********************************************************************************************************
uint32 refreshHighPressureDigitBox(void)
{
    const uint32 localPressure = getLocalPressure(gSetup.HighPressureTrigger);

    if (gSetup.Units == UNITS_METRIC)
    {
        (void)LoadCountDigit(&(*gHighPressureDigitBox).countDigit, localPressure, NUM_PRESSURE_DIGITS, DECIMAL_POINT_ONE_DIGIT, 13, 1, FALSE, FALSE);
    }
    else
    {
        (void)LoadCountDigit(&(*gHighPressureDigitBox).countDigit, localPressure, NUM_PRESSURE_DIGITS, NO_DECIMAL_POINT, 13, 1, FALSE, FALSE);
    }

    return localPressure;
}

// **********************************************************************************************************
// refreshLowPressureDigitBox - Update the digit box and return the localized value for pressure
// **********************************************************************************************************
uint32 refreshLowPressureDigitBox(void)
{
    const uint32 localPressure = getLocalPressure(gSetup.LowPressureTrigger);

    if (gSetup.Units == UNITS_METRIC)
    {
        (void)LoadCountDigit(&(*gLowPressureDigitBox).countDigit, localPressure, NUM_PRESSURE_DIGITS, DECIMAL_POINT_ONE_DIGIT, 13, 3, FALSE, FALSE);
    }
    else
    {
        (void)LoadCountDigit(&(*gLowPressureDigitBox).countDigit, localPressure, NUM_PRESSURE_DIGITS, NO_DECIMAL_POINT, 13, 3, FALSE, FALSE);
    }

    return localPressure;
}

//****************************************************************************//
//Fcn: processInputEntryEvent
//
//Desc: This function handles all the Input Entry Event that occurs when the 
// screen first loads
//****************************************************************************//
static void processInputEntryEvent(void)
{
    clearAllIsFocus();
    hideAllBoxes();
    clearAllIsEdit();
    ClearScreen();    

    //load shared digit boxes
    gHighPressureDigitBox = &digitBox1;
    gLowPressureDigitBox = &digitBox2;
    highPressure = refreshHighPressureDigitBox();
    lowPressure = refreshLowPressureDigitBox();
    
    //unhide required boxes
    (*gHighPressureDigitBox).isHidden = FALSE;
    (*gLowPressureDigitBox).isHidden = FALSE;    
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    alarmFocusIndex = FOCUS_HIGH_PRESSURE;
    *isFocusArray[alarmFocusIndex] = TRUE;   
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if(anyDigitBoxIsEdit() == FALSE)
    {
        ReturnMode = INPUT_MODE_ALARMS;
        hideAllBoxes();
        clearAllIsFocus();
    }
    highPressure = refreshHighPressureDigitBox();
    lowPressure = refreshLowPressureDigitBox();                
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( alarmFocusIndex )
    {
        case FOCUS_HIGH_PRESSURE:
            if( (*gHighPressureDigitBox).isEditMode == TRUE )
            {
                highPressure = GetCountDigitValue(&(*gHighPressureDigitBox).countDigit);
                (*gHighPressureDigitBox).isEditMode = FALSE;
                
                if (gSetup.Units == UNITS_METRIC)
                 {
                    highPressure = barToPsi(highPressure);
                 }
                (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, HighPressureTrigger), (DistVarType)highPressure);

                PublishUint32(TOPIC_HighPressureTrigger, gSetup.HighPressureTrigger);
                highPressure = refreshHighPressureDigitBox();
            }
            else
            {
                (*gHighPressureDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gHighPressureDigitBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_LOW_PRESSURE:
            if( (*gLowPressureDigitBox).isEditMode == TRUE )
            {
                lowPressure = GetCountDigitValue(&(*gLowPressureDigitBox).countDigit);
                (*gLowPressureDigitBox).isEditMode = FALSE;
                if (gSetup.Units == UNITS_METRIC)
                 {
                    lowPressure = barToPsi(lowPressure);
                 }
                (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, LowPressureTrigger), (DistVarType)lowPressure);
                PublishUint32(TOPIC_LowPressureTrigger, gSetup.LowPressureTrigger);
                lowPressure = refreshLowPressureDigitBox();
            }
            else
            {
                (*gLowPressureDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gLowPressureDigitBox).isEditMode = TRUE;
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
    if(upEventForDigitBox() == FALSE)
    {
       alarmFocusIndex = decrementAlarmFocusIndex(alarmFocusIndex);
    }   
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if(downEventForDigitBox() == FALSE)
    {
        alarmFocusIndex = incrementAlarmFocusIndex(alarmFocusIndex);
    }
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_HIGH_PRESSURE] = &((*gHighPressureDigitBox).isFocus);
    isFocusArray[FOCUS_LOW_PRESSURE] = &((*gLowPressureDigitBox).isFocus);
}
