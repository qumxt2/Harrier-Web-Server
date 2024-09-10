//! \file	io_typedef.h
//! \brief type defines for Input and/or Output modules.
//!
//! Copyright 2006-2008
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!


#ifndef IO_TYPEDEF_H
#define IO_TYPEDEF_H

#include "typedef.h"


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************

// DEVELOPER'S NOTE:  The following type(s) exist on both the primary and graphics processors
// of the ADM.  Any changes made in either location must be reflected in the opposite 
// location to ensure proper operation.  

typedef enum
{
	NOT_ASSERTED = 0,
	ASSERTED = 1
} IOState_t;

typedef uint32 u16d16_t;
typedef sint32 s16d16_t;
typedef s16d16_t mV_s16d16_t;
typedef s16d16_t mA_s16d16_t;
typedef u16d16_t ohm_u16d16_t;
typedef s16d16_t bar_s16d16_t;
typedef u16d16_t cc_u16d16_t;
typedef uint32 cc_u32d0_t;
typedef u16d16_t cc_per_min_u16d16_t;


typedef enum
{
	IOError_Ok = 0,
	IOError_InvalidPinName,
	IOError_InvalidPinConfiguration,
	IOError_OutOfRange,
	IOError_UNDEFINED
} IOErrorCode_t;


typedef struct
{
	IOState_t		state;
	IOErrorCode_t	error;
} IOrtn_digital_t;


typedef struct
{
	uint16					u16;
	IOErrorCode_t			error;	
} IOrtn_uint16_t;

typedef struct
{
	uint32					u32;
	IOErrorCode_t			error;	
} IOrtn_uint32_t;

typedef struct
{
	u16d16_t				u16d16;
	IOErrorCode_t			error;	
} IOrtn_u16d16_t;

typedef struct
{
	s16d16_t				s16d16;
	IOErrorCode_t			error;	
} IOrtn_s16d16_t;



typedef struct
{
	mV_s16d16_t				mV_s16d16;
	IOErrorCode_t			error;	
} IOrtn_mV_s16d16_t;


typedef struct
{
	ohm_u16d16_t			ohm_u16d16;
	IOErrorCode_t			error;
} IOrtn_ohm_u16d16_t;	


typedef struct
{
	mA_s16d16_t				mA_s16d16;
	IOErrorCode_t			error;
} IOrtn_mA_s16d16_t;	


typedef struct
{
	bar_s16d16_t			bar_s16d16;
	IOErrorCode_t			error;	
} IOrtn_bar_s16d16_t;

typedef struct
{
	cc_u16d16_t				cc_u16d16;
	IOErrorCode_t			error;	
} IOrtn_cc_u16d16_t;

typedef struct
{
	cc_per_min_u16d16_t		cc_per_min_u16d16;
	IOErrorCode_t			error;	
} IOrtn_cc_per_min_u16d16_t;


#endif // IO_TYPEDEF
