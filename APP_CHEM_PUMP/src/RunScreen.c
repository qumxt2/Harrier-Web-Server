// RunScreen.c

// Copyright 2015 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the run screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************

#include "pumpControlTask.h"
#include "dvseg_17G721_setup.h"
#include "dvseg_17G721_run.h"
#include "PublishSubscribe.h"
#include "screensTask.h"
#include "gdisp.h"
#include "RunScreen.h"
#include "CountDigit.h"
#include "SocketModem.h"
#include "graphics_interface.h"
#include "screen_bitmaps.h"
#include "assert_app.h"
#include <stdio.h>
#include "io_typedef.h"
#include "out_digital.h"
#include "TimeDigit.h"
#include "application.h"
#include "alarms.h"
#include "string.h"
#include "FlowScreen.h"
#include "AdvancedScreen.h"
#include "AlarmScreenBattery.h"
#include "screenStuff.h"
#include "utilities.h"
#include "systemTask.h"
#include "AlarmScreenTemp.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************
#define SCREEN_WIDTH    (32)

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawStaticText(void);
static void drawProgressBar();
static void drawModeIcon(void);
static void drawPumpIcon(void);
static void drawActiveAlarms(void);

// **********************************************************************************************************
// NetworkScreen - Main handler for the network screen
// **********************************************************************************************************

INPUT_MODE_t RunScreen(INPUT_EVENT_t InputEvent)
{
    // Unless something changes it return select mode
    INPUT_MODE_t ReturnMode = INPUT_MODE_RUN;
    // Process based on input event
    switch( InputEvent )
    {
        case INPUT_EVENT_ENTRY_INIT:
            //reset all the boxes to their default states
            clearAllIsFocus();
            hideAllBoxes();
            clearAllIsEdit();
            break;

        case INPUT_EVENT_RESET:
            break;

        case INPUT_EVENT_ENTER:
            break;

        case INPUT_EVENT_UP_ARROW:
            break;

        case INPUT_EVENT_DOWN_ARROW:
            break;

        case INPUT_EVENT_PRESS_HOLD_ENTER:
            ReturnMode = INPUT_MODE_PIN_ENTRY;
            break;

        case INPUT_EVENT_BOTH_ARROWS:
            PMP_primePump();
            break;
			
        default:
            break;
    }

    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawModeIcon();
    drawPumpIcon();
    drawStaticText();

    // Return the mode
    return (ReturnMode);
}

// **********************************************************************************************************
// drawActiveAlarms - Show the names of the active alarms, if any
// **********************************************************************************************************

static void drawActiveAlarms(void)
{
    const char alarmsActiveTxt[] = "ALARM: ";
    const char alarmsMoreTxt[] = "%u are active";
    const char ellipsisTxt[] = "...";
    const INT8U rowOffset = 8;
    const INT8U maxWidth = 16;
    char strBuf[SCREEN_WIDTH+1] = {0};
    INT8U colOffset = 1;
    INT8U idx = 0;
    INT8U activeCount = 0;
    INT8U firstActive = 0;

    if (!gRun.AlarmBitfield)
    {
        return;
    }

    gsetcpos(colOffset, rowOffset);
    gputs(alarmsActiveTxt);
    colOffset += strlen(alarmsActiveTxt);

    for (idx = ALARM_ID_NUM_ALARMS - 1; idx > ALARM_ID_Unknown; idx--)
    {
        if (gRun.AlarmBitfield & (1 << (idx-1)))
        {
            activeCount++;
            firstActive = idx;
        }
    }

    // We can only fit one alarm name on the screen
    if (activeCount == 1)
    {
        gsetcpos(colOffset, rowOffset);

        // If the alarm name won't fit, truncate it and show an elipsis
        if (strlen(ALARM_alarmNames[firstActive]) > maxWidth ||
            strlen(ALARM_alarmNames[firstActive]) >= (sizeof(strBuf) - 1))
        {
            strncpy(strBuf, ALARM_alarmNames[firstActive], maxWidth-2);
            strcat(strBuf, ellipsisTxt);
        }
        else
        {
            strcpy(strBuf, ALARM_alarmNames[firstActive]);
        }

        gputs(strBuf);
    }
    else
    {
        // With multiple alarms, show a count of the number active
        gsetcpos(colOffset, rowOffset);
        sprintf(strBuf, alarmsMoreTxt, activeCount);
        gputs(strBuf);
    }
}

// **********************************************************************************************************
// drawStaticText - Draw the rest of the screen
// **********************************************************************************************************

