// TankScreenHorizontal.c

#include "stdio.h"

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Implements the logic for the horizontal tank screen which contains user settings for horizontal tanks

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
#include "TankScreenHorizontal.h"
#include "PublishSubscribe.h"
#include "dvseg_17G721_levelChart.h"
#include "in_pressure.h"
#include "assert.h"
#include "debug_app.h"
#include "screenStuff.h"
#include "utilities.h"
#include "AdvancedScreen.h"
#include "limits.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_VOLUME_DIGITS                          (5)
#define HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS  (21)
#define PRESSURE_SENSOR_CHANNEL_4TO20MA            (5)

//This is a height radius ratio lookup table used to speed up the calculation process and to minimize the number of values that need to be entered by the user
//This is used in conjunction with the AREA_PERCENTAGE_X100 variable, Do NOT CHANGE ONE WITHOUT ADJUSTING THE OTHER
uint32 HEIGHT_RADIUS_RATIO_X10000[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS] =  
                                            {0,
                                             1000, 
                                             2000,
                                             3000,
                                             4000,
                                             5000, 
                                             6000, 
                                             7000, 
                                             8000, 
                                             9000, 
                                             10000, 
                                             11000, 
                                             12000, 
                                             13000, 
                                             14000, 
                                             15000, 
                                             16000, 
                                             17000, 
                                             18000, 
                                             19000, 
                                             20000}; 

//This is an area percentage table used to speed up the calculation process and to minimize the number of values that need to be entered by the user
//This is used in conjunction with the HEIGHT_RADIUS_RATIO_X10000 variable, Do NOT CHANGE ONE WITHOUT ADJUSTING THE OTHER
uint32 AREA_PERCENTAGE_X100[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS] =    
                                           {0,
                                            187,
                                            520,
                                            941,
                                            1424,
                                            1955, 
                                            2523,
                                            3119,
                                            3735,
                                            4364,
                                            5000,
                                            5636,
                                            6265,
                                            6881,
                                            7477,
                                            8045,
                                            8576,
                                            9059,
                                            9480,
                                            9813,
                                            10000}; 


// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************
typedef enum
{
    FOCUS_MAX_VOLUME = 0,
    FOCUS_FILL_VOLUME,
    FOCUS_SENSOR_VOLUME,
    NUMBER_HORIZONTAL_ITEMS
} HORIZONTAL_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t* gMaxVolumeDigitBox;
static DIGIT_BOX_t* gFillVolumeDigitBox;
static DIGIT_BOX_t* gSensorVolumeDigitBox;

static INPUT_MODE_t ReturnMode;
static HORIZONTAL_FOCUS_t horizontalFocusIndex = FOCUS_MAX_VOLUME;
static uint32 maxVolume = 0;
static uint32 fillVolume = 0;
static uint32 sensorVolume = 0;
static uint32 p_VS_HR_X10000 = 0;
static uint32 pressure_x_1000 = 0;
static IOrtn_mV_s16d16_t fillVoltage;
static bool* isFocusArray[NUMBER_HORIZONTAL_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawHorizontalScreen(HORIZONTAL_FOCUS_t focusIndex);
static uint16 incrementFocusIndex(uint16 focusIndex, uint16 numberItems);
static uint16 decrementFocusIndex(uint16 focusIndex, uint16 numberItems);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);
static uint32 getHeightRadiusRatioInterpolate_X10000(uint32 area_X100);
static uint32 getAreaInterpolate_X100(uint32 h_r_X10);
static uint32 getLowerIndexForInterpolate(uint32 *a, uint32 numEls, uint32 val);
static void processInputRefreshScreenEvent(void);
static void loadDigitBoxes(void);

