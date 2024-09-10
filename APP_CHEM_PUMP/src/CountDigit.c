//------------------------------------------------------------------------------ 
// File:		CountDigit.c       
//
// Purpose:     Routines to display and edit 2 and 4 digit values
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
#include "CountDigit.h"
#include "gdisp.h"

#define DECIMAL_POINT_OFFSET		(4u)
#define FONT_WIDTH                  (7)
#define FONT_HEIGHT                 (12)

//------------------------------------------------------------------------------
// Private functions
//------------------------------------------------------------------------------

static INT32U pow_u32( INT32U base, INT32U exp );

//------------------------------------------------------------------------------ 
//  FUNCTION:       GetCountDigitValue ()
//
//  PARAMETERS:     COUNT_DIGIT_t *CountDigit  // Count digit to value 
//
//  DESCRIPTION:    Returns value associated with digits in pointed to count digit    
//
//  RETURNS:        INT16U Value
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
INT32U GetCountDigitValue (COUNT_DIGIT_t *CountDigit)
{
    INT32U Value = 0;
    INT32 sValue = 0;

    if (CountDigit->ActiveDigits >= 6)
    {
        Value = CountDigit->DigitValue[COUNT_DIGIT_100000];
    }
    
    if (CountDigit->ActiveDigits >= 5)
    {
        Value = (Value * 10) + CountDigit->DigitValue[COUNT_DIGIT_10000];
    }

    if (CountDigit->ActiveDigits >= 4)
    {
        Value = (Value * 10) + CountDigit->DigitValue[COUNT_DIGIT_1000];
    }
    
    if (CountDigit->ActiveDigits >= 3)
    {
        Value = (Value * 10) + CountDigit->DigitValue[COUNT_DIGIT_100];
    }

    if (CountDigit->ActiveDigits >= 2)
    {
        Value = (Value * 10) + CountDigit->DigitValue[COUNT_DIGIT_10];
    }
    
    if (CountDigit->ActiveDigits >= 1)
    {
        Value = (Value * 10) + CountDigit->DigitValue[COUNT_DIGIT_1];
    }

    if(CountDigit->FixedPoint)
    {
        Value = IntegerToFixedPoint(Value, CountDigit->DecimalPlaces);
    }
    
    // If sign bit enabled, convert to signed & make the UINT32 return value represent a signed #
    if (CountDigit->SignDigit)
    {
        if (CountDigit->DigitValue[COUNT_DIGIT_SIGN] == SIGN_DIGIT_NEG)
        {
            sValue = (INT32)Value;
            sValue *= -1;
            Value = (INT32U)sValue;
        }
    }

    return (Value);
}

//------------------------------------------------------------------------------ 
//  FUNCTION:       LoadCountDigit ()
//
//  PARAMETERS:     COUNT_DIGIT_t *CountDigit  // Count digit load
//                  INT16U Value               // Value to load            
//                  INT8U ActiveDigits         // Number of active digits
//
//  DESCRIPTION:    Load Value into the pointed to count digit    
//
//  RETURNS:        BOOLEAN
//                      TRUE --> Value was in range and was loaded
//                      FALSE --> Value was out of range and not loaded
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
BOOLEAN LoadCountDigit (COUNT_DIGIT_t *CountDigit,
                        INT32U Value,
                        INT8U ActiveDigits,
						DECIMAL_PLACE_t decimalPlaces,
                        INT8U PositionX,
                        INT8U PositionY, 
                        BOOLEAN FixedPoint,
                        BOOLEAN SignDigit)
{
    INT8U i;
    INT32U MaxValue = 0;
    bool rtnVal = TRUE;
    INT32 sValue = (sint32)Value;

    CountDigit->ActiveDigits = ActiveDigits;
    CountDigit->PositionX = PositionX;
    CountDigit->PositionY = PositionY;
    CountDigit->FixedPoint = FixedPoint;
    CountDigit->SignDigit = SignDigit;
    
    // If sign digit enabled, set the +/- digit
    if (CountDigit->SignDigit)
    {
        // If we have the sign digit enabled, we are reserving the MSB as a sign flag, ie: 0x1000 0001 = -1
        if (sValue < 0)
        {
            CountDigit->DigitValue[COUNT_DIGIT_SIGN] = SIGN_DIGIT_NEG;
            
            // We are using a UINT32 to represent a signed #, but first we need to convert the # to its unsigned value in order to display it
            sValue *= -1;
        }
        else
        {
            CountDigit->DigitValue[COUNT_DIGIT_SIGN] = SIGN_DIGIT_POS;
        }
        
        Value = sValue;
    }
    
    if(CountDigit->FixedPoint)
    {
        Value = FixedPointToInteger(Value, decimalPlaces);
    }

    // Range check the input
    for (i=0; i<CountDigit->ActiveDigits; i++)
    {
        MaxValue = (MaxValue * 10) + 9;
    }

    if (Value > MaxValue)
    {
        Value = MaxValue;
        rtnVal = FALSE;
    }

    // Load the values
    CountDigit->DigitValue[COUNT_DIGIT_1] = Value % 10;
    Value = Value / 10;
    CountDigit->DigitValue[COUNT_DIGIT_10] = Value % 10;
    Value = Value / 10;
    CountDigit->DigitValue[COUNT_DIGIT_100] = Value % 10;
    Value = Value / 10;
    CountDigit->DigitValue[COUNT_DIGIT_1000] = Value % 10;
    Value = Value / 10;
    CountDigit->DigitValue[COUNT_DIGIT_10000] = Value % 10;
    Value = Value / 10;
    CountDigit->DigitValue[COUNT_DIGIT_100000] = Value % 10;

	// Load the decimal point flag
    if (decimalPlaces > CountDigit->ActiveDigits)
    {
        CountDigit->DecimalPlaces = CountDigit->ActiveDigits;
    }
    else if (decimalPlaces > DECIMAL_PLACE_MAX)
    {
        CountDigit->DecimalPlaces = DECIMAL_PLACE_MAX;
    }
    else
    {
        CountDigit->DecimalPlaces = decimalPlaces;
    }
	return (rtnVal);
}
     
