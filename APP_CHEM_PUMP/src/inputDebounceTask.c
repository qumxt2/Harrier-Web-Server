// inputDebounceTask.c

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The inputDebounceTask is used to implement debouncing of digital inputs

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"
#include "rtos.h"
#include "debug_app.h"
#include "in_digital.h"
#include "io_pin.h"
#include "inputDebounceTask.h"
#include "uC_peripheral_map.h"

// ******************************************************************************************
// CONSTANTS AND MACROS
// ******************************************************************************************
#define INPUT_DEBOUNCE_TIC_EVENT_FLAG           (RTOS_EVENT_FLAG_1)
#define INPUT_DEBOUNCE_TASK_FREQ_HZ             (200)
#define START_IMMEDIATELY                       (0x0001)
#define ALWAYS_WAIT_FOR_MATCH                   (0x0000)
#define DEBOUNCE_THRESHOLD                      (4u)

// ******************************************************************************************
// LOCAL TYPES
// ******************************************************************************************


// ******************************************************************************************
// STATIC VARIABLES
// ******************************************************************************************
static uint8 gInputDebounceTaskID;
static uint8 gInputDebounceTaskTimerID;
static debounce_t gInput[DB_NUMBER_INPUTS] = {{0}};

// ******************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// ******************************************************************************************
void InputDebounceTask (void);
static void initializeInputs(void);
static void debounce(void);

// ******************************************************************************************
// PUBLIC FUNCTIONS
// ******************************************************************************************

uint8 inputDebounceTaskInit (void)
{
    uint8 status = INPUT_DEBOUNCE_TASK_INIT_OK;

    initializeInputs();

    // Reserve a task timer
    gInputDebounceTaskTimerID = RtosTimerReserveID();
    if (gInputDebounceTaskTimerID == RTOS_INVALID_ID)
    {
        status = INPUT_DEBOUNCE_TASK_INIT_ERROR;
    }

    // Reserve a task ID and queue the task
    gInputDebounceTaskID = RtosTaskCreateStart(InputDebounceTask);
    if (gInputDebounceTaskID == RTOS_INVALID_ID)
    {
        status = INPUT_DEBOUNCE_TASK_INIT_ERROR;
    }
    
    if (status == INPUT_DEBOUNCE_TASK_INIT_ERROR)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "RTC TASK INIT ERROR\r\n");
    }
    return status;
}

IOrtn_digital_t DB_DigitalStateGet(db_input_t input)
{
    IOrtn_digital_t retVal;

    if (input < DB_NUMBER_INPUTS)
    {
        retVal.state = gInput[input].state;
        retVal.error = IOError_Ok;
    }
    else
    {
        retVal.state = NOT_ASSERTED;
        retVal.error = IOError_OutOfRange;
    }
    return retVal;
}

// Allow a callback to be registered for when an input settles on a new state. Use a null callback
// pointer to deregister.
// Returns TRUE on success, FALSE otherwise.
bool DB_RegisterCallback(db_input_t input, db_callback_t callback)
{
    bool retVal = FALSE;

    if (input < DB_NUMBER_INPUTS)
    {
        gInput[input].callback = callback;
        retVal = TRUE;
    }

    return retVal;
}

// ******************************************************************************************
// PRIVATE FUNCTIONS
// ******************************************************************************************

void InputDebounceTask (void)
{
    static uint8 status;

    // Create and start the timer
    status = K_Timer_Create(gInputDebounceTaskTimerID,RTOS_NOTIFY_SPECIFIC,gInputDebounceTaskID,INPUT_DEBOUNCE_TIC_EVENT_FLAG);
    status |= K_Timer_Start(gInputDebounceTaskTimerID,START_IMMEDIATELY,(RtosGetTickFreq()/INPUT_DEBOUNCE_TASK_FREQ_HZ));

    if (status != K_OK)
    {
        DEBUG_PRINT_STRING(DBUG_ALWAYS, "DEVELOPMENT TASK TIMER ERROR\r\n");
    }

    // Enter the cyclic portion of the task.
    for (;;)
    {
        status = K_Event_Wait(INPUT_DEBOUNCE_TIC_EVENT_FLAG,ALWAYS_WAIT_FOR_MATCH,RTOS_CLEAR_EVENT_FLAGS_AFTER);
        debounce();
//        LAT_RELAY_WATCHDOG ^= 1;
        ClrWdt();
    }
}

static void initializeInputs(void)
{
    uint8 i;

    for (i = 0; i < DB_NUMBER_INPUTS; i++)
    {
        gInput[i].count = 0;

        // Inputs are pulled up by default
        gInput[i].state = ASSERTED;
    }
    gInput[DB_INPUT_1].pin = IOPIN_INPUT_1;
    gInput[DB_INPUT_2].pin = IOPIN_INPUT_2;
    gInput[DB_INPUT_3].pin = IOPIN_INPUT_3;
    gInput[DB_INPUT_4].pin = IOPIN_INPUT_4;
}

static void debounce(void)
{
    uint8 i;

    for (i = 0; i < DB_NUMBER_INPUTS; i++)
    {
        const IOrtn_digital_t currentValue = IN_Digital_State_Get(gInput[i].pin);

        if (currentValue.error == IOError_Ok)
        {
           if (gInput[i].state != currentValue.state)
            {
                gInput[i].count++;

                if (gInput[i].count > DEBOUNCE_THRESHOLD)
                {
                    gInput[i].state = currentValue.state;

                    if (gInput[i].callback)
                    {
                        gInput[i].callback(i, gInput[i].state);
                    }
                }
            }
            else
            {
                gInput[i].count = 0;
            }
        }
    }
}
