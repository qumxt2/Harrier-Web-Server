// AlarmScreenControl.c

#include "stdio.h"


// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the alarm control screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "CountDigit.h"
#include "AlarmScreenControl.h"
#include "screensTask.h"
#include "alarms.h"
#include "screenStuff.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define MAX_DISPLAYED_ALARM_LENGTH  (12)

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_ALARM_1 = 0,
    FOCUS_ALARM_2,
    FOCUS_REMOTE_OFF,
    NUMBER_ALARM_CONTROL_ITEMS
} ALARM_CONTROL_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static SELECTION_BOX_t* gAlarm1SelectBox;
static SELECTION_BOX_t* gAlarm2SelectBox;
static SELECTION_BOX_t* gRemoteOffSelectBox;

static char* ppAlarmSelectBoxTextList[] =
{
    "DISABLED",
    "NOR CLOSED",
    "NOR OPEN"
};
static INPUT_MODE_t ReturnMode;
static ALARM_CONTROL_FOCUS_t alarmControlFocusIndex = FOCUS_ALARM_1;
static LOGIC_LEVELS_t alarm1Type = LOGIC_DISABLED;
static LOGIC_LEVELS_t alarm2Type = LOGIC_DISABLED;
static LOGIC_LEVELS_t remoteOffType = LOGIC_DISABLED;
static bool* isFocusArray[NUMBER_ALARM_CONTROL_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawAlarmScreenControl(ALARM_CONTROL_FOCUS_t index);
static ALARM_CONTROL_FOCUS_t incrementAlarmFocusIndex(ALARM_CONTROL_FOCUS_t focusIndex);
static ALARM_CONTROL_FOCUS_t decrementAlarmFocusIndex(ALARM_CONTROL_FOCUS_t focusIndex);
static void upperInPlace(char* pString);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// AlarmScreenControl - The main handler for the alarm control screen display
// **********************************************************************************************************
INPUT_MODE_t AlarmScreenControl(INPUT_EVENT_t InputEvent)
{
    ReturnMode = INPUT_MODE_ALARMS_CONTROL;

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

    drawAlarmScreenControl(alarmControlFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementAlarmFocusIndex - Move focus to the next field
// **********************************************************************************************************
static ALARM_CONTROL_FOCUS_t incrementAlarmFocusIndex(ALARM_CONTROL_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex < (NUMBER_ALARM_CONTROL_ITEMS - 1) )
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
static ALARM_CONTROL_FOCUS_t decrementAlarmFocusIndex(ALARM_CONTROL_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = NUMBER_ALARM_CONTROL_ITEMS - 1;
    }
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawAlarmScreenControl - Draw the rest of the alarm control screen
// **********************************************************************************************************
static void drawAlarmScreenControl(ALARM_CONTROL_FOCUS_t focusIndex)
{
    char strBuf[MAX_DISPLAYED_ALARM_LENGTH+1] = {0};    
    
    gsetcpos(0, 1);
    (void)snprintf(strBuf, sizeof(strBuf), "%s ", ALARM_alarmNames[ALARM_ID_Input_1]);
    upperInPlace(strBuf);
    gputs(strBuf);

    gsetcpos(0, 2);
    (void)snprintf(strBuf, sizeof(strBuf), "%s ", ALARM_alarmNames[ALARM_ID_Input_2]);
    upperInPlace(strBuf);
    gputs(strBuf);

    gsetcpos(0, 3);
    (void)snprintf(strBuf, sizeof(strBuf), "%s ", ALARM_alarmNames[ALARM_ID_Remote_Off]);
    upperInPlace(strBuf);
    gputs(strBuf);
    
    drawAllDigitBoxes();
    drawAllSelectBoxes();
}

// **********************************************************************************************************
// upperInPlace -- Capitalize a string in place
// **********************************************************************************************************
static void upperInPlace(char* pString)
{
    while(*pString)
    {
        *pString = toupper(*pString);
        pString++;
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
    ClearScreen();
    clearAllIsFocus();
    hideAllBoxes();
    clearAllIsEdit();
    
    alarm1Type = gSetup.Alarm1Trigger;
    alarm2Type = gSetup.Alarm2Trigger;
    remoteOffType = gSetup.RemoteOffTrigger;
    
    //load shared select boxes
    gAlarm1SelectBox = &selectBox1;
    gAlarm2SelectBox = &selectBox2;
    gRemoteOffSelectBox = &selectBox3;
    (void) selectBoxConfigure(gAlarm1SelectBox, 0, NUMBER_LOGIC_LEVELS, FALSE, FALSE, FALSE, FALSE, 11, 1, 10, ppAlarmSelectBoxTextList);
    (void) selectBoxConfigure(gAlarm2SelectBox, 0, NUMBER_LOGIC_LEVELS, FALSE, FALSE, FALSE, FALSE, 11, 2, 10, ppAlarmSelectBoxTextList);
    (void) selectBoxConfigure(gRemoteOffSelectBox, 0, NUMBER_LOGIC_LEVELS, FALSE, FALSE, FALSE, FALSE, 11, 3, 10, ppAlarmSelectBoxTextList);

    (*gAlarm1SelectBox).index = alarm1Type;
    (*gAlarm2SelectBox).index = alarm2Type;
    (*gRemoteOffSelectBox).index = remoteOffType;
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    alarmControlFocusIndex = FOCUS_ALARM_1;    
    *isFocusArray[alarmControlFocusIndex] = TRUE;   
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if(anySelectBoxIsEdit() == FALSE)
    {
        ReturnMode = INPUT_MODE_ALARMS;
        hideAllBoxes();
        clearAllIsFocus();
    }    
    (*gAlarm1SelectBox).index = alarm1Type;
    (*gAlarm2SelectBox).index = alarm2Type;
    (*gRemoteOffSelectBox).index = remoteOffType;
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( alarmControlFocusIndex )
    {
        case FOCUS_ALARM_1:
            if( (*gAlarm1SelectBox).isEditMode == TRUE )
            {
                alarm1Type = (*gAlarm1SelectBox).index;
                (*gAlarm1SelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, Alarm1Trigger), (DistVarType)alarm1Type);
            }
            else
            {
                (*gAlarm1SelectBox).isEditMode = TRUE;
            }
            break;

       case FOCUS_ALARM_2:
            if( (*gAlarm2SelectBox).isEditMode == TRUE )
            {
                alarm2Type = (*gAlarm2SelectBox).index;
                (*gAlarm2SelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, Alarm2Trigger), (DistVarType)alarm2Type);
            }
            else
            {
                (*gAlarm2SelectBox).isEditMode = TRUE;
            }
            break;

       case FOCUS_REMOTE_OFF:
            if( (*gRemoteOffSelectBox).isEditMode == TRUE )
            {
                remoteOffType = (*gRemoteOffSelectBox).index;
                (*gRemoteOffSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, RemoteOffTrigger), (DistVarType)remoteOffType);
                if(remoteOffType == LOGIC_DISABLED)
                {
                    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, RemoteDisableActive), FALSE);
                }
            }
            else
            {
                (*gRemoteOffSelectBox).isEditMode = TRUE;
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
    if(upEventForSelectBox() == FALSE)
    {
       alarmControlFocusIndex = decrementAlarmFocusIndex(alarmControlFocusIndex);
    }   
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if(downEventForSelectBox() == FALSE)
    {
        alarmControlFocusIndex = incrementAlarmFocusIndex(alarmControlFocusIndex);
    }
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_ALARM_1] = &((*gAlarm1SelectBox).isFocus);
    isFocusArray[FOCUS_ALARM_2] = &((*gAlarm2SelectBox).isFocus);
    isFocusArray[FOCUS_REMOTE_OFF] = &((*gRemoteOffSelectBox).isFocus);
}
