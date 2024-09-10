//! \file	in_voltage.h
//! \brief The Analog Voltage Input header file.
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \b DESCRIPTION:
//!    This module provides an interface for the analog voltage inputs
//!    on the FCM3 component

#ifndef IN_VOLTAGE_H
#define IN_VOLTAGE_H

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

// Include basic platform types
#include "typedef.h"

// Include IO definitions
#include "io_typedef.h"
#include "io_pin.h"

// *****************************************************************************
// * MACROS & CONSTANTS
// *****************************************************************************

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

typedef enum
{
	IN_VOLTAGE_RANGE_5V = 0,
	IN_VOLTAGE_RANGE_10V,
	IN_VOLTAGE_RANGE_UNDEFINED
} IN_VOLTAGE_RANGE_t;

typedef enum
{
	ADC_CHANNEL_CH0 = 0,
	ADC_CHANNEL_CH1,
	ADC_CHANNEL_CH2,
	ADC_CHANNEL_CH3,
	ADC_CHANNEL_CH4,
	ADC_CHANNEL_CH5,
	ADC_CHANNEL_CH6,
	ADC_CHANNEL_CH7,
	ADC_CHANNEL_NUM,
	ADC_CHANNEL_UNDEFINED = ADC_CHANNEL_NUM
} ADC_CHANNEL_t;

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************

///----------------------------------------------------------------------------
//! \brief Get function for reading 12-bit ADC value
//!
//! \return 
//----------------------------------------------------------------------------
IOrtn_uint16_t IN_A2D_Get( ADC_CHANNEL_t analogChannel );

IOrtn_mV_s16d16_t IN_Voltage_Pressure_Get_Diff (uint8 Channel);
IOrtn_mV_s16d16_t IN_Voltage_Pressure_Get_1to5V( uint8 Channel );
IOrtn_mV_s16d16_t IN_Voltage_Pressure_Get_4to20mA( uint8 Channel );


#endif // OUT_VOLTAGE_H
