// TankScreen.h

// Copyright 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the main tank screen

#ifndef TANK_SCREEN_H
#define TANK_SCREEN_H

#include "in_pressure.h"
#include "screensTask.h"

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
#define NUM_STRAP_CHART_LINES           20

// The max values that can be entered on the strap chart screen
#define INVALID_CHART_ENTRY_VOL_G       99999u                     // 9,999.9  gallons
#define INVALID_CHART_ENTRY_VOL_L       999999u                    // 99,999.9  liters
#define INVALID_CHART_ENTRY_LVL_IN      99999u                     // 999.99 inches
#define INVALID_CHART_ENTRY_LVL_CM      9999u                      // 999.9  cm

// The value that is stored in the dvar as the "invalid" value
#define INVALID_CHART_ENTRY_DVAR_VOL    INVALID_CHART_ENTRY_VOL_G  // Stored in US units so use the invalid entry for gallons, Note: 99999 L = 26 G so this will still let us use 99999 L
#define INVALID_CHART_ENTRY_DVAR_LVL    INVALID_CHART_ENTRY_LVL_IN // Stored in US units so use the invalid entry for inches

#define INVALID_CHART_INDEX             99
#define TENTH_GAL_PER_CU_IN             4329
#define TENTH_GAL_PER_CU_IN_FLOAT       .04329
#define TANK_HEIGHT_MIN                 0
#define TANK_HEIGHT_MAX                 (144u * 100u)     // Inches * 10
#define TANK_VOLUME_SCALE_FACTOR        230855u           // (12 in/ft * 8.33 lb/gal) / 0.433 psi/ft) scaled by 1000 for extra precision
#define MAX_VOLUME_G                    (99999u)          //  9,999.9 gallons
#define MAX_VOLUME_L                    (378496u)         // 37,849.6 liters
#define NUM_VOLUME_DIGITS_G             5u                //  9,999.9 Gallons max
#define NUM_VOLUME_DIGITS_L             6u                // 37,849.6 Liters max

typedef enum
{
    TANK_TYPE_VERTICAL = 0,
    TANK_TYPE_HORIZONTAL,
    TANK_TYPE_CUSTOM,     
    NUMBER_TANK_TYPES
} TANK_TYPES_t;

typedef enum
{
	PRESSURE_SENSOR_TYPE_INVALID = 0,
    PRESSURE_SENSOR_TYPE_15M669,
    PRESSURE_SENSOR_TYPE_17L037,
    PRESSURE_SENSOR_TYPE_XXXXXX,
    PRESSURE_SENSOR_TYPE_420420,
    NUMBER_PRESSURE_SENSOR_TYPES
} PRESSURE_SENSOR_TYPE_t;

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************
typedef enum
{
    FOCUS_VOLUME_COLUMN = 0,
    FOCUS_LEVEL_COLUMN,
    NUMBER_STRAP_CHART_COLUMNS
} COLUMN_FOCUS_t;

// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************
INPUT_MODE_t TankScreen(INPUT_EVENT_t InputEvent);
void UpdateTankLevel(void);
DVarErrorCode DVAR_SetPointLocal_wRetry(DistVarID id, DistVarType value);
void TankArraySort(DistVarType lookupColumn[], DistVarType volumeColumn[]);
uint32 validateVolumeEntry(uint32 volumeEntry);


//*******************************************************************************************


#endif