//------------------------------------------------------------------------------ 
//  FUNCTION:       ProcessCountDigitEvent ()
//
//  PARAMETERS:     COUNT_DIGIT_t *CountDigit  // Count digit operate on
//                  INPUT_EVENT_t InputEvent   // Keypad event to process
//
//  DESCRIPTION:    Uses passed keypad event to update pointed to count digit.    
//
//  RETURNS:        BOOLEAN
//                      TRUE --> Event was processed successfully
//                      FALSE ->> Event could not be processed        
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
BOOLEAN ProcessCountDigitEvent (COUNT_DIGIT_t *CountDigit,
                                INPUT_EVENT_t InputEvent)
{
    BOOLEAN ReturnValue = TRUE;
    
    // Process based on event
    switch (InputEvent)
    {
        case INPUT_EVENT_UP_ARROW:
            // This toggles the +/- sign digit if it's enabled
            if(CountDigit->DigitSelected < (NUMBER_OF_COUNT_DIGITS - CountDigit->ActiveDigits) && CountDigit->SignDigit)
            {
                CountDigit->DigitValue[COUNT_DIGIT_SIGN] = !CountDigit->DigitValue[COUNT_DIGIT_SIGN];
            }  
            // This increases the currently selected digit
            else if (CountDigit->DigitValue[CountDigit->DigitSelected] < 9)
            {
                CountDigit->DigitValue[CountDigit->DigitSelected]++;
            }
            else
            {
                CountDigit->DigitValue[CountDigit->DigitSelected] = 0;
            }

            break;

        case INPUT_EVENT_DOWN_ARROW:
            // This toggles the +/- sign digit if it's enabled
            if(CountDigit->DigitSelected < (NUMBER_OF_COUNT_DIGITS - CountDigit->ActiveDigits) && CountDigit->SignDigit)
            {
                CountDigit->DigitValue[COUNT_DIGIT_SIGN] = !CountDigit->DigitValue[COUNT_DIGIT_SIGN];
            }             
            // This decreases the currently selected digit
            else if (CountDigit->DigitValue[CountDigit->DigitSelected] > 0)
            {
                CountDigit->DigitValue[CountDigit->DigitSelected]--;
            }
            else
            {
                CountDigit->DigitValue[CountDigit->DigitSelected] = 9;
            }

            break;

        case INPUT_EVENT_RIGHT_ARROW:
            // Changes selected digit
            if (CountDigit->DigitSelected < (NUMBER_OF_COUNT_DIGITS - 1) )
            {
                CountDigit->DigitSelected++;
            }
            else
            {
                CountDigit->DigitSelected = (NUMBER_OF_COUNT_DIGITS - CountDigit->ActiveDigits - (INT8U)CountDigit->SignDigit);
            }
            
            break;

        case INPUT_EVENT_LEFT_ARROW:
            // Change Selected digit

            if (CountDigit->DigitSelected > (NUMBER_OF_COUNT_DIGITS - CountDigit->ActiveDigits - (INT8U)CountDigit->SignDigit))
            {
                CountDigit->DigitSelected--;
            }
            else
            {
                CountDigit->DigitSelected = (NUMBER_OF_COUNT_DIGITS - 1);
            }

            break;

        default:
            ReturnValue = FALSE;
            
            break;
    }

    return (ReturnValue);
}

