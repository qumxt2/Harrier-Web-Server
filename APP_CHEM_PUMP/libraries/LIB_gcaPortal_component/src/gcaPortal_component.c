// gcaPortal_component.c

// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This module contains all of the function code for presenting a user-interactive 
// console on the debug serial port for the component level of the primary processor
// in the Display Control Module.  This console can be used for troubleshooting 
// purposes as well as for general testing.


//**********************************************************************************************
// HEADER FILES
//**********************************************************************************************
#include "typedef.h"						// Compiler specific type definitions

#include "stdlib.h"							// Standard library header file
#include "stdio.h"							// Standard I/O header file
#include "string.h"							// Standard string header file
#include "ccaportal.h"						// Function prototypes for the GCA Portal
#include "debug.h"							// Debug prototypes for the GCA Portal
#include "rtos.h"							// RTOS prototypes and constants
#include "gcaPortal_component.h"			// Prototypes for the component level
#include "io_pin.h"							// Switchable IO pin prototypes
#include "out_digital.h"					// Switchable IO digital output prototypes
#include "in_digital.h"						// Switchable IO digital input prototypes
#include "graphics_interface.h"				// Graphic Display control functions
#include "p32mx795f512l.h"



//**********************************************************************************************
// TYPE DEFINITIONS AND STRUCTURES
//**********************************************************************************************
typedef struct
{
	char *cmdString;
	void (*function)(int, char **);
} CommandTableEntry;


//**********************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
//**********************************************************************************************
static bool GCAP_CompInterpreter( int argc, char **argv );

static void HelpCommand( int argc, char **argv );
static void SetBacklightIntensity(int argc, char **argv);
static void IOPinList( int argc, char **argv );
static void IOFunctionList( int argc, char **argv );
static void IOGetPinFunction( int argc, char **argv );
static void IOSetPinFunction( int argc, char **argv );
static void ODSetLatch( int argc, char **argv );
static void ODGetLatch( int argc, char **argv );
static void ODGetFault( int argc, char **argv );
static void IDGetPin( int argc, char **argv );
static void __attribute__((nomips16))PicSoftReset( int argc, char **argv );



//**********************************************************************************************
// CONSTANTS AND MACROS
//**********************************************************************************************
static const CommandTableEntry CommandTable[] =
{
	{
		"?",
		HelpCommand
	},

	{
		"setbacklight",
		SetBacklightIntensity
	},
		
	{
		"iopins",
		IOPinList
	},

	{
		"iofuncs",
		IOFunctionList
	},
	
	{
		"iofset",
		IOSetPinFunction
	},

	{
		"iofget",
		IOGetPinFunction
	},

	{
		"odset",
		ODSetLatch
	},
	
	{
		"odget",
		ODGetLatch
	},
		
	{
		"odfault",
		ODGetFault
	},

	{
		"idget",
		IDGetPin
	},
	{
		"reset",
		// Using compiler funtion for processore reset, lint isn't aware
		/*lint -e611 */
		(void*)PicSoftReset
		/*lint -e611 */
	},	

	/* This MUST be the last entry... DO NOT REMOVE */
	{
		NULL,
		NULL
	}
};


//**********************************************************************************************
// PUBLIC FUCTIONS
//**********************************************************************************************
void GCAP_CompInterpreterInit (void)
{
	CCAPORTAL_RegisterCompCallback( GCAP_CompInterpreter );
}


//**********************************************************************************************
// PRIVATE FUNCTIONS
//**********************************************************************************************

static bool GCAP_CompInterpreter( int argc, char **argv )
{
	const CommandTableEntry *pEntry = &CommandTable[0];

	while( pEntry->cmdString != NULL )
	{
		/* Find a match between the first word and a command table entry */
		if( strcmp( argv[0], pEntry->cmdString ) == 0 )
		{
			(pEntry->function)( argc, argv );
			return TRUE;
		}
		pEntry++;
	}

	return FALSE;
}

//**********************************************************************************************

static void HelpCommand( int argc, char **argv )
{
	uint16 i;
	char *p;

	for( i = 0; sizeof(CommandTable) > i; i++)
	{
		p = CommandTable[i].cmdString; 		//lint !e662 !e661

		if (NULL != p)
		{
			(void)printf( "%s\n", p );
		}
		else
		{
			break;
		}
	}
}



//**********************************************************************************************

static void SetBacklightIntensity(int argc, char **argv)
{
	uint8 newIntensity;
	
	if (argc != 2)
	{
		(void)printf( "Command requires intensity setting (0-255)\n" );
	}
	else
	{
		newIntensity = atoi(argv[1]);
		setBacklightIntensity(newIntensity);
	}
}

//**********************************************************************************************

static void IOPinList( int argc, char **argv )
{
	IOPin_t pin;

	for( pin = (IOPin_t)0; pin < IOPIN_NUM_PINS; pin++)
	{
		(void)printf( "%2d --> %s\n", (uint8)pin, IOPin_NameString_Get( pin ) );
	}
}

//**********************************************************************************************

static void IOFunctionList( int argc, char **argv )
{
	IOFunction_t function;

	for( function = (IOFunction_t)0; function < IOFUNC_NUM_FUNCS; function++)
	{
		(void)printf( "%2d --> %s\n", (uint8)function, IOPin_FuncString_Get( function ) );
	}
}

//**********************************************************************************************

static void IOGetPinFunction( int argc, char **argv )
{
	IOPin_t pin;

	if( argc < 2 )
	{
		(void)printf("Command requires a pin ID:\n");
		(void)printf(" - Use command 'iopins' to show list of pin IDs\n");
		return;
	}

	pin = (IOPin_t)strtoul( argv[ 1 ], NULL, 0 );

	(void)printf( "%s: %s\n",
				  IOPin_NameString_Get( pin ),
				  IOPin_FuncString_Get(IOPin_Function_Get( pin ) )
				);
}

