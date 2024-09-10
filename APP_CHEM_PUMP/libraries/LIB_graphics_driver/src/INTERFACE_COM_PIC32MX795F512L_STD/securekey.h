//! \file	securekey.h
//! \brief The interface for Secure Key tokens (Datakey IET tokens).
//!
//! Copyright 2007
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \addtogroup gca_skey			Secure Key Token
//! \brief Secure Key Token interface module 
//!
//! \b DESCRIPTION:
//!    This module provides a simple interface for accessing information
//!    stored on Secure Key tokens
//!
 
#ifndef SECUREKEY_H
#define SECUREKEY_H

//! \ingroup gca_skey
//@{

#include "typedef.h"


//----------------------------------------------------------------------------
//! \brief Check for presence of Secure Key token.
//!
//! \return TRUE if a Secure Key token is detected in the Datakey socket,
//!         otherwise FALSE.
//!
//! \b Notes:
//!	\li This function may be called as often as necessary to check for the
//!     presence of a Secure Key token. This function does not cause any
//!     read or write operations to occur on the Secure Key token.  It only
//!     checks the state of a couple hardware lines in order to detect if
//!     a Secure Key token is inserted in the socket.  
//----------------------------------------------------------------------------
bool SKEY_CheckPresence( void );


//----------------------------------------------------------------------------
//! \brief Retrieve information from Secure Key token
//!
//! \param tag		The Tag (null-terminated character string) to search for
//!					on the Secure Key token.
//! \param value	If \a tag is found, this function will load the 
//!                 associated value (null-terminated character string) into the
//!                 buffer pointed to by \a value.
//! \param bufsize	The size in bytes of the buffer pointed to by \a value.
//!
//! \return TRUE if \a tag is found, otherwise FALSE.
//!
//! \b Notes:
//!	\li OVER-USE OF THIS FUNCTION WILL RESULT IN FAILURE OF SECURE KEY TOKEN!!!
//! \li This function should only be used after detecting insertion of a new
//!     Secure Key token (typically at application start-up).  If the data read
//!     from the Secure Key token is necessary for continuing operation of the
//!     application, then the data must be copied to RAM (or some other location
//!     seperate from the Secure Key Token).  After reading necessary data from
//!     the Secure Key token, the function ::SKEY_CheckPresence() may be used
//!		as often as necessary to ensure the Secure Key token is not removed from
//!     the system.
//! \li The security features built into the Secure Key token cause an EEPROM
//!     write to occur within the token each time this function is called.
//!     Being EEPROM based, the Secure Key token has a limited number of 
//!     write cycles per cell (rated for 100,000 minimum). Reading from the
//!     Secure Key token (i.e. calling this function) will eventually cause
//!     the EEPROM cells linked to the security features to wear out, preventing
//!     any further access to the Secure Key token. 
//!     Reference Graco Test Report TR6011.
//! \li Tokens with many elements to read will want to kick the watchdog or
//!     include some wait states between element reads.
//!     A 32 byte read from the first element takes ~40mS
//!     A 32 byte read from the last element can up to ~75mS as it has to read
//!     all data up to as well as the target element.
//!
//! \sa SKEY_CheckPresence()
//----------------------------------------------------------------------------
bool SKEY_GetElement( uint8 *tag, uint8 *value, uint8 bufsize );


//@}

#endif // SECUREKEY_H

