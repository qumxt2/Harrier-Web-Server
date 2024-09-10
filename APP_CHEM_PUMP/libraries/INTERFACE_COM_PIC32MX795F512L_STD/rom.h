// rom.h
 
// Copyright 2006
// Graco Inc., Minneapolis, MN
// All Rights Reserved

/*
** This library contains functions for reading data from Program Memory
*/

#ifndef ROM_H
#define ROM_H


#include "typedef.h"

#define VIRT_TO_PHYS_ADDR_CONST				(0x80000000)
#define PIC32_VIRT_TO_PHYS_ADDR(addr)		( (addr) | (VIRT_TO_PHYS_ADDR_CONST))

uint8 	RomReadUint8 (uint32 address);
uint16	RomReadUint16 (uint32 address);
uint32	RomReadUint32 (uint32 address);
uint64	RomReadUint64 (uint32 address);


// Read a string from Program memory - return value is string length
uint16	RomReadString (uint32 address, uint8 *destBuffer, uint16 bufferLength);

// Read an uint8 array from Program memory (unpacked, 2 uint8 per program word)
//   return value is actual number of uint8s read
uint16 	RomReadUint8Array (uint32 address, uint8 *dest, uint16 readcount);

// Read an uint16 array from Program memory (unpacked, 1 uint16 per program word)
//   return value is actual number of uint16s read
uint16 	RomReadUint16Array (uint32 address, uint16 *dest, uint16 readcount);

#endif /* _ROM_H */
