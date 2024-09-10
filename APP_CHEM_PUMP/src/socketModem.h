// socketModem.h

// Copyright 2014
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// The header file for communication to the modem

#ifndef SOCKET_MODEM_H
#define SOCKET_MODEM_H

#include "typedef.h"
#include "stdint.h"

// **********************************************************************************************************
// Constants
// **********************************************************************************************************

// Disable SSL/TLS connections to the MQTT broker
//#define FORCE_INSECURE

#ifdef __DEBUG
#define HOST_FQDN                   "harrier-dev.graco.com"
#else
#ifndef FORCE_INSECURE
#define HOST_FQDN                   "harrier.graco.com"
#else
#define HOST_FQDN                   "graco-vpn.graniteriver.com"
#endif
#endif

// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************

typedef enum
{
    CONNECTION_SUCCESS = 0,
    CONNECTION_IN_PROGRESS,
    CONNECTION_AT_ERROR,
    CONNECTION_CONFIG_VERBOSITY_ERROR,
    CONNECTION_SIGNAL_STRENGTH_ERROR,
    CONNECTION_UNIQUE_ID_ERROR,
    CONNECTION_REGISTRATION_ERROR,
    CONNECTION_OPEN_DATA_ERROR,
    CONNECTION_OPEN_SOCKET_ERROR,
    CONNECTION_LOCATION_ERROR,
    CONNECTION_SSL_ERROR,
    CONNECTION___ENUM_SIZE
} CONNECTION_STATUS_t;

extern const char* const ConnectionStatusText[CONNECTION___ENUM_SIZE];

// **********************************************************************************************************
// Public functions
// **********************************************************************************************************

CONNECTION_STATUS_t InitializeSocketModem(void);
char* GetId(void);
char* GetLocation(void);
uint8_t GetSignalStrength(void);
CONNECTION_STATUS_t GetConnectionStatus(void);
int getSocketModemData(uint8* pReply, int maxCount);

#endif
