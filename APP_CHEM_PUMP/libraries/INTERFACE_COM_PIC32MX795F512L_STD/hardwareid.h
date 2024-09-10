// hardwareid.h
 
// Copyright 2006
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// DESCRIPTION

#ifndef HARDWAREID_H
#define HARDWAREID_H

#include "typedef.h"


#define HARDWARE_ID_INVALID			(0xFFFFUL)
#define HARDWARE_VERSION_INVALID	(0xFFFFUL)
#define HARDWARE_WORD_UNPROGRAMMED	(0xFFFFUL)

typedef enum
{
	HARDWARE_ID = 0,
	HARDWARE_VERS,
	HARDWARE_WORD2,
	HARDWARE_WORD3,
	NUM_HARDWARE_WORDS = 4
} HARDWAREID_WORD_DESC;

uint16 HWID_GetWord( HARDWAREID_WORD_DESC desiredWord );

// Serial Number String is 8 characters + null terminator
//  (in other words, buffer better damn well be able to 
//   hold at least 9 characters).
void HWID_GetSerialNumberString( uint8 *buffer );

#endif // SYSCFGID_H

