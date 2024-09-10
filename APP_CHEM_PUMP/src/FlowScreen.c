// FlowScreen.c

// Copyright 2014 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the flow rate pump settings screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "gdisp.h"
#include "screensTask.h"
#include "FlowScreen.h"
#include "CountDigit.h"
#include "pumpControlTask.h"
#include "AdvancedScreen.h"
#include "assert.h"
#include "TimeDigit.h"
#include "PublishSubscribe.h"
#include "screenStuff.h"
#include "utilities.h"
#include "SystemTask.h"
#include "volumeTask.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

#define NUM_FLOW_RATE_DIGITS_GPD    5u
#define NUM_FLOW_RATE_DIGITS_LPD    5u
#define NUM_K_FACTOR_DIGITS         4u
#define NUM_RPM_DIGITS              3u
#define MIN_RPM                     1u

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_METERING_MODE = 0,
    FOCUS_FLOW_RATE,
    FOCUS_K_FACTOR,
    FOCUS_INTERVAL,
    FOCUS_CYCLE_SWITCH,
    FOCUS_RPM_OR_TIMEOUT,
    NUMBER_VOLUME_ITEMS
} FLOW_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

static DIGIT_BOX_t * gFLowRateDigitBox;
static DIGIT_BOX_t * gKFactorDigitBox;
static DIGIT_BOX_t * gRpmDigitBox;

static SELECTION_BOX_t* gIntervalSelectBox;
static SELECTION_BOX_t* gMeteringModeSelectBox;
static SELECTION_BOX_t* gCycleSwitchSelectBox;

static char* ppIntervalSelectBoxTextList[] =
{
    "SHORT",
    "MEDIUM",
    "LONG"
};

static char* ppMeteringModeSelectBoxTextList[] =
{
    "FLOW",
    "TIME",
    "CYCLES"
};

static char* ppCycleSwitchSelectBoxTextList[] = 
{
    "NO",
    "YES",
};

static TIME_DIGIT_BOX_t* gTimeoutDigitBox;

static FLOW_FOCUS_t flowFocusIndex = FOCUS_METERING_MODE;
static METERING_MODE_t meteringMode = METERING_MODE_VOLUME;  
static INT32U flowRate;
static INT32U pumpKFactor = 0;
static INTERVAL_INDEX_t interval;
static CYCLE_SW_t cycleSwitchEnable;
static INT16U rpmNameplate = DEFAULT_PUMP_RPM;      // Never let this be zero to avoid divide by zero errors
static INT32U timeout;
static INPUT_MODE_t ReturnMode;

