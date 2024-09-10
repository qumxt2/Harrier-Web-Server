// dvseg_17G721_run.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains the run dvars used to store system runtime information

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "eeprom.h"
#include "rtos.h"
#include "assert.h"
#include "dvcb_helper.h"
#include "ee_map.h"
#include "dvinterface_17G721.h"
#include "debug.h"
#include "securekey.h"
#include "pumpControlTask.h"
#include "TankScreen.h"
#include "FlowScreen.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************

#define IDX(element) (DVA17G721_SS(gRun,element) - DVA17G721_gRun_BASE)

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

 DVS17G721_gRun_t gRun;

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

static DistVarType Callback( DistVarID id, DistVarType oldVal, DistVarType newVal );
static DistVarType pumpStatusCallback( DistVarID id, DistVarType oldVal, DistVarType newVal, DistVarType defVal );

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************

//DVCB_Helper callback definition arrays; indexed by offset within segment
static const DVCB_definition_t sDvcbRun[sizeof (DVS17G721_gRun_t) / sizeof(DistVarType)] = {
    [IDX( Pressure_1_Psi )] =
    { NULL, DVCB_CHECK_COERCE, 0, 14000, 0 },

    [IDX( BatteryMillivolts )] =
    { NULL, DVCB_CHECK_COERCE, 0, 50000, 0 },

    [IDX( PumpStatus )] =
    { pumpStatusCallback, DVCB_CHECK_COERCE, 0, 999, 0 },

    [IDX( ConnectionStatus )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },

    [IDX( IpAddress )] =
    { NULL, DVCB_CHECK_NEW, 0, 0xFFFFFFFF, 0 },

    [IDX( CycleProgress )] =
    { NULL, DVCB_CHECK_NEW, 0, 100, 0 },

    [IDX( Total )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },

    [IDX( GrandTotal )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },

    [IDX( AlarmBitfield)] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },

    [IDX( MqttConnected )] =
    { NULL, DVCB_CHECK_REJECT_DEF, FALSE, TRUE, FALSE },

    [IDX( RemoteDisableActive )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFF, 0 },

    [IDX( ModemFound )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },

    [IDX( TankLevel )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },
    
    [IDX( Pressure_2_Psi )] =
    { NULL, DVCB_CHECK_COERCE, 0, 14000, 0 },    
    
    [IDX( TankHeightCalc )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, TANK_HEIGHT_MAX, 0 },      // 144.00
    
    [IDX( AnalogFlowRate )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_FLOW_RATE_GAL, 0 },
    
    [IDX( AnalogInmV )] =
    { NULL, DVCB_CHECK_COERCE, 0, 2500, 0 },    
    
    [IDX( refPsiInProcess )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },    
    
    [IDX( ModbusHeartbeatTick )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },

    [IDX( TankHeightNotifyTrigger )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, TANK_HEIGHT_MAX, 0 },       // 144.00 inches

    [IDX( TankHeightShutoffTrigger )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, TANK_HEIGHT_MAX, 0 },        // 144.00 inches
    
    [IDX( TestOverride )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },
    
    [IDX( Temperature )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },
    
    [IDX( TempProbeDisableActive )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFF, 0 },
    
    [IDX( MaxMotorCurrentReadingmV)] = 
    { NULL, DVCB_CHECK_COERCE, 0, 5000, 2500},
    
    [IDX( AverageMotorCurrentReadingmV)] = 
    { NULL, DVCB_CHECK_COERCE, 0, 5000, 2500},
    
};

static const DVCB_segmentSet_t sDvcbRunSegment = {
    .baseAddr     = DVA17G721_gRun_BASE,
    .segmentLen   = DVA17G721_gRun_LEN,
    .segmentCount = 1,
    .dvcbArray    = sDvcbRun,
    .dvarCount    = sizeof (DVS17G721_gRun_t) / sizeof(DistVarType),
};


// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************

sint8 DVSEG_17G721_RUN_Initialize( bool resetDefaults )
{
    DVarErrorCode dvarError = DVarError_Ok;
    sint8 rVal = 0;

    dvarError = DVAR_RegisterSegment(
                                      DVA17G721_gRun_BASE,                      // base address
                                      sizeof (DVS17G721_gRun_t) / sizeof (DistVarType), 		// length
                                      VARIABLE_FLAVOR_RAM,		// flavor
                                      (DistVarType*) & gRun,			// RAM location
                                      EEPROM_SIZE );				// invalid address as not EE backed

    dvarError |= DVAR_RegisterOwnership(
                                         DVA17G721_gRun_BASE,			// base address
                                         sizeof (DVS17G721_gRun_t) / sizeof (DistVarType), 		// length
                                         Callback,					// calback function
                                         0 );					// lint !e655 bit-wise operation ok // Auto-broadcast period

    assert( !dvarError );

    if ( resetDefaults )
    {
        dvarError = (DVarErrorCode)DVCB_HELPER_SetDefaults( &sDvcbRunSegment );
    }
    else
    {
        dvarError = (DVarErrorCode)DVCB_HELPER_Validate( &sDvcbRunSegment );
    }

    assert( !dvarError );

    if( dvarError )	
    {
        rVal = -1;
    }
    
    return rVal;
}

DistVarType DVSEG_17G721_RUN_GetDvar( DistVarID dvar_id )
{
    DistVarType rtnval = 0;

    if( (sint32)dvar_id >= DVA17G721_gRun_BASE )
    {
        dvar_id = dvar_id - DVA17G721_gRun_BASE;

        if( dvar_id < (sizeof (gRun) / sizeof (DistVarType)) )
        {
            rtnval = *((DistVarType*) & gRun + dvar_id);
        }
    }

    return rtnval;
}

// *****************************************************************************
// * PRIVATE FUNCTION
// *****************************************************************************

static DistVarType Callback( DistVarID id, DistVarType oldVal, DistVarType newVal )
{
    DistVarType rVal = 0;

    //route all DVar callback traffic through DVCB_Helper
    if ( DVCB_HELPER_Callback( &sDvcbRunSegment, id, oldVal, newVal, &rVal ) )
    {
        return 0xffffffff;
    }

    return rVal;
}

static DistVarType pumpStatusCallback( DistVarID id, DistVarType oldVal, DistVarType newVal, DistVarType defVal )
{
    if( newVal != oldVal )
    {
        if ((newVal <= PUMP_STATUS_Standby) || (newVal > PUMP_STATUS_Lockout_Temperature))
        {
            newVal = PUMP_STATUS_Standby;
        }

        gUsage.PumpStatusBackup = newVal;
        return newVal;
    }
    else
    {
        return oldVal;
    }
}

