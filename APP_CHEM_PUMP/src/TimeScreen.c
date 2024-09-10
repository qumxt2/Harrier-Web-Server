// TimeScreen.c

// Copyright 2014 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the time pump settings screen

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
#include "TimeScreen.h"
#include "screenStuff.h"
#include "utilities.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

#define MIN_ON_TIME     1
#define MIN_OFF_TIME    0

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_METERING_MODE = 0,
    FOCUS_ON_TIME,
    FOCUS_OFF_TIME,
    NUMBER_TIME_ITEMS
} TIME_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static TIME_DIGIT_BOX_t* gPumpOnTimeDigit;
static TIME_DIGIT_BOX_t* gPumpOffTimeDigit;

static SELECTION_BOX_t* gMeteringModeSelectBox;

static char* ppMeteringModeSelectBoxTextList[] =
{
    "FLOW",
    "TIME",
    "CYCLES"
};

static INPUT_MODE_t ReturnMode = INPUT_MODE_TIME;
static TIME_FOCUS_t timeFocusIndex = FOCUS_ON_TIME;
static METERING_MODE_t meteringMode = METERING_MODE_CYCLES;
static INT32U onTime;
static INT32U offTime;

static bool* isFocusArray[NUMBER_TIME_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawTimeScreen(TIME_FOCUS_t focusIndex);
static TIME_FOCUS_t incrementTimeFocusIndex(TIME_FOCUS_t focusIndex);
static TIME_FOCUS_t decrementTimeFocusIndex(TIME_FOCUS_t focusIndex);

static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);

static void loadIsFocusArray(void);

// **********************************************************************************************************
// TimeScreen - The main handler for the time screen
// **********************************************************************************************************

INPUT_MODE_t TimeScreen(INPUT_EVENT_t InputEvent)
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
    
    (void)(*processInputEvent[InputEvent])();

    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawTimeScreen(timeFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementTimeFocusIndex - Move focus to the next item on the screen
// **********************************************************************************************************
static TIME_FOCUS_t incrementTimeFocusIndex(TIME_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex < (NUMBER_TIME_ITEMS - 1) )
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
// decrementTimeFocusIndex - Move focus to the previos item on the screen
// **********************************************************************************************************

static TIME_FOCUS_t decrementTimeFocusIndex(TIME_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = (NUMBER_TIME_ITEMS - 1);
    }
    
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawTimeScreen - Draw the time screen
// **********************************************************************************************************

static void drawTimeScreen(TIME_FOCUS_t focusIndex)
{
    gsetcpos(0, 3);
    gputs("ON TIME");
    
    gsetcpos(0, 4);
    gputs("OFF TIME");
    
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
	
    ReturnMode = INPUT_MODE_TIME;
    timeFocusIndex = FOCUS_METERING_MODE;
    onTime = gSetup.OnTime;
    offTime = gSetup.OffTime;
	meteringMode = gSetup.MeteringMode;	
	
	//load the shared time boxes
    gPumpOnTimeDigit = &timeBox1;
    gPumpOffTimeDigit = &timeBox2;
    (void)LoadTimeDigit(&(*gPumpOnTimeDigit).timeDigit, onTime, 6, 10, 3);
    (void)LoadTimeDigit(&(*gPumpOffTimeDigit).timeDigit, offTime, 6, 10, 4);
    
    //load shared select boxes
    gMeteringModeSelectBox = &selectBox1;
    (void) selectBoxConfigure(gMeteringModeSelectBox, 0, NUMBER_METERING_MODES, FALSE, FALSE, FALSE, TRUE, 1, 1, 10, ppMeteringModeSelectBoxTextList);    	
    (*gMeteringModeSelectBox).index = meteringMode;		
	
    //load the is focus array
    loadIsFocusArray();
	
    //set the first object to be in focus
    *isFocusArray[timeFocusIndex] = TRUE;
	
	//unhide the required boxes
	(*gPumpOnTimeDigit).isHidden = FALSE;
    (*gPumpOffTimeDigit).isHidden = FALSE;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if( ((*gPumpOnTimeDigit).isEditMode == FALSE) && ((*gPumpOffTimeDigit).isEditMode == FALSE) )
    {
        ReturnMode = INPUT_MODE_CONFIG;
        hideAllBoxes();
        clearAllIsFocus();
    }
	(void)clearAllSelectBoxIsEdit();
    (*gPumpOnTimeDigit).isEditMode = FALSE;
    (*gPumpOffTimeDigit).isEditMode = FALSE;
    (void)LoadTimeDigit(&(*gPumpOnTimeDigit).timeDigit, onTime, 6, 10, 3);
    (void)LoadTimeDigit(&(*gPumpOffTimeDigit).timeDigit, offTime, 6, 10, 4);
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( timeFocusIndex )
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
		
        case FOCUS_ON_TIME:
            if( (*gPumpOnTimeDigit).isEditMode == TRUE )
            {
                (*gPumpOnTimeDigit).isEditMode = FALSE;
                onTime = GetTimeDigitValue(&(*gPumpOnTimeDigit).timeDigit);
                if( onTime != gSetup.OnTime )
                {
                    onTime = clampValue(onTime,MAX_TIME,MIN_ON_TIME);
                    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OnTime), (DistVarType)onTime);
                    PMP_resetStates();
                    PublishUint32(TOPIC_OnTime, gSetup.OnTime);
                    (void)LoadTimeDigit(&(*gPumpOnTimeDigit).timeDigit, onTime, 6, 10, 3);
                }
            }
            else
            {
                (*gPumpOnTimeDigit).timeDigit.DigitSelected = COUNT_DIGIT_10;
                (*gPumpOnTimeDigit).isEditMode = TRUE;
            }
            break;

        case FOCUS_OFF_TIME:
            if( (*gPumpOffTimeDigit).isEditMode == TRUE )
            {
                (*gPumpOffTimeDigit).isEditMode = FALSE;
                offTime = GetTimeDigitValue(&(*gPumpOffTimeDigit).timeDigit);
                if( offTime != gSetup.OffTime )
                {
                    offTime = clampValue(offTime,MAX_TIME,MIN_OFF_TIME);
                    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OffTime), (DistVarType)offTime);
                    PMP_resetStates();
                    PublishUint32(TOPIC_OffTime, gSetup.OffTime);
                    (void)LoadTimeDigit(&(*gPumpOffTimeDigit).timeDigit, offTime, 6, 10, 4);
                }
            }
            else
            {
                (*gPumpOffTimeDigit).timeDigit.DigitSelected = COUNT_DIGIT_10;
                (*gPumpOffTimeDigit).isEditMode = TRUE;
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
    if( upEventForTimeBox() == FALSE && upEventForSelectBox() == FALSE)
    {
        timeFocusIndex = decrementTimeFocusIndex(timeFocusIndex);
    }
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if( downEventForTimeBox() == FALSE && downEventForSelectBox() == FALSE )
    {
        timeFocusIndex = incrementTimeFocusIndex(timeFocusIndex);
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
    isFocusArray[FOCUS_ON_TIME] = &((*gPumpOnTimeDigit).isFocus);
    isFocusArray[FOCUS_OFF_TIME] = &((*gPumpOffTimeDigit).isFocus);
}
