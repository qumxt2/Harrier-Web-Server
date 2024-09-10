//! \file	in_digital_comp.h
//! \brief The Digital Input header file (for internal use within component library).
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!


#ifndef IN_DIGITAL_COMP_H
#define IN_DIGITAL_COMP_H

// Include basic platform types
#include "typedef.h"

// Include IO definitions
#include "io_typedef.h"
#include "io_pin.h"
	
	
// ***************************************************
// * PUBLIC FUCTION PROTOTYPES
// ***************************************************


///----------------------------------------------------------------------------
//! \brief Reset to default all hardware related to digital input pin
//!
//! \return 
//----------------------------------------------------------------------------
IOErrorCode_t IN_Digital_Reset( IOPin_t pin );


#endif // IN_DIGITAL_COMP_H

