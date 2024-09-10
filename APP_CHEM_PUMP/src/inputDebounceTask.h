// inputDebounceTask.h

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Input Debounce
// Task.   


#ifndef INPUT_DEBUOUNCE_TASK_H
#define INPUT_DEBUOUNCE_TASK_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"                                // Compiler specific type definitions
#include "io_typedef.h"
#include "in_digital.h"


// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
#define INPUT_DEBOUNCE_TASK_INIT_OK             (0x00)      // Initialization successful
#define INPUT_DEBOUNCE_TASK_INIT_ERROR          (0xFF)      // Initialization error
#define INPUT_DEBOUNCE_TIMER_INVALID            (0xFF)


typedef enum
{
    DB_INPUT_1 = 0,
    DB_INPUT_2,
    DB_INPUT_3,
    DB_INPUT_4,
    DB_NUMBER_INPUTS
} db_input_t;

// Prototype for callbacks for when an input settles on a new state
typedef void (*db_callback_t)( db_input_t inputId, bool state );

typedef struct
{
    IOPin_t                 pin;
    IOState_t               state;
    uint8                   count;
    db_callback_t           callback;
} debounce_t;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

uint8 inputDebounceTaskInit (void);
IOrtn_digital_t DB_DigitalStateGet(db_input_t input);
bool DB_RegisterCallback(db_input_t input, db_callback_t callback);

//*******************************************************************************************


#endif
