//! \file	in_digital.h
//! \brief The Digital Input header file.
//!
//! Copyright 2010
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_adm
//@{

//! \addtogroup gca_adm_indigital			ADM Digital Inputs
//! \brief Advanced Display Module Digital Inputs
//!
//! \b DESCRIPTION:
//!    This module provides an interface for the digital inputs
//!    on the ADM component

#ifndef IN_DIGITAL_H
#define IN_DIGITAL_H

//! \ingroup gca_adm_indigital
//@{

// Include basic platform types
#include "typedef.h"

// Include IO definitions
#include "io_typedef.h"
#include "io_pin.h"
	

// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************


// ***************************************************
// * PUBLIC FUCTION PROTOTYPES
// ***************************************************


///----------------------------------------------------------------------------
//! \brief Get function for reading state of Digital Input pin
//!
//! \return 
//----------------------------------------------------------------------------
IOrtn_digital_t IN_Digital_State_Get( IOPin_t pin );


#endif // OUT_DIGITAL_H


//@}