//------------------------------------------------------------------------------ 
//  FUNCTION:       DisplayCountDigit ()
//
//  PARAMETERS:     COUNT_DIGIT_t *CountDigit   // Count digit to value 
//                  INT8U PositionX             // X char position to place value
//                  INT8U PositionY             // Y char position to place value
//
//  DESCRIPTION:    Displays pointed to count digit at x,y char coordinates   
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DisplayCountDigit (COUNT_DIGIT_t *CountDigit)
{
    INT8U i;
    INT8U DecimalPoint = CountDigit->ActiveDigits - CountDigit->DecimalPlaces;
    INT8U activePlusSignDigits= CountDigit->SignDigit? CountDigit->ActiveDigits + 1 : CountDigit->ActiveDigits;
        
    if (CountDigit->PositionY == 0)
    {
        gsetpos(CountDigit->PositionX*FONT_WIDTH, FONT_HEIGHT);
    }
    else
    {
        gsetcpos(CountDigit->PositionX, CountDigit->PositionY);
    }

    for (i=0; i<activePlusSignDigits; i++)
    {   
        if ((CountDigit->DecimalPlaces > DECIMAL_PLACE_NONE) && (i == DecimalPoint))
        {
            gputch('.');
        }
        
        if ((CountDigit->SignDigit) && (i == 0))
        {
            if (CountDigit->DigitValue[COUNT_DIGIT_SIGN] >= SIGN_DIGIT_POS)
            {
                gputch('+');
                // The + sign is ~2 pixels less wide than other characters so force our cursor to the right position
                gsetpos(CountDigit->PositionX*FONT_WIDTH + 7, CountDigit->PositionY*FONT_HEIGHT + 11);
            }
            else
            {
                gputch('-');
            }
        }
        else
        {
            // No sign digit enabled, just populate the digits                                                                                                           
            if(CountDigit->SignDigit)
            {
                gputch(CountDigit->DigitValue[NUMBER_OF_COUNT_DIGITS - CountDigit->ActiveDigits + (i-1)] + '0');
            }
            else
            {
                // Populate the remaining digits, offset 1 to the right of the sign digit
                gputch(CountDigit->DigitValue[NUMBER_OF_COUNT_DIGITS - CountDigit->ActiveDigits + (i)] + '0');
            }
        }
    }
}


//------------------------------------------------------------------------------ 
//  FUNCTION:       DrawCountDigitDigitBox ()
//
//  PARAMETERS:     COUNT_DIGIT_t *CountDigit   // Count digit 
//
//  DESCRIPTION:    Draws box around active digit in pointed to count digit   
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DrawCountDigitDigitBox (COUNT_DIGIT_t *CountDigit)
{
    INT8U i;
    INT8U DigitIndex;
    INT8U DecimalPoint = CountDigit->ActiveDigits - CountDigit->DecimalPlaces;
    INT8U activePlusSignDigits = CountDigit->SignDigit? CountDigit->ActiveDigits + 1: CountDigit->ActiveDigits;

    for(i=0; i<activePlusSignDigits; i++)
    {
        DigitIndex = CountDigit->SignDigit? (NUMBER_OF_COUNT_DIGITS - CountDigit->ActiveDigits - 1) : (NUMBER_OF_COUNT_DIGITS - CountDigit->ActiveDigits);

        if (CountDigit->DigitSelected == (DigitIndex + i))
        {
            if ((CountDigit->DecimalPlaces > DECIMAL_PLACE_NONE) && (i >= DecimalPoint))
            {
                DrawDigitBoxAfterDecimal(CountDigit->PositionX + i, CountDigit->PositionY);
            }
            else
            {
                DrawDigitBox(CountDigit->PositionX + i, CountDigit->PositionY);
            }
        }
    }
}


//------------------------------------------------------------------------------ 
//  FUNCTION:       DrawCountDigitBox ()
//
//  PARAMETERS:     COUNT_DIGIT_t *CountDigit   // Count digit 
//
//  DESCRIPTION:    Draws box around the pointed to count digit   
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DrawCountDigitBox (COUNT_DIGIT_t *CountDigit)
{
    INT8U length = CountDigit->SignDigit? (CountDigit->ActiveDigits + 1) : CountDigit->ActiveDigits;
    
    // Currently only support decimal points for 4 and 5 digit values
    if ((CountDigit->DecimalPlaces > DECIMAL_PLACE_NONE) && (CountDigit->ActiveDigits >= 2))
    {
        DrawBoxWithDecimal(CountDigit->PositionX, CountDigit->PositionY, length);
    }
    else
    {
        DrawBox(CountDigit->PositionX, CountDigit->PositionY, length);
    }
}

