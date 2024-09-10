//! \file	icb_internal.h
//! \brief The Internal Communications Bus Protocol Daemon module.
//!
//! Copyright 2006-12
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_comm_internal
//@{

//! \defgroup icbp_internal			Communications Protocol
//! \brief Low-level implentation of GCA Communications Protocol
//!
//! \b DESCRIPTION:
//!		This module transmits, receives, and routes messages along the
//!		system's internal communication bus (ICB).  The module also
//!		negotiates with other components in the system to establish a unique
//!		identifier, generates heartbeat messages to notify other components
//!		of the local system's presence, and informs the application about
//!		the presence of other components in the system.

#ifndef ICB_INTERNAL_H
#define ICB_INTERNAL_H

//! \ingroup icbp_internal	
//@{

#include "canbuffer.h"

//----------------------------------------------------------------------------
/// List of all possible return values from the API.
typedef enum
{
	/// Function completed request successfully.
	ICBIError_Ok = 0,

	/// No application-level message is available.
	ICBIError_NoMessageAvailable,

	/// The provided pointer was invalid.
	ICBIError_BadPointer,

	/// The provided message priority value was invalid.
	ICBIError_BadPriority,

	/// The content of a parameter was out of range.
	ICBIError_BadContent,

	/// The function could not complete successfully due to a shortage
	/// of necessary resources.
	ICBIError_ResourcesUnavailable

} ICBIErrorCode;
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/// The list of all possible ICB message priorities.
typedef enum
{
	/// High priority; reserved for ::ICBI_PutHighPriorityMessage()
	ICB_HIGH_PRIORITY = 0,

	/// Heartbeat priority; reserved for internal use
	ICB_HEARTBEAT_PRIORITY,

	/// Standard traffic priority; used with ::ICBI_PutOperationalMessage()
	ICB_OPERATIONAL_PRIORITY_0,

	/// Standard traffic priority; used with ::ICBI_PutOperationalMessage()
	ICB_OPERATIONAL_PRIORITY_1,

	/// Standard traffic priority; used with ::ICBI_PutOperationalMessage()
	ICB_OPERATIONAL_PRIORITY_2,

	/// Standard traffic priority; used with ::ICBI_PutOperationalMessage()
	ICB_OPERATIONAL_PRIORITY_3,

	/// Standard traffic priority; used with ::ICBI_PutOperationalMessage()
	ICB_OPERATIONAL_PRIORITY_4,

	/// Administrative priority; reserved for ::ICBI_PutAdminMessage()
	ICB_ADMIN_PRIORITY,

	/// @cond

	/// Number of different message categories
	ICB_PRIORITY_COUNT,

	/// @endcond

	/// Highest priority allowed by ::ICBI_PutOperationalMessage()
	ICB_HIGHEST_OPERATIONAL_PRIORITY = ICB_OPERATIONAL_PRIORITY_0,

	/// Lowest priority allowed by ::ICBI_PutOperationalMessage()
	ICB_LOWEST_OPERATIONAL_PRIORITY = ICB_OPERATIONAL_PRIORITY_4,

	BOOTLOADER_COMMAND_MESSAGE_PRIORITY = ICB_OPERATIONAL_PRIORITY_1,

	BOOTLOADER_BULK_DATA_PRIORITY = ICB_OPERATIONAL_PRIORITY_4

} ICBMessagePriority;
//----------------------------------------------------------------------------

typedef void (*BootloadMessageHandler)( CANBuffer *pBuffer );

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the remainder of this interface.
//!
//! \return A negative value if the initialization failed, 0 on success.
//----------------------------------------------------------------------------
sint16 ICBPInit     (void);

void ICBI_RegisterBootloadHandlerFunction( BootloadMessageHandler bootloadCallback );

//----------------------------------------------------------------------------
//! \brief Inform the ICBP module where to send Rx'd Application level messages.
//!
//! \param	mailboxId	The mailbox ID to send the messages to.
//!
//! \return
//!	\li	::ICBIError_Ok if successful.
//----------------------------------------------------------------------------
ICBIErrorCode	ICBI_SetAppRxMailbox(uint8 mailboxId);