static void drawStaticText(void)
{
    char buf[25];
    static TIME_DIGIT_BOX_t OnTimeDigit;
    static TIME_DIGIT_BOX_t OffTimeDigit;
    void* pLightningIcon = (void*)&BMP_Lightning_6x12[0];
    void* pPressureIcon = (void*)&BMP_Pressure_12x12[0];
    void* pTankIcon = (void*)&BMP_Tank_10x17[0];
    void* pThermometerIcon = (void*)&BMP_Thermometer_9x14[0];
    INT32U flowRate = 0;
    uint32 tankLevel = getLocalVolume(gRun.TankLevel);
    uint32 maxTankVolume = getLocalVolume(gSetup.MaxTankVolume);
    const INT32U pressure = getLocalPressure(gRun.Pressure_1_Psi);
    INT32 temperature = getLocalTemperature(gRun.Temperature);
    
    if(gSetup.AnalogInControl == AIN_FLOW_RATE)
    {
        flowRate = getLocalFlowRate(gRun.AnalogFlowRate);
    }
    else
    {
        flowRate = getLocalFlowRate(gSetup.DesiredFlowRate);
    }
    
    if (POWER_SAVE_OFF != gSetup.PowerSaveMode)
    {
        // Display battery voltage, with a hack for alignment
        (void)placeBitmap(0,  0, pLightningIcon);
        sprintf(buf, "%lu.%lu", gRun.BatteryMillivolts / MV_PER_VOLT, (gRun.BatteryMillivolts % MV_PER_VOLT) / 100);
        gsetcpos(3, 0);
        gputs(buf);

        // Draw the units separately so that we can align them with the other units
        gsetcpos(8, 0);
        sprintf(buf, "V");
        gputs(buf);
    }

    // Display pressure
    (void)placeBitmap(0,  gfgetfh(pPressureIcon), pPressureIcon);
    gsetcpos(3, 1);
    
    if (gSetup.Units == UNITS_METRIC)
    {
        sprintf(buf, "%lu.%lu", pressure / 10, pressure % 10);
        gputs(buf);
        
        // Draw the units separately so that we can align them with the other units
        gsetcpos(8, 1);
        sprintf(buf, "BAR");
        gputs(buf);
    }
    else
    {
        sprintf(buf, "%-4lu", pressure);
        gputs(buf);
        
        // Draw the units separately so that we can align them with the other units
        gsetcpos(8, 1);
        sprintf(buf, "PSI");
        gputs(buf);
    }    

    // Display tank
   (void)placeBitmap(0,  gfgetfh(pLightningIcon) + gfgetfh(pPressureIcon) + 1, pTankIcon);    
    gsetcpos(3, 2);

    if (gSetup.Units == UNITS_METRIC)
    {
        if (tankLevel > 999)
        {
            // Don't display max tank volume for tank levels > 3 digits because it doesn't fit well
            sprintf(buf, "%d.%d L", tankLevel/10, tankLevel%10);
        }
        else
        {
            sprintf(buf, "%d.%d of %d.%d L", tankLevel/10, tankLevel%10, maxTankVolume/10, maxTankVolume%10);
        }
    }
    else
    {
        if (tankLevel > 999)
        {
            // Don't display max tank volume for tank levels > 3 digits because it doesn't fit well
            sprintf(buf, "%d.%d GAL", tankLevel/10, tankLevel%10);
        }
        else
        {
            sprintf(buf, "%d.%d of %d.%d GAL", tankLevel/10, tankLevel%10, maxTankVolume/10, maxTankVolume%10);
        }
    }
    gputs(buf);
    
    // Display temperature if it isn't disabled
    if(gSetup.TempControl != TEMP_CONTROL_DISABLED)
    {
        (void)placeBitmap(0,  gfgetfh(pLightningIcon) + gfgetfh(pPressureIcon) + gfgetfh(pTankIcon) + 1, pThermometerIcon);
        gsetcpos(3, 3);
        
        if (gRun.Temperature != TEMPERATURE_INVALID_DEG_F)
        {
            if (gSetup.Units == UNITS_METRIC)
            {
                sprintf(buf, "%d ", temperature);
                gputs(buf);
                gputch(DEGREE_SYMBOL);
                gputs("C");
            }
            else
            {
                sprintf(buf, "%d ", temperature);
                gputs(buf);
                gputch(DEGREE_SYMBOL);
                gputs("F");
            }
            
        }
        else
        {
            gputs("Disconnected");
        }
    }

    switch (gSetup.MeteringMode)
    {
        case METERING_MODE_VOLUME:
            gsetcpos(0, 5);
            if (gSetup.Units == UNITS_METRIC)
            {
                sprintf(buf, "%lu.%lu LPD" , flowRate / 10, flowRate % 10);
            }
            else
            {
                sprintf(buf, "%lu.%02u GPD" , flowRate / 100, flowRate % 100);
            }
            gputs(buf);
            break;

        case METERING_MODE_TIME:
            (void)LoadTimeDigit(&OnTimeDigit.timeDigit, gSetup.OnTime, 6, 4, 4);
            (void)LoadTimeDigit(&OffTimeDigit.timeDigit, gSetup.OffTime, 6, 4, 5);
            gsetcpos(0, 4);
            gputs("On:");
            DisplayTimeDigit(&OnTimeDigit.timeDigit);
            gsetcpos(0, 5);
            gputs("Off:");
            DisplayTimeDigit(&OffTimeDigit.timeDigit);
            break;

        case METERING_MODE_CYCLES:
            gsetcpos(0, 4);
            sprintf(buf, "On: %lu cycles" , gSetup.OnCycles);
            gputs(buf);
            (void)LoadTimeDigit(&OffTimeDigit.timeDigit, gSetup.OffTime, 6, 4, 5);
            gsetcpos(0, 5);
            gputs("Off:");
            DisplayTimeDigit(&OffTimeDigit.timeDigit);
            break;

        default:
            break;
    }

    // Display all alarms if active, including notify only ones
    drawActiveAlarms();

    switch (gRun.PumpStatus)
    {
        case PUMP_STATUS_Lockout_Remote:
            gsetcpos(1, 8);
            gputs("Disabled by Remote           ");
            break;
            
        case PUMP_STATUS_Lockout_Temperature:
            gsetcpos(1, 8);
            gputs("Disabled by Temp             ");
            break;

        case PUMP_STATUS_Standby:
            gsetcpos(1, 8);
            gputs("Standby Mode                 ");
            break;
            
        case PUMP_STATUS_Powersave:
            gsetcpos(1, 8);
            gputs("Power save mode             ");
            break;
            
        default:
            break;
    }
    
    drawProgressBar();
}

