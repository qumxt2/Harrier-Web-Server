// TankScreenCustom.c

#include "stdio.h"

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Implements the logic for the custom tank screen which contains user settings & strap chart for custom tanks

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
#include "TankScreenCustom.h"
#include "PublishSubscribe.h"
#include "dvseg_17G721_levelChart.h"
#include "assert.h"
#include "debug_app.h"
#include "screenStuff.h"
#include "utilities.h"
#include "AlarmScreenTank.h"
#include "AdvancedScreen.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************
typedef enum
{
    FOCUS_FILL_HEIGHT = 0,
    FOCUS_SENSOR_HEIGHT,
    FOCUS_SENSOR_POSITION,
    FOCUS_LEVEL_CHART,
    NUMBER_LCHART_ITEMS
} LCHART_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static DIGIT_BOX_t * gFillHeightDigitBox;
static DIGIT_BOX_t * gSensorHeightDigitBox;
static DIGIT_BOX_t * gTankLevelDigitBox;
static DIGIT_BOX_t * gTankVolumeDigitBox;

static SELECTION_BOX_t* gSensorPositionSelectBox;

static char* ppPositionSelectBoxTextList[] =
{
    "ABOVE",
    "BELOW"
};

static INPUT_MODE_t ReturnMode;
static LCHART_FOCUS_t levelChartFocusIndex = FOCUS_FILL_HEIGHT;
static COLUMN_FOCUS_t columnFocusIndex = FOCUS_VOLUME_COLUMN;
static uint32 fillHeight = 0;
static IOrtn_mV_to_psi_int_t fillPressure;
static uint32 sensorHeight = 0;
static uint8_t gLevelChartIndex = 0;
static uint32 tankLevel = 0;
static uint32 tankVolume = 0;
static SENSOR_POSITIONS_t sensorPosition = POSITION_ABOVE_TANK;
static bool* isFocusArray[NUMBER_LCHART_ITEMS];
static bool gTankCalcsComplete = FALSE;

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawLevelChartScreen(LCHART_FOCUS_t focusIndex, COLUMN_FOCUS_t columnIndex);
static void drawLevelChart(LCHART_FOCUS_t focusIndex, COLUMN_FOCUS_t columnIndex);
static bool GetChartInterpolationIndex(uint32_t value, uint8_t * const minIndex, uint8_t * const maxIndex);
static uint16 incrementFocusIndex(uint16 focusIndex, uint16 numberItems);
static uint16 decrementFocusIndex(uint16 focusIndex, uint16 numberItems);
static void LoadTankVolumeDigitBox(void);
static void LoadTankLevelDigitBox(void);
static void LoadHeightParameterDigitBoxes(void);
static void ValidateLevelChart(DistVarType levelColumn[], DistVarType volumeColumn[]);
static void loadDigitBoxes();
static bool ValidateFillHeight(void);
static bool ValidateSensorHeight(void);
static void UpdateMaxTankVolume(void);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void processStrapChartInputRightArrowEvent(void);
static void processStrapChartInputLeftArrowEvent(void);
static void loadIsFocusArray(void);
static void processInputRefreshScreenEvent(void);
static uint32 getLocalHeight(uint32 height);
static void setLocalHeight(uint32 height, uint32 dvarID);

// **********************************************************************************************************
// TankScreenCustom - The main handler for the level chart display
// **********************************************************************************************************
INPUT_MODE_t TankScreenCustom(INPUT_EVENT_t InputEvent)
{  
    ReturnMode = INPUT_MODE_CUSTOM;

    void (*processInputEvent[NUMBER_OF_INPUT_EVENTS])(void);
      
    processInputEvent[INPUT_EVENT_ENTRY_INIT] = processInputEntryEvent;
    processInputEvent[INPUT_EVENT_RESET] = processInputResetEvent;
    processInputEvent[INPUT_EVENT_ENTER] = processInputEnterEvent;
    processInputEvent[INPUT_EVENT_UP_ARROW] = processInputUpArrowEvent;
    processInputEvent[INPUT_EVENT_DOWN_ARROW] = processInputDownArrowEvent;
    processInputEvent[INPUT_EVENT_RIGHT_ARROW] = processStrapChartInputRightArrowEvent;
    processInputEvent[INPUT_EVENT_LEFT_ARROW] = processStrapChartInputLeftArrowEvent;
    processInputEvent[INPUT_EVENT_PRESS_HOLD_ENTER] = processInputDefaultEvent;
    processInputEvent[INPUT_EVENT_BOTH_ARROWS] = processInputDefaultEvent;
    processInputEvent[INPUT_EVENT_REFRESH_SCREEN] = processInputRefreshScreenEvent;    
   
    // Process based on input event
    (void)(*processInputEvent[InputEvent])();

    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawLevelChartScreen(levelChartFocusIndex, columnFocusIndex);
    
    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// drawLevelChartScreen - Draw the rest of the level chart screen
// **********************************************************************************************************
static void drawLevelChartScreen(LCHART_FOCUS_t focusIndex, COLUMN_FOCUS_t columnIndex)
{
    gsetcpos(0, 0);
    gputs("CURRENT HEIGHT");
    gsetcpos(20, 0);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("CM");
    }
    else
    {
        gputs("IN");
    }
    
    gsetcpos(0, 1);
    gputs("SENSOR HEIGHT");
    gsetcpos(20, 1);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("CM");
    }
    else
    {
        gputs("IN");
    }
    
    gsetcpos(0, 2);
    gputs("SENSOR LOC");
    
    // Always draw lines 1 to 4
    drawLevelChart(focusIndex, columnIndex);

    if(levelChartFocusIndex == FOCUS_LEVEL_CHART)
    {
        if(columnIndex == FOCUS_VOLUME_COLUMN)
        {
            (*gTankVolumeDigitBox).isHidden = FALSE;
            (*gTankVolumeDigitBox).isFocus = TRUE;
            (*gTankLevelDigitBox).isHidden = TRUE; 
            (*gTankLevelDigitBox).isFocus = FALSE;
        }
        else
        {
            (*gTankVolumeDigitBox).isHidden = TRUE;
            (*gTankVolumeDigitBox).isFocus = FALSE;
            (*gTankLevelDigitBox).isHidden = FALSE;
            (*gTankLevelDigitBox).isFocus = TRUE;
        }
    }
    else
    {
        (*gTankVolumeDigitBox).isHidden = TRUE;
        (*gTankVolumeDigitBox).isFocus = FALSE;
        (*gTankLevelDigitBox).isHidden = TRUE; 
        (*gTankLevelDigitBox).isFocus = FALSE;
    }

    drawAllDigitBoxes();
    drawAllSelectBoxes();
}

