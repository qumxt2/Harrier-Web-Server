//! \file	CANBL_can_host_cmd.h
//! Copyright 2006-12
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef CANBL_CAN_HOST_CMD_H
#define CANBL_CAN_HOST_CMD_H

void CANBL_Cmd_Init( void );

void CANBL_Cmd_PrepForUpdateCheck( CanBootloadResults_t* results );

bool CANBL_Cmd_SendUpdateHeader( uint8* pData, uint8 size );

void CANBL_Cmd_WaitOnUpdateResults( CanBootloadResults_t* results );

bool CANBL_Cmd_SendPacketHeader( uint16 packet_length, uint16 sequence );

bool CANBL_Cmd_SendPacketData( uint8* data, uint16 length );

bool CANBL_Cmd_SendPacketEnd( void );

bool CANBL_Cmd_SendUpdateEnd( void );


#endif // CANBL_CAN_HOST_CMD_H
