//! \file	serial_common.h
//! \brief The Serial Port common header file.
//!
//! Copyright 2006-2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This fine defines enums and structs for use with the serial port.

#ifndef SERIAL_COMMON_H
#define SERIAL_COMMON_H

typedef void (*uartTxCB_t)();
typedef void (*uartRxCB_t)();

enum INIT_ERRORS
{
	NOERROR = 0,
	ERROR_ALREADY_INITIALIZED = 0,
	ERROR_HARDWARE_INIT,
	ERROR_INVALID_BAUD,
	ERROR_INVALID_RESOURCE_ID,
	ERROR_INVALID_RTOS_ID	
};

typedef struct {
	uint32 baudRate;
	uint8 rxPolarity;
	uint8 txPolarity;
	uint8 flowControl;
	uint8 parity;
	uint8 stopBits;
	uartTxCB_t txCallback;
	uartRxCB_t rxCallback;
	
} UART_OPTIONS_T;

#define RX_POL_POS		(0)
#define RX_POL_NEG		(1)
#define TX_POL_POS		(0)
#define TX_POL_NEG		(1)
#define FC_NONE			(0)
#define FC_CTSRTS		(1)
#define PARITY_NONE		(0)
#define PARITY_ODD		(1)
#define PARITY_EVEN		(2)


#endif // SERIAL_COMMON_H
