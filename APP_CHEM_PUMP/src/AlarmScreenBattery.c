// AlarmScreenBattery.c

#include "stdio.h"


// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the alarm battery screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "CountDigit.h"
#include "screensTask.h"
#include "AlarmScreenBattery.h"
#include "AdvancedScreen.h"
#include "units_pressure.h"
#include "screenStuff.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_BATTERY_DIGITS          4u
#define CONFIG_TRIGGER_TO_DECIVOLTS (100) // config screen stores in tenths of a volt (decivolts?)

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_POWER_SAVE = 0,  
    FOCUS_BATTERY_WARNING,
    FOCUS_BATTERY_SHUTOFF,
    NUMBER_ALARM_BATT_ITEMS
} ALARM_BATT_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t * gBatteryWarningDigitBox;
static DIGIT_BOX_t * gBatteryShutoffDigitBox;

static SELECTION_BOX_t* gPowerSavingSelectBox;

static char* ppPowerSavingSelectBoxTextList[] =
{
    "OFF",
    "NOTIFY",
    "MIN",
    "NORMAL",
    "MAX"
};

static INPUT_MODE_t ReturnMode;
static ALARM_BATT_FOCUS_t alarmBattFocusIndex = FOCUS_POWER_SAVE;
static POWER_SAVE_MODES_t powerSaveMode = POWER_SAVE_NOTIFY;
static uint32 batteryWarning = 0;
static uint32 batteryShutoff = 0;
static bool* isFocusArray[NUMBER_ALARM_BATT_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawAlarmScreenBattery(ALARM_BATT_FOCUS_t index);
static ALARM_BATT_FOCUS_t incrementAlarmFocusIndex(ALARM_BATT_FOCUS_t focusIndex);
static ALARM_BATT_FOCUS_t decrementAlarmFocusIndex(ALARM_BATT_FOCUS_t focusIndex);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// AlarmScreenBattery - The main handler for the alarm battery screen display
// **********************************************************************************************************
INPUT_MODE_t AlarmScreenBattery(INPUT_EVENT_t InputEvent)
{
    ReturnMode = INPUT_MODE_ALARMS_BATT;

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

    drawAlarmScreenBattery(alarmBattFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementAlarmFocusIndex - Move focus to the next field
// **********************************************************************************************************
static ALARM_BATT_FOCUS_t incrementAlarmFocusIndex(ALARM_BATT_FOCUS_t focusIndex)
{
    uint8_t numAlarmItems = NUMBER_ALARM_BATT_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    // Index two fewer lines when power save mode is off as
    // power save related controls are not shown
    if (POWER_SAVE_OFF == gSetup.PowerSaveMode)
    {
        numAlarmItems -= 2;
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
static ALARM_BATT_FOCUS_t decrementAlarmFocusIndex(ALARM_BATT_FOCUS_t focusIndex)
{
    uint8_t numAlarmItems = NUMBER_ALARM_BATT_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    // Index two fewer lines when power save mode is off as
    // power save related controls are not shown
    if (POWER_SAVE_OFF == gSetup.PowerSaveMode)
    {
        numAlarmItems -= 2;
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
// drawAlarmScreenBattery - Draw the rest of the alarm battery screen
// **********************************************************************************************************
static void drawAlarmScreenBattery(ALARM_BATT_FOCUS_t focusIndex)
{
    gsetcpos(0, 1);
    gputs("POWER SAVINGS");    
    
    if (POWER_SAVE_OFF != gSetup.PowerSaveMode)
    {
        //wrong assumption about the boxes, unhide them
        (*gBatteryWarningDigitBox).isHidden = FALSE;
        (*gBatteryShutoffDigitBox).isHidden = FALSE;
        
        gsetcpos(0, 3);
        gputs("POWER SAVE ON                V");

        gsetcpos(0, 4);
        gputs("BATTERY SHUTOFF            V");
        
        (*gBatteryWarningDigitBox).isHidden = FALSE;
        (*gBatteryShutoffDigitBox).isHidden = FALSE;
    }
    else
    {
        (*gBatteryWarningDigitBox).isHidden = TRUE;
        (*gBatteryShutoffDigitBox).isHidden = TRUE;
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
        
    powerSaveMode = gSetup.PowerSaveMode;
    batteryWarning = gSetup.BatteryWarningTrigger / CONFIG_TRIGGER_TO_DECIVOLTS;
    batteryShutoff = gSetup.BatteryShutoffTrigger / CONFIG_TRIGGER_TO_DECIVOLTS;

    //load shared digit boxes
    gBatteryWarningDigitBox = &digitBox1;
    gBatteryShutoffDigitBox = &digitBox2;
    //The unhiding of these boxes is taken care of in the draw function
    (void)LoadCountDigit(&(*gBatteryWarningDigitBox).countDigit, batteryWarning, NUM_BATTERY_DIGITS, DECIMAL_POINT_ONE_DIGIT, 16, 3, FALSE, FALSE);
    (void)LoadCountDigit(&(*gBatteryShutoffDigitBox).countDigit, batteryShutoff, NUM_BATTERY_DIGITS, DECIMAL_POINT_ONE_DIGIT, 16, 4, FALSE, FALSE);
    
    //load shared select boxes
    gPowerSavingSelectBox = &selectBox1;
    (void) selectBoxConfigure(gPowerSavingSelectBox, 0, NUMBER_POWER_SAVE_MODES, FALSE, FALSE, FALSE, FALSE, 13, 1, 7, ppPowerSavingSelectBoxTextList);

    (*gPowerSavingSelectBox).index = powerSaveMode;
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    alarmBattFocusIndex = FOCUS_POWER_SAVE;
    *isFocusArray[alarmBattFocusIndex] = TRUE;   
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
    (void)LoadCountDigit(&(*gBatteryWarningDigitBox).countDigit, batteryWarning, NUM_BATTERY_DIGITS, DECIMAL_POINT_ONE_DIGIT, 16, 3, FALSE, FALSE);
    (void)LoadCountDigit(&(*gBatteryShutoffDigitBox).countDigit, batteryShutoff, NUM_BATTERY_DIGITS, DECIMAL_POINT_ONE_DIGIT, 16, 4, FALSE, FALSE);
    (*gPowerSavingSelectBox).index = powerSaveMode;
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( alarmBattFocusIndex )
    {
        case FOCUS_POWER_SAVE:
            if( (*gPowerSavingSelectBox).isEditMode == TRUE )
            {
                powerSaveMode = (*gPowerSavingSelectBox).index;
                (*gPowerSavingSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, PowerSaveMode), (DistVarType)powerSaveMode);
                PublishUint32(TOPIC_PowerSaveMode, gSetup.PowerSaveMode);
                PMP_resetStates();
            }
            else
            {
                (*gPowerSavingSelectBox).isEditMode = TRUE;
            }
            break;        
        
        case FOCUS_BATTERY_WARNING:
            if( (*gBatteryWarningDigitBox).isEditMode == TRUE )
            {
                batteryWarning = GetCountDigitValue(&(*gBatteryWarningDigitBox).countDigit);
                (*gBatteryWarningDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, BatteryWarningTrigger), (DistVarType)(batteryWarning * CONFIG_TRIGGER_TO_DECIVOLTS));
                PublishUint32(TOPIC_BatteryWarningTrigger, gSetup.BatteryWarningTrigger);
            }
            else
            {
                (*gBatteryWarningDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gBatteryWarningDigitBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_BATTERY_SHUTOFF:
            if( (*gBatteryShutoffDigitBox).isEditMode == TRUE )
            {
                batteryShutoff = GetCountDigitValue(&(*gBatteryShutoffDigitBox).countDigit);
                (*gBatteryShutoffDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, BatteryShutoffTrigger), (DistVarType)(batteryShutoff * CONFIG_TRIGGER_TO_DECIVOLTS));
                PublishUint32(TOPIC_LowBatteryTrigger, gSetup.BatteryShutoffTrigger);
            }
            else
            {
                (*gBatteryShutoffDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gBatteryShutoffDigitBox).isEditMode = TRUE;
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
    if( upEventForSelectBox() == FALSE &&  upEventForDigitBox() == FALSE)
    {
       alarmBattFocusIndex = decrementAlarmFocusIndex(alarmBattFocusIndex);
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
        alarmBattFocusIndex = incrementAlarmFocusIndex(alarmBattFocusIndex);
    }
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_POWER_SAVE] = &((*gPowerSavingSelectBox).isFocus);    
    isFocusArray[FOCUS_BATTERY_WARNING] = &((*gBatteryWarningDigitBox).isFocus);
    isFocusArray[FOCUS_BATTERY_SHUTOFF] = &((*gBatteryShutoffDigitBox).isFocus);
}
