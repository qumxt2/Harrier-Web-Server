//! \file	eeprom_internal.h
//! \brief The EEPROM Device Driver header file.
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    EEPROM write function to be used by only other common libraries
 
#ifndef EEPROM_INTERNAL_H
#define EEPROM_INTERNAL_H

#include "typedef.h"
#include "eeprom.h"

//----------------------------------------------------------------------------
//! \brief Write a series of bytes to EEPROM
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_WriteDVar( uint16 offset, uint32 dVarValue );

EEPROMErrorCode EEPROM_ReadStringNoRtos( uint16 offset, uint16 length, uint8 *pOutput );

EEPROMErrorCode EEPROM_WriteStringNoRtos( uint16 offset, uint8 length, uint8 *pInput );

#endif // EEPROM_INTERNAL_H

