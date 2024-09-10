//! \file	dvar.h
//! \brief The Distributed Variable Module header file. \sa dvar_module
//!
//! Copyright 2006-2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_comm
//@{

//! \addtogroup gca_dvar				Distributed Variables
//! \brief Distributed Variables.
//!
//! \b DESCRIPTION:
//!    This module provides a high-level interface for declaring, tracking,
//!    and/or setting the value of distributed variables.

#ifndef DVAR_H
#define DVAR_H

//! \ingroup dvar
//@{

// Include basic platform types
#include "typedef.h"


//**************************************************************************
// Types
//**************************************************************************

/// The type for a distributed variable's identifier
typedef uint32 DistVarID;

/// The type for a distributed variable's value.
typedef uint32 DistVarType;
//
// *** Private note to maintainer of this module ***
//
// The design of this module is based on the assumption that the
// distributed variable content size is 32 bits.  If the above
// typedef should change, the distributed variable module will need
// to be heavily modified.
//

/// The type for specifying an offset into EEPROM
typedef uint16 EEPROMOffset;

/// The type for a set-point variable callback function.
/// \sa DVAR_RegisterOwnership()
typedef DistVarType (*SetPointCallback)( DistVarID id, DistVarType oldVal, DistVarType newVal );

/// The type for a dvar update callback function.
typedef void (*DistVarUpdateCallback)( DistVarID id, DistVarType Val );

//----------------------------------------------------------------------------
/// List of possible distributed variable storage types.
typedef enum
{
	/// Local copy stored in RAM only.
	VARIABLE_FLAVOR_RAM = 0,

	/// Local copy stored in EEPROM, with a shadowed copy stored in RAM.
	VARIABLE_FLAVOR_SHADOWED_EEPROM,

	/// Local copy stored in EEPROM only.
	VARIABLE_FLAVOR_EEPROM,

	/// @cond

	/// Maximum valid parameter
	VARIABLE_FLAVOR_MAX_TYPE = VARIABLE_FLAVOR_EEPROM

	/// @endcond

} DistVarFlavor;
//----------------------------------------------------------------------------


typedef struct OwnerTracking_struct
{
	struct OwnerTracking_struct *	nextInBroadcastList;
	struct OwnerTracking_struct *	nextInOwnerGroup;
	DistVarID						smallest;
	DistVarID						biggest;
	DistVarType *					localAddress;
	SetPointCallback				callback;
	uint16							pollsPerBroadcast;
	uint16							broadcastCountdown;
} OwnerTrackingItem;

typedef struct VarTracking_struct
{
	struct VarTracking_struct *		next;
	OwnerTrackingItem *				ownerList;
	DistVarID						smallest;
	DistVarID						biggest;
	DistVarFlavor					flavor;
	DistVarType *					localAddress;
	EEPROMOffset					eeOffset;
} VarTrackingItem;

/// The type for a distributed variable search context
typedef struct
{
	DistVarID id;
	DistVarType value;
	DistVarFlavor flavor;
	bool owned;
} DVarSearchContext;


//----------------------------------------------------------------------------
/// List of all possible return values from the API.
typedef enum
{
	/// Function completed request successfully.
	DVarError_Ok = 0,

	/// The distributed variable address range specified by the caller
	/// was not valid.
	DVarError_InvalidAddressRange,

	/// Specified variable storage type not valid. See also ::DistVarFlavor.
	DVarError_InvalidFlavor,

	/// The local RAM address (or the inferred range) provided by the
	/// caller was not valid.
	DVarError_InvalidLocalAddress,

	/// The EEPROM offset (or the inferred range) provided by the caller
	/// was not valid.
	DVarError_InvalidEEPROMOffset,

	/// The request failed due to a transient or permanent shortage
	/// of resources.
	DVarError_NotEnoughResources,

	/// A callback function pointer, required by the particular function
	/// call, was not provided.
	DVarError_MissingCallback,

	/// The requested broadcast period was not valid.
	DVarError_InvalidBroadcastPeriod,

	/// The specified variable cannot be found.
	DVarError_VariableNotFound,

	/// The specified variable is not locally owned.
	DVarError_UnownedVariable,

	/// The Operating System has not started running yet
	DVarError_RtosNotStarted,
	
	/// The DVAR_SetPointRemote_ACK function timed out without receiving
	/// confirmation of change
	DVarError_Timeout,

	// When traffic is suppressed, certain message will not be sent.  Messages
	// other than heartbeats, id requests, bootloader messages and common level
	// Dvar traffic are not permitted.
	DVarError_TrafficSuppressed

} DVarErrorCode;
//----------------------------------------------------------------------------

