// typedef.h
 
// Copyright 2006
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains all of the standardized Graco type definitions for use with the Microchip XC32 compiler
// This file also attempts to include stddef.h which, among other things, should have "the one true" defininition of NULL

#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <stddef.h>

#ifndef NULL_TERM_CHAR
	#define NULL_TERM_CHAR			(0x00)
#endif

// 8-bit signed integer.
typedef signed char sint8;

// 16-bit signed integer.
typedef signed short sint16;

// 32-bit signed integer.
typedef signed long sint32;

// 64-bit signed integer.
typedef signed long long sint64;

// 8-bit unsigned integer.
typedef unsigned char uint8;

// 16-bit unsigned integer.
typedef unsigned short uint16;

// 32-bit unsigned integer.
typedef unsigned long uint32;

// 64-bit unsigned integer.
typedef unsigned long long uint64;

// 32-bit floating point.
typedef float float32;

// 64-bit floating point.
typedef long double float64;

#ifndef __cplusplus
// Boolean value: 0 = false, 1 = true
typedef unsigned char bool;
#endif

// Defines the default value for a false expression.  For use with 'bool' data type.
#ifndef FALSE
#define FALSE (0)
#endif

// Defines the default value for a true expression.  For use with 'bool' data type.
#ifndef TRUE
#define TRUE (1)
#endif

#ifndef VOID
#define VOID void
#endif
typedef bool BOOLEAN;
typedef uint8 INT8U;
typedef uint16 INT16U;
typedef uint32 INT32U;
typedef sint32 INT32;

#endif // TYPEDEF_H
