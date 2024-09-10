//! \file	canbuffer.h
//! \brief CAN buffer pool header file.
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_comm_internal
//@{

//! \defgroup can_buffer CAN Buffer
//! \brief CAN Buffer pool.
//!
//! \b DESCRIPTION:
//!		This module provides a simple interface for allocating and
//!		deallocating specialized buffers from a fixed resource pool.
//!		These buffers are used by the ICB daemon (icbp.c) and the CAN
//!		driver module (candrv.c) to exchange inbound and outbound CAN
//!		frames.

#ifndef CANBUFFER_H
#define CANBUFFER_H

//! \ingroup can_buffer	
//@{

/// The return value from ::AllocCANBuffer() if no CAN buffer is currently
/// available.
/// \hideinitializer
#define	NO_BUFFER_AVAILABLE				((CANBuffer *) NULL)

#define MAX_BUFFER_DEPTH_UNSPECIFIED	(0xFF)

//----------------------------------------------------------------------------

/// Identities of the entities that may request CAN buffers.
/// The identity is passed as a parameter to ::AllocCANBuffer().
typedef enum
{
	/// ICB Protocol Daemon, Tx direction, priority 0 (High Priority messages)
	ALLOC_REQ_ID_ICBD_TX_HIGH_PRIORITY = 0,

	/// ICB Protocol Daemon, Tx direction, priority 1 (heartbeat messages)
	ALLOC_REQ_ID_ICBD_TX_HEARTBEAT,
	
	/// ICB Protocol Daemon, Tx direction, priority 2 (Highest priority Operational messages)
	ALLOC_REQ_ID_ICBD_TX_OP0,
	
	/// ICB Protocol Daemon, Tx direction, priority 3 (Autoresponse to Setpoint Request messages)
	ALLOC_REQ_ID_ICBD_TX_OP1,

	/// ICB Protocol Daemon, Tx direction, priority 4 (Setpoint Request messages)
	ALLOC_REQ_ID_ICBD_TX_OP2,

	/// ICB Protocol Daemon, Tx direction, priority 5 (Standard Broadcast messages)
	ALLOC_REQ_ID_ICBD_TX_OP3,

	/// ICB Protocol Daemon, Tx direction, priority 6 (Lowest Priority Operational messages)
	ALLOC_REQ_ID_ICBD_TX_OP4,

	/// ICB Protocol Daemon, Tx direction, priority 7 (Administration messages)
	ALLOC_REQ_ID_ICBD_TX_ADMIN,

	/// CAN Driver, Rx direction
	ALLOC_REQ_ID_CAN_DRIVER_RX,
	
	/// ICB Protocol Daemon, LOOPBACK message 
	ALLOC_REQ_ID_ICBD_LOOPBACK,

#ifdef GATEWAY_MODULE
	/// Gateway daemon (not yet implemented)
	ALLOC_REQ_ID_GATEWAY,
#endif
	/// @cond

	/// Maximum valid parameter
	ALLOC_REQ_ID_MAX

	/// @endcond

} BufferReqId;

//----------------------------------------------------------------------------

/* Create a CMSGSID data type. */
typedef struct
{
	unsigned SID:11;
	unsigned :21;
}txcmsgsid;

/* Create a CMSGEID data type. */
typedef struct
{
	unsigned DLC:4;
	unsigned RB0:1;
	unsigned :3;
	unsigned RB1:1;
	unsigned RTR:1;
	unsigned EID:18;
	unsigned IDE:1;
	unsigned SRR:1;
	unsigned :2;
}txcmsgeid;

/* Create a CMSGDATA0 data type. */
typedef struct
{
	unsigned Byte0:8;
	unsigned Byte1:8;
	unsigned Byte2:8;
	unsigned Byte3:8;
}txcmsgdata0;

/* Create a CMSGDATA1 data type. */
typedef struct
{
	unsigned Byte4:8;
	unsigned Byte5:8;
	unsigned Byte6:8;
	unsigned Byte7:8;
}txcmsgdata1;

/// Structure used to convey ICB messages to and from the CAN driver.
typedef union uCANTxMessageBuffer
{
	struct
	{
		txcmsgsid CMSGSID;
		txcmsgeid CMSGEID;
		union
		{
			struct
			{
				txcmsgdata0 CMSGDATA0;
				txcmsgdata1 CMSGDATA1;
			};
			uint8 data[8];
		};
	};
	int messageWord[4];
}CANBuffer;

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the remainder of this interface.
//!
//! \return A negative value if the initialization failed, 0 on success.
//----------------------------------------------------------------------------
sint16 CANBufferInit		(void);				// initialize buffer resources


//----------------------------------------------------------------------------
//! \brief Retrieve an instance of ::CANBuffer from the free pool.
//!
//! \param requestSource	The nature of the thread making the request.
//!
//! \b Notes:
//!	\li	The ::BufferReqID value passed to this function enables the
//!		module to limit the number of CANBuffer items that any particular
//!		entity can possess at any given moment.  The intent is to prevent
//!		a client in the message-passing pipeline from starving all other
//!		clients through resource-hogging.
//!	\li	Buffers received from this function should be placed back into the
//!		free pool by calling ::FreeCANBuffer().
//!	\li	Tasks must \b not attempt to short-cut the CANBuffer allocation process
//!		by internally recycling CANBuffer items.  Doing so will cripple
//!		the mechanism that guarantees a roughly even distribution of
//!		buffers to all clients.
//!
//! \return
//!	\li A pointer to a CANBuffer structure if successful.
//!	\li ::NO_BUFFER_AVAILABLE if no free buffers are immediately available.
//----------------------------------------------------------------------------
CANBuffer *AllocCANBuffer( BufferReqId requestSource );


//----------------------------------------------------------------------------
//! \brief Return an instance of CANBuffer to the free pool.
//!
//! \param pBuffer		A pointer to the buffer to return to the free pool.
//!
//! \b Notes:
//!	\li	Once this function has been invoked for a specified buffer pointer,
//!		that pointer is longer valid.
//!	\li	It is not necessarily the responsibility of the thread that allocated
//!		a particular CANBuffer to free it.  Rather, the buffer should be
//!		freed by the entity that retained possession of the pointer, and
//!		determined that no entity has any more use for the contents of the
//!		buffer.
//!	\li	Tasks must \b not attempt to short-cut the CANBuffer allocation process
//!		by internally recycling CANBuffer items.  Doing so will cripple
//!		the mechanism that guarantees a roughly even distribution of
//!		buffers to all clients.
//!
//! \return
//!	\li A pointer to a CANBuffer structure if successful.
//!	\li ::NO_BUFFER_AVAILABLE if no free buffers are immediately available.
//----------------------------------------------------------------------------
void FreeCANBuffer( CANBuffer *pBuffer );

//----------------------------------------------------------------------------
//! \brief Sets the maxiumum number of buffers that can be allocated for a
//!		   particular message type.
//!
//! \param requestSource	The nature of the message
//! \param max_depth		The maximum number of buffers allowed
//!
//! \b Notes:
//!	\li	This function does not fix the number of buffers allocated to a
//!		particular message type.
//----------------------------------------------------------------------------
void SetMaxBufferDepth( BufferReqId id, uint8 max_depth );

//@}

#endif // CANBUFFER_H

//@}
