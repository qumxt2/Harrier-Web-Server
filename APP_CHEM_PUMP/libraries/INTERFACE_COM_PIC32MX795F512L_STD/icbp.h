//! \file	icbp.h
//! \brief The Internal Communications Bus Protocol Daemon module.
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_comm
//@{

//! \addtogroup gca_icbp				Comunication Network
//! \brief The Internal Communications Bus Protocol Daemon module.
//!
//! \b DESCRIPTION:
//!		This module automatically tracks the state of components that can
//!		be reached via the Internal Communication Bus (ICB).
//!		This module automatically negotiates with other components in the
//!		system to establish a unique identifier (or "Node ID") for the
//!		local system, generates heartbeat messages to notify other
//!		components of the local system's presence, and can inform the
//!		application about the presence of other components in the system.
//!		<p>
//!		This module allows the user to query about the status of other
//!		components in the system, each of which is identified by a unique
//!		Node ID.  This module can also provide the caller with the system
//!		configuration identifier for a particular Node ID, thereby enabling
//!		the application to learn about the specifics of a particular
//!		component.  So that the user does not need to continually poll
//!		for node status changes, this module can be configured to send
//!		a signal to the application whenever a new node appears on the ICB,
//!		or when an existing node ceases sending messages along the ICB.
 
#ifndef ICBP_H
#define ICBP_H

//! \ingroup icbp_internal	
//@{

#include "syscfgid.h"

/*
** CAN ID construction/deconstruction macros.  Macros are based on the
** following bit field layout:
**
** 10 - 8  :::   Message Class
** 7       :::   -=RESERVED=-
** 6       :::   Modifier Bit A
** 5  - 0  :::   Node ID
*/
#define	GENERATE_CAN_ID_MSG_CLASS_BITS(x)	((((uint16) (x)) & 0x7) << 8)
#define	GENERATE_CAN_ID_BOOTLOAD_BIT(x)		((((uint16) (x)) & 0x1) << 7)
#define	GENERATE_CAN_ID_MOD_BIT_A			((uint16) (1 << 6))
#define	GENERATE_CAN_ID_NODE_ID(x)			(((uint16) (x)) & 0x3F)
#define	EXTRACT_MSG_CLASS_FROM_CAN_ID(id)	((((uint16) (id)) >> 8) & 0x7)
#define	EXTRACT_BOOTLOAD_BIT_FROM_CAN_ID(id) ((((uint16) (id)) >> 7) & 0x1)
#define	EXTRACT_A_BIT_FROM_CAN_ID(id)		((((uint16) (id)) >> 6) & 0x1)
#define	EXTRACT_NODE_ID_FROM_CAN_ID(id)		(((NodeID) (id)) & 0x3F)

/// The maximum number of components in a system.
#define	MAXIMUM_NODE_IDS				(64)

/// Indicates the present online/offline status of a node.
typedef enum
{
	OFFLINE = 0,
	ONLINE = 1
} NodePresentStatus;

/// Indicates whether or not an offline/online event occurred
typedef enum
{
	NO_EVENT_OCCURRED = 0,
	EVENT_OCCURRED = 1
} NodeEventStatus;

/// Indicates the CAN polling Frequency
typedef enum
{
	STANDARD_POLL_FREQ = 0,
	HIGH_POLL_FREQ,
	LAST_POLL_FREQUENCY
} ICBP_POLL_FREQ;

//----------------------------------------------------------------------------
/// This structure tells the caller what the present status of a particular
/// node ID is, as well as what status transitions occurred since the
/// last time this node ID was checked. \sa ICBPD_GetNodeStatus()
typedef struct
{
	/// \brief Set to ::EVENT_OCCURRED whenever a node makes a transition from
	/// ::ONLINE to ::OFFLINE.
	uint8 offlineEvent :	1;

	/// \brief Set to ::EVENT_OCCURRED whenever a node makes a transition from
	/// ::OFFLINE to ::ONLINE.
	uint8 onlineEvent :		1;

	/// \brief Indicates the status of the node ID at the present moment.
	uint8 currentStatus :	1;

} NodeStatus;
//----------------------------------------------------------------------------

/// The Node ID type
typedef uint8 NodeID;

//----------------------------------------------------------------------------
/// List of all possible return values from the API.
typedef enum
{
	/// Function completed request successfully.
	ICBPDError_Ok = 0,

	/// The provided pointer was invalid.
	ICBPDError_BadPointer,

	/// The content of a parameter was out of range.
	ICBPDError_BadContent,

	/// No information exists for the specified ID.
	ICBPDError_NonexistantID,

    /// The operation cannot be completed because traffic is being suppressed
    ICBPDError_TrafficSuppressed,

} ICBPDErrorCode;
//----------------------------------------------------------------------------

