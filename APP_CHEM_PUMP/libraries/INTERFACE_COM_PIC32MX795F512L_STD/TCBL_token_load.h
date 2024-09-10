//! \file	TCBL_token_load.h
//! Copyright 2006-12
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef TCBL_TOKEN_LOAD_H
#define TCBL_TOKEN_LOAD_H

#include "CANBL_can_host.h"

// ***************************************************
// * MACROS
// ***************************************************

#define UNSPECIFIED_PACKET_LENGTH_BYTES				(0xFFFF)

#define	BLOCK_TYPE_FIELD_SIZE						(sizeof(uint16))
#define	CRC32_FIELD_SIZE							(sizeof(uint32))
#define	MINIMUM_UPDATE_BLOCK_SIZE					(BLOCK_TYPE_FIELD_SIZE + CRC32_FIELD_SIZE)

// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************

typedef uint32 (*TCBL_UpdateGetLength)( void );
typedef uint32 (*TCBL_UpdateGetDelay)( void );
typedef bool (*TCBL_UpdateCheckForValidity)( void );
typedef void (*TCBL_UpdateSendData)( void );
typedef void (*TCBL_UpdateSendHeader)( CanBootloadResults_t* results );
typedef bool (*TCBL_UpdateSecondaryCheck)( void );
typedef void (*TCBL_UpdateConfigMultiple)( void );

typedef struct TCBL_UpdateConfigData
{
    uint16 update_type;
    TCBL_UpdateGetLength updateGetLength;
    TCBL_UpdateGetDelay updateGetDelay;
    TCBL_UpdateCheckForValidity updateGetValidity;
    TCBL_UpdateSendData updateSendData;
    TCBL_UpdateSendHeader updateSendHeader;
    TCBL_UpdateSecondaryCheck updateSecondaryCheck;
    TCBL_UpdateConfigMultiple updateConfigMultiple;
    struct TCBL_UpdateConfigData* nextUpdate;
    bool piRequired;
} TCBL_UpdateConfigData_t;

// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************

// To be called on startup to latch the token present state.
void TCBL_LatchTokenState( void );

// Initializes the token CAN bootloader functionality
void TCBL_TokenLoadInit( void );

// Registers an update configuration
void TCBL_RegisterUpdateConfig( TCBL_UpdateConfigData_t* update_config, uint16 update_type,
        TCBL_UpdateGetLength update_get_length, TCBL_UpdateGetDelay update_get_delay,
        TCBL_UpdateCheckForValidity update_get_validity, TCBL_UpdateSendData update_send_data,
        TCBL_UpdateSendHeader update_send_header, TCBL_UpdateSecondaryCheck update_secondary_check,
        TCBL_UpdateConfigMultiple update_config_multiple, bool pi_required );

// Gets the update type (block ID) when pointing at the beginning of a file.
// returns TRUE if successful, FALSE otherwise.
bool TCBL_CheckBlockType( uint16* updateType );

// Sends the current in packets with the maximum size specified in max_packet_length.
// returns TRUE if successful, FALSE otherwise
bool TCBL_SendRecord( uint16 max_packet_length );

#endif // TCBL_TOKEN_LOAD_H
