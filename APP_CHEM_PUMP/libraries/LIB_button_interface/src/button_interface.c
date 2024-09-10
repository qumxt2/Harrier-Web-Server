// button_interface.c

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved


// ******************************************************************************************
// * HEADER FILES
// ******************************************************************************************

#include "typedef.h"				// Compiler specific type definitions
#include "p32mx795f512l.h"			// Processor specific header file
#include "rtos.h"					// RTOS function prototypes

#include "debug.h"					// Function prototypes for the CCA Debug Portal

#include "button_interface.h"	// Button interface function prototypes and type definitions


// ******************************************************************************************
// * CONSTANTS
// ******************************************************************************************

#define	INPUT_SCAN_FREQUENCY		(100)						// frequency of digital input scans (in Hertz)
#define INPUT_SCAN_PERIOD_ms    	(1000/INPUT_SCAN_FREQUENCY) // period of digital input scans (in milliseconds)

#define BUTTON_DEBOUNCE_ms			(20)                        // Button input debounce period (in milliseconds)

#define BUTTON_INTERFACE_TIMER_EVENT_FLAG	(RTOS_EVENT_FLAG_1)


// ******************************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ******************************************************************************************
// the Task function is not declared "static" because the RTOS task list needs to reference it.
void ButtonInterfaceTask (void);

static uint8 readButtonInput (KeyCode_t membraneButtonName);

// ******************************************************************************************
// * PRIVATE VARIABLES - GLOBAL TO THIS FILE
// ******************************************************************************************
static KeyCode_t gActiveButton;										// ID of currently active (pressed) button

static bool gActiveButtonChanged;									// flag that is set by DigitalInTask to indicate
																	//  that a change has occured since the last time
																	//  this flag was cleared.
static uint16 gActiveButtonTime_ms;									// Amount of time in mS that active button has
                                                                    // active (held down)
static uint8 gButtonDebouncePeriod[KeyCode_Num_Inputs_Plus_One];    // amount of time (in RTOS task iterations) to
                                                                    // debounce membrane switch button inputs

static uint8 gButtonInterfaceTaskID;                                // Global variable to this file, store the task ID
static uint8 gButtonInterfaceTaskTimerID;                           // Global variable to this file, store the task timer ID

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************


// This function initializes the hardware necessary for interfacing with the user
// interface buttons  This function should be called at the 
// component level only.  The function call should be included in ComponentInit(). There 
// is no need to call this function at the application level.  

sint16 buttonInterfaceInit (void)
{
	sint16 status;
	
	_TRISB0 = 1;		// Set keyswitch #1 to an input
	_TRISB1 = 1;		// Set keyswitch #2 to an input
	_TRISB2 = 1;		// Set keyswitch #3 to an input
	_TRISB3 = 1;		// Set keyswitch #4 to an input
	_TRISB4 = 1;		// Set keyswitch #5 to an input
	_TRISB5 = 1;		// Set keyswitch #6 to an input
	
	status = ButtonInterfaceTaskInit();		// Initialize the ButtonInterface task
	if (status == BUTTON_INIT_ERROR)
    {
    	return BUTTON_INIT_ERROR;				// Initialization error
    }
    
    return BUTTON_INIT_NO_ERROR;				// No initialization error
}

//************************************************************************************************
//************************************************************************************************

// This function should be called at the component level only.  The function call 
// should be included in ComponentInit(). There is no need to call this function 
// at the application level.  A return value of BUTTON_INIT_ERROR implies that an error
// occurred during task initialization (reserving the task and/or timer ID values).  A
// return value of BUTTON_INIT_NO_ERROR indicates that the initialization was successful.
// This function initializes a periodic task to scan and maintain the status of the 
// various button inputs.  The task information must be included in main() just like
// any other task.  

// TASK_X_FUNCTION_NAME		ButtonInterfaceTask
// TASK_X_STACK_SIZE		************NEEDS DETERMINED************
// TASK_X_PRIORITY			Determined by the application, but the task should be able to 
//							run every 10ms

