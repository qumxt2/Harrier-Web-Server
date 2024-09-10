//------------------------------------------------------------------------------ 
//
// File:		Temperature.h        
//
// Purpose:     Support file for temperature.h
//
// Programmer:  William R. Ockert
//				OMNI Engineering Services
//				370 West Second Street
//				Suite 100
//				Winona, MN  55987
//				507.454.5293
//
// Revision:	1/26/2009			Initial    
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 

#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_

#include "typedef.h"						// Compiler specific type definitions

     

// Function definitions
void TEMPERATURE_Init(sint8 heaterOnTemperatureC, sint8 heaterOffTemperatureC);
sint16 TEMPERATURE_ReadDegreesC(void);
bool TEMPERATURE_MonitorHeat(void);

#endif // _TEMPERATURE_H_


