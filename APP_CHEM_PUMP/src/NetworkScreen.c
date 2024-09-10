// NetworkScreen.c

// Copyright 2014 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the logic for the network settings screen

// **********************************************************************************************************
// Header files
// **********************************************************************************************************

#include "pumpControlTask.h"
#include "dvseg_17G721_setup.h"
#include "dvseg_17G721_run.h"
#include "PublishSubscribe.h"
#include "screensTask.h"
#include "gdisp.h"
#include "NetworkScreen.h"
#include "CountDigit.h"
#include "SocketModem.h"
#include <stdio.h>
#include "modemTask.h"
#include "systemTask.h"
#include "screenStuff.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

#define FOUR_BARS               15u
#define THREE_BARS              10u
#define TWO_BARS                6u
#define ONE_BAR                 1u
#define NUM_SLAVE_ID_DIGITS      3
// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    BAUD_RATE_9600 = 0,
    BAUD_RATE_19200,
    BAUD_RATE_57600,
    BAUD_RATE_115200,
    NUMBER_BAUD_RATE_ITEMS
} BAUD_RATE_t;

typedef enum
{
    PARITY_NONE = 0,
    PARITY_ODD,
    PARITY_EVEN,
    NUMBER_PARITY_ITEMS
} PARITY_t;

typedef enum
{
    STOP_BITS_ONE = 0,
    STOP_BITS_TWO,
    NUMBER_STOP_BIT_ITEMS
} STOP_BITS_t;

typedef enum
{
    FOCUS_MODBUS_SLAVE_ID = 0,
    FOCUS_NETWORK_MODE,
    FOCUS_BAUD_RATE,
    FOCUS_MODBUS_PARITY,
    FOCUS_MODUBS_STOP_BITS,
    NUMBER_NETWORK_ITEMS
} NETWORK_FOCUS_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

static DIGIT_BOX_t     * gModbusSlaveIdDigitBox;
static SELECTION_BOX_t * gNetworkModeSelectBox;
static SELECTION_BOX_t * gBaudRateSelectBox;
static SELECTION_BOX_t * gParitySelectBox;
static SELECTION_BOX_t * gStopBitsSelectBox;

static char* ppNetworkModeSelectBoxTextList[] =
{
    "NONE",
    "CELL",
    "MODBUS"
};

static char* ppBaudRateSelectBoxTextList[] =
{
    "9600",
    "19200",
    "57600",
    "115200"
};

static char* ppParitySelectBoxTextList[] =
{
    "NONE",
    "ODD",
    "EVEN"
};

static char* ppStopBitsSelectBoxTextList[] =
{
    "ONE",
    "TWO"
};

static INPUT_MODE_t ReturnMode;
static NETWORK_FOCUS_t networkFocusIndex = FOCUS_MODBUS_SLAVE_ID;
static BAUD_RATE_t baudRateIndex = BAUD_RATE_115200;
static PARITY_t parityIndex = PARITY_NONE;
static STOP_BITS_t stopBitIndex = STOP_BITS_ONE;
static NETWORK_MODE_t networkMode = NETWORK_MODE_CELLULAR;
static INT32U modbusSlaveID = 0;
static INT32U modbusBaudRate = 0;
static INT32U modbusBaudRateIndex = 0;
static INT32U modbusParity = 0;
static INT32U modbusStopBits = 0;
static bool* isFocusArray[NUMBER_NETWORK_ITEMS];

static uint32 BaudRate[4] = {9600, 19200, 57600, 115200};

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static void drawNetworkScreen(NETWORK_FOCUS_t focusIndex);
static NETWORK_FOCUS_t incrementNetworkFocusIndex(NETWORK_FOCUS_t focusIndex);
static NETWORK_FOCUS_t decrementNetworkFocusIndex(NETWORK_FOCUS_t focusIndex);
static void drawSignalBars(void);
static void processInputEntryEvent(void);
static void processInputResetEvent(void);
static void processInputEnterEvent(void);
static void processInputUpArrowEvent(void);
static void processInputDownArrowEvent(void);
static void processInputBothArrowEvent(void);
static void loadIsFocusArray(void);

// **********************************************************************************************************
// NetworkScreen - Main handler for the network screen
// **********************************************************************************************************

INPUT_MODE_t NetworkScreen(INPUT_EVENT_t InputEvent)
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
	processInputEvent[INPUT_EVENT_BOTH_ARROWS] = processInputBothArrowEvent;
	processInputEvent[INPUT_EVENT_REFRESH_SCREEN] = processInputDefaultEvent;
    
    // Process based on input event
    (void)(*processInputEvent[InputEvent])();
    
    // Draw the result
    gselvp(VIEWPORT_MAIN);

    // Clear the Viewport
    gclrvp();

    drawNetworkScreen(networkFocusIndex);

    // Return the mode
    return (ReturnMode);
}