typedef void (*BootloadClientActivityFunction)( void );

//**************************************************************************
// Public Defines and Constants
//**************************************************************************

/// Default set-point variable callback function. This function will allow
/// any set-point request to an owned variable block to proceed unimpeded.
/// \hideinitializer
#define	DVAR_OWNER_CALLBACK_ACCEPT_ANY		(__DVarAcceptAnyChange)

/// Default set-point variable callback function. This function will reject
/// any set-point request to an owned variable block.
/// \hideinitializer
#define	DVAR_OWNER_CALLBACK_ACCEPT_NONE		(__DVarRejectAnyChange)


//**************************************************************************
// Available Public Methods
//**************************************************************************

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the remainder of this interface.
//!
//! \return A negative value if the initialization failed, 0 on success.
//----------------------------------------------------------------------------
sint16 DVAR_Init( void );



//----------------------------------------------------------------------------
//! \brief Register a segment (cluster) of distributed variables to monitor.
//!
//! \param base		The ID of the first (smallest) distributed variable in
//!					the cluster to track.
//! \param length	The length of the cluster to track.
//! \param flav		The type of local storage to be used for this cluster.
//! \param ramLocation	An address in local RAM which specifies the starting
//!						location for storing the values of the distributed
//!						variables in this cluster.
//! \param eeOffset		An offset in EEPROM which specifies the starting
//!						location for storing the values of the distributed
//!						variables in this cluster.
//!
//! \b Notes:
//!	\li	The parameter \c ramLocation must be specified only when the
//!		storage flavor uses RAM.  Otherwise, it should be set to NULL.
//!	\li	The parameter \c eeOffset must be specified only when the
//!		storage flavor uses EEPROM.  Otherwise, it should be set to 0.
//!	\li	The \c base and \c length parameters specify the range of distributed
//!		variables to track.
//!	\li	<b>Be certain that the specified RAM and/or EEPROM locations
//!		provide enough space for the values of the distributed variables.
//!		</b> Recall that each distributed variable ID is of type
//!		::DistVarType, and \b not a byte.
//!
//! \return
//!	\li ::DVarError_Ok if successful.
//!	\li ::DVarError_InvalidAddressRange if the specified distributed variable
//!     range is not within the distributed variable address space, or the
//!     specified range has at least a partial overlap with a previously
//!     registered range.
//!	\li ::DVarError_InvalidFlavor if the specified flavor is not a member of
//!     the DistVarFlavor enumeration.
//!	\li ::DVarError_InvalidLocalAddress if the specified local address range
//!     goes outside of the range of valid RAM addresses.
//!	\li ::DVarError_InvalidEEPROMOffset if the specified EEPROM offset range
//!     goes outside of the range of valid EEPROM offsets, or if the module
//!		was unable to access the EEPROM.
//!	\li ::DVarError_NotEnoughResources if the system has run out of the
//!     resources necessary to track this registration.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_RegisterSegment(
			DistVarID base,
			uint16 length,
			DistVarFlavor flav,
			DistVarType *ramLocation,
			EEPROMOffset eeOffset
);


