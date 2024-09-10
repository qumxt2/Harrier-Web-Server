// modemTask.c

// Copyright 2015 - 2016
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The modem task handles communication with the modem

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"
#include "rtos.h"
#include "debug_app.h"
#include "modemTask.h"
#include "serial_uart_u1a.h"
#include "socketModem.h"
#include "stdio.h"
#include "PublishSubscribe.h"
#include "dvseg_17G721_setup.h"
#include "sslHelper.h"
#include "out_digital.h"
#include "dvseg_17G721_run.h"
#include "dvseg_17G721_setup.h"
#include "NetworkScreen.h"
#include "SnoopScreen.h"
#include "utilities.h"

// ******************************************************************************************
// CONSTANTS AND MACROS
// ******************************************************************************************
#define MODEM_TIMEOUT_DEFAULT                   (100)
#define MODEM_BAUD_RATE                         (115200)
#define HEARTBEAT_PERIOD                        (100) //ms
#define STARTUP_DELAY                           (5000) //ms
#define MODEM_RESET_DELAY                       (1000)

const char noCarrierString[] =                  "\r\nNO CARRIER\r\n";
const char connectString[] =                    "\r\nCONNECT\r\n";

// ******************************************************************************************
// PUBLIC VARIABLES
// ******************************************************************************************

uint8 gModemTaskID;

// ******************************************************************************************
// PRIVATE VARIABLES
// ******************************************************************************************
bool gDebugTerminal = FALSE;
bool gSnoop = FALSE;
bool gCarrierDetect = FALSE;
uint8 gNoCarrierMatchLen = 0;
uint8 gConnectMatchLen = 0;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

// ******************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// ******************************************************************************************

static void modemTxCallback(void);
static void modemRxCallback(void);
static void modemCheckCarrier(uint8 rxByte);

// ******************************************************************************************
// PUBLIC FUNCTIONS
// ******************************************************************************************

uint8 modemTaskInit (void)
{
    uint8 status = MODEM_TASK_INIT_OK;

    // Reserve a task ID and queue the task
    gModemTaskID = RtosTaskCreateStart(ModemTask);
    if (gModemTaskID == RTOS_INVALID_ID)
    {
        status = MODEM_TASK_INIT_ERROR;
    }

    if (status == MODEM_TASK_INIT_ERROR)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "MODEM TASK INIT ERROR\r\n");
    }

    PUBLISH_init();
    
    return status;
}

void ModemTask (void)
{
    uint8 modemByte = 0;

    // Wait for DVARs to be ready
    (void)K_Resource_Wait(gDvarLockoutResID, 0);
    (void)K_Resource_Release(gDvarLockoutResID);

#ifndef FORCE_INSECURE
    (void)SSL_Init();
#endif
    
    // Allow a chance to start terminal mode before the normal chatter begins
    delay(STARTUP_DELAY);

    if (!gDebugTerminal)
    {
        (void)PUBLISH_reconnect();
    }
    while (1)
    {
        delay(HEARTBEAT_PERIOD);
        if (gSetup.NetworkMode == NETWORK_MODE_CELLULAR)
        {
            if (gDebugTerminal)
            {
                while (MODEM_GetByteTerminal(&modemByte) && gDebugTerminal)
                {
                    printf("%c", modemByte);
                }
            }
            else
            {
                if (!gCarrierDetect || !gRun.MqttConnected)
                {
                    (void)PUBLISH_reconnect();
                }
                ServiceSubscriptions();
                PUBLISH_WriteSerial();
            }
        }
    }
}

// Allow data to be sent while in terminal mode
void MODEM_SendStringTerminal(uint8* pData, uint16 len)
{
    if (gDebugTerminal)
    {
        (void)Serial_U1A_Tx(pData, len, MODEM_TIMEOUT_DEFAULT);
    }
}
// Setup the UART for the modem task
void MODEM_configureUart(void)
{
     sint8 error = 0;
        
    UART_OPTIONS_T serialPortCfg =
    {
        .baudRate = MODEM_BAUD_RATE,
        .flowControl = FC_CTSRTS,
        .parity = PARITY_NONE,
        .stopBits = 1,
        .txCallback = modemTxCallback,
        .rxCallback = modemRxCallback,
        .txPolarity = TX_POL_POS,
        .rxPolarity = RX_POL_POS
    };
    
    // Modem setup must be done after the RTOS is running
    error = Serial_U1A_Init(&serialPortCfg);
    if (error != 0)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "SERIAL PORT INIT ERROR\r\n");
    }
    gCarrierDetect = FALSE;
}

// Set flag to reset modem
void MODEM_HardwareReset(void)
{
    (void)OUT_Digital_Latch_Set(IOPIN_MODEM_RESET, NOT_ASSERTED);
    delay(MODEM_RESET_DELAY);
    (void)OUT_Digital_Latch_Set(IOPIN_MODEM_RESET, ASSERTED);
}

