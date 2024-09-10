// application.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file initializes all application level resources and tasks associated with the 
// development of the graphics processor.  The application level GCA Portal
// functionality is also contained in this file.  

//**********************************************************************************************
// HEADER FILES
// *********************************************************************************************

#include "typedef.h"                    // Compiler specific type definitions
#include "application.h"                // Application initialization constants and prototypes 
#include "rtcTask.h"
#include "ccaportal.h"
#include "string.h"
#include "stdio.h"
#include "button_interface.h"
#include "eeprom.h"
#include "dvar.h"
#include "pumpControlTask.h"
#include "pumpControlTask.h"
#include "inputDebounceTask.h"
#include "stdlib.h"
#include "volumeTask.h"
#include "dvseg_17G721_setup.h"
#include "dvseg_17G721_run.h"
#include "rtos.h"
#include "modemTask.h"
#include "serial_uart3a.h"
#include "socketModem.h"
#include "d2a.h"
#include "screensTask.h"
#include "in_voltage.h"
#include "alarms.h"
#include "in_pressure.h"
#include "temperature.h"
#include "modbus_task.h"
#include "tokfs.h"
#include "CountDigit.h"

// *********************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// *********************************************************************************************

static bool PrimaryPortalAppInterpreter(int argc, char **argv);
static void HelpCommand( int argc, char **argv );
static void ReadToken( int argc, char **argv );
static void PumpRunMode( int argc, char **argv );
static void PumpStandbyMode( int argc, char **argv );
static void fakeReed( int argc, char **argv );
static void timeMode( int argc, char **argv );
static void cycleMode( int argc, char **argv );
static void volumeMode( int argc, char **argv );
static void ModemPortal( int argc, char **argv );
static void setDac( int argc, char **argv );
static void getAdc( int argc, char **argv );
static void alarmControl( int argc, char **argv );
static void screenTemperature( int argc, char **argv );
static void PressureInPSI( int argc, char **argv );
static void PressureInBAR( int argc, char **argv );
static void InVoltageDifferential( int argc, char **argv );
static void setTestOverride( int argc, char **argv );

// *********************************************************************************************
// TYPE DEFINITIONS AND CONSTANTS
// *********************************************************************************************
#define	CONSOLE_EVENT_INPUT_SERIAL3A	(RTOS_EVENT_FLAG_2)

typedef struct
{
    char *cmdString;
    void (*function)(int, char **);
} CommandTableEntry;

static const CommandTableEntry CommandTable[] =
{
    {
        "?",
        HelpCommand
    },

    {
        "readtoken",
        ReadToken
    },
    
    {
        "run",
        PumpRunMode
    },
        
    {
        "standby",
        PumpStandbyMode
    },

    {
        "fakeReed",
        fakeReed
    },

    {
        "timeMode",
         timeMode
    },

    {
        "cycleMode",
         cycleMode
    },

    {
        "volumeMode",
         volumeMode
    },

    {
        "modem",
        ModemPortal,
    },

    {
        "dac",
        setDac,
    },

    {
        "adc",
        getAdc,
	},

    {
	    "alarm",
        alarmControl,
    },

    {
	    "PSI",
        PressureInPSI,
    },

    {
	    "BAR",
        PressureInBAR,
    },

    {
        "temperature",
        screenTemperature,
    },

	{
		"ivdiff",
		InVoltageDifferential
	},
    
	{
		"testOverride",
        setTestOverride
	},    

    /* This MUST be the last entry... DO NOT REMOVE */
    {
        NULL,
        NULL
    }
};


//**********************************************************************************************
// PUBLIC FUNCTIONS
// *********************************************************************************************

sint16 ApplicationInit(void)
{
    sint16 error = APPLICATION_INIT_OK;
    
    (void)screensTaskInit();
    (void)rtcTaskInit();
    (void)inputDebounceTaskInit();
    (void)pumpControlTaskInit();
    (void)volumeTaskInit();

    (void)modemTaskInit();
    (void)MODBUS_Task_Initialize();
    (void)MODBUS_Task_Start();
 
    CCAPORTAL_RegisterAppCallback(PrimaryPortalAppInterpreter);

    return error;
}


