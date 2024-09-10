// dvseg_17G721_usage.h

// Copyright 2015
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The header file for the usage dvars used to periodically back up system information to eeprom

#ifndef DVSEG_17G721_USAGE_H
#define DVSEG_17G721_USAGE_H

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "dvinterface_17G721.h"
#include "datastore.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************

#define GUSAGE_DATASTORE_COUNT        (sizeof(DVS17G721_gUsage_t)/4)
#define GUSAGE_DATASTORE_DEPTH        (10)
#define GUSAGE_DATASTORE_ELEMENT_SIZE (sizeof(DistVarType))
#define GUSAGE_DATASTORE_SIZE         (CALC_DATASTORE_SIZE(GUSAGE_DATASTORE_DEPTH,GUSAGE_DATASTORE_ELEMENT_SIZE))
#define GUSAGE_DATASTORE_TOTAL_SIZE   (GUSAGE_DATASTORE_COUNT * GUSAGE_DATASTORE_SIZE)

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

extern DVS17G721_gUsage_t gUsage;

// *****************************************************************************
// * PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************

bool DVSEG_17G721_USAGE_Initialize (bool resetDefaults);

void DVSEG_17G721_USAGE_SaveValues ( void );


#endif // DVSEG_17G721_USAGE_H