// **********************************************************************************************************
// UpdateTankLevelCustom
// **********************************************************************************************************
void UpdateTankLevelCustom(void)
{
    sint32 pressure_x_1000 = 0;
    uint32 height = 0;
    uint8_t minIndex = INVALID_CHART_INDEX;
    uint8_t maxIndex = INVALID_CHART_INDEX;
    uint32 volume = 0;
    
    if (gSetup.TankSetupComplete)
    {
        pressure_x_1000 = FixedPointToInteger(gRun.Pressure_2_Psi, DECIMAL_PLACE_THREE);

        // PSI = 0.433 * (HEIGHT")/(12"/Ft)*(Fluid Density)/(8.33 PSI/Ft).  Height = xxx.xx * 100 format
        if (gSetup.FluidDensity > 0)
        {
            height = integerDivideRound(pressure_x_1000 * TANK_VOLUME_SCALE_FACTOR, (gSetup.FluidDensity * 100));
        }

        if (GetChartInterpolationIndex(height, &minIndex, &maxIndex) == TRUE)
        {
            volume = Interpolate(height, gLevelChart.TankLevel[minIndex], gLevelChart.TankVolume[minIndex],
                                         gLevelChart.TankLevel[maxIndex], gLevelChart.TankVolume[maxIndex]);
        }
        else if (gLevelChart.TankVolume[0] != 0)
        {
            // Check for a (x,0) entry in chart and if none found, force a (0,0) point
            if ((gLevelChart.TankLevel[0] != INVALID_CHART_ENTRY_DVAR_LVL) || (gLevelChart.TankVolume[0] != INVALID_CHART_ENTRY_DVAR_VOL))
            {
                volume = Interpolate(height, 0, 0, gLevelChart.TankLevel[0], gLevelChart.TankVolume[0]);
            }
        }
    }
    gRun.TankLevel = volume;
    gRun.TankHeightCalc = height;

    // Send info to debug portal if DBUG_TANK bit is set in debug mask
    DEBUG_PRINT_STRING(DBUG_TANK, "pressure_x_1000/Height_x_100/Volume_x_100,");
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, pressure_x_1000);
    DEBUG_PRINT_STRING(DBUG_TANK, ",");        
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, height);
    DEBUG_PRINT_STRING(DBUG_TANK, ",");          
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, gRun.TankLevel);
    DEBUG_PRINT_STRING(DBUG_TANK, ",Density,");
    DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, gSetup.FluidDensity);        
    DEBUG_PRINT_STRING(DBUG_TANK, "\r\n");
}

