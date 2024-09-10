// screensTask.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The ScreensTask is used to manage the screens for the system

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"                            // Compiler specific type definitions
#include "rtos.h"                               // RTOS constants and prototypes
#include "debug.h"                              // Debug constants and prototypes
#include "screensTask.h"
#include "graphics_interface.h"
#include "screen_bitmaps.h"
#include "button_interface.h"
#include "gdisp.h"
#include "fonts.h"
#include "ConfigScreen.h"
#include "assert_app.h"
#include "FlowScreen.h"
#include "TimeScreen.h"
#include "CyclesScreen.h"
#include "NetworkScreen.h"
#include "RunScreen.h"
#include "dvseg_17G721_run.h"
#include "pumpControlTask.h"
#include "dvinterface_17G721.h"
#include "rtcTask.h"
#include "PinScreen.h"
#include "PinEntryScreen.h"
#include "alarms.h"
#include "AlarmScreen.h"
#include "AlarmScreenBattery.h"
#include "AlarmScreenControl.h"
#include "AlarmScreenPressure.h"
#include "AlarmScreenTank.h"
#include "AlarmScreenMotorCurrent.h"
#include "AdvancedScreen.h"
#include "ActivationScreen.h"
#include "SnoopScreen.h"
#include <stdio.h>
#include "io_pin.h"
#include "TankScreen.h"
#include "TankScreenHorizontal.h"
#include "TankScreenCustom.h"
#include "TankScreenVertical.h"
#include "TankScreenDebug.h"
#include "FluidPressureScreen.h"
#include "AnalogInFlowScreen.h"
#include "utilities.h"
#include "AnalogOutFlowScreen.h"
#include "AlarmScreenTemp.h"

// ******************************************************************************************
// CONSTANTS AND MACROS
// ******************************************************************************************
#define SCREENS_TASK_KEYS_FREQ_HZ               (10)
#define SCREENS_TASK_LOOP_PERIOD_ms             (1000/SCREENS_TASK_KEYS_FREQ_HZ)

#define SCREENS_KEY_TIMER_EVENT_FLAG            (RTOS_EVENT_FLAG_1)
#define SCREENS_FORCE_REFRESH_EVENT             (RTOS_EVENT_FLAG_2)
#define START_IMMEDIATELY                       (0x0001)
#define ALWAYS_WAIT_FOR_MATCH                   (0x0000)

// Definition of Pull down box character in arial14 font
#define PULL_DOWN_BOX_CHARACTER             0xFF

#define SCREESAVER_TIME                     (30u)
#define GRACO_LOGO_X_POS                    ((LCD_X_MAX-72)/2)
#define GRACO_LOGO_Y_POS                    (3)

#define LCD_HEATER_ON_TEMPERATURE_C         (-18)       // 0F
#define LCD_HEATER_OFF_TEMPERATURE_C        (-12)       // 10F
#define LCD_HEATER_TIME_ON                  (900)        // seconds

// ******************************************************************************************
// STATIC VARIABLES
// ******************************************************************************************
static INT8U gScreensTimerID;
static INT8U gScreensRefreshTimerID;
static INT8U gScreensTaskID;
static INT8U gBacklightTimerID;
static BOOLEAN gIsBacklightEnabled = FALSE;
static BOOLEAN gLastHeaterState = FALSE;
static INT8U gDisplayHeaterTimerID;

// Variable to keep information on the current select box
static INT8U SelectBoxPositionX;
static INT8U SelectBoxPositionY;
static INT8U SelectBoxWidth;
static INT8U SelectBoxHeight;

