//! \file	CANBL_system_definition.h
//! Copyright 2006-12, 2015
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef CANBL_SYSTEM_DEFINITION_H
#define CANBL_SYSTEM_DEFINITION_H

typedef struct __attribute__((packed))
{
	uint8 numUpdates;			// Number of updates available in system
	uint8 systemName_str[32];	// Any value to this if it's only in English?
	uint32 systemVersion;		// In format of 0x00CCBBAA; AA=major, BB=minor, CC= build
	uint8 tokenPN_str[16];		// Null character terminated string
	uint8 tokenSeries_str[4];	// Null character terminated string
	uint8 tokenDateTime_seconds;
	uint8 tokenDateTime_minutes;
	uint8 tokenDateTime_hours;
	uint8 tokenDateTime_dayOfWeek;
	uint8 tokenDateTime_dayOfMonth;
	uint8 tokenDateTime_month;
	uint8 tokenDateTime_year;	// Epoch is year 2000
	uint8 tokenDateTime_reserved;
	uint8 reserved[63];
} systemDefinition;

typedef struct __attribute__((packed))
{
	uint8 softwarePN_str[16];	// Null character terminated string
	uint32 softwareVersion;		// In format of 0x00CCBBAA; AA=major, BB=minor, CC= build
	uint8 componentId;
	uint16 applicationId;
	uint8 statusBitfield;
    uint16 updateType;          // Block type: PIC software, flash image, usb, etc.
    uint8 reserved[6];
} softwareDefinition;

typedef enum
{
	BITNUM_SOFTWARE_DEF_STATUS_PUBLIC = 0,
	BITNUM_SOFTWARE_DEF_STATUS_UPDATE_MODE,
	BITNUM_SOFTWARE_DEF_STATUS_SECONDARY_UPDATE,
	BITNUM_SOFTWARE_DEF_STATUS_BOOTLOAD_HOST
} SOFTWARE_DEF_STATUS_BITNUMS_t;

typedef void (*SYSDEF_UpdateFunction)( void );

#define SYSDEF_CMP_ID_UNSPECIFIED      (0xFF)
#define SYSDEF_APP_ID_UNSPECIFIED      (0xFFFF)

void SYSDEF_RegisterUpdateFunction( SYSDEF_UpdateFunction update_func );

void SYSDEF_UpdateOrVerify( void );

// The following functions return TRUE if successful, FALSE otherwise.
bool SYSDEF_LookupExpectedPartNumber( uint16 update_type, uint8 component_id, uint16 app_id, uint8 part_number[16] );
bool SYSDEF_LookupExpectedVersion( uint16 update_type, uint8 component_id, uint16 app_id, uint32* version );
bool SYSDEF_LookupExpectedStatusBitfield( uint16 update_type, uint8 component_id, uint16 app_id, uint8* status_bits );

uint8 SYSDEF_CountAvailableUpdates( uint16 update_type, uint8 component_id );
bool SYSDEF_RetrieveUpdateInformation( uint32 sequence, uint16 update_type, uint8 component_id, uint8 pn_buffer[16], uint16* appId );

bool SYSDEF_GetSystemNumUpdates( uint8* value );
bool SYSDEF_GetSystemName_str( uint8 value[32], uint8 length );
bool SYSDEF_GetSystemVersion( uint32* value );
//This function reads the value from RAM instead of EEPROM
bool SYSDEF_BUFFERED_GetSystemVersion( uint32* value );
bool SYSDEF_GetSystemTokenPN_str( uint8 value[16], uint8 length );
//This function reads the value from RAM instead of EEPROM
bool SYSDEF_BUFFERED_GetSystemTokenPN_str( uint8 value[16], uint8 length );
bool SYSDEF_GetSystemTokenSeries_str( uint8 value[4], uint8 length );
bool SYSDEF_GetSystemTokenDate( uint32* value );
bool SYSDEF_GetSoftwareDefinitionPN( uint8 update_num, uint8 value[16], uint8 length );
bool SYSDEF_GetSoftwareDefinitionVersion( uint8 update_num, uint32* value );
bool SYSDEF_GetSoftwareDefinitionCompId( uint8 update_num, uint8* value );
bool SYSDEF_GetSoftwareDefinitionAppId( uint8 update_num, uint16* value );
bool SYSDEF_GetSoftwareDefinitionStatusBitfield( uint8 update_num, uint8* value );
bool SYSDEF_GetSoftwareDefinitionUpdateType( uint8 update_num, uint16* value );

#endif // CANBL_SYSTEM_DEFINITION_H
