// AlarmScreen.c

#include "stdio.h"


// Copyright 2014 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the alarm screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "gdisp.h"
#include "CountDigit.h"
#include "screensTask.h"
#include "AlarmScreen.h"
#include "alarms.h"
#include "AdvancedScreen.h"
#include "units_pressure.h"
#include "screenStuff.h"
#include "dvseg_17G721_run.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_ALARM_TYPE = 0,
    FOCUS_ALARM_ACTION,
    NUMBER_ALARM_ITEMS
} ALARM_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static SELECTION_BOX_t* gAlarmTypeSelectBox;
static SELECTION_BOX_t* gAlarmActionSelectBox;

static char* ppAlarmTypeSelectBoxTextList[] =
{
    "CONTROL",
    "BATTERY",
    "PRESSURE",
    "TANK", 
    "TEMP",
    "MOTOR"
};

static char* ppAlarmActionSelectBoxTextList[] =
{
    "STOP",
    "NOTIFY"
};

static INPUT_MODE_t ReturnMode;
static ALARM_FOCUS_t alarmFocusIndex = FOCUS_ALARM_TYPE;
static ALARM_TYPES_t alarmType = ALARM_CONTROL;
static ALARM_ACTIONS_t alarmAction = ACTION_ALARM;
static bool* isFocusArray[NUMBER_ALARM_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawAlarmScreen(ALARM_FOCUS_t index);
static ALARM_FOCUS_t incrementAlarmFocusIndex(ALARM_FOCUS_t focusIndex);
static ALARM_FOCUS_t decrementAlarmFocusIndex(ALARM_FOCUS_t focusIndex);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// AlarmScreen - The main handler for the alarm screen display
// **********************************************************************************************************
INPUT_MODE_t AlarmScreen(INPUT_EVENT_t InputEvent)
{
    ReturnMode = INPUT_MODE_ALARMS;

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

    drawAlarmScreen(alarmFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementAlarmFocusIndex - Move focus to the next field
// **********************************************************************************************************
static ALARM_FOCUS_t incrementAlarmFocusIndex(ALARM_FOCUS_t focusIndex)
{   
    uint8_t numItems = NUMBER_ALARM_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    // Index one less line when tank and battery alarm screens are selected
    if ((alarmType == ALARM_BATTERY) || (alarmType == ALARM_TANK))
    {
        numItems -= 1;
    }
        
    if( focusIndex < (numItems - 1) )
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
static ALARM_FOCUS_t decrementAlarmFocusIndex(ALARM_FOCUS_t focusIndex)
{   
    uint8_t numItems = NUMBER_ALARM_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;

    // Index one less line when tank and battery alarm screens are selected
    if ((alarmType == ALARM_BATTERY) || (alarmType == ALARM_TANK))
    {
        numItems -= 1;
    }
    
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = numItems - 1;
    }
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawAlarmScreen - Draw the rest of the alarm screen
// **********************************************************************************************************
static void drawAlarmScreen(ALARM_FOCUS_t focusIndex)
{
    gsetcpos(0, 1);
    gputs("ALARM TYPE");
    
    if ((alarmType != ALARM_BATTERY) && (alarmType != ALARM_TANK))
    {
        gsetcpos(0, 3);
        gputs("ALARM ACTION");
        (*gAlarmActionSelectBox).isHidden = FALSE;
    }
    else
    {
        (*gAlarmActionSelectBox).isHidden = TRUE;    
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
    
    alarmAction = gSetup.AlarmAction;	
    
    //load shared select boxes
    gAlarmTypeSelectBox = &selectBox1;
    gAlarmActionSelectBox = &selectBox2;
    (void) selectBoxConfigure(gAlarmTypeSelectBox, 0, NUMBER_ALARM_TYPES, FALSE, FALSE, FALSE, FALSE, 11, 1, 9, ppAlarmTypeSelectBoxTextList);        
    (void) selectBoxConfigure(gAlarmActionSelectBox, 0, NUMBER_ALARM_ACTIONS, FALSE, FALSE, FALSE, FALSE, 13, 3, 7, ppAlarmActionSelectBoxTextList);    
    
    (*gAlarmTypeSelectBox).index = alarmType;
    (*gAlarmActionSelectBox).index = alarmAction;    
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    alarmFocusIndex = FOCUS_ALARM_TYPE;
    *isFocusArray[alarmFocusIndex] = TRUE;   
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if( anySelectBoxIsEdit() == FALSE )
    {
        ReturnMode = INPUT_MODE_CONFIG;
        hideAllBoxes();
        clearAllIsFocus();
    }
    (*gAlarmTypeSelectBox).index = alarmType;
    (*gAlarmActionSelectBox).index = alarmAction;    
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
        case FOCUS_ALARM_TYPE:
            if( (*gAlarmTypeSelectBox).isEditMode == TRUE)
            {
                alarmType = (*gAlarmTypeSelectBox).index;
                (*gAlarmTypeSelectBox).isEditMode = FALSE;
				switch(alarmType)
				{
					case ALARM_CONTROL:
						ReturnMode = INPUT_MODE_ALARMS_CONTROL;
						break;
					case ALARM_BATTERY:
						ReturnMode = INPUT_MODE_ALARMS_BATT;
						break;
					case ALARM_PRESSURE:
						ReturnMode = INPUT_MODE_ALARMS_PRESS;
						break;
					case ALARM_TANK:
						ReturnMode = INPUT_MODE_ALARMS_TANK;
						break;
                    case ALARM_TEMP:
                        ReturnMode = INPUT_MODE_ALARMS_TEMP;
                        break;
                    case ALARM_MOTOR_CURRENT:
                        ReturnMode = INPUT_MODE_ALARMS_MOTOR_CURRENT;
                        break;
					default:
						break;
				}						                
            }
            else
            {
                (*gAlarmTypeSelectBox).isEditMode = TRUE;
            }
            break;
            
        case FOCUS_ALARM_ACTION:
            if( (*gAlarmActionSelectBox).isEditMode == TRUE)
            {
                alarmAction = (*gAlarmActionSelectBox).index;
                (*gAlarmActionSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, AlarmAction), (DistVarType)alarmAction);
            }
            else
            {
                (*gAlarmActionSelectBox).isEditMode = TRUE;
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
    if(downEventForSelectBox() == FALSE)
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
    isFocusArray[FOCUS_ALARM_TYPE] = &((*gAlarmTypeSelectBox).isFocus);    
    isFocusArray[FOCUS_ALARM_ACTION] = &((*gAlarmActionSelectBox).isFocus);
}
