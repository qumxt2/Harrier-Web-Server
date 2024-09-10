// AnalogOutFlowScreen.c

// Copyright 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the analog output that is scaled based on the flow rate

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "screensTask.h"
#include "screenStuff.h"
#include "dvinterface_17G721.h"
#include "dvseg_17G721_setup.h"
#include "out_digital.h"
#include "AnalogOutFlowScreen.h"
// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_VOLTAGE_OUT_DIGITS      (2)
#define MAX_5V_OUTPUT               (59)        // Allow setting up to 5.9V
// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************
typedef enum
{
    FOCUS_OUTPUT_TYPE = 0,
    FOCUS_MIN_OUT,
    FOCUS_MAX_OUT,
    NUMBER_AOUT_FLOW_ITEMS
} AOUT_FLOW_FOCUS_t;

typedef enum
{
    OUTPUT_VOLTS = 0,
    OUTPUT_MA,
    NUMBER_OUTPUT_TYPES
} AOUT_OUTPUT_TYPES_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static SELECTION_BOX_t* gOutputTypeSelectBox;
static DIGIT_BOX_t* gMinOutDigitBox;
static DIGIT_BOX_t* gMaxOutDigitBox;

static INPUT_MODE_t ReturnMode;
static AOUT_FLOW_FOCUS_t analogOutFlowFocusIndex = FOCUS_MIN_OUT;
static INT32U AoutMinOutput;
static INT32U AoutMaxOutput;
static AOUT_OUTPUT_TYPES_t AoutOutputType = OUTPUT_VOLTS;
static bool* isFocusArray[NUMBER_AOUT_FLOW_ITEMS];

static char* ppOutputTypeSelectBoxTextList[] =
{
    "0-5V",
};

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawAnalogOutFlowScreen(AOUT_FLOW_FOCUS_t index);
static AOUT_FLOW_FOCUS_t incrementAoutFlowFocusIndex(AOUT_FLOW_FOCUS_t focusIndex);
static AOUT_FLOW_FOCUS_t decrementAoutFlowFocusIndex(AOUT_FLOW_FOCUS_t focusIndex);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadCountDigitBoxes(void);
static void loadIsFocusArray(void);