static screen_t gScreenHandler[NUMBER_OF_INPUT_MODES] =
{
    NULL,                   //INPUT_MODE_STARTUP
    ConfigScreen,           //INPUT_MODE_CONFIG
    FlowScreen,             //INPUT_MODE_VOLUME
    TimeScreen,             //INPUT_MODE_TIME
    CyclesScreen,           //INPUT_MODE_CYCLES
    AlarmScreen,            //INPUT_MODE_ALARMS
    AlarmScreenControl,     //INPUT_MODE_ALARMS_CONTROL
    AlarmScreenBattery,     //INPUT_MODE_ALARMS_BATT
    AlarmScreenPressure,    //INPUT_MODE_ALARMS_PRESS
    AlarmScreenTank,        //INPUT_MODE_ALARMS_TANK
    AlarmScreenTemp,        //INPUT_MODE_ALARMS_TEMP
    AlarmScreenMotorCurrent,//INPUT_MODE_ALARMS_MOTOR_CURRENT
    PinScreen,              //INPUT_MODE_PIN_CODE
    NetworkScreen,          //INPUT_MODE_NETWORK
    RunScreen,              //INPUT_MODE_RUN
    PinEntryScreen,         //INPUT_MODE_PIN_ENTRY
    AdvancedScreen,         //INPUT_MODE_CALIBRATION
    ActivationScreen,       //INPUT_MODE_ACTIVATION
    SnoopScreen,            //INPUT_MODE_SNOOP
	FluidPressureScreen,    //INPUT_MODE_FLUID_PRESS
    TankScreen,             //INPUT_MODE_TANK
    TankScreenVertical,     //INPUT_MODE_VERTICAL
    TankScreenHorizontal,   //INPUT_MODE_HORIZONTAL
    TankScreenCustom,       //INPUT_MODE_CUSTOM
    TankScreenDebug,        //INPUT_MODE_TANK_DEBUG
    AnalogInFlowScreen,     //INPUT_MODE_AIN_FLOW
    AnalogOutFlowScreen,    //INPUT_MODE_AOUT_FLOW
};

static INPUT_MODE_t gScreenID = INPUT_MODE_CONFIG;

// ******************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// ******************************************************************************************
static void setupViewports(void);
static void handleScreenEvent(INPUT_EVENT_t event);
static void screenTransitionActivity(INPUT_MODE_t oldScreen, INPUT_MODE_t newScreen);
static void enableBacklight(void);
static void disableBacklight(INT8U id);
static void handleKeyPress(KeyCode_t key);
static void showSplashScreen(void);
static void checkDisplayHeater(void);
static void disableDisplayHeater(INT8U id);

// ******************************************************************************************
// PUBLIC FUNCTIONS
// ******************************************************************************************

INT8U screensTaskInit (void)
{
    INT8U status = DEVELOPMENT_TASK_INIT_OK;
    
    // Reserve a task timer
    gScreensTimerID = RtosTimerReserveID();
    if (gScreensTimerID == RTOS_INVALID_ID)
    {
        status = DEVELOPMENT_TASK_INIT_ERROR;
    }

    // Reserve a task timer
    gScreensRefreshTimerID = RtosTimerReserveID();
    if (gScreensRefreshTimerID == RTOS_INVALID_ID)
    {
        status = DEVELOPMENT_TASK_INIT_ERROR;
    }
    
    // Reserve a task ID and queue the task
    gScreensTaskID = RtosTaskCreateStart(ScreensTask);
    if (gScreensTaskID == RTOS_INVALID_ID)
    {
        status = DEVELOPMENT_TASK_INIT_ERROR;
    }
    
    if (status == DEVELOPMENT_TASK_INIT_ERROR)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "DEVELOPMENT TASK INIT ERROR\r\n");
    }

    setupViewports();
    
    // Set up the screen-temperature sensor
    TEMPERATURE_Init(LCD_HEATER_ON_TEMPERATURE_C, LCD_HEATER_OFF_TEMPERATURE_C);    
    
    return status;
}

// ******************************************************************************************
// PRIVATE FUNCTIONS
// ******************************************************************************************

