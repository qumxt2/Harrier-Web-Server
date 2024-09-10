// dvcb_helper.h

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

//
// This is a description of the file.
//

#ifndef DVCB_HELPER_H
#define DVCB_HELPER_H

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#ifdef ADM_GRAPHICS_PROCESSOR
#include "ipcTypedef.h"
#else
#include "dvar.h"
#endif // ADM_GRAPHICS_PROCESSOR

// *****************************************************************************
// * MACROS
// *****************************************************************************

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

typedef enum
{
    DVCB_CHECK_NEW = 0, //always use newVal
    DVCB_CHECK_OLD, //always use oldVal
    DVCB_CHECK_DEF, //always use defVal
    DVCB_CHECK_COERCE, //coerce newVal into range
    DVCB_CHECK_COERCE_SIGNED, //coerce newVal into range
    DVCB_CHECK_REJECT_OLD, //use oldVal if newVal isn't in range
                           //will also check that oldVal is in range
                           //if not it will use defVal
    DVCB_CHECK_REJECT_DEF, //use defVal if newVal isn't in range

    DVCB_CHECK_COUNT,
    DVCB_CHECK_INVALID = DVCB_CHECK_COUNT
} DVCB_check_t;

typedef DistVarType ( *DVCB_func_t )(DistVarID   id,
                                     DistVarType oldVal,
                                     DistVarType newVal,
                                     DistVarType defVal);

typedef const struct DVCB_definition_t
{
    DVCB_func_t  dvcb; //if null, will use following parameters
    DVCB_check_t check; //type of checking to perform
    DistVarType  minVal; //minimum allowed value
    DistVarType  maxVal; //maximum allowed value
    DistVarType  defVal; //default value
} DVCB_definition_t;

typedef struct DVCB_segmentSet_t
{
    DistVarID                baseAddr; //address of first segment
    const uint16             segmentLen; //distance between segments (usually greater than actual number of dvars)
    const uint8              segmentCount; //number of segments
    const DVCB_definition_t *dvcbArray; //should be same length as number of dvars
    const uint16             dvarCount; //length of array, and number of dvars in segment
} DVCB_segmentSet_t;

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************

// returns TRUE on error
// will use SetpointLocal to reset all variables without callbacks, in reverse 
// order (so any magic number in first spot gets updated last)
bool DVCB_HELPER_SetDefaults ( const DVCB_segmentSet_t *pSegmentSet );

// returns TRUE on error
// will use SetpointLocal_wCallback to validate all variables by attempting to
// set them to their current value (also happens in reverse order)
bool DVCB_HELPER_Validate ( const DVCB_segmentSet_t *pSegmentSet );

// returns TRUE on error
bool DVCB_HELPER_Callback ( const DVCB_segmentSet_t *pSegmentSet, //will call dvcb in DVCB_definition, or range check if null
                            DistVarID          id,
                            DistVarType        oldVal,
                            DistVarType        newVal,
                            DistVarType       *rtnVal );

// returns TRUE on error
bool DVCB_HELPER_Check ( const DVCB_segmentSet_t *pSegmentSet, //will not call dvcb, will simply range check (for use within dvcb, mostly)
                         DistVarID          id,
                         DistVarType        oldVal,
                         DistVarType        newVal,
                         DistVarType       *rtnVal );

#endif // DVCB_HELPER_H

