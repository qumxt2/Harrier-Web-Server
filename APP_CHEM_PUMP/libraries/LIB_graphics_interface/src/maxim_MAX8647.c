// maxim_MAX8647.c

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains the object code used to initialize and interface with the
// Maxim MAX8647 LED backlight controller. The functions include setting and 
// retrieving the power settings.

#include "maxim_MAX8647.h"
#include "rtos.h"
#include "i2c_channel2.h"

#define WAIT_FOREVER (0x0000)

static uint8 setpoint_pct;
static uint8 setpoint_setting;
static bool current_pct;
static uint8 current_setting;
static uint8 active_channels;

static uint8 gI2C2ResourceID;

void MAX8647_InitializeBacklight( uint8 p1, uint8 p2 ) 
{
    gI2C2ResourceID = I2C2_Get_Resource_ID();
	active_channels = p1;
}

void MAX8647_SetBacklightChannel( uint8 channel, uint8 setting ) 
{
	bool ackstat = FALSE;
	
	if( K_Resource_Wait(gI2C2ResourceID, WAIT_FOREVER) == K_OK )
	{
		(void) I2C2_Generate_START();
		(void) I2C2_Data_Transmit( (MAX8647_I2C_ADDRESS << 0) );

		ackstat |= I2C2_Wait_ACK();
		if( !ackstat )
		{
			(void) K_Resource_Release( gI2C2ResourceID );
			return;
		}

		(void) I2C2_Data_Transmit( channel | (setting & 31) );

		ackstat |= I2C2_Wait_ACK();
		if( !ackstat )
		{
			(void) K_Resource_Release( gI2C2ResourceID );
			return;
		}
		(void) I2C2_Generate_STOP();
		(void) K_Resource_Release( gI2C2ResourceID );
		

	}
}

void MAX8647_SetBacklightOff( void ) 
{
	MAX8647_SetBacklightChannel( MAX8647_LED1, 0 );
	MAX8647_SetBacklightChannel( MAX8647_LED2, 0 );
	MAX8647_SetBacklightChannel( MAX8647_LED3, 0 );
	MAX8647_SetBacklightChannel( MAX8647_LED4, 0 );
	MAX8647_SetBacklightChannel( MAX8647_LED5, 0 );
	MAX8647_SetBacklightChannel( MAX8647_LED6, 0 );

	current_pct = FALSE;
}

void MAX8647_SetBacklightOn( void ) 
{
	if( active_channels & MAX8647_C1 )
	{
		MAX8647_SetBacklightChannel( MAX8647_LED1, current_setting );
	}
	if( active_channels & MAX8647_C2 )
	{
		MAX8647_SetBacklightChannel( MAX8647_LED2, current_setting );
	}
	if( active_channels & MAX8647_C3 )
	{
		MAX8647_SetBacklightChannel( MAX8647_LED3, current_setting );
	}
	if( active_channels & MAX8647_C4 )
	{
		MAX8647_SetBacklightChannel( MAX8647_LED4, current_setting );
	}
	if( active_channels & MAX8647_C5 )
	{
		MAX8647_SetBacklightChannel( MAX8647_LED5, current_setting );
	}
	if( active_channels & MAX8647_C6 )
	{
		MAX8647_SetBacklightChannel( MAX8647_LED6, current_setting );
	}

	current_pct = TRUE;
}

void MAX8647_SetBacklightPct( uint8 pct ) 
{
	if( pct > 100 )
	{
		pct = 100;
	}

	setpoint_pct = pct;
	setpoint_setting = (pct * MAX8647_MAXIMUM) / 100;
	current_setting = setpoint_setting;

	MAX8647_SetBacklightOn();
}

uint8 MAX8647_GetBacklightPct( void ) 
{
	return setpoint_pct;
}

bool MAX8647_IsBacklightOn( void ) 
{
	return current_pct;
}