// ******************************************s****************************************************************
// FlowScreen - The main handler for the flow screen display
// **********************************************************************************************************
INPUT_MODE_t AnalogOutFlowScreen(INPUT_EVENT_t InputEvent)
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
    
    // Unless something changes it return to the same screen
    ReturnMode = INPUT_MODE_AOUT_FLOW;

    // Process based on input event
    (void)(*processInputEvent[InputEvent])();
    
    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawAnalogOutFlowScreen(analogOutFlowFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementAoutFlowFocusIndex - Move focus to the next field
// **********************************************************************************************************
static AOUT_FLOW_FOCUS_t incrementAoutFlowFocusIndex(AOUT_FLOW_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex < (NUMBER_AOUT_FLOW_ITEMS - 1) )
    {
        focusIndex = focusIndex + 1;
    }
    else
    {
        focusIndex = 1;
    }
    
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// decrementAoutFlowFocusIndex - Move focus to the previous field
// **********************************************************************************************************
static AOUT_FLOW_FOCUS_t decrementAoutFlowFocusIndex(AOUT_FLOW_FOCUS_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex > 1 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = (NUMBER_AOUT_FLOW_ITEMS - 1);
    }
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawVolumeScreen - Draw the rest of the flow screen
// **********************************************************************************************************
static void drawAnalogOutFlowScreen(AOUT_FLOW_FOCUS_t focusIndex)
{
    gsetcpos(0, 1);
    gputs("OUTPUT");
    gsetcpos(8, 1);
    gputs("0-5V");
    
    gsetcpos(0, 3);
    gputs("MIN OUT");
    gsetcpos(12, 3);
    gputs("V");
    
    gsetcpos(0, 4);
    gputs("MAX OUT");
    gsetcpos(12, 4);
    gputs("V");
    
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
    
    AoutOutputType = gSetup.AoutOutputType;
    AoutMinOutput = gSetup.AoutMinOut;
    AoutMaxOutput = gSetup.AoutMaxOut;
    
    analogOutFlowFocusIndex  = FOCUS_MIN_OUT;

    //load shared digit boxes
    gMinOutDigitBox = &digitBox1;
    gMaxOutDigitBox = &digitBox2;
    loadCountDigitBoxes();
    
    //unhide the required boxes
    (*gMinOutDigitBox).isHidden = FALSE;
    (*gMaxOutDigitBox).isHidden = FALSE;
    
    //load shared select boxes
    gOutputTypeSelectBox = &selectBox1;
    (void)selectBoxConfigure(gOutputTypeSelectBox, 0, NUMBER_OUTPUT_TYPES-1, FALSE, FALSE, TRUE, FALSE, 8, 1, 7, ppOutputTypeSelectBoxTextList);
    (*gOutputTypeSelectBox).index = AoutOutputType;
    
	//load the is focus array
    loadIsFocusArray();
	
    //set the first object to be in focus
    *isFocusArray[FOCUS_OUTPUT_TYPE] = TRUE;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if ((anyDigitBoxIsEdit() == FALSE) && (anySelectBoxIsEdit() == FALSE))
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
    switch( analogOutFlowFocusIndex  )
    {
        case FOCUS_OUTPUT_TYPE:
            if( (*gOutputTypeSelectBox).isEditMode == TRUE )
            {
                AoutOutputType = (*gOutputTypeSelectBox).index;
                (*gOutputTypeSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, AoutOutputType), (DistVarType)AoutOutputType);
                if(AoutOutputType == OUTPUT_VOLTS)
                {
                    // 1 = 0-10 V Output (Default)
                    (void)OUT_Digital_Latch_Set(IOPIN_SPEED_CNTL_OUTPUT_SELECT, ASSERTED);
                }
                else if(AoutOutputType == OUTPUT_MA)
                {
                    // 0 = 4-20 mA Output
                    (void)OUT_Digital_Latch_Set(IOPIN_SPEED_CNTL_OUTPUT_SELECT, NOT_ASSERTED);
                }
            }
            else
            {
                (*gOutputTypeSelectBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_MIN_OUT:
            if( (*gMinOutDigitBox).isEditMode == TRUE)
            {
                AoutMinOutput = GetCountDigitValue(&(*gMinOutDigitBox).countDigit);
                if(AoutMinOutput > gSetup.AoutMaxOut)
                {
                    AoutMinOutput = gSetup.AoutMinOut;
                }
                (*gMinOutDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, AoutMinOut), (DistVarType)AoutMinOutput);
                loadCountDigitBoxes();
            }
            else
            {
                (*gMinOutDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10;
                (*gMinOutDigitBox).isEditMode = TRUE;
            }
            break;
            
        case FOCUS_MAX_OUT:
            if( (*gMaxOutDigitBox).isEditMode == TRUE)
            {
                AoutMaxOutput = GetCountDigitValue(&(*gMaxOutDigitBox).countDigit);
                if(AoutMaxOutput > MAX_5V_OUTPUT)
                {
                    AoutMaxOutput  = MAX_5V_OUTPUT;
                }
                else if(AoutMaxOutput < gSetup.AoutMinOut)
                {
                    AoutMaxOutput = gSetup.AoutMaxOut;
                }
                (*gMaxOutDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, AoutMaxOut), (DistVarType)AoutMaxOutput);
                loadCountDigitBoxes();
            }
            else
            {
                (*gMaxOutDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10;
                (*gMaxOutDigitBox).isEditMode = TRUE;
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
        analogOutFlowFocusIndex  = decrementAoutFlowFocusIndex(analogOutFlowFocusIndex );
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
        analogOutFlowFocusIndex  = incrementAoutFlowFocusIndex(analogOutFlowFocusIndex );
    }
}

/****************************************************************************/
//Fcn: loadCountDigitBoxes
//
//Desc: Load all the required values for the count digit boxes
//****************************************************************************//
static void loadCountDigitBoxes(void)
{
    AoutMinOutput = gSetup.AoutMinOut;
    AoutMaxOutput = gSetup.AoutMaxOut;
    
    (void)LoadCountDigit(&(*gMinOutDigitBox).countDigit, AoutMinOutput, NUM_VOLTAGE_OUT_DIGITS, DECIMAL_POINT_ONE_DIGIT, 8, 3, FALSE, FALSE);
    (void)LoadCountDigit(&(gMaxOutDigitBox)->countDigit, AoutMaxOutput, NUM_VOLTAGE_OUT_DIGITS, DECIMAL_POINT_ONE_DIGIT, 8, 4, FALSE, FALSE);
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{            
    isFocusArray[FOCUS_OUTPUT_TYPE] = &((*gOutputTypeSelectBox).isFocus);	
    isFocusArray[FOCUS_MIN_OUT] = &((*gMinOutDigitBox).isFocus);
    isFocusArray[FOCUS_MAX_OUT] = &((*gMaxOutDigitBox).isFocus);	
}

