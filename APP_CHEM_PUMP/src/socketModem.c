// SocketModem.c

#include "stdio.h"


// Copyright 2014
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the serial communication to the cellular modem

// **********************************************************************************************************
// Header files
// **********************************************************************************************************

#include "typedef.h"
#include "stdint.h"
#include "debug.h"
#include "rtos.h"
#include "modemTask.h"
#include "socketModem.h"
#include "string.h"
#include "dvar.h"
#include "dvinterface_17G721.h"
#include "dvseg_17G721_run.h"
#include "screensTask.h"
#include "PublishSubscribe.h"
#include "utilities.h"

// **********************************************************************************************************
// Constants and macros
// **********************************************************************************************************

#define MAX_REPLY_SIZE              500u
#define AT_RETRIES                  30u
#define NETWORK_RETRIES             600u
#define MODEM_ERROR                 0xFF

#define CHAR_TO_INT(x)              (x - '0')

// 1883 is the insecure MQTT port, and 8883 is the SSL/TLS-enabled one
#ifndef FORCE_INSECURE
#define MQTT_PORT                   "8883"
#else
#define MQTT_PORT                   "1883"
#endif
#define HTTP_PORT                   "443"

// Modem command strings and expected responses
#define VERBOSE_ERROR_COMMAND       "AT+CMEE=2\r\n"
#define VERBOSE_ERROR_RESPONSE      "\r\nOK"
#define SIGNAL_STRENGTH_COMMAND     "AT+CSQ\r\n"
#define SIGNAL_STRENGTH_RESPONSE    "\r\n+CSQ: "
#define REGISTRATION_COMMAND        "AT+CREG?\r\n"
#define REGISTRATION_RESPONSE       "\r\n+CREG: 0,"
#define CONFIGURE_SOCKET_COMMAND    "AT#SCFG=1,1,300,240,600,2\r\n"
#define CONFIGURE_SOCKET_RESPONSE   "\r\nOK"
#define OPEN_DATA_COMMAND           "AT#SGACT=1,1\r\n"
#define OPEN_DATA_RESPONSE          "\r\n#SGACT: "
#define CHECK_OPEN_DATA_COMMAND     "AT#SGACT?\r\n"
#define CHECK_OPEN_DATA_RESPONSE    "\r\n#SGACT: 1,1"
#define SOCKET_STATUS_COMMAND       "AT#SS\r\n"
#define SOCKET_STATUS_RESPONSE      "\r\n#SS: 1,0"
#define CLOSE_SOCKET_COMMAND        "AT#SH=1\r\n"
#define CLOSE_SOCKET_RESPONSE       "\r\nNO CARRIER"
#define OPEN_MQTT_SOCKET_COMMAND    "AT#SD=1,0," MQTT_PORT "," HOST_FQDN ",0,0,0\r\n"
#define OPEN_HTTP_SOCKET_COMMAND    "AT#SD=1,0," HTTP_PORT "," HOST_FQDN ",0,0,0\r\n"
#define OPEN_SOCKET_RESPONSE        "\r\nCONNECT"
#define GOTO_COMMAND_MODE           "+++"
#define COMMAND_MODE_RESPONSE       "\r\n\r\nOK"
#define AT_COMMAND                  "AT\r\n"
#define AT_RESPONSE                 "\r\nOK"
#define ECHO_OFF_COMMAND            "ATE0\r\n"
#define ID_COMMAND                  "AT#CIMI\r\n"
#define ID_RESPONSE                 "\r\n"
#define RESET_COMMAND               "ATZ\r\n"
#define REST_RESPONSE               "ATZ\r\r\nOK"
#define LOCATION_COMMAND            "AT#AGPSSND\r\n"
#define LOCATION_RESPONSE           "\r\nOK"
#define LOCATION_HEADER             "\r\n#AGPSRING: 200,"
#define LOCATION_ERROR              "\r\n#AGPSRING: 200,,,,,,"
#define GSM_ACTIVATE_COMMAND        "AT+CGDCONT=1,\"IP\", iot.aer.net\r\n"
#define GSM_ACTIVATE_RESPONSE       "\r\nOK"
#define MODEM_GENERAL_ERROR         "\r\nERROR"