// *********************************************************************************************
// PRIVATE FUNCTIONS
// *********************************************************************************************

static bool PrimaryPortalAppInterpreter(int argc, char **argv)
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

// *********************************************************************************************

static void HelpCommand( int argc, char **argv )
{
    uint16 i;
    char *p;

    for( i = 0; sizeof(CommandTable) > i; i++)
    {
        p = CommandTable[i].cmdString; //lint !e662 !e661

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

// *********************************************************************************************

static void ReadToken( int argc, char **argv )
{
#define RECORD_BUFFER_SIZE		(32)
    uint8 recordBuffer[RECORD_BUFFER_SIZE];
    RecordLength length;
    sint16 error;
    error = TOKFS_Init( );
    error |= (sint16)TOKFS_NextFile( );
    error |= (sint16)TOKFS_NextRecord( );
    error |= (sint16)TOKFS_GetRecordLength( &length );

    if( ((TOKFSErrorCode)error == TOKFSError_Ok) && ( length < RECORD_BUFFER_SIZE ) )
    {
        (void)TOKFS_ImportRecord( recordBuffer, &length );
        // we're going to try printing it as a string, so make
        // sure whatever we got is null terminated
        recordBuffer[ (RECORD_BUFFER_SIZE - 1) ] = '\0';
        (void)printf( "%s\n", recordBuffer );
    }
    else
    {
        (void)printf( "FAIL - Read Token\n" );
    }
}

// *********************************************************************************************

static void PumpRunMode( int argc, char **argv )
{
    PMP_setRunMode();;
}

// *********************************************************************************************

static void PumpStandbyMode( int argc, char **argv )
{
    PMP_setStandbyMode();;
}

// *********************************************************************************************

static void fakeReed( int argc, char **argv )
{
    uint8 isEnabled;
    
    if (argc != 2)
    {
        (void)printf( "Command requires argument:  0-disabled, 1-enabled\n" );
        return;
    }

    isEnabled = atoi(argv[1]);
    
    if ( (isEnabled != 1) && (isEnabled != 0) ) 
    {
        (void)printf( "Command requires argument:  0-disabled, 1-enabled\n" );
    }
    else
    {
        isEnabled = atoi(argv[1]);
        VOL_fakeReedSwitch(isEnabled);
    }
}

// *********************************************************************************************

static void timeMode( int argc, char **argv )
{
    if (argc != 3)
    {
        (void)printf( "Command requires 2 arguments: OnTime, Offtime\n" );
        (void)printf( "Ex:  timeMode 10 10\n" );
        return;
    }

    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OnTime), (DistVarType)atoi(argv[1]));
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OffTime), (DistVarType)atoi(argv[2]));
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MeteringMode), (DistVarType)1);

    PMP_resetStates();
}

// *********************************************************************************************

static void cycleMode( int argc, char **argv )
{
    if (argc != 3)
    {
        (void)printf( "Command requires 2 arguments: OnCycles, Offtime\n" );
        (void)printf( "Ex:  cycleMode 10 10\n" );
        return;
    }

    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OnCycles), (DistVarType)atoi(argv[1]));
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, OffTime), (DistVarType)atoi(argv[2]));
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MeteringMode), (DistVarType)2);

    PMP_resetStates();
}

// *********************************************************************************************

static void volumeMode( int argc, char **argv )
{
    if (argc != 2)
    {
        (void)printf( "Command requires 1 argument: DesiredFlowRate (x100)\n" );
        (void)printf( "Ex:  volumeMode 50\n" );
        return;
    }

    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, DesiredFlowRate), (DistVarType)atoi(argv[1]));
    (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, MeteringMode), (DistVarType)0);

    PMP_resetStates();
}

// *********************************************************************************************