//------------------------------------------------------------------------------ 
//  FUNCTION:       DrawDigitBox ()
//
//  PARAMETERS:     INT8U PositionX             // X char position to place value
//                  INT8U PositionY             // Y char position to place value
//
//  DESCRIPTION:    Draws box around the pointed to digit   
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DrawDigitBox (INT8U PositionX,
                   INT8U PositionY)
{
    sint8 offsetX = PositionX > 0? -2:0;
    sint8 offsetY = PositionY > 0? -2:0;    

    grectangle ((ggetfw () * PositionX) + offsetX,
                (ggetfh () * PositionY) + offsetY,
                (ggetfw () * (PositionX + 1)),
                (ggetfh () * PositionY) + offsetY + 13);
}

//------------------------------------------------------------------------------ 
//  FUNCTION:       DrawDigitBoxAfterDecimal ()
//
//  PARAMETERS:     INT8U PositionX             // X char position to place value
//                  INT8U PositionY             // Y char position to place value
//
//  DESCRIPTION:    Draws box around the pointed to digit   
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DrawDigitBoxAfterDecimal(INT8U PositionX,
	INT8U PositionY)
{
    sint8 offsetX = PositionX > 0? -2:0;
    sint8 offsetY = PositionY > 0? -2:0;
    
	grectangle((ggetfw() * PositionX) + offsetX + DECIMAL_POINT_OFFSET,
		(ggetfh() * PositionY) + offsetY,
		(ggetfw() * (PositionX + 1)) + DECIMAL_POINT_OFFSET,
		(ggetfh() * PositionY) + offsetX + 13);   
}


//------------------------------------------------------------------------------ 
//  FUNCTION:       DrawBox ()
//
//  PARAMETERS:     INT8U PositionX             // X char position to place value
//                  INT8U PositionY             // Y char position to place value
//                  INT8U Length                // Length of area to surround
//
//  DESCRIPTION:    Draws box around the pointed to area   
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DrawBox (INT8U PositionX,
              INT8U PositionY,
              INT8U Length)
{   
    sint8 offsetX = PositionX > 0? -2:0;
    sint8 offsetY = PositionY > 0? -2:0;
    
    grectangle ((ggetfw () * PositionX) + offsetX,
                (ggetfh () * PositionY) + offsetY,
                (ggetfw () * (PositionX + Length)) + offsetX + 3,
                (ggetfh () * PositionY) + offsetY + 13);    
}

//------------------------------------------------------------------------------ 
//  FUNCTION:       DrawBoxWithDecimal ()
//
//  PARAMETERS:     INT8U PositionX             // X char position to place value
//                  INT8U PositionY             // Y char position to place value
//                  INT8U Length                // Length of area to surround
//
//  DESCRIPTION:    Draws box around the pointed to area   
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DrawBoxWithDecimal(INT8U PositionX,
	INT8U PositionY,
	INT8U Length)
{
    sint8 offsetX = PositionX > 0? -2:0;
    sint8 offsetY = PositionY > 0? -2:0;
    
	grectangle((ggetfw() * PositionX) + offsetX,
		(ggetfh() * PositionY) + offsetY,
		(ggetfw() * (PositionX + Length)) + offsetX + 3 + DECIMAL_POINT_OFFSET,
		(ggetfh() * PositionY) + offsetY + 13);   
}

static INT32U pow_u32( INT32U base, INT32U exp )
{
    INT32U result = 1;

    while ( exp )
    {
        if ( exp & 1 )
        {
            result *= base;
        }

        exp >>= 1;
        base *= base;
    }

    return result;
}

INT32U FixedPointToInteger(INT32U Val, DECIMAL_PLACE_t Decimals)
{
    INT32U iPart;
    INT32U fPart;
    INT32U fBits;
    INT32U multiple;

    multiple = pow_u32( 10, Decimals );
    iPart    = multiple * (Val >> 16);
    multiple = 0xffffffff / multiple;
    fBits    = (Val << (32 - 16));

    if ( (0xffffffff - multiple / 2) >= fBits )
    { // check for overflow before adding 0.5 multiple for proper rounding
        fBits += multiple / 2;
    }

    fPart = fBits / multiple;
    Val =  fPart + iPart;
    return Val;
}

INT32U IntegerToFixedPoint(INT32U Val, DECIMAL_PLACE_t Decimals)
{
    INT32U iPart;
    INT32U fPart;
    INT32U iBits;
    INT32U fBits;
    INT32U multiple;
    INT32U TempValue;

    TempValue = Val;
    multiple = pow_u32( 10, Decimals );
    iPart = TempValue / multiple;
    fPart = TempValue - iPart * multiple;
    iBits = iPart << 16;

    while ( !((multiple >> 31) || (fPart >> 31)) )
    {
        multiple <<= 1;
        fPart <<= 1;
    }

    multiple >>= 16;
    fBits = fPart / multiple;
    Val = (iBits + fBits);
    return Val;
}