#define NETWORK_HOME                1u
#define NETWORK_ROAMING             5u
#define MIN_SIGNAL_STRENGTH         1u
#define MAX_SIGNAL_STRENGTH         99u
#define LOCATION_OFFSET             13u
#define UNIQUE_ID_SIZE              16u
#define LOCATION_SIZE               PUBLISH_PAYLOAD_SIZE

#define LOCATION_GET_OK                 (0)
#define LOCATION_GET_NONE               (-1)
#define LOCATION_GET_ERROR_FATAL        (-2)

const char* const ConnectionStatusText[CONNECTION___ENUM_SIZE] =
{
    "success",
    "in progress",
    "AT error",
    "config verbose error",
    "sig strength error",
    "unique ID error",
    "registration error",
    "open data error",
    "open socket error",
    "location error",
    "SSL error"
};

typedef enum
{
    AT_RESP_OK = 0,
    AT_RESP_NO_RESP = -1,
    AT_RESP_WRONG_REPLY = -2,
    AT_RESP_ERROR = -3,
    AT_RESP_TIMEOUT = -4,
} AT_response_t;

// **********************************************************************************************************
// Private variables
// **********************************************************************************************************

//static uint8_t gReceiveBuf[BUFFER_SIZE];
static char gUniqueID[UNIQUE_ID_SIZE] = "<NOT SET>";
static uint8_t gSignalStrength = 0;
static char gLocation[LOCATION_SIZE] = "";

// Allow the location check to be skipped, which is needed
// for certain buggy Telit modem firmware releases
static bool gSkipLocationCheck = FALSE;

// **********************************************************************************************************
// Private functions
// **********************************************************************************************************

static int8_t waitForSignalStrength(void);
static int8_t waitForNetwork(void);
static void sendString(char* pData, uint16_t size);
static int8_t openData(void);
static int8_t openSocket(void);
static AT_response_t sendATCommand(char* pCmd, uint16_t cmdSize, char* pExpectedRsp,
                            uint16_t rspSize, char* pActualRsp, uint16 rspBufLen, uint16_t timeout);
static int8_t waitForOK(void);
static int8_t waitVerboseErrorMessages(void);
static int8_t getUniqueID(char* pID);
static sint8 getLocation(char* pLocation);
static CONNECTION_STATUS_t connectModem(void);
static CONNECTION_STATUS_t resetModem(void);
static int MODEM_sscanf(const char* str, const char* format, ...);
static int8_t sendGsmActivate(void);

// **********************************************************************************************************
// InitializeSocketModem - Initial communication and connection to the modem
// **********************************************************************************************************

CONNECTION_STATUS_t InitializeSocketModem(void)
{
    static CONNECTION_STATUS_t connectionStatus = CONNECTION_IN_PROGRESS;

    (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, ConnectionStatus), (DistVarType)connectionStatus);
    RefreshScreen();

    // Attempt to connect to the network and also get location
    connectionStatus = connectModem();

    // Don't really consider ourselves successful until we've made an MQTT connection
    if (connectionStatus != CONNECTION_SUCCESS)
    {
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, ConnectionStatus), (DistVarType)connectionStatus);
        RefreshScreen();

        // If connection is unsucessful, reset modem and try once more
        if (CONNECTION_IN_PROGRESS == resetModem())
        {
            // If reset is successful try again to connect
            connectionStatus = connectModem();
        }
    }

    if( connectionStatus == CONNECTION_SUCCESS )
    {
        // Pretend we're still not fully successful, since success only comes with a full MQTT connection
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, ConnectionStatus), (DistVarType)CONNECTION_IN_PROGRESS);
        RefreshScreen();

        printf( "Socket opened\n" );
    }
    else
    {
        // Be sure the network screen is in sync
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, ConnectionStatus), (DistVarType)connectionStatus);
        RefreshScreen();

        // Perform a HW reset on the modem so it can 
        // hopefully recover from the error.
        MODEM_HardwareReset();
        
        printf( "Could not open socket!\n" );
    }

    return connectionStatus;
}

