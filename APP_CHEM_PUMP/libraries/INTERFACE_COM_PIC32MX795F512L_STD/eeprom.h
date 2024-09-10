//! \file	eeprom.h
//! \brief The EEPROM Device Driver header file.
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This module provides a simple interface for reading and writing to
//!    the AT25256 EEPROM.
//!
//! \note
//! This module internally buffers EEPROM write requests for processing
//! at a later time, so that the caller is not bogged down by a busy EEPROM.
//! Additionally, this module will search through the list of
//! as-yet-unserviced write requests whenever an EEPROM read request is
//! made, and will return the buffered write value if the EEPROM offset
//! in the buffered write match up with the offset in the EEPROM read request.
//! However, this feature does have some limitations.  Specifically, the
//! EEPROM read range must "fit" completely inside the buffered write range
//! for there to be a "hit"; partial overlaps will not count.
 
#ifndef EEPROM_H
#define EEPROM_H

#include "typedef.h"

#define	EEPROM_SIZE						(0x8000)
#define DEFAULT_APP_EEPROM_OFFSET		(0x2000)

typedef enum
{
	EEPROMError_Ok = 0,
	EEPROMError_SupportFailure,
	EEPROMError_PortInitFailure,
	EEPROMError_InvalidOffset,
	EEPROMError_InvalidLength,
	EEPROMError_BadPointer,
	EEPROMError_OutOfResources,
	EEPROMError_BusFailure,
	EEPROMError_RtosNotStarted
} EEPROMErrorCode;

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the EEPROM.
//!
//! \return
//! Returns 0 for success.  A negative value indicates failure.
//----------------------------------------------------------------------------
sint16 EEPROM_Init( void );

//----------------------------------------------------------------------------
//! \brief Set EEPROM Page Size.
//!
//! Library attempts to optimize writes by writing up to
//! a page worth of data at a time.  page_size must match device
//! being used or data may get corrupted under certain conditions.
//!
//! \return
//! new page size.  If successful, return value will match page_size parameter
//!
//! \note default page_size is 64 to match 256kbit parts.
//! \note page_size parameter must be a power of 2 no greater than 64.
//----------------------------------------------------------------------------
uint8 EEPROM_SetPageSize( uint8 page_size );
uint8 EEPROM_GetPageSize( void );

//----------------------------------------------------------------------------
//! \brief Read a byte from EEPROM
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_ReadByte( uint16 offset, uint8 *pOutput );

//----------------------------------------------------------------------------
//! \brief Read a word from EEPROM
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_ReadWord( uint16 offset, uint16 *pOutput );

//----------------------------------------------------------------------------
//! \brief Read a longword from EEPROM
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_ReadLongword( uint16 offset, uint32 *pOutput );

//----------------------------------------------------------------------------
//! \brief Read a series of bytes from EEPROM
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_ReadString( uint16 offset, uint16 length, uint8 *pOutput );

//----------------------------------------------------------------------------
//! \brief Write a byte to EEPROM
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_WriteByte( uint16 offset, uint8 input );

//----------------------------------------------------------------------------
//! \brief Write a word to EEPROM
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_WriteWord( uint16 offset, uint16 input );

//----------------------------------------------------------------------------
//! \brief Write a longword to EEPROM
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_WriteLongword( uint16 offset, uint32 input );

//----------------------------------------------------------------------------
//! \brief Write a series of bytes to EEPROM
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_WriteString( uint16 offset, uint8 length, uint8 *pInput );

//----------------------------------------------------------------------------
//! \brief Seed the random number generator with a seed value that
//! was stored in EEPROM during the last power cycle.  Also, store
//! a new seed value in EEPROM for the next power cycle.
//!
//! NOTE: This function must be called after the operating system is running
//! NOTE: This function will only do something once during a
//! power cycle (between uC resets) 
//----------------------------------------------------------------------------
void EEPROM_srand( void );

//----------------------------------------------------------------------------
// Erases EEPROM memory specified by beginOffset and endOffset
// This function 
//----------------------------------------------------------------------------
EEPROMErrorCode EEPROM_BulkErase( uint16 beginOffset, uint16 endOffset );

//----------------------------------------------------------------------------
// This function is to be called in component initialization to define where EEPROM
// memory begins for the application.
// A return value of TRUE indicates a failure.
// A return value of FALSE indicates success.
//----------------------------------------------------------------------------
bool EEPROM_SetFirstApplicationOffset( uint16 offset );

//----------------------------------------------------------------------------
// This function returns the EEPROM offset for the first memory location allocated to the application.
//----------------------------------------------------------------------------
uint16 EEPROM_GetFirstApplicationOffset( void );

#endif // EEPROM_H