sint16 ButtonInterfaceTaskInit (void)
{
	uint8 index;	

	// Button input hardware configured in ComponentInit()

	// initialize default values for buttons
	gActiveButton = KeyCode_None;
	gActiveButtonTime_ms = 0;
	gActiveButtonChanged = TRUE;
	for (index = 0; index < (uint8)KeyCode_Num_Inputs_Plus_One; index++)
	{
		gButtonDebouncePeriod[index] = BUTTON_DEBOUNCE_ms / INPUT_SCAN_PERIOD_ms;
	}
	
	gButtonInterfaceTaskTimerID = RtosTimerReserveID();			// Reserve task timer ID
	if (gButtonInterfaceTaskTimerID == RTOS_INVALID_ID)
	{
		return BUTTON_INIT_ERROR;
	}
	
	gButtonInterfaceTaskID = RtosTaskCreateStart(ButtonInterfaceTask);	// Create and trigger the ButtonInterfaceTask
	if (gButtonInterfaceTaskID == RTOS_INVALID_ID)
	{
		return BUTTON_INIT_ERROR;
	}
	
	return BUTTON_INIT_NO_ERROR;
}


//************************************************************************************************
//************************************************************************************************


// Calling this function will give the caller a button status update since the last time
// this function was called.  The caller must declare a local structure of type
// buttonInfo_t and pass a pointer to this structure as the parameter.  The function will
// fill the local structure with a copy of the structure maintained by the component
// level software.

void fillLocalButtonInfo (buttonInfo_t *ptrlocalButtonInfo)
{
	static KeyCode_t previousActiveButton;
	static bool	localActiveButtonChanged;

	// Lock down the OS so that we can get a copy of the currently active button
	K_Task_Lock();
		ptrlocalButtonInfo->buttonPressed = gActiveButton;
		ptrlocalButtonInfo->timePressed = gActiveButtonTime_ms;
		localActiveButtonChanged = gActiveButtonChanged;	// get the changed status
		gActiveButtonChanged = FALSE;						// and reset the changed flag
	K_Task_Unlock();

	// assume no button is pressed...
	ptrlocalButtonInfo->buttonStatus = RELEASED;
	// check if our assumption was true
	if (ptrlocalButtonInfo->buttonPressed != KeyCode_None)
	{
		// check if the changed flag has been set or if this is a different 
		// button than was active the last time someone checked
		if ( localActiveButtonChanged ||
			 (ptrlocalButtonInfo->buttonPressed != previousActiveButton) )
		{
			// changed occured from last time, report pressed
			ptrlocalButtonInfo->buttonStatus = PRESSED;
		}
		else
		{
			// nope, same button, so report still pressed
			ptrlocalButtonInfo->buttonStatus = STILL_PRESSED;
		}
	}
	
	// record the currently active button for next time we're called...
	previousActiveButton = ptrlocalButtonInfo->buttonPressed;
}	


// ******************************************************************************************
// * PRIVATE FUNCTIONS
// ******************************************************************************************

// This function reads the pin input value for a given button input and returns that 
// value (either 0 (pressed) or 1 (not pressed)).    


//  **Changed to reflect the GLC4400 keyswitches**
static uint8 readButtonInput (KeyCode_t buttonName)
{
	uint8 buttonValue;
	
	// assume button is not pressed
	buttonValue = 1;

	switch(buttonName)
	{
        case KeyCode_UpArrow:
            buttonValue = _RB0;
            break;

        case KeyCode_DownArrow:
            buttonValue = _RB1;
            break;

        case KeyCode_Enter:
            buttonValue = _RB2;
            break;

        case KeyCode_Cancel:
            buttonValue = _RB3;
            break;

        case KeyCode_Left:
            buttonValue = _RB4 || !_RB5;
            break;

        case KeyCode_Right:
            buttonValue = _RB5 || !_RB4;
            break;

        case KeyCode_BothArrows:
            buttonValue = _RB4 || _RB5;
            break;

        case KeyCode_None:
        case KeyCode_Num_Inputs_Plus_One:
        default:
            break;
    }
    
    return buttonValue; 
}

//       **End Change**


//************************************************************************************************
//************************************************************************************************


// This task runs in the background at the component level to scan the button inputs.

