// AlarmScreenTemp.c

// Copyright 2018
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the alarm temp control screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "CountDigit.h"
#include "screensTask.h"
#include "AlarmScreenTemp.h"
#include "AdvancedScreen.h"
#include "screenStuff.h"
#include "stdint.h"
#include "PublishSubscribe.h"
#include "dvseg_17G721_run.h"
#include "gdisp.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_TEMP_DIGITS          3u

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_TEMP_CONTROL = 0,  
    FOCUS_TEMP_SETPOINT,
    NUMBER_ALARM_TEMP_ITEMS
} ALARM_TEMP_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t * gTempSetpointDigitBox;

static SELECTION_BOX_t* gTempControlSelectBox;

static char* ppTempControlSelectBoxTextList[] =
{
    "DISABLED",
    "DISPLAY",
    "ON BELOW",     // OFF ABOVE
    "ON ABOVE",     // OFF BELOW
};

static INPUT_MODE_t ReturnMode;
static ALARM_TEMP_FOCUS_t alarmTempFocusIndex = FOCUS_TEMP_CONTROL;
static TEMP_CONTROL_MODES_t TempControl = TEMP_CONTROL_DISABLED;
static uint32 tempSetpoint = 0;
static bool* isFocusArray[NUMBER_ALARM_TEMP_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawAlarmScreenTemp(ALARM_TEMP_FOCUS_t index);
static ALARM_TEMP_FOCUS_t incrementAlarmFocusIndex(ALARM_TEMP_FOCUS_t focusIndex);
static ALARM_TEMP_FOCUS_t decrementAlarmFocusIndex(ALARM_TEMP_FOCUS_t focusIndex);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// AlarmScreenTemp - The main handler for the alarm temp screen display
// **********************************************************************************************************
INPUT_MODE_t AlarmScreenTemp(INPUT_EVENT_t InputEvent)
{
    ReturnMode = INPUT_MODE_ALARMS_TEMP;

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

    drawAlarmScreenTemp(alarmTempFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementAlarmFocusIndex - Move focus to the next field
// **********************************************************************************************************
static ALARM_TEMP_FOCUS_t incrementAlarmFocusIndex(ALARM_TEMP_FOCUS_t focusIndex)
{
    uint8_t numAlarmItems = NUMBER_ALARM_TEMP_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    // Index one less line when disabled or display are selected
    if ((TempControl == TEMP_CONTROL_DISABLED) || (TempControl == TEMP_CONTROL_DISPLAY))
    {
        numAlarmItems -= 1;
    }
    
    if( focusIndex < (numAlarmItems - 1) )
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
static ALARM_TEMP_FOCUS_t decrementAlarmFocusIndex(ALARM_TEMP_FOCUS_t focusIndex)
{
    uint8_t numAlarmItems = NUMBER_ALARM_TEMP_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;

    // Index one less line when disabled or display are selected
    if ((TempControl == TEMP_CONTROL_DISABLED) || (TempControl == TEMP_CONTROL_DISPLAY))
    {
        numAlarmItems -= 1;
    }
    
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = numAlarmItems - 1;
    }
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawAlarmScreenTemp - Draw the rest of the alarm temp screen
// **********************************************************************************************************
static void drawAlarmScreenTemp(ALARM_TEMP_FOCUS_t focusIndex)
{
    gsetcpos(0, 1);
    gputs("CONTROL");    
    
    if ((gSetup.TempControl == TEMP_CONTROL_DISABLED) || (gSetup.TempControl == TEMP_CONTROL_DISPLAY))
    {
        // Hide all temp control setpoint & don't display temperature on run screen
        (*gTempSetpointDigitBox).isHidden = TRUE;
    }
    else
    {
        (*gTempSetpointDigitBox).isHidden = FALSE;
        
        gsetcpos(0, 3);
        gputs("SETPOINT");
        gsetcpos(14, 3);
        if (gSetup.Units == UNITS_METRIC)
        {
            gputch(DEGREE_SYMBOL);
            gputs("C");
        }
        else
        {
            gputch(DEGREE_SYMBOL);            
            gputs("F");
        }
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
    ClearScreen();
    clearAllIsFocus();
    hideAllBoxes();
    clearAllIsEdit();
        
    TempControl = gSetup.TempControl;
    tempSetpoint = (uint32)getLocalTemperature(gSetup.TempSetpoint);

    //load shared digit boxes
    gTempSetpointDigitBox = &digitBox1;
    //The unhiding of these boxes is taken care of in the draw function
    (void)LoadCountDigit(&(*gTempSetpointDigitBox).countDigit, tempSetpoint, NUM_TEMP_DIGITS, NO_DECIMAL_POINT, 9, 3, FALSE, TRUE);
    
    // unhide required digit boxes
    (*gTempSetpointDigitBox).isHidden = FALSE;
    
    //load shared select boxes
    gTempControlSelectBox = &selectBox1;
    (void) selectBoxConfigure(gTempControlSelectBox, 0, NUMBER_TEMP_CONTROL_MODES, FALSE, FALSE, FALSE, FALSE, 9, 1, 10, ppTempControlSelectBoxTextList);

    (*gTempControlSelectBox).index = TempControl;
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    alarmTempFocusIndex = FOCUS_TEMP_CONTROL;
    *isFocusArray[alarmTempFocusIndex] = TRUE;   
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if( (anyDigitBoxIsEdit() == FALSE) && (anySelectBoxIsEdit() == FALSE))
    {
        ReturnMode = INPUT_MODE_ALARMS;
        hideAllBoxes();
        clearAllIsFocus();
    }
    (void)LoadCountDigit(&(*gTempSetpointDigitBox).countDigit, tempSetpoint, NUM_TEMP_DIGITS, NO_DECIMAL_POINT, 9, 3, FALSE, TRUE);
    (*gTempControlSelectBox).index = TempControl;
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{    
    switch( alarmTempFocusIndex )
    {
        case FOCUS_TEMP_CONTROL:
            if( (*gTempControlSelectBox).isEditMode == TRUE )
            {
                TempControl = (*gTempControlSelectBox).index;
                (*gTempControlSelectBox).isEditMode = FALSE;
                
                // Publish the temperature if we're changing from disabled to one of the enabled states
                if ((gSetup.TempControl == TEMP_CONTROL_DISABLED) && (TempControl > TEMP_CONTROL_DISABLED))
                {
                    PublishInt32(TOPIC_Temperature, (int32_t)gRun.Temperature);
                }
                
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TempControl), (DistVarType)TempControl);
                PublishUint32(TOPIC_TemperatureControl, gSetup.TempControl + 1);
                
                // Make sure the "disabled by temperature" is cleared if it was just disabled or set to display only
                if((gSetup.TempControl == TEMP_CONTROL_DISABLED) || (gSetup.TempControl == TEMP_CONTROL_DISPLAY))
                {
                    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, TempProbeDisableActive), FALSE);
                }
            }
            else
            {
                (*gTempControlSelectBox).isEditMode = TRUE;
            }
            break;        
        
        case FOCUS_TEMP_SETPOINT:
            if( (*gTempSetpointDigitBox).isEditMode == TRUE )
            {
                // Gets a +/- value from the user, returned as a signed # stored in a uint32
                tempSetpoint = GetCountDigitValue(&(*gTempSetpointDigitBox).countDigit);
                (*gTempSetpointDigitBox).isEditMode = FALSE;
                setLocalTemperature(tempSetpoint, DVA17G721_SS(gSetup, TempSetpoint));
                (void)LoadCountDigit(&(*gTempSetpointDigitBox).countDigit, (uint32)getLocalTemperature(gSetup.TempSetpoint), NUM_TEMP_DIGITS, NO_DECIMAL_POINT, 9, 3, FALSE, TRUE);
            }
            else
            {
                (*gTempSetpointDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gTempSetpointDigitBox).isEditMode = TRUE;
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
    if( upEventForSelectBox() == FALSE && upEventForDigitBox() == FALSE)
    {
       alarmTempFocusIndex = decrementAlarmFocusIndex(alarmTempFocusIndex);
    }   
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if(downEventForSelectBox() == FALSE && downEventForDigitBox() == FALSE)
    {
        alarmTempFocusIndex = incrementAlarmFocusIndex(alarmTempFocusIndex);
    }
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_TEMP_CONTROL] = &((*gTempControlSelectBox).isFocus);    
    isFocusArray[FOCUS_TEMP_SETPOINT] = &((*gTempSetpointDigitBox).isFocus);
}
