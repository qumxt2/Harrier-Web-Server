// AdvancedScreen.c

#include <stdio.h>

// Copyright 2015 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the advance screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "gdisp.h"
#include "CountDigit.h"
#include "screensTask.h"
#include "AdvancedScreen.h" 
#include "pumpControlTask.h"
#include "volumeTask.h"
#include "in_pressure.h"
#include "screenStuff.h"
#include "dvseg_17G721_run.h"
#include "systemTask.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_CAL_VOLUME_DIGITS           5
#define MAX_SCREEN_WIDTH_CHAR           25

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_CAL_VOLUME = 0,
    FOCUS_CAL,
    FOCUS_UNITS,
    FOCUS_ANALOG_INPUT,
    FOCUS_ANALOG_OUTPUT,
    NUMBER_ADVANCED_ITEMS
} ADVANCED_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t * gCalVolumeDigitBox;

static SELECTION_BOX_t* gCalSelectBox;
static SELECTION_BOX_t* gUnitsSelectBox;
static SELECTION_BOX_t* gAnalogInputSelectBox;
static SELECTION_BOX_t* gAnalogOutputSelectBox;

static char* ppCalSelectBoxTextList[] =
{
    "STOP",
    "START"
};

static char* ppUnitsSelectBoxTextList[] =
{
    "US",
    "METRIC"
};

static char* ppAINSelectBoxTextList[] =
{
    "OFF",
    "FLOW"
};

static char* ppAOUTSelectBoxTextList[] =
{
    "OFF",
    "ON",
};

