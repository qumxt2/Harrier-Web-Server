// i2c_channel2.c

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains object code for communicating via the second I2C channel on the
// PIC32MX795F512L device.  These functions constitute a basic hardware driver, specific
// to I2C channel 2 on the PIC32MX795F512L device.

// These functions were written with the intention of being used for communication to a
// real time clock chip.  The driver uses a lot of operating systems waits because timing
// was not a concern.  This driver is not suited for high speed or high data rate
// communication.


// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "I2C_Channel2.h"						// I2C channel 2 prototypes and constants

#include "p32mx795f512l.h"						// Processor specific header file
#include "rtos.h"
#include "oscillator.h"


// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************

// while loops below take 7 cycles per loop (87.5nS per while loop at 80MIPS)
// I2C bus is running at 400kHz (2.5uS per bit)
// so, one I2C bit-time is equivalent to about 30 loops.
// set the one-bit delay (time-out) count to 100 to be absolutely sure
// we give each function enough time to complete.
#define ONE_BIT_DELAY_COUNT						(100)

#define I2C_FSCK								(400000)  //400kHz

// ******************************************************************************************
// PRIVATE VARIABLES
// ******************************************************************************************
static uint8 mI2C2ResourceID = RTOS_INVALID_ID;

// ******************************************************************************************
// PUBLIC FUNCTIONS
// ******************************************************************************************

sint16 I2C2_Init( void )
{
	sint16 rVal = 0;
	sint32 PBClk;                               //Peripheral Bus Clock Speed (Hz)
	uint32 BRGval;

	// Make sure we aren't initalized already
	if( mI2C2ResourceID == RTOS_INVALID_ID )
	{
		// Silicon Errata Issue #1
		// The TRIS bit for the SDA pin is not correctly controlled by the I2C module during initialization
		// Before enabling the I2C module, set the LAT and TRIS bits to '0' for the SDA pin.
		// SDA for I2C2 port is on RA3.
		// Implementing this fix appears to actually make communications fail.
		// LATACLR = 1<<3;
		// TRISACLR = 1<<3;

		I2C2STAT = 0x00000000;					// Ensure reset values - contains all I2C2 status flags

		I2C2CONCLR = _I2C2CON_I2CSIDL_MASK;		// Continue operation in Device Idle Mode
		I2C2CONSET = _I2C2CON_SCLREL_MASK;		// Release SCL2 clock, used only in slave mode
		I2C2CONCLR = _I2C2CON_STRICT_MASK;		// Strict I2C Reserved Address Rule not enabled.
		I2C2CONCLR = _I2C2CON_A10M_MASK;		// I2C2ADD is a 7-bit slave address, opposed to 10-bit
		I2C2CONCLR = _I2C2CON_DISSLW_MASK;		// Slew rate control is enabled
		I2C2CONCLR = _I2C2CON_SMEN_MASK;		// SMBus input thresholds disabled
		I2C2CONCLR = _I2C2CON_GCEN_MASK;		// General call address disabled, used only in slave mode
		I2C2CONCLR = _I2C2CON_STREN_MASK;		// Clock stretching disabled, used only in slave mode

		//Calculate the correct value for the I2C Baud Rate Generator
		PBClk = PeripheralBusGetFcy();

		// Incorrect formula in datasheet:	I2CBRG = (PBCLK / (2*FSCK)) - 2
		// Results match calculation of:	I2CBRG = (PBCLK / (2*FSCK)) - 3 (for PBClk = SysClk/4)
		// Results match calculation of:	I2CBRG = (PBCLK / (2*FSCK)) - 5 (for PBClk = SysClk/2)
		// This information was verified by Microchip and they will reportedly be updating their
		// data sheets.
		BRGval = ((uint32)PBClk/(2*I2C_FSCK)) - 5;	//Equation taken from PIC32 I2C manual

		I2C2BRG = BRGval;

		I2C2ADD = 0x0000;		// Holds the slave address for the PIC24.  This address is used by
								// the PIC32 when in a slave mode.  If the PIC24 is not used as a
								// slave, the value should be set to 0x0000.


		I2C2MSK = 0;                                // Used by the PIC32 only when in a slave mode

		I2C2CONSET = _I2C2CON_ON_MASK;              // Enables the I2C2 module and configures the port pins
                                                    // as required by the module

		// Test the I2C bus and
		// Reserve a resource ID
		if( !I2C2_Generate_STOP() ||
			(mI2C2ResourceID = RtosResourceReserveID()) == RTOS_INVALID_ID )
		{
			// Either we failed to generate a STOP bit or
			// we didn't get a resource ID
			rVal = -1;
		}
	}

	return rVal;
}

