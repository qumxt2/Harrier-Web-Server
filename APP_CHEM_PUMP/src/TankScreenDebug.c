// TankScreenDebug.c

#include "stdio.h"

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Implements the logic for the debug tank screen which contains voltage, pressure, & other tank info

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include <stdint.h>
#include "dvseg_17G721_setup.h"
#include "dvseg_17G721_run.h"
#include "gdisp.h"
#include "screensTask.h"
#include "in_pressure.h"
#include "CountDigit.h"
#include "TankScreen.h"
#include "PublishSubscribe.h"
#include "dvseg_17G721_levelChart.h"
#include "assert.h"
#include "debug_app.h"
#include "screenStuff.h"
#include "TankScreenHorizontal.h"
#include "TankScreenCustom.h"
#include "TankScreenVertical.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_PRESSURE_DIGITS             (4)
#define NUM_VOLTAGE_DIGITS              (5)
#define NUM_FLUID_DENSITY_DIGITS        (4)
#define MIN_FLUID_DENSITY               (1u)
#define NUM_PRESSURE_CAL_DIGITS         (5)

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************
typedef enum
{
    FOCUS_FLUID_DENSITY,
    NUMBER_TANK_DEBUG_ITEMS
} TANK_DEBUG_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t* gCurrentPressureDigitBox;
static DIGIT_BOX_t* gCurrentVoltageDigitBox;
static DIGIT_BOX_t* gFillPressureDigitBox;
static DIGIT_BOX_t* gFillVoltageDigitBox;
static DIGIT_BOX_t* gFluidDensityDigitBox;
static DIGIT_BOX_t* gTankHeightCalcDigitBox;
static DIGIT_BOX_t * gPressure_2_OffsetDigitBox;
static DIGIT_BOX_t * gPressure_2_SlopeDigitBox;