static bool* isFocusArray[NUMBER_VOLUME_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawVolumeScreen(FLOW_FOCUS_t index);
static FLOW_FOCUS_t incrementFlowFocusIndex(FLOW_FOCUS_t focusIndex);
static FLOW_FOCUS_t decrementFlowFocusIndex(FLOW_FOCUS_t focusIndex);
static INT32U validateTimeInput(INT32U inputTime);
static void LoadFlowRateDigitBox(void);
static void loadDigitBoxes(void);

static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void processInputRefreshScreenEvent(void);

static void loadIsFocusArray(void);

// **********************************************************************************************************
// FlowScreen - The main handler for the flow screen display
// **********************************************************************************************************

INPUT_MODE_t FlowScreen(INPUT_EVENT_t InputEvent)
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
    
    // Unless something changes it return select mode
    ReturnMode = INPUT_MODE_FLOW;

    // Process based on input event
    (void)(*processInputEvent[InputEvent])();
    
    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawVolumeScreen(flowFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementFlowFocusIndex - Move focus to the next field
// **********************************************************************************************************
static FLOW_FOCUS_t incrementFlowFocusIndex(FLOW_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex < (NUMBER_VOLUME_ITEMS - 1) )
    {
        focusIndex = focusIndex + 1;
        //check to see if the FlowRate is supposed to be hiden and skip over the index if it is
        if((gSetup.AnalogInControl == AIN_FLOW_RATE)&& focusIndex == FOCUS_FLOW_RATE)
        {
            ++focusIndex;
        }
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

static FLOW_FOCUS_t decrementFlowFocusIndex(FLOW_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
        //check to see if the FlowRate is supposed to be hidden and skip over the index if it is
        if((gSetup.AnalogInControl == AIN_FLOW_RATE)&& focusIndex == FOCUS_FLOW_RATE)
        {
            --focusIndex;
        }
    }
    else
    {
        focusIndex = (NUMBER_VOLUME_ITEMS - 1);
    }
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawVolumeScreen - Draw the rest of the flow screen
// **********************************************************************************************************

static void drawVolumeScreen(FLOW_FOCUS_t focusIndex)
{
    //check to see if the FlowRate is supposed to be hidden
    if(gSetup.AnalogInControl != AIN_FLOW_RATE)
    {    
        gsetcpos(0, 3);
        if (gSetup.Units == UNITS_METRIC)
        {
            gputs("FLOW RATE             LPD");
        }
        else
        {
            gputs("FLOW RATE             GPD");
        }

    }
    
    gsetcpos(0, 4);
    gputs("K-FACTOR");

    gsetcpos(0, 5);
    gputs("INTERVAL");
    
    gsetcpos(0, 6);
    gputs("CYCLE SW");
    
    // Display RPM box if cycle sw = NO or TIMEOUT box if cycle sw = YES
    if(gSetup.CycleSwitchEnable == CYCLE_SW_NO)
    {
        (*gTimeoutDigitBox).isHidden = TRUE;
        (*gRpmDigitBox).isHidden = FALSE;
        gsetcpos(0, 7);
        gputs("RPM");
    }
    else if(gSetup.CycleSwitchEnable == CYCLE_SW_YES)
    {
        (*gTimeoutDigitBox).isHidden = FALSE;
        (*gRpmDigitBox).isHidden = TRUE;
        gsetcpos(0, 7);
        gputs("TIMEOUT");
    }    

    drawAllDigitBoxes();
    drawAllTimeBoxes();
    drawAllSelectBoxes();
}

// **********************************************************************************************************
// setLocalFlowRate - Set the flow rate using the local units
// **********************************************************************************************************
void setLocalFlowRate(INT32U flowRate, INT32U dvarID)
{
    // Flow rate is stored as a 5 digit int, xxx.xx * 100.  Gallons  are entered as 5 digits xxx.xx * 100.
    // Liters are entered as 4 digits xxx.x * 10, so we need to scale the liters by 10

    if (gSetup.Units == UNITS_METRIC)
    {
        flowRate *=10;   //need to have 2 decimal places
        flowRate = litersToGallons(flowRate);
    }
    
    // Purposely converted to gallons first so we can limit check in gallons
    if (flowRate <= MIN_FLOW_RATE_GAL)
    {
        flowRate = MIN_FLOW_RATE_GAL;
    }
    else if (flowRate >= MAX_FLOW_RATE_GAL)
    {
        flowRate = MAX_FLOW_RATE_GAL;
    }

    (void)DVAR_SetPointLocal(dvarID,(DistVarType)flowRate);
}

// **********************************************************************************************************
// validateTimeInput - Limit checking for the time settings
// **********************************************************************************************************

static INT32U validateTimeInput(INT32U inputTime)
{
    if( inputTime > MAX_TIME )
    {
        inputTime = MAX_TIME;
    }

    return inputTime;
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
    
    interval = gSetup.VolumeModeInterval;
    flowFocusIndex = FOCUS_METERING_MODE;
	meteringMode = gSetup.MeteringMode;
    rpmNameplate = gSetup.RpmNameplate;
    cycleSwitchEnable = gSetup.CycleSwitchEnable;

    //load shared digit boxes
    gFLowRateDigitBox = &digitBox1;
    gKFactorDigitBox = &digitBox2;
    gRpmDigitBox = &digitBox3;
   
    //load shared time boxes
	gTimeoutDigitBox = &timeBox1;
    
    loadDigitBoxes();

    //load shared select boxes
    gIntervalSelectBox = &selectBox1;
	gMeteringModeSelectBox = &selectBox2;
    gCycleSwitchSelectBox = &selectBox3;
    (void) selectBoxConfigure(gIntervalSelectBox, 0, NUMBER_INTERVALS, FALSE, FALSE, FALSE, FALSE, 10, 5, 7, ppIntervalSelectBoxTextList);    
    (void) selectBoxConfigure(gMeteringModeSelectBox, 0, NUMBER_METERING_MODES, FALSE, FALSE, FALSE, TRUE, 1, 1, 10, ppMeteringModeSelectBoxTextList);    	
    (void) selectBoxConfigure(gCycleSwitchSelectBox, 0, NUMBER_CYCLE_SW_ITEMS, FALSE, FALSE, FALSE, FALSE, 10, 6, 5, ppCycleSwitchSelectBoxTextList);
    (*gIntervalSelectBox).index = interval;
	(*gMeteringModeSelectBox).index = meteringMode;
    (*gCycleSwitchSelectBox).index = cycleSwitchEnable;
    
    //unhide the required boxes

    //check to see if the FlowRate is supposed to be hidden
    if(gSetup.AnalogInControl == AIN_FLOW_RATE)
    { 
        (*gFLowRateDigitBox).isHidden = TRUE;
    }
    else
    {
        (*gFLowRateDigitBox).isHidden = FALSE;
    }
    
    (*gKFactorDigitBox).isHidden = FALSE;
    (*gRpmDigitBox).isHidden = FALSE;
    (*gTimeoutDigitBox).isHidden = FALSE;
   
	//load the is focus array
    loadIsFocusArray();
	
    //set the first object to be in focus
    *isFocusArray[flowFocusIndex] = TRUE;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if ( (anyDigitBoxIsEdit() == FALSE) &&
         (anySelectBoxIsEdit() == FALSE) && ((*gTimeoutDigitBox).isEditMode == FALSE))
    {
        ReturnMode = INPUT_MODE_CONFIG;
        clearAllIsFocus();
        hideAllBoxes();
    }
	(void) clearAllSelectBoxIsEdit();
    loadDigitBoxes();
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( flowFocusIndex )
    {
		case FOCUS_METERING_MODE:
			if( (*gMeteringModeSelectBox).isEditMode == TRUE )
			{
				meteringMode = (*gMeteringModeSelectBox).index;
				(*gMeteringModeSelectBox).isEditMode = FALSE;
				(void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MeteringMode), (DistVarType)meteringMode);
				PublishUint32(TOPIC_MeteringMode, gSetup.MeteringMode + 1);
				PMP_resetStates();
				switch(meteringMode)
				{
					case METERING_MODE_VOLUME:
						ReturnMode = INPUT_MODE_FLOW;
						break;
					case METERING_MODE_TIME:
						ReturnMode = INPUT_MODE_TIME;
						break;
					case METERING_MODE_CYCLES:
						ReturnMode = INPUT_MODE_CYCLES;
						break;
					default:
						break;
				}						
			}
			else
			{
				(*gMeteringModeSelectBox).isEditMode = TRUE;
			}                    
			break; 
			
        case FOCUS_FLOW_RATE:
            if( (*gFLowRateDigitBox).isEditMode == TRUE )
            {
                flowRate = GetCountDigitValue(&(*gFLowRateDigitBox).countDigit);
                (*gFLowRateDigitBox).isEditMode = FALSE;
                setLocalFlowRate(flowRate, DVA17G721_SS( gSetup, DesiredFlowRate));
                PublishUint32(TOPIC_FlowRate, gSetup.DesiredFlowRate);
                loadDigitBoxes();
                PMP_resetStates();
            }
            else
            {
                (*gFLowRateDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gFLowRateDigitBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_K_FACTOR:
            if( (*gKFactorDigitBox).isEditMode == TRUE )
            {
                pumpKFactor = GetCountDigitValue(&(*gKFactorDigitBox).countDigit);
                (*gKFactorDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, KFactor), (DistVarType)pumpKFactor);
                PMP_resetStates();
            }
            else
            {
                (*gKFactorDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gKFactorDigitBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_INTERVAL:
            if( (*gIntervalSelectBox).isEditMode == TRUE )
            {
                interval = (*gIntervalSelectBox).index;
                (*gIntervalSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, VolumeModeInterval), (DistVarType)interval);
                PMP_resetStates();
            }
            else
            {
                (*gIntervalSelectBox).isEditMode = TRUE;
            }
            break;
            
        case FOCUS_CYCLE_SWITCH:
            if( (*gCycleSwitchSelectBox).isEditMode == TRUE )
            {
                cycleSwitchEnable = (*gCycleSwitchSelectBox).index;
                (*gCycleSwitchSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, CycleSwitchEnable), (DistVarType)cycleSwitchEnable);
                // set focus for either the rpm or timeout box depending on what's selected for cycle switch
                loadIsFocusArray();
            }
            else
            {
                (*gCycleSwitchSelectBox).isEditMode = TRUE;
            }
            break;
            
        case FOCUS_RPM_OR_TIMEOUT:
            if(gSetup.CycleSwitchEnable == CYCLE_SW_NO)
            {
                // RPM is displayed
                if( (*gRpmDigitBox).isEditMode == TRUE )
                {
                    rpmNameplate = GetCountDigitValue(&(*gRpmDigitBox).countDigit);
                    (*gRpmDigitBox).isEditMode = FALSE;
                    if (rpmNameplate <= MIN_RPM)
                    {
                        // Don't allow 0 RPM
                        rpmNameplate = MIN_RPM;
                    }
                    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, RpmNameplate), (DistVarType)rpmNameplate);
                    loadDigitBoxes();
                    VOL_updateRpm(rpmNameplate);
                    // May not need to reset everything but we do need to reset the on timeout & count on the volume on state to not start it if this is enabled (>0)
                    PMP_resetStates();
                }
                else
                {
                    (*gRpmDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10;
                    (*gRpmDigitBox).isEditMode = TRUE;
                }
                
            }
            else if(gSetup.CycleSwitchEnable == CYCLE_SW_YES)
            {
                // Timeout is displayed
                if( (*gTimeoutDigitBox).isEditMode == TRUE )
                {
                    (*gTimeoutDigitBox).isEditMode = FALSE;
                    timeout = GetTimeDigitValue(&(*gTimeoutDigitBox).timeDigit);
                    if(timeout != gSetup.OnTimeout)
                    {
                        timeout = validateTimeInput(timeout);
                        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OnTimeout), (DistVarType)timeout);
                        PMP_resetStates();
                        PublishUint32(TOPIC_OnTimeout, gSetup.OnTimeout);
                        (void)LoadTimeDigit(&(*gTimeoutDigitBox).timeDigit, timeout, 6, 10, 7);
                    }
                }
                else
                {
                    (*gTimeoutDigitBox).timeDigit.DigitSelected = COUNT_DIGIT_100;
                    (*gTimeoutDigitBox).isEditMode = TRUE;
                }
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
        flowFocusIndex = decrementFlowFocusIndex(flowFocusIndex);
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
        flowFocusIndex = incrementFlowFocusIndex(flowFocusIndex);
    }
}

//****************************************************************************//
//Fcn: processInputRefreshScreenEvent
//
//Desc: This handles the refreshing screen event
//****************************************************************************//
static void processInputRefreshScreenEvent(void)
{
    if( (*gFLowRateDigitBox).isEditMode == FALSE )
    {
       LoadFlowRateDigitBox();
    } 
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_METERING_MODE] = &((*gMeteringModeSelectBox).isFocus);	
    isFocusArray[FOCUS_FLOW_RATE] = &((*gFLowRateDigitBox).isFocus);
    isFocusArray[FOCUS_K_FACTOR] = &((*gKFactorDigitBox).isFocus);
    isFocusArray[FOCUS_INTERVAL] = &((*gIntervalSelectBox).isFocus);
    isFocusArray[FOCUS_CYCLE_SWITCH] = &((*gCycleSwitchSelectBox).isFocus);
    
    if(gSetup.CycleSwitchEnable == CYCLE_SW_NO)
    {
        isFocusArray[FOCUS_RPM_OR_TIMEOUT] = &((*gRpmDigitBox).isFocus);
    }
    else if(gSetup.CycleSwitchEnable == CYCLE_SW_YES)
    {
        isFocusArray[FOCUS_RPM_OR_TIMEOUT] = &((*gTimeoutDigitBox).isFocus);
    }
}

