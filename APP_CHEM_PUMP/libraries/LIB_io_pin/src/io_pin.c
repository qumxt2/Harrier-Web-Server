//! \file	io_pin.c
//! \brief Module for ADCM I/O Pin Functions
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This module provides access the Pin Function settings
//!    on the ADCM component (Advanced Display Control Module)


// Include basic platform types
#include "typedef.h"

// Include IO definitions
#include "io_typedef.h"
#include "io_pin.h"
#include "in_digital.h"
#include "out_digital.h"

#include "uC_peripheral_map.h"

// ***************************************************
// * CONSTANTS
// ***************************************************
static const uint8 INVALID_STRING[] = "INVALID";

// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************

typedef IOErrorCode_t (*IOPin_Function_Set_t)( IOPin_t, IOFunction_t );

typedef struct
{
	const uint8 *pinName;
	IOPin_Function_Set_t IOPin_Function_Set;
} IOPinDescriptor_ROM_t;

typedef struct
{
	IOFunction_t currentFunction;
} IOPinDescriptor_RAM_t;

typedef struct
{
	const uint8 *functionName;
} IOFunctionDescriptor_ROM_t;


// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************

static IOErrorCode_t IOPin_Function_Set_dio_digital_only( IOPin_t pin_name, IOFunction_t function );
//static IOErrorCode_t IOPin_Function_Set_dio_hispeed_out( IOPin_t pin_name, IOFunction_t function );
//static IOErrorCode_t IOPin_Function_Set_dio_hispeed_io( IOPin_t pin_name, IOFunction_t function );

static IOErrorCode_t IOPin_Function_Set_GND( IOPin_t pin_name, IOFunction_t function );
static IOErrorCode_t IOPin_Function_Set_VCAN( IOPin_t pin_name, IOFunction_t function );
//static IOErrorCode_t IOPin_Function_Set_10V( IOPin_t pin, IOFunction_t function );
static IOErrorCode_t IOPin_Function_Set_5V( IOPin_t pin, IOFunction_t function );
//static IOErrorCode_t IOPin_Function_Set_mV( IOPin_t pin, IOFunction_t function );
static IOErrorCode_t IOPin_Function_Set_vin( IOPin_t pin, IOFunction_t function );
static IOErrorCode_t IOPin_Function_Set_vout( IOPin_t pin, IOFunction_t function );
static IOErrorCode_t IOPin_Function_Set_cout( IOPin_t pin, IOFunction_t function );


// ***************************************************
// * PRIVATE (STATIC) VARIABLES
// ***************************************************