// **********************************************************************************************************
// connectModem - Calls the sequence of events to initialize and connect modem
// **********************************************************************************************************

static CONNECTION_STATUS_t connectModem(void)
{
    CONNECTION_STATUS_t status = CONNECTION_IN_PROGRESS;
    sint8 locStatus = LOCATION_GET_NONE;

    // Wait for AT
    if ((status == CONNECTION_IN_PROGRESS) && (waitForOK() != 0))
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "AT Error");
        status = CONNECTION_AT_ERROR;
    }

    // Wait for verbose error messages to be enabled
    if( (status == CONNECTION_IN_PROGRESS) && (waitVerboseErrorMessages() != 0) )
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "Error Message Config Error");
        status = CONNECTION_CONFIG_VERBOSITY_ERROR;
    }

    // Wait for signal strength
    if ((status == CONNECTION_IN_PROGRESS) && (waitForSignalStrength() != 0))
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "Signal Strength Error");
        status = CONNECTION_SIGNAL_STRENGTH_ERROR;
    }

    // Get unique ID
    if ((status == CONNECTION_IN_PROGRESS) && (getUniqueID(gUniqueID) != 0))
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "Unique ID Error");
        status = CONNECTION_UNIQUE_ID_ERROR;
    }

    //Get Activate if GSM modem
    if ((status == CONNECTION_IN_PROGRESS) && (sendGsmActivate() != 0))
    {
        // It is okay if this step fails (normal for CDMA)
        // For, GSM it is for the edge case where the activation 
        // is forgotten or lost.
    }
    
    // Wait for network
    if ((status == CONNECTION_IN_PROGRESS) && (waitForNetwork() != 0))
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "Network Registration Error");
        status = CONNECTION_REGISTRATION_ERROR;
    }

    // Open the data connection
    if ((status == CONNECTION_IN_PROGRESS) && (openData() != 0))
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "Open Data Error");
        status = CONNECTION_OPEN_DATA_ERROR;
    }

    if( (status == CONNECTION_IN_PROGRESS) && !gSkipLocationCheck)
    {
        locStatus = getLocation(gLocation);

        if (locStatus == LOCATION_GET_NONE)
        {
            // Failing to get the location is not a fatal error and should not interrupt
            // the connection sequence, but we should say something about it on the debug
            // portal.
            DEBUG_PRINT_STRING(DBUG_ALWAYS, "Could not get location");
        }
        else if (locStatus == LOCATION_GET_ERROR_FATAL)
        {
            // An error response from the modem while getting the location *is* a problem,
            // so we will need to restart the connect, and we need to skip the location check
            // the next time through
            status = CONNECTION_LOCATION_ERROR;
            gSkipLocationCheck = TRUE;
            DEBUG_PRINT_STRING(DBUG_ALWAYS, "Abort error getting location");
        }
    }

    // Open the socket connection
    if ((status == CONNECTION_IN_PROGRESS) && (openSocket() != 0))
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "Open Socket Error");
        status = CONNECTION_OPEN_SOCKET_ERROR;
    }

    // Confirm success
    if (status == CONNECTION_IN_PROGRESS)
    {
        status = CONNECTION_SUCCESS;
    }

    return status;
}

// **********************************************************************************************************
// resetModem - Sends the command to reset the modem
// **********************************************************************************************************

static CONNECTION_STATUS_t resetModem(void)
{
    char modemReply[MAX_REPLY_SIZE];
    CONNECTION_STATUS_t status = CONNECTION_AT_ERROR;

    if (waitForOK() == 0)
    {
        // Send reset command and ignore reply
        (void)sendATCommand(RESET_COMMAND, sizeof(RESET_COMMAND), 
                      REST_RESPONSE, sizeof(REST_RESPONSE), modemReply, sizeof(modemReply), 2000);

        if (waitForOK() == 0)
        {
            status = CONNECTION_IN_PROGRESS;
        }
    }

    return status;
}