// Send data unless in terminal mode
sint16 MODEM_SendString(uint8* pData, uint16 len)
{
    sint16 retVal = len;
    
    if ((!gDebugTerminal) && (gSetup.NetworkMode == NETWORK_MODE_CELLULAR))
    {
        if (Serial_U1A_Tx(pData, len, MODEM_TIMEOUT_DEFAULT))
        {
            (void)printf("\n##UART1 send timed out\n");
            retVal = MODEM_ERR_TIMEOUT;
        }

        if (gSnoop)
        {
            uint16 i;

            (void)printf("\n[MODEM OUT]");
            for (i = 0; i < len; i++)
            {
#ifndef BINARY_MODEM_SNOOP
                if ((pData[i] >= ' ' && pData[i] < 127) || pData[i] == '\n')
                {
                    (void)printf("%c", pData[i]);
                    SNOOP_putc(pData[i]);
                }
                else if (pData[i] == '\r')
                {
                    // Replace with linefeed to avoid line overwrites
                    (void)printf("\n");
                    SNOOP_putc('\n');
                }
                else
#endif
                {
                    (void)printf(" 0x%02X", pData[i]);
                }
            }
            (void)printf("[MODEM OUT DONE]\n");
        }
    }

    return retVal;
}

// Get data while in terminal mode
bool MODEM_GetByteTerminal(uint8* pByte)
{
    bool dataAvailable = FALSE;
    if (gDebugTerminal)
    {
        dataAvailable = Serial_U1A_Rx(pByte);
    }
    return dataAvailable;
}

// Allow data to be retrieved unless in terminal mode
bool MODEM_GetByte(uint8* pByte)
{
    bool dataAvailable = FALSE;
    if (!gDebugTerminal)
    {
        dataAvailable = Serial_U1A_Rx(pByte);

        if (dataAvailable)
        {
            modemCheckCarrier(*pByte);

            if (gSnoop)
            {
#ifndef BINARY_MODEM_SNOOP
                if ((*pByte >= ' ' && *pByte < 127) || *pByte == '\n')
                {
                    (void)printf("%c", *pByte);
                    SNOOP_putc(*pByte);
                }
                else if (*pByte == '\r')
                {
                    // Replace with linefeed to avoid line overwrites
                    (void)printf("\n");
                    SNOOP_putc('\n');
                }
                else
#endif
                {
                    (void)printf(" 0x%02X", *pByte);
                }
            }
        }
    }
    return dataAvailable;
}

void MODEM_DebugEcho(bool echoOn)
{
    gDebugTerminal = echoOn;
}

void MODEM_Snoop(bool snoopOn)
{
    gSnoop = snoopOn;
}

// Returns true if we have an open socket connection
bool MODEM_Connected(void)
{
    return gCarrierDetect;
}

////
//// Private functions
////

static void modemTxCallback(void)
{
    // Ignore
}

static void modemRxCallback(void)
{
    (void)K_Event_Signal(RTOS_NOTIFY_SPECIFIC, gModemTaskID, EVT_MODEM_BYTES_RECEIVED);
}

// Monitor the data flow from the modem to try to infer whether we're currently connected
// Not infallible; can be fooled by payload bytes containing the magic words. However,
// since we expect to be sending only TLS encrypted data over the interface, this shouldn't
// be a hazard. Revisit if we plan to return to sending plaintext.
static void modemCheckCarrier(uint8 rxByte)
{
    // Look for the string of characters corresponding to a dropped connection in
    // the received bytestream.
    if (gCarrierDetect)
    {
        if ( (uint8)noCarrierString[gNoCarrierMatchLen] == rxByte)
        {
            // Sequence matching so far
            gNoCarrierMatchLen++;
        }
        else if ( (uint8)noCarrierString[0] == rxByte)
        {
            // Also check if we're starting the match over. Important when
            // we had a partial match immediately prior.
            gNoCarrierMatchLen = 1;
        }
        else
        {
            // Broke our string of matches
            gNoCarrierMatchLen = 0;
        }

        if (gNoCarrierMatchLen == sizeof(noCarrierString)-1 )
        {
            gCarrierDetect = FALSE;
            printf("\nDisconnection detected\n");
        }
    }

    if (!gCarrierDetect)
    {
        if ( (uint8)connectString[gConnectMatchLen] == rxByte)
        {
            // Sequence matching so far
            gConnectMatchLen++;
        }
        else if ( (uint8)connectString[0] == rxByte)
        {
            // Also check if we're starting the match over. Important when
            // we had a partial match immediately prior.
            gConnectMatchLen = 1;
        }
        else
        {
            // Broke our string of matches
            gConnectMatchLen = 0;
        }

        if (gConnectMatchLen == sizeof(connectString)-1 )
        {
            gCarrierDetect = TRUE;
            printf("\nConnection detected\n");
        }
    }
}

