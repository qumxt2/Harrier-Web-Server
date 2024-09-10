// TankScreen.c

#include "stdio.h"

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Implements the logic for the main tank screen which contains supported tank shape options

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
#include "AdvancedScreen.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************
typedef enum
{
    FOCUS_TANK_TYPE = 0,
    NUMBER_TANK_ITEMS
} TANK_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************
static SELECTION_BOX_t* gTankModeSelectBox;

static char* ppTankModeSelectBoxTextList[] =
{
    "VERTICAL",
    "HORIZONTAL",
    "CUSTOM"
};

static INPUT_MODE_t ReturnMode;
static TANK_FOCUS_t tankFocusIndex = FOCUS_TANK_TYPE;
static TANK_TYPES_t tankType = TANK_TYPE_VERTICAL;
static bool* isFocusArray[NUMBER_TANK_ITEMS];

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawTankScreen(TANK_FOCUS_t focusIndex);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void processInputBothArrowsEvent(void);
static void loadIsFocusArray(void);
static void resetForVerticalHorizontal(void);
static void resetForCustom(void);

// **********************************************************************************************************
// TankScreen - The main handler for the tank screen display
// **********************************************************************************************************
INPUT_MODE_t TankScreen(INPUT_EVENT_t InputEvent)
{  
    ReturnMode = INPUT_MODE_TANK;

    void (*processInputEvent[NUMBER_OF_INPUT_EVENTS])(void);
      
    processInputEvent[INPUT_EVENT_ENTRY_INIT] = processInputEntryEvent;
    processInputEvent[INPUT_EVENT_RESET] = processInputResetEvent;
    processInputEvent[INPUT_EVENT_ENTER] = processInputEnterEvent;
    processInputEvent[INPUT_EVENT_UP_ARROW] = processInputUpArrowEvent;
    processInputEvent[INPUT_EVENT_DOWN_ARROW] = processInputDownArrowEvent;
    processInputEvent[INPUT_EVENT_RIGHT_ARROW] = processInputRightArrowEvent;
    processInputEvent[INPUT_EVENT_LEFT_ARROW] = processInputLeftArrowEvent;
    processInputEvent[INPUT_EVENT_PRESS_HOLD_ENTER] = processInputDefaultEvent;
    processInputEvent[INPUT_EVENT_BOTH_ARROWS] = processInputBothArrowsEvent;    
    processInputEvent[INPUT_EVENT_REFRESH_SCREEN] = processInputDefaultEvent;
   
    // Process based on input event
    (void)(*processInputEvent[InputEvent])();

    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawTankScreen(tankFocusIndex);
    
    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// drawTankScreen - Draw the rest of the tank screen
// **********************************************************************************************************
static void drawTankScreen(TANK_FOCUS_t focusIndex)
{
    gsetcpos(0, 1);
    gputs("TANK");

    drawAllDigitBoxes();
    drawAllSelectBoxes();
}

// **********************************************************************************************************
// UpdateTankLevel
// **********************************************************************************************************
void UpdateTankLevel(void)
{   
    switch(gSetup.TankType)
    {
        case TANK_TYPE_VERTICAL:
            UpdateTankLevelVertical();
            break;
            
        case TANK_TYPE_HORIZONTAL:
            UpdateTankLevelHorizontal();
            break;            
            
        case TANK_TYPE_CUSTOM:
            UpdateTankLevelCustom();
            break;
        
        default:
            break;
    }
}

// **********************************************************************************************************
// DVAR_SetPointLocal_wRetry
// **********************************************************************************************************
DVarErrorCode DVAR_SetPointLocal_wRetry(DistVarID id, DistVarType value)
{
    uint8 retryCount = 0;    
    DVarErrorCode status = DVarError_InvalidAddressRange;
    
    while ((status != DVarError_Ok) && (retryCount < 3))
    {
        status = DVAR_SetPointLocal( id, value);
        ++retryCount;
        (void)K_Task_Wait(6);    //delay before retrying
    }
    return status;
}

// **********************************************************************************************************
// TankArraySort - Sorts the arrays for the volume interpolation calculations
// **********************************************************************************************************
void TankArraySort(DistVarType lookupColumn[], DistVarType volumeColumn[])
{
    DistVarType tmp;
    static uint8 i;
    static uint8 j;
    uint8 minVal;
    
    for(i=0; i < NUM_STRAP_CHART_LINES; ++i)
    {
        minVal = i;
        
        for(j=i+1; j<NUM_STRAP_CHART_LINES; ++j)
        {
            if(lookupColumn[minVal] > lookupColumn[j])
            {
                minVal = j;
            }
        }
        
        //swap the lookup (height or pressure) entries
        tmp = lookupColumn[i];
        lookupColumn[i] = lookupColumn[minVal];
        lookupColumn[minVal] = tmp;
        
        //swap the volume entries
        tmp = volumeColumn[i];
        volumeColumn[i] = volumeColumn[minVal];
        volumeColumn[minVal] = tmp;        
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

    tankType = gSetup.TankType;    
    
    //load shared select boxes
    gTankModeSelectBox = &selectBox1;
    (void) selectBoxConfigure(gTankModeSelectBox, 0, NUMBER_TANK_TYPES, FALSE, FALSE, FALSE, FALSE, 6, 1, 14, ppTankModeSelectBoxTextList);
    (*gTankModeSelectBox).index = tankType;
    
    loadIsFocusArray();
    *isFocusArray[tankFocusIndex] = TRUE;
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    if((anyDigitBoxIsEdit() == FALSE) && (anySelectBoxIsEdit() == FALSE))
    {
        hideAllBoxes();
        clearAllIsFocus();   
        ReturnMode = INPUT_MODE_CONFIG;
    }
    (*gTankModeSelectBox).index = tankType;
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    bool newTankType = FALSE;
    switch( tankFocusIndex )
    {
        case FOCUS_TANK_TYPE:
            if( (*gTankModeSelectBox).isEditMode == TRUE)
            {
                tankType = (*gTankModeSelectBox).index;
                if(tankType != gSetup.TankType)
                {
                    newTankType = TRUE;
                    Restore_4to20mA_Transducer_Defaults();
                    (void)DVAR_SetPointLocal_wCallback (DVA17G721_SS( gRun, Pressure_2_Psi), 0);
                }
                (*gTankModeSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal (DVA17G721_SS( gSetup, TankType), tankType );
                PublishUint32(TOPIC_TankType, gSetup.TankType + 1);
                switch(gSetup.TankType)
                {
                    case TANK_TYPE_VERTICAL:
                        if(newTankType)
                        {
                            resetForVerticalHorizontal();
                        }
                        ReturnMode = INPUT_MODE_VERTICAL;
                        break;
                    case TANK_TYPE_HORIZONTAL:
                        if(newTankType)
                        {
                            resetForVerticalHorizontal();
                        }
                        ReturnMode = INPUT_MODE_HORIZONTAL;
                        break;                        
                    case TANK_TYPE_CUSTOM:
                        if(newTankType)
                        {
                            resetForCustom();
                        }
                        ReturnMode = INPUT_MODE_CUSTOM;
                        break;
                    default:
                        break;
                }
            }
            else
            {
                (*gTankModeSelectBox).isEditMode = TRUE;
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
//Fcn: processInputBothArrowsEvent
//
//Desc: This function processes the both arrows event
//****************************************************************************//
static void processInputBothArrowsEvent(void)
{
    ReturnMode = INPUT_MODE_TANK_DEBUG;
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_TANK_TYPE] = &((*gTankModeSelectBox).isFocus);
}

// **********************************************************************************************************
// validateFillVolume - Limit checking for the volume settings
// **********************************************************************************************************
uint32 validateVolumeEntry(uint32 volumeEntry)
{
    if( volumeEntry > gSetup.MaxTankVolume )
    {
        volumeEntry = gSetup.MaxTankVolume;
    }

    return volumeEntry;
}


// **********************************************************************************************************
// resetForVertical - Reset the required DVARS for the Horizontal and Vertical Tanks
// **********************************************************************************************************
static void resetForVerticalHorizontal(void)
{
    (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, MaxTankVolume), (DistVarType)MAX_TANK_DEF);
    (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, TankFillVolume), 0);
    (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, TankSensorVolume), 0);
}


// **********************************************************************************************************
// resetForCustom - Reset the required DVARS for the Custom Tank
// **********************************************************************************************************
static void resetForCustom(void)
{
    (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankFillHeight ), 0 );
    (void)DVAR_SetPointLocal_wRetry( DVA17G721_SS( gSetup, TankSetupComplete ), (DistVarType)FALSE );
}

