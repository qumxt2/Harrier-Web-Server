// ee_helper.h
 
// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

//
// Getter - Setter functions for things in EEPROM.  Uses retries with delay
//

#ifndef EE_HELPER_H
#define EE_HELPER_H

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "typedef.h"

// *****************************************************************************
// * MARCROS
// *****************************************************************************

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTIONS PROTOTYPES
// *****************************************************************************

bool ev_get_uint8( uint16 offset, uint8 *value );
bool ev_set_uint8( uint16 offset, uint8 value );
bool ev_get_uint16( uint16 offset, uint16 *value );
bool ev_set_uint16( uint16 offset, uint16 value );
bool ev_get_string( uint16 offset, uint16 length, uint8 *string );
bool ev_set_string( uint16 offset, uint16 length, uint8 *string );
bool ev_get_uint32( uint16 offset, uint32 *value );
bool ev_set_uint32( uint16 offset, uint32 value );
bool ev_get_datastore ( uint16 addr, uint8 depth, uint8 *output_value, uint8 len );
bool ev_set_datastore ( uint16 addr, uint8 depth, uint8 *input_value, uint8 len );

#endif // EE_HELPER_H