//****************************************************************************//       
//Fcn: loadIsFocusArray
//
//Desc: Tie all isFocus items to an array to make the updating of them easier
//****************************************************************************//
static void  loadIsFocusArray(void)
{
    isFocusArray[FOCUS_MODBUS_SLAVE_ID] = &((*gModbusSlaveIdDigitBox).isFocus);
    isFocusArray[FOCUS_NETWORK_MODE] = &((*gNetworkModeSelectBox).isFocus);
    isFocusArray[FOCUS_BAUD_RATE] = &((*gBaudRateSelectBox).isFocus);
    isFocusArray[FOCUS_MODBUS_PARITY] = &((*gParitySelectBox).isFocus);
    isFocusArray[FOCUS_MODUBS_STOP_BITS] = &((*gStopBitsSelectBox).isFocus);
}

// **********************************************************************************************************
// incrementNetworkFocusIndex - Move focus to the next field
// **********************************************************************************************************
static NETWORK_FOCUS_t incrementNetworkFocusIndex(NETWORK_FOCUS_t focusIndex)
{
    uint8_t numItems = NUMBER_NETWORK_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;    

    // Only one select box is shown when CELL or NONE is selected
    if ((gSetup.NetworkMode == NETWORK_MODE_CELLULAR) || (gSetup.NetworkMode == NETWORK_MODE_NO_NETWORK))
    {
        focusIndex = FOCUS_NETWORK_MODE;
    }
    else
    {
        if( focusIndex < (numItems - 1) )
        {
            focusIndex = focusIndex + 1;
        }
        else
        {
            focusIndex = 0;
        }        
    }

    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// decrementNetworkFocusIndex - Move focus to the previous field
// **********************************************************************************************************
static NETWORK_FOCUS_t decrementNetworkFocusIndex(NETWORK_FOCUS_t focusIndex)
{
    uint8_t numItems = NUMBER_NETWORK_ITEMS;
    
    //take the focus away from the old item
    *isFocusArray[focusIndex] = FALSE;
    
    // Only one select box is shown when CELL or NONE is selected
    if ((gSetup.NetworkMode == NETWORK_MODE_CELLULAR) || (gSetup.NetworkMode == NETWORK_MODE_NO_NETWORK))
    {
        focusIndex = FOCUS_NETWORK_MODE;
    }
    else
    {
        if( focusIndex > 0 )
        {
            focusIndex = focusIndex - 1;
        }
        else
        {
            focusIndex = (numItems - 1);
        }
    }

    //give the new item the focus
    *isFocusArray[focusIndex] = TRUE;
    
    return focusIndex;
}

// **********************************************************************************************************
// drawStaticText - Draw the rest of the screen
// **********************************************************************************************************
static void drawNetworkScreen(NETWORK_FOCUS_t focusIndex)
{
    char buf[25];   

    if( gSetup.NetworkMode == NETWORK_MODE_CELLULAR )
    {
        gsetcpos(0, 0);
        gputs(PUBLISH_getPumpName());        
        gsetcpos(0, 1);
        sprintf(buf, "ID: %s", GetId());
        gputs(buf);
    }

    gsetcpos(0, 3);
    gputs("TYPE:");

    if( gSetup.NetworkMode == NETWORK_MODE_CELLULAR )
    {
        if( gRun.ConnectionStatus == CONNECTION_SUCCESS)
        {
            sprintf(buf, "%s", "STATUS: ONLINE");
            gsetcpos(0, 5);
            gputs(buf);

            gsetcpos(0, 7);
            sprintf(buf, "SIGNAL: %d", GetSignalStrength());
            gputs(buf);

            drawSignalBars();

        }
        else
        {
            if (gRun.ModemFound)
            {
                sprintf(buf, "STATUS: CONNECTING... (%d)", (uint8_t)gRun.ConnectionStatus);
                gsetcpos(0, 5);
                gputs(buf);
            }
            else
            {
                sprintf(buf, "SEARCHING FOR MODEM...");
                gsetcpos(0, 5);
                gputs(buf);
            }
        }
    }
    else if( gSetup.NetworkMode == NETWORK_MODE_MODBUS)
    {
        gsetcpos(0, 1);
        gputs("SLAVE ID: ");
        
        gsetcpos(0, 4);
        gputs("BAUD: ");
        
        gsetcpos(0, 5);
        gputs("PARITY: ");
        
        gsetcpos(0, 6);
        gputs("STOP: ");
    }
    
    if ((gSetup.NetworkMode == NETWORK_MODE_CELLULAR) || (gSetup.NetworkMode == NETWORK_MODE_NO_NETWORK))
    {
        (*gNetworkModeSelectBox).isOutlined = TRUE;
        (*gNetworkModeSelectBox).isHidden = FALSE;
        (*gModbusSlaveIdDigitBox).isHidden = TRUE;
        (*gBaudRateSelectBox).isHidden = TRUE;
        (*gParitySelectBox).isHidden = TRUE;
        (*gStopBitsSelectBox).isHidden = TRUE;
    }
    else
    {
        (*gNetworkModeSelectBox).isOutlined = FALSE;
        (*gNetworkModeSelectBox).isHidden = FALSE;
        (*gModbusSlaveIdDigitBox).isHidden = FALSE;
        (*gBaudRateSelectBox).isHidden = FALSE;
        (*gParitySelectBox).isHidden = FALSE;
        (*gStopBitsSelectBox).isHidden = FALSE;
    }
    
    drawAllDigitBoxes();
    drawAllSelectBoxes();
}

// **********************************************************************************************************
// drawSignalBars - Draw the signal strength bars
// **********************************************************************************************************
static void drawSignalBars(void)
{
    if( gSetup.NetworkMode == NETWORK_MODE_CELLULAR )
    {
        const INT8U xStart = 67;
        const INT8U yStart = 93;
        const INT8U baseHeight = 4;
        const INT8U stepHeight = 3;
        const INT8U width = 3;
        const INT8U numberBars = 4;
        const INT32U signalStrength = GetSignalStrength();
        INT8U i;
        INT8U signalStrengthBars = 0;

        // Determine number of bars from signal strength
        // These parameters were determined from the Telit User guide
        if( signalStrength >= FOUR_BARS )
        {
            signalStrengthBars = 4;
        }

        else if( signalStrength >= THREE_BARS )
        {
            signalStrengthBars = 3;
        }

        else if( signalStrength >= TWO_BARS )
        {
            signalStrengthBars = 2;
        }

        else if( signalStrength >= ONE_BAR )
        {
            signalStrengthBars = 1;
        }

        else
        {
            signalStrengthBars = 0;
        }

        // Draw the signal strength bars
        for( i = 0; i < numberBars; i++ )
        {
            // Draw rectangles 
            grectangle(xStart + (i * width), yStart - baseHeight - (i * stepHeight), xStart + (i * width) + width, yStart);

            // Fill in rectangles using lines
            if( signalStrengthBars > i )
            {
                gmoveto(xStart + (i * width) + 1, yStart - baseHeight - (i * stepHeight));
                glineto(xStart + (i * width) + 1, yStart);
                gmoveto(xStart + (i * width) + 2, yStart - baseHeight - (i * stepHeight));
                glineto(xStart + (i * width) + 2, yStart);
            }
        }
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
    
    // Unless something changes it return select mode
    ReturnMode = INPUT_MODE_NETWORK;
    
    networkMode = gSetup.NetworkMode;
    modbusSlaveID = gSetup.ModbusSlaveID;
    modbusBaudRateIndex = gSetup.ModbusBaudrateIndex;
    modbusBaudRate = BaudRate[modbusBaudRateIndex];
    modbusParity = gSetup.ModbusParity;
    modbusStopBits = gSetup.ModbusStopBits;
    
    //load shared digit boxes
    gModbusSlaveIdDigitBox = &digitBox1;
    (void)LoadCountDigit(&(*gModbusSlaveIdDigitBox).countDigit, modbusSlaveID, NUM_SLAVE_ID_DIGITS, NO_DECIMAL_POINT, 9, 1, FALSE, FALSE);
    
    //load shared select boxes
    gNetworkModeSelectBox = &selectBox1;
    gBaudRateSelectBox = &selectBox2;
    gParitySelectBox = &selectBox3;
    gStopBitsSelectBox = &selectBox4;
    
    (void)selectBoxConfigure(gNetworkModeSelectBox, 0, NUMBER_NETWORK_MODES, FALSE, FALSE, FALSE, TRUE, 7, 3, 8, ppNetworkModeSelectBoxTextList);
    (void)selectBoxConfigure(gBaudRateSelectBox, 0, NUMBER_BAUD_RATE_ITEMS, FALSE, FALSE, FALSE, FALSE, 7, 4, 8, ppBaudRateSelectBoxTextList);
    (void)selectBoxConfigure(gParitySelectBox, 0, NUMBER_PARITY_ITEMS, FALSE, FALSE, FALSE, FALSE, 7, 5, 8, ppParitySelectBoxTextList);
    (void)selectBoxConfigure(gStopBitsSelectBox, 0, NUMBER_STOP_BIT_ITEMS, FALSE, FALSE, FALSE, FALSE, 7, 6, 8, ppStopBitsSelectBoxTextList);
    
    (*gNetworkModeSelectBox).index = networkMode;
    (*gBaudRateSelectBox).index = modbusBaudRateIndex;
    (*gParitySelectBox).index = modbusParity;
    (*gStopBitsSelectBox).index = modbusStopBits;
    
    // load focus array with editable boxes & set starting focus to first item
    loadIsFocusArray();
    if (gSetup.NetworkMode == NETWORK_MODE_MODBUS)
    {
        networkFocusIndex = FOCUS_MODBUS_SLAVE_ID;
    }
    else
    {
        networkFocusIndex = FOCUS_NETWORK_MODE;
    }
    *isFocusArray[networkFocusIndex] = TRUE;
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
    (void)LoadCountDigit(&(*gModbusSlaveIdDigitBox).countDigit, modbusSlaveID, NUM_SLAVE_ID_DIGITS, NO_DECIMAL_POINT, 9, 1, FALSE, FALSE);
    (*gNetworkModeSelectBox).index = networkMode;
    (*gBaudRateSelectBox).index = modbusBaudRateIndex;
    (*gParitySelectBox).index = modbusParity;
    (*gStopBitsSelectBox).index = modbusStopBits;
    clearAllIsEdit();
}

//****************************************************************************//
//Fcn: processInputEnterEvent
//
//Desc: This function processes the enter button events
//****************************************************************************//
static void processInputEnterEvent(void)
{
    switch( networkFocusIndex )
    {
        case FOCUS_NETWORK_MODE:
            if( (*gNetworkModeSelectBox).isEditMode == TRUE )
            {
                networkMode = (*gNetworkModeSelectBox).index;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, NetworkMode), (DistVarType)networkMode);
                (*gNetworkModeSelectBox).isEditMode = FALSE;

                if (networkMode == NETWORK_MODE_CELLULAR)
                {
                    SYSTEM_StopCommsTimer();
                    ReturnMode = INPUT_MODE_ACTIVATION;
                }
                else if (networkMode == NETWORK_MODE_MODBUS)
                {
                    SYSTEM_StartCommsTimer();    
                }
                SYSTEM_enableModbusOrCellular();
            }
            else
            {
                (*gNetworkModeSelectBox).isEditMode = TRUE;
            }

            break;

        case FOCUS_MODBUS_SLAVE_ID:
            if( (*gModbusSlaveIdDigitBox).isEditMode == TRUE )
            {
                modbusSlaveID = GetCountDigitValue(&(*gModbusSlaveIdDigitBox).countDigit);
                (*gModbusSlaveIdDigitBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, ModbusSlaveID), (DistVarType)modbusSlaveID);
                SYSTEM_enableModbusOrCellular();
            }
            else
            {
                (*gModbusSlaveIdDigitBox).countDigit.DigitSelected = COUNT_DIGIT_100;
                (*gModbusSlaveIdDigitBox).isEditMode = TRUE;
            }
            break;
            
        case FOCUS_BAUD_RATE:
            if( (*gBaudRateSelectBox).isEditMode == TRUE)
            {
                modbusBaudRateIndex = (*gBaudRateSelectBox).index;
                modbusBaudRate = BaudRate[(*gBaudRateSelectBox).index];
                (*gBaudRateSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal( DVA17G721_SS( gSetup, ModbusBaudrateIndex ), modbusBaudRateIndex );
                SYSTEM_enableModbusOrCellular();
            }
            else
            {
                (*gBaudRateSelectBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_MODBUS_PARITY:
            if( (*gParitySelectBox).isEditMode == TRUE)
            {
                modbusParity = (*gParitySelectBox).index;
                (*gParitySelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal( DVA17G721_SS( gSetup, ModbusParity ), modbusParity );
                SYSTEM_enableModbusOrCellular();
            }
            else
            {
                (*gParitySelectBox).isEditMode = TRUE;
            }
            break;

        case FOCUS_MODUBS_STOP_BITS:
            if( (*gStopBitsSelectBox).isEditMode == TRUE)
            {
                modbusStopBits = (*gStopBitsSelectBox).index;
                (*gStopBitsSelectBox).isEditMode = FALSE;
                (void)DVAR_SetPointLocal( DVA17G721_SS( gSetup, ModbusStopBits ), modbusStopBits );
                SYSTEM_enableModbusOrCellular();
            }
            else
            {
                (*gStopBitsSelectBox).isEditMode = TRUE;
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
    if((upEventForDigitBox() == FALSE) && (upEventForSelectBox() == FALSE))
    {
        networkFocusIndex = decrementNetworkFocusIndex(networkFocusIndex);
    }
}

//****************************************************************************//
//Fcn: processInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processInputDownArrowEvent(void)
{   
    if((downEventForDigitBox() == FALSE) && (downEventForSelectBox() == FALSE))
    {
        networkFocusIndex = incrementNetworkFocusIndex(networkFocusIndex);
    }
}

//****************************************************************************//
//Fcn: processInputBothArrowEvent
//
//Desc: This handles the both arrows pressed event
//****************************************************************************//
static void processInputBothArrowEvent(void)
{
    ReturnMode = INPUT_MODE_SNOOP;
}

