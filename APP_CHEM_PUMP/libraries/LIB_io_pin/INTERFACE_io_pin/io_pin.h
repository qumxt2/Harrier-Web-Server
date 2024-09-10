//! \file	io_pin.h
//! \brief The IO Pin Functions header file
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_adcm
//@{

//! \addtogroup gca_adcm_iopin		ACDM IO Pin Functions
//! \brief Advanced Display Control Module (ADCM) IO Pin Functions
//!
//! \b DESCRIPTION:
//!    This module provides an interface for the IO Pin functions
//!    on the ACDM (Advanced Display Control Module)

#ifndef IO_PIN_H
#define IO_PIN_H

//! \ingroup gca_adcm_iopin
//@{

// Include basic platform types
#include "typedef.h"

// Include IO definitions
#include "io_typedef.h"

	
// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************

// DCM I/O PIN NAMES
typedef enum
{
	IOPIN_INPUT_1 = 0,
    IOPIN_INPUT_2,
    IOPIN_INPUT_3,
    IOPIN_INPUT_4,

    IOPIN_OUTPUT_1,
    IOPIN_OUTPUT_2,
    IOPIN_OUTPUT_3,
    IOPIN_OUTPUT_4,

    IOPIN_ALARM_LED,
    IOPIN_PUMP_LED,
    IOPIN_CYCLE_LED,

    IOPIN_HEAT_EN,
    IOPIN_MODEM_RESET,
    IOPIN_TP1,
    IOPIN_TP6,

    IOPIN_EXT_CNTL_INPUT_SELECT,
    IOPIN_FEEDBACK_OUTPUT_SELECT,
    IOPIN_SPEED_CNTL_OUTPUT_SELECT,

	IOPIN_NUM_PINS,
	IOPIN_UNDEFINED = IOPIN_NUM_PINS
} IOPin_t;

// FCM3 I/O FUNCTIONS
typedef enum
{
	IOFUNC_FIXED_VCANRTN = 0,
	IOFUNC_FIXED_VCAN,

	IOFUNC_FIXED_10VDC,
	IOFUNC_FIXED_5VDC,
	IOFUNC_FIXED_GND,

	IOFUNC_IN_DIGITAL,
	IOFUNC_OUT_DIGITAL,

	IOFUNC_IN_VOLTAGE,
	IOFUNC_OUT_VOLTAGE,

	IOFUNC_IN_CURRENT,
	IOFUNC_OUT_CURRENT,

	IOFUNC_NUM_FUNCS,
	IOFUNC_UNDEFINED = IOFUNC_NUM_FUNCS
} IOFunction_t;


// ***************************************************
// * PUBLIC FUCTION PROTOTYPES
// ***************************************************

///----------------------------------------------------------------------------
//! \brief Set the function of an I/O pin
//!  	If using as High Speed input capture, set pin to digital INPUT, and
//!		enable high speed module IN_Hispeed_Mode_Set declared in in_hispeed.h.
//!
//!		example: 
//!			IOPin_Function_Set(IOPIN_ADCM_DIO_4_2, IOFUNC_IN_DIGITAL);
//!			IN_Hispeed_Mode_Set(IOPIN_ADCM_DIO_4_2, IN_HISPEED_MODE_FALLING_EDGE);
//!
//! \return 
//----------------------------------------------------------------------------
IOErrorCode_t IOPin_Function_Set(IOPin_t pin, IOFunction_t function );


///----------------------------------------------------------------------------
//! \brief Get the current function setting of an I/O pin
//!
//! \return The function that pin is currently set to
//----------------------------------------------------------------------------
IOFunction_t IOPin_Function_Get( IOPin_t pin );


///----------------------------------------------------------------------------
//! \brief Get the text description of an IOPin_t
//!
//! \return Pointer to a null-terminated character string (stored in const memory)
//----------------------------------------------------------------------------
const uint8 *IOPin_NameString_Get( IOPin_t pin );


///----------------------------------------------------------------------------
//! \brief Get the text description of an IOFunction_t
//!
//! \return Pointer to a null-terminated character string (stored in const memory)
//----------------------------------------------------------------------------
const uint8 *IOPin_FuncString_Get( IOFunction_t function );


#endif // IO_PIN_H


//@}