static void loadDigitBoxes()
{
    pumpKFactor = gSetup.KFactor;
    timeout = gSetup.OnTimeout;
	rpmNameplate = gSetup.RpmNameplate;

    LoadFlowRateDigitBox();
    (void)LoadCountDigit(&(*gKFactorDigitBox).countDigit, pumpKFactor, NUM_K_FACTOR_DIGITS, NO_DECIMAL_POINT, 10, 4, FALSE, FALSE);
    (void)LoadCountDigit(&(*gRpmDigitBox).countDigit, rpmNameplate, NUM_RPM_DIGITS, NO_DECIMAL_POINT, 10, 7, FALSE, FALSE);
    (void)LoadTimeDigit(&(*gTimeoutDigitBox).timeDigit, timeout, 6, 10, 7);
}

static void LoadFlowRateDigitBox(void)
{
    flowRate = getLocalFlowRate(gSetup.DesiredFlowRate);
        
    if (gSetup.Units == UNITS_METRIC)
    {
        (void)LoadCountDigit(&(*gFLowRateDigitBox).countDigit, flowRate, NUM_FLOW_RATE_DIGITS_LPD, DECIMAL_POINT_ONE_DIGIT, 10, 3, FALSE, FALSE);
    }
    else
    {
        (void)LoadCountDigit(&(*gFLowRateDigitBox).countDigit, flowRate, NUM_FLOW_RATE_DIGITS_GPD, DECIMAL_POINT_TWO_DIGIT, 10, 3, FALSE, FALSE);    
    }
}

