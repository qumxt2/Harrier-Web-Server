// TankScreenVertical.c

#include "stdio.h"

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Implements the logic for the vertical tank screen which contains user settings for vertical tanks

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
#include "TankScreenVertical.h"
#include "PublishSubscribe.h"
#include "dvseg_17G721_levelChart.h"
#include "in_pressure.h"
#include "assert.h"
#include "debug_app.h"
#include "screenStuff.h"
#include "utilities.h"
#include "TankScreen.h"
#include "AdvancedScreen.h"
#include "limits.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_VOLUME_DIGITS                  (5)
#define PRESSURE_SENSOR_CHANNEL_4TO20MA    (5)

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************
typedef enum
{
    FOCUS_MAX_VOLUME = 0,
    FOCUS_FILL_VOLUME,
    FOCUS_SENSOR_VOLUME,
    NUMBER_BASIC_ITEMS
} BASIC_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t* gMaxVolumeDigitBox;
static DIGIT_BOX_t* gFillVolumeDigitBox;
static DIGIT_BOX_t* gSensorVolumeDigitBox;

static INPUT_MODE_t ReturnMode;
static BASIC_FOCUS_t basicFocusIndex = FOCUS_MAX_VOLUME;
static bool* isFocusArray[NUMBER_BASIC_ITEMS];
static uint32 maxVolume = 0;
static uint32 fillVolume = 0;
static sint32 fillPressure = 0;
static IOrtn_mV_s16d16_t fillVoltage;
static uint32 sensorVolume = 0;

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawBasicScreen(BASIC_FOCUS_t focusIndex);
static void loadDigitBoxes(void);
static uint16 incrementFocusIndex(uint16 focusIndex, uint16 numberItems);
static uint16 decrementFocusIndex(uint16 focusIndex, uint16 numberItems);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);
static void processInputRefreshScreenEvent(void);