// **********************************************************************************************************
// waitForSignalStrength - Sends a command to check the signal strength of the modem
// **********************************************************************************************************

static int8_t waitForSignalStrength(void)
{
    char modemReply[MAX_REPLY_SIZE];
    uint32_t i;

    for (i = 0; i < NETWORK_RETRIES; i++)
    {

        if (sendATCommand(SIGNAL_STRENGTH_COMMAND, sizeof(SIGNAL_STRENGTH_COMMAND),
                          SIGNAL_STRENGTH_RESPONSE, sizeof(SIGNAL_STRENGTH_RESPONSE), 
                          modemReply, sizeof(modemReply), 1000) == 0)
        {
            
            // Calculate signal strength based on where ',' appears in reply (1 digit vs 2 digits)
            if (modemReply[10] == ',')
            {
                gSignalStrength = (CHAR_TO_INT(modemReply[8]) * 10) + CHAR_TO_INT(modemReply[9]);
            }
            else
            {
                gSignalStrength = CHAR_TO_INT(modemReply[8]);
            }

            // Check for a valid signal strength
            if ((gSignalStrength > MIN_SIGNAL_STRENGTH) && (gSignalStrength < MAX_SIGNAL_STRENGTH))
            {
                return 0;
            }
        }
        delay(1000);
    }
    return MODEM_ERROR;
}

// **********************************************************************************************************
// waitForNetwork - Sends a command to check register the modem on the cellular network
// **********************************************************************************************************

static int8_t waitForNetwork(void)
{
    uint32_t i;
    char modemReply[MAX_REPLY_SIZE];

    for (i = 0; i < NETWORK_RETRIES; i++)
    {
        if (sendATCommand(REGISTRATION_COMMAND, sizeof(REGISTRATION_COMMAND), 
                          REGISTRATION_RESPONSE, sizeof(REGISTRATION_RESPONSE), modemReply, sizeof(modemReply), 1000) == 0)
        {
            const uint8_t registrationType = CHAR_TO_INT(modemReply[11]);

            // Check for a valid registration type
            if ((registrationType == NETWORK_HOME) || (registrationType == NETWORK_ROAMING))
            {
                return 0;
            }
        }
        delay(1000);
    }

    return MODEM_ERROR;
}

// **********************************************************************************************************
// waitForOK - Sends a generic command to the modem to see if it responds appropriately with "OK"
// **********************************************************************************************************

static int8_t waitForOK(void)
{
    uint8_t i;
    char modemReply[MAX_REPLY_SIZE];

    // Exit early if initially OK
    if (sendATCommand(AT_COMMAND, sizeof(AT_COMMAND), 
                      AT_RESPONSE, sizeof(AT_RESPONSE), modemReply, sizeof(modemReply), 1000) == 0)
    {
        // Early return when OK is successfully returned
        return 0;
    }

    // System may be in online mode, so send command to exit and return to command mode
    (void)sendATCommand(GOTO_COMMAND_MODE, sizeof(GOTO_COMMAND_MODE),
                  COMMAND_MODE_RESPONSE, sizeof(COMMAND_MODE_RESPONSE), modemReply, sizeof(modemReply), 5000);

    // Continue retries as modem may still be booting up.
    for (i = 0; i < AT_RETRIES; i++)
    {
        // Send echo off command and ignore reply
        (void)sendATCommand(ECHO_OFF_COMMAND, sizeof(ECHO_OFF_COMMAND),
                      AT_RESPONSE, sizeof(AT_RESPONSE), modemReply, sizeof(modemReply), 1000);

        if (sendATCommand(AT_COMMAND, sizeof(AT_COMMAND), 
                          AT_RESPONSE, sizeof(AT_RESPONSE), modemReply, sizeof(modemReply), 1000) == 0)
        {
            // Early return when OK is successfully returned
            return 0;
        }
    }
    return MODEM_ERROR;
}

