// dvcb_helper.c

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "dvcb_helper.h"
#include "cpfuncs.h"
#include "debug.h"
#include "rtos.h"
#include "assert_app.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************

#define SETPOINT_RETRIES (10)
#define SETPOINT_WAIT_TICKS (RtosGetTickFreq()/1000)

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

static bool GenericValidate ( const DVCB_segmentSet_t *pSegmentSet, bool reset );

static bool GenericCallback ( const DVCB_segmentSet_t *pSegmentSet,
                              DistVarID          id,
                              DistVarType        oldVal,
                              DistVarType        newVal,
                              DistVarType       *rtnVal,
                              bool               useDvcb );

static DistVarType Check ( const DVCB_segmentSet_t *pSegmentSet,
                           uint16             idx,
                           DistVarType        oldVal,
                           DistVarType        newVal );

// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************

bool DVCB_HELPER_SetDefaults ( const DVCB_segmentSet_t *pSegmentSet )
{
    return GenericValidate( pSegmentSet, TRUE );
}

bool DVCB_HELPER_Validate ( const DVCB_segmentSet_t *pSegmentSet )
{
    return GenericValidate( pSegmentSet, FALSE );
}

bool DVCB_HELPER_Callback ( const DVCB_segmentSet_t *pSegmentSet, //will call dvcb in DVCB_definition, or range check if null
                            DistVarID          id,
                            DistVarType        oldVal,
                            DistVarType        newVal,
                            DistVarType       *rtnVal )
{
    return GenericCallback( pSegmentSet, id, oldVal, newVal, rtnVal, TRUE );
}

bool DVCB_HELPER_Check ( const DVCB_segmentSet_t *pSegmentSet, //will not call dvcb, will simply range check (for use within dvcb, mostly)
                         DistVarID          id,
                         DistVarType        oldVal,
                         DistVarType        newVal,
                         DistVarType       *rtnVal )
{
    return GenericCallback( pSegmentSet, id, oldVal, newVal, rtnVal, FALSE );
}

// *****************************************************************************
// * PRIVATE FUNCTIONS
// *****************************************************************************

static bool GenericValidate ( const DVCB_segmentSet_t *pSegmentSet, bool reset )
{
    bool rVal = FALSE;
    uint8 segmentIndex;
    uint16 dvarIndex;
    DVarErrorCode error;
    DistVarID id;
    DistVarType val;
    DVarSearchContext search;
    uint8 retries;
    DVarErrorCode ( *setpointFunc )(DistVarID id, DistVarType value) = NULL;

    if ( reset )
    {
        setpointFunc = DVAR_SetPointLocal;
    }
    else
    {
        setpointFunc = DVAR_SetPointLocal_wCallback;
    }

    for( segmentIndex = pSegmentSet->segmentCount;
         segmentIndex;
         segmentIndex-- )
    {
        for( dvarIndex = pSegmentSet->dvarCount;
             dvarIndex;
             dvarIndex-- )
        {
            id = pSegmentSet->baseAddr;
            id += (uint32)pSegmentSet->segmentLen * (segmentIndex - 1);
            id += dvarIndex - 1;

            if ( reset )
            {
                val = pSegmentSet->dvcbArray[dvarIndex - 1].defVal;
            }
            else
            {
                if ( DVAR_InitSearch( &search ) ||
                     DVAR_SeekLocalVariable( &search, id ) ||
                     !search.owned )
                {
                    rVal = TRUE;
                    val = 0;
                }
                else
                {
                    val = search.value;
                }
            }

            retries = SETPOINT_RETRIES;
            error = setpointFunc( id, val );

            while ( error && retries-- )
            {
                (void)K_Task_Wait( SETPOINT_WAIT_TICKS );
                error = setpointFunc( id, val );
            }

            if ( error )
            {
                rVal = TRUE;
            }
        }
    }

    return rVal;
}

static bool GenericCallback ( const DVCB_segmentSet_t *pSegmentSet, //will call dvcb in DVCB_definition, or range check if null
                              DistVarID          id,
                              DistVarType        oldVal,
                              DistVarType        newVal,
                              DistVarType       *rtnVal,
                              bool               useDvcb )
{
    uint8 segmentIndex = 0;
    uint32 definitionIndex = 0;

    while ( id >= (pSegmentSet->baseAddr +
                   ((uint32)pSegmentSet->segmentLen * (segmentIndex + 1))) )
    {
        segmentIndex++;
    }

    definitionIndex = id - pSegmentSet->baseAddr - (uint32)pSegmentSet->segmentLen * segmentIndex;

    if ( definitionIndex >= pSegmentSet->dvarCount )
    {
        return TRUE;
    }
    else
    {
        if ( useDvcb && pSegmentSet->dvcbArray[definitionIndex].dvcb )
        {
            *rtnVal = pSegmentSet->dvcbArray[definitionIndex].dvcb( id, oldVal,
                                                                    newVal, pSegmentSet->dvcbArray[definitionIndex].defVal );
        }
        else
        {
            *rtnVal = Check( pSegmentSet, (uint16)definitionIndex, oldVal, newVal );
        }

        return FALSE;
    }
}

static DistVarType Check( const DVCB_segmentSet_t *pSegmentSet,
                          uint16 idx,
                          DistVarType oldVal,
                          DistVarType newVal )
{
    DistVarType rVal = newVal;
    DVCB_definition_t *def = &pSegmentSet->dvcbArray[idx];

    switch ( def->check )
    {
        case DVCB_CHECK_DEF:
            rVal = def->defVal;
            break;

        case DVCB_CHECK_OLD:
            rVal = oldVal;
            break;

        case DVCB_CHECK_NEW:
            rVal = newVal;
            break;

        case DVCB_CHECK_COERCE:
            if ( newVal < def->minVal )
            {
                rVal = def->minVal;
            }
            else if ( newVal > def->maxVal )
            {
                rVal = def->maxVal;
            }
            else
            {
                rVal = newVal;
            }
            break;

        case DVCB_CHECK_COERCE_SIGNED:
            if ( (sint32)newVal < (sint32)def->minVal )
            {
                rVal = def->minVal;
            }
            else if( (sint32)newVal > (sint32)def->maxVal )
            {
                rVal = def->maxVal;
            }
            else
            {
                rVal = newVal;
            }
            break;

        case DVCB_CHECK_REJECT_DEF:
            if ( newVal > def->maxVal || newVal < def->minVal )
            {
                rVal = def->defVal;
            }
            else
            {
                rVal = newVal;
            }
            break;

        case DVCB_CHECK_REJECT_OLD:
            if ( newVal > def->maxVal || newVal < def->minVal )
            {
                // Before we set the value back to oldVal, lets make sure it is
                // within bounds. If not, use the default value.
                if( oldVal > def->maxVal || oldVal < def->minVal )
                {
                    rVal = def->defVal;
                }
                else
                {
                    rVal = oldVal;
                }
            }
            else
            {
                rVal = newVal;
            }
            break;

        default:
            // We need to use the default assert here since this file could be
            // used by another project which does not have assert_always().
            assert( 0 ); //lint !e506
            break;
    }

    return rVal;
}

