// AlarmScreenMotorCurrent.h

// Copyright 2018
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This header file contains all prototypes and constants necessary for the Alarm Pressure Screen

#ifndef ALARMSCREENMOTORCURRENT_H
#define ALARMSCREENMOTORCURRENT_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// CONSTANTS
// ******************************************************************************************
typedef enum{
    MTR_PROTECTION_OFF = 0,
    MTR_PROTECTION_ON,
    NUMBER_OF_MTR_PROTECTION_ITEMS,
}MTR_PROTECTION_t;

typedef enum{
    MTR_VOLTAGE_12VDC = 0,
    MTR_VOLTAGE_24VDC,
    MTR_VOLTAGE_120VAC,
    MTR_VOLTAGE_230VAC,
    NUMBER_OF_MTR_VOLTAGE_CHOICES,
}MTR_VOLTAGE_t;

typedef enum{
    MTR_SELECTION_0,
    MTR_SELECTION_1,
    MTR_SELECTION_2,
    MTR_SELECTION_3,
    MTR_SELECTION_4,
    NUMBER_OF_MTR_PMP_SELECTION_CHOICES,
}MTR_SELECTION_t;

#define NUMBER_OF_12VDC_MOTORS 5
#define NUMBER_OF_24VDC_MOTORS 2
#define NUMBER_OF_120VAC_MOTORS 4
#define NUMBER_OF_230VAC_MOTORS 2
// ******************************************************************************************
// PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************

INPUT_MODE_t AlarmScreenMotorCurrent(INPUT_EVENT_t InputEvent);
uint32 PackageMotorProtectionSettings(void);
//*******************************************************************************************


#endif
