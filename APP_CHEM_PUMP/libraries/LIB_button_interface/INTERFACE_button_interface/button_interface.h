//! \file	button_interface.h
//! \brief  Button Interface Library
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//!
//! \addtogroup gca_adcm
//@{
//!
//! \addtogroup gca_adcm_button_interface		ADCM Button Interface Library
//! \brief ADCM (Advanced Display Module) Button Interface Library
//!
//! \b DESCRIPTION:
//!    This file contains all of the constants, types, and function prototypes associated with
//!    the button input interface on the front of the Advanced Display Control Module (ADCM) enclosure.

#ifndef BUTTON_INTERFACE_H
#define BUTTON_INTERFACE_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"		//!         Compiler specific type definitions

// ******************************************************************************************
// * CONSTANTS
// ******************************************************************************************

// Constants used as return values for the ButtonInterfaceInit and ButtonInterfaceTaskInit 
// functions
#define BUTTON_INIT_NO_ERROR	(0)			
#define BUTTON_INIT_ERROR		(-1)		

// ******************************************************************************************
// * TYPE DEFINITIONS (STRUCTURES AND ENUMERATED TYPES)
// ******************************************************************************************

// An enumerated type that can be used to reference the button inputs
typedef enum
{
    KeyCode_None,
    KeyCode_UpArrow,            //!         Keyswitch 1
    KeyCode_DownArrow,          //!         Keyswitch 2
    KeyCode_Enter,              //!         Keyswitch 3,
    KeyCode_Cancel,             //!         Keyswitch 4
    KeyCode_Left_Right,         //!         Both arrows
    KeyCode_Left,               //!         Keyswitch 5, Left arrow
    KeyCode_Right,              //!         Keyswitch 6, Right arrow
    KeyCode_Num_Inputs_Plus_One //!         6 total buttons
} KeyCode_t;

// An enumerated type that can be used as the status of each button.
typedef enum
{
	RELEASED = 0,
	PRESSED,
	STILL_PRESSED					
} buttonStatus_t;


// A structure to contain the button status information.
// This structure is the form used with the fillLocalButtonInfo() function.  The first 
// element defines which button has been pressed, if any.  The second elements gives a 
// status update since the last time the fillLocalButtonInfo() function was called.  For
// example, assume BUTTON1 is pressed.  The first call of fillLocalButtonInfo() will 
// return BUTTON1 as buttonPressed and PRESSED as the buttonStatus.  The time for which 
// the button has been pressed (in milliseconds) will be returned as timePressed.  If
// BUTTON1 is held, the next call of fillLocalButtonInfo() will return BUTTON1 as 
// buttonPressed, STILL_PRESSED for buttonStatus, and the total time pressed for
// timePressed.  In general, the fillLocalButtonInfo() function will return the most 
// recent event since the last time it was called.  Multiple button presses are not
// supported.  Therefore, if Button B is pressed while Button A is held, Button B 
// will be ignored.  On the other hand, if Button A is held, then Button B is pressed 
// and held while Button A is released, Button B will register as PRESSED as soon as 
// button A is released.  A buttonStatus will never change from PRESSED to STILL_PRESSED
// without a call of fillLocalButtonInfo().  The fillLocalButtonInfo() function must be 
// called "fast enough" to capture all button presses.

typedef struct
{
	KeyCode_t buttonPressed;
	buttonStatus_t buttonStatus;
	uint16 timePressed;
} buttonInfo_t;


// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************

///------------------------------------------------------------------------------------------
//! \fn sint16 initInterface( void )
//! \brief  This function initializes the hardware and software necessary for interfacing with
//!         the user interface buttons on the front of the Advanced/Display Control Module enclosure.
//!         This function should be called at the component level only.
//!         The function call should be included in ComponentInit(). There
//!         is no need to call this function at the application level.
//!
//! \return A return value of BUTTON_INIT_ERROR implies that an error occurred during the
//!         initialization.  A return value of BUTTON_INIT_NO_ERROR indicates that the
//!         initialization was successful.
///------------------------------------------------------------------------------------------
sint16 buttonInterfaceInit( void );


///------------------------------------------------------------------------------------------
//! \fn sint16 ButtonInterfaceTaskInit( void )
//! \brief  This function is called at the component level from within buttonInterfaceInit() which
//!         is included in ComponentInit(). There is no need to call this function
//!         at the application level.
//! \return A return value of BUTTON_INIT_ERROR implies that an error
//!         occurred during task initialization (reserving the task and/or timer ID values).  A
//!         return value of BUTTON_INIT_NO_ERROR indicates that the initialization was successful.
//!         This function initializes a periodic task to scan and maintain the status of the
//!         various button inputs.
//!
//! \note   This task information must be included in main() just like any other task!!!
//!
//!         TASK_X_FUNCTION_NAME		ButtonInterfaceTask
//!         TASK_X_STACK_SIZE		************NEEDS DETERMINED************
//!         TASK_X_PRIORITY			Determined by the application, but the task should be able to
//!                                 run every 10ms
///------------------------------------------------------------------------------------------
sint16 ButtonInterfaceTaskInit( void );


///------------------------------------------------------------------------------------------
//! \fn fillLocalButtonInfo( buttonInfo_t *ptrlocalButtonInfo )
//! \brief  Calling this function will give the caller a button status update since the last time
//!         this function was called.  The caller must declare a local structure of type
//!         buttonInfo_t and pass a pointer to this structure as the parameter.  The function will
//!         fill the local structure with a copy of the structure maintained by the component
//!         level software.  
//! \param  ptrlocalButtonInfo  Local copy of buttonInfo_t structure to be filled in by the function
//!
//! \note   For an explanation of the structure, see the structure type
//!         definition above.  The fillLocalButtonInfo() function must be called "fast enough"
//!         to capture all button presses.
///------------------------------------------------------------------------------------------
void fillLocalButtonInfo( buttonInfo_t *ptrlocalButtonInfo );


#endif //!         BUTTON_INTERFACE_H

//@}
