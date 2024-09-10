// dvseg_17G721_setup.h

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The header file for the setup dvars used to store system setup information

#ifndef DVSEG_17G721_SETUP_H
#define DVSEG_17G721_SETUP_H

// *****************************************************************************
// HEADER FILES
// *****************************************************************************
#include "dvinterface_17G721.h"
#include "TankScreen.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************
#define MAX_TANK_DEF                            10000

// *****************************************************************************
// TYPEDEFS & STRUCTURES
// *****************************************************************************

typedef enum
{
	IOHARDWARE_PRESSURESENSOR_A = 0,
	IOHARDWARE_PRESSURESENSOR_B,
	IOHARDWARE_NUM_PRESSURESENSORS
} IOHARDWARE_PRESSURESENSOR_t;

typedef struct
{
    uint32                mV_int;
    uint32                psi_int;
    IOErrorCode_t         error;
} IOrtn_mV_to_psi_int_t;

// *****************************************************************************
// GLOBAL VARIABLES
// *****************************************************************************

extern DVS17G721_gSetup_t gSetup;

// *****************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************

sint8 DVSEG_17G721_SETUP_Initialize( bool resetDefaults );

DistVarType DVSEG_17G721_SETUP_GetDvar( DistVarID dvar_id );
bool DVSEG_17G721_SETUP_MagicNumberMismatch( void );
void PressureSensorValidateDVAR (PRESSURE_SENSOR_TYPE_t type, bool reset );
void Init_Pressure_Transducer(TANK_TYPES_t TankType);
IOrtn_mV_to_psi_int_t IOHARDWARE_PressureSensorRef (IOHARDWARE_PRESSURESENSOR_t port);
bool CalculatePressureSensorOffset(void);
bool CalculateFluidDensity(void);
void Restore_4to20mA_Transducer_Defaults(void);

#endif 		// DVSEG_17G721_SETUP_H
