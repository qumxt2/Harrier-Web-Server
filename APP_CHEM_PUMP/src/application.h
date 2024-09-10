// application.h

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all of the necessary information to initialize the application
// level development software.

#ifndef APPLICATION_H
#define APPLICATION_H
	
//**********************************************************************************************

#define APPLICATION_INIT_OK								(0x0)

#define BATTERY_ADC_CHANNEL                             (2)
#define MOTOR_ADC_CHANNEL                               (3)
#define THERMISTOR_ADC_CHANNEL                          (6)
#define BATTERY_ADC_MULTIPLIER                          (732)
#define BATTERY_ADC_DIVISOR                             (100)
#define MV_PER_VOLT                                     (1000)
#define BATTERY_OFFSET_MV                               (100)
#define MOTOR_ADC_MULTIPLIER                            (625) //625/512 = 1.2207 .61035mV per count 2.5V ref 12 bit Current sense chip is 0-5 volts output with a /2 resistor network 2*.61035 = 1.2207
#define MOTOR_ADC_DIVISOR_BIT_SHIFT                     (9)    //#>>9 is the same as dividing by 512                               

sint16 ApplicationInit(void);

#endif 	// APPLICATION_H
