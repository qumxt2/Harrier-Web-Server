// AlarmScreenTank.c

#include "stdio.h"


// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the alarm tank screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "CountDigit.h"
#include "AlarmScreenTank.h"
#include "screensTask.h"
#include "screenStuff.h"
#include "AdvancedScreen.h"
#include "TankScreen.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_PERCENTAGE_DIGITS         3u

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************
typedef enum
{
    FOCUS_TANK_NOTIFY = 0,
    FOCUS_TANK_SHUTOFF,
    FOCUS_FLOW_VERIFY_ENABLE,  
    FOCUS_FLOW_VERIFY_PERCENT,
    NUMBER_ALARM_TANK_ITEMS
} ALARM_TANK_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t * gTankLowNotifyDigitBox;
static DIGIT_BOX_t * gTankLowShutoffDigitBox;
static DIGIT_BOX_t * gFlowVerifyPercentDigitBox;

static SELECTION_BOX_t* gFlowVerifySelectBox;

static char* ppFlowVerifySelectBoxTextList[] =
{
    "DISABLED",
    "ENABLED"
};

static INPUT_MODE_t ReturnMode;
static ALARM_TANK_FOCUS_t alarmTankFocusIndex = FOCUS_TANK_NOTIFY;
static uint32 tankLevelNotify = 0;
static uint32 tankLevelShutoff = 0;
static bool isFlowVerifyEnabled= TRUE;
static uint32 flowVerifyPercentage = 0;
static bool* isFocusArray[NUMBER_ALARM_TANK_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawAlarmScreenTank(ALARM_TANK_FOCUS_t index);
static ALARM_TANK_FOCUS_t incrementAlarmFocusIndex(ALARM_TANK_FOCUS_t focusIndex);
static ALARM_TANK_FOCUS_t decrementAlarmFocusIndex(ALARM_TANK_FOCUS_t focusIndex);
static void loadDigitBoxes(void);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// AlarmScreenTank - The main handler for the alarm tank screen display
// **********************************************************************************************************
INPUT_MODE_t AlarmScreenTank(INPUT_EVENT_t InputEvent)
{
    ReturnMode = INPUT_MODE_ALARMS_TANK;

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

    drawAlarmScreenTank(alarmTankFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementAlarmFocusIndex - Move focus to the next field
// **********************************************************************************************************
static ALARM_TANK_FOCUS_t incrementAlarmFocusIndex(ALARM_TANK_FOCUS_t focusIndex)
{   
    uint8_t numItems = NUMBER_ALARM_TANK_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if (gSetup.FlowVerifyEnable == FALSE)
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
static ALARM_TANK_FOCUS_t decrementAlarmFocusIndex(ALARM_TANK_FOCUS_t focusIndex)
{   
    uint8_t numItems = NUMBER_ALARM_TANK_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if (gSetup.FlowVerifyEnable == FALSE)
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
// drawAlarmScreenTank - Draw the rest of the alarm tank screen
// **********************************************************************************************************
static void drawAlarmScreenTank(ALARM_TANK_FOCUS_t focusIndex)
{   
    gsetcpos(0, 1);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("TANK NOTIFY");
        gsetcpos(21, 1);
        gputs("L");
    }
    else
    {
        gputs("TANK NOTIFY");
        gsetcpos(20, 1);
        gputs("G");
    }
    
    gsetcpos(0, 2);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("TANK SHUTOFF");
        gsetcpos(21, 2);
        gputs("L");
    }
    else
    {
        gputs("TANK SHUTOFF");
        gsetcpos(20, 2);
        gputs("G");
    }
    
    gsetcpos(0, 3);
    gputs("FLOW VERIFY");
    
    if (gSetup.FlowVerifyEnable)
    {
        gsetcpos(0, 4);
        gputs("VERIFY PERCENT            %");
        (*gFlowVerifyPercentDigitBox).isHidden = FALSE;
    }
    else
    {
        (*gFlowVerifyPercentDigitBox).isHidden = TRUE;
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

    isFlowVerifyEnabled = gSetup.FlowVerifyEnable;
    
    //load shared digit boxes
    gTankLowNotifyDigitBox = &digitBox1;
    gTankLowShutoffDigitBox = &digitBox2;
    gFlowVerifyPercentDigitBox = &digitBox3;
    loadDigitBoxes();
    
    //unhide required boxes
    (*gTankLowNotifyDigitBox).isHidden = FALSE;
    (*gTankLowShutoffDigitBox).isHidden = FALSE;        
    (*gFlowVerifyPercentDigitBox).isHidden = FALSE;
    
    //load shared select boxes
    gFlowVerifySelectBox = &selectBox1;    
    (void) selectBoxConfigure(gFlowVerifySelectBox, 0, NUMBER_FLOW_VERIFY_ITEMS, FALSE, FALSE, FALSE, FALSE, 13, 3, 8, ppFlowVerifySelectBoxTextList);
    
    (*gFlowVerifySelectBox).index = isFlowVerifyEnabled;
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    alarmTankFocusIndex = FOCUS_TANK_NOTIFY;
    *isFocusArray[alarmTankFocusIndex] = TRUE;   
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
    loadDigitBoxes();
    (*gFlowVerifySelectBox).index = isFlowVerifyEnabled;
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( alarmTankFocusIndex )
    {
		case FOCUS_TANK_NOTIFY:
			if( (*gTankLowNotifyDigitBox).isEditMode == TRUE)
			{
				tankLevelNotify = GetCountDigitValue(&(*gTankLowNotifyDigitBox).countDigit);
				(*gTankLowNotifyDigitBox).isEditMode = FALSE;
                // Check the screen inputs so it doesn't overflow while converting L to G
                // Note: Gallons are limited to 9999.9 G by the digit box so no need to check gallons
                if (gSetup.Units == UNITS_METRIC)
                {
                    if (tankLevelNotify > MAX_VOLUME_L)
                    {
                        tankLevelNotify = MAX_VOLUME_L;
                    }
                }
                tankLevelNotify = setLocalVolume(tankLevelNotify);
				(void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TankLevelNotifyTrigger), (DistVarType)tankLevelNotify);
                PublishUint32(TOPIC_TankLevelNotifyTrigger, gSetup.TankLevelNotifyTrigger);
                loadDigitBoxes();
			}
			else
			{                        
                (*gTankLowNotifyDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10000;
                (*gTankLowNotifyDigitBox).isEditMode = TRUE;
			}
			break;

		case FOCUS_TANK_SHUTOFF:
			if( (*gTankLowShutoffDigitBox).isEditMode == TRUE)
			{
				tankLevelShutoff = GetCountDigitValue(&(*gTankLowShutoffDigitBox).countDigit);
				(*gTankLowShutoffDigitBox).isEditMode = FALSE;
                // Check the screen inputs so it doesn't overflow while converting L to G
                // Note: Gallons are limited to 9999.9 G by the digit box so no need to check gallons
                if (gSetup.Units == UNITS_METRIC)
                {
                    if (tankLevelShutoff > MAX_VOLUME_L)
                    {
                        tankLevelShutoff = MAX_VOLUME_L;
                    }
                }
				tankLevelShutoff = setLocalVolume(tankLevelShutoff);
				(void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, TankLevelShutoffTrigger), (DistVarType)tankLevelShutoff);
                PublishUint32(TOPIC_TankLevelShutoffTrigger, gSetup.TankLevelShutoffTrigger);
                loadDigitBoxes();
			}
			else
			{       
                (*gTankLowShutoffDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10000;
                (*gTankLowShutoffDigitBox).isEditMode = TRUE;
			}
			break;
            
            case FOCUS_FLOW_VERIFY_ENABLE:
                if( (*gFlowVerifySelectBox).isEditMode == TRUE)
                {
                    isFlowVerifyEnabled = (*gFlowVerifySelectBox).index;
                    (*gFlowVerifySelectBox).isEditMode = FALSE;                  
                    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, FlowVerifyEnable), (DistVarType)isFlowVerifyEnabled);
                    PublishUint32(TOPIC_FlowVerifyEnable, gSetup.FlowVerifyEnable);
                }
                else
                {
                    (*gFlowVerifySelectBox).isEditMode = TRUE;
                }
                break;
                
        case FOCUS_FLOW_VERIFY_PERCENT:
			if( (*gFlowVerifyPercentDigitBox).isEditMode == TRUE)
			{
				flowVerifyPercentage = GetCountDigitValue(&(*gFlowVerifyPercentDigitBox).countDigit);
				(*gFlowVerifyPercentDigitBox).isEditMode = FALSE;
				(void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, FlowVerifyPercentage), (DistVarType)flowVerifyPercentage);
				PublishUint32(TOPIC_FlowVerifyPercentage, gSetup.FlowVerifyPercentage);
			}
			else
			{                        
                (*gFlowVerifyPercentDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gFlowVerifyPercentDigitBox).isEditMode = TRUE;
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
       alarmTankFocusIndex = decrementAlarmFocusIndex(alarmTankFocusIndex);
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
        alarmTankFocusIndex = incrementAlarmFocusIndex(alarmTankFocusIndex);
    }
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_TANK_NOTIFY] = &((*gTankLowNotifyDigitBox).isFocus);
    isFocusArray[FOCUS_TANK_SHUTOFF] = &((*gTankLowShutoffDigitBox).isFocus);
    isFocusArray[FOCUS_FLOW_VERIFY_ENABLE] = &((*gFlowVerifySelectBox).isFocus);
    isFocusArray[FOCUS_FLOW_VERIFY_PERCENT] = &((*gFlowVerifyPercentDigitBox).isFocus);
}

static void loadDigitBoxes(void)
{
    uint32 numDigits = NUM_VOLUME_DIGITS_G;
    tankLevelNotify = getLocalVolume(gSetup.TankLevelNotifyTrigger);
    tankLevelShutoff = getLocalVolume(gSetup.TankLevelShutoffTrigger);
    flowVerifyPercentage = gSetup.FlowVerifyPercentage;
    
    // Default to 5 digits for gallons & change to 6 for liters if in metric
    if (gSetup.Units == UNITS_METRIC)
    {
        numDigits = NUM_VOLUME_DIGITS_L;
    }
    
    (void)LoadCountDigit(&(*gTankLowNotifyDigitBox).countDigit, tankLevelNotify, numDigits, DECIMAL_POINT_ONE_DIGIT, 13, 1, FALSE, FALSE);
    (void)LoadCountDigit(&(*gTankLowShutoffDigitBox).countDigit, tankLevelShutoff, numDigits, DECIMAL_POINT_ONE_DIGIT, 13, 2, FALSE, FALSE);
    (void)LoadCountDigit(&(*gFlowVerifyPercentDigitBox).countDigit, flowVerifyPercentage, NUM_PERCENTAGE_DIGITS, NO_DECIMAL_POINT, 16, 4, FALSE, FALSE);
}

