// CyclesScreen.c

// Copyright 2014 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the cycle mode pump settings screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************

#include "pumpControlTask.h"
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "screensTask.h"
#include "gdisp.h"
#include "CountDigit.h"
#include "TimeDigit.h"
#include "CyclesScreen.h"
#include "screenStuff.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

#define MAX_NUMBER_CYCLES       99u
#define MIN_NUMBER_CYCLES       0u
#define NUM_K_FACTOR_DIGITS     4u

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_METERING_MODE = 0,
    FOCUS_ON_CYCLES,
    FOCUS_ON_TIMEOUT,
    FOCUS_OFF_TIME,
    FOCUS_K_FACTOR,
    NUMBER_CYCLES_ITEMS
} CYCLES_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

static DIGIT_BOX_t* gPumpOnCyclesDigit;
static DIGIT_BOX_t* gKFactorDigitBox;

static SELECTION_BOX_t* gMeteringModeSelectBox;

static TIME_DIGIT_BOX_t* gPumpOnTimeoutDigit;
static TIME_DIGIT_BOX_t* gPumpOffTimeDigit;

static char* ppMeteringModeSelectBoxTextList[] =
{
    "FLOW",
    "TIME",
    "CYCLES"
};

static INPUT_MODE_t ReturnMode;
static CYCLES_FOCUS_t cyclesFocusIndex = FOCUS_METERING_MODE;
static METERING_MODE_t meteringMode = METERING_MODE_CYCLES;
static INT32U onCount;
static INT32U offTime;
static INT32U onTimeout;
static INT32U pumpKFactor = 0;
static bool* isFocusArray[NUMBER_CYCLES_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawCyclesScreen(CYCLES_FOCUS_t focusIndex);
static CYCLES_FOCUS_t incrementCyclesFocusIndex(CYCLES_FOCUS_t focusIndex);
static CYCLES_FOCUS_t decrementCyclesFocusIndex(CYCLES_FOCUS_t focusIndex);
static INT32U validateTimeInput(INT32U inputTime);
static INT32U validateCyclesInput(INT32U inputCycles);

static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// CyclesScreen - Main handler for the cycles screen
// **********************************************************************************************************

INPUT_MODE_t CyclesScreen(INPUT_EVENT_t InputEvent)
{
    ReturnMode = INPUT_MODE_CYCLES;

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
	
    (void)(*processInputEvent[InputEvent])();
	
    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawCyclesScreen(cyclesFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementCyclesFocusIndex - Move to the next field on the screen
// **********************************************************************************************************
static CYCLES_FOCUS_t incrementCyclesFocusIndex(CYCLES_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex < (NUMBER_CYCLES_ITEMS - 1) )
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
// decrementCyclesFocusIndex - Move to the previous field on the screen
// **********************************************************************************************************

static CYCLES_FOCUS_t decrementCyclesFocusIndex(CYCLES_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex > 0 )
    {
        focusIndex =  focusIndex - 1;
    }
    else
    {
        focusIndex = (NUMBER_CYCLES_ITEMS - 1);
    }
     //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawCyclesScreen - Draw the cycles screen
// **********************************************************************************************************

static void drawCyclesScreen(CYCLES_FOCUS_t focusIndex)
{
    gsetcpos(0, 3);
    gputs("ON CYCLES");

    gsetcpos(0, 4);
    gputs("ON TIMEOUT");

    gsetcpos(0, 5);
    gputs("OFF TIME");

    gsetcpos(0, 6);
    gputs("K-FACTOR");

    //draw all the required boxes
    drawAllTimeBoxes();
    drawAllDigitBoxes();
	drawAllSelectBoxes();
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

// **********************************************************************************************************
// validateCyclesInput - Limit checking for the number of cycles settign
// **********************************************************************************************************

static INT32U validateCyclesInput(INT32U inputCycles)
{
    if( inputCycles <= MIN_NUMBER_CYCLES )
    {
        inputCycles = MIN_NUMBER_CYCLES;
    }

    if( inputCycles > MAX_NUMBER_CYCLES )
    {
        inputCycles = MAX_NUMBER_CYCLES;
    }

    return inputCycles;
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
	
    onCount = gSetup.OnCycles;
    onTimeout = gSetup.OnTimeout;
    offTime = gSetup.OffTime;    
    pumpKFactor = gSetup.KFactor;
    meteringMode = gSetup.MeteringMode;	
	
	//load shared digit boxes
    gPumpOnCyclesDigit = &digitBox1;
    gKFactorDigitBox = &digitBox2;
    (void)LoadCountDigit(&(*gPumpOnCyclesDigit).countDigit, onCount, 2, NO_DECIMAL_POINT, 11, 3, FALSE, FALSE);
    (void)LoadCountDigit(&(*gKFactorDigitBox).countDigit, pumpKFactor, NUM_K_FACTOR_DIGITS, NO_DECIMAL_POINT, 11, 6, FALSE, FALSE);    
	
	//load shared time boxes
    gPumpOnTimeoutDigit = &timeBox1;    
    gPumpOffTimeDigit = &timeBox2;
    (void)LoadTimeDigit(&(*gPumpOnTimeoutDigit).timeDigit, onTimeout, 6, 11, 4);
    (void)LoadTimeDigit(&(*gPumpOffTimeDigit).timeDigit, offTime, 6, 11, 5);
    
    //load shared select boxes
    gMeteringModeSelectBox = &selectBox1;
    (void) selectBoxConfigure(gMeteringModeSelectBox, 0, NUMBER_METERING_MODES, FALSE, FALSE, FALSE, TRUE, 1, 1, 10, ppMeteringModeSelectBoxTextList);
    (*gMeteringModeSelectBox).index = meteringMode;	
	
    //unhide the required boxes
    (*gPumpOnCyclesDigit).isHidden = FALSE;
    (*gPumpOnTimeoutDigit).isHidden = FALSE; 
    (*gPumpOffTimeDigit).isHidden = FALSE;   
    (*gKFactorDigitBox).isHidden = FALSE;
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    cyclesFocusIndex = FOCUS_METERING_MODE;
    *isFocusArray[cyclesFocusIndex] = TRUE;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if( (anyDigitBoxIsEdit() == FALSE) && (anyTimeBoxIsEdit() == FALSE))
    {
        ReturnMode = INPUT_MODE_CONFIG;
        hideAllBoxes();
        clearAllIsFocus();
    }
    clearAllDigitBoxIsEdit();
    clearAllTimeBoxIsEdit();
	clearAllSelectBoxIsEdit();
    (void)LoadCountDigit(&(*gPumpOnCyclesDigit).countDigit, onCount, 2, NO_DECIMAL_POINT, 11, 3, FALSE, FALSE);
    (void)LoadTimeDigit(&(*gPumpOnTimeoutDigit).timeDigit, onTimeout, 6, 11, 4);
    (void)LoadTimeDigit(&(*gPumpOffTimeDigit).timeDigit, offTime, 6, 11, 5);
    (void)LoadCountDigit(&(*gKFactorDigitBox).countDigit, pumpKFactor, NUM_K_FACTOR_DIGITS, NO_DECIMAL_POINT, 11, 6, FALSE, FALSE);
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( cyclesFocusIndex )
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
		
        case FOCUS_ON_CYCLES:
            if( (*gPumpOnCyclesDigit).isEditMode == TRUE )
            {
                (*gPumpOnCyclesDigit).isEditMode = FALSE;
                onCount = GetCountDigitValue(&(*gPumpOnCyclesDigit).countDigit);
                if( onCount != gSetup.OnCycles )
                {
                    onCount = validateCyclesInput(onCount);
                    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OnCycles), (DistVarType)onCount);
                    PMP_resetStates();
                    PublishUint32(TOPIC_OnCycles, gSetup.OnCycles);
                    (void)LoadCountDigit(&(*gPumpOnCyclesDigit).countDigit, onCount, 2, NO_DECIMAL_POINT, 11, 3, FALSE, FALSE);
                }
            }
            else
            {
                (*gPumpOnCyclesDigit).countDigit.DigitSelected = COUNT_DIGIT_10;
                (*gPumpOnCyclesDigit).isEditMode = TRUE;
            }
            break;

        case FOCUS_ON_TIMEOUT:
            if( (*gPumpOnTimeoutDigit).isEditMode == TRUE )
            {
                (*gPumpOnTimeoutDigit).isEditMode = FALSE;
                onTimeout = GetTimeDigitValue(&(*gPumpOnTimeoutDigit).timeDigit);
                if( onTimeout != gSetup.OnTimeout )
                {
                    onTimeout = validateTimeInput(onTimeout);
                   (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OnTimeout), (DistVarType)onTimeout);
                    PMP_resetStates();
                    PublishUint32(TOPIC_OnTimeout, gSetup.OnTimeout);
                    (void)LoadTimeDigit(&(*gPumpOnTimeoutDigit).timeDigit, onTimeout, 6, 11, 4);
                }
            }
            else
            {
                (*gPumpOnTimeoutDigit).timeDigit.DigitSelected = COUNT_DIGIT_10;
                (*gPumpOnTimeoutDigit).isEditMode = TRUE;
            }
            break;

        case FOCUS_OFF_TIME:
            if( (*gPumpOffTimeDigit).isEditMode == TRUE )
            {
                (*gPumpOffTimeDigit).isEditMode = FALSE;
                offTime = GetTimeDigitValue(&(*gPumpOffTimeDigit).timeDigit);
                if( offTime != gSetup.OffTime )
                {
                    offTime = validateTimeInput(offTime);
                   (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OffTime), (DistVarType)offTime);
                   PMP_resetStates();
                   PublishUint32(TOPIC_OffTime, gSetup.OffTime);
                    (void)LoadTimeDigit(&(*gPumpOffTimeDigit).timeDigit, offTime, 6, 11, 5);
                }
            }
            else
            {
                (*gPumpOffTimeDigit).timeDigit.DigitSelected = COUNT_DIGIT_10;
                (*gPumpOffTimeDigit).isEditMode = TRUE;
            }
            break;

        case FOCUS_K_FACTOR:
            if( (*gKFactorDigitBox).isEditMode == TRUE )
            {
                pumpKFactor = GetCountDigitValue(&(*gKFactorDigitBox).countDigit);
                (*gKFactorDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, KFactor), (DistVarType)pumpKFactor);
            }
            else
            {
                (*gKFactorDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gKFactorDigitBox).isEditMode = TRUE;
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
    if( upEventForDigitBox() == FALSE && upEventForTimeBox() == FALSE && upEventForSelectBox() == FALSE)
    {
        cyclesFocusIndex = decrementCyclesFocusIndex(cyclesFocusIndex);
    }    
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if( downEventForDigitBox() == FALSE && downEventForTimeBox() == FALSE && downEventForSelectBox() == FALSE)
    {
        cyclesFocusIndex = incrementCyclesFocusIndex(cyclesFocusIndex);
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
	isFocusArray[FOCUS_ON_CYCLES] = &((*gPumpOnCyclesDigit).isFocus);
    isFocusArray[FOCUS_ON_TIMEOUT] = &((*gPumpOnTimeoutDigit).isFocus);
    isFocusArray[FOCUS_OFF_TIME] = &((*gPumpOffTimeDigit).isFocus);
    isFocusArray[FOCUS_K_FACTOR] = &((*gKFactorDigitBox).isFocus);
}