// **********************************************************************************************************
// waitVerboseErrorMessages - Configures modem for verbose error messages
// **********************************************************************************************************

static int8_t waitVerboseErrorMessages(void)
{
    uint8_t i;
    char modemReply[MAX_REPLY_SIZE];

    for( i = 0; i < AT_RETRIES; i++ )
    {
        if( sendATCommand(VERBOSE_ERROR_COMMAND, sizeof(VERBOSE_ERROR_COMMAND),
            VERBOSE_ERROR_RESPONSE, sizeof(VERBOSE_ERROR_RESPONSE), modemReply, sizeof(modemReply), 1000) == 0 )
        {
            // Early return when OK is successfully returned
            return 0;
        }
    }
    return MODEM_ERROR;
}

// **********************************************************************************************************
// sendGsmActivate - Sends the GSM activate command
// **********************************************************************************************************

static int8_t sendGsmActivate(void)
{
    char modemReply[MAX_REPLY_SIZE];
    
    // This command really only needs to be sent one time when a GSM modem
    // is activated.  It is repeated here in case this step is forgotten or
    // the setting is lost (only applies to GSM)

    if (sendATCommand(GSM_ACTIVATE_COMMAND, sizeof(GSM_ACTIVATE_COMMAND),
       GSM_ACTIVATE_RESPONSE, sizeof(GSM_ACTIVATE_RESPONSE), modemReply, sizeof(modemReply), 1000) == 0)
    {
        return 0;        
    }
    
    return MODEM_ERROR;
}

// **********************************************************************************************************
// getUniqueID - Sends a command to determine the ESN of the modem
// **********************************************************************************************************

static int8_t getUniqueID(char* pID)
{
    char modemReply[MAX_REPLY_SIZE];

    if (sendATCommand(ID_COMMAND, sizeof(ID_COMMAND), 
                      ID_RESPONSE, sizeof(ID_RESPONSE), modemReply, sizeof(modemReply), 1000) == 0)
    {
        uint16_t i;


        // Extract string from reply
        for (i = 0; i < UNIQUE_ID_SIZE; i++)
        {
            // Copy the reply to the unique ID string (offset by 9 due to response formatting)
            if (modemReply[i + 9] != '\r')
            {
                pID[i] = modemReply[i + 9];
            }
            
            // Terminate string and exit when return character is reached
            else
            {               
                pID[i] = '\0';
                return 0;
            }
        }
    }

    // Ensure that we're null terminated even if we ran over the size limit
    pID[UNIQUE_ID_SIZE - 1] = 0;

    return MODEM_ERROR;
}


// **********************************************************************************************************
// getLocation - Sends a command to determine the location of the modem
// **********************************************************************************************************

