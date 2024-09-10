//! \file	ADM_Bootload_Host_Primary.h
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef ADM_BOOTLOAD_HOST_PRIMARY_H
#define ADM_BOOTLOAD_HOST_PRIMARY_H

// PUBLIC FUNCTIONS

// This function is called at the component level, included in ComponentInit(), to 
// initialize the TokenBootloadTask.  There is no need to call this function from 
// the application.
bool TokenBootloadTask_Initialize( void );

	
// NOTE -- This task information must be included in main() just like any other task!!!

// TASK_X_FUNCTION_NAME		TokenBootloadTask
// TASK_X_STACK_SIZE		************AS REQUIRED************
// TASK_X_PRIORITY			Determined by the application;
void TokenBootloadTask( void );

#endif // ADM_BOOTLOAD_HOST_PRIMARY_H