// Function signature of callback function for high priority CAN messages
typedef void( *HighPriorityCANReceiveFunction_t )(NodeID id, const uint8* data, uint8 size);

// Note: When traffic is suppressed, certain message will not be sent.

typedef enum
{
	ICBIContolMode_HoldInBootloadMode = 0,
	ICBIContolMode_Reset,
	ICBIContolMode_SuppressTraffic,
} ICBIContolMode_t;

//----------------------------------------------------------------------------
//! \brief	Retrieve the value of the local component's Node ID.
//!
//! \param	pNodeID	A pointer to the location to which the local component's
//!			Node ID should be copied.
//!
//! \return
//!	\li	::ICBPDError_Ok if successful.
//!	\li	::ICBPDError_BadPointer if \c pNodeID is set to NULL.
//!	\li	::ICBPDError_NonexistantID if the local system has not yet
//! established a Node ID for itself.
//----------------------------------------------------------------------------
ICBPDErrorCode	ICBPD_GetLocalNodeID( NodeID *pNodeID );



//----------------------------------------------------------------------------
//! \brief	Retrieve the configuration ID of a particular node.
//!
//! \param	id			The node ID of the component in question.
//!	\param	pConfigID	A pointer to the location to which the specified
//!			node's configuration ID should be copied.
//!
//! \b Notes:
//!	\li	The caller may specify the node ID of the local component.  This
//! would be a roundabout way of invoking ::SYS_GetConfigurationID().
//!
//! \return
//!	\li	::ICBPDError_Ok if successful.
//!	\li	::ICBPDError_BadPointer if \c pConfigID is set to NULL.
//!	\li	::ICBPDError_BadContent if the specified node ID is not within the
//!	range of valid node ID's.
//!	\li	::ICBPDError_NonexistantID if the specified node ID does not reference
//! a component in the system that is presently online.
//----------------------------------------------------------------------------
ICBPDErrorCode	ICBPD_GetConfigurationID( NodeID id, SysConfigID_t *pConfigID );



//----------------------------------------------------------------------------
//! \brief	Configure the module to generate a signal when a node's status
//!			changes.
//!
//! \param	taskID	The ID of the task to receive the signal.
//!	\param	signal	The signal to send to the specified task.
//!
//! This function will cause the specified task to receive the specified
//! signal whenever a new node is discovered by the ICB protocol handler,
//! or whenever an existing node is believed to have gone offline due to
//! lack of response.  The task receiving the signal is expected to
//! examine the status of all potential node ID's via calls to
//! ::ICBPD_GetNodeStatus(), so that the application may react appropriately
//! to the new, or now non-existant, node.
//!
//! \b Notes:
//! \li	Specifying a \c taskID of 0 will disable this signalling mechanism.
//! \li The validity of the \c taskID is not examined by this function.
//!
//! \return
//!	\li	::ICBPDError_Ok, always.
//----------------------------------------------------------------------------
ICBPDErrorCode	ICBPD_BindNodeEvent( uint8 taskID, uint8 signal );



