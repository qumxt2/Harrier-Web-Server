//------------------------------------------------------------------------------ 
// File:		CountDigit.h        
//
// Purpose:     Support file for CountDigit.c
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

#ifndef _COUNTDIGIT_H_
#define _COUNTDIGIT_H_

#include "typedef.h"
#include "screensTask.h"

#define NO_DECIMAL_POINT                (0)
#define DECIMAL_POINT_ONE_DIGIT         (1)
#define DECIMAL_POINT_TWO_DIGIT         (2)
#define DECIMAL_POINT_THREE_DIGIT       (3)
#define DECIMAL_POINT_FOUR_DIGIT        (4)
#define DECIMAL_POINT_FIVE_DIGIT        (5)

// Structure / defines for editing counts / pins
typedef enum
{
    COUNT_DIGIT_SIGN = 0,
    COUNT_DIGIT_100000,
    COUNT_DIGIT_10000,
    COUNT_DIGIT_1000,
    COUNT_DIGIT_100,
    COUNT_DIGIT_10,
    COUNT_DIGIT_1, 

    NUMBER_OF_COUNT_DIGITS
} COUNT_DIGITS_t;

typedef enum
{
    DECIMAL_PLACE_NONE = 0,
    DECIMAL_PLACE_ONE,
    DECIMAL_PLACE_TWO,
    DECIMAL_PLACE_THREE,
    DECIMAL_PLACE_FOUR,
    DECIMAL_PLACE_FIVE,
    DECIMAL_PLACE_MAX = NUMBER_OF_COUNT_DIGITS, //lint !e488
} DECIMAL_PLACE_t;

typedef enum
{
    SIGN_DIGIT_NEG = 0,
    SIGN_DIGIT_POS,
} SIGN_DIGIT_t;

typedef struct
{
    INT8U DigitSelected;
    DECIMAL_PLACE_t DecimalPlaces;
    INT8U ActiveDigits;
    INT8U PositionX;
    INT8U PositionY;
    INT8U DigitValue[NUMBER_OF_COUNT_DIGITS];
    BOOLEAN FixedPoint;
    BOOLEAN SignDigit;
} COUNT_DIGIT_t;


// Function Prototypes
INT32U GetCountDigitValue (COUNT_DIGIT_t *CountDigit);
BOOLEAN LoadCountDigit (COUNT_DIGIT_t *CountDigit,
                        INT32U Value,
                        INT8U ActiveDigits,
						DECIMAL_PLACE_t decimalPlaces,
                        INT8U PositionX,
                        INT8U PositionY,
                        BOOLEAN FixedPoint, 
                        BOOLEAN SignDigit);
BOOLEAN ProcessCountDigitEvent (COUNT_DIGIT_t *CountDigit,
                                INPUT_EVENT_t InputEvent);
VOID DisplayCountDigit (COUNT_DIGIT_t *CountDigit);
VOID DrawCountDigitDigitBox (COUNT_DIGIT_t *CountDigit);
VOID DrawCountDigitBox (COUNT_DIGIT_t *CountDigit);
VOID DrawDigitBox (INT8U PositionX,
                   INT8U PositionY);
VOID DrawBox (INT8U PositionX,
              INT8U PositionY,
              INT8U Length);
VOID DrawBoxWithDecimal(INT8U PositionX,
	INT8U PositionY,
	INT8U Length);
VOID DrawDigitBoxAfterDecimal(INT8U PositionX,
	INT8U PositionY);

INT32U FixedPointToInteger(INT32U Val, DECIMAL_PLACE_t Decimals);
INT32U IntegerToFixedPoint(INT32U Val, DECIMAL_PLACE_t Decimals);

#endif // _COUNTDIGIT_H_
