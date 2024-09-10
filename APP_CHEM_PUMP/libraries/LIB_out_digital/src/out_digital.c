//! \file	out_digital.c
//! \brief Module for DCM Digital Outputs
//!
//! Copyright 2006-2011
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This module provides access the (Low-speed) Digital Outputs
//!    on the DCM component (Display Control Module)


// Include basic platform types
#include "typedef.h"
#include "p32mx795f512l.h"
#include "debug.h"

// Include IO definitions
#include "io_typedef.h"
#include "io_pin.h"
#include "uC_peripheral_map.h"
#include "out_digital.h"


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************
typedef enum
{
	ODCMD_SET_LATCH = 0,
	ODCMD_GET_LATCH,
	ODCMD_GET_FAULT
} ODCMD_t;


// ***************************************************
// * PRIVATE (STATIC) VARIABLES
// ***************************************************


// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************
static IOErrorCode_t ODFunc( ODCMD_t command, IOPin_t pin, IOState_t *state );

static IOErrorCode_t ODFunc_P_Output( ODCMD_t command, IOPin_t pin, IOState_t *state );
						  	      
static IOState_t DO_P_Output( bool set, IOPin_t pin, IOState_t state );
static IOState_t DO_P_Status( IOPin_t pin );


// ***************************************************
// * CONSTANTS
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************
#define DO_P_Output_Set( pin, state )		(void)DO_P_Output( TRUE, pin, state )
#define DO_P_Output_Read( pin )				DO_P_Output( FALSE, pin, NOT_ASSERTED )

#define DO_P_Status_Read( pin )				DO_P_Status( pin )


// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************
IOErrorCode_t OUT_Digital_Latch_Set( IOPin_t pin, IOState_t state )
{
	return ODFunc( ODCMD_SET_LATCH, pin, &state );
}
	

IOrtn_digital_t OUT_Digital_Latch_Get( IOPin_t pin )
{
	IOrtn_digital_t rtnval;
	rtnval.error = ODFunc( ODCMD_GET_LATCH, pin, &(rtnval.state) );
	
	return rtnval;
}

IOrtn_digital_t OUT_Digital_Fault_Get( IOPin_t pin )
{
	IOrtn_digital_t rtnval;

	rtnval.error = ODFunc( ODCMD_GET_FAULT, pin, &(rtnval.state) );
	
	return rtnval;
}


// ***************************************************
// * PRIVATE FUCTIONS
// ***************************************************
static IOState_t DO_P_Output( bool set, IOPin_t pin, IOState_t state )
{
	volatile unsigned int* setAddr = NULL;
	volatile unsigned int* clrAddr = NULL;
	volatile unsigned int* readAddr = NULL;
	uint32 bitmask = 0x0000;
	
	switch( pin )
	{
        case IOPIN_OUTPUT_1:
		{
			setAddr = &LAT_SET_OUTPUT_1;
			clrAddr = &LAT_CLR_OUTPUT_1;
			readAddr = &LAT_READ_OUTPUT_1;
			bitmask = BITMASK_OUTPUT_1;
			break;
		}

        case IOPIN_OUTPUT_2:
		{
			setAddr = &LAT_SET_OUTPUT_2;
			clrAddr = &LAT_CLR_OUTPUT_2;
			readAddr = &LAT_READ_OUTPUT_2;
			bitmask = BITMASK_OUTPUT_2;
			break;
		}

        case IOPIN_OUTPUT_3:
		{
			setAddr = &LAT_SET_OUTPUT_3;
			clrAddr = &LAT_CLR_OUTPUT_3;
			readAddr = &LAT_READ_OUTPUT_3;
			bitmask = BITMASK_OUTPUT_3;
			break;
		}
 
//        case IOPIN_OUTPUT_4:
//		{
//			setAddr = &LAT_SET_OUTPUT_4;
//			clrAddr = &LAT_CLR_OUTPUT_4;
//			readAddr = &LAT_READ_OUTPUT_4;
//			bitmask = BITMASK_OUTPUT_4;
//			break;
//		}
        
        case IOPIN_ALARM_LED:
		{
			setAddr = &LAT_SET_ALARM_LED;
			clrAddr = &LAT_CLR_ALARM_LED;
			readAddr = &LAT_READ_ALARM_LED;
			bitmask = BITMASK_ALARM_LED;
			break;
		}

        case IOPIN_PUMP_LED:
		{
			setAddr = &LAT_SET_PUMP_LED;
			clrAddr = &LAT_CLR_PUMP_LED;
			readAddr = &LAT_READ_PUMP_LED;
			bitmask = BITMASK_PUMP_LED;
			break;
		}

        case IOPIN_CYCLE_LED:
		{
			setAddr = &LAT_SET_CYCLE_LED;
			clrAddr = &LAT_CLR_CYCLE_LED;
			readAddr = &LAT_READ_CYCLE_LED;
			bitmask = BITMASK_CYCLE_LED;
			break;
		}

        case IOPIN_HEAT_EN:
		{
			setAddr = &LAT_SET_HEAT_EN;
			clrAddr = &LAT_CLR_HEAT_EN;
			readAddr = &LAT_READ_HEAT_EN;
			bitmask = BITMASK_HEAT_EN;
			break;
		}

        case IOPIN_MODEM_RESET:
		{
			setAddr = &LAT_SET_MODEM_RESET;
			clrAddr = &LAT_CLR_MODEM_RESET;
			readAddr = &LAT_READ_MODEM_RESET;
			bitmask = BITMASK_MODEM_RESET;
			break;
		}

        case IOPIN_EXT_CNTL_INPUT_SELECT:
		{
			setAddr = &LAT_SET_EXT_CONTROL_SELECT;
			clrAddr = &LAT_CLR_EXT_CONTROL_SELECT;
			readAddr = &LAT_READ_EXT_CONTROL_SELECT;
			bitmask = BITMASK_EXT_CONTROL_SELECT;
			break;
		}

        case IOPIN_FEEDBACK_OUTPUT_SELECT:
		{
			setAddr = &LAT_SET_EXT_FEEDBACK_SELECT;
			clrAddr = &LAT_CLR_EXT_FEEDBACK_SELECT;
			readAddr = &LAT_READ_EXT_FEEDBACK_SELECT;
			bitmask = BITMASK_EXT_FEEDBACK_SELECT;
			break;
		}

        case IOPIN_SPEED_CNTL_OUTPUT_SELECT:
		{
			setAddr = &LAT_SET_MOTOR_SELECT;
			clrAddr = &LAT_CLR_MOTOR_SELECT;
			readAddr = &LAT_READ_MOTOR_SELECT;
			bitmask = BITMASK_MOTOR_SELECT;
			break;
		}

		default:
		{
			setAddr = NULL;
			clrAddr = NULL;
			readAddr = NULL;
			bitmask = 0x0000;
			break;
		}
	}
	if( (setAddr != NULL) &&
		(clrAddr !=	NULL) &&
		(readAddr != NULL)
	  )
	{
		if( set )
		{	
			// Inhibit lint messages 662 & 661: Possible creation of out-of-bounds pointer
			// and Possible access of out-of-bounds pointer
			//lindt -e{661,662}
			if( state == NOT_ASSERTED )
			{
				*clrAddr = bitmask;
			}
			else
			{
				*setAddr = bitmask;
			}
		}
		else
		{
			if( *readAddr & bitmask )
			{
				state = ASSERTED;
			}
			else
			{
				state = NOT_ASSERTED;
			}
		}
	}
	
	return state;
}