//**********************************************************************************************

static void IOSetPinFunction( int argc, char **argv )
{
	IOPin_t pin;
	IOFunction_t function;
	IOErrorCode_t error;

	if( argc < 3 )
	{
		(void)printf("Command requires a pin ID and a function ID:\n");
		(void)printf(" - Use command 'iopins' to show list of pin IDs\n");
		(void)printf(" - Use command 'iofuncs' to show list of function IDs\n");
		return;
	}

	pin = (IOPin_t)strtoul( argv[ 1 ], NULL, 0 );
	function = (IOFunction_t)strtoul( argv[ 2 ], NULL, 0 );
	
	//if((function == IOFUNC_IN) || (function == IOFUNC_OUT))
	//{
	//	(void)printf("High-speed functions not available on ADM\n");
	//	return;
	//}	
	
	error = IOPin_Function_Set( pin, function );
	if( error != IOError_Ok )
	{
		(void)printf( "error: %d\n", (uint8)error );
	}
	else
	{
		(void)printf( "%s: %s\n",
					  IOPin_NameString_Get( pin ),
					  IOPin_FuncString_Get( function )
					);
	}
}

//**********************************************************************************************

static void ODSetLatch( int argc, char **argv )
{
	IOPin_t pin;
	IOState_t state;
	IOErrorCode_t error;

	if( argc < 3 )
	{
		(void)printf("Command requires a pin ID and an output state:\n");
		(void)printf(" - Use command 'iopins' to show list of pin IDs\n");
		(void)printf(" - 0 -> NOT_ASSERTED, 1 -> ASSERTED\n");
		return;
	}

	pin = (IOPin_t)strtoul( argv[ 1 ], NULL, 0 );
	state = (IOState_t)strtoul( argv[ 2 ], NULL, 0 );
	
	error = OUT_Digital_Latch_Set( pin, state );
	if( error != IOError_Ok )
	{
		(void)printf( "error: %d\n", (uint8)error );
	}
	else
	{
		(void)printf( "%s: %s\n",
					  IOPin_NameString_Get( pin ),
					  ((state == NOT_ASSERTED) ? "NOT_ASSERTED" : "ASSERTED" )
					);
	}
}

//**********************************************************************************************

static void ODGetLatch( int argc, char **argv )
{
	IOPin_t pin;
	IOrtn_digital_t iortn;

	if( argc < 2 )
	{
		(void)printf("Command requires a pin ID:\n");
		(void)printf(" - Use command 'iopins' to show list of pin IDs\n");
		return;
	}

	pin = (IOPin_t)strtoul( argv[ 1 ], NULL, 0 );

	iortn = OUT_Digital_Latch_Get( pin );
	if( iortn.error != IOError_Ok )
	{
		(void)printf( "error: %d\n", (uint8)iortn.error );
	}
	else
	{
		(void)printf( "%s: %s\n",
					  IOPin_NameString_Get( pin ),
					  ((iortn.state == NOT_ASSERTED) ? "NOT_ASSERTED" : "ASSERTED" )
					);
	}
}

//**********************************************************************************************

static void ODGetFault( int argc, char **argv )
{
	IOPin_t pin;
	IOrtn_digital_t iortn;

	if( argc < 2 )
	{
		(void)printf("Command requires a pin ID:\n");
		(void)printf(" - Use command 'iopins' to show list of pin IDs\n");
		return;
	}

	pin = (IOPin_t)strtoul( argv[ 1 ], NULL, 0 );

	iortn = OUT_Digital_Fault_Get( pin );
	if( iortn.error != IOError_Ok )
	{
		(void)printf( "error: %d\n", (uint8)iortn.error );
	}
	else
	{
		(void)printf( "%s: %s\n",
					  IOPin_NameString_Get( pin ),
					  ((iortn.state == DIGOUT_NOFAULT) ? "NO FAULT" : "FAULT" )
					);
	}
}

//**********************************************************************************************

static void IDGetPin( int argc, char **argv )
{
	IOPin_t pin;
	IOrtn_digital_t iortn;

	if( argc < 2 )
	{
		(void)printf("Command requires a pin ID:\n");
		(void)printf(" - Use command 'iopins' to show list of pin IDs\n");
		return;
	}

	pin = (IOPin_t)strtoul( argv[ 1 ], NULL, 0 );

	iortn = IN_Digital_State_Get( pin );
	if( iortn.error != IOError_Ok )
	{
		(void)printf( "error: %d\n", (uint8)iortn.error );
	}
	else
	{
		(void)printf( "%s: %s\n",
					  IOPin_NameString_Get( pin ),
					  ((iortn.state == NOT_ASSERTED) ? "NOT_ASSERTED" : "ASSERTED" )
					);
	}
}

//**********************************************************************************************

// Using compiler __attribute__ function and ASR call
// lint isn't aware how to interpret these
/*lint -e529*/

static void __attribute__((nomips16)) PicSoftReset( int argc, char **argv )
{
	//int	intStat;
	//unsigned int status = 0;
	//volatile uint8* p = &RSWRST;

	//mSYSTEMUnlock(intStat, dmaSusp);
	do
	{
		//asm volatile("di    %0" : "=r"(status));
		//intStat = status;
        K_OS_Disable_Interrupts();
		SYSKEY = 0, SYSKEY = 0xAA996655, SYSKEY = 0x556699AA;
	} while(0);

	RSWRSTSET=_RSWRST_SWRST_MASK;
	RSWRST; //*p;

	while(1);
}
/*lint +e529*/

//**********************************************************************************************


