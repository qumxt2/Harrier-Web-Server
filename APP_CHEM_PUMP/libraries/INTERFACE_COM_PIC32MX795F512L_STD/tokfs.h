//! \file	tokfs.h
//! \brief The Datakey Token File System interface.
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \addtogroup gca_tokfs		
//! \brief File level Interface for Datakey Update Tokens
//!
//! \b DESCRIPTION:
//!    This module provides a simple interface for accessing files stored
//!    on Datakey Update Tokens.
//!

#ifndef TOKFS_H
#define TOKFS_H

//! \ingroup gca_tokfs		
//@{

#include "typedef.h"

typedef uint32 RecordLength;
typedef uint32 FileOffset;

typedef enum
{
	TOKFSError_Ok = 0,
	TOKFSError_BadParameter,
	TOKFSError_InsufficientBufferSize,
	TOKFSError_NoDeviceFound,
	TOKFSError_NoFileSystemFound,
	TOKFSError_NoMoreFiles,
	TOKFSError_EndOfRecords,
	TOKFSError_NoRecordSelected,
	TOKFSError_DriverError
} TOKFSErrorCode;

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the Token interface.
//! Subsequent calls will reset the file interface to its initial state.
//----------------------------------------------------------------------------
sint16 TOKFS_Init( void );


//----------------------------------------------------------------------------
//! \brief Advance to next file.  If called after TOKFS_Init(), system links
//! to first file.
//----------------------------------------------------------------------------
TOKFSErrorCode TOKFS_NextFile( void );

//----------------------------------------------------------------------------
//! \brief Resets the file record locator so that a file can be parsed from the
//!	beginning.
//----------------------------------------------------------------------------
TOKFSErrorCode TOKFS_ResetFileRecordLocator( void );

//----------------------------------------------------------------------------
//! \brief Advance to next record in a file.  If called after
//! TOKFS_NextFile(), system links to first record in file.
//----------------------------------------------------------------------------
TOKFSErrorCode TOKFS_NextRecord( void );

//----------------------------------------------------------------------------
//! \brief Get length of current record.
//----------------------------------------------------------------------------
TOKFSErrorCode TOKFS_GetRecordLength( RecordLength *pLength );

//----------------------------------------------------------------------------
//! \brief Returns data offset (address) for current record.
//----------------------------------------------------------------------------
FileOffset TOKFS_GetRecordDataOffset( void );

//----------------------------------------------------------------------------
//! \brief Copy current record to a buffer.
//! On call, pLength points to the buffer length; on exit, value is changed
//! to indicate the number of bytes actually copied.
//----------------------------------------------------------------------------
TOKFSErrorCode TOKFS_ImportRecord( uint8 *pBuffer, RecordLength *pLength );

//@}

#endif // TOKFS_H