// **********************************************************************************************************
// TankScreenHorizontal - The main handler for the horizontal tank screen display
// **********************************************************************************************************
INPUT_MODE_t TankScreenHorizontal(INPUT_EVENT_t InputEvent)
{  
    ReturnMode = INPUT_MODE_HORIZONTAL;

    void (*processInputEvent[NUMBER_OF_INPUT_EVENTS])(void);
      
    processInputEvent[INPUT_EVENT_ENTRY_INIT] = processInputEntryEvent;
    processInputEvent[INPUT_EVENT_RESET] = processInputResetEvent;
    processInputEvent[INPUT_EVENT_ENTER] = processInputEnterEvent;
    processInputEvent[INPUT_EVENT_UP_ARROW] = processInputUpArrowEvent;
    processInputEvent[INPUT_EVENT_DOWN_ARROW] = processInputDownArrowEvent;
    processInputEvent[INPUT_EVENT_RIGHT_ARROW] = processInputRightArrowEvent;
    processInputEvent[INPUT_EVENT_LEFT_ARROW] = processInputLeftArrowEvent;
    processInputEvent[INPUT_EVENT_PRESS_HOLD_ENTER] = processInputDefaultEvent;
    processInputEvent[INPUT_EVENT_REFRESH_SCREEN] = processInputRefreshScreenEvent;    
    processInputEvent[INPUT_EVENT_BOTH_ARROWS] = processInputDefaultEvent;
   
    // Process based on input event
    (void)(*processInputEvent[InputEvent])();

    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawHorizontalScreen(horizontalFocusIndex);
    
    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// drawHorizontalScreen - Draw the rest of the horizontal tank screen
// **********************************************************************************************************
static void drawHorizontalScreen(HORIZONTAL_FOCUS_t focusIndex)
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
// UpdateTankLevelHorizontal
// **********************************************************************************************************
void UpdateTankLevelHorizontal(void)
{
    uint32 volume = 0;    
    uint32 areaPercent_x100 = 0;
    uint32 sensorH_R_x10000 = 0;
    uint32 sensorVolumePercentage_X100 = 0;
    uint32 calcHR = 0;
    uint64 tempResult = 0;
   
    maxVolume = gSetup.MaxTankVolume;
    sensorVolume = gSetup.TankSensorVolume;
 
    sensorVolumePercentage_X100 = ((uint64)sensorVolume * 10000uLL) / maxVolume;   //100 to get to percent and 100 to scale
    sensorH_R_x10000 = getHeightRadiusRatioInterpolate_X10000(sensorVolumePercentage_X100);        
            
    pressure_x_1000 = FixedPointToInteger(gRun.Pressure_2_Psi, DECIMAL_PLACE_THREE);
    
    p_VS_HR_X10000 = gSetup.PressureVSHeightRadius_x10000;
    
    // Calculate HR
    // Catch some bad setup edge cases that cause results > uint32, ie: large initial HR (1/2 full or more), 0 sensor HR (sensor at bottom of tank), & 0.001 fill psi 
    tempResult = (uint64)pressure_x_1000 * (uint64)p_VS_HR_X10000 + 500uLL;  //500 is for rounding
    tempResult = tempResult / 1000;
    tempResult = tempResult + sensorH_R_x10000;
    // Clamp out of bounds results to uint32
    if (tempResult > UINT_MAX)
    {
        calcHR = UINT_MAX;
    }
    else
    {
        calcHR = (uint32)tempResult;
    }
    areaPercent_x100 = getAreaInterpolate_X100(calcHR);  
    
    // Calculate Volume
    tempResult = (uint64)maxVolume * (uint64)areaPercent_x100 + 5000uLL;    //undo the percent, round and x100
    tempResult = tempResult / 10000;
    // Clamp out of bounds results to uint32
    if (tempResult > UINT_MAX)
    {
        volume = UINT_MAX;
    }
    else
    {
        volume = (uint32)tempResult;
    }
    gRun.TankLevel = volume;
  
    // Send info to debug portal if DBUG_TANK bit is set in debug mask
    DEBUG_PRINT_STRING(DBUG_TANK, "\r\npressure_x_1000: ");
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, pressure_x_1000);
    DEBUG_PRINT_STRING(DBUG_TANK, "\r\n%Area of Circle Filled, %Area x 100: ");        
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, areaPercent_x100);
    DEBUG_PRINT_STRING(DBUG_TANK, "\r\nTank Volume x 100: ");          
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, gRun.TankLevel);
    DEBUG_PRINT_STRING(DBUG_TANK, "\r\nMax Tank Volume x 100: ");          
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, gSetup.MaxTankVolume);
    DEBUG_PRINT_STRING(DBUG_TANK, "GAL\r\n");
    DEBUG_PRINT_STRING(DBUG_TANK, "\r\npVSHR: ");          
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, p_VS_HR_X10000);
    DEBUG_PRINT_STRING(DBUG_TANK, "\r\nCalcHR: ");          
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, calcHR);
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
    horizontalFocusIndex = FOCUS_MAX_VOLUME;
    *isFocusArray[horizontalFocusIndex] = TRUE;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if( (anyDigitBoxIsEdit() == FALSE) &&
        (anySelectBoxIsEdit() == FALSE))
    {
        uint32 initalH_R_x10000 = 0;
        uint32 sensorH_R_x10000 = 0;
        uint32 sensorVolumePercentage_X100 = 0;
        uint32 initialFillPercentage_X100 = 0;
        //load all the saved values
        fillVolume = gSetup.TankFillVolume;
        maxVolume = gSetup.MaxTankVolume;
        sensorVolume = gSetup.TankSensorVolume;
        pressure_x_1000 = gSetup.TankFillPressure;
        
        hideAllBoxes();
        clearAllIsFocus();   
        ReturnMode = INPUT_MODE_TANK;
        
        //update the initial fill percentage
        initialFillPercentage_X100 = ((uint64)fillVolume * 10000uLL) / maxVolume;   //100 to get to percent and 100 to scale

        sensorVolumePercentage_X100 = ((uint64)sensorVolume * 10000uLL) / maxVolume;   //100 to get to percent and 100 to scale
        //We are assuming that 0 PSI is 0 height 
        
        if(pressure_x_1000 == 0)
        {
            // prevent divide by zero error, clamp to zero
            p_VS_HR_X10000 = 0;
        }
        else
        {
            initalH_R_x10000 = getHeightRadiusRatioInterpolate_X10000(initialFillPercentage_X100);
            sensorH_R_x10000 = getHeightRadiusRatioInterpolate_X10000(sensorVolumePercentage_X100);
            p_VS_HR_X10000 = (((initalH_R_x10000-sensorH_R_x10000)*1000)+(pressure_x_1000 >> 1)) / pressure_x_1000;    //get the pressure Vs H/R slope and convert to 10000X and round
        }

        (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, PressureVSHeightRadius_x10000 ), (DistVarType)p_VS_HR_X10000);
        
        DEBUG_PRINT_STRING(DBUG_TANK, "initialFillPercentage_X100: ");
        DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, initialFillPercentage_X100);
        DEBUG_PRINT_STRING(DBUG_TANK, "% pressureVSHeightRadius_X10000: ");        
        DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, p_VS_HR_X10000);
        DEBUG_PRINT_STRING(DBUG_TANK, "  initalH_R_x10000: ");          
        DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, initalH_R_x10000);
        DEBUG_PRINT_STRING(DBUG_TANK, "  pressure_x_1000: ");          
        DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, pressure_x_1000);
        DEBUG_PRINT_STRING(DBUG_TANK, "\r\n");   
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
    sint32 fillPressure = 0;
    maxVolume = gSetup.MaxTankVolume;
    fillVolume = gSetup.TankFillVolume;
    sensorVolume = gSetup.TankSensorVolume;
    switch( horizontalFocusIndex )
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
                validateVertHorzInput(maxVolume, fillVolume, sensorVolume);
                (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, MaxTankVolume), (DistVarType)maxVolume);
                PublishUint32(TOPIC_TankLevelVolumeMax, gSetup.MaxTankVolume);
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
                // Check the screen input so it doesn't overflow while converting L to G
                // Note: Gallons are limited to 9999.9 G by the digit box so no need to check gallons
                if (gSetup.Units == UNITS_METRIC)
                {
                    if (fillVolume > MAX_VOLUME_L)
                    {
                        fillVolume = MAX_VOLUME_L;
                    }
                }
                fillVolume = setLocalVolume(fillVolume);
                validateVertHorzInput(maxVolume, fillVolume, sensorVolume);
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
                // Check the screen input so it doesn't overflow while converting L to G
                // Note: Gallons are limited to 9999.9 G by the digit box so no need to check gallons
                if (gSetup.Units == UNITS_METRIC)
                {
                    if (sensorVolume > MAX_VOLUME_L)
                    {
                        sensorVolume = MAX_VOLUME_L;
                    }
                }
                sensorVolume = setLocalVolume(sensorVolume);
                validateVertHorzInput(maxVolume, fillVolume, sensorVolume);
                (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, TankSensorVolume), (DistVarType)sensorVolume);
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
    loadDigitBoxes();
}

