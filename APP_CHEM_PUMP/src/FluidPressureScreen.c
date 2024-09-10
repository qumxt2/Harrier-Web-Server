// FluidPressureScreen.c

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "typedef.h"
#include "CountDigit.h"
#include "stdint.h"
#include "io_typedef.h"
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "units_pressure.h"
#include "AdvancedScreen.h"
#include "screensTask.h"
#include "FluidPressureScreen.h"
#include "screenStuff.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_PRESSURE_DIGITS             4

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_PRESSURE_OFFSET = 0,
    FOCUS_PRESSURE_SLOPE,
    NUMBER_FLUID_PRESS_ITEMS,
} FLUID_PRESS_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t* gPressure_1_OffsetDigitBox;
static DIGIT_BOX_t* gPressure_1_SlopeDigitBox;

static INPUT_MODE_t ReturnMode;
static FLUID_PRESS_FOCUS_t fluidPressureFocusIndex = FOCUS_PRESSURE_OFFSET;
static IOrtn_mV_s16d16_t Pressure_1_Offset;
static uint32_t Pressure_1_Slope = 0;
static bool* isFocusArray[NUMBER_FLUID_PRESS_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawFluidPressureScreen(FLUID_PRESS_FOCUS_t focusIndex);
static uint16_t incrementPressureFocusIndex(uint16_t focusIndex);
static uint16_t decrementPressureFocusIndex(uint16_t focusIndex);

static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// FluidPressureScreen - The main handler for the sensor screen display
// **********************************************************************************************************
INPUT_MODE_t FluidPressureScreen(INPUT_EVENT_t InputEvent)
{    
    ReturnMode = INPUT_MODE_FLUID_PRESS;

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

    drawFluidPressureScreen(fluidPressureFocusIndex);
    
    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementFocusIndex - Move focus to the next field
// **********************************************************************************************************
static uint16_t incrementPressureFocusIndex(uint16_t focusIndex)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    //modify the focus index
    if( focusIndex < (NUMBER_FLUID_PRESS_ITEMS - 1) )
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
// decrementFocusIndex - Move focus to the previous field
// **********************************************************************************************************
static uint16_t decrementPressureFocusIndex(uint16_t focusIndex)
{   
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    //modify the focus index
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = NUMBER_FLUID_PRESS_ITEMS - 1;
    }
    
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;    
}

// **********************************************************************************************************
// drawFluidPressureScreen - Draw the rest of the sensor screen
// **********************************************************************************************************
static void drawFluidPressureScreen(FLUID_PRESS_FOCUS_t focusIndex)
{
    gsetcpos(0, 1);
    gputs("OFFSET");
    
    gsetcpos(0, 3);
    gputs("SENSITIVITY");

    drawAllDigitBoxes();
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
    
    Pressure_1_Offset.mV_s16d16 = gSetup.Pressure_1_Offset;
    Pressure_1_Slope = gSetup.Pressure_1_Slope;
    
    //load shared digit boxes
    gPressure_1_OffsetDigitBox = &digitBox1;
    gPressure_1_SlopeDigitBox = &digitBox2;
    (void)LoadCountDigit(&(*gPressure_1_OffsetDigitBox).countDigit, gSetup.Pressure_1_Offset, NUM_PRESSURE_DIGITS, DECIMAL_POINT_TWO_DIGIT, 17, 1, TRUE, FALSE);        
    (void)LoadCountDigit(&(*gPressure_1_SlopeDigitBox).countDigit, gSetup.Pressure_1_Slope, NUM_PRESSURE_DIGITS, DECIMAL_POINT_TWO_DIGIT, 17, 3, TRUE, FALSE);
    
    //unhide required boxes
    (*gPressure_1_OffsetDigitBox).isHidden = FALSE;
    (*gPressure_1_SlopeDigitBox).isHidden = FALSE;

    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    fluidPressureFocusIndex = FOCUS_PRESSURE_OFFSET;
    *isFocusArray[fluidPressureFocusIndex] = TRUE;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if(anyDigitBoxIsEdit() == FALSE)
    {
        hideAllBoxes();
        ReturnMode = INPUT_MODE_CONFIG;
        clearAllIsFocus();
    }
    (void)LoadCountDigit(&(*gPressure_1_OffsetDigitBox).countDigit, gSetup.Pressure_1_Offset, NUM_PRESSURE_DIGITS, DECIMAL_POINT_TWO_DIGIT, 17, 1, TRUE, FALSE);        
    (void)LoadCountDigit(&(*gPressure_1_SlopeDigitBox).countDigit, gSetup.Pressure_1_Slope, NUM_PRESSURE_DIGITS, DECIMAL_POINT_TWO_DIGIT, 17, 3, TRUE, FALSE);
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( fluidPressureFocusIndex )
    {       
        case FOCUS_PRESSURE_OFFSET:
            if( (*gPressure_1_OffsetDigitBox).isEditMode == TRUE )
            {
                Pressure_1_Offset.mV_s16d16 = GetCountDigitValue(&(*gPressure_1_OffsetDigitBox).countDigit);
                (*gPressure_1_OffsetDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, Pressure_1_Offset), Pressure_1_Offset.mV_s16d16);
                // Update screen
                (void)LoadCountDigit(&(*gPressure_1_OffsetDigitBox).countDigit, gSetup.Pressure_1_Offset, NUM_PRESSURE_DIGITS, DECIMAL_POINT_TWO_DIGIT, 17, 1, TRUE, FALSE);        
            }
            else
            {
                (*gPressure_1_OffsetDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gPressure_1_OffsetDigitBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_PRESSURE_SLOPE:
            if( (*gPressure_1_SlopeDigitBox).isEditMode == TRUE )
            {
                Pressure_1_Slope = GetCountDigitValue(&(*gPressure_1_SlopeDigitBox).countDigit);
                (*gPressure_1_SlopeDigitBox).isEditMode = FALSE;
                ( void )DVAR_SetPointLocal_wCallback( DVA17G721_SS( gSetup, Pressure_1_Slope ), Pressure_1_Slope );
                // Update screen if new value rejected and defaults used instead
                if (gSetup.Pressure_1_Slope != Pressure_1_Slope)
                {
                    (void)LoadCountDigit(&(*gPressure_1_SlopeDigitBox).countDigit, gSetup.Pressure_1_Slope, NUM_PRESSURE_DIGITS, DECIMAL_POINT_TWO_DIGIT, 17, 3, TRUE, FALSE);
                }
            }
            else
            {
                (*gPressure_1_SlopeDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gPressure_1_SlopeDigitBox).isEditMode = TRUE;
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
    if(upEventForDigitBox() == FALSE)
    {
        fluidPressureFocusIndex = decrementPressureFocusIndex(fluidPressureFocusIndex);
    }    
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if(downEventForDigitBox() == FALSE)
    {
        fluidPressureFocusIndex = incrementPressureFocusIndex(fluidPressureFocusIndex);
    }
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_PRESSURE_OFFSET] = &((*gPressure_1_OffsetDigitBox).isFocus);
    isFocusArray[FOCUS_PRESSURE_SLOPE] = &((*gPressure_1_SlopeDigitBox).isFocus);
}