//----------------------------------------------------------------------------
//! \brief	Place an operation message into the transmit queue.
//!
//! \param	srcBuffer	Pointer to location of message to transmit.
//! \param	priority	Priority level of message.
//! \param	loopback	If FALSE, message will only go out over CAN bus.
//!						If TRUE, message will be fed back into this component's ICB recieve mechanism
//!                     in addition to being transmitted over the CAN bus.
//!
//! This function will place the contents of the specified message buffer
//! into its internal transmit queue for later transmission on the ICB.
//!
//! \b Notes:
//! \li	The contents of the specified message buffer are copied by this
//! function.  The caller does not need to preserve the contents of the
//! message buffer for any length of time once this function has
//! completed successfully.
//! \li The only acceptable values for \c priority are those in the range
//! [::ICB_HIGHEST_OPERATIONAL_PRIORITY, ::ICB_LOWEST_OPERATIONAL_PRIORITY].
//!
//! \return
//!	\li	::ICBIError_Ok if successful.
//! \li ::ICBIError_BadPriority if the specified priority is not with the
//!	above-mentioned range.
//!	\li	::ICBIError_BadPointer if \c tgtBuffer is set to NULL.
//! \li ::ICBIError_BadContent if the message's payload size is out of range.
//! \li ::ICBIError_ResourcesUnavailable if the message could not be sent
//! due to a transient shortage of resources.
//----------------------------------------------------------------------------
ICBIErrorCode	ICBI_PutOperationalMessage( const CANBuffer *srcBuffer, ICBMessagePriority priority, bool loopback );



//----------------------------------------------------------------------------
//! \brief	Place a High Priority-level message into the transmit queue.
//!
//! \param	srcBuffer	Pointer to location of message to transmit.
//! \param	loopback	If FALSE, message will only go out over CAN bus.
//!						If TRUE, message will be fed back into this component's ICB recieve mechanism
//!                     in addition to being transmitted over the CAN bus.
//!
//! This function will place the contents of the specified message buffer
//! into its internal transmit queue for later transmission on the ICB.
//! The message is designated as being of the highest priority, and,
//! as such, may bypass most other messages that are queued for
//! transmission.
//!
//! \b Notes:
//! \li	The contents of the specified message buffer are copied by this
//! function.  The caller does not need to preserve the contents of the
//! message buffer for any length of time once this function has
//! completed successfully.
//! \li The message is transmitted with a priority setting of
//! ::ICB_HIGH_PRIORITY.
//!
//! \return
//!	\li	::ICBIError_Ok if successful.
//!	\li	::ICBIError_BadPointer if \c tgtBuffer is set to NULL.
//! \li ::ICBIError_BadContent if the message's payload size is out of range.
//! \li ::ICBIError_ResourcesUnavailable if the message could not be sent
//! due to a transient shortage of resources.
//----------------------------------------------------------------------------
ICBIErrorCode	ICBI_PutHighPriorityMessage( const CANBuffer *srcBuffer, bool loopback );



//----------------------------------------------------------------------------
//! \brief	Place an administrative-level message into the transmit queue.
//!
//! \param	srcBuffer	Pointer to location of message to transmit.
//! \param	loopback	If FALSE, message will only go out over CAN bus.
//!						If TRUE, message will be fed back into this component's ICB recieve mechanism
//!                     in addition to being transmitted over the CAN bus.
//!
//! This function will place the contents of the specified message buffer
//! into its internal transmit queue for later transmission on the ICB.
//! The message is designated as being of the lowest priority.
//!
//! \b Notes:
//! \li	The contents of the specified message buffer are copied by this
//! function.  The caller does not need to preserve the contents of the
//! message buffer for any length of time once this function has
//! completed successfully.
//! \li The message is transmitted with a priority setting of
//! ::ICB_ADMIN_PRIORITY.
//!
//! \return
//!	\li	::ICBIError_Ok if successful.
//!	\li	::ICBIError_BadPointer if \c tgtBuffer is set to NULL.
//! \li ::ICBIError_BadContent if the message's payload size is out of range.
//! \li ::ICBIError_ResourcesUnavailable if the message could not be sent
//! due to a transient shortage of resources.
//----------------------------------------------------------------------------
ICBIErrorCode	ICBI_PutAdminMessage( const CANBuffer *srcBuffer, bool loopback );


ICBIErrorCode	ICBI_PutBootloaderMessage( const CANBuffer *tgtBuffer, ICBMessagePriority priority );


/// @cond


/// @endcond

//@}

#endif // ICB_INTERNAL_H

//@}