// **********************************************************************************************************
// drawLevelChart
// **********************************************************************************************************
static void drawLevelChart(LCHART_FOCUS_t focusIndex, COLUMN_FOCUS_t columnIndex)
{   
    uint8_t i;
    uint8_t index;
    char buf[25];
    uint32 localTankVolume = 0;
    uint32 localTankLevel = 0;
    
    gsetcpos(3, 3);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("VOL (L)");
    }
    else
    {
        gputs("VOL (GAL)");
    }
    gsetcpos(14, 3);
    if (gSetup.Units == UNITS_METRIC)
    {
        gputs("LVL (CM)");
    }
    else
    {
        gputs("LVL (IN)");
    }
    (void)drawLine(0, 46, 159, 46);
    
    for (i = 0; i < 4; i++)
    {    
        index = (gLevelChartIndex - gLevelChartIndex%4) + i;
        if (gLevelChart.TankVolume[index] != INVALID_CHART_ENTRY_DVAR_VOL)
        {
            localTankVolume = getLocalVolume(gLevelChart.TankVolume[index]);
        }
        if (gLevelChart.TankLevel[index] != INVALID_CHART_ENTRY_DVAR_LVL)
        {
            localTankLevel = getLocalHeight(gLevelChart.TankLevel[index]);
        }

        // A digit box is displayed in the column on the line we're currently editing, so display text for the opposite column
        if ((gLevelChartIndex % 4 == i) && (focusIndex == FOCUS_LEVEL_CHART))
        {
            if (columnIndex == FOCUS_VOLUME_COLUMN)
            { 
                // Editing the volume column so display the text for the level column
                if (gLevelChart.TankLevel[index] != INVALID_CHART_ENTRY_DVAR_LVL)
                {
                    gsetcpos(0, 4 + i);
                    (void)sprintf(buf, "%d.", index + 1);
                    gputs(buf);
                    gsetcpos(14, 4 + i);
                    if (gSetup.Units == UNITS_METRIC)
                    {
                        (void)sprintf(buf, "%03d.%01d", localTankLevel/10, localTankLevel%10);
                    }
                    else
                    {
                        (void)sprintf(buf, "%03d.%02u", localTankLevel/100, localTankLevel%100);
                    }
                    gputs(buf);
                }
                else
                {
                    gsetcpos(0, 4 + i);
                    (void)sprintf(buf, "%d.", index + 1);
                    gputs(buf);
                    gsetcpos(14, 4 + i);
                    gputs("-----");
                }
                (*gTankVolumeDigitBox).countDigit.PositionX = 3;
                (*gTankVolumeDigitBox).countDigit.PositionY = 4 + i;
            }
            else if (columnIndex == FOCUS_LEVEL_COLUMN)
            {
                // Editing the level column so display the text for the volume column
                if (gLevelChart.TankVolume[index] != INVALID_CHART_ENTRY_DVAR_VOL)
                {
                    gsetcpos(0, 4 + i);
                    (void)sprintf(buf, "%d.", index + 1);
                    gputs(buf);
                    gsetcpos(3, 4 + i);
                    if (gSetup.Units == UNITS_METRIC)
                    {
                        (void)sprintf(buf, "%05d.%01d", localTankVolume/10, localTankVolume%10);
                    }
                    else
                    {
                        (void)sprintf(buf, "%04d.%01d", localTankVolume/10, localTankVolume%10);
                    }
                    gputs(buf);
                }
                else
                {
                    gsetcpos(0, 4 + i);
                    (void)sprintf(buf, "%d.", index + 1);
                    gputs(buf);
                    gsetcpos(3, 4 + i);
                    gputs("-----");
                }
                (*gTankLevelDigitBox).countDigit.PositionX = 14;
                (*gTankLevelDigitBox).countDigit.PositionY = 4 + i;                
            }
        }
        // No digit box to display.  Display either the current value or dashes for "invalid"/blank entries
        else
        {
            if ((gLevelChart.TankVolume[index] != INVALID_CHART_ENTRY_DVAR_VOL) && (gLevelChart.TankLevel[index] != INVALID_CHART_ENTRY_DVAR_LVL))
            {
                // Both columns have valid values so display them
                gsetcpos(0, 4 + i);
                (void)sprintf(buf, "%d.", index + 1);
                gputs(buf);
                gsetcpos(3, 4 + i);
                if (gSetup.Units == UNITS_METRIC)
                {
                    (void)sprintf(buf, "%05d.%01d", localTankVolume/10, localTankVolume%10);
                }
                else
                {
                    (void)sprintf(buf, "%04d.%01d", localTankVolume/10, localTankVolume%10);
                }
                gputs(buf);
                gsetcpos(14, 4 + i);
                if (gSetup.Units == UNITS_METRIC)
                {
                    (void)sprintf(buf, "%03d.%.01d", localTankLevel/10, localTankLevel%10);
                }
                else
                {
                    (void)sprintf(buf, "%03d.%02u", localTankLevel/100, localTankLevel%100);
                }
                gputs(buf);
            }
            else if (gLevelChart.TankVolume[index] != INVALID_CHART_ENTRY_DVAR_VOL)
            {
                // Display the value in the volume column & dashes in the level column
                gsetcpos(0, 4 + i);
                (void)sprintf(buf, "%d.", index + 1);
                gputs(buf);
                gsetcpos(3, 4 + i);
                if (gSetup.Units == UNITS_METRIC)
                {
                    (void)sprintf(buf, "%05d.%01d", localTankVolume/10, localTankVolume%10);
                }
                else
                {
                    (void)sprintf(buf, "%04d.%01d", localTankVolume/10, localTankVolume%10);
                }
                gputs(buf);
                gsetcpos(14, 4 + i);
                gputs("-----");
            }
            else if (gLevelChart.TankLevel[index] != INVALID_CHART_ENTRY_DVAR_LVL)
            {
                // Display the value in the level column & dashes in the volume column
                gsetcpos(0, 4 + i);
                (void)sprintf(buf, "%d.", index + 1);
                gputs(buf);
                gsetcpos(3, 4 + i);
                gputs("-----");
                gsetcpos(14, 4 + i);
                if (gSetup.Units == UNITS_METRIC)
                {
                    (void)sprintf(buf, "%03d.%01d", localTankLevel/10, localTankLevel%10);
                }
                else
                {
                    (void)sprintf(buf, "%03d.%02u", localTankLevel/100, localTankLevel%100);
                }
                gputs(buf);
            }
            else
            {
                gsetcpos(0, 4 + i);
                (void)sprintf(buf, "%d.", index + 1);
                gputs(buf);
                gsetcpos(3, 4 + i);
                gputs("-----");
                gsetcpos(14, 4 + i);
                gputs("-----");
            }
        }
    }                    
}

