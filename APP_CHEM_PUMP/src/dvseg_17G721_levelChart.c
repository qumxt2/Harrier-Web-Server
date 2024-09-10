// dvseg_17G721_levelChart.c

// Copyright 2016 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains the level chart dvars used for the tank strap chart

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************
#include "assert.h"
#include "dvcb_helper.h"
#include "ee_map.h"
#include "dvinterface_17G721.h"
#include "TankScreen.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************
#define IDX(element, index)                     (DVA17G721_SA(gLevelChart,element,index) - DVA17G721_gLevelChart_BASE)
#define MAX_VOLUME_ENTRY                        (INVALID_CHART_ENTRY_VOL_G * 10)          // Entries are stored in dvars as US units x10
#define MAX_LEVEL_ENTRY                         (INVALID_CHART_ENTRY_LVL_IN * 10)         // Entries are stored in dvars as US units x10

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

DVS17G721_gLevelChart_t gLevelChart;

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************
static DistVarType Callback( DistVarID id, DistVarType oldVal, DistVarType newVal );

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************
//DVCB_Helper callback definition arrays; indexed by offset within segment
static const DVCB_definition_t sDvcbLevelChart[sizeof (DVS17G721_gLevelChart_t) / sizeof(DistVarType)] = {
    [IDX( TankLevel, 0 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },      // max entry (999.98" (99998 * 10)) is > than the invalid entry that's not x10

    [IDX( TankLevel, 1 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 2 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 3 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 4 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 5 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 6 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 7 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 8 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 9 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 10 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 11 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 12 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 13 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 14 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 15 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 16 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 17 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 18 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankLevel, 19 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_LEVEL_ENTRY, INVALID_CHART_ENTRY_DVAR_LVL },

    [IDX( TankVolume, 0 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },      // max entry (999.98G (99998* 10)) is > than the invalid entry that's not x10

    [IDX( TankVolume, 1 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 2 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 3 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 4 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 5 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 6 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 7 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 8 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 9 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 10 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 11 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 12 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 13 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 14 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 15 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 16 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 17 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 18 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },

    [IDX( TankVolume, 19 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, MAX_VOLUME_ENTRY, INVALID_CHART_ENTRY_DVAR_VOL },
};

static const DVCB_segmentSet_t sDvcbLevelChartSegment = {
    .baseAddr     = DVA17G721_gLevelChart_BASE,
    .segmentLen   = DVA17G721_gLevelChart_LEN,
    .segmentCount = 1,
    .dvcbArray    = sDvcbLevelChart,
    .dvarCount    = sizeof (DVS17G721_gLevelChart_t) / sizeof(DistVarType),
};

// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************
sint8 DVSEG_17G721_LevelChart_Initialize( bool resetDefaults )
{
    DVarErrorCode dvarError = DVarError_Ok;
    sint8 rVal = 0;

    // Registering the segment will take care of restoring the EEPROM value to RAM
	dvarError = DVAR_RegisterSegment( DVA17G721_gLevelChart_BASE,									// base address
									  sizeof (DVS17G721_gLevelChart_t) / sizeof (DistVarType),		// length
									  VARIABLE_FLAVOR_SHADOWED_EEPROM,								// flavor
									  (DistVarType*)&gLevelChart,									// RAM location
									  EE_gLevelChart_BASE );                                        // eeprom offset

	dvarError = DVAR_RegisterOwnership( DVA17G721_gLevelChart_BASE,                                 // base address
										sizeof (DVS17G721_gLevelChart_t) / sizeof (DistVarType),	// length
										Callback,                                                   // calback function
										0 );

    assert( !dvarError );

    if ( resetDefaults )
    {
        dvarError = (DVarErrorCode)DVCB_HELPER_SetDefaults( &sDvcbLevelChartSegment );
    }
    else
    {
        dvarError = (DVarErrorCode)DVCB_HELPER_Validate( &sDvcbLevelChartSegment );
    }

    assert( !dvarError );

    if( dvarError )	
    {
        rVal = -1;
    }

    return rVal;
}

DistVarType DVSEG_17G721_LevelChart_GetDvar( DistVarID dvar_id )
{
    DistVarType rtnval = 0;

    if( dvar_id >= DVA17G721_gLevelChart_BASE )
    {
        dvar_id = dvar_id - DVA17G721_gLevelChart_BASE;

        if( dvar_id < (sizeof (gLevelChart) / sizeof (DistVarType)) )
        {
            rtnval = *((DistVarType*) & gLevelChart + dvar_id);
        }
    }

    return rtnval;
}

// *****************************************************************************
// * PRIVATE FUNCIONS
// *****************************************************************************
static DistVarType Callback( DistVarID id, DistVarType oldVal, DistVarType newVal )
{
    DistVarType rVal = 0;

    //route all DVar callback traffic through DVCB_Helper
    if ( DVCB_HELPER_Callback( &sDvcbLevelChartSegment, id, oldVal, newVal, &rVal ) )
    {
        return 0xffffffff;
    }

    return rVal;
}