void ScreensTask (void)
{
    INT8U status;
    buttonInfo_t buttonInfo;
    INT32U lastAlarmBitfield = 0;
    static KeyCode_t lastButtonPressed = KeyCode_None;
    static INT8U refreshCounter = 0;
    
    // Wait for DVARs to be ready
    (void)K_Resource_Wait(gDvarLockoutResID, 0);
    (void)K_Resource_Release(gDvarLockoutResID);

    // Create and start the timer
    status = K_Timer_Create(gScreensTimerID,RTOS_NOTIFY_SPECIFIC,gScreensTaskID,SCREENS_KEY_TIMER_EVENT_FLAG);
    status |= K_Timer_Start(gScreensTimerID,START_IMMEDIATELY,(RtosGetTickFreq()/SCREENS_TASK_LOOP_PERIOD_ms));
    
    if (status != K_OK)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "SCREENS TASK TIMER ERROR\r\n");
    }

    gBacklightTimerID = RTC_createTimer(SCREESAVER_TIME, disableBacklight);
    enableBacklight();
    showSplashScreen();
    handleScreenEvent(INPUT_EVENT_ENTRY_INIT);
    
    gDisplayHeaterTimerID = RTC_createTimer(LCD_HEATER_TIME_ON, disableDisplayHeater);

    // Enter the cyclic portion of the task.
    for (;;)
    {
        fillLocalButtonInfo(&buttonInfo);

        if (buttonInfo.buttonPressed != lastButtonPressed)
        {
            lastButtonPressed = buttonInfo.buttonPressed;
            
            // Act on a button press only if the backlight is enabled
            if (gIsBacklightEnabled)
            {
                handleKeyPress(lastButtonPressed);
            }
            
            enableBacklight();
            checkDisplayHeater();
        }

        // Refresh run screen periodically
        else if ( (gScreenID == INPUT_MODE_RUN) && ((refreshCounter % 150) == 0) )
        {
            const INT32U progress = PMP_getCycleProgress();
            static INT32U pumpStatus;
            static INT32U tankLevel;
            static INT32 temperature;

            if (progress != gRun.CycleProgress)
            {
               (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, CycleProgress), (DistVarType)progress);
               handleScreenEvent(INPUT_EVENT_REFRESH_SCREEN);
            }

            // Refresh run screen if pump status changes
            if (pumpStatus != gRun.PumpStatus)
            {
                pumpStatus = gRun.PumpStatus;
                handleScreenEvent(INPUT_EVENT_REFRESH_SCREEN);
            }
            
            // Refresh run screen if tank level changes
            if (tankLevel != gRun.TankLevel)
            {
                tankLevel = gRun.TankLevel;
                handleScreenEvent(INPUT_EVENT_REFRESH_SCREEN);
            }
            
            if (temperature != (sint32)gRun.Temperature)
            {
                temperature = (sint32)gRun.Temperature;
                handleScreenEvent(INPUT_EVENT_REFRESH_SCREEN);
            }

            if ( (buttonInfo.buttonPressed == KeyCode_Enter) && (buttonInfo.timePressed > 2000) )
            {
                (void)ALARM_CancelAll();
                handleScreenEvent(INPUT_EVENT_PRESS_HOLD_ENTER);
                handleScreenEvent(INPUT_EVENT_REFRESH_SCREEN);
            }
        }

        else if ( gRun.AlarmBitfield != lastAlarmBitfield)
        {
            handleScreenEvent(INPUT_EVENT_REFRESH_SCREEN);
        }

        // If we need a forced refresh, only do so if we haven't refreshed for some other reason
        // Do a bitwise-AND instead of an equality test because the timer event could
        // occur at the same time as the refresh event
        else if (status & SCREENS_FORCE_REFRESH_EVENT)
        {
            handleScreenEvent(INPUT_EVENT_REFRESH_SCREEN);
        }

        lastAlarmBitfield = gRun.AlarmBitfield;
        refreshCounter++;
        status = K_Event_Wait(SCREENS_KEY_TIMER_EVENT_FLAG | SCREENS_FORCE_REFRESH_EVENT,
            ALWAYS_WAIT_FOR_MATCH,
            RTOS_CLEAR_EVENT_FLAGS_AFTER);
    }
}

INPUT_MODE_t getCurrentScreen(void)
{
    return gScreenID;
}

static void handleKeyPress(KeyCode_t key)
{
    switch (key)
    {
        case KeyCode_None:
            break;

        case KeyCode_UpArrow:
            handleScreenEvent(INPUT_EVENT_UP_ARROW);
            break;

        case KeyCode_DownArrow:
            handleScreenEvent(INPUT_EVENT_DOWN_ARROW);
            break;

        case KeyCode_Enter:
            handleScreenEvent(INPUT_EVENT_ENTER);
            break;

        case KeyCode_Cancel:
            handleScreenEvent(INPUT_EVENT_RESET);
            if (gScreenID != INPUT_MODE_ALARMS)
            {
                (void)ALARM_CancelAll();
            }
            break;

        case KeyCode_Left:
            handleScreenEvent(INPUT_EVENT_LEFT_ARROW);
            break;

        case KeyCode_Right:
            handleScreenEvent(INPUT_EVENT_RIGHT_ARROW);
            break;

        case KeyCode_Left_Right:
            handleScreenEvent(INPUT_EVENT_BOTH_ARROWS);
            DEBUG_PRINT_STRING(DBUG_ALWAYS, "Both Arrows Pressed\r\n");
            break;

        case KeyCode_Num_Inputs_Plus_One:
            assert_always();
            break;
    }
}

