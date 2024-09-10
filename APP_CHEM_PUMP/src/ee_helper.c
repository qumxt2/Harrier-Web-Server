// ee_helper.c

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

//
// Getter - Setter functions for things in EEPROM.
//

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "ee_helper.h"
#include "cpfuncs.h"
#include "eeprom.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************

#define MAX_EEPROM_WRITE_ATTEMPTS (5)
#define MAX_EEPROM_READ_ATTEMPTS (5)
#define EEPROM_DELAY_TICKS (5)

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************

bool ev_get_uint8( uint16 offset, uint8 *value )
{
	uint8 i;
	
	for( i=0; i<MAX_EEPROM_READ_ATTEMPTS; i++ )
	{
		if( EEPROM_ReadByte( offset, value ) == EEPROMError_Ok ) 
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}

bool ev_set_uint8( uint16 offset, uint8 value )
{
	uint8 i;
		
	for( i=0; i<MAX_EEPROM_WRITE_ATTEMPTS; i++ )
	{
		if( EEPROM_WriteByte( offset, value ) == EEPROMError_Ok )
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}

bool ev_get_uint16( uint16 offset, uint16 *value )
{
	uint8 i;
	
	for( i=0; i<MAX_EEPROM_READ_ATTEMPTS; i++ )
	{
		if( EEPROM_ReadWord( offset, value) == EEPROMError_Ok )
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}

bool ev_set_uint16( uint16 offset, uint16 value )
{
	uint8 i;
		
	for( i=0; i<MAX_EEPROM_WRITE_ATTEMPTS; i++ )
	{
		if( EEPROM_WriteWord( offset, value) == EEPROMError_Ok )
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}

bool ev_get_uint32( uint16 offset, uint32 *value )
{
	uint8 i;
	
	for( i=0; i<MAX_EEPROM_READ_ATTEMPTS; i++ )
	{
		if( EEPROM_ReadLongword( offset, value) == EEPROMError_Ok )
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}
bool ev_set_uint32( uint16 offset, uint32 value )
{
	uint8 i;
		
	for( i=0; i<MAX_EEPROM_WRITE_ATTEMPTS; i++ )
	{
		if( EEPROM_WriteLongword(offset, value) == EEPROMError_Ok )
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}

bool ev_get_string( uint16 offset, uint16 length, uint8* string )
{
	uint8 i;
	
	for( i=0; i<MAX_EEPROM_READ_ATTEMPTS; i++ )
	{
		if( EEPROM_ReadString( offset, length, string) == EEPROMError_Ok )
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}

bool ev_set_string( uint16 offset, uint16 length, uint8* string )
{
	uint8 i;
	
	for( i=0; i<MAX_EEPROM_WRITE_ATTEMPTS; i++ )
	{
		if( EEPROM_WriteString( offset, length, string) == EEPROMError_Ok )
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}

bool ev_get_datastore ( uint16 addr, uint8 depth, uint8 *output_value, uint8 len )
{
	uint8 i;
	
	for( i=0; i<MAX_EEPROM_READ_ATTEMPTS; i++ )
	{
		if( DATASTORE_RestoreData(addr,depth,output_value, len) == EEPROMError_Ok )
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}

bool ev_set_datastore ( uint16 addr, uint8 depth, uint8 *input_value, uint8 len )
{
    uint8 i;
	
	for( i=0; i<MAX_EEPROM_READ_ATTEMPTS; i++ )
	{
		if( DATASTORE_SaveData(addr, depth, input_value, len) == EEPROMError_Ok )
		{
			return FALSE;
		}
		
		(void)K_Task_Wait(EEPROM_DELAY_TICKS);
	}
	
	return TRUE;
}

// *****************************************************************************
// * PRIVATE FUNCTIONS
// *****************************************************************************