// **********************************************************************************************************
// TankScreenVertical - The main handler for the vertical tank screen display
// **********************************************************************************************************
INPUT_MODE_t TankScreenVertical(INPUT_EVENT_t InputEvent)
{  
    ReturnMode = INPUT_MODE_VERTICAL;

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
   
    // Process based on input event
    (void)(*processInputEvent[InputEvent])();

    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawBasicScreen(basicFocusIndex);
    
    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// drawBasicScreen - Draw the rest of the vertical tank screen
// **********************************************************************************************************
static void drawBasicScreen(BASIC_FOCUS_t focusIndex)
{
    gsetcpos(0, 1);
    gputs("MAXIMUM VOL");
    if (gSetup.Units == UNITS_METRIC)
    {
        gsetcpos(20, 1);
        gputs("L");
    }
    else
    {
        gsetcpos(19, 1);
        gputs("G");
    }
    
    gsetcpos(0, 3);
    gputs("CURRENT VOL");
    if (gSetup.Units == UNITS_METRIC)
    {
        gsetcpos(20, 3);
        gputs("L");
    }
    else
    {
        gsetcpos(19, 3);
        gputs("G");
    }

    gsetcpos(0, 5);
    gputs("SENSOR VOL");
    if (gSetup.Units == UNITS_METRIC)
    {
        gsetcpos(20, 5);
        gputs("L");
    }
    else
    {
        gsetcpos(19, 5);
        gputs("G");
    }
    
    drawAllDigitBoxes();    
}

// **********************************************************************************************************
// UpdateTankLevel
// **********************************************************************************************************
void UpdateTankLevelVertical( void )
{
    uint32 currentPressure_x_1000 = 0;
    uint32 fillPressure_x_1000 = 0;
    uint32 volume = 0;
    
    currentPressure_x_1000 = FixedPointToInteger(gRun.Pressure_2_Psi, DECIMAL_PLACE_THREE);
    fillPressure_x_1000 = gSetup.TankFillPressure;
    
    volume = Interpolate(currentPressure_x_1000, 0, gSetup.TankSensorVolume, fillPressure_x_1000, gSetup.TankFillVolume);
    gRun.TankLevel = volume;
    
    // Send info to debug portal if DBUG_TANK bit is set in debug mask
    DEBUG_PRINT_STRING(DBUG_TANK, "currentPessure_x_1000/Volume_x_100,");
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, currentPressure_x_1000);
    DEBUG_PRINT_STRING(DBUG_TANK, ",");        
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, gRun.TankLevel);
    DEBUG_PRINT_STRING(DBUG_TANK, "\r\n");
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
    gMaxVolumeDigitBox = &digitBox1;
    gFillVolumeDigitBox = &digitBox2;
    gSensorVolumeDigitBox = &digitBox3;
    loadDigitBoxes();
    
    //unhide the required boxes
	(*gMaxVolumeDigitBox).isHidden = FALSE;
    (*gFillVolumeDigitBox).isHidden = FALSE;    
    (*gSensorVolumeDigitBox).isHidden = FALSE;

    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    basicFocusIndex = FOCUS_MAX_VOLUME;
    *isFocusArray[basicFocusIndex] = TRUE;
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
    switch( basicFocusIndex )
    {
        case FOCUS_MAX_VOLUME:
            if( (*gMaxVolumeDigitBox).isEditMode == TRUE )
            {
                (*gMaxVolumeDigitBox).isEditMode = FALSE;
                
                maxVolume = GetCountDigitValue(&(*gMaxVolumeDigitBox).countDigit);
                // Check the screen input so it doesn't overflow while converting L to G
                // Note: Gallons are limited to 9999.9 G by the digit box so no need to check gallons
                if (gSetup.Units == UNITS_METRIC)
                {
                    if (maxVolume > MAX_VOLUME_L)
                    {
                        maxVolume = MAX_VOLUME_L;
                    }
                }
                maxVolume = setLocalVolume(maxVolume);
                validateVertHorzInput(maxVolume, gSetup.TankFillVolume, gSetup.TankSensorVolume);
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MaxTankVolume), (DistVarType)maxVolume);
                PublishUint32(TOPIC_TankLevelVolumeMax, gSetup.MaxTankVolume);
                loadDigitBoxes();
            }
            else
            {
                (*gMaxVolumeDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10000;
                (*gMaxVolumeDigitBox).isEditMode = TRUE;
            }            
            break;
            
        case FOCUS_FILL_VOLUME:
            if( (*gFillVolumeDigitBox).isEditMode == TRUE )
            {
                (*gFillVolumeDigitBox).isEditMode = FALSE;
                
                fillVolume = GetCountDigitValue(&(*gFillVolumeDigitBox).countDigit);
                // Check the screen inputs so it doesn't overflow while converting L to G
                // Note: Gallons are limited to 9999.9 G by the digit box so no need to check gallons
                if (gSetup.Units == UNITS_METRIC)
                {
                    if (fillVolume > MAX_VOLUME_L)
                    {
                        fillVolume = MAX_VOLUME_L;
                    }
                }
                fillVolume = setLocalVolume(fillVolume);
                validateVertHorzInput(gSetup.MaxTankVolume, fillVolume, gSetup.TankSensorVolume);
                (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, TankFillVolume), (DistVarType)fillVolume);
                PublishUint32(TOPIC_TankLevel, gSetup.TankFillVolume);
                
                fillPressure = FixedPointToInteger(gRun.Pressure_2_Psi, DECIMAL_PLACE_THREE);                
                (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, TankFillPressure), (DistVarType)fillPressure);
                
                fillVoltage = IN_Voltage_Pressure_Get_4to20mA(PRESSURE_SENSOR_CHANNEL_4TO20MA);
                (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, TankFillVoltage ), (DistVarType)fillVoltage.mV_s16d16);
                
                loadDigitBoxes();
                
                DEBUG_PRINT_STRING(DBUG_TANK, "fillPressure:");
                DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, fillPressure);
                DEBUG_PRINT_STRING(DBUG_TANK, "\r\n");                
            }
            else
            {
                (*gFillVolumeDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10000;
                (*gFillVolumeDigitBox).isEditMode = TRUE;
            }
            break;
            
        case FOCUS_SENSOR_VOLUME:
            if( (*gSensorVolumeDigitBox).isEditMode == TRUE )
            {
                (*gSensorVolumeDigitBox).isEditMode = FALSE;
                
                sensorVolume = GetCountDigitValue(&(*gSensorVolumeDigitBox).countDigit);
                // Check the screen inputs so it doesn't overflow while converting L to G
                // Note: Gallons are limited to 9999.9 G by the digit box so no need to check gallons
                if (gSetup.Units == UNITS_METRIC)
                {
                    if (sensorVolume > MAX_VOLUME_L)
                    {
                        sensorVolume = MAX_VOLUME_L;
                    }
                }
                sensorVolume = setLocalVolume(sensorVolume);
                validateVertHorzInput(gSetup.MaxTankVolume, gSetup.TankFillVolume, sensorVolume);            
                (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, TankSensorVolume), (DistVarType)sensorVolume);
                
                loadDigitBoxes();
            }
            else
            {
                (*gSensorVolumeDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10000;
                (*gSensorVolumeDigitBox).isEditMode = TRUE;
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
        basicFocusIndex = decrementFocusIndex(basicFocusIndex, NUMBER_BASIC_ITEMS);
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
        basicFocusIndex = incrementFocusIndex(basicFocusIndex, NUMBER_BASIC_ITEMS);
    } 
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_MAX_VOLUME] = &((*gMaxVolumeDigitBox).isFocus);
    isFocusArray[FOCUS_FILL_VOLUME] = &((*gFillVolumeDigitBox).isFocus);
    isFocusArray[FOCUS_SENSOR_VOLUME] = &((*gSensorVolumeDigitBox).isFocus);    
}