uint8 I2C2_Get_Resource_ID( void )
{
	return mI2C2ResourceID;
}

bool I2C2_Generate_START( void )
{
	volatile uint16 delaycount = ONE_BIT_DELAY_COUNT;        // Start takes a bit longer...

	I2C2CONSET = _I2C2CON_SEN_MASK;					// Generate start condition
	while(I2C2CONbits.SEN && delaycount)
	{
		delaycount--;
	}

	return !I2C2CONbits.SEN;
}

bool I2C2_Generate_REPEATED_START( void )
{
	volatile uint16 delaycount = ONE_BIT_DELAY_COUNT;

	I2C2CONSET = _I2C2CON_RSEN_MASK;				// Generate repeated start condition
	while(I2C2CONbits.RSEN && delaycount)
	{
		delaycount--;
	}

	return !I2C2CONbits.RSEN;
}

bool I2C2_Generate_STOP( void )
{
	volatile uint16 delaycount = ONE_BIT_DELAY_COUNT;

	I2C2CONSET = _I2C2CON_PEN_MASK;					// Generate stop condition
	while(I2C2CONbits.PEN && delaycount)
	{
		delaycount--;
	}

	return !I2C2CONbits.PEN;
}

bool I2C2_Generate_ACK( void )
{
	volatile uint16 delaycount = ONE_BIT_DELAY_COUNT;

	I2C2CONCLR = _I2C2CON_ACKDT_MASK;				// Clear to generate ACK
	I2C2CONSET = _I2C2CON_ACKEN_MASK;				// Generate acknowledge sequence

	while(I2C2CONbits.ACKEN && delaycount)
	{
		delaycount--;
	}

	return !I2C2CONbits.ACKEN;
}

bool I2C2_Generate_NOACK( void )
{
	volatile uint16 delaycount = ONE_BIT_DELAY_COUNT;

	I2C2CONSET = _I2C2CON_ACKDT_MASK;				// Set to generate NACK
	I2C2CONSET = _I2C2CON_ACKEN_MASK;				// Generate acknowledge sequence

	while(I2C2CONbits.ACKEN && delaycount)
	{
		delaycount--;
	}

	return !I2C2CONbits.ACKEN;
}

bool I2C2_Wait_ACK( void )
{
	volatile uint16 delaycount = ONE_BIT_DELAY_COUNT;

	while(I2C2STATbits.TRSTAT && delaycount)		// Wait for ACK bit (9th clock cycle)
	{
		delaycount--;
	}

	return !I2C2STATbits.ACKSTAT;
}

bool I2C2_Data_Transmit( uint8 data )
{
	volatile uint16 delaycount = 8 * ONE_BIT_DELAY_COUNT;

	I2C2TRN = data;                                 // Load byte to be sent into transmit register

	while(I2C2STATbits.TBF && delaycount)			// Wait on data transfer
	{
		delaycount--;
	}

	return !I2C2STATbits.TBF;
}

bool I2C2_Data_Receive( uint8 *data )
{
	volatile uint16 delaycount = 8 * ONE_BIT_DELAY_COUNT;

	I2C2CONSET = _I2C2CON_SCLREL_MASK;              // Release clock line
	I2C2CONSET = _I2C2CON_RCEN_MASK;                // Start the receive sequence

	while(I2C2CONbits.RCEN  && delaycount)
	{
		delaycount--;
	}

	*data = (uint8)I2C2RCV;                         // Read data from the receive register

	return !I2C2CONbits.RCEN;
}
