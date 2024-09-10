// AlarmScreenMotorCurrent.c

#include "stdio.h"


// Copyright 2018
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the alarm battery screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************
#include "dvseg_17G721_setup.h"
#include "PublishSubscribe.h"
#include "CountDigit.h"
#include "screensTask.h"
#include "AlarmScreenMotorCurrent.h"
#include "AdvancedScreen.h"
#include "units_pressure.h"
#include "screenStuff.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define NUM_BATTERY_DIGITS          4u
#define CONFIG_TRIGGER_TO_DECIVOLTS (100) // config screen stores in tenths of a volt (decivolts?)

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    FOCUS_MTR_PROTECTION_ENABLED = 0,  
    FOCUS_MTR_VOLTAGE,
    FOCUS_MTR_PN,
    NUMBER_ALARM_MTR_PROTECTION_CURRENT_ITEMS
} ALARM_MTR_PROTECTION_CURRENT_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

static SELECTION_BOX_t* gMtrProtectionEnabledSelectionBox;
static SELECTION_BOX_t* gMtrVoltageSelectionBox;
static SELECTION_BOX_t* gMtrPNSelectionBox;


static char* ppMtrProtectionEnabledSelectionBox[] =
{
    "OFF",
    "ON",
};

static char* ppMtrVoltageSelectionBox[] =
{
    "12 VDC",
    "24 VDC",
    "115 VAC",
    "230 VAC",
};

static char* pp12VMtrSelectionSelectionBox[] =
{
    "B32002",
    "B32023",
    "B32032",
    "B32109",
    "C1D1",
};

static char* pp24VMtrSelectionSelectionBox[] =
{
    "B32236",
    "C1D1",
};

static char* pp120VACMtrSelectionSelectionBox[] =
{
    "B32146",
    "B32705",
    "B32761",
    "C1D1",
};

static char* pp230VACMtrSelectionSelectionBox[] =
{
    "B32147",
    "C1D1",
};

static const uint32 MAX_PUMP_CURRENT_MV[NUMBER_OF_MTR_VOLTAGE_CHOICES][NUMBER_OF_MTR_PMP_SELECTION_CHOICES] = 
{   //30A is 500mV, 0A is 2500mV, and -30A is 4500mV equation is mV = (-4000/60)*Current+2500 AC motors need peak value, not RMS
    {1590, 1760, 1230, 1430, 960}, //12 VDC  B32002(13.6A) B32023(11A) B32032(19A)     B32109(16A)     C1D1(23A)
    {1760, 1700, 2500, 2500, 2500}, //24 VDC  B32236(11A)   C1D1(12A)   NA              NA              NA
    {2290, 2190, 2260, 2035, 2500}, //115 VAC B32146(2A)    B32705(3A)  B32761(2.25A)   C1D1(4.8A)      NA
    {2365, 2255, 2500, 2500, 2500}, //230 VAC B32147(1.2A)  C1D1(2.3A)  NA              NA              NA  
};



