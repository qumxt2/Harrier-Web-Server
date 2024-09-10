// dvseg_17G721_usage.c

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains the usage dvars used to periodically back up system information to eeprom

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "dvseg_17G721_usage.h"

#include "string.h"

#include "ee_helper.h"
#include "ee_map.h"
#include "assert_app.h"
#include "pumpControlTask.h"
#include "screensTask.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************

#define DATASTORE_ADDR(x)      (EE_gUsage_BASE + (x) * GUSAGE_DATASTORE_SIZE)

#define P_U8(x,y)  ((uint8*)P_U32(x,y)) //uint8 pointer to element # y in struct x
#define P_U32(x,y) (&E_U32(x,y))        //uint32 pointer to element # y in struct x
#define E_U32(x,y) (((uint32*)&x)[y])   //derefernced element # y in struct x

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

static DistVarType  Callback ( DistVarID id, DistVarType oldVal, DistVarType newVal );
static void         SaveRestoreValues ( bool saveval );

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

DVS17G721_gUsage_t  gUsage = {};

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************

static DVS17G721_gUsage_t mUsage = {}; //copy of what is currently in EEPROM to prevent unnecessary writes

// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************

bool DVSEG_17G721_USAGE_Initialize (bool resetDefaults)
{
    bool rVal = FALSE;

    // Initialize to standby mode & no alarms in case defaults are restored, to prevent random values from being saved to EEprom.
    gUsage.PumpStatusBackup = PUMP_STATUS_Standby;

    memset(&mUsage, 0xFF, sizeof(mUsage));
    SaveRestoreValues(resetDefaults);
    
	rVal |= (bool)!!DVAR_RegisterSegment( DVA17G721_gUsage_BASE,    // base address
                                          GUSAGE_DATASTORE_COUNT, 	// length
                                          VARIABLE_FLAVOR_RAM,		// flavor
                                          (DistVarType*)&gUsage,	// RAM location
                                          0 );                      // eeprom offset
	assert(!rVal);

    rVal |= (bool)!!DVAR_RegisterOwnership( DVA17G721_gUsage_BASE,	// base address
                                            GUSAGE_DATASTORE_COUNT,	// length
                                            Callback,               // calback function
                                            0 );					//lint !e655 bit-wise operation ok // Auto-broadcast period
    assert(!rVal);

    return rVal;
}

void DVSEG_17G721_USAGE_SaveValues ( void )
{
    SaveRestoreValues(TRUE);
}

// *****************************************************************************
// * PRIVATE FUNCTIONS
// *****************************************************************************

static DistVarType Callback( DistVarID id, DistVarType oldVal, DistVarType newVal )
{
    /*
    LOCK_DVCB_Entry();
    {
        
    }
    LOCK_DVCB_Exit();
    */
    return oldVal;
}

static void SaveRestoreValues ( bool saveval )
{
    static bool doOnce = TRUE;
    uint16 i;

    for (i = 0; i < GUSAGE_DATASTORE_COUNT; i++)
    {
        if (saveval)
        {
            if (E_U32(gUsage,i) != E_U32(mUsage,i))
            {
                //copy global to static
                E_U32(mUsage,i) = E_U32(gUsage,i);
                //save static
                if (ev_set_datastore(
                        DATASTORE_ADDR(i), 
                        GUSAGE_DATASTORE_DEPTH, 
                        P_U8(mUsage,i),
                        sizeof(DistVarType)))
                { //some parameter must be incorrect
                    if(doOnce)
                    {
                        doOnce = FALSE;
                        //EVENT_HELPER_SetSoftwareError();
                    }
                }
            }
        }
        else
        {
            //restore static
            if(ev_get_datastore(
                    DATASTORE_ADDR(i),
                    GUSAGE_DATASTORE_DEPTH, 
                    P_U8(mUsage,i),
                    sizeof(DistVarType)))
            { //fails from data not existing, or parameters wrong, caught above
                if(doOnce)
                {
                    doOnce = FALSE;
                    //EVENT_HELPER_SetSoftwareError();
                }
            }
            else
            {
                //copy static to global
                E_U32(gUsage,i) = E_U32(mUsage,i);
            }
        }
    }   
}