static void ModemPortal( int argc, char **argv )
{
    const char infoCommand[] = "info";
    const char terminalCommand[] = "terminal";
    const char snoopCommand[] = "snoop";
    const char onArg[] = "on";
    const char offArg[] = "off";
    uint8 inputCharacter;
    bool run = TRUE;

    if (argc < 2)
    {
        (void)printf("See modem and network connection information, or start an interactive terminal.\n\n");
        (void)printf("\t%s\t\tShow modem and network information\n", infoCommand);
        (void)printf("\t%s\tStart dumb terminal to modem\n", terminalCommand);
        (void)printf("\t%s\tSnoop the data coming to and going from the modem (on/off)\n", snoopCommand);
        return;
    }

    if (strcmp(argv[1], infoCommand) == 0)
    {
        (void)printf("IP address: %u.%u.%u.%u\n",
            (uint8)((gRun.IpAddress >> 24) & 0xFF),
            (uint8)((gRun.IpAddress >> 16) & 0xFF),
            (uint8)((gRun.IpAddress >> 8) & 0xFF),
            (uint8)((gRun.IpAddress >> 0) & 0xFF));
        (void)printf("Connection status: %s\n", ConnectionStatusText[gRun.ConnectionStatus]);
    }
    else if (strcmp(argv[1], terminalCommand) == 0)
    {
        (void)printf("\n***Entering terminal mode (allow several seconds for first response). Press <ctrl>-c to quit.\n");
        MODEM_DebugEcho(TRUE);

        while (run)
        {
            (void)K_Event_Wait( CONSOLE_EVENT_INPUT_SERIAL3A, 100, RTOS_CLEAR_EVENT_FLAGS_AFTER );
            while( run && Serial3ARx( (uint8 *)&inputCharacter ) )
            {
                if( inputCharacter == 0x03 )		// CTRL-C character
                {
                    MODEM_DebugEcho(FALSE);
                    (void)printf("\n***Exiting terminal mode.\n");
                    run = FALSE;
                }
                else
                {
                    (void)MODEM_SendStringTerminal(&inputCharacter, 1);
                }
            }
        }
    }
    else if (strcmp(argv[1], snoopCommand) == 0)
    {
        if (argc < 3)
        {
            (void)printf("Snoop takes an argument: %s | %s\n", onArg, offArg);
        }
        else if (strcmp(argv[2], onArg) == 0)
        {
            MODEM_Snoop(TRUE);
        }
        else if (strcmp(argv[2], offArg) == 0)
        {
            MODEM_Snoop(FALSE);
        }
    }
    else
    {
        (void)printf("Invalid argument\n");
    }
}

// *********************************************************************************************

static void setDac( int argc, char **argv )
{
    uint32 channel;
    uint32 output;

    if (argc < 3)
    {
        (void)printf( "Command requires a channel and a voltage in mV (0-2500)\n");

        return;
    }

    channel = atoi(argv[1]);
    output = atoi(argv[2]);

    if (output > 2500)
    {
        output = 2500;
    }
    if (channel >= D2A_NUM_CHANNELS)
    {
        (void)printf( "Invalid channel.  Select channel 0 - 7\n");
    }

    (void)D2A_Output_Set(channel, (D2A_FULLSCALE * output) / 2500);
}

// *********************************************************************************************


static void getAdc( int argc, char **argv )
{
    uint32 channel;
    IOrtn_uint16_t value;

    if (argc != 2)
    {
        (void)printf( "Command requires a channel (0-7)\n" );
        return;
    }

    channel = atoi(argv[1]);

    value = IN_A2D_Get(channel);

    if (channel == BATTERY_ADC_CHANNEL)
    {
        (void)printf("Value: %u (%u mV)\n", value.u16, (value.u16*BATTERY_ADC_MULTIPLIER/BATTERY_ADC_DIVISOR)+BATTERY_OFFSET_MV);
    }
    else
    {
        (void)printf("Value: %u\n", value.u16);
    }

}


// *********************************************************************************************