//----------------------------------------------------------------------------
//! \brief Register system-wide ownership of a cluster of distributed
//!        variables.
//!
//! This function will establish the local component as the owner
//! of a cluster of distributed variables.  By owning distributed variables,
//! the component now actively maintains (and optionally, broadcasts) the
//! value of these variables.  The component ill field requests from other
//! components to alter the value of these variables.
//!
//! \param base			The ID of the first (smallest) distributed variable in
//!						the cluster to register as owned.
//! \param length		The length of the cluster to track.
//! \param spCallback	The callback to invoke in response to set-point
//!						requests.
//! \param broadcastPeriod	The delay between automatic broadcasts of this
//!							cluster's values to the ICB (expressed in
//!							milliseconds).
//!
//! \b Notes:
//!	\li	The specified cluster of distributed variables must already be
//!		registered through ::DVAR_RegisterSegment().
//!	\li	It is permitted to assert ownership on just a subset of a cluster
//!		that was registered through ::DVAR_RegisterSegment().
//!	\li	The callback function is invoked automatically whenever a set-point
//!		request is received from the ICB for an owned variable.  The
//!		callback function is provided with the variable ID, the present
//!		"old" value of the variable, and the proposed "new" value for the
//!		variable. The callback is expected to return the true "new" value
//!		for the variable, based on these inputs, which may be either the
//!		old value (change not authorized), the proposed new value (change
//!		authorized), or any variation deemed proper by the application
//!		developer (change accepted with override).
//!	\li	The callback function \b MUST return immediately.  That is, the
//!		callback function is not permitted to invoke an operating
//!		system call that could block.  Doing so will cause the entire
//!		distributed variable system to hang.
//!	\li	If the caller does not wish to automatically broadcast the state
//!		of the owned variables, then \c broadcastPeriod should be set to 0.
//!	\li	Because of the delays inherent in retrieving data from EEPROM,
//!		\c broadcastPeriod must be set to 0 if the owned variables are
//!		stored only in EEPROM.  Variables that use shadowed EEPROM, however,
//!		may use the autobroadcast feature.
//!	\li	One of two pre-defined callsbacks may be specified as \c spCallback
//!		instead of a user-defined callback:
//! <ul>
//! <li>	::DVAR_OWNER_CALLBACK_ACCEPT_ANY() will automatically accept
//!		all set-point requests -- with any value -- for all variables in
//!		the cluster.</li>
//! <li>	::DVAR_OWNER_CALLBACK_ACCEPT_NONE() will automatically reject all
//!		set-point requests for all variables in the cluster (e.g. read-only
//!		variables).</li>
//! </ul>
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//!	\li	::DVarError_InvalidAddressRange if the specified distributed
//!	variable range does not fit within a segment previously registered via
//!	the ::DVAR_RegisterSegment() call, or the specified range
//!	overlaps with some portion of an existing ownership record.
//!	\li	::DVarError_NotEnoughResources if the system has run out of the
//!	resources necessary to track this registration.
//!	\li	::DVarError_MissingCallback if a callback was not specified.
//!	\li	::DVarError_InvalidBroadcastPeriod if the caller specified a non-zero
//!	broadcast frequency on a memory segment that resides in local EEPROM
//!	memory only.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_RegisterOwnership(
			DistVarID base,
			uint16 length,
			SetPointCallback spCallback,
			uint16 broadcastPeriod
);



//----------------------------------------------------------------------------
//! \brief	Transmit a set-point request to a remote component.
//!
//! This function will generate a set-point request message and transmit
//! it along the ICB.
//!
//! \param	id				The ID of the distributed variable to modify.
//! \param	setPointValue	The proposed new value of the variable.
//!
//! \b Notes:
//!	\li	The local copy of the specified distributed variable will NOT
//!	be modified by this operation; it will be modified if/when the
//!	remote component broadcasts an update of that variable's value.
//!	\li	The set-point request message that is generated by this function
//! will also be internallized within the component such that this function can
//! be used to change a variable that is owned within the component.
//!
//! \sa DAVR_SetPointRemote_ACK()
//! \sa DVAR_SetPointLocal()
//! \sa DVAR_SetPointLocal_wCallback()
//! \sa DVAR_SetFlagsLocal()
//! \sa DVAR_ClearFlagsLocal()
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//!	\li	::DVarError_InvalidAddressRange if the specified distributed variable
//!	is not within the distributed variable address space.
//!	\li	::DVarError_NotEnoughResources if the system does not presently have
//!	the resources necessary to transmit the set-point request.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_SetPointRemote( DistVarID id, DistVarType setPointValue );

