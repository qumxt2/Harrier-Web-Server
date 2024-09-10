// mbtask.c

// Copyright 2013
// Graco Inc., Minneapolis, MN
// All Rights Reserved

//
// This system task is modbus task operations 
//

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************
#include "typedef.h"
#include "rtos.h"
#include "mb.h"
#include "debug.h"
#include "NetworkScreen.h"
#include "dvseg_17G721_setup.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************
#define APP_FREQ_APP                        (100U)
#define MODBUS_TIMER_PERIOD 				(RtosGetTickFreq()/APP_FREQ_APP)
#define EVENT(x) 							((uint8)0b1 << x)
#define RTOS_WAIT(ms) 						(((uint32)RtosGetTickFreq() * ms) / 1000)
#define RTOS_WAIT_FOREVER 					(0)


// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************
typedef enum Events
{
    EVENT_TIMER = 0,
    EVENT_COUNT
} Events;


// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************
static uint8 modbus_TimerID = RTOS_INVALID_ID;
static uint8 modbus_TaskID = RTOS_INVALID_ID;
static uint8 i;


// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************
void MODBUS_Task( void );


// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************

sint8 MODBUS_Task_Initialize( void )
{
    sint8 rVal = 0;

    modbus_TimerID = RtosTimerReserveID( );
    if( modbus_TimerID == RTOS_INVALID_ID )
    {
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "* MODBUS Task Init..." );
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "\r\x1b[74C[\x1b[31mFAIL\x1b[0m]\n" );
        rVal = -1;
    }

    modbus_TaskID = RtosTaskCreate( MODBUS_Task );
    if( modbus_TaskID == RTOS_INVALID_ID )
    {
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "* MODBUS Task Init..." );
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "\r\x1b[74C[\x1b[31mFAIL\x1b[0m]\n" );
        rVal = -1;
    }

    return rVal;
}

sint8 MODBUS_Task_Start( void )
{
    sint8 rVal = 0;

    if( K_Task_Start( modbus_TaskID ) != K_OK )
    {
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "* MODBUS Task Start..." );
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "\r\x1b[74C[\x1b[31mFAIL\x1b[0m]\n" );
        rVal = -1;
    }

    return rVal;
}

// *****************************************************************************
// * PRIVATE FUNCTIONS
// *****************************************************************************

void MODBUS_Task( void )
{
    sint8 events;

    events = K_Timer_Create( modbus_TimerID,
                             RTOS_NOTIFY_SPECIFIC,
                             modbus_TaskID,
                             EVENT( (int)EVENT_TIMER ) );

    events |= K_Timer_Start( modbus_TimerID,
                             MODBUS_TIMER_PERIOD,
                             MODBUS_TIMER_PERIOD );

    if( events != K_OK )
    {
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "* MODBUS Task Start..." );
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "\r\x1b[74C[\x1b[31mFAIL\x1b[0m]\n" );
    }
    else
    {
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "* MODBUS Task Start..." );
        DEBUG_PRINT_STRING( DBUG_ALWAYS, "\r\x1b[74C[ \x1b[32mOK\x1b[0m ]\n" );
    }

    for( ; ; )
    {
        {
            events = K_Event_Wait( EVENT( (int)EVENT_TIMER ),
                                   RTOS_WAIT_FOREVER,
                                   RTOS_CLEAR_EVENT_FLAGS_AFTER );

            for( i = 0; i < (int)EVENT_COUNT; i++ )
            {
                switch ( events & ((uint8)0b1 << i) )
                {
                    case EVENT( (int)EVENT_TIMER ):
                        if (gSetup.NetworkMode == NETWORK_MODE_MODBUS)
                        {

                            (void) eMBPoll(  );
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