// **********************************************************************************************************
// GetChartInterpolationIndex - Assumes chart values are entered in order, smallest to largest
// **********************************************************************************************************
static bool GetChartInterpolationIndex(uint32_t value, uint8_t * const minIndex, uint8_t  * const maxIndex)
{
    static sint8 i;
    uint8_t numValidRows = 0;
    
    *minIndex = INVALID_CHART_INDEX;
    *maxIndex = INVALID_CHART_INDEX;

    // Find the # of valid rows.  Assumes chart is sorted with invalid rows last
    for (i = 0; i < NUM_STRAP_CHART_LINES; i++)
    {
        if ((gLevelChart.TankLevel[i] != INVALID_CHART_ENTRY_DVAR_LVL) && (gLevelChart.TankVolume[i] != INVALID_CHART_ENTRY_DVAR_VOL))
        {
            numValidRows++;
        }
    }
    
    // Need at least two valid rows
    if (numValidRows >= 2)
    {
        // Clamp high out of range values to the max valid chart entry
        if (value > gLevelChart.TankLevel[numValidRows-1]) value = gLevelChart.TankLevel[numValidRows-1];

        // Find min index
        for (i = (numValidRows - 2); i >= 0; i--)
        {
            if (gLevelChart.TankLevel[i] != INVALID_CHART_ENTRY_DVAR_LVL)
            {
                if (value >= gLevelChart.TankLevel[i])
                {
                   *minIndex = i;
                    break;
                }
            }
        }      

        // Find max index
        for (i=1; i < numValidRows; i++)
        {
            if (gLevelChart.TankLevel[i] != INVALID_CHART_ENTRY_DVAR_LVL)
            {
                if (value < gLevelChart.TankLevel[i])
                {
                    *maxIndex = i;
                    break;
                }
            }
        }
        
        // Check for a max index = last entry in the chart
        if ((*maxIndex == INVALID_CHART_INDEX) && (value == gLevelChart.TankLevel[numValidRows-1]))
        {
            *maxIndex = numValidRows - 1;
        }
    }
    
    // Make sure it found a valid region to prevent divide by zero errors during interpolation
    if ((*minIndex == INVALID_CHART_INDEX) || (*maxIndex == INVALID_CHART_INDEX) || (*minIndex == *maxIndex))
    {
        return FALSE;
    }
    return TRUE;
}

// **********************************************************************************************************
// ValidateLevelChart - Sorts the arrays for the volume interpolation calculations
// **********************************************************************************************************
static void ValidateLevelChart(DistVarType levelColumn[], DistVarType volumeColumn[])
{
    uint8 i;
    DistVarType volume[NUM_STRAP_CHART_LINES];
    DistVarType level[NUM_STRAP_CHART_LINES];
    
    // If either entry in a row is invalid, force them both to be invalid
    for(i=0; i<NUM_STRAP_CHART_LINES; ++i)
    {
        if ((levelColumn[i] == INVALID_CHART_ENTRY_DVAR_LVL) && (volumeColumn[i] != INVALID_CHART_ENTRY_DVAR_VOL))
        {
            // If either entry is invalid, force them both to be invalid
            (void)DVAR_SetPointLocal_wRetry( DVA17G721_SA( gLevelChart, TankVolume, i ), INVALID_CHART_ENTRY_DVAR_VOL);
        }
        else if ((volumeColumn[i] == INVALID_CHART_ENTRY_DVAR_VOL) && (levelColumn[i] != INVALID_CHART_ENTRY_DVAR_LVL))
        {
            // If either entry is invalid, force them both to be invalid
            (void)DVAR_SetPointLocal_wRetry( DVA17G721_SA( gLevelChart, TankLevel, i ), INVALID_CHART_ENTRY_DVAR_LVL);
        }        
        level[i] =  levelColumn[i];
        volume[i] = volumeColumn[i];
    }
                
    // Sort the columns by level (height) & force invalid entries to the end of the chart
    TankArraySort(level, volume);
                
    // Update the dvars if the order has changed
    for(i=0; i<NUM_STRAP_CHART_LINES; ++i)
    {
        if (gLevelChart.TankLevel[i] != level[i])
        {
            (void)DVAR_SetPointLocal_wRetry( DVA17G721_SA( gLevelChart, TankLevel, i ), level[i]);
        }

        if (gLevelChart.TankVolume[i] != volume[i])
        {
            (void)DVAR_SetPointLocal_wRetry( DVA17G721_SA( gLevelChart, TankVolume, i ), volume[i]);
        }
    }
}

// **********************************************************************************************************
// LoadTankVolumeDigitBox
// **********************************************************************************************************
static void LoadTankVolumeDigitBox(void)
{
    uint32 numDigits = NUM_VOLUME_DIGITS_G;
    uint32 localTankVolume;
    
    if (gSetup.Units == UNITS_METRIC)
    {
        numDigits = NUM_VOLUME_DIGITS_L;
    }
    
    if(gLevelChart.TankVolume[gLevelChartIndex] == INVALID_CHART_ENTRY_DVAR_VOL)
    {
        (void)LoadCountDigit(&(*gTankVolumeDigitBox).countDigit, 0, numDigits, DECIMAL_POINT_ONE_DIGIT, 3, 6, FALSE, FALSE);
    }
    else
    {
        localTankVolume = getLocalVolume(gLevelChart.TankVolume[gLevelChartIndex]);
        (void)LoadCountDigit(&(*gTankVolumeDigitBox).countDigit, localTankVolume, numDigits, DECIMAL_POINT_ONE_DIGIT, 3, 6, FALSE, FALSE);
    }
}

