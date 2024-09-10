// ADM_Bootload_Host_Primary.c

// Copyright 2006-12
// Graco Inc., Minneapolis, MN
// All Rights Reserved

#include "typedef.h"
#include "dvar.h"
#include "debug.h"
#include "rtos.h"
#include "icbp.h"
#include "token.h"
#include "bootload_host.h"

#include "TCBL_update_defs.h"
#include "TCBL_token_load.h"
#include "CANBL_can_host.h"

#include "p32mx795f512l.h"

// ***************************************************
// * MACROS
// ***************************************************

// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************

// ***************************************************
// * STATIC VARIABLES
// ***************************************************

static uint8 tokenBootload_TaskID;

// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************

// ***************************************************
// * CONSTANTS
// ***************************************************

// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************

bool TokenBootloadTask_Initialize( void )
{
    // Drive the "token present" line low
    _TRISF13 = 0;
    _LATF13 = 0;

    // CS
    _LATF12 = 0;
    _TRISF12 = 0;

    // Clock
    _LATG6 = 0;
    _TRISG6 = 0;

    // SDI
    _LATG7 = 0;
    _TRISG7 = 0;

    // Set the token present line to an input
    _TRISF13 = 1;

    TCBL_LatchTokenState();
    
	tokenBootload_TaskID = RtosTaskCreateStart( TokenBootloadTask );

	if( tokenBootload_TaskID == RTOS_INVALID_ID )
	{
		return FALSE;
	}
    
	return TRUE;
}

void TokenBootloadTask( void )
{
    (void)CANBL_HoldModulesInBootloadMode( 750, FALSE );

    // Initialize token load
    TCBL_TokenLoadInit();

    // Initialize token update definitions
    TCBL_RegisterUpdateDefs();

	// Check to see if a token contains an update to be broadcast out
	CANBL_CheckForUpdates( );

	// We're done bootloading
	K_Task_End();
}

// ***************************************************
// * PRIVATE FUCTIONS
// ***************************************************
