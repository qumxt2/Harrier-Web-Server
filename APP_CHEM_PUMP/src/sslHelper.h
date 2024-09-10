// sslHelper.h

// Copyright 2015
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// The header file for ssl communications

#ifndef SSLHELPER_H
#define	SSLHELPER_H

#include "typedef.h"
#include "stdint.h"

// **********************************************************************************************************
// Constants
// **********************************************************************************************************


// **********************************************************************************************************
// Enumerations
// **********************************************************************************************************


// **********************************************************************************************************
// Public functions
// **********************************************************************************************************

bool SSL_Init(void);
bool SSL_Connect(const char* hostname, bool useSni);
void SSL_Disconnect(void);
int SSL_readBytes(uint8* buf, int maxBytes);
sint16 SSL_writeBytes(const char* buf, uint16 numBytes);
sint16 SSL_isError(void);

#endif	/* SSLHELPER_H */

