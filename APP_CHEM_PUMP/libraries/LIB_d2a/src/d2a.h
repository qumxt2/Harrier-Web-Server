// d2a.h

// Copyright 2006-2008
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains function prototypes to interface with the
// Digital to Analog converters on FCM3 (GCA Cube - Fluid Control Module)

#ifndef D2A_H
#define D2A_H

#include "typedef.h"						// Compiler specific type definitions


// ***************************************************
// * CONSTANTS
// ***************************************************

// Hardware (Analog Devices AD5338) is really only 10-bit (instead of
// 12-bit implied by fullscale).  But, the way the D2A chip is designed,
// the communications format is compatible between the 8, 10, and 12 bit
// variations.
#define D2A_FULLSCALE	(0x0FFF)


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************
typedef enum {
	D2A_MODULE_0_CHA = 0,
	D2A_MODULE_0_CHB,
	D2A_MODULE_1_CHA,
	D2A_MODULE_1_CHB,
	D2A_MODULE_2_CHA,
	D2A_MODULE_2_CHB,
	D2A_MODULE_3_CHA,
	D2A_MODULE_3_CHB,
	D2A_NUM_CHANNELS,
	D2A_CHANNEL_INVALID = D2A_NUM_CHANNELS
} D2A_CHANNEL_t;


// ***************************************************
// * PUBLIC FUCTION PROTOTYPES
// ***************************************************

///----------------------------------------------------------------------------
//! \brief Initialization function for the A/D converter
//!
//! \return
//----------------------------------------------------------------------------
sint16 D2A_Initialize( void );


///----------------------------------------------------------------------------
//! \brief Get the current D2A output setting
//!
//! \return TRUE if successful, otherwise FALSE
//----------------------------------------------------------------------------
bool D2A_Output_Get( D2A_CHANNEL_t channel, uint16 *value );


///----------------------------------------------------------------------------
//! \brief Set the D2A output
//!
//! \return TRUE if successful, otherwise FALSE
//!
//! NOTE: This function accepts a 12-bit value (D2A_FULLSCALE).  However, the EFCM
//! hardware only uses a 10-bit D/A converter chip.  So, the least 2 significan bits
//! of the value will be coerced to 0, resulting in an effective 10-bit D2A resolution.
//----------------------------------------------------------------------------
bool D2A_Output_Set( D2A_CHANNEL_t channel, uint16 value );


///----------------------------------------------------------------------------
//! \brief Get the text description of an D2A_CHANNEL_t
//!
//! \return Pointer to a null-terminated character string (stored in const memory)
//----------------------------------------------------------------------------
const char *D2A_Channel_NameString_Get( D2A_CHANNEL_t channel );


#endif		// D2A_H