// **********************************************************************************************************
// incrementFocusIndex - Move focus to the next field
// **********************************************************************************************************
static uint16 incrementFocusIndex(uint16 focusIndex, uint16 numberItems)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex < (numberItems - 1) )
    {
        focusIndex = focusIndex + 1;
    }
    else
    {
        focusIndex = 0;
    }
    
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// decrementFocusIndex - Move focus to the previous field
// **********************************************************************************************************
static uint16 decrementFocusIndex(uint16 focusIndex, uint16 numberItems)
{
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    if( focusIndex > 0 )
    {
        focusIndex = focusIndex - 1;
    }
    else
    {
        focusIndex = (numberItems - 1);
    }
    
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

static void loadDigitBoxes()
{
    uint32 numDigits = NUM_VOLUME_DIGITS_G;
    maxVolume = getLocalVolume(gSetup.MaxTankVolume);
    fillVolume = getLocalVolume(gSetup.TankFillVolume);
    sensorVolume = getLocalVolume(gSetup.TankSensorVolume);
    
    // Default to 5 digits for gallons & change to 6 for liters if in metric
    if (gSetup.Units == UNITS_METRIC)
    {
        numDigits = NUM_VOLUME_DIGITS_L;
    }   
    
    (void)LoadCountDigit(&(*gMaxVolumeDigitBox).countDigit, maxVolume, numDigits, DECIMAL_POINT_ONE_DIGIT, 12, 1, FALSE, FALSE);
    (void)LoadCountDigit(&(*gFillVolumeDigitBox).countDigit, fillVolume, numDigits, DECIMAL_POINT_ONE_DIGIT, 12, 3, FALSE, FALSE);   
    (void)LoadCountDigit(&(*gSensorVolumeDigitBox).countDigit, sensorVolume, numDigits, DECIMAL_POINT_ONE_DIGIT, 12, 5, FALSE, FALSE);       
}

//****************************************************************************//
//Fcn: processInputRefreshScreenEvent
//
//Desc: This handles the refreshing screen event
//****************************************************************************//
static void processInputRefreshScreenEvent(void)
{
   if((*gMaxVolumeDigitBox).isEditMode == FALSE)
    {
        maxVolume = getLocalVolume(gSetup.MaxTankVolume);
        (void)LoadCountDigit(&(*gMaxVolumeDigitBox).countDigit, maxVolume, NUM_VOLUME_DIGITS, DECIMAL_POINT_ONE_DIGIT, 13, 1, FALSE, FALSE);
    } 
   if((*gFillVolumeDigitBox).isEditMode == FALSE)
    {
        fillVolume = getLocalVolume(gSetup.TankFillVolume);
        (void)LoadCountDigit(&(*gFillVolumeDigitBox).countDigit, fillVolume, NUM_VOLUME_DIGITS, DECIMAL_POINT_ONE_DIGIT, 13, 3, FALSE, FALSE);   
    }    
   if((*gSensorVolumeDigitBox).isEditMode == FALSE)
    {
        sensorVolume = getLocalVolume(gSetup.TankSensorVolume);
        (void)LoadCountDigit(&(*gSensorVolumeDigitBox).countDigit, sensorVolume, NUM_VOLUME_DIGITS, DECIMAL_POINT_ONE_DIGIT, 13, 5, FALSE, FALSE);       
    }   
}