// **********************************************************************************************************
// LoadTankLevelDigitBox
// **********************************************************************************************************
static void LoadTankLevelDigitBox(void)
{
    uint32 localTankLevel = 0;
    
    if(gLevelChart.TankLevel[gLevelChartIndex] != INVALID_CHART_ENTRY_DVAR_LVL)
    {
        localTankLevel = getLocalHeight(gLevelChart.TankLevel[gLevelChartIndex]);
    }

    // Display two decimals for inches and one decimal for centimeters
    if (gSetup.Units == UNITS_METRIC)
    {
        (void)LoadCountDigit(&(*gTankLevelDigitBox).countDigit, localTankLevel, NUM_TANK_LEVEL_DIGITS-1, DECIMAL_POINT_ONE_DIGIT, 14, 6, FALSE, FALSE);
    }
    else
    {
        (void)LoadCountDigit(&(*gTankLevelDigitBox).countDigit, localTankLevel, NUM_TANK_LEVEL_DIGITS, DECIMAL_POINT_TWO_DIGIT, 14, 6, FALSE, FALSE);
    }    
}

// **********************************************************************************************************
// LoadHeightParameterDigitBoxes - Display one decimal point for centimeters & two for inches
// **********************************************************************************************************
static void LoadHeightParameterDigitBoxes(void)
{
    fillHeight = getLocalHeight(gSetup.TankFillHeight);
    sensorHeight = getLocalHeight(gSetup.TankSensorHeight);
    
    if(gSetup.Units == UNITS_METRIC)
    {
        (void)LoadCountDigit(&(*gFillHeightDigitBox).countDigit, fillHeight, NUM_TANK_LEVEL_DIGITS-1, DECIMAL_POINT_ONE_DIGIT, 14, 0, FALSE, FALSE);
        (void)LoadCountDigit(&(*gSensorHeightDigitBox).countDigit, sensorHeight, NUM_TANK_LEVEL_DIGITS-1, DECIMAL_POINT_ONE_DIGIT, 14, 1, FALSE, FALSE);        
    }
    else
    {
        (void)LoadCountDigit(&(*gFillHeightDigitBox).countDigit, fillHeight, NUM_TANK_LEVEL_DIGITS, DECIMAL_POINT_TWO_DIGIT, 14, 0, FALSE, FALSE);
        (void)LoadCountDigit(&(*gSensorHeightDigitBox).countDigit, sensorHeight, NUM_TANK_LEVEL_DIGITS, DECIMAL_POINT_TWO_DIGIT, 14, 1, FALSE, FALSE);        
    }    
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
    
    gLevelChartIndex = 0;
    sensorPosition = gSetup.TankSensorPosition;
    
    //load shared digit boxes
    gFillHeightDigitBox = &digitBox1;
    gSensorHeightDigitBox = &digitBox2;
    gTankLevelDigitBox = &digitBox3;
    gTankVolumeDigitBox = &digitBox4;
    loadDigitBoxes();
    
    //unhide the required boxes
    (*gFillHeightDigitBox).isHidden = FALSE;
    (*gSensorHeightDigitBox).isHidden = FALSE;
	(*gTankLevelDigitBox).isHidden = FALSE;
    (*gTankVolumeDigitBox).isHidden = FALSE;
    
    //load shared select boxes
    gSensorPositionSelectBox = &selectBox1;
    (void) selectBoxConfigure(gSensorPositionSelectBox, 0, NUMBER_SENSOR_POSITIONS, FALSE, FALSE, FALSE, FALSE, 14, 2, 6, ppPositionSelectBoxTextList);
    (*gSensorPositionSelectBox).index = sensorPosition;    
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    levelChartFocusIndex = FOCUS_FILL_HEIGHT;
    *isFocusArray[levelChartFocusIndex] = TRUE;    
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    bool tankSetupStatus = FALSE;
    
    tankSetupStatus = gSetup.TankSetupComplete;     // Make sure a fill height has been entered & pressure read successfully

    if((anyDigitBoxIsEdit() == FALSE) && (anySelectBoxIsEdit() == FALSE))
    {
        hideAllBoxes();
        clearAllIsFocus();   
        ReturnMode = INPUT_MODE_TANK;        
        
        // Sort charts when exiting screen
        ValidateLevelChart(gLevelChart.TankLevel, gLevelChart.TankVolume);
        UpdateMaxTankVolume();
        
        if (!gTankCalcsComplete)
        {
            // Calculate density and offset based on values entered previously for fill height and sensor height
            if (tankSetupStatus == TRUE)
            {
                tankSetupStatus = CalculateFluidDensity();
            }
            else
            {
                DEBUG_PRINT_STRING(DBUG_PUMP, "Density calc skipped");
            }
            if (tankSetupStatus == TRUE)    
            {
                tankSetupStatus = CalculatePressureSensorOffset();
            }
            else
            {
                DEBUG_PRINT_STRING(DBUG_PUMP, "Offset calc skipped");    
            }
            if (tankSetupStatus != gSetup.TankSetupComplete)
            {
                (void)DVAR_SetPointLocal( DVA17G721_SS( gSetup, TankSetupComplete ), (DistVarType)tankSetupStatus );
            }
            if (!tankSetupStatus)
            {
                // Zero heights on setup screen & force tank level to 0 w/ TankSetupComplete to make it obvious there was a problem
                (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankFillHeight ), 0 );
                (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankSensorHeight ), 0 );
                DEBUG_PRINT_STRING(DBUG_PUMP, "Error, cleared fill & sensor heights...");    
            }
            gTankCalcsComplete = tankSetupStatus;
        }
    }
    loadDigitBoxes();
    (*gSensorPositionSelectBox).index = sensorPosition;
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{   
    fillPressure.psi_int = 0;
    fillPressure.mV_int = 0;
    fillPressure.error = IOError_UNDEFINED;
    
    switch( levelChartFocusIndex )
    {
        case FOCUS_FILL_HEIGHT:
            if(( (*gFillHeightDigitBox).isEditMode == TRUE ) && (gRun.refPsiInProcess == FALSE))
            {
                // Only get fill pressure if a valid height was entered
                // Doesn't catch a height that is too high or low for measured pressure
                gTankCalcsComplete = FALSE;
                fillHeight = GetCountDigitValue(&(*gFillHeightDigitBox).countDigit);
                if (gSetup.Units == UNITS_METRIC)
                {
                    // CM are entered as xxx.x so we need to x10 to match inches, which are entered as xxx.xx
                    fillHeight *= 10;
                    fillHeight = centimetersToInches(fillHeight);
                }
                if (ValidateFillHeight())
                {
                    gRun.refPsiInProcess = TRUE;
                    fillPressure = IOHARDWARE_PressureSensorRef(IOHARDWARE_PRESSURESENSOR_B);
                    gRun.refPsiInProcess = FALSE;
                    
                    if ((fillPressure.error == IOError_Ok) && (fillPressure.psi_int > 0))
                    {
                        (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankFillHeight ), fillHeight );
                        (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankSetupComplete ), (DistVarType)TRUE );
                    }
                    else
                    {
                        // Zero fill height on setup screen & force tank level to 0 w/ TankSetupComplete to make it obvious there was a problem
                        (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankFillHeight ), 0 );
                        (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankSetupComplete ), (DistVarType)FALSE );
                        DEBUG_PRINT_STRING(DBUG_TANK, "Error getting ref psi, clearing fill height...");
                    }
                    (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankFillPressure ), (DistVarType)fillPressure.psi_int);
                    (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankFillVoltage ), (DistVarType)IntegerToFixedPoint(fillPressure.mV_int, DECIMAL_PLACE_THREE ));
                }

                DEBUG_PRINT_STRING(DBUG_TANK, "fillPressure:");
                DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, fillPressure.psi_int);
                DEBUG_PRINT_STRING(DBUG_TANK, "\r\n");
                
                loadDigitBoxes();
                (*gFillHeightDigitBox).isEditMode = FALSE;
            }
            else
            {
                (*gFillHeightDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gFillHeightDigitBox).isEditMode = TRUE;
            }
            break;
            
        case FOCUS_SENSOR_HEIGHT:
            if( (*gSensorHeightDigitBox).isEditMode == TRUE )
            {
                gTankCalcsComplete = FALSE;
                sensorHeight = GetCountDigitValue(&(*gSensorHeightDigitBox).countDigit);
                if (gSetup.Units == UNITS_METRIC)
                {
                    // CM are entered as xxx.x so we need to x10 to match inches, which are entered as xxx.xx
                    sensorHeight *= 10;
                    sensorHeight = centimetersToInches(sensorHeight);
                }
                if (ValidateSensorHeight())
                {
                    (void)DVAR_SetPointLocal( DVA17G721_SS( gSetup, TankSensorHeight ), sensorHeight );
                }
                loadDigitBoxes();
                (*gSensorHeightDigitBox).isEditMode = FALSE;
            }
            else
            {
                (*gSensorHeightDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gSensorHeightDigitBox).isEditMode = TRUE;
            }
            break;            
            
        case FOCUS_SENSOR_POSITION:
            if( (*gSensorPositionSelectBox).isEditMode == TRUE)
            {
                gTankCalcsComplete = FALSE;
                sensorPosition = (*gSensorPositionSelectBox).index;
                (*gSensorPositionSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal( DVA17G721_SS( gSetup, TankSensorPosition ), sensorPosition );                
            }
            else
            {
                (*gSensorPositionSelectBox).isEditMode = TRUE;
            }
            break;
        
        case FOCUS_LEVEL_CHART:
            switch(columnFocusIndex)
            {
                case FOCUS_LEVEL_COLUMN:
                    if( (*gTankLevelDigitBox).isEditMode == TRUE )
                    {
                        tankLevel = GetCountDigitValue(&(*gTankLevelDigitBox).countDigit);
                        (*gTankLevelDigitBox).isEditMode = FALSE;
                        if ((tankLevel == INVALID_CHART_ENTRY_LVL_IN) && (gSetup.Units == UNITS_IMPERIAL))
                        {
                            // Imperial units have two decimals so invalid entries will be 999.99 & we want to save them as 999.99
                            (void)DVAR_SetPointLocal( DVA17G721_SA( gLevelChart, TankLevel, gLevelChartIndex ), (DistVarType)INVALID_CHART_ENTRY_DVAR_LVL);
                        }
                        else if ((tankLevel == INVALID_CHART_ENTRY_LVL_CM) && (gSetup.Units == UNITS_METRIC))
                        {
                            // Metric units have one decimal so invalid entries will be 999.9 & we want to save them as 999.99
                            (void)DVAR_SetPointLocal( DVA17G721_SA( gLevelChart, TankLevel, gLevelChartIndex ), (DistVarType)INVALID_CHART_ENTRY_DVAR_LVL);
                        }
                        else
                        {
                            setLocalHeight(tankLevel, DVA17G721_SA( gLevelChart, TankLevel, gLevelChartIndex ));
                        }
                    }
                    else
                    {
                        (*gTankLevelDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10000;
                        (*gTankLevelDigitBox).isEditMode = TRUE;
                    }
                    break;

                case FOCUS_VOLUME_COLUMN:
                    if( (*gTankVolumeDigitBox).isEditMode == TRUE )
                    {
                        tankVolume = GetCountDigitValue(&(*gTankVolumeDigitBox).countDigit);
                        (*gTankVolumeDigitBox).isEditMode = FALSE;
                        if ((tankVolume == INVALID_CHART_ENTRY_VOL_G) && (gSetup.Units == UNITS_IMPERIAL))
                        {
                            // Imperial units have four digits & one decimal so invalid entries will be 9999.9 *10 & we want to save them as 9999.9 * 10
                            (void)DVAR_SetPointLocal( DVA17G721_SA( gLevelChart, TankVolume, gLevelChartIndex ), (DistVarType)INVALID_CHART_ENTRY_DVAR_VOL);
                        }
                        else if ((tankVolume == INVALID_CHART_ENTRY_VOL_L) && (gSetup.Units == UNITS_METRIC))
                        {
                            // Metric units have five digits & one decimal so invalid entries will be 99,999.9 *10 & we want to save them as 9999.9 * 10
                            (void)DVAR_SetPointLocal( DVA17G721_SA( gLevelChart, TankVolume, gLevelChartIndex ), (DistVarType)INVALID_CHART_ENTRY_DVAR_VOL);
                        }
                        else
                        {
                            // Check the screen input so it doesn't overflow while converting L to G
                            // Note: Gallons are limited to 9999.9 G by the digit box so no need to check gallons
                            if (gSetup.Units == UNITS_METRIC)
                            {
                                if (tankVolume > MAX_VOLUME_L)
                                {
                                    tankVolume = MAX_VOLUME_L;
                                }
                            }
                            tankVolume = setLocalVolume(tankVolume);
                            // Limited to 999.9 gallons or liters by digit box
                            (void)DVAR_SetPointLocal(DVA17G721_SA( gLevelChart, TankVolume, gLevelChartIndex ), (DistVarType)tankVolume);
                        }
                        UpdateMaxTankVolume();
                        loadDigitBoxes();
                    }
                    else
                    {
                        (*gTankVolumeDigitBox).countDigit.DigitSelected = COUNT_DIGIT_10000;
                        (*gTankVolumeDigitBox).isEditMode = TRUE;
                    }
                    break;

                default:
                    break;
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
    if(upEventForDigitBox() == FALSE && upEventForSelectBox() == FALSE)
    {
        if (levelChartFocusIndex == FOCUS_LEVEL_CHART)
        {
            gLevelChartIndex = decrementGenericFocusIndex(gLevelChartIndex, NUM_STRAP_CHART_LINES);

            // Reset the screen's main focus index when the level chart reaches the first line and rolls back to the last line
            if (gLevelChartIndex == NUM_STRAP_CHART_LINES - 1)
            {
                gLevelChartIndex = 0;
                levelChartFocusIndex = decrementFocusIndex(levelChartFocusIndex, NUMBER_LCHART_ITEMS);
            }
            // Load digit boxes with value at new index
            LoadTankLevelDigitBox();
            LoadTankVolumeDigitBox();
        }
        else
        {
            levelChartFocusIndex = decrementFocusIndex(levelChartFocusIndex, NUMBER_LCHART_ITEMS);
        }
    } 
}

//****************************************************************************//
// Fcn: processInputDownArrowEvent
//
// Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if(downEventForDigitBox() == FALSE && downEventForSelectBox() == FALSE)
    {
        if (levelChartFocusIndex == FOCUS_LEVEL_CHART)
        {        
            gLevelChartIndex = incrementGenericFocusIndex(gLevelChartIndex, NUM_STRAP_CHART_LINES);

            // Load digit boxes with value at new index
            LoadTankLevelDigitBox();
            LoadTankVolumeDigitBox();        
        }
        else
        {
            levelChartFocusIndex = incrementFocusIndex(levelChartFocusIndex, NUMBER_LCHART_ITEMS);
        }
    }
}

//****************************************************************************//
//Fcn: processInputRightArrowEvent
//
//Desc: This function processes the right arrow events
//****************************************************************************//
static void processStrapChartInputRightArrowEvent(void)
{       
    if(rightEventForDigitBox() == FALSE)
    {             
        columnFocusIndex = incrementGenericFocusIndex(columnFocusIndex, NUMBER_STRAP_CHART_COLUMNS);

        if(levelChartFocusIndex == FOCUS_LEVEL_CHART)
        {
            if (columnFocusIndex == FOCUS_VOLUME_COLUMN)
            {
                (*gTankVolumeDigitBox).isFocus = TRUE;
                (*gTankLevelDigitBox).isFocus = FALSE;
            }
            else
            {
                (*gTankVolumeDigitBox).isFocus = FALSE;
                (*gTankLevelDigitBox).isFocus = TRUE;
            }
        }
    }
}

//****************************************************************************//
//Fcn: processInputLeftEvent
//
//Desc: This function processes the left arrow events
//****************************************************************************//
static void processStrapChartInputLeftArrowEvent(void)
{                
    if(leftEventForDigitBox() == FALSE)
    {        
        columnFocusIndex = decrementGenericFocusIndex(columnFocusIndex, NUMBER_STRAP_CHART_COLUMNS);

        if (columnFocusIndex == FOCUS_VOLUME_COLUMN)
        {
            (*gTankVolumeDigitBox).isFocus = TRUE;
            (*gTankLevelDigitBox).isFocus = FALSE;
        }
        else
        {
            (*gTankVolumeDigitBox).isFocus = FALSE;
            (*gTankLevelDigitBox).isFocus = TRUE;
        }
    }
}

//****************************************************************************//       
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_FILL_HEIGHT] = &((*gFillHeightDigitBox).isFocus);
    isFocusArray[FOCUS_SENSOR_HEIGHT] = &((*gSensorHeightDigitBox).isFocus);
    isFocusArray[FOCUS_SENSOR_POSITION] = &((*gSensorPositionSelectBox).isFocus);
    isFocusArray[FOCUS_LEVEL_CHART] = &((*gTankVolumeDigitBox).isFocus);
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
    LoadHeightParameterDigitBoxes();
    LoadTankLevelDigitBox();
    LoadTankVolumeDigitBox();
}