//----------------------------------------------------------------------------
//! \brief	Transmit a set-point request to a remote component and wait for
//!         confirmation of change
//!
//! This function will generate a set-point request message and transmit
//! it along the ICB.  The function will wait for confirmation of the change.
//!
//! \param	id				The ID of the distributed variable to modify.
//! \param	setPointValue	The proposed new value of the variable.
//! \param  timeout_ms		The amount of time to wait for confirmation of
//! 						the change
//!
//! \b Notes:
//!	\li	The local copy of the specified distributed variable will NOT
//!	be modified by this operation (if registered); it will be modified if/when the
//!	remote component broadcasts an update of that variable's value.
//!	\li	The set-point request message that is generated by this function
//! will also be internallized within the component such that this function can
//! be used to change a variable that is owned within the component.
//!	\li	This function will acknowledge a successful setpoint request of any DVar,
//! regardless of registration.
//! \li This function will acknowledge a successful setpoint request regardless
//! of what the orginal setpoint value was.
//!
//! \sa DAVR_SetPointRemote()
//! \sa DVAR_SetPointLocal()
//! \sa DVAR_SetPointLocal_wCallback()
//! \sa DVAR_SetFlagsLocal()
//! \sa DVAR_ClearFlagsLocal()
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//!	\li	::DVarError_InvalidAddressRange if the specified distributed variable
//!	is not within the distributed variable address space.
//!	\li	::DVarError_NotEnoughResources if the system does not presently have
//!	the resources necessary to transmit the set-point request.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_SetPointRemote_ACK( DistVarID id, DistVarType setPointValue, uint16 timeout_ms );

//----------------------------------------------------------------------------
//! \brief	Transmit a set-point request to a remote component and wait for
//!         confirmation of change, if no confirmation by retry_rate_ms it will
//!         send a new request, it will do this max_attempts number of times.
//!
//! This function will generate a set-point request message and transmit
//! it along the ICB.  The function will wait for confirmation of the change.
//!
//! \param	id				The ID of the distributed variable to modify.
//! \param	setPointValue	The proposed new value of the variable.
//! \param  retry_period_ms The amount of time to wait during each retry
//! 						for confirmation of the change
//! \parag max_attempts     The number of times it will attempt a SetPointRemote
//!
//! \b Notes:
//!	\li	The local copy of the specified distributed variable will NOT
//!	be modified by this operation (if registered); it will be modified if/when the
//!	remote component broadcasts an update of that variable's value.
//!	\li	The set-point request message that is generated by this function
//! will also be internallized within the component such that this function can
//! be used to change a variable that is owned within the component.
//!	\li	This function will acknowledge a successful setpoint request of any DVar,
//! regardless of registration.
//! \li This function will acknowledge a successful setpoint request regardless
//! of what the orginal setpoint value was.
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//!	\li	::DVarError_InvalidAddressRange if the specified distributed variable
//!	is not within the distributed variable address space.
//!	\li	::DVarError_NotEnoughResources if the system does not presently have
//!	the resources necessary to transmit the set-point request.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_SetPointRemote_ACK_Retry( DistVarID id, DistVarType setPointValue, uint16 retry_period_ms, uint8 max_attempts);

//----------------------------------------------------------------------------
//! \brief	Request the value of a specified distributed variable and force a
//!			remote broadcast.  The function will wait to receive the data
//!			until the specified timeout period has been reached.
//!
//! This function will generate a broadcast request message (in the form of a
//! special setpoint request message) for a specified DVar.  The DVar value
//! will be broadcast on its normal DVar address, but this function will also
//! return the requested value via a pointer to the calling function.  This
//! function is valid for any DVar, regardless of registration.
//!
//! The return value should be checked to ensure that the requestedValue
//! data is valid.
//!
//! 8/10/12 Update: This function now processes variables that are locally
//! owned.
//!
//! \param	id				The ID of the distributed variable to broadcast
//!							and get the value of
//! \param	requestedValue	The location to store the specified distributed
//!							variable value
//! \param  timeout_ms		The amount of time to wait for confirmation of the change
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//!	\li	::DVarError_Timeout if the specified distributed variable value could
//! not be obtained within the specified timeout limit
//! \li ::DVarError_InvalidEEPROMOffset if specified distributed variable is
//! locally owned and has an invalid EEPROM offset.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_RequestBroadcast_ACK( DistVarID id, DistVarType *requestedValue, uint16 timeout_ms );

