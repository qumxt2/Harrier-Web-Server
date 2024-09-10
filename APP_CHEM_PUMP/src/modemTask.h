// modemTask.h

// Copyright 2015 - 2016
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the modem task

#ifndef MODEM_TASK_H
#define MODEM_TASK_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"                                // Compiler specific type definitions

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
#define MODEM_TASK_INIT_OK                    0x00
#define MODEM_TASK_INIT_ERROR                 0xFF

// Events
#define EVT_MODEM_BYTES_RECEIVED                (RTOS_EVENT_FLAG_1)
#define EVT_MODEM_HEARTBEAT                     (RTOS_EVENT_FLAG_2)

#define ONE_SHOT                				(0x0000)

// Based on the WolfSSL error enum
enum ModemErrorsEnum {
    MODEM_ERR_GENERAL    = -1,     /* general unexpected err */
    MODEM_ERR_WANT_READ  = -2,     /* need to call read  again */
    MODEM_ERR_WANT_WRITE = -2,     /* need to call write again */
    MODEM_ERR_CONN_RST   = -3,     /* connection reset */
    MODEM_ERR_ISR        = -4,     /* interrupt */
    MODEM_ERR_CONN_CLOSE = -5,     /* connection closed or epipe */
    MODEM_ERR_TIMEOUT    = -6      /* socket timeout */
};

// ******************************************************************************************
// USEFUL MACROS
// ******************************************************************************************

// ******************************************************************************************
// PUBLIC VARIABLES
// ******************************************************************************************

extern uint8 gModemTaskID;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

uint8 modemTaskInit (void);
void ModemTask (void);
void MODEM_SendStringTerminal(uint8* pData, uint16 len);
sint16 MODEM_SendString(uint8* pData, uint16 len);
bool MODEM_GetByteTerminal(uint8* pByte);
bool MODEM_GetByte(uint8* pByte);
void MODEM_DebugEcho(bool echoOn);
void MODEM_Snoop(bool snoopOn);
bool MODEM_Connected(void);
void MODEM_HardwareReset(void);
void MODEM_configureUart(void);

//*******************************************************************************************


#endif //MODEM_TASK_H
