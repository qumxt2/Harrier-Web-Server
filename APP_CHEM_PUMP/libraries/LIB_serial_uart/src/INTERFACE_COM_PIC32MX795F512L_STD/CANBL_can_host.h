//! \file	CANBL_can_host.h
//! Copyright 2006-12
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef CANBL_CAN_HOST_H
#define CANBL_CAN_HOST_H

// ***************************************************
// * MACROS
// ***************************************************
#define DEFAULT_BOOTLOAD_HOLD_TIME_ms				(2500)

// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************
typedef struct
{
	uint64 acceptedNodes;
	uint64 rejectedNodes;
	uint64 noChangeNodes;
} CanBootloadResults_t;

typedef bool (*CANBL_CheckForUpdateSource)( void );

typedef void (*CANBL_UpdateParser)( void );

// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************

// Initializes the CAN bootloader host update library.
void CANBL_Init( CANBL_UpdateParser update_parser );

// Registers a function that checks to see if the update source is present.
void CANBL_RegisterUpdateSource( CANBL_CheckForUpdateSource check_for_update_source );

// Function to manually enable/disable the CAN bootloading functionality.  By default,
// CAN bootloading is enabled. Must be called within application initialization to disable.
void CANBL_SetEnableState( bool enabled );

// Initiates CAN bootloading process.
void CANBL_CheckForUpdates( void );

// If other modules are currently in bootload mode, this function will keep modules in bootload mode for
// a period of holdtime_ms.  Additional calls to this function will set the holdtime to the new value.
// If a module isn't currently in bootload mode, this function may put the module into bootload mode, however,
// the specified time may not apply because the module will reset.
// returns TRUE if the update source is present and bootloader is enabled, FALSE otherwise
bool CANBL_HoldModulesInBootloadMode( uint16 holdtime_ms, bool force_source_check );

// Initiates process to check modules for secondary updates (updates not updated directly, host-->!CAN enabled bootloader )
void CANBL_TransitionToSecondaryUpdates( void );

// Waits for feedback selection to be made for a given module type.
void CANBL_WaitOnUserFdbck( uint64 multiple_app_nodes, uint16 update_type, uint8 cmp_id );

// Notifies user interface of nodes receiving updates and waits for the okay to proceed.
void CANBL_WaitOnUpdateBeginFdbck( uint64 accepted_nodes );

// Notifies user interface that updates are complete and waits for acknowledgement
// before proceeding / returning.
void CANBL_WaitOnUpdateCompleteFdbck( void );

// Call to indicate that the host is active.
void CANBL_SetHostStatusActive( void );

// ************************************************
// Multiple Updates Per Module Support
// ************************************************

// Sets the currently installed update id for the specified module/node.  In the case of PIC updates,
// this corresponds to the application ID.
void CANBL_SetNodeUpdateSelection( uint8 node, uint16 selected_update_id );

// Checks to see if multiple updates are available for the specified module.
// returns TRUE if multiple updates are available, FALSE otherwise
bool CANBL_CheckSystemForMultipleUpdates( uint16 update_type, uint8 component_id );

// Gets the currently installed update id for the specified module/node.  In the case of PIC updates,
// this corresponds to the application ID.
uint16 CANBL_GetNodeUpdateSelection( uint8 node );

#endif // CANBL_CAN_HOST_H