// The order of the elements in this array must always remain consistent
// with the order defined in the IOPin_t enumerated type
static const IOPinDescriptor_ROM_t IOPinDescriptor_ROM[ IOPIN_NUM_PINS ] =
{
	{(const uint8 *)"Input1", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Input2", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Input3", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Input4", 	IOPin_Function_Set_dio_digital_only},
    
	{(const uint8 *)"Output1", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Output2", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Output3", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Output4", 	IOPin_Function_Set_dio_digital_only},

	{(const uint8 *)"Alarm LED", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Pump LED", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Cycle LED", 	IOPin_Function_Set_dio_digital_only},

	{(const uint8 *)"Heater En", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Modem Reset", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"TP1",          IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"TP6",          IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Ext Control", 	IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"Feedback",     IOPin_Function_Set_dio_digital_only},
	{(const uint8 *)"SpeedControl", IOPin_Function_Set_dio_digital_only},
    
};

static IOPinDescriptor_RAM_t IOPinDescriptor_RAM[ IOPIN_NUM_PINS ] =
{
	{IOFUNC_IN_DIGITAL},			// Input1
	{IOFUNC_IN_DIGITAL},			// Input2
	{IOFUNC_IN_DIGITAL},			// Input3
	{IOFUNC_IN_DIGITAL},			// Input4

    {IOFUNC_OUT_DIGITAL},			// Output1
	{IOFUNC_OUT_DIGITAL},			// Output2
	{IOFUNC_OUT_DIGITAL},			// Output3
	{IOFUNC_OUT_DIGITAL},			// Output4

    {IOFUNC_OUT_DIGITAL},			// Alarm LED
	{IOFUNC_OUT_DIGITAL},			// Pump LED
	{IOFUNC_OUT_DIGITAL},			// Cycle LED

    {IOFUNC_OUT_DIGITAL},			// Heater En
	{IOFUNC_OUT_DIGITAL},			// Modem Reset
	{IOFUNC_IN_DIGITAL},			// TP1
	{IOFUNC_IN_DIGITAL},			// TP6

    {IOFUNC_OUT_DIGITAL},           // Analog Input2 Selector
    {IOFUNC_OUT_DIGITAL},           // Analog Output1 - Feedback Signal
    {IOFUNC_OUT_DIGITAL},           // Analog Output2 - Speed Control
};

// The order of the elements in this array must always remain consistent
// with the order defined in the IOPinNames_t enumerated type      -------->  Correction, 'IOFunction_t'?
static const IOFunctionDescriptor_ROM_t IOFunctionDescriptor_ROM[ IOFUNC_NUM_FUNCS ] =
{
	{(const uint8 *)"VCANRTN"},
	{(const uint8 *)"VCAN"},

	{(const uint8 *)"10VDC"},
	{(const uint8 *)"5VDC"},
	{(const uint8 *)"GND"},

	{(const uint8 *)"IN_D"},
	{(const uint8 *)"OUT_D"},

	{(const uint8 *)"IN_V"},
	{(const uint8 *)"OUT_V"},

	{(const uint8 *)"IN_C"},
	{(const uint8 *)"OUT_C"}
};

// ***************************************************
// * MACROS
// ***************************************************


// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************
IOErrorCode_t IOPin_Function_Set(IOPin_t pin, IOFunction_t function)
{
	IOErrorCode_t error = IOError_InvalidPinName;

	if( pin < IOPIN_NUM_PINS )
	{
		error = IOPinDescriptor_ROM[pin].IOPin_Function_Set( pin, function );
	}
	
	return error;
}


IOFunction_t IOPin_Function_Get( IOPin_t pin )
{
	IOFunction_t function = IOFUNC_UNDEFINED;
	
	if( pin < IOPIN_NUM_PINS )
	{
		 function = IOPinDescriptor_RAM[pin].currentFunction;
	}
	
	return function;
}


const uint8 *IOPin_NameString_Get( IOPin_t pin )
{
	uint8 *nameString = (uint8 *)INVALID_STRING;
	
	if( pin < IOPIN_UNDEFINED )
	{
		 nameString = (uint8 *)IOPinDescriptor_ROM[pin].pinName;
	}
		
	return nameString;
}


const uint8 *IOPin_FuncString_Get( IOFunction_t function )
{
	uint8 *functionString = (uint8 *)INVALID_STRING;
	
	if( function < IOFUNC_UNDEFINED )
	{
		 functionString = (uint8 *)IOFunctionDescriptor_ROM[function].functionName;
	}
	
	return functionString;
}


// ***************************************************
// * PRIVATE FUCTIONS
// ***************************************************
static IOErrorCode_t IOPin_Function_Set_dio_digital_only( IOPin_t pin, IOFunction_t function )
{
	IOErrorCode_t error = IOError_UNDEFINED;
	switch( function )
	{						
		case IOFUNC_IN_DIGITAL:
		{
			IOPinDescriptor_RAM[pin].currentFunction = IOFUNC_UNDEFINED;

			// Disable the Digital Output module for this pin.
			(void)OUT_Digital_Latch_Set( pin, NOT_ASSERTED );

			IOPinDescriptor_RAM[pin].currentFunction = function;
			
			// Set up the Digital Input module for this pin.
			error = IN_Digital_State_Get( pin ).error;

			break;
		}

		case IOFUNC_OUT_DIGITAL:
		{
			IOPinDescriptor_RAM[pin].currentFunction = IOFUNC_UNDEFINED;

			// Disable the Digital Input module for this pin.
			(void)IN_Digital_State_Get( pin );

			IOPinDescriptor_RAM[pin].currentFunction = function;
			
			// Set up the Digital Output module for this pin.
			error = OUT_Digital_Latch_Set( pin, NOT_ASSERTED );
	
			break;
		}
						
		default:
		{
			error = IOError_InvalidPinConfiguration;
			
			break;
		}
		
	}
	return error;
}