static sint8 getLocation(char* pLocation)
{
    char modemReply[MAX_REPLY_SIZE];
    AT_response_t locResponse = AT_RESP_NO_RESP;
    uint16_t i;
    uint16_t j;
    sint8 retVal = LOCATION_GET_NONE;

    // Check to see if the socket is already open
    if( sendATCommand(SOCKET_STATUS_COMMAND, sizeof(SOCKET_STATUS_COMMAND),
        SOCKET_STATUS_RESPONSE, sizeof(SOCKET_STATUS_RESPONSE), modemReply, sizeof(modemReply), 1000) != 0 )
    {
        // Close current socket - this will close any active MQTT session
        (void)sendATCommand(CLOSE_SOCKET_COMMAND, sizeof(CLOSE_SOCKET_COMMAND),
                      CLOSE_SOCKET_RESPONSE, sizeof(CLOSE_SOCKET_RESPONSE), modemReply, sizeof(modemReply), 20000);
    }

    locResponse = sendATCommand(LOCATION_COMMAND, sizeof(LOCATION_COMMAND),
        LOCATION_RESPONSE, sizeof(LOCATION_RESPONSE), modemReply, sizeof(modemReply), 5000);
    
    memset(modemReply, 0, sizeof(modemReply));
    memset(pLocation, 0, LOCATION_SIZE);

    if( locResponse == AT_RESP_OK)
    {
        for( i = 0; i < AT_RETRIES && retVal == LOCATION_GET_NONE; i++ )
        {
            delay(300);
            if(getSocketModemData((unsigned char*)modemReply, (LOCATION_SIZE + strlen(LOCATION_HEADER) - 1)))
            {

                if ( (memcmp(modemReply, LOCATION_HEADER, strlen(LOCATION_HEADER)) == 0) &&
                     (memcmp(modemReply, LOCATION_ERROR, strlen(LOCATION_ERROR)) != 0) )
                {
                    // Extract string from reply, saving room for a trailing null
                    for( j = 0; j < (LOCATION_SIZE - 1); j++ )
                    {
                        // Copy the reply to location string
                        if( modemReply[j + LOCATION_OFFSET] != '\r' )
                        {
                            // Consider success to be the reception of at least one valid byte
                            pLocation[j] = modemReply[j + LOCATION_OFFSET];
                            retVal = LOCATION_GET_OK;
                        }

                        // Terminate string and exit when return character is reached
                        else
                        {
                            pLocation[j] = '\0';
                            break;
                        }
                    }

                }
                else if (strstr(modemReply, MODEM_GENERAL_ERROR) != NULL)
                {
                    // Method #1 of detecting fatal location error
                    retVal = LOCATION_GET_ERROR_FATAL;
                    break;
                }
            }
        }
    }
    else if ( locResponse == AT_RESP_ERROR)
    {
        // Method #2 of detecting fatal location error (see above)
        retVal = LOCATION_GET_ERROR_FATAL;
    }

    return retVal;
}

// **********************************************************************************************************
// openData - Sends a command to configure and open a data connection on the cellular network
// **********************************************************************************************************

static int8_t openData(void)
{
    char modemReply[MAX_REPLY_SIZE] = {0};
    sint16 ipBytes[4] = {0};
    uint32 ipAddress = 0;
    sint8 success = 0;

    // Check to see if the data connection is already established
    if (sendATCommand(CHECK_OPEN_DATA_COMMAND, sizeof(CHECK_OPEN_DATA_COMMAND), 
                      CHECK_OPEN_DATA_RESPONSE, sizeof(CHECK_OPEN_DATA_RESPONSE), modemReply, sizeof(modemReply), 1000) != 0)
    {

        // Configure socket connection
        if (sendATCommand(CONFIGURE_SOCKET_COMMAND, sizeof(CONFIGURE_SOCKET_COMMAND), 
                          CONFIGURE_SOCKET_RESPONSE, sizeof(CONFIGURE_SOCKET_RESPONSE), 
                          modemReply, sizeof(modemReply), 1000) != 0)
        {
            return MODEM_ERROR;
        }

        // Open data session
        if (sendATCommand(OPEN_DATA_COMMAND, sizeof(OPEN_DATA_COMMAND),
                          OPEN_DATA_RESPONSE, sizeof(OPEN_DATA_RESPONSE), modemReply, sizeof(modemReply), 20000) != 0)
        {
            return MODEM_ERROR;
        }
        else
        {
            success = MODEM_sscanf(modemReply, "\r\n#SGACT: \"%u.%u.%u.%u\"\r\n", &ipBytes[3], &ipBytes[2], &ipBytes[1], &ipBytes[0]);
            
            // Sometimes seems to take on a slightly different format for some reason, so check that too
            if (success != sizeof(ipBytes)/sizeof(ipBytes[0]))
            {
                success = MODEM_sscanf(modemReply, "\r\n#SGACT: %u.%u.%u.%u\r\n", &ipBytes[3], &ipBytes[2], &ipBytes[1], &ipBytes[0]);            
            }

            if (success == sizeof(ipBytes)/sizeof(ipBytes[0]))
            {
                ipAddress |= (uint32)ipBytes[3] << 24;  //lint !e571
                ipAddress |= (uint32)ipBytes[2] << 16;  //lint !e571
                ipAddress |= (uint32)ipBytes[1] << 8;   //lint !e571
                ipAddress |= (uint32)ipBytes[0] << 0;   //lint !e571
            }
            
            (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, IpAddress), (DistVarType)ipAddress);
        }
    }

    return 0;
}