// **********************************************************************************************************
// drawSignalBars - Draw the signal strength bars
// **********************************************************************************************************

static void drawProgressBar()
{
    const INT8U xStart = 0;
    const INT8U yStart = 75;
    const INT8U height = 10;
    const INT8U width = GDISPW - 1;
    const INT32U progress = integerDivideRound(gRun.CycleProgress * GDISPW, 100);
    INT16U i;

    grectangle(xStart, yStart, xStart + width, yStart + height);
    for (i = 1; i < height; i++)
    {
        gmoveto(xStart, yStart + i);
        glineto(xStart + progress, yStart + i);
    }
}

// **********************************************************************************************************
// drawModeIcon - Draw the mode icon in the upper right hand corner of the screen
// **********************************************************************************************************

static void drawModeIcon(void)
{
    void* pIcon = (void*)&BMP_Screen_Modeicons_32x32[8];

    switch (gSetup.MeteringMode)
    {
        case METERING_MODE_VOLUME:
            pIcon = (void*)&BMP_Screen_Modeicons_32x32[8];
            break;

        case METERING_MODE_TIME:
            pIcon = (void*)&BMP_Screen_Modeicons_32x32[0];
            break;

        case METERING_MODE_CYCLES:
            pIcon = (void*)&BMP_Screen_Modeicons_32x32[2];
            break;

        default:
            break;
    }

    // Right justify tool icon
    (void)placeBitmap( GDISPW - gfgetfw(pIcon) - 1, 0, pIcon );
}

static void drawPumpIcon(void)
{
    IOrtn_digital_t pumpOutput;
    void* pIcon = (void*)&BMP_Screen_pumpIcons_32x36[0];

    pumpOutput = OUT_Digital_Latch_Get(IOPIN_OUTPUT_1);

    if ((pumpOutput.error == IOError_Ok) && (pumpOutput.state == ASSERTED))
    {
        pIcon = (void*)&BMP_Screen_pumpIcons_32x36[1];
    }
    else
    {
        pIcon = (void*)&BMP_Screen_pumpIcons_32x36[5];
    }

    (void)placeBitmap( GDISPW - gfgetfw(pIcon) - 1, gfgetfh(pIcon), pIcon );
}

