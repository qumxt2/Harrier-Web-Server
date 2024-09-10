//! \file	CANBL_client_load.h
//! Copyright 2006-12, 2015
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef CANBL_CAN_CLIENT_LOAD_H
#define CANBL_CAN_CLIENT_LOAD_H

typedef enum
{
	CanUpdateHeaderResponse_NextHeader = 0,
	CanUpdateHeaderResponse_NotApplicable,
	CanUpdateHeaderResponse_NoChange,
	CanUpdateHeaderResponse_AcceptUpdate,

	CanUpdateHeaderResponse_NumOptions,
} CanUpdateHeaderResponse_t;

typedef enum
{
	CanUpdateState_WaitForUpdateHeader0 = 0,
	CanUpdateState_WaitForUpdateHeader1,
	CanUpdateState_WaitForUpdateHeader2,
	CanUpdateState_WaitForUpdateHeader3,
	CanUpdateState_WaitForUpdateHeader4,
	CanUpdateState_WaitForUpdateHeader5,
	CanUpdateState_WaitForUpdateHeader6,
	CanUpdateState_WaitForUpdateHeader7,
	CanUpdateState_WaitForPacketHeaderOrUpdateEnd,
	CanUpdateState_WaitForDataOrPacketEnd,

	CanUpdateState_NumStates,
} CanUpdateState_t;


typedef CanUpdateHeaderResponse_t (*CanBootloaderUpdateHeaderHandler)( uint8 *pBuffer, CanUpdateState_t headerState );

typedef void (*CanBootloaderUpdateEndHandler)( void );

typedef void (*CanBootloaderPacketHeaderHandler)( void );

// Must result in or trigger a call to CANBL_client_SendPacketAckCmd()
typedef void (*CanBootloaderPacketEndHandler)( uint32 length );

typedef void (*CanBootloaderDataHandler)( uint8 c );

typedef struct
{
	uint8 CurrentHostId;
	uint16 SequenceNumber;
	uint16 PacketLength_bytes;
	uint16 ReceivedBytes;
	CanUpdateState_t CurrentState;
	bool UpdateInProcess;
	uint8* pDataBuffer;
	uint32 BufferSize;
	CanBootloaderUpdateHeaderHandler UpdateHeaderHandler;
	CanBootloaderUpdateEndHandler UpdateEndHandler;
	CanBootloaderPacketHeaderHandler PacketHeaderHandler;
	CanBootloaderPacketEndHandler PacketEndHandler;
	CanBootloaderDataHandler DataHandler;
} CanUpdateData_t;

void CANBL_InitClientLoader( CanBootloaderUpdateHeaderHandler update_header_handler,
							 CanBootloaderUpdateEndHandler update_end_handler,
							 CanBootloaderPacketHeaderHandler packet_header_handler,
							 CanBootloaderPacketEndHandler packet_end_handler,
							 CanBootloaderDataHandler data_handler );


void CANBL_client_SendPacketAckCmd( bool success );

void CANBL_client_SendUpdateCompleteAckCmd( bool success );

void CANBL_client_SendDataFlowCtrlCmd( uint32 holdtime_ms );

#endif // CANBL_CAN_CLIENT_LOAD_H
