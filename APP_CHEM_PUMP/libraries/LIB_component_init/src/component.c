// component.c
 
// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file includes all of the hardware and software initialization for the component 
// level of the Advanced Display Control Module (ADCM).  

// *********************************************************************************************
// * HEADER FILES
// *********************************************************************************************

#include "typedef.h"					// Compiler specific type definitions
#include "p32mx795f512l.h"				// Processor specific header file

#include "component.h"					// Component initialization constants and prototypes 
#include "syscfgid.h"					// Configuration ID prototypes and constants 
#include "graphics_interface.h"			// Graphic Interface prototypes and constants
#include "button_interface.h"			// Button Interface prototypes and constants
#include "i2c_channel2.h"				// I2C Communication prototypes and constants
#include "uC_peripheral_map.h"			// Switchable IO initialization macros
#include "gcaPortal_component.h"		// GCA Debug Portal component level prototypes

#include "bootload_host.h"              // Component Token Bootload

// #include any other component level library header files...

// *********************************************************************************************
// CONSTANTS AND MACROS
// *********************************************************************************************

// Set the component class ID
#define COMPCLASSID_HARRIER_PLUS	(0x0023)
_SYS_COMPONENT_CLASS_ID(COMPCLASSID_HARRIER_PLUS);	//lint !e19 Lint doesn't understnad this macro
#define DIGITAL_INPUT               (1)
#define DIGITAL_OUTPUT              (0)

// *********************************************************************************************
// * PRIVATE FUCTION PROTOTYPES
// *********************************************************************************************

// This function sets all pin configurations to a known state, disables all pin  peripherals 
// (CN, OC, IC, etc.), and shuts off all timers.  From this state, everything needed for 
// full component functionality is initialized/de-initialized as needed.  

static void setComponentConfiguration (void);
static void initializeDigitalInputs(void);
static void initializeDigitalOutputs(void);


// *********************************************************************************************
// * PUBLIC FUCTIONS
// *********************************************************************************************

// This function is called in main to initialize all component level resources
// and tasks.  The function performs the following - 
//
//  10. Set all pin configurations to a known state, disable all pin peripherals (CN, OC,
//		IC, etc.), and shutdown all timers
//	20. Initialize hardware and software for I2C communication
//	25. Initialize hardware and software for controlling the LCD backlight
//	3. Initialize hardware and software for interfacing with the LCD
//  4. Initialize hardware and software related to the Digital Inputs
//  5. Initialize hardware and software related to the Digital Outputs
//  6. Initialize hardware and software for reading analog inputs
//	7. Initialize hardware and software for reading the analog temperature and automatically
//		adjusting the LCD contrast
//  8. Initialize hardware and software for the user interface push buttons and front red
// 		warning LED
//  9. Initialize hardware and software for the H-bridge drive circuit
// 10. Initialize software for the component level portion of the CCA Portal

#include "debug.h"
sint16 ComponentInit (void)
{
	sint16 error = COMPONENT_INIT_OK;
	
	setComponentConfiguration();
	

	if (I2C2_Init() != 0)
    {
  		//error |= COMPONENT_INIT_I2C_ERROR;
        // Can be enaled when I2C pull-ups exist
    }

    initializeDigitalInputs();
    initializeDigitalOutputs();

	if(lcdInit() != 0)
	{
		error |= COMPONENT_INIT_LCD_ERROR;
	}

	backlightInit();


	if(buttonInterfaceInit() != 0)
	{
		error |= COMPONENT_INIT_BUTTON_ERROR;
	}

	//if( !TokenBootloadTask_Initialize() )
	//{
	//	error |= COMPONENT_INIT_TOKEN_LOAD_ERROR;
	//}


	GCAP_CompInterpreterInit();


	//******************************************************************************************
	
	if (error)
	{
		return error;
	}
	
    return COMPONENT_INIT_OK;
}

// *********************************************************************************************
// * PRIVATE FUCTIONS
// *********************************************************************************************

// This function sets all pin configurations to a known state, disables all pin  peripherals 
// (CN, OC, IC, etc.), and shuts off all timers.  From this state, everything needed for 
// full component functionality is initialized/de-initialized as needed.  Processor 
// configuration registers must maintain any states set by the Common level of code. 

static void setComponentConfiguration (void)
{

	AD1PCFGSET = 0x0000FFFF;				// Set AN0 through AN16 to digital mode.

	OC1CONbits.OCM = 0b000;							// Disable OC1
	OC2CONbits.OCM = 0b000;							// Disable OC2
	OC3CONbits.OCM = 0b000;							// Disable OC3
	OC4CONbits.OCM = 0b000;							// Disable OC4
	OC5CONbits.OCM = 0b000;							// Disable OC5


	IC1CONbits.ICM = 0b000;							// Disable IC1
	IC2CONbits.ICM = 0b000;							// Disable IC2
	IC3CONbits.ICM = 0b000;							// Disable IC3
	IC4CONbits.ICM = 0b000;							// Disable IC4
	IC5CONbits.ICM = 0b000;							// Disable IC5

	// Timer1 is used by the CMXTiny Operating System
	T2CONbits.TON = 0b0;							// Stop Timer2
	T3CONbits.TON = 0b0;							// Stop Timer3
	T4CONbits.TON = 0b0;							// Stop Timer4
	T5CONbits.TON = 0b0;							// Stop Timer5


}

static void initializeDigitalInputs(void)
{
    TRIS_INPUT_1 = DIGITAL_INPUT;
    TRIS_INPUT_2 = DIGITAL_INPUT;
    TRIS_INPUT_3 = DIGITAL_INPUT;
    TRIS_INPUT_4 = DIGITAL_INPUT;
    TRIS_TP6 = DIGITAL_INPUT;
}

static void initializeDigitalOutputs(void)
{
    TRIS_OUTPUT1 = DIGITAL_OUTPUT;
    LAT_OUTPUT1 = 0;

    TRIS_OUTPUT2 = DIGITAL_OUTPUT;
    LAT_OUTPUT2 = 0;

    TRIS_OUTPUT3 = DIGITAL_OUTPUT;
    LAT_OUTPUT3 = 0;

//    TRIS_OUTPUT4 = DIGITAL_OUTPUT;
//    LAT_OUTPUT4 = 0;

    TRIS_ALARM_LED = DIGITAL_OUTPUT;
    LAT_ALARM_LED = 0;

    TRIS_PUMP_LED = DIGITAL_OUTPUT;
    LAT_PUMP_LED = 0;

    TRIS_CYCLE_LED = DIGITAL_OUTPUT;
    LAT_CYCLE_LED = 0;

    TRIS_HEAT_EN = DIGITAL_OUTPUT;
    LAT_HEAT_EN = 0;

    TRIS_TP1 = DIGITAL_OUTPUT;
    LAT_TP1 = 1;

//    TRIS_RELAY_WATCHDOG = DIGITAL_OUTPUT;
//    LAT_RELAY_WATCHDOG = 0;
    
    TRIS_MODEM_RESET = DIGITAL_OUTPUT;
    LAT_MODEM_RESET = 1;

    TRIS_MOTOR_SELECT = DIGITAL_OUTPUT;
    LAT_MOTOR_SELECT = 1;

    TRIS_EXT_CONTROL_SELECT = DIGITAL_OUTPUT;
    LAT_EXT_CONTROL_SELECT = 1;

    TRIS_EXT_FEEDBACK_SELECT = DIGITAL_OUTPUT;
    LAT_EXT_FEEDBACK_SELECT = 1;
}
