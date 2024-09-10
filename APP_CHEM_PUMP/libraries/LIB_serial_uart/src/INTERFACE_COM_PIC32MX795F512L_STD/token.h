//! \file	token.h
//! \brief The Datakey Token Device Driver header file.
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \addtogroup gca_token_internal	
//! \brief Low-level Interface for Datakey Update Tokens
//!
//! \b DESCRIPTION:
//!    This module provides a simple interface for reading and writing to
//!    the Datakey Token.
//!

#ifndef TOKEN_H
#define TOKEN_H

//! \ingroup gca_token_internal	
//@{

#include "typedef.h"

typedef enum
{
	TOKENError_Ok = 0,
	TOKENError_SupportFailure,
	TOKENError_PortInitFailure,
	TOKENError_InvalidOffset,
	TOKENError_InvalidLength,
	TOKENError_BadPointer,
	TOKENError_OutOfResources,
	TOKENError_BusFailure,
	TOKENError_NoTokenFound
} TOKENErrorCode;

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the Token interface.
//----------------------------------------------------------------------------
sint16 TOKEN_Init( void );

//----------------------------------------------------------------------------
//! \brief Returns TRUE if an update token is present, FALSE otherwise
//----------------------------------------------------------------------------
bool TokenPresent(void);

//----------------------------------------------------------------------------
//! \brief Read a byte from the token
//----------------------------------------------------------------------------
TOKENErrorCode TOKEN_ReadByte( uint32 offset, uint8 *pOutput );

//----------------------------------------------------------------------------
//! \brief Read a word from the token
//----------------------------------------------------------------------------
TOKENErrorCode TOKEN_ReadWord( uint32 offset, uint16 *pOutput );

//----------------------------------------------------------------------------
//! \brief Read a longword from the token
//----------------------------------------------------------------------------
TOKENErrorCode TOKEN_ReadLongword( uint32 offset, uint32 *pOutput );

//----------------------------------------------------------------------------
//! \brief Read a series of bytes from the token
//----------------------------------------------------------------------------
TOKENErrorCode TOKEN_ReadString( uint32 offset, uint16 length, uint8 *pOutput );

//@}

#endif // TOKEN_H

