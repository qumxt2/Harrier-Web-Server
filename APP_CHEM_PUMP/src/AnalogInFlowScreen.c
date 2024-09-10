// AnalogInFlowScreen.c

// Copyright 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the adjustable analog flow control pump settings screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "gdisp.h"
#include "screensTask.h"
#include "AnalogInFlowScreen.h"
#include "CountDigit.h"
#include "pumpControlTask.h"
#include "AdvancedScreen.h"
#include "assert.h"
#include "TimeDigit.h"
#include "PublishSubscribe.h"
#include "screenStuff.h"
#include "utilities.h"
#include "FlowScreen.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

#define NUM_FLOW_RATE_DIGITS_GPD    5u
#define NUM_FLOW_RATE_DIGITS_LPD    5u
#define NUM_MA_DIGITS               5u

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_LOW_MA_SETTING = 0,
    FOCUS_LOW_MA_FLOW_SETPOINT,
    FOCUS_HIGH_MA_SETTING,
    FOCUS_HIGH_MA_FLOW_SETPOINT,
    NUMBER_ANALOG_FLOW_CONTROL_ITEMS
} ANALOG_FLOW_CONTROL_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

static DIGIT_BOX_t * gLowmAFlowSetpointDigitBox;
static DIGIT_BOX_t * gHighmAFlowSetpointDigitBox;
static DIGIT_BOX_t * gLowmASetpointDigitBox;
static DIGIT_BOX_t * gHighmASetpointDigitBox;

static ANALOG_FLOW_CONTROL_FOCUS_t analogFlowControlFocusIndex = FOCUS_LOW_MA_SETTING;

static INT32U flowRateLowmA;
static INT32U flowRateHighmA;
static INT16U lowmASetting;
static INT16U highmASetting;

static INPUT_MODE_t ReturnMode;

