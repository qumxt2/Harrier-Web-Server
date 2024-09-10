// maxim_MAX8647.h

// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains all of the function prototypes, constant declarations, and type
// definitions necessary for interfacing with the Maxim MAX8647 LED backlight controller. 

// The functions described below are 
// RTOS dependent.  In other words, calling these functions from outside of the RTOS 
// context will generate run time errors.  In addition, these functions must not be called 
// from within a locked portion of any RTOS task.  

#ifndef MAXIM_MAX8647_H
#define MAXIM_MAX8647_H

#include "typedef.h"						// Compiler specific type definitions


#define MAX8647_I2C_ADDRESS (0x9A)
#define MAX8647_LED1 (1<<5)
#define MAX8647_LED2 (2<<5)
#define MAX8647_LED3 (3<<5)
#define MAX8647_LED4 (4<<5)
#define MAX8647_LED5 (5<<5)
#define MAX8647_LED6 (6<<5)

#define MAX8647_C1 (0x01)
#define MAX8647_C2 (0x02)
#define MAX8647_C3 (0x04)
#define MAX8647_C4 (0x08)
#define MAX8647_C5 (0x10)
#define MAX8647_C6 (0x20)

#define MAX8647_MAXIMUM (31)

// Bitmask to indicate which channels will be used
void MAX8647_InitializeBacklight(uint8 p1, uint8 p2);

void MAX8647_SetBacklightChannel(uint8 channel, uint8 setting);

void MAX8647_SetBacklightOff(void);

void MAX8647_SetBacklightOn(void);

void MAX8647_SetBacklightPct(uint8 pct);

uint8 MAX8647_GetBacklightPct(void);

bool MAX8647_IsBacklightOn(void);

#endif /* MAXIM_MAX8647_H */
