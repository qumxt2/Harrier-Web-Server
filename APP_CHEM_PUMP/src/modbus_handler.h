// modbus_handler.h

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// The header file for modbus initialization & other generic modbus callbacks

#ifndef MODBUS_HANDLER_H
#define MODBUS_HANDLER_H

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************
sint8 Modbus_Init( uint8 SlaveID, uint8 Parity, uint8 StopBits );


#endif//MODBUS_HANDLER_H