static INPUT_MODE_t ReturnMode;
static ADVANCED_FOCUS_t advFocusIndex = FOCUS_CAL;
static uint32 calibrationVolume = 0;
static BOOLEAN gCalComplete = FALSE;
static ADV_STATES_t calMode = CAL_STOP;
static UNIT_TYPES_t units = UNITS_IMPERIAL;
static bool* isFocusArray[NUMBER_ADVANCED_ITEMS];
static Analog_In_Control_t analogInputMode = AIN_OFF;
static Analog_Out_Control_t analogOutputMode = AOUT_OFF;
static METERING_MODE_t meteringMode = METERING_MODE_TIME;

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawAdvScreen(void);
static ADVANCED_FOCUS_t incrementAdvancedFocusIndex(ADVANCED_FOCUS_t focusIndex);
static ADVANCED_FOCUS_t decrementAdvancedFocusIndex(ADVANCED_FOCUS_t focusIndex);
static void calibrationCompleteHandler(void);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// AdvancedScreen - The main handler for the calibration screen display
// **********************************************************************************************************
INPUT_MODE_t AdvancedScreen(INPUT_EVENT_t InputEvent)
{
    ReturnMode = INPUT_MODE_ADVANCED;

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

    drawAdvScreen();

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// calibrationCompleteHandler - Notify the screen that the calibration is complete
// **********************************************************************************************************
static void calibrationCompleteHandler(void)
{
    gCalComplete = TRUE;
    RefreshScreen();
}

// **********************************************************************************************************
// incrementAdvancedFocusIndex - Move focus to the next field
// **********************************************************************************************************
static ADVANCED_FOCUS_t incrementAdvancedFocusIndex(ADVANCED_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    //modify the focus index
    if( (focusIndex < (NUMBER_ADVANCED_ITEMS - 1)) && (meteringMode == METERING_MODE_VOLUME) )  // we are in flow mode and not at the bottom of the screen
    {
        focusIndex = focusIndex + 1;
    }
    else if(focusIndex < (NUMBER_ADVANCED_ITEMS - 3))  //we are not in flow mode and not at the bottom of the screen.  Hide Analog Input & Output boxes if we're not in Flow Mode
    {
        focusIndex = focusIndex + 1;
    }
    else    //we are at the bottom of the screen
    {
        focusIndex = 1;
    }
    
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;   
}

// **********************************************************************************************************
// decrementAdvancedFocusIndex - Move focus to the previous field
// **********************************************************************************************************
static ADVANCED_FOCUS_t decrementAdvancedFocusIndex(ADVANCED_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    //modify the focus index
    if( focusIndex > 1 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        if(meteringMode == METERING_MODE_VOLUME)
        {
            focusIndex = NUMBER_ADVANCED_ITEMS - 1;             // Show Analog Input & Output boxes if we're in Flow Mode
        }
        else
        {
            focusIndex = NUMBER_ADVANCED_ITEMS - 3;             //Hide Analog Input & Output boxes if we're not in Flow Mode
        }
        
    }
    
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawAdvScreen - Draw the rest of the advanced screen
// **********************************************************************************************************
static void drawAdvScreen(void)
{
    gsetcpos(0, 1);
    if (gCalComplete == TRUE)
    {
        if (gSetup.Units == UNITS_METRIC)
        {
            gputs("CAL VOLUME                   CC");
        }
        else
        {
            gputs("CAL VOLUME                   OZ");
        }
        (*gCalVolumeDigitBox).isHidden = FALSE;
    }
    else
    {
        (*gCalVolumeDigitBox).isHidden = TRUE;
        char buf[MAX_SCREEN_WIDTH_CHAR];
        sprintf(buf, "K-FACTOR");
        gputs(buf);
        gsetpos(91, 23);
        sprintf(buf, "%04d", (uint32_t)gSetup.KFactor);
        gputs(buf);
    }

    gsetcpos(0, 2);
    gputs("CALIBRATION");

    gsetcpos(0, 3);
    gputs("UNITS");
    
    if(meteringMode == METERING_MODE_VOLUME)
    {
        //we are in flow mode it is okay to show these
        gsetcpos(0, 4);
        gputs("ANALOG IN");  
        gsetcpos(0, 5);
        gputs("ANALOG OUT");
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
    //reset all the boxes to their default states
    clearAllIsFocus();
    hideAllBoxes();
    clearAllIsEdit();
    ClearScreen();    
    
    units = gSetup.Units;
    meteringMode = gSetup.MeteringMode;
    analogInputMode = gSetup.AnalogInControl;
    analogOutputMode = gSetup.AnalogOutControl;
    
    //load shared digit boxes
    gCalVolumeDigitBox = &digitBox1;
    (void)LoadCountDigit(&(*gCalVolumeDigitBox).countDigit, calibrationVolume, NUM_CAL_VOLUME_DIGITS, DECIMAL_POINT_TWO_DIGIT, 14, 1, FALSE, FALSE);
    
    //load shared select boxes
    gCalSelectBox = &selectBox1;
    gUnitsSelectBox = &selectBox2;    
    gAnalogInputSelectBox = &selectBox3;
    gAnalogOutputSelectBox = &selectBox4;
    (void) selectBoxConfigure(gCalSelectBox, 0, NUMBER_CAL_STATES, FALSE, FALSE, FALSE, FALSE, 13, 2, 6, ppCalSelectBoxTextList);
    (void) selectBoxConfigure(gUnitsSelectBox, 0, NUMBER_UNITS_TYPES, FALSE, FALSE, FALSE, FALSE, 13, 3, 7, ppUnitsSelectBoxTextList);
    (void) selectBoxConfigure(gAnalogInputSelectBox, 0, NUMBER_ANALOG_IN_CONTROL_ITEMS, FALSE, FALSE, FALSE, FALSE, 13, 4, 5, ppAINSelectBoxTextList);
    (void) selectBoxConfigure(gAnalogOutputSelectBox, 0, NUMBER_ANALOG_OUT_CONTROL_ITEMS, FALSE, FALSE, FALSE, FALSE, 13, 5, 5, ppAOUTSelectBoxTextList);

    (*gCalSelectBox).index = calMode;  
    (*gUnitsSelectBox).index = units;
    (*gAnalogInputSelectBox).index = analogInputMode;
    (*gAnalogOutputSelectBox).index = analogOutputMode;
    
    //check to see if we are in flow mode
    //only show the analog in control if we are in flow mode
    if(meteringMode == METERING_MODE_VOLUME)
    {
       (*gAnalogInputSelectBox).isHidden = FALSE;
       (*gAnalogOutputSelectBox).isHidden = FALSE;
    }
    else
    {
        (*gAnalogInputSelectBox).isHidden = TRUE;
        (*gAnalogOutputSelectBox).isHidden = TRUE;
    }
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    advFocusIndex = FOCUS_CAL;
    if (gCalComplete == TRUE)
    {
        advFocusIndex = FOCUS_CAL_VOLUME;
    }
    *isFocusArray[advFocusIndex] = TRUE;
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
        hideAllBoxes();
        ReturnMode = INPUT_MODE_CONFIG;
        gCalComplete = FALSE;   //reset the gCalComplete to false to prevent the volume entering field from reappearing when reentering the screen
        clearAllIsFocus();
        PMP_setStandbyMode();      //Ensure the pump is not running if the user leaves the screen before a calibration is complete
    }
    (void)LoadCountDigit(&(*gCalVolumeDigitBox).countDigit, calibrationVolume, NUM_CAL_VOLUME_DIGITS, DECIMAL_POINT_TWO_DIGIT, 14, 1, FALSE, FALSE);
    (*gCalSelectBox).index = calMode;
    (*gUnitsSelectBox).index = units;
    (*gAnalogInputSelectBox).index = analogInputMode;
    (*gAnalogOutputSelectBox).index = analogOutputMode;
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( advFocusIndex )
    {
        case FOCUS_CAL_VOLUME:
            if( (*gCalVolumeDigitBox).isEditMode == TRUE )
            {
                calibrationVolume = GetCountDigitValue(&(*gCalVolumeDigitBox).countDigit);
                (*gCalVolumeDigitBox).isEditMode = FALSE;
                if (calibrationVolume > 0)
                {
                    gCalComplete = FALSE;
                    advFocusIndex = FOCUS_CAL;
                    calMode = CAL_STOP;
                    (*gCalSelectBox).index = calMode;
                    VOL_updateKfactor(calibrationVolume);
                    RefreshScreen();
                }
            }
            else
            {
                (*gCalVolumeDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gCalVolumeDigitBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_CAL:
            if( (*gCalSelectBox).isEditMode == TRUE )
            {
                calMode = (*gCalSelectBox).index;
                (*gCalSelectBox).isEditMode = FALSE;
                if (calMode == CAL_START)
                {
                    PMP_calibrate(calibrationCompleteHandler);
                    calibrationVolume = 0;
                    (void)LoadCountDigit(&(*gCalVolumeDigitBox).countDigit, calibrationVolume, NUM_CAL_VOLUME_DIGITS, DECIMAL_POINT_TWO_DIGIT, 14, 1, FALSE, FALSE);
                }
                if (calMode == CAL_STOP)
                {
                    PMP_setStandbyMode(); //for restoring the settings after a cal routine
                }
            }
            else
            {
                (*gCalSelectBox).isEditMode = TRUE;
            }
            break;
            
        case FOCUS_UNITS:
            if( (*gUnitsSelectBox).isEditMode == TRUE )
            {
                units = (*gUnitsSelectBox).index;
                (*gUnitsSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, Units), (DistVarType)units);
            }
            else
            {
                (*gUnitsSelectBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_ANALOG_INPUT:

            if( (*gAnalogInputSelectBox).isEditMode == TRUE )
            {
                analogInputMode = (*gAnalogInputSelectBox).index;
                (*gAnalogInputSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, AnalogInControl), (DistVarType)analogInputMode);
                PublishUint32(TOPIC_AnalogInputMode, gSetup.AnalogInControl);
                
                if(analogInputMode == AIN_FLOW_RATE)
                {
                    //Now jump to a new screen to allow the entering of the flow rates
                    hideAllBoxes();
                    ReturnMode = INPUT_MODE_AIN_FLOW;
                    gCalComplete = FALSE;   //reset the gCalComplete to false to prevent the volume entering field from reappearing when reentering the screen
                    PMP_setStandbyMode();      //Ensure the pump is not running if the user leaves the screen before a calibration is complete
                    clearAllIsFocus();
                    clearAllIsEdit();
                }//ends if    
            }//ends if is edit
            else
            {
                (*gAnalogInputSelectBox).isEditMode = TRUE;
            }// ends else
            
            break;
            
        case FOCUS_ANALOG_OUTPUT:
            if( (*gAnalogOutputSelectBox).isEditMode == TRUE )
            {
                analogOutputMode = (*gAnalogOutputSelectBox).index;
                (*gAnalogOutputSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, AnalogOutControl), (DistVarType)analogOutputMode);

                //Now jump to a new screen to allow the entering of the flow rates
                hideAllBoxes();
                ReturnMode = INPUT_MODE_AOUT_FLOW;
                gCalComplete = FALSE;       //reset the gCalComplete to false to prevent the volume entering field from reappearing when reentering the screen
                PMP_setStandbyMode();       //Ensure the pump is not running if the user leaves the screen before a calibration is complete
                clearAllIsFocus();
                clearAllIsEdit();
            }
            else
            {
                (*gAnalogOutputSelectBox).isEditMode = TRUE;
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
    if( upEventForDigitBox() == FALSE && upEventForSelectBox() == FALSE)
    {
        advFocusIndex = decrementAdvancedFocusIndex(advFocusIndex);
    }    
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if( downEventForDigitBox() == FALSE &&  downEventForSelectBox() == FALSE )
    {
        advFocusIndex = incrementAdvancedFocusIndex(advFocusIndex);
    }
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_CAL_VOLUME] = &((*gCalVolumeDigitBox).isFocus);
    isFocusArray[FOCUS_CAL] = &((*gCalSelectBox).isFocus);
    isFocusArray[FOCUS_UNITS] = &((*gUnitsSelectBox).isFocus);
    isFocusArray[FOCUS_ANALOG_INPUT] = &((*gAnalogInputSelectBox).isFocus);
    isFocusArray[FOCUS_ANALOG_OUTPUT] = &((*gAnalogOutputSelectBox).isFocus);
}