void GotoScreen(INPUT_MODE_t screenID)
{
    INPUT_MODE_t oldScreenId = gScreenID;
    
    if (screenID != gScreenID)
    {
        gScreenID = screenID;
        screenTransitionActivity(oldScreenId, screenID);
        assert(gScreenHandler[gScreenID] != NULL);
        (void)(gScreenHandler[gScreenID](INPUT_EVENT_ENTRY_INIT));
    }
}

//------------------------------------------------------------------------------
//  FUNCTION:       DrawSelectBox ()
//
//  PARAMETERS:     INT8U PositionX
//                  INT8U PositionY
//                  INT8U Width
//                  INT8U Height
//
//  DESCRIPTION:    Sets up viewport and outlines a select box
//
//  RETURNS:        None
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------
void DrawSelectBox (INT8U PositionX,
                    INT8U PositionY,
                    INT8U Width,
                    INT8U Height)
{
    // Remember the information
    SelectBoxPositionX = PositionX;
    SelectBoxPositionY = PositionY;
    SelectBoxWidth = Width;
    SelectBoxHeight = Height;

    sint8 offsetX = SelectBoxPositionX > 0? -4:0;
    sint8 offsetY = SelectBoxPositionY > 0? -4:0;
    
    gfillvp ((ggetfw () * PositionX) + offsetX,                 // Upper left X
             (ggetfh () * PositionY) + offsetY,                 // Upper left Y
             (ggetfw () * (PositionX + Width)) + offsetX + 8,   // Lower Right X
             (ggetfh () * (PositionY + Height)) + offsetY + 8,  // Lower Right Y
             0x0000);    

    // Put the cursor at the top and end of this
    gsetcpos (PositionX + Width,
              PositionY);

    // Output the pulldown box character
    ShowPullDownCharacter();
}

//------------------------------------------------------------------------------
//  FUNCTION:       PopulateSelectBox ()
//
//  PARAMETERS:     INT8U Index
//                  char SelectText
//
//  DESCRIPTION:    Populates selected entry with text in SelectText
//
//  RETURNS:        None
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------
void PopulateSelectBox (INT8U Index,
                        char *SelectText)
{
    // Position the cursor
    gsetcpos (SelectBoxPositionX,
              SelectBoxPositionY + Index);

    gputs (SelectText);
}

//------------------------------------------------------------------------------
//  FUNCTION:       SelectSelectBox ()
//
//  PARAMETERS:     INT8U Index
//
//  DESCRIPTION:    Draws a box around selected box entry
//
//  RETURNS:        None
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------
void SelectSelectBox (INT8U Index)
{
    sint8 offsetX = SelectBoxPositionX > 0? -4:0;
    sint8 offsetY = SelectBoxPositionY > 0? -4:0;
    
    // Draw a rectangle just inside the box
    grectangle ((ggetfw () * SelectBoxPositionX) + offsetX,                         // Upper left X
                (ggetfh () * SelectBoxPositionY) + offsetY,                         // Upper left Y
                (ggetfw () * (SelectBoxPositionX + SelectBoxWidth)) + 0,            // Lower Right X
                (ggetfh () * (SelectBoxPositionY + SelectBoxHeight)) + offsetY + 6);

    offsetX = SelectBoxPositionX > 0? -2:1;
    offsetY = SelectBoxPositionY > 0? -2:1;
    
    // Draw a rectangle spaced out just a bit
    grectangle ((ggetfw () * SelectBoxPositionX) + offsetX,                         // Upper left X
                (ggetfh () * SelectBoxPositionY) + offsetY,                         // Upper left Y
                (ggetfw () * (SelectBoxPositionX + SelectBoxWidth)) + offsetX + 2,  // Lower Right X
                (ggetfh () * (SelectBoxPositionY + SelectBoxHeight)) + offsetY + 3);    


    // Draw a rectangle around the selected item    
    grectangle ((ggetfw () * SelectBoxPositionX) + offsetX,
                (ggetfh () * (SelectBoxPositionY + Index)) + offsetY,
                (ggetfw () * (SelectBoxPositionX + SelectBoxWidth)) + offsetX,
                (ggetfh () * (SelectBoxPositionY + Index + 1)) + (offsetY + 1));    
}

void ShowPullDownCharacter(void)
{
    (void)gselfont((PGFONT)&legacy);
    gputch(PULL_DOWN_BOX_CHARACTER);
    (void)gselfont((PGFONT)&Xtreme);
}

