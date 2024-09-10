//! \file	oscillator.h
//! 
//! Copyright 2010
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \addtogroup gca_osc			Primary Oscillator
//!
//! \b DESCRIPTION:
//!
#ifndef OSCILLATOR_H
#define OSCILLATOR_H

//! \ingroup gca_osc
//@{

#include "typedef.h"

sint32 OscInit_80MIPS (void);

sint32 OscGetFcy (void);

sint32 PeripheralBusGetFcy(void);

//@}

#endif