void ButtonInterfaceTask (void)
{
	static uint8 status;
	static uint8 index;	

	static KeyCode_t currentButton;
	static uint32 currentButtonTime;

	static bool allButtonsReleased = TRUE;
	static bool localActiveButtonChanged = FALSE;
	
	static uint16 buttonPressedCount[KeyCode_Num_Inputs_Plus_One];		// time that each membrane switch 
																		// button has been pressed (in task
																		//  loop iterations)

	// Reset buttonPressedCount to 0 for all buttons
	for (index = 0; index < (uint8)KeyCode_Num_Inputs_Plus_One; index++)
	{
		buttonPressedCount[index] = 0;
	}	

	// Set-up a cyclic timer to signal this task
	status = K_Timer_Create(gButtonInterfaceTaskTimerID,0,gButtonInterfaceTaskID,BUTTON_INTERFACE_TIMER_EVENT_FLAG);		

	// Set-up tmr. for immediate start and reoccur at 100Hz rate 
	status |= K_Timer_Start(gButtonInterfaceTaskTimerID,1,(RtosGetTickFreq() / INPUT_SCAN_FREQUENCY));
	if (status != K_OK)
	{
		DEBUG_PRINT_STRING(DBUG_ALWAYS, "BUTTON INTERFACE TASK TIMER ERROR!");
		K_Task_End();
	}


	for (;;)
	{
		status = K_Event_Wait(BUTTON_INTERFACE_TIMER_EVENT_FLAG,0,2);	// Task event index 1, wait 0 ticks (forever), 
																		// clear event flag at the end
		
		// Loop through all of the buttons to get the current button state
		for (index = 1; index < (int)KeyCode_Num_Inputs_Plus_One; index++)
		{
			// check if button is currently pressed (return value of 0 means pressed)
			if (!readButtonInput((KeyCode_t)index))
			{
				// this button is currently pressed, so increment this button's pressed count
				// but make sure it doesn't roll over
				if (buttonPressedCount[index] != 0xFFFF)
				{
					buttonPressedCount[index]++;
				}
			}
			else
			{
				// this button is not currently pressed, so reset this button's pressed count to 0
				buttonPressedCount[index] = 0;
			}
		}

		// check if there was NOT and active button last time through
		//  -OR-
		// check if the previously active button isn't pressed anymore
		if ( (gActiveButton == KeyCode_None) ||
			 (buttonPressedCount[gActiveButton] <= gButtonDebouncePeriod[gActiveButton]) )
		{
			localActiveButtonChanged = TRUE;
			// we need to try and find ourselves a new button...
			index = 1;
			currentButton = KeyCode_None;
			// search for a button that has been debounced...
			while ( (currentButton == KeyCode_None) && 
					(index < (int)KeyCode_Num_Inputs_Plus_One) )
			{
				if (buttonPressedCount[index] >= gButtonDebouncePeriod[index])
				{
					currentButton = (KeyCode_t)index;
				}
				else
				{
					index++;
				}
			} 			
			// check if ALL the buttons are released
			if (currentButton == KeyCode_None)
			{
				allButtonsReleased = TRUE;
			}
			// A button is currently pressed, check if all the buttons were released last time through. 
			// * This test will force all the buttons to be released before any new button presses are recognized *
			else if (!allButtonsReleased)
			{
				// force the reported active button to be KeyCode_None
				currentButton = KeyCode_None;
				allButtonsReleased = FALSE;
			}
			// otherwise, we have a new, valid button press...
			else
			{
				allButtonsReleased = FALSE;
			}
		}				
		// otherwise, the same button as last time is still active.
		else
		{
			localActiveButtonChanged = FALSE;
			currentButton = gActiveButton;
		}

		// calculate the amount of time that the current button has been active (if there is an active button)
		if (currentButton != KeyCode_None)
		{
			currentButtonTime = INPUT_SCAN_PERIOD_ms * (uint32)buttonPressedCount[currentButton];
			if (currentButtonTime > 0xFFFF)
			{
				currentButtonTime = 0xFFFF;
			}
		}
		else
		{
			currentButtonTime = 0;
		}

		// Lock down the operating system while we make any necessary changes to the active button info
		K_Task_Lock();
			gActiveButton = currentButton;
			gActiveButtonTime_ms = (uint16)currentButtonTime;
			gActiveButtonChanged = gActiveButtonChanged || localActiveButtonChanged;
		K_Task_Unlock();

	} 	// end for(;;) (the endless loop for the task)

} 	// end ButtonInterfaceTask
