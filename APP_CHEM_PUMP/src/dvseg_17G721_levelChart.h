// dvseg_17G721_levelChart.h

// Copyright 2016 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

#ifndef DVSEG_17G721_LEVELCHART_H
#define DVSEG_17G721_LEVELCHART_H

// The header file for the level chart dvars used for the tank strap chart

// *****************************************************************************
// HEADER FILES
// *****************************************************************************
#include "dvinterface_17G721.h"

// *****************************************************************************
// CONSTANTS
// *****************************************************************************

// *****************************************************************************
// GLOBAL VARIABLES
// *****************************************************************************

extern DVS17G721_gLevelChart_t gLevelChart;

// *****************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************
sint8 DVSEG_17G721_LevelChart_Initialize( bool resetDefaults );

DistVarType DVSEG_17G721_LevelChart_GetDvar( DistVarID dvar_id );

#endif 		// DVSEG_17G721_LEVELCHART_H
