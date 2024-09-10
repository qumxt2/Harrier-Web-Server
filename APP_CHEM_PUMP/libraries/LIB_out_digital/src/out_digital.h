//! \file	out_digital.h
//! \brief The Digital Output header file.
//!
//! Copyright 2006-2008
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_adm
//@{

//! \addtogroup gca_adm_outdigital			ADM Digital Outputs
//! \brief Advanced Display Module Digital Outputs
//!
//! \b DESCRIPTION:
//!    This module provides an interface for the digital outputs
//!    on the ADM component

#ifndef OUT_DIGITAL_H
#define OUT_DIGITAL_H

//! \ingroup gca_adm_outdigital
//@{

// Include basic platform types
#include "typedef.h"

// Include IO definitions
#include "io_typedef.h"
#include "io_pin.h"
	
	
// ***************************************************
// * MACROS
// ***************************************************

// DEVELOPER'S NOTE:  The following constants exist on both the primary and graphics 
// processors of the ADM.  Any changes made in either location must be reflected in the 
// opposite location to ensure proper operation.  

#define DIGOUT_NOFAULT		( ASSERTED )
#define DIGOUT_FAULT		( NOT_ASSERTED )


// ***************************************************
// * PUBLIC FUCTION PROTOTYPES
// ***************************************************

///----------------------------------------------------------------------------
//! \brief Set function for Digital Output Latch
//!
//! \return 
//----------------------------------------------------------------------------
IOErrorCode_t OUT_Digital_Latch_Set( IOPin_t pin, IOState_t state );


///----------------------------------------------------------------------------
//! \brief Get function for reading state of Digital Output Latch
//!
//! \return 
//----------------------------------------------------------------------------
IOrtn_digital_t OUT_Digital_Latch_Get( IOPin_t pin );


///----------------------------------------------------------------------------
//! \brief Get function for reading fault status of Digital Output pin
//!
//! \return 
//----------------------------------------------------------------------------
IOrtn_digital_t OUT_Digital_Fault_Get( IOPin_t pin );


#endif // OUT_DIGITAL_H


//@}