static IOState_t DO_P_Status( IOPin_t pin )
{
	IOState_t state = DIGOUT_FAULT;
	
	volatile unsigned int* reg = NULL;
	uint16 bitmask = 0x0000;
	
	switch( pin )
	{

		default:
		{
			reg = NULL;
			bitmask = 0x0000;
			break;
		}
	}

	if( reg != NULL )
	{
		if( *reg & bitmask )
		{
			state = DIGOUT_NOFAULT;
		}
		else
		{
			state = DIGOUT_FAULT;
		}
	}
		
	return state;
}

static IOErrorCode_t ODFunc( ODCMD_t command, IOPin_t pin, IOState_t *state )
{
	IOErrorCode_t error = IOError_UNDEFINED;
	
	switch( pin )
	{
        case IOPIN_OUTPUT_1:
        case IOPIN_OUTPUT_2:
        case IOPIN_OUTPUT_3:
        case IOPIN_OUTPUT_4:
        case IOPIN_ALARM_LED:
        case IOPIN_PUMP_LED:
        case IOPIN_CYCLE_LED:
        case IOPIN_HEAT_EN:
        case IOPIN_MODEM_RESET:
        case IOPIN_EXT_CNTL_INPUT_SELECT:
        case IOPIN_FEEDBACK_OUTPUT_SELECT:
        case IOPIN_SPEED_CNTL_OUTPUT_SELECT:
        {
			error = ODFunc_P_Output( command, pin, state );
			break;
		}

		default:
		{
			error = IOError_InvalidPinName;
			break;
		}
	}
		
	return error;
}


static IOErrorCode_t ODFunc_P_Output( ODCMD_t command, IOPin_t pin, IOState_t *state ) 
{
	IOErrorCode_t error = IOError_Ok;
	IOFunction_t currentFunction = IOPin_Function_Get( pin );

	switch( command )
	{
		case ODCMD_SET_LATCH:
		{
			switch( currentFunction )
			{				
				case IOFUNC_FIXED_VCAN:
				{
					if( *state != ASSERTED )
					{
						*state = ASSERTED;
						error = IOError_InvalidPinConfiguration;
					}
					// slide thru to next case
				}
				// -lint w616  -  control flows into next case on purpose
				case IOFUNC_OUT_DIGITAL:
				{
					break;
				}

				default:
				{
					*state = NOT_ASSERTED;
					error = IOError_InvalidPinConfiguration;
					
					break;
				}
			}
			
			DO_P_Output_Set( pin, *state );

			break;
		}
		
		case ODCMD_GET_LATCH:
		{
			*state = DO_P_Output_Read( pin );
			break;
		}

		case ODCMD_GET_FAULT:
		{
			switch( currentFunction )
			{
				case IOFUNC_FIXED_VCAN:
				case IOFUNC_OUT_DIGITAL:
				{
					*state = DO_P_Status_Read( pin );
					
					break;
				}

				default:
				{
					*state = DIGOUT_FAULT;
					error = IOError_InvalidPinConfiguration;
				
					break;
				}
			}
			
			break;
		}

		default:
		{
			*state = NOT_ASSERTED;
			error = IOError_UNDEFINED;
			break;
		}
	}
	
	return error;
}
