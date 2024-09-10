//! \file	spi.h
//! \brief The SPI Port Module header file.
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \addtogroup gca_spi			SPI Ports
//! \brief Low-level Interface for PIC24H SPI Ports
//! 
//! \b DESCRIPTION:
//!    This module provides a simple interface for transmitting and receiving
//!    characters through any of the integrated SPI ports.
 
#ifndef SPI_H
#define SPI_H

//! \ingroup gca_spi	
//@{

#include "typedef.h"

typedef enum
{
	SPI_PORT_1 = 0,
	SPI_PORT_1A,
	SPI_PORT_2A,
	SPI_PORT_3A,
	
	MAX_SPI_PORTS
} SPIPORT;

typedef enum
{
	SPI_MODE_CKP_0_CKE_0 = 0,
	SPI_MODE_CKP_0_CKE_1,
	SPI_MODE_CKP_1_CKE_0,
	SPI_MODE_CKP_1_CKE_1,
	SPI_MODE_CRYPTO_I2C,

	SPI_MODE_MAX_INDEX
} SPIMODE;

typedef enum
{
	SPIError_Ok = 0,
	SPIError_SupportFailure,
	SPIError_PortInitFailure,
	SPIError_InvalidPort,
	SPIError_InitConflict,
	SPIError_BadPointer,
	SPIError_OutOfResources,
	SPIError_BadHandle,
	SPIError_BadParameter,
	SPIError_BusFailure
} SPIErrorCode;

typedef struct
{
	uint8 commandLength;
	uint8 rxDataLength;
	uint8 txDataLength;
} SPI_CRYPTOI2C_TxRx_HEADER;

typedef void (*CSModifyFunc_t)(void);
typedef void * SPIContextHandle;

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the remainder of this interface.
//----------------------------------------------------------------------------
SPIErrorCode SPI_Init( void );

bool SPI_GetSDOState_Port2A_NO_RTOS( void );

//----------------------------------------------------------------------------
//! \brief Gets a content handle.
//----------------------------------------------------------------------------
SPIErrorCode SPI_GetContextHandle(
	SPIContextHandle *pHandle,
	SPIPORT port,
	uint32 clockRate,
	SPIMODE commMode,
	CSModifyFunc_t devEnable,
	CSModifyFunc_t devDisable
);



//----------------------------------------------------------------------------
//! \brief Transmit and receive to a specified device.
//!
//! NOTE: When this function is used for devices with mode SPI_MODE_CRYPTO_I2C,
//! the first byte of data is the number of command bytes to transmit.
//! the second byte of data is the number of data bytes to receive.
//! the third byte of data is the number of data bytes to transmit.
//! the actual data begins at the fourth byte.
//----------------------------------------------------------------------------
SPIErrorCode SPI_TxRx( SPIContextHandle handle, uint8 *data, uint16 length );


//----------------------------------------------------------------------------
//! \brief Lock an SPI port to a specified device.  This will allow a caller
//! to make multiple calls to SPI_TxRx without interruption.
//!
//! NOTE: ::SPI_Unlock MUST be called when done using the device.
//----------------------------------------------------------------------------
SPIErrorCode SPI_Lock( SPIContextHandle handle );


//----------------------------------------------------------------------------
//! \brief Release the lock of an SPI port to a specified device.
//----------------------------------------------------------------------------
SPIErrorCode SPI_Unlock( SPIContextHandle handle );


//----------------------------------------------------------------------------
//! \brief Read the state of the uC SDO pin by temporarilly disabling the
//! SPI port and setting the SDO pin to an imput.  Restore the SDO pin and 
//! SPI port to previous settings on exit
//----------------------------------------------------------------------------
SPIErrorCode SPI_ReadSDO( SPIContextHandle handle, bool *pinstate );


//----------------------------------------------------------------------------
//! \brief Sends loop * (Start + 15 clocks + Stop) over the CryptoMemory
//! bit-banged I2C bus.
//!
//! NOTE: This function will only work if the device specified by handle
//! is of the mode SPI_MODE_CRYPTO_I2C and if the device is locked to the 
//! SPI port.
//----------------------------------------------------------------------------
SPIErrorCode SPI_WaitClock( SPIContextHandle handle, uint8 loop );


// Not to be called by component or application.
SPIErrorCode SPIPort2ATxRx( uint8 * data, uint16 length );

//@}

#endif // SPI_H

