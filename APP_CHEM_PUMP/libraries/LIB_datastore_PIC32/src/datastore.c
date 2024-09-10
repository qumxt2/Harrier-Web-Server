// datastore.c

// Copyright 2009
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// DESCRIPTION

#include "typedef.h"
#include "rtos.h"
#include "debug.h"

#ifdef ADM_GRAPHICS_PROCESSOR
#include "ipcGraphicsEeprom.h"
#else
#include "eeprom.h"
#endif

#include "datastore.h"


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************


// ***************************************************
// * CONSTANTS
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************
#define FRESHNESS_BYTE_OFFSET			(0)
#define CHECKSUM_BYTE_OFFSET			(1)
#define DATA_BYTE_OFFSET				(3)


// ***************************************************
// * PRIVATE VARIABLES
// ***************************************************


// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************
static uint8 FindFreshestBackupCell( uint16 offset, uint8 backup_depth, uint16 data_bytecount, uint8 *freshness );

static bool VerifyBackupCellChecksum( uint16 offset, uint8 backup_cell_index, uint16 data_bytecount );


// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************

EEPROMErrorCode DATASTORE_RestoreData( uint16 offset, uint8 backup_depth,
									   uint8 *data, uint16 data_bytecount  )
{
	uint8 stale_count;

	return DATASTORE_RestoreData_FreshInfo( offset, backup_depth, data, data_bytecount, &stale_count );
}

EEPROMErrorCode DATASTORE_RestoreData_FreshInfo( uint16 offset, uint8 backup_depth,
                                                 uint8 *data, uint16 data_bytecount,
                                                 uint8 *stale_count )
{
	uint8 backupCellIndex;
	uint16 backupCellOffset;
	uint16 dataByteIndex;
	
	#ifdef ADM_GRAPHICS_PROCESSOR
	EEPROMErrorCode error;
	#endif
	
	
	if( (backup_depth == 0) || (backup_depth >= 86) )
	{
		return EEPROMError_InvalidLength;		
	}

	if( (data == NULL) || (stale_count == NULL) )
	{
		return EEPROMError_BadPointer;
	}
	
	if( offset >= EEPROM_SIZE )
	{
		return EEPROMError_InvalidOffset;
	}
	
	if( ((uint32)offset + (uint32)backup_depth*(DATA_BYTE_OFFSET + data_bytecount)) >= EEPROM_SIZE )
	{
		return EEPROMError_InvalidLength;
	}
	
	backupCellIndex = FindFreshestBackupCell( offset, backup_depth, data_bytecount, stale_count );
	
	if( backupCellIndex < backup_depth )
	{
		backupCellOffset = offset + (backupCellIndex * (DATA_BYTE_OFFSET + data_bytecount));

		for( dataByteIndex=0; dataByteIndex<data_bytecount; dataByteIndex++ )
		{
			#ifdef ADM_GRAPHICS_PROCESSOR	
			while( !IpcEEPROM_ReadByte(&error, backupCellOffset + DATA_BYTE_OFFSET + dataByteIndex, 
									   &data[dataByteIndex] ) || (error != EEPROMError_Ok) )
			{
				(void)K_Task_Wait(1);
			}	
			#else
			while( EEPROM_ReadByte( backupCellOffset + DATA_BYTE_OFFSET + dataByteIndex, 
									&data[dataByteIndex] ) != EEPROMError_Ok )
			{
				(void)K_Task_Wait(1);
			}
			#endif
		}
		
		return EEPROMError_Ok;
	}
	else
	{
		*stale_count = 0xFF;
		return EEPROMError_OutOfResources;
	}		
}	


