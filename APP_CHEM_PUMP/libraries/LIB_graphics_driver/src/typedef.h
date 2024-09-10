// typedef.h
 
// Copyright 2006
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains all of the standardized Graco type definitions for use with the Microchip C30 compiler
// This file also attempts to include stddef.h which, among other things, should have "the one true" defininition of NULL

#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <stddef.h>

// 8-bit signed integer.
typedef signed char sint8;

// 16-bit signed integer.
typedef signed int sint16;

// 32-bit signed integer.
typedef signed long sint32;

// 64-bit signed integer.
typedef signed long long sint64;

// 8-bit unsigned integer.
typedef unsigned char uint8;

// 16-bit unsigned integer.
typedef unsigned int uint16;

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
#define FALSE (0)

// Defines the default value for a true expression.  For use with 'bool' data type.
#define TRUE (1)

#endif // TYPEDEF_H
