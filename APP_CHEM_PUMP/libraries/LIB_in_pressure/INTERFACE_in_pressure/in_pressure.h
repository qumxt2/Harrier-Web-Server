//! \file	in_pressure.h
//! \brief interface for pressure measurement.
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!


#ifndef IN_PRESSURE_H
#define IN_PRESSURE_H

#include "io_typedef.h"
#include "in_voltage.h"


// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

typedef struct
{
    mV_s16d16_t     zeroBarOffset_mV_s16d16;
    s16d16_t        scaleFactor_bar_per_mV_s16d16;
    bar_s16d16_t    fullScale_bar_s16d16;
    s16d16_t        zero_cal_mVpV_s16d16;
    s16d16_t        sensitivity_cal_mVpV_s16d16;
} PressureSensor_t;

typedef enum
{
	PS_PORT_A = 0,
	PS_PORT_B,
	PS_NUM_PORTS,
	PS_PORT_INVALID = PS_NUM_PORTS
} PressureSensor_Port_t;

typedef struct
{
    mV_s16d16_t             mV_s16d16;
    bar_s16d16_t            bar_s16d16;
    IOErrorCode_t           error;
} IOrtn_mV_to_bar_s16d16_t;

typedef struct
{
    mV_s16d16_t             mV_s16d16;
    s16d16_t                psi_s16d16;
    IOErrorCode_t           error;
} IOrtn_mV_to_psi_s16d16_t;

// *****************************************************************************
// * PUBLIC FUCTION PROTOTYPES
// *****************************************************************************

///----------------------------------------------------------------------------
//! \brief Initialize a differential pin for pressure input.
//!        Must call this before calling any other IN_Pressure functions.
//!        pressure_sensor should be supplied from an pressure_XXXXXX.h definition
//!
//! \return
//----------------------------------------------------------------------------
IOErrorCode_t IN_Pressure_Init( PressureSensor_Port_t port, const PressureSensor_t *pressureSensor );

///----------------------------------------------------------------------------
//! \brief Updates the differential pin values with new calibration data.
//!
//! \return
//----------------------------------------------------------------------------
IOErrorCode_t IN_Pressure_Cal( PressureSensor_Port_t port, s16d16_t zero_cal_mVpV_s16d16,
                               s16d16_t fullScale_cal_mVpV_s16d16 );


///----------------------------------------------------------------------------
//! \brief Get function for reading pressure sensor differential input
//!
//! \return 
//----------------------------------------------------------------------------
IOrtn_mV_to_bar_s16d16_t IN_Pressure_Get_Diff( PressureSensor_Port_t port );


///----------------------------------------------------------------------------
//! \brief Get function for reading 1-5V output pressure sensor on 0-10V analog 
//! input
//! \return 
//----------------------------------------------------------------------------
IOrtn_mV_to_bar_s16d16_t IN_Pressure_Get_1to5V( PressureSensor_Port_t port );

///----------------------------------------------------------------------------
//! \brief Get function for reading 4to20mA output pressure sensor on 0-10V analog 
//! input
//! \return 
//----------------------------------------------------------------------------
IOrtn_mV_to_bar_s16d16_t IN_Pressure_Get_4to20mA( PressureSensor_Port_t port );


///----------------------------------------------------------------------------
//! \brief Get function for reading the current pin configuration
//!
//! \return
//----------------------------------------------------------------------------
IOErrorCode_t IN_Pressure_GetPinConfig( PressureSensor_Port_t port,
                                        PressureSensor_t *pressureSensor );

IOrtn_mV_to_psi_s16d16_t GetPressurePSI (PressureSensor_Port_t port);
IOrtn_mV_to_bar_s16d16_t GetPressureBar (PressureSensor_Port_t port);
IOrtn_mV_s16d16_t GetPressureOffset_mV();							
										
#endif // IN_PRESSURE_H