EEPROMErrorCode DATASTORE_SaveData( uint16 offset, uint8 backup_depth,
									uint8 *data, uint16 data_bytecount )
{
	uint8 backupCellIndex;
	uint8 freshness;
	uint8 stale_count;
	
	uint16 backupCellOffset;
	uint16 dataByteIndex;
	uint16 checksum;
	
	#ifdef ADM_GRAPHICS_PROCESSOR
	EEPROMErrorCode error;
	#endif
	

	if( (backup_depth == 0) || (backup_depth >= 86) )
	{
		return EEPROMError_InvalidLength;		
	}

	if( data == NULL )
	{
		return EEPROMError_BadPointer;
	}
	
	if( offset >= EEPROM_SIZE )
	{
		return EEPROMError_InvalidOffset;
	}
	
	if( ((uint32)offset + (uint32)backup_depth*(DATA_BYTE_OFFSET + data_bytecount)) >= EEPROM_SIZE )
	{
		return EEPROMError_InvalidLength;
	}
	
	backupCellIndex = FindFreshestBackupCell( offset, backup_depth, data_bytecount, &stale_count );
	if( backupCellIndex < backup_depth )
	{
		backupCellOffset = offset + (backupCellIndex * (DATA_BYTE_OFFSET + data_bytecount));
		
		#ifdef ADM_GRAPHICS_PROCESSOR
		while( !IpcEEPROM_ReadByte(&error, backupCellOffset + FRESHNESS_BYTE_OFFSET, 
								   &freshness ) || (error != EEPROMError_Ok) )
		{
			(void)K_Task_Wait(1);
		}			
		#else
		while( EEPROM_ReadByte( backupCellOffset + FRESHNESS_BYTE_OFFSET,
								&freshness ) != EEPROMError_Ok )
		{
			(void)K_Task_Wait(1);
		}
		#endif			
		
		backupCellIndex++;
		if( backupCellIndex >= backup_depth )
		{
			backupCellIndex = 0;
		}
	
		freshness++;
		if( freshness >= 0xFF )
		{
			freshness = 0;
		}
	}
	else
	{
		backupCellIndex = 0;
		freshness = 0;
	}
	
	backupCellOffset = offset + (backupCellIndex * (DATA_BYTE_OFFSET + data_bytecount));
	checksum = freshness;
	
	for( dataByteIndex=0; dataByteIndex<data_bytecount; dataByteIndex++ )
	{
		#ifdef ADM_GRAPHICS_PROCESSOR
		while( !IpcEEPROM_WriteByte(&error, backupCellOffset + DATA_BYTE_OFFSET + dataByteIndex, 
									data[dataByteIndex] ) || (error != EEPROMError_Ok) )
		{
			(void)K_Task_Wait(1);
		}			
		#else
		while( EEPROM_WriteByte( backupCellOffset + DATA_BYTE_OFFSET + dataByteIndex,
								 data[dataByteIndex] ) != EEPROMError_Ok )
		{
			(void)K_Task_Wait(1);
		}
		#endif

		checksum += data[dataByteIndex];
	}
	
	#ifdef ADM_GRAPHICS_PROCESSOR
	while( !IpcEEPROM_WriteWord(&error, backupCellOffset + CHECKSUM_BYTE_OFFSET, 
								checksum ) || (error != EEPROMError_Ok) )
	{
		(void)K_Task_Wait(1);
	}			
	#else
	while( EEPROM_WriteWord( backupCellOffset + CHECKSUM_BYTE_OFFSET,
							 checksum ) != EEPROMError_Ok )
	{
		(void)K_Task_Wait(1);
	}
	#endif
	
	#ifdef ADM_GRAPHICS_PROCESSOR
	while( !IpcEEPROM_WriteByte(&error, backupCellOffset + FRESHNESS_BYTE_OFFSET, 
								freshness ) || (error != EEPROMError_Ok) )
	{
		(void)K_Task_Wait(1);
	}			
	#else	
	while( EEPROM_WriteByte( backupCellOffset + FRESHNESS_BYTE_OFFSET,
							 freshness ) != EEPROMError_Ok )
	{
		(void)K_Task_Wait(1);
	}
	#endif
	
	return EEPROMError_Ok;
}


