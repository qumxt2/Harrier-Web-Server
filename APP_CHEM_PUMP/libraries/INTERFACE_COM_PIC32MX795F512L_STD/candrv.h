//! \file candrv.h
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

//! \addtogroup gca_comm_internal
//@{

//! \defgroup can_driver CAN Driver
//! \brief Low-level hardware driver for PIC24H CAN peripheral.
//!
//!		This driver module standardizes access to the ECAN peripheral.  The
//!		module actively monitors the ECAN peripheral for incoming CAN frames,
//!		queues up outbound CAN frames for transmission, and monitors the
//!		controller for error conditions.

#ifndef CANDRV_H
#define CANDRV_H

//! \ingroup can_driver
//@{

// ***************************************************************************
// * Include Platform Support
// ***************************************************************************

#include "typedef.h"
#include "canbuffer.h"


// ***************************************************************************
// * Include Module Support
// ***************************************************************************


// ***************************************************************************
// * Public Defines and Constants
// ***************************************************************************

/// Enumerated list of available errors
typedef enum
{
	 CANDrvError_Ok = 0
	,CANDrvError_NoMessageAvailable
	,CANDrvError_BadParameter
	,CANDrvError_BadContent
	,CANDrvError_ResourcesUnavailable
} CANDrvErrorCode;

/// Enumerated list of available channels.
typedef enum
{
	 ECAN1 = 0
	/* ,ECAN2		 ECAN channel 2 is not supported in this library	*/
	,LAST_CHAN
} CAN_CHAN;

typedef enum
{
	STANDARD_FREQ = 0,
	HIGH_FREQ,
	LAST_FREQUENCY
} CAN_POLL_FREQ;

/// Enumerated list of available priorities.
typedef enum 
{
	 NORMAL = 0
	,HIGH
	,LAST_PRIORITY
} CAN_PRIORITY;


// ***************************************************************************
// * Public Attributes
// ***************************************************************************


// ***************************************************************************
// * Available Public Methods
// ***************************************************************************

//----------------------------------------------------------------------------
//! \brief Initializes the ECAN peripheral for use.
//!
//! This method does not accept any input parameter.
//!
//! \return
//! Returns 0 for success.  A negative integer indicates failure.
//----------------------------------------------------------------------------
sint8 CANDrvInit(void);



//----------------------------------------------------------------------------
//! \brief	Change the frequency at which the CAN driver polls for received
//!			messages
//!
//! \param	chan
//! The CAN channel to associate the mailboxId with
//!
//! \param	frequency
//! The freqeuncy that the CAN driver polls for messages, either
//! STANDARD_FREQ (200Hz) or HIGH_FREQ (1000Hz)
//!
//! The default polling frequency is 200Hz, and that should work for almost
//! all applications. This function is provided to allow for high-speed
//! control loops that need higher bandwidth and lower latency.
//!
//! This function must be called after the CAN driver task has already started
//! running. This can be done by calling this function from inside a task,
//! with a K_Task_Wait( 1 ) at some point before this function call. (This
//! allows the CAN driver task to create the timer before this function
//! attempts to modify the timer.)
//!
//! \return
//! \li ::CANDrvError_Ok for success.  Any other integer indicates failure.
//! \li ::CANDrvError_BadParameter if the requested frequency is invalid
//! \li ::CANDrvError_ResourcesUnavailable if K_Timer_Cyclic() failed
//----------------------------------------------------------------------------
CANDrvErrorCode CANDrvSetRxPollFrequency(CAN_CHAN chan, CAN_POLL_FREQ frequency);



//----------------------------------------------------------------------------
//! \brief Inform the CAN driver where to send Rx'd Messages.
//!
//! \param chan
//! The CAN channel to associate the mailboxId with
//!
//! \param mailboxId
//! The mailbox ID to send the messages to.
//!
//! \return
//! Returns CANDrvError_Ok for success.  Any other integer indicates failure.
//----------------------------------------------------------------------------
CANDrvErrorCode CANDrvSetRxMailbox(CAN_CHAN chan, uint8 mailboxId);



//----------------------------------------------------------------------------
//! \brief Submits a CAN packet for transmission.
//!
//! \param chan
//! The destination CAN channel for this packet.
//!
//! \param priority
//! The priority for this CAN packet.
//!
//! \param pBuffer
//! A pointer to the buffer (data) to be transmitted.
//!
//! \b Notes:
//!	\li	If this function indicates success, then the information in the CAN 
//!     buffer pointed to by pBuffer has been copied over to the CAN peripheral
//!     and *pBuffer is no longer needed by this module.  
//! \li The CAN buffer pointed to by pBuffer is NOT freed by this module.
//!     This must be performed at the caller level after this function indicates success.
//!
//! \return
//! Returns CANDrvError_Ok for success.  Any other integer indicates failure.
//----------------------------------------------------------------------------
CANDrvErrorCode CANDrvSubmitForTx( CAN_CHAN chan, CAN_PRIORITY priority, CANBuffer *pBuffer );

// Functions that allow for external control over the CAN communications LED
// Intended to be used for functional testing.  No need to use these to indicate
// CAN communication at the application level.
void CANDriver_EnableExtCANLedControl( void );
void CANDriver_DisableExtCANLedControl( void );

// Transmits a CAN message directly.  This function is only to be used when absolutely necessary, such as prior
// the RTOS being fired up.
void CANDrvEmergencyTx(CAN_CHAN chan, uint16 sid, uint8 dlc, uint8 *pData);

uint64 CANDrvGetTotalRxMessages( void );

uint64 CANDrvGetTotalTxHighPriMessages( void );

uint64 CANDrvGetTotalTxNormalPriMessages( void );

void CANDrvResetMessageStats( void );

uint64 CANDrvMessagesLastTickReading( void );

//@}

#endif // CANDRV_H

//@}