void ClearScreen (void)
{
    (void)clearArea( LCD_X_MIN, LCD_Y_MIN, LCD_X_MAX, LCD_Y_MAX );
}

void RefreshScreen(void)
{
    // Always do screen updates in the Screens task regardless of the caller, since the
    // graphics driver appears not to be threadsafe.
    (void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, gScreensTaskID, SCREENS_FORCE_REFRESH_EVENT);
}

static void setupViewports(void)
{
    gsetupvp (VIEWPORT_MAIN,        // Viewport Number 
                  0,                // Upper left X
                  0,                // Upper left Y
                  GDISPW - 1,       // Lower Right X
                  GDISPH -1,        // Lower Right Y
                  &Xtreme,
                  NULL,
                  GNORMAL);         //lint !e534 
}

static void handleScreenEvent(INPUT_EVENT_t event)
{
    INPUT_MODE_t screenID;
    
    assert(gScreenHandler[gScreenID] != NULL);
    screenID = gScreenHandler[gScreenID](event);

    // Check to see if screen has changed and sen INPUT_EVENT_ENTRY_INIT
    // if necessary
    if (screenID != gScreenID)
    {
        screenTransitionActivity(gScreenID, screenID);
        gScreenID = screenID;
        assert(gScreenHandler[gScreenID] != NULL);
        (void)(gScreenHandler[gScreenID](INPUT_EVENT_ENTRY_INIT));
    }
}

static void screenTransitionActivity(INPUT_MODE_t oldScreen, INPUT_MODE_t newScreen)
{
    if( (oldScreen == INPUT_MODE_RUN) && ((newScreen == INPUT_MODE_PIN_ENTRY) || (newScreen == INPUT_MODE_CONFIG)) )
    {
        PMP_setStandbyMode();
    }

    if( (newScreen == INPUT_MODE_RUN) && ((oldScreen == INPUT_MODE_CONFIG) || (oldScreen == INPUT_MODE_PIN_ENTRY)) )
    {
        PMP_setRunMode();
    }
}

static void enableBacklight(void)
{
    gIsBacklightEnabled = TRUE;
    setBacklightIntensity(BACKLIGHT_100);
    (void)RTC_resetTimer(gBacklightTimerID, SCREESAVER_TIME);
    (void)RTC_startTimer(gBacklightTimerID);
}

static void disableBacklight(INT8U id)
{
    if (getCurrentScreen() == INPUT_MODE_SNOOP)
    {
        enableBacklight();
    }
    else
    {
        gIsBacklightEnabled = FALSE;
        setBacklightOnOff(BACKLIGHT_OFF);
        (void)RTC_stopTimer(gBacklightTimerID);
    }
}

static void showSplashScreen(void)
{
    char versionStr[] ="XX.XX.XX";

    (void)clearArea( LCD_X_MIN, LCD_Y_MIN, LCD_X_MAX, LCD_Y_MAX );
    (void)placeBitmap( GRACO_LOGO_X_POS, GRACO_LOGO_Y_POS, (void*)BMP_Screen_GracoLogo_74x80 );
    sprintf(versionStr, "%2d.%2d.%2d", VER17G721_MAJOR, VER17G721_MINOR, VER17G721_BUILD);
    gsetcpos(8, 8);
    gputs(versionStr);
    delay(5000);
}

static void checkDisplayHeater(void)
{
    bool currentHeaterState;
    
    // Check the temperature of the screen and turn on the heater if necessary
    // Only check if a button has been pressed and only run heater for 15 min
    
    currentHeaterState = TEMPERATURE_MonitorHeat();
    if (currentHeaterState != gLastHeaterState)
    {
        if (currentHeaterState == TRUE)
        {
            (void)RTC_resetTimer(gDisplayHeaterTimerID, LCD_HEATER_TIME_ON);
            (void)RTC_startTimer(gDisplayHeaterTimerID);
        }
        else
        {
            (void)RTC_stopTimer(gDisplayHeaterTimerID);
        }
        (void)printf("\nHeater state now: %u\n", currentHeaterState);
    }
    gLastHeaterState = currentHeaterState;        
}

static void disableDisplayHeater(INT8U id)
{
    (void)OUT_Digital_Latch_Set(IOPIN_HEAT_EN, NOT_ASSERTED);
    (void)RTC_stopTimer(gDisplayHeaterTimerID);    
    gLastHeaterState = FALSE;
    (void)printf("\nHeater state now: %u\n", 0u);
}