static INPUT_MODE_t ReturnMode;
static TANK_DEBUG_FOCUS_t tankDebugFocusIndex = FOCUS_FLUID_DENSITY;
static bool* isFocusArray[NUMBER_TANK_DEBUG_ITEMS];
static uint32 fluidDensity = 0;

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawTankScreenDebug(TANK_DEBUG_FOCUS_t focusIndex);
static uint16 incrementFocusIndex(TANK_DEBUG_FOCUS_t focusIndex);
static uint16 decrementFocusIndex(TANK_DEBUG_FOCUS_t focusIndex);
static void loadDigitBoxes(void);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// TankScreenDebug - The main handler for the tank debug screen display
// **********************************************************************************************************
INPUT_MODE_t TankScreenDebug(INPUT_EVENT_t InputEvent)
{  
    ReturnMode = INPUT_MODE_TANK_DEBUG;

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

    drawTankScreenDebug(tankDebugFocusIndex);
    
    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// drawTankScreenDebug - Draw the rest of the tank debug screen
// **********************************************************************************************************
static void drawTankScreenDebug(TANK_DEBUG_FOCUS_t focusIndex)
{
    gsetcpos(0, 0);
    gputs("CURRENT PRESS");

    gsetcpos(0, 1);
    gputs("CURRENT VOLTS");

    gsetcpos(0, 2);
    if (gSetup.TankType == TANK_TYPE_CUSTOM)
    {
        gputs("FILL PRESSURE");
    }
    else
    {
        gputs("FILL PRESS (AVG)");
    }

    gsetcpos(0, 3);
    gputs("FILL VOLTS");
    
    if (gSetup.TankType == TANK_TYPE_CUSTOM)
    {
        gsetcpos(0, 4);
        gputs("DENSITY");
    
        gsetcpos(0, 5);
        gputs("HEIGHT");
    }
    else
    {
        (*gFluidDensityDigitBox).isHidden = TRUE;
        (*gTankHeightCalcDigitBox).isHidden = TRUE;
    }
    
    gsetcpos(0, 6);
    gputs("OFFSET");
    
    gsetcpos(0, 7);
    gputs("SENSITIVITY");

    drawAllDigitBoxes();
    drawAllSelectBoxes();
}

static void loadDigitBoxes()
{
    IOrtn_mV_to_psi_s16d16_t currentPressure = GetPressurePSI(IOHARDWARE_PRESSURESENSOR_B);
    
    (void)LoadCountDigit(&(*gCurrentPressureDigitBox).countDigit, currentPressure.psi_s16d16, NUM_PRESSURE_DIGITS, DECIMAL_PLACE_THREE, 16, 0, TRUE, FALSE);
    (void)LoadCountDigit(&(*gCurrentVoltageDigitBox).countDigit, currentPressure.mV_s16d16, NUM_VOLTAGE_DIGITS, DECIMAL_POINT_ONE_DIGIT, 16, 1, TRUE, FALSE);
    (void)LoadCountDigit(&(*gFillPressureDigitBox).countDigit, gSetup.TankFillPressure, NUM_PRESSURE_DIGITS, DECIMAL_POINT_THREE_DIGIT, 16, 2, FALSE, FALSE);
    (void)LoadCountDigit(&(*gFillVoltageDigitBox).countDigit, gSetup.TankFillVoltage, NUM_VOLTAGE_DIGITS, DECIMAL_POINT_ONE_DIGIT, 16, 3, TRUE, FALSE);
    (void)LoadCountDigit(&(*gFluidDensityDigitBox).countDigit, gSetup.FluidDensity, NUM_FLUID_DENSITY_DIGITS, DECIMAL_POINT_TWO_DIGIT, 16, 4, FALSE, FALSE);
    (void)LoadCountDigit(&(*gTankHeightCalcDigitBox).countDigit, gRun.TankHeightCalc, NUM_TANK_LEVEL_DIGITS, DECIMAL_POINT_TWO_DIGIT, 16, 5, FALSE, FALSE);
    (void)LoadCountDigit(&(*gPressure_2_OffsetDigitBox).countDigit, gSetup.Pressure_2_Offset, NUM_PRESSURE_CAL_DIGITS, DECIMAL_POINT_ONE_DIGIT, 16, 6, TRUE, FALSE);    
    (void)LoadCountDigit(&(*gPressure_2_SlopeDigitBox).countDigit, gSetup.Pressure_2_Slope, NUM_PRESSURE_CAL_DIGITS, DECIMAL_POINT_THREE_DIGIT, 16, 7, TRUE, FALSE);        
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

    //load shared digit boxes
    gCurrentPressureDigitBox = &digitBox1;
    gCurrentVoltageDigitBox = &digitBox2;
    gFillPressureDigitBox = &digitBox3;
    gFillVoltageDigitBox = &digitBox4;
    gFluidDensityDigitBox = &digitBox5;
    gTankHeightCalcDigitBox = &digitBox6;
    gPressure_2_OffsetDigitBox = &digitBox7;
    gPressure_2_SlopeDigitBox = &digitBox8;    
    loadDigitBoxes();
    
    //unhide the required boxes
	(*gCurrentPressureDigitBox).isHidden = FALSE;
    (*gCurrentVoltageDigitBox).isHidden = FALSE;
	(*gFillPressureDigitBox).isHidden = FALSE;
    (*gFillVoltageDigitBox).isHidden = FALSE;
    (*gFluidDensityDigitBox).isHidden = FALSE;    
    (*gTankHeightCalcDigitBox).isHidden = FALSE;
	(*gPressure_2_OffsetDigitBox).isHidden = FALSE;
    (*gPressure_2_SlopeDigitBox).isHidden = FALSE;        

    loadIsFocusArray();
    
    if (gSetup.TankType == TANK_TYPE_CUSTOM)
    {
        *isFocusArray[tankDebugFocusIndex] = TRUE;
    }
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
        clearAllIsFocus();
        ReturnMode = INPUT_MODE_TANK;
    }
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
    switch( tankDebugFocusIndex )
    {       
        case FOCUS_FLUID_DENSITY:
            if( (*gFluidDensityDigitBox).isEditMode == TRUE )
            {
                fluidDensity = GetCountDigitValue(&(*gFluidDensityDigitBox).countDigit);
                (*gFluidDensityDigitBox).isEditMode = FALSE;
                if (fluidDensity < MIN_FLUID_DENSITY)
                {
                    fluidDensity = MIN_FLUID_DENSITY;
                    (void)LoadCountDigit(&(*gFluidDensityDigitBox).countDigit, fluidDensity, NUM_FLUID_DENSITY_DIGITS, DECIMAL_POINT_TWO_DIGIT, 17, 4, FALSE, FALSE);
                }
                (void)DVAR_SetPointLocal( DVA17G721_SS( gSetup, FluidDensity ), fluidDensity );
                (void)CalculatePressureSensorOffset();
                loadDigitBoxes();
                
            }
            else
            {
                (*gFluidDensityDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gFluidDensityDigitBox).isEditMode = TRUE;
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
    (void)upEventForDigitBox();
    (void)upEventForSelectBox();
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    (void)downEventForDigitBox();
    (void)downEventForSelectBox();
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_FLUID_DENSITY] = &((*gFluidDensityDigitBox).isFocus);
}