//----------------------------------------------------------------------------
//! \brief	Get the online/offline status of a node.
//!
//! \param	id		The node ID for which to get the status.
//! \param	pStatus	A pointer to the ::NodeStatus location to which
//!					the specified node's status should be copied.
//!
//! This function will provide the caller with a copy of the module's
//!	::NodeStatus entry for the specified node ID.
//!
//! \b IMPORTANT:
//! Users should note that
//! two of the fields in the ::NodeStatus structure, specifically
//! ::NodeStatus::offlineEvent and ::NodeStatus::onlineEvent, are
//! "clear-on-read" fields; after the ::NodeStatus structure is copied
//! to the specified location, the aforementioned fields will be set to
//! ::NO_EVENT_OCCURRED in the module's internal copy, so that subsequent
//! checks of this node status will not result in the perception
//! of continual online/offline events.  In this sense, the two fields
//! should be looked upon as "edge-triggered" events.  Therefore, the
//! task that examines the results of this call must take appropriate
//! action in response to the online/offline events immediately, as
//! these indicators are not preserved between calls.
//! <p>
//! In general, the ::NodeStatus value provided by this call should be
//! interpreted as follows:
//! <p>
//! <TABLE>
//! <TR>
//! <TD><B>offlineEvent</B></TD><TD><B>onlineEvent</B></TD>
//! <TD><B>currentStatus</B></TD><TD><B>Interpretation</B></TD>
//! </TR>
//! <TR>
//! <TD>NO_EVENT_OCCURRED</TD><TD>NO_EVENT_OCCURRED</TD><TD>OFFLINE</TD>
//! <TD>Node still offline</TD>
//! </TR>
//! <TR>
//! <TD>NO_EVENT_OCCURRED</TD><TD>NO_EVENT_OCCURRED</TD><TD>ONLINE</TD>
//! <TD>Node still online</TD>
//! </TR>
//! <TR>
//! <TD>NO_EVENT_OCCURRED</TD><TD>EVENT_OCCURRED</TD><TD>OFFLINE</TD>
//! <TD><B><I>IMPOSSIBLE</I></B><TD>
//! </TR>
//! <TR>
//! <TD>NO_EVENT_OCCURRED</TD><TD>EVENT_OCCURRED</TD><TD>ONLINE</TD>
//! <TD>Node has recently gone from offline to online</TD>
//! </TR>
//! <TR>
//! <TD>EVENT_OCCURRED</TD><TD>NO_EVENT_OCCURRED</TD><TD>OFFLINE</TD>
//! <TD>Node has recently gone from online to offline</TD>
//! </TR>
//! <TR>
//! <TD>EVENT_OCCURRED</TD><TD>NO_EVENT_OCCURRED</TD><TD>ONLINE</TD>
//! <TD><B><I>IMPOSSIBLE</I></B><TD>
//! </TR>
//! <TR>
//! <TD>EVENT_OCCURRED</TD><TD>EVENT_OCCURRED</TD><TD>OFFLINE</TD>
//! <TD>Transient online "spike"; node still offline</TD>
//! </TR>
//! <TR>
//! <TD>EVENT_OCCURRED</TD><TD>EVENT_OCCURRED</TD><TD>ONLINE</TD>
//! <TD>Transient offline "spike"; re-evaluate node<SUP><B>1</B></SUP></TD>
//! </TR>
//! </TABLE>
//!
//! <B>Footnote 1</B>: No assumptions should be made about the state of the
//! component answering to this node ID.  The component may have
//! undergone a complete reset, or a component may have co-opted
//! another component's node ID, meaning that the component now answering
//! to this node ID may be a completely different device.
//! 
//! \b Notes:
//! \li	If this function call fails for any reason, the state of the
//! module's internal table will not be modified.
//! \li If this function is invoked with \c id set equal to the local
//! node ID, the function will always indicate a status of ::ONLINE,
//! and no offline/online transitions.
//!
//! \return
//!	\li	::ICBPDError_Ok if successful.
//!	\li	::ICBPDError_BadPointer if \c pStatus is set to NULL.
//!	\li	::ICBPDError_BadContent if the specified node ID is not within the
//!	range of valid node ID's.
//----------------------------------------------------------------------------
ICBPDErrorCode	ICBPD_GetNodeStatus( NodeID id, NodeStatus *pStatus );

//----------------------------------------------------------------------------
//! \brief	Provides feedback as to the status of node ID arbitration.
//!
//! \return
//!	\li	::TRUE	There have been no node ID request messages received for a
//! period of time that indicates all nodes are settled in on an ID.
//!	\li	::FALSE	Indicates there have been ID request messages recently.
//----------------------------------------------------------------------------
bool ICBPD_AllNodeIdsSettled( void );

//----------------------------------------------------------------------------
//! \brief	Sends a message that will cause all modules to reset.
//!
//! \param	mode	Specifies action to take on all modules.
//!
//! \param	holdtime_ms	Specifies the amount of time in ms that the modules will
//!			be held in the desired mode.
//!
//!	\b Notes:
//! \li There is no confirmation that this message is received by any or all
//!	modules
//!
//!	\b Notes:
//! \li This does not affect the calling module.  Only receiving modules
//!	will be affected.
//!
//! \li When a module is initially reset, the holdtime_ms parameter will not be
//! used.
//!
//! \return
//!	\li	::TRUE	Message sent sucessfully.
//!	\li	::FALSE	Message send failure.
//----------------------------------------------------------------------------
bool ICBI_ControlAllModules( ICBIContolMode_t mode, uint16 holdtime_ms );

