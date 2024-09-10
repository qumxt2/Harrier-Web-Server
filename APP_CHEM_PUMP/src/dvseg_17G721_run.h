// dvseg_17G721_run.h

// Copyright 20015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The header file for the run dvars used to store system runtime information

#ifndef DVSEG_17G721_RUN_H
#define DVSEG_17G721_RUN_H

// *****************************************************************************
// HEADER FILES
// *****************************************************************************
#include "dvinterface_17G721.h"

// *****************************************************************************
// CONSTANTS
// *****************************************************************************

//#define DEMO_VERSION

// *****************************************************************************
// GLOBAL VARIABLES
// *****************************************************************************

extern DVS17G721_gRun_t gRun;

// *****************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************

sint8 DVSEG_17G721_RUN_Initialize( bool resetDefaults );

DistVarType DVSEG_17G721_RUN_GetDvar( DistVarID dvar_id );
#endif 		// DVSEG_17G721_RUN_H
