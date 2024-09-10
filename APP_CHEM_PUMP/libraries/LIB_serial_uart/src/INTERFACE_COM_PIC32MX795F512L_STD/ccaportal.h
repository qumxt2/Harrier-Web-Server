//! \file	ccaportal.h
//! \brief The CCA Debug Portal module header file.
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!		This module presents a user-interactive console on the debug serial
//!		port.
//!

#ifndef CCAPORTAL_H
#define CCAPORTAL_H


#include "typedef.h"


//----------------------------------------------------------------------------
//! \brief Type of callback function for component and application portal interpreter
/// The type for a CCA Interpreter callback function.
/// \sa CCAPORTAL_RegisterCompCallback()
/// \sa CCAPORTAL_RegisterAppCallback()
//!
//! \param argc Number of arguments on command line
//! \param argv Pointers to arguements from command line
//!
//! \return TRUE if the callback function acted on the command, FALSE if it did not
//----------------------------------------------------------------------------
typedef bool (*CCAInterpreterCallback)( int argc, char **argv );


//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the remainder of this interface.
//!
//! \return A negative value if the initialization failed, 0 on success.
//----------------------------------------------------------------------------
sint16 CcaPortalInit( void );


//----------------------------------------------------------------------------
//! \brief Register a component level CCA Portal Interpreter callback function
//!
//! \param ccaiCallback	The Component level CCA Portal Interpreter function.
//!
//! \return Void
//----------------------------------------------------------------------------
void CCAPORTAL_RegisterCompCallback(CCAInterpreterCallback ccaiCallback);


//----------------------------------------------------------------------------
//! \brief Register an application level CCA Portal Interpreter callback function
//!
//! \param ccaiCallback	The Application level CCA Portal Interpreter function.
//!
//! \return Void
//----------------------------------------------------------------------------
void CCAPORTAL_RegisterAppCallback(CCAInterpreterCallback ccaiCallback);


void CcaInputChar( char c );


typedef void( *CcaOutputCharFunction_t )( char c, bool flush );
void CcaRegisterOutputCharFunction( CcaOutputCharFunction_t output_char_function );


//----------------------------------------------------------------------------
//! \brief	Enqueue an array of octets be transmitted out the debug serial
//!			port.
//!
//! \param buffer	A pointer to the array of characters to be output.
//! \param length	The length of the array.
//!
//! This function provides the ability for any task to generate output on
//! the debug serial port.  The output is internally buffered, and the
//! buffer is periodically serviced by the CCA Debug Portal thread.
//!
//! \b Notes:
//!	\li Invoking this function directly is not the intended application.
//!		Rather, this function is utilized by the debug module to
//!		transfer its output to the debug serial port.
//!	\li	This function treats the specified array as raw octet data, not as
//!		conventioncal C character strings.  Hence, null characters (octet
//!		value 0x00) are copied just as readily as any other character.
//!	\li	The specified array is copied to an internal queue for later
//!		transmission.  This queue is limited in size.  Hence, it is
//!		possible, under certain conditions, that only part of the specified
//!		message (or perhaps none of it) will be copied to the internal queue.
//!
//! \return Void
//!
//! \sa debug_module
//----------------------------------------------------------------------------
void CcaPortalPrint( const char *buffer, uint16 length );


//----------------------------------------------------------------------------
//! \brief	Get the amount of free space in the CcaPortal Print buffer.
//!
//! \return	 The amount of free space in the CcaPortal Print buffer in bytes
//----------------------------------------------------------------------------
uint16 CcaPortalGetBufferStatus( void );


//----------------------------------------------------------------------------
//! \brief	Immediately output an array of bytes to the debug serial port.
//!
//! \param bytes	A pointer to the array of bytes to be output.
//! \param length	The length of the array.
//!
//! This function provides the ability for any task to immediately output an
//! array of bytes on the debug serial port.  The output is not buffered and this
//! function will not return until all bytes have been sent.
//!
//! \b Notes:
//!	\li	This function treats all bytes as raw binary data.
//!	\li	This function automatically disables print buffer output before transmitting
//!     the data.  When complete, the print buffer output state will be restored to
//!     same state as it was prior to this function being called.
//----------------------------------------------------------------------------
void CcaPortalWriteByteArray( uint8 *bytes, uint32 length );


//----------------------------------------------------------------------------
//! \brief	Immediately output an array of escaped binary data to the debug serial port.
//!
//! \param bytes	A pointer to the array of bytes to be output.
//! \param length	The length of the array.
//!
//! This function provides the ability for any task to immediately output an
//! array of bytes on the debug serial port.  The output is not buffered and this
//! function will not return until all bytes have been sent.
//!
//! \b Notes:
//!	\li	This function treats all bytes as raw binary data.
//! \li This function will escape any 0x7D and 0x7E character in bytes array
//!     by first sending 0x7D (escape) character and then the data byte xor'd with 0x20.
//! \li This function will terminate data frame with a single 0x7E (framing) character.
//!	\li	This function automatically disables print buffer output before transmitting
//!     the data.  When complete, the print buffer output state will be restored to
//!     same state as it was prior to this function being called.
//----------------------------------------------------------------------------
void CcaPortalWriteEscapedDataFrame( uint8 *bytes, uint32 length );


//----------------------------------------------------------------------------
//! \brief	Enable / disable print buffer output
//!
//! \param enabled	The desired state of print buffer output
//!
//! This function provides the ability to disable output from the print buffer.
//! Its intended purpose is to mute any misc. chatter on the CCA portal when transmitting
//! binary data (e.g. CcaPortalWriteByteArray() or CcaPortalWriteEscapedDataFrame())
//!
//! \b Notes:
//! \li Disabling the output buffer only prevents the buffer from being transmitted.
//!     CcaPortalPrint() will continue to put data in buffer (if not full).
//!	\li	This function also disables any unbuffered stdio functions (e.g. printf, putc, etc.)
//----------------------------------------------------------------------------
void CcaPortalSetPrintBufferState( bool enabled );


#endif // CCAPORTAL_H