static bool* isFocusArray[NUMBER_ANALOG_FLOW_CONTROL_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawAnalogInFlowScreen(ANALOG_FLOW_CONTROL_FOCUS_t index);
static ANALOG_FLOW_CONTROL_FOCUS_t incrementFlowFocusIndex(ANALOG_FLOW_CONTROL_FOCUS_t focusIndex);
static ANALOG_FLOW_CONTROL_FOCUS_t decrementFlowFocusIndex(ANALOG_FLOW_CONTROL_FOCUS_t focusIndex);

static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void processInputRefreshScreenEvent(void);

static void loadCountDigitBoxes(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// FlowScreen - The main handler for the flow screen display
// **********************************************************************************************************

INPUT_MODE_t AnalogInFlowScreen(INPUT_EVENT_t InputEvent)
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
    processInputEvent[INPUT_EVENT_REFRESH_SCREEN] = processInputRefreshScreenEvent;
    
    // Unless something changes it return to the same screen
    ReturnMode = INPUT_MODE_AIN_FLOW;

    // Process based on input event
    (void)(*processInputEvent[InputEvent])();
    
    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawAnalogInFlowScreen(analogFlowControlFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementFlowFocusIndex - Move focus to the next field
// **********************************************************************************************************
static ANALOG_FLOW_CONTROL_FOCUS_t incrementFlowFocusIndex(ANALOG_FLOW_CONTROL_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex < (NUMBER_ANALOG_FLOW_CONTROL_ITEMS - 1) )
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
// decrementFlowFocusIndex - Move focus to the previous field
// **********************************************************************************************************

static ANALOG_FLOW_CONTROL_FOCUS_t decrementFlowFocusIndex(ANALOG_FLOW_CONTROL_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = (NUMBER_ANALOG_FLOW_CONTROL_ITEMS - 1);
    }
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawVolumeScreen - Draw the rest of the flow screen
// **********************************************************************************************************

static void drawAnalogInFlowScreen(ANALOG_FLOW_CONTROL_FOCUS_t focusIndex)
{
    gsetcpos(0, 2);
    gputs("LOW SETPOINT             mA");

    gsetcpos(0, 3);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("LOW mA FLOW              LPD");
    }
    else
    {
        gputs("LOW mA FLOW              GPD");
    }
     
    gsetcpos(0, 5);
    gputs("HIGH SETPOINT            mA");
    
    gsetcpos(0, 6);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("HIGH mA FLOW             LPD");
    }
    else
    {
        gputs("HIGH mA FLOW             GPD");
    }

    drawAllDigitBoxes();
    drawAllTimeBoxes();
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
    
    analogFlowControlFocusIndex  = FOCUS_LOW_MA_SETTING;

    //load shared digit boxes
    gLowmASetpointDigitBox = &digitBox1;
    gLowmAFlowSetpointDigitBox = &digitBox2;
    gHighmASetpointDigitBox = &digitBox3;
    gHighmAFlowSetpointDigitBox = &digitBox4;
    
    
    loadCountDigitBoxes();
    
    //unhide the required boxes
	(*gLowmASetpointDigitBox).isHidden = FALSE;
    (*gLowmAFlowSetpointDigitBox).isHidden = FALSE;
    (*gHighmASetpointDigitBox).isHidden = FALSE;
    (*gHighmAFlowSetpointDigitBox).isHidden = FALSE;
    
	//load the is focus array
    loadIsFocusArray();
	
    //set the first object to be in focus
    *isFocusArray[FOCUS_LOW_MA_SETTING] = TRUE;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if ( (anyDigitBoxIsEdit() == FALSE))
    {
        ReturnMode = INPUT_MODE_ADVANCED;
        clearAllIsFocus();
        hideAllBoxes();
    }
	(void)clearAllSelectBoxIsEdit();
    loadCountDigitBoxes();
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( analogFlowControlFocusIndex  )
    {
        case FOCUS_LOW_MA_SETTING :
            if( (*gLowmASetpointDigitBox).isEditMode == TRUE )
            {
                lowmASetting = GetCountDigitValue(&(*gLowmASetpointDigitBox).countDigit);
                if(lowmASetting < MA_MIN_SETTING)
                {
                    lowmASetting  = MA_MIN_SETTING;
                }
                else if(lowmASetting >= gSetup.HighmASetpoint)
                {
                    lowmASetting = gSetup.LowmASetpoint;
                }
                (*gLowmASetpointDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, LowmASetpoint), (DistVarType)lowmASetting);
                PMP_resetStates();
                loadCountDigitBoxes();
            }
            else
            {
                (*gLowmASetpointDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gLowmASetpointDigitBox).isEditMode = TRUE;
            }
            break; 
            
		case FOCUS_LOW_MA_FLOW_SETPOINT :
			if( (*gLowmAFlowSetpointDigitBox).isEditMode == TRUE )
            {
                flowRateLowmA = GetCountDigitValue(&(*gLowmAFlowSetpointDigitBox).countDigit);
                (*gLowmAFlowSetpointDigitBox).isEditMode = FALSE;
                setLocalFlowRate(flowRateLowmA, DVA17G721_SS(gSetup, FlowRateLowmASetpoint));
                PMP_resetStates();
                loadCountDigitBoxes();
            }
            else
            {
                (*gLowmAFlowSetpointDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10000;
                (*gLowmAFlowSetpointDigitBox).isEditMode = TRUE;
            }
            break; 
			
        case FOCUS_HIGH_MA_SETTING:
            if( (*gHighmASetpointDigitBox).isEditMode == TRUE )
            {
                highmASetting = GetCountDigitValue(&(*gHighmASetpointDigitBox).countDigit);
                if(highmASetting > MA_MAX_SETTING)
                {
                    highmASetting  = MA_MAX_SETTING;
                }
                else if(highmASetting <= gSetup.LowmASetpoint)
                {
                    highmASetting = gSetup.HighmASetpoint;
                }
                (*gHighmASetpointDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, HighmASetpoint), (DistVarType)highmASetting);
                PMP_resetStates();
                loadCountDigitBoxes();
            }
            else
            {
                (*gHighmASetpointDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gHighmASetpointDigitBox).isEditMode = TRUE;
            }
            break; 
            
        case FOCUS_HIGH_MA_FLOW_SETPOINT :
            if( (*gHighmAFlowSetpointDigitBox).isEditMode == TRUE )
            {
                flowRateHighmA = GetCountDigitValue(&(*gHighmAFlowSetpointDigitBox).countDigit);
                (*gHighmAFlowSetpointDigitBox).isEditMode = FALSE;
                setLocalFlowRate(flowRateHighmA, DVA17G721_SS(gSetup, FlowRateHighmASetpoint));
                PMP_resetStates();
                loadCountDigitBoxes();
            }
            else
            {
                (*gHighmAFlowSetpointDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10000;
                (*gHighmAFlowSetpointDigitBox).isEditMode = TRUE;
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
    if(upEventForDigitBox() == FALSE &&  upEventForSelectBox() == FALSE && upEventForTimeBox() == FALSE )
    {
        analogFlowControlFocusIndex  = decrementFlowFocusIndex(analogFlowControlFocusIndex );
    }
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if(downEventForDigitBox() == FALSE &&  downEventForSelectBox() == FALSE && downEventForTimeBox() == FALSE )
    {
        analogFlowControlFocusIndex  = incrementFlowFocusIndex(analogFlowControlFocusIndex );
    }
}

//****************************************************************************//
//Fcn: processInputRefreshScreenEvent
//
//Desc: This handles the refreshing screen event
//****************************************************************************//
static void processInputRefreshScreenEvent(void)
{
    if( (*gLowmAFlowSetpointDigitBox).isEditMode == FALSE )
    {
        flowRateLowmA = getLocalFlowRate(gSetup.FlowRateLowmASetpoint);
        if(gSetup.Units == UNITS_METRIC)
        {
            (void)LoadCountDigit(&(*gLowmAFlowSetpointDigitBox).countDigit, flowRateLowmA, NUM_FLOW_RATE_DIGITS_LPD, DECIMAL_POINT_ONE_DIGIT, 12, 3, FALSE, FALSE);
        }
        else
        {
            (void)LoadCountDigit(&(*gLowmAFlowSetpointDigitBox).countDigit, flowRateLowmA, NUM_FLOW_RATE_DIGITS_GPD, DECIMAL_POINT_TWO_DIGIT, 12, 3, FALSE, FALSE);
        }
    }
    if( (*gHighmAFlowSetpointDigitBox).isEditMode == FALSE )
    {
        flowRateHighmA = getLocalFlowRate(gSetup.FlowRateHighmASetpoint);
        if(gSetup.Units == UNITS_METRIC)
        {
            (void)LoadCountDigit(&(*gHighmAFlowSetpointDigitBox).countDigit, flowRateHighmA, NUM_FLOW_RATE_DIGITS_LPD, DECIMAL_POINT_ONE_DIGIT, 12, 6, FALSE, FALSE);
        }
        else
        {
            (void)LoadCountDigit(&(*gHighmAFlowSetpointDigitBox).countDigit, flowRateHighmA, NUM_FLOW_RATE_DIGITS_GPD, DECIMAL_POINT_TWO_DIGIT, 12, 6, FALSE, FALSE);
        }
    }
    if( (*gLowmASetpointDigitBox).isEditMode == FALSE )
    {
        lowmASetting = gSetup.LowmASetpoint;
        (void)LoadCountDigit(&(*gLowmASetpointDigitBox).countDigit, lowmASetting, NUM_MA_DIGITS, DECIMAL_POINT_TWO_DIGIT, 12, 2, FALSE, FALSE);
    }
    if( (*gHighmASetpointDigitBox).isEditMode == FALSE )
    {
        highmASetting = gSetup.HighmASetpoint;
        (void)LoadCountDigit(&(*gHighmASetpointDigitBox).countDigit, highmASetting, NUM_MA_DIGITS, DECIMAL_POINT_TWO_DIGIT, 12, 5, FALSE, FALSE);
    }
}

//****************************************************************************/
//Fcn: loadCountDigitBoxes
//
//Desc: Load all the required values for the count digit boxes
//****************************************************************************//
static void loadCountDigitBoxes(void)
{       
    lowmASetting = gSetup.LowmASetpoint;
    flowRateLowmA = getLocalFlowRate(gSetup.FlowRateLowmASetpoint);
    highmASetting = gSetup.HighmASetpoint;
    flowRateHighmA = getLocalFlowRate(gSetup.FlowRateHighmASetpoint);
    
    (void)LoadCountDigit(&(*gLowmASetpointDigitBox).countDigit, lowmASetting, NUM_MA_DIGITS, DECIMAL_POINT_TWO_DIGIT, 12, 2, FALSE, FALSE);
    (void)LoadCountDigit(&(*gHighmASetpointDigitBox).countDigit, highmASetting, NUM_MA_DIGITS, DECIMAL_POINT_TWO_DIGIT, 12, 5, FALSE, FALSE);
    
    if(gSetup.Units == UNITS_METRIC)
    {
        (void)LoadCountDigit(&(*gLowmAFlowSetpointDigitBox).countDigit, flowRateLowmA, NUM_FLOW_RATE_DIGITS_LPD, DECIMAL_POINT_ONE_DIGIT, 12, 3, FALSE, FALSE);
        (void)LoadCountDigit(&(*gHighmAFlowSetpointDigitBox).countDigit, flowRateHighmA, NUM_FLOW_RATE_DIGITS_LPD, DECIMAL_POINT_ONE_DIGIT, 12, 6, FALSE, FALSE);
    }
    else
    {
        (void)LoadCountDigit(&(*gLowmAFlowSetpointDigitBox).countDigit, flowRateLowmA, NUM_FLOW_RATE_DIGITS_GPD, DECIMAL_POINT_TWO_DIGIT, 12, 3, FALSE, FALSE);
        (void)LoadCountDigit(&(*gHighmAFlowSetpointDigitBox).countDigit, flowRateHighmA, NUM_FLOW_RATE_DIGITS_GPD, DECIMAL_POINT_TWO_DIGIT, 12, 6, FALSE, FALSE);
    }
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_LOW_MA_SETTING] = &((*gLowmASetpointDigitBox).isFocus);	
    isFocusArray[FOCUS_LOW_MA_FLOW_SETPOINT] = &((*gLowmAFlowSetpointDigitBox).isFocus);
    isFocusArray[FOCUS_HIGH_MA_SETTING] = &((*gHighmASetpointDigitBox).isFocus);	
    isFocusArray[FOCUS_HIGH_MA_FLOW_SETPOINT] = &((*gHighmAFlowSetpointDigitBox).isFocus);
}