// ***************************************************
// * PRIVATE FUNCTIONS
// ***************************************************
static uint8 FindFreshestBackupCell( uint16 offset, uint8 backup_depth, uint16 data_bytecount, uint8 *stale_count )
{
	uint8 backupCellIndex;
	uint16 backupCellOffset;
	
	uint8 freshness;
	
	uint8 freshestBackupCellIndex = 0xFF;
	uint8 freshestFreshnessValue = 0xFF;
	bool foundFresher = FALSE;
	
	#ifdef ADM_GRAPHICS_PROCESSOR
	EEPROMErrorCode error;
	#endif
	
	*stale_count = 0;
	// Search for the freshness values to find the most recent backup cell
	for( backupCellIndex=0; backupCellIndex<backup_depth; backupCellIndex++ )
	{
		foundFresher = FALSE;

		backupCellOffset = offset + (backupCellIndex * (DATA_BYTE_OFFSET + data_bytecount));

		#ifdef ADM_GRAPHICS_PROCESSOR
		while( !IpcEEPROM_ReadByte(&error, backupCellOffset + FRESHNESS_BYTE_OFFSET, 
								   &freshness ) || (error != EEPROMError_Ok) )
		{
			(void)K_Task_Wait(1);
		}			
		#else
		while( EEPROM_ReadByte( backupCellOffset + FRESHNESS_BYTE_OFFSET,
								&freshness ) != EEPROMError_Ok )
		{
			(void)K_Task_Wait(1);
		}
		#endif			

		if( freshness == 0xFF )
		{
			// A save count of 0xFF is invalid, so move on to next backup cell
			continue;
		}
		
		if(	freshestFreshnessValue == 0xFF )
		{
			foundFresher = TRUE;
		}				
		else if( (freshness > (0xFF - backup_depth)) &&
				 (freshestFreshnessValue < backup_depth) )
		{
			// DO NOTHING - This is NOT the freshest backup cell!
			// This case is here to filter out the conditions that we
			// run in to when a freshness rollover has occured.
			// The only reason freshness appears to be greater than freshestFreshnessValue
			// is because we've found some older data with a freshness value from
			// before a rollover occurred.
		}
		else if( freshness > freshestFreshnessValue )
		{
			foundFresher = TRUE;
		}				
		else if( (freshness < backup_depth) &&
				 (freshestFreshnessValue > (0xFF - backup_depth)) )
		{
			foundFresher = TRUE;
		}

		if( foundFresher )
		{
		    if( VerifyBackupCellChecksum(offset, backupCellIndex, data_bytecount) )
			{
				freshestBackupCellIndex = backupCellIndex;
				freshestFreshnessValue = freshness;
			}
			else
			{
				(*stale_count)++;
			}
		}
	}
	
	return freshestBackupCellIndex;
}

static bool VerifyBackupCellChecksum( uint16 offset, uint8 backup_cell_index, uint16 data_bytecount )
{
	uint16 backupCellOffset;
	uint16 dataByteIndex;
	
	uint8 temp8;
	uint16 temp16;
	uint16 checksum;
	
	#ifdef ADM_GRAPHICS_PROCESSOR
	EEPROMErrorCode error;
	#endif
	

	backupCellOffset = offset + (backup_cell_index * (DATA_BYTE_OFFSET + data_bytecount));

	#ifdef ADM_GRAPHICS_PROCESSOR
	while( !IpcEEPROM_ReadByte(&error, backupCellOffset + FRESHNESS_BYTE_OFFSET, 
							   &temp8 ) || (error != EEPROMError_Ok) )
	{
		(void)K_Task_Wait(1);
	}			
	#else
	while( EEPROM_ReadByte( backupCellOffset + FRESHNESS_BYTE_OFFSET,
							&temp8 ) != EEPROMError_Ok )
	{
		(void)K_Task_Wait(1);
	}
	#endif			
	
	checksum = temp8;
	
	for( dataByteIndex=0; dataByteIndex<data_bytecount; dataByteIndex++ )
	{
		#ifdef ADM_GRAPHICS_PROCESSOR
		while( !IpcEEPROM_ReadByte(&error, backupCellOffset + DATA_BYTE_OFFSET + dataByteIndex, 
								   &temp8 ) || (error != EEPROMError_Ok) )
		{
			(void)K_Task_Wait(1);
		}			
		#else
		while( EEPROM_ReadByte( backupCellOffset + DATA_BYTE_OFFSET + dataByteIndex,
								&temp8 ) != EEPROMError_Ok )
		{
			(void)K_Task_Wait(1);
		}
		#endif

		checksum += temp8;
	}
	
	#ifdef ADM_GRAPHICS_PROCESSOR
	while( !IpcEEPROM_ReadWord(&error, backupCellOffset + CHECKSUM_BYTE_OFFSET, 
							   &temp16 ) || (error != EEPROMError_Ok) )
	{
		(void)K_Task_Wait(1);
	}			
	#else
	while( EEPROM_ReadWord( backupCellOffset + CHECKSUM_BYTE_OFFSET, 
							&temp16 ) != EEPROMError_Ok )
	{
		(void)K_Task_Wait(1);
	}
	#endif

	if( checksum == temp16 )
	{
		return TRUE;
	}
	else
	{
		// Checksum failed.  Flag this datastore cell as invalid.
		#ifdef ADM_GRAPHICS_PROCESSOR
		while( !IpcEEPROM_WriteByte(&error, backupCellOffset + FRESHNESS_BYTE_OFFSET,
									0xFF ) || (error != EEPROMError_Ok) )
		{
			(void)K_Task_Wait(1);
		}
		#else
		while( EEPROM_WriteByte( backupCellOffset + FRESHNESS_BYTE_OFFSET,
								 0xFF ) != EEPROMError_Ok )
		{
			(void)K_Task_Wait(1);
		}
		#endif

		return FALSE;
	}
}