//----------------------------------------------------------------------------
//! \brief	Sends a message that will cause all modules of a particular
//!			component class to reset.
//!
//! \param	mode	Specifies action to take on all modules.
//!
//!	\param	class	Class ID of modules that will be reset.
//!
//! \param	holdtime_ms	Specifies the amount of time in ms that the modules will
//!			be held in the desired mode.
//!
//!	\b Notes:
//! \li There is no confirmation that this message is received by any or all
//!	modules
//!
//!	\b Notes:
//! \li This does not affect the calling module.  Only receiving modules
//!	will be affected.
//!
//! \li When a module is initially reset, the holdtime_ms parameter will not be
//! used.
//!
//! \return
//!	\li	::TRUE	Message sent sucessfully.
//!	\li	::FALSE	Message send failure.
//----------------------------------------------------------------------------
bool ICBI_ControlModuleClass( ICBIContolMode_t mode, uint8 class, uint16 holdtime_ms );

//----------------------------------------------------------------------------
//! \brief	Sends a message that will cause a specific module to reset.
//!
//! \param	mode	Specifies action to take on all modules.
//!
//!	\param	id	Node ID of module that will be reset.
//!
//! \param	holdtime_ms	Specifies the amount of time in ms that the modules will
//!			be held in the desired mode.
//!
//!	\b Notes:
//! \li There is no confirmation that this message is received by any or all
//!	modules
//!
//!	\b Notes:
//! \li This does not affect the calling module.  Only receiving modules
//!	will be affected.
//!
//! \li When a module is initially reset, the holdtime_ms parameter will not be
//! used.
//!
//! \return
//!	\li	::TRUE	Message sent sucessfully.
//!	\li	::FALSE	Message send failure.
//----------------------------------------------------------------------------
bool ICBI_ControlSpecificModule( ICBIContolMode_t mode, NodeID id, uint16 holdtime_ms );

//----------------------------------------------------------------------------
//! \brief	Checks to see if traffic is being suppressed on the local module.
//!
//! \return
//!	\li	::TRUE	Messages are being suppressed.
//!	\li	::FALSE	Messages are not being suppressed.
//----------------------------------------------------------------------------
bool ICBI_CheckTrafficSuppress( void );

//----------------------------------------------------------------------------
//! \brief	Suppresses trafic on the local module only.
//!
//! \param	holdtime_ms	Specifies the amount of time in ms that the calling
//!			module will have its traffic suppressed.
//----------------------------------------------------------------------------
void ICBI_SetLocalTrafficSuppress( uint16 holdtime_ms );

//----------------------------------------------------------------------------
//! \brief	Transmit a high priority CAN message across the network
//!
//! \param	data	An array containing the data to be transmitted (1-8 bytes)
//!	\param	size	The number of bytes to be transmitted (must be 1-8)
//!
//! This function will transmit a high priority CAN message. This message
//! has arbitration priority over all other ICBP message types. When received
//! by other nodes on the CAN network, it will be processed by each nodes'
//! callback function as registered by ICBPD_SetHighPriorityRxCallback
//!
//! \return
//!	\li	::ICBPDError_Ok if successful
//! \li ::ICBPDError_BadContent if size is out of range
//! \li ::ICBPDError_NonexistantID if the message could not be sent
//----------------------------------------------------------------------------
ICBPDErrorCode	ICBPD_SendHighPriorityMessage( const uint8* data, uint8 size );

//----------------------------------------------------------------------------
//! \brief	Register a callback function to handle received high priority
//!			CAN messages
//!
//! \param	callbackFunction	A function pointer to the callback function
//!
//! This function registers a callback function that is called whenever a
//! high priority CAN message is received. The callback function is
//! called in a common-level communication task, so it must return quickly
//! and not call any blocking functions.
//!
//! \return
//!	\li	::ICBPDError_Ok always
//----------------------------------------------------------------------------
ICBPDErrorCode	ICBPD_SetHighPriorityRxCallback( HighPriorityCANReceiveFunction_t callbackFunction );

//----------------------------------------------------------------------------
//! \brief	Change the frequency that CAN messages are polled at
//!
//! \param	frequency	The desired polling frequency (in Hz)
//!
//! This function TODO
//!
//! \return
//!	\li	::ICBPDError_Ok if the change was successful
//! \li ::ICBPDError_BadContent if the requested frequency is out of range
//! \li ::ICBPDError_NonexistantID if the RTOS returned a timer error
//----------------------------------------------------------------------------
ICBPDErrorCode	ICBPD_SetPollingFrequency( ICBP_POLL_FREQ frequency );





//@}

#endif // ICBP_H

//@}
