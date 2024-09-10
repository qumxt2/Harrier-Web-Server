//! \file	app_compatibility.h
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef APP_COMPATIBILITY_H
#define APP_COMPATIBILITY_H

// ***************************************************
// * MACROS
// ***************************************************
#define SOFTWARE_PART_NUM_MAX_STRING_LENGTH		(16)	// bytes

// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************

typedef struct __attribute__((packed))
{
	uint16 applicationId_u16;				// Info can be obtained from both bootloader and EEPROM
	uint8 componentId_u8;					// Info can be obtained from both bootloader and EEPROM
	uint8 purposeId_u8;						// Info can be obtained from both bootloader and EEPROM
	uint8 softwarePartNum_str[SOFTWARE_PART_NUM_MAX_STRING_LENGTH];	// Info can be obtained from both bootloader and EEPROM
	uint16 majorVersion_u16;				// Info can be obtained from both bootloader and EEPROM
	uint16 minorVersion_u16;				// Info can be obtained from both bootloader and EEPROM
	uint16 buildVersion_u16;				// Info can be obtained from both bootloader and EEPROM
} PreviousAppData_t;

typedef struct __attribute__((packed))
{
	PreviousAppData_t previousApp;			// Info can be obtained from both bootloader and EEPROM
	uint8 EraseQueuedButNotCompleted_bool;	// Obtained from EEPROM only (not bootloader)
	uint8 RandomNumberSeedUsed_bool;			// Obtained from EEPROM only (not bootloader)
} AppCompatibilityCriteria_t;

// Note: The following structure is separately defined for the bootloader, however, the structures must match.
typedef struct __attribute__((packed))
{
	PreviousAppData_t previousApp;			// Variables in AppCompatibilityCriteria_t structure
	uint8 bootloaderPresent_str[6];			// Bootloader verification string
	uint8 BootloaderUpdateOccurred_bool;		// Indicates that something actually changed.
	uint8 ApplicationExecuted_bool;			// Used by the bootloader to keep track of resets
	uint8 ApplicationInfoLatched_bool;		// Indicates that a valid application had previously been loaded
} BootloaderCompatibilityVars_t;

typedef struct __attribute__((packed))
{
    uint8 bootloaderPartNumber[SOFTWARE_PART_NUM_MAX_STRING_LENGTH]; // Makes Bootloader Part Number available at application level
    uint16 bootloaderMajorVersion_u16;		// Makes Bootloader Version information available at application level
	uint16 bootloaderMinorVersion_u16;		// Makes Bootloader Version information available at application level
	uint16 bootloaderBuildVersion_u16;		// Makes Bootloader Version information available at application level
} BootloaderIdVars_t;

typedef struct __attribute__((packed))
{
	PreviousAppData_t previousApp;			// Variables in AppCompatibilityCriteria_t structure
	uint8 EraseQueued_bool;					// Indicates whether or not an erase was previously queued and not completed
} EEPROMCompatibilityVars_t;


// A return value of TRUE indicates that EEPROM should be erased.
// A return value of FALSE indicates that EEPROM should NOT be erased.
typedef bool (*AppCompatibilityHandler)( AppCompatibilityCriteria_t *check_criteria );

// ***************************************************
// * PUBLIC FUNCTIONS
// ***************************************************

// Registers a AppCompatibilityHandler type function to be called prior to RTOS execution.
// This function will determine whether or not the external EEPROM memory should be scrubbed.
void APPCHECK_RegisterCompatibilityHandler( AppCompatibilityHandler function );

// Pre-canned handler #1
// Intended for older style applications.
// Erases applicaiton EEPROM space if app ID, component ID, or software PN
// differ or if an erase was previously queued but not completed.
// **************************************************************************
// NOTE:This is the default option if none is specfied by the application!
// **************************************************************************
bool APPCHECK_OlderApplicationHandler( AppCompatibilityCriteria_t *check_criteria );

// Pre-canned handler #2
// Intended for newer style applications.
// Erases using same criteria as an old application PLUS an erase will occur if
// the random number seed was used (EEPROM location has a value != 0xFFFF).
bool APPCHECK_NewerApplicationHandler( AppCompatibilityCriteria_t *check_criteria );

// Returns TRUE if a CAN capable bootloader is present, FALSE otherwise.
bool APPCHECK_CheckCanEnabledBootloaderPresent( void );

// If successful, buffer will contain the bootloader part number string and TRUE will be returned.
// FALSE will be returned if the part number cannot be obtained.
// Buffer must be at least SOFTWARE_PART_NUM_MAX_STRING_LENGTH bytes in size.
bool APPCHECK_GetBootloaderPartNumberStr( uint8* buffer );

// If successful, versions will be populated and TRUE will be returned.
// FALSE will be returned if the versions cannot be obtained.
bool APPCHECK_GetBootloaderVersion( uint16* major, uint16* minor, uint16* build );

// If RCON data was latched in bootloader latched_rcon will be updated with the latched value and TRUE
// will be returned.  If bootloader is not present, FALSE will be returned.
bool APPCHECK_GetBootloaderLatchedRconData( uint32* latched_rcon );

#endif // APP_COMPATIBILITY_H
