// I2C_Channel2.h

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains function prototypes and constants for communicating via the second
// I2C channel on the PIC32MX795F512L device.  These functions constitute a basic hardware
// driver, specific to I2C channel 2 on the PIC32MX795F512L device.

#ifndef I2C_CHANNEL2_H
#define I2C_CHANNEL2_H


// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"                                    // Compiler specific type definitions


// ******************************************************************************************
// CONSTANTS AND MACROS
// ******************************************************************************************

// ******************************************************************************************
// GLOBAL VARIABLES
// ******************************************************************************************


// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

///----------------------------------------------------------------------------
//! \brief Initalize the I2C bus (channel 2). The module will be set to
//!		operate as a master, and the bus speed will be set to 400kHz (assuming 40MIPS).
//!
//! \return TRUE if successful, otherwise FALSE
//----------------------------------------------------------------------------
sint16 I2C2_Init( void );

///----------------------------------------------------------------------------
//! \brief Get the I2C bus (channel 2) resource ID. Before calling any I2C functions
//!		you must have the I2C resource. When done using the I2C bus relase
//!		the resource ID.
//!
//! \return RTOS resource ID for the I2C2 bus
//----------------------------------------------------------------------------
uint8 I2C2_Get_Resource_ID( void );

///----------------------------------------------------------------------------
//! \brief Generate a START condition on the I2C bus (channel 2)
//!
//! \return TRUE if successful, otherwise FALSE
//----------------------------------------------------------------------------
bool I2C2_Generate_START( void );


///----------------------------------------------------------------------------
//! \brief Generate a REPEATED START condition on the I2C bus (channel 2)
//!
//! \return TRUE if successful, otherwise FALSE
//----------------------------------------------------------------------------
bool I2C2_Generate_REPEATED_START( void );


///----------------------------------------------------------------------------
//! \brief Generate a STOP condition on the I2C bus (channel 2)
//!
//! \return TRUE if successful, otherwise FALSE
//----------------------------------------------------------------------------
bool I2C2_Generate_STOP( void );


///----------------------------------------------------------------------------
//! \brief Generate a ACKNOWLEDGE condition on the I2C bus (channel 2)
//!
//! \return TRUE if successful, otherwise FALSE
//----------------------------------------------------------------------------
bool I2C2_Generate_ACK( void );


///----------------------------------------------------------------------------
//! \brief Generate a NOT ACKNOWLEDGE condition on the I2C bus (channel 2)
//!
//! \return TRUE if successful, otherwise FALSE
//----------------------------------------------------------------------------
bool I2C2_Generate_NOACK( void );


///----------------------------------------------------------------------------
//! \brief Generate a NOT ACKNOWLEDGE condition on the I2C bus (channel 2)
//!
//! \return TRUE if ACK received, otherwise FALSE
//----------------------------------------------------------------------------
bool I2C2_Wait_ACK( void );


///----------------------------------------------------------------------------
//! \brief Transmit 1 byte of data on the I2C bus (channel 2)
//!
//! \return TRUE if successful, otherwise FALSE
//----------------------------------------------------------------------------
bool I2C2_Data_Transmit( uint8 data );


///----------------------------------------------------------------------------
//! \brief Received 1 byte of data on the I2C bus (channel 2)
//!
//! \return TRUE if successful, otherwise FALSE
//!         received byte is placed into *data
//----------------------------------------------------------------------------
bool I2C2_Data_Receive( uint8 *data );

#endif		// I2C_CHANNEL2_H

//@}
