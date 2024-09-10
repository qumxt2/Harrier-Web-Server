//! \file	in_voltage.c
//! \brief Module for ADCM Analog Voltage Inputs
//!
//! Copyright 2012 
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This module provides access to the Analog Voltage Inputs
//!    on the ADCM component (Advanced Display Module)

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

// Include basic platform types
#include "typedef.h"
#include "p32mx795f512l.h"

// Include IO definitions
#include "io_typedef.h"
#include "io_pin.h"
#include "spi.h"
#include "in_voltage.h"
#include "stdio.h"
#include "debug_app.h"
#include "CountDigit.h"
#include "debug.h"

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

typedef enum
{
	ADC_MODE_DIFF = 0,	
	ADC_MODE_SGL
} ADC_MODE_t;

// *****************************************************************************
// * PRIVATE (STATIC) VARIABLES
// *****************************************************************************

static SPIContextHandle s_ADC_Handle = 0;
static uint8 spiBuffer[3];

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

static void ADC_setup_command( ADC_MODE_t mode, ADC_CHANNEL_t channel, uint8* command_buffer );
static void ADC_ChipEnable (void);
static void ADC_ChipDisable (void);

// *****************************************************************************
// * MACROS & CONSTANTS
// *****************************************************************************

//Macros for the MCP3208 ADC
#define SPI_CLOCK_RATE		1000000
#define SENSOR_DETECT		(100)						// ADC Count above which sensor considered connected

// *****************************************************************************
// * PUBLIC FUCTIONS
// *****************************************************************************

IOrtn_uint16_t IN_A2D_Get( ADC_CHANNEL_t ch )
{
	IOrtn_uint16_t rtnval;

	rtnval.u16 = 0;
	rtnval.error = IOError_UNDEFINED;
	
	if (ch == ADC_CHANNEL_UNDEFINED)
	{
		return rtnval;
	}

	if (s_ADC_Handle == 0)
	{
		if ((uint8)SPI_Init() != (uint8) SPIError_Ok)
		{
			rtnval.error = IOError_UNDEFINED;
		}
		if ((uint8)SPI_GetContextHandle(&s_ADC_Handle, SPI_PORT_2A, SPI_CLOCK_RATE,
										SPI_MODE_CKP_0_CKE_0, ADC_ChipEnable, ADC_ChipDisable) !=  (uint8) SPIError_Ok)
		{
			rtnval.error = IOError_UNDEFINED;
		}	
	}

	ADC_setup_command(ADC_MODE_SGL, ch, spiBuffer);

	if(SPI_TxRx(s_ADC_Handle, spiBuffer, 3) != SPIError_Ok)
	{
		rtnval.error = IOError_UNDEFINED;
	}
	else
	{
		rtnval.error = IOError_Ok;
	}

	rtnval.u16 = (uint16)((uint16)(0x0F00&spiBuffer[1]<<8) + (uint16)spiBuffer[2]);

    return rtnval;
}

// *****************************************************************************
// * PRIVATE FUCTIONS
// *****************************************************************************

static void ADC_ChipEnable (void)
{
	// Set RA6 to 0 (enabled), wait 100nS.
	LATACLR = 1<<6;
}

static void ADC_ChipDisable (void)
{
	// Set RA6 to 1 (disabled), wait 100nS.
	LATASET = 1<<6;
}

// Setup command to send to the MCP3208 ADC
void ADC_setup_command(ADC_MODE_t mode, ADC_CHANNEL_t channel, uint8* command_buffer)
{
	uint8 start_bit = 0x04;
	uint8 mode_mask = ((uint8)mode<<1);
	uint8 byte1_channel_mask;
	uint8 byte2_channel_mask;

	// Clear command bytes
	command_buffer[0] = 0x00;	
	command_buffer[1] = 0x00;

	// Set up command buffer according to MSP3208 protocol
	byte1_channel_mask = 0x01&(((uint8)channel)>>2);
	byte2_channel_mask = 0xC0&(((uint8)channel)<<6);

	command_buffer[0] |= start_bit;
	command_buffer[0] |= mode_mask;
	command_buffer[0] |= byte1_channel_mask;
	command_buffer[1] |= byte2_channel_mask;
}

