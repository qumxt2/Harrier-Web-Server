// d2a.c

// Copyright 2006-2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// Functions to interface with Digital to Analog converters
// (Analog Devices p/n AD5338) on EFCM (Enhanced Fluid Control Module)

#include "d2a.h"
				
#include "rtos.h"

#include "i2c_channel2.h"					// I2C function prototypes and constants


// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************

typedef struct
{
	uint8 address;
	uint8 channel;
}D2A_Address_t;

// ***************************************************
// * CONSTANTS
// ***************************************************

static const D2A_Address_t const d2a_i2c_addresses[D2A_NUM_CHANNELS] =
{
	[D2A_MODULE_0_CHA] = {0x0C, 0x01},
	[D2A_MODULE_0_CHB] = {0x0C, 0x02},
	[D2A_MODULE_1_CHA] = {0x0D, 0x01},
	[D2A_MODULE_1_CHB] = {0x0D, 0x02},
	[D2A_MODULE_2_CHA] = {0x22, 0x01},
	[D2A_MODULE_2_CHB] = {0x22, 0x02},
	[D2A_MODULE_3_CHA] = {0x23, 0x01},
	[D2A_MODULE_3_CHB] = {0x23, 0x02},
};

static const char * const INVALID_STRING = "INVALID";
static const char * const D2A_CHANNEL_STRINGS[D2A_NUM_CHANNELS] =
{
	[D2A_MODULE_0_CHA] = "D2A_MODULE_0_CHA",
	[D2A_MODULE_0_CHB] = "D2A_MODULE_0_CHB",
	[D2A_MODULE_1_CHA] = "D2A_MODULE_1_CHA",
	[D2A_MODULE_1_CHB] = "D2A_MODULE_1_CHB",
	[D2A_MODULE_2_CHA] = "D2A_MODULE_2_CHA",
	[D2A_MODULE_2_CHB] = "D2A_MODULE_2_CHB",
	[D2A_MODULE_3_CHA] = "D2A_MODULE_3_CHA",
	[D2A_MODULE_3_CHB] = "D2A_MODULE_3_CHB",
};

// ***************************************************
// * MACROS
// ***************************************************

#define I2C_WRITE_ADDR(addr)            (addr<<1)
//#define I2C_READ_ADDR(addr)             ((addr<<1) | 0x01)

// ***************************************************
// * PRIVATE (STATIC) VARIABLES
// ***************************************************

// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************
sint16 D2A_Initialize( void )
{
	// Make sure the I2C2 driver has been initalized
	return I2C2_Init();
}


bool D2A_Output_Get( D2A_CHANNEL_t channel, uint16 *value )
{
	bool success = TRUE;

	if( channel >= D2A_NUM_CHANNELS )
	{
		return FALSE;
	}
	
	if( K_Resource_Wait( I2C2_Get_Resource_ID(), 0 ) != K_OK )
	{
		return FALSE;
	}
	
	success &= I2C2_Generate_START();
	success &= I2C2_Data_Transmit( d2a_i2c_addresses[channel].address << 1 );
	success &= I2C2_Wait_ACK();
	success &= I2C2_Data_Transmit( d2a_i2c_addresses[channel].channel );
	success &= I2C2_Wait_ACK();
	
	success &= I2C2_Generate_REPEATED_START();
	success &= I2C2_Data_Transmit( (d2a_i2c_addresses[channel].address << 1) | 0x01 );
	success &= I2C2_Wait_ACK();
	
	success &= I2C2_Data_Receive( (uint8*)value + 1 );
	success &= I2C2_Generate_ACK();
	success &= I2C2_Data_Receive( (uint8*)value );
	success &= I2C2_Generate_NOACK();
	success &= I2C2_Generate_STOP();

	*value = *value & D2A_FULLSCALE;
	
	// we know we got the resource earlier in this function, so we can
	// safely ignore the return value.
	(void)K_Resource_Release( I2C2_Get_Resource_ID() );

	return success;
}


bool D2A_Output_Set( D2A_CHANNEL_t channel, uint16 value )
{
	bool success = TRUE;

	if( ( channel >= D2A_NUM_CHANNELS ) || ( value > D2A_FULLSCALE ) )
	{
		return FALSE;
	}
	
	
	// Blank out the least significant 2 bits to force the 12-bit code into a 10-bit code
	// This is done because it was discovered that some of our 10-bit parts were actually
	// outputting at 12-bit resolution.  We can't take the chance that qualification testing has been 
	// performed with a part outputting 12-bit resolution and then having later production parts
	// only outputting 10-bit resolution.
	value = value & (~0x0003);
	
	
	if( K_Resource_Wait( I2C2_Get_Resource_ID(), 0 ) != K_OK )
	{
		return FALSE;
	}
	
	success &= I2C2_Generate_START();
	success &= I2C2_Data_Transmit( I2C_WRITE_ADDR(d2a_i2c_addresses[channel].address) );
	success &= I2C2_Wait_ACK();
	success &= I2C2_Data_Transmit( d2a_i2c_addresses[channel].channel );
	success &= I2C2_Wait_ACK();
	
	// 0x20 -> PD0 = 0, PD1 = 0, ~CLR = 1, ~LDAC = 0
	success &= I2C2_Data_Transmit( 0x20 | (uint8)(value>>8) );
	success &= I2C2_Wait_ACK();
	success &= I2C2_Data_Transmit( (uint8)value );
	success &= I2C2_Wait_ACK();
	success &= I2C2_Generate_STOP();
	
	// we know we got the resource earlier in this function, so we can
	// safely ignore the return value.
	(void)K_Resource_Release( I2C2_Get_Resource_ID() );

	return success;
}


const char *D2A_Channel_NameString_Get( D2A_CHANNEL_t channel )
{
	const char *nameString = INVALID_STRING;

	if( channel < D2A_NUM_CHANNELS )
	{
		 nameString = D2A_CHANNEL_STRINGS[channel];
	}

	return nameString;
}


// ***************************************************
// * PRIVATE FUCTIONS
// ***************************************************