static INPUT_MODE_t ReturnMode;
static ALARM_MTR_PROTECTION_CURRENT_FOCUS_t alarmMotorCurrentFocusIndex = FOCUS_MTR_PROTECTION_ENABLED ;
MTR_PROTECTION_t mtrProtectionEnabled = FALSE;
MTR_VOLTAGE_t mtrVoltage = MTR_VOLTAGE_12VDC;
MTR_SELECTION_t mtrSelection = 0;
static bool* isFocusArray[NUMBER_ALARM_MTR_PROTECTION_CURRENT_ITEMS];
static char** mtrTextSelection[NUMBER_OF_MTR_VOLTAGE_CHOICES] = {pp12VMtrSelectionSelectionBox, pp24VMtrSelectionSelectionBox, pp120VACMtrSelectionSelectionBox, pp230VACMtrSelectionSelectionBox}; 
uint8 numberOfMtrSelections[NUMBER_OF_MTR_VOLTAGE_CHOICES] = {NUMBER_OF_12VDC_MOTORS, NUMBER_OF_24VDC_MOTORS, NUMBER_OF_120VAC_MOTORS, NUMBER_OF_230VAC_MOTORS};
static bool settingChanged = FALSE;

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************
static void drawAlarmScreenMotorCurrent(ALARM_MTR_PROTECTION_CURRENT_FOCUS_t index);
static ALARM_MTR_PROTECTION_CURRENT_FOCUS_t incrementAlarmFocusIndex(ALARM_MTR_PROTECTION_CURRENT_FOCUS_t focusIndex);
static ALARM_MTR_PROTECTION_CURRENT_FOCUS_t decrementAlarmFocusIndex(ALARM_MTR_PROTECTION_CURRENT_FOCUS_t focusIndex);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// AlarmScreenBattery - The main handler for the alarm battery screen display
// **********************************************************************************************************
INPUT_MODE_t AlarmScreenMotorCurrent(INPUT_EVENT_t InputEvent)
{
    ReturnMode = INPUT_MODE_ALARMS_MOTOR_CURRENT;

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

    drawAlarmScreenMotorCurrent(alarmMotorCurrentFocusIndex);

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// incrementAlarmFocusIndex - Move focus to the next field
// **********************************************************************************************************
static ALARM_MTR_PROTECTION_CURRENT_FOCUS_t incrementAlarmFocusIndex(ALARM_MTR_PROTECTION_CURRENT_FOCUS_t focusIndex)
{    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    // When motor protection is turned off, do not give any other options
    if (gSetup.MtrProtectionEnabled == MTR_PROTECTION_ON)
    {
        if( focusIndex < (NUMBER_ALARM_MTR_PROTECTION_CURRENT_ITEMS - 1) )
        {
            focusIndex = focusIndex + 1;
        }
        else
        {
            focusIndex = 0;
        }
    }
    else
    {
        focusIndex = FOCUS_MTR_PROTECTION_ENABLED;
    }
    
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// decrementAlarmFocusIndex - Move focus to the previous field
// **********************************************************************************************************
static ALARM_MTR_PROTECTION_CURRENT_FOCUS_t decrementAlarmFocusIndex(ALARM_MTR_PROTECTION_CURRENT_FOCUS_t focusIndex)
{    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    // When motor protection is turned off, do not give any other options
    if (gSetup.MtrProtectionEnabled == MTR_PROTECTION_ON)
    {
        if( focusIndex > 0 )
        {
            focusIndex = focusIndex - 1;
        }
        else
        {
            focusIndex = NUMBER_ALARM_MTR_PROTECTION_CURRENT_ITEMS - 1;
        }
    }
    else
    {
        focusIndex = FOCUS_MTR_PROTECTION_ENABLED;
    }
    
    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawAlarmScreenBattery - Draw the rest of the alarm battery screen
// **********************************************************************************************************
static void drawAlarmScreenMotorCurrent(ALARM_MTR_PROTECTION_CURRENT_FOCUS_t focusIndex)
{
    gsetcpos(0, 1);
    gputs("PROTECTION");    
    
    if(gSetup.MtrProtectionEnabled == TRUE) //only show these when Motor protection is enabled
    {
        gsetcpos(0, 2);
        gputs("VOLTAGE");

        gsetcpos(0, 3);
        gputs("MOTOR PN");
        
        (*gMtrVoltageSelectionBox).isHidden = FALSE;
        (*gMtrPNSelectionBox).isHidden = FALSE;
    }
    else
    {
        (*gMtrVoltageSelectionBox).isHidden = TRUE;
        (*gMtrPNSelectionBox).isHidden = TRUE;
    }
    
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
    ClearScreen();
    clearAllIsFocus();
    hideAllBoxes();
    clearAllIsEdit();
      
    settingChanged = FALSE;
    //load all the initial values that are needed
    mtrProtectionEnabled = gSetup.MtrProtectionEnabled;
    mtrVoltage = gSetup.MtrVoltage;
    mtrSelection = gSetup.MtrSelection;

    //load the shared select boxes
    gMtrProtectionEnabledSelectionBox = &selectBox1;
    (void) selectBoxConfigure(gMtrProtectionEnabledSelectionBox, 0, NUMBER_OF_MTR_PROTECTION_ITEMS, FALSE, FALSE, FALSE, FALSE, 13, 1, 7, ppMtrProtectionEnabledSelectionBox);
    (*gMtrProtectionEnabledSelectionBox).index = mtrProtectionEnabled;

    gMtrVoltageSelectionBox = &selectBox2;
    (void) selectBoxConfigure(gMtrVoltageSelectionBox, 0, NUMBER_OF_MTR_VOLTAGE_CHOICES, FALSE, FALSE, TRUE, FALSE, 13, 2, 7, ppMtrVoltageSelectionBox);
    (*gMtrVoltageSelectionBox).index = mtrVoltage;
    
    gMtrPNSelectionBox = &selectBox3;
    (void) selectBoxConfigure(gMtrPNSelectionBox, 0, numberOfMtrSelections[mtrVoltage], FALSE, FALSE, TRUE, FALSE, 13, 3, 7, mtrTextSelection[mtrVoltage]);
    (*gMtrPNSelectionBox).index = mtrSelection;    

    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    alarmMotorCurrentFocusIndex = FOCUS_MTR_PROTECTION_ENABLED;
    *isFocusArray[alarmMotorCurrentFocusIndex] = TRUE;   
}

//****************************************************************************//
//Fcn: processInputResetEvent
//
//Desc: This function processes the reset button events
//****************************************************************************//
static void processInputResetEvent(void)
{
    uint32 maxMotorCurrentmV = 0;
    if( (anyDigitBoxIsEdit() == FALSE) && (anySelectBoxIsEdit() == FALSE))
    { 
        maxMotorCurrentmV = MAX_PUMP_CURRENT_MV[mtrVoltage][mtrSelection];
        
        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MaxMtrCurrentmV), (DistVarType)maxMotorCurrentmV);
        
        ReturnMode = INPUT_MODE_ALARMS;
        hideAllBoxes();
        clearAllIsFocus();
        if(settingChanged)
        {
            uint32 dataToPublish = 0;
            dataToPublish = PackageMotorProtectionSettings();
            
            PublishUint32(TOPIC_MotorProtectionSettings, dataToPublish);
        }
    }
    
    (*gMtrProtectionEnabledSelectionBox).index = mtrProtectionEnabled;
    (*gMtrVoltageSelectionBox).index = mtrVoltage;
    (*gMtrPNSelectionBox).index = mtrSelection;

    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( alarmMotorCurrentFocusIndex )
    {
        case FOCUS_MTR_PROTECTION_ENABLED:
            if( (*gMtrProtectionEnabledSelectionBox).isEditMode == TRUE )
            {
                mtrProtectionEnabled = (*gMtrProtectionEnabledSelectionBox).index;
                (*gMtrProtectionEnabledSelectionBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MtrProtectionEnabled), (DistVarType)mtrProtectionEnabled);
                settingChanged = TRUE;
            }
            else
            {
                (*gMtrProtectionEnabledSelectionBox).isEditMode = TRUE;
            }
            break;        
        
        case FOCUS_MTR_VOLTAGE:
            if( (*gMtrVoltageSelectionBox).isEditMode == TRUE )
            {
                mtrVoltage = (*gMtrVoltageSelectionBox).index;
                (*gMtrVoltageSelectionBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MtrVoltage), (DistVarType)mtrVoltage);
                mtrSelection = 0;
                (*gMtrPNSelectionBox).index = mtrSelection;  //reset the index so that we don't reference a null pointer
                (*gMtrPNSelectionBox).numberItems = numberOfMtrSelections[mtrVoltage];  //reset the number of items so that we don't display more items than we should
                (*gMtrPNSelectionBox).selectBoxText = mtrTextSelection[mtrVoltage];   //update the list of text to display
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MtrSelection), (DistVarType)mtrSelection);    // update the DVAR to ensure no saved out of range pointers can occur between exiting and reentering the screen
                settingChanged = TRUE;
            }
            else
            {
                (*gMtrVoltageSelectionBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_MTR_PN:
            if( (*gMtrPNSelectionBox).isEditMode == TRUE )
            {
                mtrSelection = (*gMtrPNSelectionBox).index;
                (*gMtrPNSelectionBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MtrSelection), (DistVarType)mtrSelection);
                settingChanged = TRUE;
            }
            else
            {
                (*gMtrPNSelectionBox).isEditMode = TRUE;
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
    if( upEventForSelectBox() == FALSE)
    {
       alarmMotorCurrentFocusIndex = decrementAlarmFocusIndex(alarmMotorCurrentFocusIndex);
    }   
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{
    if(downEventForSelectBox() == FALSE)
    {
        alarmMotorCurrentFocusIndex = incrementAlarmFocusIndex(alarmMotorCurrentFocusIndex);
    }
}

//****************************************************************************//
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void loadIsFocusArray(void)
{
    isFocusArray[FOCUS_MTR_PROTECTION_ENABLED] = &((*gMtrProtectionEnabledSelectionBox).isFocus);    
    isFocusArray[FOCUS_MTR_VOLTAGE] = &((*gMtrVoltageSelectionBox).isFocus);
    isFocusArray[FOCUS_MTR_PN] = &((*gMtrPNSelectionBox).isFocus);
}


//****************************************************************************//
//FCN: PackageMotorProtectionSettings
//
//Desc: Takes the motor protection settings and packages them together for 
// sending to the web server        
//****************************************************************************//
uint32 PackageMotorProtectionSettings(void)
{
    uint32 rVal = 0;
    rVal = (gSetup.MtrProtectionEnabled & 0x000000FF);
    rVal = rVal << 8;
    rVal += (gSetup.MtrVoltage & 0x000000FF);
    rVal = rVal << 8;
    rVal += (gSetup.MtrSelection & 0x000000FF);
    return rVal;    
}
