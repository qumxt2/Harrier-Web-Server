//! \file	in_digital.c
//! \brief Module for FCM3 Digital Inputs
//!
//! Copyright 2011
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This module provides access to the (Low-speed) Digital Inputs
//!    on the DCM component (Display Control Module, 24L096)


// Include basic platform types
#include "typedef.h"

#include "p32mx795f512l.h"

// Include IO definitions
#include "io_typedef.h"
#include "io_pin.h"
#include "uC_peripheral_map.h"

#include "in_digital.h"


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************
typedef enum
{
	IDCMD_GET_PIN = 0
} IDCMD_t;


// ***************************************************
// * PRIVATE (STATIC) VARIABLES
// ***************************************************


// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************
static IOrtn_digital_t DI_N_Input( IOPin_t pin );


// ***************************************************
// * CONSTANTS
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************


// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************

IOrtn_digital_t IN_Digital_State_Get( IOPin_t pin )
{
	IOrtn_digital_t rtnval;

	IOFunction_t currentFunction = IOPin_Function_Get( pin );

	rtnval.state = NOT_ASSERTED;
	rtnval.error = IOError_UNDEFINED;
	
	switch( currentFunction )
	{
		case IOFUNC_IN_DIGITAL:
		{
			rtnval = DI_N_Input( pin );
			
			break;
		}

		case IOFUNC_UNDEFINED:
		{
			rtnval.state = NOT_ASSERTED;
			rtnval.error = IOError_InvalidPinName;

			break;
		}
		
		default:
		{
			rtnval.state = NOT_ASSERTED;
			rtnval.error = IOError_InvalidPinConfiguration;
		
			break;
		}
	}
	
	return rtnval;
}


// ***************************************************
// * PRIVATE FUCTIONS
// ***************************************************

static IOrtn_digital_t DI_N_Input( IOPin_t pin )
{
	IOrtn_digital_t rtnval;
	
	volatile unsigned int* reg = NULL;
	uint32 bitmask = 0x0000;

	rtnval.state = NOT_ASSERTED;
	rtnval.error = IOError_UNDEFINED;
	
	switch( pin )
	{
		case IOPIN_INPUT_1:
		{
			reg = &PORT_INPUT_1;
			bitmask = BITMASK_INPUT_1;
			break;
		}

        case IOPIN_INPUT_2:
		{
			reg = &PORT_INPUT_2;
			bitmask = BITMASK_INPUT_2;
			break;
		}

        case IOPIN_INPUT_3:
		{
			reg = &PORT_INPUT_3;
			bitmask = BITMASK_INPUT_3;
			break;
		}

        case IOPIN_INPUT_4:
		{
			reg = &PORT_INPUT_4;
			bitmask = BITMASK_INPUT_4;
			break;
		}

        case IOPIN_TP6:
		{
			reg = &PORT_TP6;
			bitmask = BITMASK_TP6;
			break;
		}

		default:
		{
			reg = NULL;
			bitmask = 0x0000;

			rtnval.state = NOT_ASSERTED;
			rtnval.error = IOError_InvalidPinConfiguration;
			break;
		}
	}

	if( reg != NULL )
	{
		if( *reg & bitmask )
		{
			rtnval.state = ASSERTED;
		}
		else
		{
			rtnval.state = NOT_ASSERTED;
		}
		rtnval.error = IOError_Ok;
	}
		
	return rtnval;
}