//----------------------------------------------------------------------------
//! \brief	Modify a locally owned distributed variable.
//!
//! This function will modify the value of a distributed variable if that
//! variable is owned by the local component.
//!
//! \param	id				The ID of the distributed variable to modify.
//! \param	setPointValue	The new value of the variable.
//!
//! \b Notes:
//!	\li	The local copy of the specified distributed variable will be updated
//!	immediately.
//! \li This mechanism bypasses the set-point callback function specified
//! in the ::DVAR_RegisterOwnership() call.  It is expected
//! that the caller already is aware of what values are valid for a locally
//! owned variable, and can manually reschedule any tasks that need to react
//! to the modification of the variable's value.
//!	\li	This function should \b not be used to modify a variable that the
//!	local component does not own.  For variables that are not owned by the
//!	local component, use ::DVAR_SetPointRemote() instead.
//! \li If the present value of the variable is equal to the new value,
//! as specified by \c setPointValue, then this function will return without
//! performing any modification to the system state.
//! \li If the value of the distributed variable is modified, a broadcast
//! message will be generated automatically in order to inform the other
//! components in the system about the change.
//!
//! \sa DVAR_SetPointRemote()
//! \sa DAVR_SetPointRemote_ACK()
//! \sa DVAR_SetPointLocal_wCallback()
//! \sa DVAR_SetFlagsLocal()
//! \sa DVAR_ClearFlagsLocal()
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//! \li	::DVarError_UnownedVariable if the specified distributed variable is
//! not tracked and owned by the local system.
//!	\li	::DVarError_InvalidAddressRange if the specified distributed variable
//!	is not within the distributed variable address space.
//!	\li	::DVarError_NotEnoughResources if the system does not presently have
//!	the resources necessary to write an EEPROM value.
//! \li	::DVarError_InvalidEEPROMOffset if the EEPROM could not be accessed
//! in order to retrieve the value of the distributed variable.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_SetPointLocal( DistVarID id, DistVarType setPointValue );


//----------------------------------------------------------------------------
//! \brief	Modify a locally owned distributed variable (using registered 
//!         Callback function).
//!
//! This function will request to modify the value of a distributed variable
//! if that variable is owned by the local component.  The registered callback
//! function will be called in order to process the change request
//!
//! \param	id				The ID of the distributed variable to modify.
//! \param	setPointValue	The new value of the variable.
//!
//! \b Notes:
//!	\li	The callback function of the specified distributed variable will be
//! called immediately and the value of the variable will be updated based 
//! on the result of this function call.
//!	\li	This function should \b not be used to modify a variable that the
//!	local component does not own.  For variables that are not owned by the
//!	local component, use ::DVAR_SetPointRemote() instead.
//! \li If the present value of the variable is equal to the new value,
//! as specified by \c setPointValue, then this function will return without
//! performing any modification to the system state.
//! \li If the value of the distributed variable is modified, a broadcast
//! message will be generated automatically in order to inform the other
//! components in the system about the change.
//!
//! \sa DVAR_SetPointRemote()
//! \sa DAVR_SetPointRemote_ACK()
//! \sa DVAR_SetPointLocal()
//! \sa DVAR_SetFlagsLocal()
//! \sa DVAR_ClearFlagsLocal()
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//! \li	::DVarError_UnownedVariable if the specified distributed variable is
//! not tracked and owned by the local system.
//!	\li	::DVarError_InvalidAddressRange if the specified distributed variable
//!	is not within the distributed variable address space.
//!	\li	::DVarError_NotEnoughResources if the system does not presently have
//!	the resources necessary to write an EEPROM value.
//! \li	::DVarError_InvalidEEPROMOffset if the EEPROM could not be accessed
//! in order to retrieve the value of the distributed variable.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_SetPointLocal_wCallback( DistVarID id, DistVarType setPointValue );


