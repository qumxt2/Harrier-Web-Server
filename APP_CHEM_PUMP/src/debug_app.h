// debug.h

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

//
// Add the DBUG flags to the debug.h file
//

#ifndef DEBUG_APP_H
#define DEBUG_APP_H

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "debug.h"

// *****************************************************************************
// * MARCROS
// *****************************************************************************

#define DBUG_COMMAND                DBUG_APP_CATEGORY(0) // 0x0080
#define DBUG_PUMP                   DBUG_APP_CATEGORY(1)
#define DBUG_TANK                   DBUG_APP_CATEGORY(2) // 0x0300
#define DBUG_ADC                    DBUG_APP_CATEGORY(3)
#define DBUG_MV                     DBUG_APP_CATEGORY(4)
#define DBUG_4                      DBUG_APP_CATEGORY(5)
#define DBUG_6                      DBUG_APP_CATEGORY(6)
#define DBUG_7                      DBUG_APP_CATEGORY(7)
#define DBUG_8                      DBUG_APP_CATEGORY(8)

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTIONS PROTOTYPES
// *****************************************************************************

#endif // DEBUG_APP_H

