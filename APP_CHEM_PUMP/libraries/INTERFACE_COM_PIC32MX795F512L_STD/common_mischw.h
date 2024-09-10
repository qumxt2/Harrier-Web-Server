//! \file	common_mischw.h
//! \brief The GCA Common Misc. Hardware library
//!
//! Copyright 2007
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \addtogroup gca_mischw			Common Misc. Hardware
//! \brief Misc. Hardware at GCA Common level.
//! 
//! \b DESCRIPTION:
//!    This module provides an interface for GCA Common Hardware features
//!    that do not fit into any of the other libraries.
 

#ifndef COMMON_MISCHW
#define COMMON_MISCHW

#include "typedef.h"

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the remainder of this interface.
//!
//! \return A negative value if the initialization failed, 0 on success.
//----------------------------------------------------------------------------
sint16 COMMON_MISCHW_Init( void );

//----------------------------------------------------------------------------
//! \brief Retrieve status of GCA Common button S1.
//!
//! \return TRUE if button is pressed, otherwise FALSE
//----------------------------------------------------------------------------
bool COMMON_MISCHW_ReadButtonS1( void );

#endif	// COMMON_MISCHW