// **********************************************************************************************************
// openSocket - Sends a command to open a socket connection with the server
// **********************************************************************************************************

static int8_t openSocket(void)
{
    char modemReply[MAX_REPLY_SIZE];

    // Check to see if the socket is already open
    if (sendATCommand(SOCKET_STATUS_COMMAND, sizeof(SOCKET_STATUS_COMMAND), 
                      SOCKET_STATUS_RESPONSE, sizeof(SOCKET_STATUS_RESPONSE), modemReply, sizeof(modemReply), 1000) != 0)
    {
        // Close current socket - this will close any active MQTT session
        (void)sendATCommand(CLOSE_SOCKET_COMMAND, sizeof(CLOSE_SOCKET_COMMAND),
                      CLOSE_SOCKET_RESPONSE, sizeof(CLOSE_SOCKET_RESPONSE), modemReply, sizeof(modemReply), 20000);
    }
    else
    {
        // See if we respond to an AT
        if (sendATCommand(AT_COMMAND, sizeof(AT_COMMAND),
                      AT_RESPONSE, sizeof(AT_RESPONSE), modemReply, sizeof(modemReply), 1000) != 0)
        {
            // We're probably in data mode, so get back to command mode
            (void)sendATCommand(GOTO_COMMAND_MODE, sizeof(GOTO_COMMAND_MODE),
                      COMMAND_MODE_RESPONSE, sizeof(COMMAND_MODE_RESPONSE), modemReply, sizeof(modemReply), 5000);
        }
        
        // Try closing the socket again
        (void)sendATCommand(CLOSE_SOCKET_COMMAND, sizeof(CLOSE_SOCKET_COMMAND),
                  CLOSE_SOCKET_RESPONSE, sizeof(CLOSE_SOCKET_RESPONSE), modemReply, sizeof(modemReply), 20000);
    }

    // Open socket
    if (PUBLISH_HaveUserPass())
    {
        if (sendATCommand(OPEN_MQTT_SOCKET_COMMAND, sizeof(OPEN_MQTT_SOCKET_COMMAND),
                          OPEN_SOCKET_RESPONSE, sizeof(OPEN_SOCKET_RESPONSE), modemReply, sizeof(modemReply), 20000) != 0)
        {
            return MODEM_ERROR;
        }
    }
    else
    {
        if (sendATCommand(OPEN_HTTP_SOCKET_COMMAND, sizeof(OPEN_HTTP_SOCKET_COMMAND),
                          OPEN_SOCKET_RESPONSE, sizeof(OPEN_SOCKET_RESPONSE), modemReply, sizeof(modemReply), 20000) != 0)
        {
            return MODEM_ERROR;
        }
    }
    return 0;
}

// **********************************************************************************************************
// sendString - Sends a string to the modem
// **********************************************************************************************************

static void sendString(char* pData, uint16_t size)
{
    // Strip null character from the end of the strings
    (void)MODEM_SendString((uint8*)pData, size - 1);
}

// **********************************************************************************************************
// sendATCommand - Sends a command to the modem and waits for a response
// **********************************************************************************************************