static void alarmControl( int argc, char **argv )
{
    alarm_id_t alarmId = ALARM_ID_Unknown;
    uint8 idx = 0;
    bool alarmState = FALSE;
    const char commandSet[] = "set";
    const char commandClear[] = "clear";
    char* pState;

    if (argc < 3)
    {
        (void)printf( "Usage:\n");
        (void)printf( "\t%s <alarm_id> (%s|%s)\n\n", argv[0], commandSet, commandClear );
        (void)printf( "Known alarm IDs, states, and names:\n\n");

        for (idx = ALARM_ID_Unknown + 1; idx < ALARM_ID_NUM_ALARMS; idx++)
        {
            alarmState = ALARM_AlarmState(idx);
            pState = alarmState ? (char*)commandSet : (char*)commandClear;
            (void)printf( "%d - %s\t\t%s\n", idx, pState, ALARM_alarmNames[idx]);
        }

        return;
    }

    alarmId = atoi(argv[1]);
    if (alarmId <= ALARM_ID_Unknown || alarmId >= ALARM_ID_NUM_ALARMS)
    {
        (void)printf( "Error: invalid alarm ID\n");
        return;
    }

    if (strcmp(commandSet, argv[2]) == 0 )
    {
        (void)ALARM_ActivateAlarm(alarmId);
    }
    else if (strcmp(commandClear, argv[2]) == 0)
    {
        (void)ALARM_CancelAlarm(alarmId);
    }
    else
    {
        (void)printf( "Error: invalid argument\n");
    }

    return;
}

// *********************************************************************************************

static void screenTemperature( int argc, char **argv )
{
    (void)printf("Temperature: %d degrees C\n", TEMPERATURE_ReadDegreesC());

    return;
}



static void PressureInPSI( int argc, char **argv )
{
    uint32 channel;
    IOrtn_mV_to_psi_s16d16_t Pressure;

    if (argc != 2)
    {
        (void)printf("Command requires a channel (0-1)\n");
        return;
    }
    
    channel = atoi(argv[1]);
    
    Pressure = GetPressurePSI(channel);
    
    if (channel == IOHARDWARE_PRESSURESENSOR_A)
    {
        (void)printf("Pressure in PSI: %lu\n", Pressure.psi_s16d16);
    }
    else
    {
        (void) printf("Pressure in PSI: %lu\n", FixedPointToInteger(Pressure.psi_s16d16, DECIMAL_PLACE_THREE));
    }
    
}

static void PressureInBAR( int argc, char **argv )
{
    uint32 channel;
    IOrtn_mV_to_bar_s16d16_t Pressure;
    
    if (argc != 2)
    {
        (void)printf("Command requires a channel (0-1)\n");
        return;
    }
    
    channel = atoi(argv[1]);
    
    Pressure = GetPressureBar(channel);
    
    if (channel == IOHARDWARE_PRESSURESENSOR_A)
    {
        (void)printf("Pressure in PSI: %lu\n", Pressure.bar_s16d16);
    }
    else
    {
        (void) printf("Pressure in PSI: %lu\n", FixedPointToInteger(Pressure.bar_s16d16, DECIMAL_PLACE_THREE));
    }

}

static void InVoltageDifferential( int argc, char **argv )
{
	IOrtn_mV_s16d16_t iortn;
    const uint8 PRESSURE_CHANNEL = 0;

	iortn = IN_Voltage_Pressure_Get_Diff( PRESSURE_CHANNEL );
	(void)printf( "Differential Input: %d.%u mV\n",
				  (uint16)(iortn.mV_s16d16 >> 16),
				  (uint16)(((iortn.mV_s16d16 & 0x0000FFFF) * 10000) / 65536)
				);
//  ToDO: Check to see if sensor detect returns an error.  portal says "error:4"
//	if( iortn.error != IOError_Ok )
//	{
//		(void)printf( "error: %d\n", (uint8)iortn.error );
//	}
}

static void setTestOverride( int argc, char **argv )
{
    uint32 override;

    if (argc != 2)
    {
        (void)printf( "Override: %d\r\n", gRun.TestOverride);
        (void)printf( "Command requires argument:  0-disabled, 1-enabled\n" );
        return;
    }

    override = atoi(argv[1]);

    if(override < 2)
    {
        (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gRun, TestOverride), (DistVarType)override);    
    }
    
    if (gRun.TestOverride == 1)
    {   
        (void)printf("Override On\r\n");
    }
    else if (gRun.TestOverride == 0)
    {
        (void)printf("Override Off\r\n");
    }
}

