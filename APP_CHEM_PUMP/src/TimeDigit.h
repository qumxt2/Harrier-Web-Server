//------------------------------------------------------------------------------ 
// File:		TimeDigit.h        
//
// Purpose:     Support file for TimeDigit.c
//
// Programmer:  William R. Ockert
//				OMNI Engineering Services
//				370 West Second Street
//				Suite 100
//				Winona, MN  55987
//				507.454.5293
//
// Revision:	3/10/2009			Initial    
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 

#ifndef _TIMEDIGIT_H_
#define _TIMEDIGIT_H_

// Defines
#define MAX_TIME                ((99UL * 3600UL) + (59UL * 60UL) + 59UL)

typedef enum 
{
    TIME_DIGIT_SL = 0,
    TIME_DIGIT_SH,
    TIME_DIGIT_ML,
    TIME_DIGIT_MH,
    TIME_DIGIT_HL,
    TIME_DIGIT_HH,

    NUMBER_OF_TIME_DIGITS
} TIME_DIGITS_t;


typedef struct
{
    INT8U DigitSelected;
    INT8U ActiveDigits;
    INT8U PositionX;
    INT8U PositionY;
    INT8U DigitValue[NUMBER_OF_TIME_DIGITS];
} TIME_DIGIT_t;


// Function Prototypes
INT32U GetTimeDigitValue (TIME_DIGIT_t *TimeDigit);
BOOLEAN LoadTimeDigit (TIME_DIGIT_t *TimeDigit,
                       INT32U Time,
                       INT8U ActiveDigits,
                       INT8U PositionX,
                       INT8U PositionY);
BOOLEAN ProcessTimeDigitEvent (TIME_DIGIT_t *TimeDigit,
                                INPUT_EVENT_t InputEvent);

VOID DisplayTimeDigit (TIME_DIGIT_t *TimeDigit);

VOID DrawTimeDigitDigitBox (TIME_DIGIT_t *TimeDigit);
VOID DrawTimeDigitBox (TIME_DIGIT_t *TimeDigit);

BOOLEAN SecondsToHMS (INT32U SecondsIn,
                      INT8U *Hours,
                      INT8U *Minutes,
                      INT8U *Seconds);

#endif  // _TIMEDIGIT_H_