static AT_response_t sendATCommand(char* pCmd, uint16_t cmdSize, char* pExpectedRsp,
                            uint16_t rspSize, char* pActualRsp, uint16 rspBufLen, uint16_t timeout)
{
    uint8 status;
    sint8 retVal = AT_RESP_NO_RESP;
    uint16 ticks = TICKS_MS(timeout);
    sint16 byteCount = EOF;
    uint8 bufTemp = 0;

    // Purge the receive buffer
    while (getSocketModemData(&bufTemp, sizeof(bufTemp)));
    (void)K_Event_Reset(gModemTaskID, EVT_MODEM_BYTES_RECEIVED);

    // Send the command
    sendString(pCmd, cmdSize);

    // Wait for data back from the modem
    status = K_Event_Wait(EVT_MODEM_BYTES_RECEIVED, ticks, RTOS_CLEAR_EVENT_FLAGS_AFTER);

    if (status != EVT_MODEM_BYTES_RECEIVED)
    {
        // Timed out
        retVal = AT_RESP_TIMEOUT;
    }
    else
    {
        // Delay to allow full response (sometimes the modem is slow?)
        delay(40);
        
        byteCount = getSocketModemData((uint8*)pActualRsp, rspBufLen);
        pActualRsp[byteCount] = 0;

        // Make sure that the start of the response looks correct
        if (memcmp(pActualRsp, pExpectedRsp, rspSize - 1) == 0)
        {
            retVal = AT_RESP_OK;
        }
        else if (memcmp(pActualRsp, MODEM_GENERAL_ERROR, sizeof(MODEM_GENERAL_ERROR) - 1) == 0)
        {
            retVal = AT_RESP_ERROR;
        }
        else
        {
            retVal = AT_RESP_WRONG_REPLY;
        }
    }

    // If we've received a correct response even once, then it's safe to say a modem is present
    if (retVal == AT_RESP_OK && !gRun.ModemFound)
    {
        (void)DVAR_SetPointLocal(DVA17G721_SS(gRun, ModemFound), (DistVarType)TRUE);
    }

    return retVal;
}

// **********************************************************************************************************
// GetId - Returns the last value of the unique identifier as read from the modem
// **********************************************************************************************************

char* GetId(void)
{
    return gUniqueID;
}

// **********************************************************************************************************
// GetLocation - Returns the last value of the location as read from the modem
// **********************************************************************************************************

char* GetLocation(void)
{
    return gLocation;
}

// **********************************************************************************************************
// GetId - Returns the last value of the signal strength as read from the modem
// **********************************************************************************************************

uint8_t GetSignalStrength(void)
{
    return gSignalStrength;
}

// **********************************************************************************************************
// getModemReply - Retrieves a modem reply from the receive buffer
// **********************************************************************************************************

int getSocketModemData(uint8* pReply, int maxCount)
{
    uint16_t i = 0;
    uint8 incomingByte = 0;

    while ( (i < MAX_REPLY_SIZE) && (i < maxCount) && (MODEM_GetByte(&incomingByte) == TRUE) )
    {
        pReply[i] = incomingByte;
        i++;
    }

    return i;
}

// **********************************************************************************************************
// Reimplementation of sscanf, since the libc version included with XC32 v1.22 is known to be buggy (and will hang
// at runtime when used here). Limited to %u formatting.
// **********************************************************************************************************

static int MODEM_sscanf(const char* str, const char* format, ...)
{
    uint16 format_idx = 0;
    uint16 str_idx = 0;
    uint16 matchCount = 0;
    uint16* pArg = NULL;
    
    va_list args;
    va_start(args, format);

    while (format[format_idx] != '\0' && str[str_idx] != '\0')
    {
        if (format[format_idx] == '%' && format[format_idx+1] == 'u')
        {
            uint16 runningTotal = 0;
            
            // Abort if not a digit
            if (str[str_idx] < '0' || str[str_idx] > '9')
            {
                break;
            }

            // Convert digits to a value
            while (str[str_idx] >= '0' && str[str_idx] <= '9')
            {
                runningTotal *= 10; // shift left
                runningTotal += str[str_idx] - '0';
                str_idx++;
            }

            // Store in the va_args
            pArg = va_arg(args, uint16*); //lint !e64 -esym(628, *va_arg*)
            *pArg = runningTotal;
            matchCount++;
            format_idx++;
        }
        else if (format[format_idx] == str[str_idx])
        {
            str_idx++;
        }
        else
        {
            break;
        }
        format_idx++;
    }

    va_end(args);

    return matchCount;
}