IOrtn_mV_s16d16_t IN_Voltage_Pressure_Get_Diff( uint8 Channel )
{
    IOrtn_mV_s16d16_t rtnval;
    ADC_CHANNEL_t analogDetect = ADC_CHANNEL_CH1;			// Used to detect if sensor is connected

	IOrtn_uint16_t rawcount;
	IOrtn_uint16_t detectcount;

	rtnval.mV_s16d16 = 0;
	rtnval.error = IOError_UNDEFINED;

	// Read the actual analog input
	if( Channel != ADC_CHANNEL_UNDEFINED )
	{
        rawcount = IN_A2D_Get( Channel );

        //check if the raw count was maxed out.  If so, this would indicate
		// that we can't trust the result.  Over-pressure conditions can max out the count, so clamp it
		if( rawcount.u16 >= (0x0FFF) )
		{
			rawcount.u16 = (0x0FFF);
		}

        if( rawcount.error != IOError_Ok )
        {
            rtnval.error = rawcount.error;
            return rtnval;
        }

        // Full scale reading 4095
        // V1 = (V+ - V-) * (5 + 5*(43.2/10)) * (10.5/(10+10.5))
        // Vdiff = V1 / 13.6192
        // V1 = A2D * 2.5 / 4095
        //
        // Vdiff = A2D / 22,308.2496
        // Vdiff_mV_u16d16 = Vdiff * 1000 * 2^16
        // Vdiff_mV_u16d16 = A2D * 2938

        rtnval.mV_s16d16 = rawcount.u16 * 2938;
        rtnval.error = IOError_Ok;
    }
    else
    {
        rtnval.mV_s16d16 = 0;
		rtnval.error = IOError_InvalidPinConfiguration;
    }

    // Check sensor detect to confirm that we are reading valid data
    // if detect line shows disconnected, still return value, but set error to IOError_UNDEFINED
	if( analogDetect != ADC_CHANNEL_UNDEFINED )
	{
        detectcount = IN_A2D_Get( analogDetect );

        if( detectcount.error != IOError_Ok )
        {
            detectcount.u16 = 0;
        }

        if( detectcount.u16 < SENSOR_DETECT )
        {
            rtnval.error = IOError_UNDEFINED;
        }
    }

	return rtnval;
}

IOrtn_mV_s16d16_t IN_Voltage_Pressure_Get_1to5V( uint8 Channel )
{
    IOrtn_mV_s16d16_t rtnval;
	IOrtn_uint16_t rawcount;

	rtnval.mV_s16d16 = 0;
	rtnval.error = IOError_UNDEFINED;

	// Read the actual analog input
	if( Channel != ADC_CHANNEL_UNDEFINED )
	{
        rawcount = IN_A2D_Get( Channel );

        //check if the raw count was maxed out.  If so, this would indicate
		// that we can't trust the result.  Over-pressure conditions can max out the count, so clamp it
		if( rawcount.u16 >= (0x0FFF) )
		{
			rawcount.u16 = (0x0FFF);
		}

        // Check the 0-10 V Input Voltage
        rtnval.mV_s16d16 = rawcount.u16 * 83657; 
        
        rtnval.error = IOError_Ok;
    }
    else
    {
        rtnval.mV_s16d16 = 0;
		rtnval.error = IOError_InvalidPinConfiguration;
    }
    return rtnval;
}

IOrtn_mV_s16d16_t IN_Voltage_Pressure_Get_4to20mA( uint8 Channel )
{
    IOrtn_mV_s16d16_t rtnval;
	IOrtn_uint16_t rawcount;

	rtnval.mV_s16d16 = 0;
	rtnval.error = IOError_UNDEFINED;

	// Read the actual analog input
	if( Channel != ADC_CHANNEL_UNDEFINED )
	{
        rawcount = IN_A2D_Get( Channel );
        
        DEBUG_PRINT_STRING(DBUG_ADC, "Count:");
        DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_ADC, rawcount.u16);
        DEBUG_PRINT_STRING(DBUG_ADC, "\r\n");

        //check if the raw count was maxed out.  If so, this would indicate
		// that we can't trust the result.  Over-pressure conditions can max out the count, so clamp it
		if( rawcount.u16 >= (0x0FFF) )
		{
			rawcount.u16 = (0x0FFF);
            DEBUG_PRINT_STRING(DBUG_ADC, "Count too high\r\n");
		}        

        if( rawcount.error != IOError_Ok )
        {
            rtnval.error = rawcount.error;
            DEBUG_PRINT_STRING(DBUG_ADC, "Count error\r\n");
            return rtnval;
        }

        // Check the 0-10 V Input Voltage
        // mV = counts * (2500/4095) * 2^16
        rtnval.mV_s16d16 = rawcount.u16 * 40010;
        
        if (g_DebugMask & DBUG_MV)
        {
            printf("Counts: %d\r\n", rawcount.u16);            
        }
        
        DEBUG_PRINT_STRING(DBUG_MV, "mV:");
        DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_MV, FixedPointToInteger(rtnval.mV_s16d16, NO_DECIMAL_POINT));
        DEBUG_PRINT_STRING(DBUG_MV, "\r\n");
        
        rtnval.error = IOError_Ok;
    }
    else
    {
        rtnval.mV_s16d16 = 0;
		rtnval.error = IOError_InvalidPinConfiguration;
    }
    return rtnval;
}