//----------------------------------------------------------------------------
//! \brief	Set flags in a locally owned distributed variable.
//!
//! This function will set flags (indibidual bits) of a distributed variable
//! if that variable is owned by the local component.  Since modifying bits
//! is a read-modify-write process, this function will take care of maintaining
//! the integrity of the distributed variable through the process.
//!
//! \param	id				The ID of the distributed variable to modify.
//! \param	flags			A bitmask of the flags to be set.
//!
//! \b Notes:
//!	\li	The local copy of the specified distributed variable will be updated
//!	immediately.
//! \li This mechanism bypasses the set-point callback function specified
//! in the ::DVAR_RegisterOwnership() call.  It is expected
//! that the caller already is aware of what values are valid for a locally
//! owned variable, and can manually reschedule any tasks that need to react
//! to the modification of the variable's value.
//!	\li	This function should \b not be used to modify a variable that the
//!	local component does not own.  For variables that are not owned by the
//!	local component, use ::DVAR_SetPointRemote() instead.
//! \li If the value of the distributed variable is modified, a broadcast
//! message will be generated automatically in order to inform the other
//! components in the system about the change.
//!
//! \sa DVAR_SetPointRemote()
//! \sa DAVR_SetPointRemote_ACK()
//! \sa DVAR_SetPointLocal()
//! \sa DVAR_SetPointLocal_wCallback()
//! \sa DVAR_ClearFlagsLocal()
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//! \li	::DVarError_UnownedVariable if the specified distributed variable is
//! not tracked and owned by the local system.
//!	\li	::DVarError_InvalidAddressRange if the specified distributed variable
//!	is not within the distributed variable address space.
//!	\li	::DVarError_NotEnoughResources if the system does not presently have
//!	the resources necessary to transmit the set-point request.  In this case,
//! the variable will have been modified locally, yet the rest of the system
//! will not be aware of the modification to the variable.
//! \li	::DVarError_InvalidEEPROMOffset if the EEPROM could not be accessed
//! in order to retrieve the value of the distributed variable.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_SetFlagsLocal( DistVarID id, DistVarType flags );


//----------------------------------------------------------------------------
//! \brief	Clear flags in a locally owned distributed variable.
//!
//! This function will set flags (indibidual bits) of a distributed variable
//! if that variable is owned by the local component.  Since modifying bits
//! is a read-modify-write process, this function will take care of maintaining
//! the integrity of the distributed variable through the process.
//!
//! \param	id				The ID of the distributed variable to modify.
//! \param	flags			A bitmask of the flags to be cleared (1 -> clear bit).
//!
//! \b Notes:
//!	\li	The local copy of the specified distributed variable will be updated
//!	immediately.
//! \li This mechanism bypasses the set-point callback function specified
//! in the ::DVAR_RegisterOwnership() call.  It is expected
//! that the caller already is aware of what values are valid for a locally
//! owned variable, and can manually reschedule any tasks that need to react
//! to the modification of the variable's value.
//!	\li	This function should \b not be used to modify a variable that the
//!	local component does not own.  For variables that are not owned by the
//!	local component, use ::DVAR_SetPointRemote() instead.
//! \li If the value of the distributed variable is modified, a broadcast
//! message will be generated automatically in order to inform the other
//! components in the system about the change.
//!
//! \sa DVAR_SetPointRemote()
//! \sa DAVR_SetPointRemote_ACK()
//! \sa DVAR_SetPointLocal()
//! \sa DVAR_SetPointLocal_wCallback()
//! \sa DVAR_SetFlagsLocal()
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//! \li	::DVarError_UnownedVariable if the specified distributed variable is
//! not tracked and owned by the local system.
//!	\li	::DVarError_InvalidAddressRange if the specified distributed variable
//!	is not within the distributed variable address space.
//!	\li	::DVarError_NotEnoughResources if the system does not presently have
//!	the resources necessary to transmit the set-point request.  In this case,
//! the variable will have been modified locally, yet the rest of the system
//! will not be aware of the modification to the variable.
//! \li	::DVarError_InvalidEEPROMOffset if the EEPROM could not be accessed
//! in order to retrieve the value of the distributed variable.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_ClearFlagsLocal( DistVarID id, DistVarType flags );


//----------------------------------------------------------------------------
//! \brief	Broadcast the value of a distributed variable.
//!
//! This function triggers a broadcast of the local value of a specified
//! distributed variable onto the ICB.
//!
//! \param	id	The ID of the distributed variable to broadcast.
//!
//! \b Notes:
//!	\li	The specified variable does not need to be owned by the local
//!	component; it is only necessary (and sufficient) that the specified
//!	variable is currently being tracked by the local component.
//!	\li	While it is permissible for a local component to broadcast the value
//!	of a unowned variable, it is strongly discouraged, as the value
//!	broadcasted may conflict with the value reported by the variable's owner.
//!	\li	Broadcasting a variable's value is not equivalent to issuing a
//!	set-point request.  A component will ignore all broadcast messages for
//!	variables that it owns.
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//!	\li	::DVarError_InvalidAddressRange if the specified distributed variable
//!	is not within the distributed variable address space, or the
//!	specified variable is not presently being tracked.
//!	\li	::DVarError_NotEnoughResources if the system does not presently have
//!	the resources necessary to transmit the broadcast message.
//! \li	::DVarError_InvalidEEPROMOffset if the EEPROM could not be accessed
//! in order to retrieve the value of the distributed variable.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_BroadcastOnce( DistVarID id );



