//! \file	datastore.h
//! \brief
//!
//! Copyright 2006-2009
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:


#ifndef DATASTORE_H
#define DATASTORE_H


// Include basic platform types
#include "typedef.h"

#ifdef ADM_GRAPHICS_PROCESSOR
#include "ipcGraphicsEeprom.h"
#else
#include "eeprom.h"
#endif


// ***************************************************
// * MACROS
// ***************************************************
#define CALC_DATASTORE_SIZE(backup_depth,data_bytecount)	(backup_depth*(3+data_bytecount))


// ***************************************************
// * PUBLIC FUCTION PROTOTYPES
// ***************************************************

///----------------------------------------------------------------------------
//! \brief Restore freshest backup data from an EEPROM datastore.
//!
//! \param offset			The EEPROM address of the datastore
//! \param backup_depth		The number of backup cells within the datastore.  Valid
//!							setting is 1 - 85.
//! \param data				The address in RAM of the data block to store
//! \param data_bytecount	The number of bytes of data to store
//!
//! \return
//!	\li ::EEPROMError_Ok if successful.
//!	\li ::EEPROMError_InvalidOffset if \c offset parameter is out of range
//!	\li ::EEPROMError_InvalidLength if the \c backup_depth is ouf of range (0 or greater than 85)
//!	\li ::EEPROMError_InvalidLength if \c offset, \c backup_depth, and \c data_bytecount
//! would cause the datastore to extend past the end of EEPROM storage
//! \li ::EEPROMError_BadPointer if \c data is NULL
//! \li ::EEPROMError_OutOfResources if no valid data can be recovered from datastore
//----------------------------------------------------------------------------
EEPROMErrorCode DATASTORE_RestoreData( uint16 offset, uint8 backup_depth,
									   uint8 *data, uint16 data_bytecount );


///----------------------------------------------------------------------------
//! \brief Restore freshest backup data from an EEPROM datastore and return
//!        information regarding the "freshness" of the restored data
//!
//! \param offset			The EEPROM address of the datastore
//! \param backup_depth		The number of backup cells within the datastore.  Valid
//!							setting is 1 - 85.
//! \param data				The address in RAM of the data block to store
//! \param data_bytecount	The number of bytes of data to store
//! \param stale_count      Indicator on how "stale" the restored data is.
//!                           0 - Datastore was able to successfully restore what it determined
//!                               the most fresh data
//!                           1 - Datastore was forced to revert to what it determined to be 2nd
//!                               most fresh data due to inconsistency in 1st most fresh.
//!                           2 - Datastore was forced to revert to what it determined to be 3nd
//!                               most fresh data due to inconsistency in 1st and 2nd most fresh.
//!                           etc...
//!                           ...
//!                           255 - Datastore recovery failed (will also be indicated with EEPROMErrorCode
//!                                 return value).
//!
//! \return
//!	\li ::EEPROMError_Ok if successful.
//!	\li ::EEPROMError_InvalidOffset if \c offset parameter is out of range
//!	\li ::EEPROMError_InvalidLength if the \c backup_depth is ouf of range (0 or greater than 85)
//!	\li ::EEPROMError_InvalidLength if \c offset, \c backup_depth, and \c data_bytecount
//! would cause the datastore to extend past the end of EEPROM storage
//! \li ::EEPROMError_BadPointer if \c data or \c stale_count is NULL
//! \li ::EEPROMError_OutOfResources if no valid data can be recovered from datastore
//----------------------------------------------------------------------------
EEPROMErrorCode DATASTORE_RestoreData_FreshInfo( uint16 offset, uint8 backup_depth,
                                                 uint8 *data, uint16 data_bytecount,
                                                 uint8 *stale_count );


///----------------------------------------------------------------------------
//! \brief Save data to an EEPROM datastore.
//!
//! \param offset			The EEPROM address of the datastore
//! \param backup_depth		The number of backup cells within the datastore.  Valid
//!							setting is 1 - 85.
//! \param data				The address in RAM of the data block to store
//! \param data_bytecount	The number of bytes of data to store
//!
//! \return
//!	\li ::EEPROMError_Ok if successful.
//!	\li ::EEPROMError_InvalidOffset if \c offset parameter is out of range
//!	\li ::EEPROMError_InvalidLength if the \c backup_depth is ouf of range (0 or greater than 85)
//!	\li ::EEPROMError_InvalidLength if \c offset, \c backup_depth, and \c data_bytecount
//! would cause the datastore to extend past the end of EEPROM storage
//! \li ::EEPROMError_BadPointer if \c data is NULL
//----------------------------------------------------------------------------
EEPROMErrorCode DATASTORE_SaveData( uint16 offset, uint8 backup_depth,
									uint8 *data, uint16 data_bytecount );


#endif // DATASTORE_H
