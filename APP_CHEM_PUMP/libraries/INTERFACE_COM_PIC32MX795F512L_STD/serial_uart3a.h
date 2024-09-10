//! \file	serial_uart3a.h
//! \brief The Serial Port Module header file (for UART3A).
//!
//! Copyright 2010
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This module provides a simple interface for transmitting and receiving
//!    characters through a component's debug serial port.
 
#ifndef SERIAL_UART3A_H
#define SERIAL_UART3A_H

#include "typedef.h"

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the remainder of this interface.
//!
//! \param	baudRate	The desired baud rate for the port.
//!
//! \return A negative value if the initialization failed, 0 on success.
//----------------------------------------------------------------------------
sint16 Serial3AInit( uint32 baudRate );



//----------------------------------------------------------------------------
//! \brief Enqueues a character to be transmitted on the debug serial port.
//!
//! \param c		The character to transmit.
//!
//! \b Notes:
//!	\li	This function call may block temporarily if another thread (or
//!		threads) is/are attempting to call this function concurrently.
//! \li	This function buffers the specified character in an internal queue.
//!		Therefore, the character may not actually be transmitted out the
//!		serial port until some time later.
//!
//! \return
//!	\li TRUE if the character was enqueued successfully.
//!	\li FALSE if the character could not be enqueued due to a shortage of
//!		buffering resources.
//----------------------------------------------------------------------------
bool Serial3ATx( uint8 c );



//----------------------------------------------------------------------------
//! \brief Retrieves a received character from the UART FIFO.
//!
//! \param cPtr	A pointer to the location to store the received character.
//!
//! This function will check the UART FIFO for the presence of at least one
//! character.  If the FIFO is non-empty, the first character in the FIFO
//! is popped, and copied to the specified location.
//!
//! \b Notes:
//!	\li	This function call will not block.
//!	\li	Erroneous characters and events are silently discarded.
//!
//! \return
//!	\li TRUE if a character was successfully copied to the specified location.
//!	\li FALSE if no characters are prsent in the UART's receive FIFO, or if
//!		the specified pointer was NULL.
//----------------------------------------------------------------------------
bool Serial3ARx( uint8 *cPtr );



//----------------------------------------------------------------------------
//! \brief	Configure the module to generate a signal when a character is
//!			received.
//!
//! \param	taskID	The ID of the task to receive the signal.
//!	\param	signal	The signal to send to the specified task.
//!
//! This function will cause the specified task to receive the specified
//! signal whenever a character is received by the debug serial port UART.
//! This signal will continue to be resent on a relatively short periodic
//! basis until all characters have been drained from the receive FIFO.
//!
//! \b Notes:
//! \li	Specifying a \c taskID of 0 will disable this signalling mechanism.
//! \li The validity of the \c taskID is not examined by this function.
//!
//! \return	Void
//----------------------------------------------------------------------------
void Serial3ANotifyRx( uint8 taskID, uint8 signal );


//----------------------------------------------------------------------------
//! \brief Enqueues a null terminated string of characters to be transmitted
//!        on the debug serial port.
//!
//! \param *str		pointer to null terminated string to transmit.
//!
//! \b Notes:
//!	\li	This function call may block temporarily if another thread (or
//!		threads) is/are attempting to call this function concurrently.
//! \li	This function buffers the specified string of characters in an
//!     internal queue.  Therefore, the character may not actually be
//!     transmitted out the	serial port until some time later.
//!
//! \return
//!	\li TRUE if the string of characters was enqueued successfully.
//!	\li FALSE if the string of characters could not be enqueued due to 
//!     a shortage of buffering resources.
//----------------------------------------------------------------------------
bool Serial3ATxString( uint8 *str );


//----------------------------------------------------------------------------
//! \brief Serial Port 3A reset function.
//!
//! This function should only be used in extreme cases to reset the initialization
//! state of Serial Port 3A.  It's intended use is for within the function
//! startup_pre_datainit() to allow us to dump data over the serial port. 
//----------------------------------------------------------------------------
void Serial3AReset( void );


//----------------------------------------------------------------------------
//! \brief Transmits a byte on the debug serial port.
//!
//! \param c		The byte to transmit.
//!
//! \b Notes:
//!	\li	This function call may block temporarily if another thread (or
//!		threads) is/are attempting to call this function or Serial3ATX() concurrently.
//! \li	This function does not buffer the specified character in an internal queue.
//!		The function wait (OS safe) until the byte has been sent.
//!
//! \return
//!	\li TRUE if the character was successfully sent.
//!	\li FALSE if the character could not be sent for any reason.
//----------------------------------------------------------------------------
bool Serial3ATxBinary( uint8 byte );

// For situations where the transmit daemon is not available
bool EmergencySerialTx( uint8 c );

#endif // SERIAL_UART3A_H