//****************************************************************************//
//Fcn: processInputUpArrowEvent
//
//Desc: This function processes the up arrow events
//****************************************************************************//
static void processInputUpArrowEvent(void)
{
    if(upEventForDigitBox() == FALSE && upEventForSelectBox() == FALSE)
    {
        horizontalFocusIndex = decrementFocusIndex(horizontalFocusIndex, NUMBER_HORIZONTAL_ITEMS);
    }    
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if(downEventForDigitBox() == FALSE && downEventForSelectBox() == FALSE)
    {
        horizontalFocusIndex = incrementFocusIndex(horizontalFocusIndex, NUMBER_HORIZONTAL_ITEMS);
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

//******************************************************************************
//getHeightRadiusRatioInterpolate_X10
//
//desc: Pass in an area percentage fill, function returns the interpolated 
//  height radius ratio *10
//
//******************************************************************************
static uint32 getHeightRadiusRatioInterpolate_X10000(uint32 area_X100)
{
    uint32 lowerIndex = 0;
    uint32 h_r_X10000 = 0;
    
    lowerIndex = getLowerIndexForInterpolate(AREA_PERCENTAGE_X100, HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS, area_X100);
    
    //check to see if the value is greater than all values in the table, 
    //If so clamp to the largest value in the table
    if(lowerIndex == (HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS))
    {
        h_r_X10000 = Interpolate(area_X100, 
                AREA_PERCENTAGE_X100[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-2], HEIGHT_RADIUS_RATIO_X10000[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-2], 
                AREA_PERCENTAGE_X100[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-1], HEIGHT_RADIUS_RATIO_X10000[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-1]);
        //h_r_X10000 = HEIGHT_RADIUS_RATIO_X10000[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-1]; //used to clamp to the max
    }
    else
    {
        h_r_X10000 = Interpolate(area_X100, 
                AREA_PERCENTAGE_X100[lowerIndex], HEIGHT_RADIUS_RATIO_X10000[lowerIndex], 
                AREA_PERCENTAGE_X100[lowerIndex+1], HEIGHT_RADIUS_RATIO_X10000[lowerIndex+1]);
    }
    
    return h_r_X10000;
    
}

//******************************************************************************
//getAreaInterpolate_X100
//
//desc: Pass in the height radius ratio, function returns the interpolated 
//  area percentage *100
//
//******************************************************************************
static uint32 getAreaInterpolate_X100(uint32 h_r_X10000)
{
    uint32 lowerIndex = 0;
    uint32 area = 0;
    
    lowerIndex = getLowerIndexForInterpolate(HEIGHT_RADIUS_RATIO_X10000, HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS, h_r_X10000);
    
    //check to see if the value is greater than all values in the table, 
    //If so clamp to the largest value in the table
    if(lowerIndex == (HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS))
    {
        area = Interpolate(h_r_X10000, 
                HEIGHT_RADIUS_RATIO_X10000[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-2], AREA_PERCENTAGE_X100[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-2], 
                HEIGHT_RADIUS_RATIO_X10000[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-1], AREA_PERCENTAGE_X100[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-1]);
        //area = AREA_PERCENTAGE_X100[HORIZONTAL_CYLINDER_LOOKUP_TABLE_ELEMENTS-1]; //clamp the value to the max
    }
    else
    {
        area = Interpolate(h_r_X10000, 
                HEIGHT_RADIUS_RATIO_X10000[lowerIndex], AREA_PERCENTAGE_X100[lowerIndex], 
                HEIGHT_RADIUS_RATIO_X10000[lowerIndex+1], AREA_PERCENTAGE_X100[lowerIndex+1]);
    }
    
    DEBUG_PRINT_STRING(DBUG_TANK, "\r\nlowerIndex: ");
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, lowerIndex);
    DEBUG_PRINT_STRING(DBUG_TANK, " \r\nh_r_X10000: ");        
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, h_r_X10000);
    DEBUG_PRINT_STRING(DBUG_TANK, " \r\nAreaX100: ");        
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, area);
    DEBUG_PRINT_STRING(DBUG_TANK, " \r\n");
    
    return area;
}

//******************************************************************************
//getLowerIndexForInterpolate
//
//desc: This function will return the index of the low bound for interpolation
//      Function assumes the array is sorted in ascending order, numEls-1 is return
//      if the val is greater than all array elements
// 
//
//******************************************************************************
static uint32 getLowerIndexForInterpolate(uint32 *a, uint32 numEls, uint32 val)
{
   uint32 index = 0;
   
   bool found = FALSE;  // needed for the boundary condition so that we do not go out of bounds on the array
   while(index < numEls && !found)
   {
       if(a[index] > val)
       {
           found = TRUE;
           if(index !=0)    //index should never be 0 because of the >, however this will ensure no out of bounds arrays happen, error will be caught by interpolate function and resulting value will be forced to 0
           {
               --index; //go back 1 element for lower index
           }           
       }
       else
       {
          ++index;
       }
   }
   
   return (index);
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