//----------------------------------------------------------------------------
//! \brief	Initialize a search context object
//!
//! This function (re)initializes the state of a search context object
//! for use in searching and/or seeking distributed variables in the
//! local distributed variable database.
//!
//! \param	pContext	A pointer to a search context object.
//!
//! \b Notes:
//!	\li	If this function completes without an error, then the context will
//! contain the state of the first distributed variable in the local
//! database.  If this function returns an error, then the state of the
//! context object is undefined.
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//!	\li	::DVarError_InvalidAddressRange if pContext was a NULL pointer.
//! \li ::DVarError_VariableNotFound if the local database is empty.
//! \li ::DVarError_InvalidEEPROMOffset if the first variable in the database
//! was stored in EEPROM, and the system was unable to successfully access
//! the EEPROM in order to retrieve that variable's value.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_InitSearch( DVarSearchContext *pContext );



//----------------------------------------------------------------------------
//! \brief	Seek out the state of a specific distributed variable.
//!
//! This function will update a search context object to contain the
//! state of the specified distributed variable.
//!
//! \param	pContext	A pointer to a search context object.
//! \param	id			The ID of the distributed variable to locate.
//!
//! \b Notes:
//!	\li	If this function returns an error, then the state of the
//! context object is undefined.
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//!	\li	::DVarError_InvalidAddressRange if pContext was a NULL pointer.
//! \li ::DVarError_VariableNotFound if the specified distributed variable
//! could not be found in the local database.
//! \li ::DVarError_InvalidEEPROMOffset if the specified variable
//! was stored in EEPROM, and the system was unable to successfully access
//! the EEPROM in order to retrieve that variable's value.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_SeekLocalVariable( DVarSearchContext *pContext, DistVarID id );


//----------------------------------------------------------------------------
//! \brief	Advance to the next distributed variable.
//!
//! This function will update a search context object to contain the
//! state of the next distributed variable in the local database.
//!
//! \param	pContext	A pointer to a search context object.
//!
//! \b Notes:
//! \li	It is a requirement that the provided context be updated beforehand
//! via a previous call to ::DVAR_InitSearch(), ::DVAR_SeekVariable(),
//! or ::DVAR_NextVariable().
//!	\li	If this function returns an error, then the state of the
//! context object is undefined.
//!
//! \return
//!	\li	::DVarError_Ok if successful.
//!	\li	::DVarError_InvalidAddressRange if pContext was a NULL pointer.
//! \li ::DVarError_VariableNotFound if the end of the local database has
//! been reached.
//! \li ::DVarError_InvalidEEPROMOffset if the specified variable
//! was stored in EEPROM, and the system was unable to successfully access
//! the EEPROM in order to retrieve that variable's value.
//----------------------------------------------------------------------------
DVarErrorCode DVAR_NextVariable( DVarSearchContext *pContext );


//----------------------------------------------------------------------------
//! \brief  Register a function to be called for every incoming DVAR
//!
//! \param dvuCallback  The callback function
//!
//! \return The previous callback function
//----------------------------------------------------------------------------
DistVarUpdateCallback DVAR_RegisterLoggingFunction( DistVarUpdateCallback dvuCallback );

// Used to signal bootloader client activity.  This function is not to
// be called by application level code!
void SignalBootloadClientTaskActivity( void );

// Used to register a function to handle bootloader client activity.
// This function is not to be called by application level code!
void RegisterBootloadClientActivityFunction( BootloadClientActivityFunction bootload_client_func );

uint8 DVAR_GetNumUsedVarTrackingItems( void );

uint8 DVAR_GetNumUsedOwnerTrackingItems( void );

/// @cond

/// These functions are for internal use only.  Do not include in the
/// interface documentation.

//
// Default callbacks
//
DistVarType __DVarAcceptAnyChange( DistVarID id, DistVarType oldVal, DistVarType newVal );
DistVarType __DVarRejectAnyChange( DistVarID id, DistVarType oldVal, DistVarType newVal );

/// @endcond

//@}

#endif // DVAR_H

//@}

