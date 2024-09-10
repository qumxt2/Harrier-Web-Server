//! \file	serial_uart_U1A.h
//! \brief The Serial Port Module header file (for UART U1A).
//!
//! Copyright 2006-2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This module provides a simple interface for transmitting and receiving
//!    characters through a component's debug serial port.

#ifndef SERIAL_UART_U1A_H
#define SERIAL_UART_U1A_H

#include "typedef.h"
#include "serial_common.h"

//----------------------------------------------------------------------------
//! \brief Initialization function for both the hardware and the task.
//!
//! Must be invoked prior to using the remainder of this interface.
//!
//! \param	options	A structure of configuration parameters.
//!
//! \return A negative value if the initialization failed, 0 on success.
//----------------------------------------------------------------------------
enum INIT_ERRORS Serial_U1A_Init( UART_OPTIONS_T *options );

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
bool Serial_U1A_Rx( uint8 *cPtr );

//----------------------------------------------------------------------------
//! \brief Transmits an array of characters via DMA RAM.
//!
//! \param uint8 *ptData  Pointer to character string to transmit.
//!
//! \param uint16 count  The number of bytes to transfer from this data source.
//!
//! \param uint16 timeout  The number of RTOS ticks to wait before timing out.
//!
//! \b Notes:
//!	\li	This function call may block temporarily if another thread (or
//!		threads) is/are attempting to call this function concurrently.
//!	\li	This function will call K_Task_Wait when a buffer is unavailable and
//!     will cause the calling task to sleep.
//! \li	This function buffers the specified string of characters in an
//!     DMA RAM location.  Therefore, the character may not actually be
//!     transmitted out the	serial port until some time later.  The function
//!     will return as soon as the Tx data has been queued for transmit.
//!
//! \return	FALSE The function did not time out.
//!
//! \return	TRUE The function did time out.
//----------------------------------------------------------------------------
bool Serial_U1A_Tx(uint8 *ptData, uint16 count, uint16 timeout);

//----------------------------------------------------------------------------
//! \brief Serial port reset function.
//!
//! This function should only be used in extreme cases to reset the initialization
//! state of the serial port.  It is intended use is for within the function
//! startup_pre_datainit() to allow us to dump data over the serial port.
//----------------------------------------------------------------------------
void Serial_U1A_Reset( void );


#endif // SERIAL_UART_U1A_H