static bool ValidateFillHeight(void)
{        
    if (fillHeight == 0)
    {
        return FALSE;
    }
    else if ((gSetup.TankSensorPosition == POSITION_ABOVE_TANK) && (fillHeight < gSetup.TankSensorHeight))
    {
        return FALSE;
    }
    else if (fillHeight == gSetup.TankSensorHeight)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static bool ValidateSensorHeight(void)
{        
    if ((gSetup.TankSensorPosition == POSITION_ABOVE_TANK) && (sensorHeight > gSetup.TankFillHeight))
    {
        return FALSE;
    }
    else if (sensorHeight == gSetup.TankFillHeight)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

// **********************************************************************************************************
// UpdateMaxTankVolume
// **********************************************************************************************************
static void UpdateMaxTankVolume(void)
{
    sint8 i;
    uint32 maxVolume = 0;
    
    if (gSetup.TankType == TANK_TYPE_CUSTOM)
    {
        for (i = (NUM_STRAP_CHART_LINES - 1); i >= 0; i--)
        {
            if (gLevelChart.TankVolume[i] != INVALID_CHART_ENTRY_DVAR_VOL)
            {
                maxVolume = gLevelChart.TankVolume[i];
                break;
            }
        }
        (void)DVAR_SetPointLocal( DVA17G721_SS(gSetup, MaxTankVolume), maxVolume);
        PublishUint32(TOPIC_TankLevelVolumeMax, gSetup.MaxTankVolume);
    }
}

//****************************************************************************//
//Fcn: processInputRefreshScreenEvent
//
//Desc: This handles the refreshing screen event
//****************************************************************************//
static void processInputRefreshScreenEvent(void)
{
    if((*gFillHeightDigitBox).isEditMode == FALSE)
    {
        fillHeight = gSetup.TankFillHeight;
        (void)LoadCountDigit(&(*gFillHeightDigitBox).countDigit, fillHeight, NUM_TANK_LEVEL_DIGITS, DECIMAL_POINT_TWO_DIGIT, 14, 0, FALSE, FALSE);
    } 
    if((*gSensorHeightDigitBox).isEditMode == FALSE)
    {
        sensorHeight = gSetup.TankSensorHeight;
        (void)LoadCountDigit(&(*gSensorHeightDigitBox).countDigit, sensorHeight, NUM_TANK_LEVEL_DIGITS, DECIMAL_POINT_TWO_DIGIT, 14, 1, FALSE, FALSE);
    }
    if((*gTankLevelDigitBox).isEditMode == FALSE)
    {
        LoadTankLevelDigitBox();
    }
    if((*gTankVolumeDigitBox).isEditMode == FALSE)
    {
        LoadTankVolumeDigitBox();
    }
}

static uint32 getLocalHeight(uint32 height)
{
    // The heights are stored internally in the XXX.XX *100 format.
    // They need to be divided by 10 for use on the display (XXX.X format) if in metric units
    
    if (gSetup.Units == UNITS_METRIC)
    {
        height = inchesToCentimeters(height);
        height = integerDivideRound(height,  10);
    }
    return (height);
}

// **********************************************************************************************************
// setLocalHeight - Convert the height parameter from local units to inches before storing in DVAR
// **********************************************************************************************************
static void setLocalHeight(uint32 height, uint32 dvarID)
{
    // Heights are stored as 5 digit ints, xxx.xx * 100.  Inches are entered as 5 digits xxx.xx * 100.
    // Centimeters are entered as 4 digits xxx.x * 10 so they need to be scaled up for storing
    
    if (gSetup.Units == UNITS_METRIC)
    {
        height *= 10;
        height = centimetersToInches(height);
    }
    
    (void)DVAR_SetPointLocal(dvarID, (DistVarType)height);
}

