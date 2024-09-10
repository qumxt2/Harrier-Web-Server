//! \file	modbus_task.h
//!
//! Copyright 2013
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef MODBUS_TASK_H
#define MODBUS_TASK_H

//! \defgroup modbus Modbus
//! \code #include "modbus_task.h" \endcode
//!
//! This module defines the interface for the interface for Modbus Task
//! functions implemented for GCA COMMON
//!
//! \code
//! // Create Task holder in the main.c file
//! #define TASK_20_FUNCTION_NAME	MODBUS_Task
//! #define TASK_20_STACK_SIZE		300
//! #define TASK_20_PRIORITY		198
//!
//! // Initialize Modbus Task
//! MODBUS_Task_Initialize( );
//!
//! // Start Modbus Task.
//! MODBUS_Task_Start( void );
//! \endcode
//!
//! See modbus.h for Modbus stack interface

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************

//----------------------------------------------------------------------------
//! \fn MODBUS_Task_Initialize ( void )
//!
//! \brief Initialize Modbus Task
//! \return If task start fails, function will return -1, otherwise return 0
//!
//----------------------------------------------------------------------------
sint8 MODBUS_Task_Initialize( void );


//----------------------------------------------------------------------------
//! \fn MODBUS_Task_Start( void )
//!
//! \brief  Start Modbus Task
//! \return If task start fails, function will return -1, otherwise return 0
//!
//----------------------------------------------------------------------------
sint8 MODBUS_Task_Start( void );


//----------------------------------------------------------------------------
//! \fn MODBUS_Task()
//!
//! \brief MODBUS Task.
//!	
//! \notes	called by RTOS
//----------------------------------------------------------------------------
//void MODBUS_Task( void );


#endif //MODBUS_TASK_H
