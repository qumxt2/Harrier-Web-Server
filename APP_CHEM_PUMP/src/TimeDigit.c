
//------------------------------------------------------------------------------ 
// File:		TimeDigit.c       
//
// Purpose:     Routines to display and edit MMSS and HHMMSS time values
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
#include "typedef.h"
#include "screensTask.h"
#include "CountDigit.h"
#include "TimeDigit.h"
#include "gdisp.h"

static void processTimeInputUpArrowEvent(TIME_DIGIT_t *);
static void processTimeInputDownArrowEvent(TIME_DIGIT_t *);
static void processTimeInputRightArrowEvent(TIME_DIGIT_t *);
static void processTimeInputLeftArrowEvent(TIME_DIGIT_t *);
static void processTimeInputDefaultEvent(TIME_DIGIT_t *);

//------------------------------------------------------------------------------ 
//  FUNCTION:       GetTimeDigitValue ()
//
//  PARAMETERS:     TIME_DIGIT_t *TimeDigit  // Time digit to value 
//
//  DESCRIPTION:    Returns INT32U time associated with digits in pointed to time digit    
//
//  RETURNS:        INT32U  Time
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
INT32U GetTimeDigitValue (TIME_DIGIT_t *TimeDigit)
{
    INT32U Time;

    Time = (INT32U)((TimeDigit->DigitValue[TIME_DIGIT_HH] * 10) + TimeDigit->DigitValue[TIME_DIGIT_HL]) * 3600L;
    Time += (INT32U)((TimeDigit->DigitValue[TIME_DIGIT_MH] * 10) + TimeDigit->DigitValue[TIME_DIGIT_ML]) * 60L;
    Time += (INT32U)(TimeDigit->DigitValue[TIME_DIGIT_SH] * 10) + TimeDigit->DigitValue[TIME_DIGIT_SL];

    return (Time);
}

//------------------------------------------------------------------------------ 
//  FUNCTION:       LoadTimeDigit ()
//
//  PARAMETERS:     TIME_DIGIT_t *TimeDigit  // Time digit to load
//                  INT32U Time              // Time to load            
//                  INT8U ActiveDigits       // Number of digits to use
//
//  DESCRIPTION:    Loads Time into the pointer to time digit    
//
//  RETURNS:        BOOLEAN
//                      TRUE --> Time was in range and was loaded
//                      FALSE --> Time was out of range and not loaded
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
BOOLEAN LoadTimeDigit (TIME_DIGIT_t *TimeDigit,
                       INT32U Time,
                       INT8U ActiveDigits,
                       INT8U PositionX,
                       INT8U PositionY)
{
    INT8U Hour;
    INT8U Minute;
    INT8U Second;

    // Set the active digits
    TimeDigit->ActiveDigits = ActiveDigits;

    TimeDigit->PositionX = PositionX;
    TimeDigit->PositionY = PositionY;
    
    // Range check
    switch (ActiveDigits)
    {
        case 6:
            // Range check will occur in next function
            break;

        case 4:
            if (Time > ((60 * 59) + 59))
            {
                return (FALSE);
            }

            break;

        default:
            return (FALSE);
        
            //break;
    }

    if (SecondsToHMS (Time,
                      &Hour,
                      &Minute,
                      &Second))
    {
        // Convert Hour
        TimeDigit->DigitValue[TIME_DIGIT_HH] = Hour / 10;
        TimeDigit->DigitValue[TIME_DIGIT_HL] = Hour % 10;

        // Convert Minute
        TimeDigit->DigitValue[TIME_DIGIT_MH] = Minute / 10;
        TimeDigit->DigitValue[TIME_DIGIT_ML] = Minute % 10;

        // Convert Second
        TimeDigit->DigitValue[TIME_DIGIT_SH] = Second / 10;
        TimeDigit->DigitValue[TIME_DIGIT_SL] = Second % 10;
    }
    else
    {
        return (FALSE);
    }

    return (TRUE);
}
     
//------------------------------------------------------------------------------ 
//  FUNCTION:       ProcessTimeDigitEvent ()
//
//  PARAMETERS:     TIME_DIGIT_t *CountDigit  // Time digit operate on
//                  INPUT_EVENT_t InputEvent   // Keypad event to process
//
//  DESCRIPTION:    Uses passed keypad event to update pointed to time digit.    
//
//  RETURNS:        BOOLEAN
//                      TRUE --> Event was processed succesfully
//                      FALSE ->> Event could not be processed        
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
BOOLEAN ProcessTimeDigitEvent (TIME_DIGIT_t *TimeDigit,
                                     INPUT_EVENT_t InputEvent)
{
    BOOLEAN ReturnValue = TRUE;

    void (*processInputEvent[NUMBER_OF_INPUT_EVENTS])(TIME_DIGIT_t *);
    
    processInputEvent[INPUT_EVENT_ENTRY_INIT] = processTimeInputDefaultEvent;
    processInputEvent[INPUT_EVENT_RESET] = processTimeInputDefaultEvent;
    processInputEvent[INPUT_EVENT_ENTER] = processTimeInputDefaultEvent;
    processInputEvent[INPUT_EVENT_UP_ARROW] = processTimeInputUpArrowEvent;
    processInputEvent[INPUT_EVENT_DOWN_ARROW] = processTimeInputDownArrowEvent;
    processInputEvent[INPUT_EVENT_RIGHT_ARROW] = processTimeInputRightArrowEvent;
    processInputEvent[INPUT_EVENT_LEFT_ARROW] = processTimeInputLeftArrowEvent;
    processInputEvent[INPUT_EVENT_PRESS_HOLD_ENTER] = processTimeInputDefaultEvent;
    processInputEvent[INPUT_EVENT_BOTH_ARROWS] = processTimeInputDefaultEvent;
    processInputEvent[INPUT_EVENT_REFRESH_SCREEN] = processTimeInputDefaultEvent;

    // Do a bit of error checking
    if (TimeDigit->DigitSelected < NUMBER_OF_TIME_DIGITS)
    {
        // Process based on event
        (void)(*processInputEvent[InputEvent])(TimeDigit);
    }
    else
    {
        ReturnValue = FALSE;
    }
    
    return (ReturnValue);
}

//------------------------------------------------------------------------------ 
//  FUNCTION:       DisplayTimeDigit ()
//
//  PARAMETERS:     TIME_DIGIT_t *TimeDigit   // Time digit to display
//                  INT8U PositionX             // X char position to place value
//                  INT8U PositionY             // Y char position to place value
//
//  DESCRIPTION:    Displays pointed to time digit at x,y char coordinates    
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DisplayTimeDigit (TIME_DIGIT_t *TimeDigit)
{
    switch (TimeDigit->ActiveDigits)
    {
        case 4:
            // Do the digits sequentially
            gsetcpos (TimeDigit->PositionX,
                      TimeDigit->PositionY);
        
            gputch (TimeDigit->DigitValue[TIME_DIGIT_MH] + '0');
        
        
            gsetcpos (TimeDigit->PositionX + 1,
                      TimeDigit->PositionY);
        
			gputch(TimeDigit->DigitValue[TIME_DIGIT_ML] + '0');

			gsetcpos(TimeDigit->PositionX + 2,
				TimeDigit->PositionY);

			gputch(':');
            
            gsetcpos (TimeDigit->PositionX + 3,
                      TimeDigit->PositionY);

        
            gputch (TimeDigit->DigitValue[TIME_DIGIT_SH] + '0');
            
            gsetcpos (TimeDigit->PositionX + 4,
                      TimeDigit->PositionY);
        
            gputch (TimeDigit->DigitValue[TIME_DIGIT_SL] + '0');
       
            break;
    
        case 6:
            // Do the digits sequentially
            gsetcpos (TimeDigit->PositionX,
                      TimeDigit->PositionY);
        
            gputch (TimeDigit->DigitValue[TIME_DIGIT_HH] + '0');
        
        
            gsetcpos (TimeDigit->PositionX + 1,
                      TimeDigit->PositionY);
        
            gputch (TimeDigit->DigitValue[TIME_DIGIT_HL] + '0');

			gsetcpos(TimeDigit->PositionX + 2,
				TimeDigit->PositionY);

			gputch(':');
            
            gsetcpos (TimeDigit->PositionX + 3,
                      TimeDigit->PositionY);
        
            gputch (TimeDigit->DigitValue[TIME_DIGIT_MH] + '0');
            
            gsetcpos (TimeDigit->PositionX + 4,
                      TimeDigit->PositionY);
        
            gputch (TimeDigit->DigitValue[TIME_DIGIT_ML] + '0');
			
			gsetcpos(TimeDigit->PositionX + 5,
				TimeDigit->PositionY);

			gputch(':');
        
            gsetcpos (TimeDigit->PositionX + 6,
                      TimeDigit->PositionY);
        
            gputch (TimeDigit->DigitValue[TIME_DIGIT_SH] + '0');
            
            gsetcpos (TimeDigit->PositionX + 7,
                      TimeDigit->PositionY);
        
            gputch (TimeDigit->DigitValue[TIME_DIGIT_SL] + '0');
            
            break;

        default:
            break;
    }
}


//------------------------------------------------------------------------------ 
//  FUNCTION:       DrawTimeDigitDigitBox ()
//
//  PARAMETERS:     TIME_DIGIT_t *TimeDigit   // Time digit 
//
//  DESCRIPTION:    Draws box around active digit in pointed to time digit   
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DrawTimeDigitDigitBox (TIME_DIGIT_t *TimeDigit)
{
    switch (TimeDigit->ActiveDigits)
    {
        case 4:
            switch (TimeDigit->DigitSelected)
            {
                case TIME_DIGIT_MH:
                     DrawDigitBox (TimeDigit->PositionX,
                                   TimeDigit->PositionY);

                    break;
        
                case TIME_DIGIT_ML:
                     DrawDigitBox (TimeDigit->PositionX + 1,
                                   TimeDigit->PositionY);
        
                    break;
                
                case TIME_DIGIT_SH:
                     DrawDigitBox (TimeDigit->PositionX + 3,
                                   TimeDigit->PositionY);
        
                    break;
        
                case TIME_DIGIT_SL:
                     DrawDigitBox (TimeDigit->PositionX + 4,
                                   TimeDigit->PositionY);
        
                    break;
        
                default:
                    break;
            }
            
            break;

        case 6:
            switch (TimeDigit->DigitSelected)
            {
                case TIME_DIGIT_HH:
                     DrawDigitBox (TimeDigit->PositionX,
                                   TimeDigit->PositionY);
        
                    break;
        
                case TIME_DIGIT_HL:
                     DrawDigitBox (TimeDigit->PositionX + 1,
                                   TimeDigit->PositionY);
        
                    break;
                
                case TIME_DIGIT_MH:
                     DrawDigitBox (TimeDigit->PositionX + 3,
                                   TimeDigit->PositionY);
        
                    break;
        
                case TIME_DIGIT_ML:
                     DrawDigitBox (TimeDigit->PositionX + 4,
                                   TimeDigit->PositionY);
        
                    break;
        
                case TIME_DIGIT_SH:
                     DrawDigitBox (TimeDigit->PositionX + 6,
                                   TimeDigit->PositionY);
        
                    break;
        
                case TIME_DIGIT_SL:
                     DrawDigitBox (TimeDigit->PositionX + 7,
                                   TimeDigit->PositionY);
        
                    break;
        
                default:
                    break;
            }
            
            break;
        
        default:
            break;
    }
}


//------------------------------------------------------------------------------ 
//  FUNCTION:       DrawTimeDigitBox ()
//
//  PARAMETERS:     TIME_DIGIT_t *TimeDigit   // Time digit 
//
//  DESCRIPTION:    Draws box around the pointed to time digit   
//
//  RETURNS:        VOID
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
VOID DrawTimeDigitBox (TIME_DIGIT_t *TimeDigit)
{
    switch (TimeDigit->ActiveDigits)
    {
        case 4:
            DrawBox (TimeDigit->PositionX,
                     TimeDigit->PositionY,
                     5);

            break;

        case 6:
            DrawBox (TimeDigit->PositionX,
                     TimeDigit->PositionY,
                     8);

            break;
        
        default:
            break;
    }
}

//------------------------------------------------------------------------------
//  FUNCTION:       SecondstoHMS
//
//  PARAMETERS:     INT32U SecondsIn        // Number of seconds to convert to HHMMSS
//
//  DESCRIPTION:    Converts seconds to HH MM SS
//
//  RETURNS:        BOOLEAN
//                      TRUE --> Conversion Succesful
//                      FALSE --> Value out of range of 99:59:59
//                  INT8U *Hours            // Number of hours
//                  INT8U *Minutes          // Number of minutes
//                  INT8U *Seconds          // Number of seconds
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------
BOOLEAN SecondsToHMS (INT32U SecondsIn,
                      INT8U *Hours,
                      INT8U *Minutes,
                      INT8U *Seconds)
{
    // 99 Hours, 59 minutes, 59 seconds is max time
    if (SecondsIn > ((99L * 3600L) + (59L * 60L) + 59L))
    {
        return (FALSE);
    }

    // Calculate hours
    *Hours = SecondsIn / 3600L;

    // Calculate remaining seconds
    SecondsIn -= (*Hours * 3600L);

    // Calculate minutes
    *Minutes = SecondsIn / 60L;

    // Seconds are remaining time
    *Seconds  = SecondsIn - (*Minutes * 60L);

    return (TRUE);
}

//****************************************************************************//
//Fcn: processTimeInputUpArrowEvent
//
//Desc: This function processes the up arrow events
//****************************************************************************//
static void processTimeInputUpArrowEvent(TIME_DIGIT_t *TimeDigit)
{
    switch (TimeDigit->DigitSelected)
    {
        case TIME_DIGIT_HH:
        case TIME_DIGIT_HL:
        case TIME_DIGIT_ML:
        case TIME_DIGIT_SL:
            // This inceases the currently selected digit up to 9
            if (TimeDigit->DigitValue[TimeDigit->DigitSelected] < 9)
            {
                TimeDigit->DigitValue[TimeDigit->DigitSelected]++;
            }
            else
            {
                TimeDigit->DigitValue[TimeDigit->DigitSelected] = 0;
            }

            break;

        case TIME_DIGIT_MH:
        case TIME_DIGIT_SH:
            // This increases the currently selected digit up to 5
            if (TimeDigit->DigitValue[TimeDigit->DigitSelected] < 5)
            {
                TimeDigit->DigitValue[TimeDigit->DigitSelected]++;
            }
            else
            {
                TimeDigit->DigitValue[TimeDigit->DigitSelected] = 0;
            }

            break;

        default:

            break;
    }
}

//****************************************************************************//
//Fcn: processTimeInputDownArrowEvent
//
//Desc: This function processes the down arrow events
//****************************************************************************//
static void processTimeInputDownArrowEvent(TIME_DIGIT_t *TimeDigit)
{
    switch (TimeDigit->DigitSelected)
    {
        case TIME_DIGIT_HH:
        case TIME_DIGIT_HL:
        case TIME_DIGIT_ML:
        case TIME_DIGIT_SL:
            // This decreases the currently selected with rollover to 9
            if (TimeDigit->DigitValue[TimeDigit->DigitSelected] > 0)
            {
                TimeDigit->DigitValue[TimeDigit->DigitSelected]--;
            }
            else
            {
                TimeDigit->DigitValue[TimeDigit->DigitSelected] = 9;
            }

            break;

        case TIME_DIGIT_MH:
        case TIME_DIGIT_SH:
            // This decreases the currently selected with rollover to 5
            if (TimeDigit->DigitValue[TimeDigit->DigitSelected] > 0)
            {
                TimeDigit->DigitValue[TimeDigit->DigitSelected]--;
            }
            else
            {
                TimeDigit->DigitValue[TimeDigit->DigitSelected] = 5;
            }

            break;

        default:

            break;
    }
}

//****************************************************************************//
//Fcn: processInputRightArrowEvent
//
//Desc: This function processes the right arrow events
//****************************************************************************//
static void processTimeInputRightArrowEvent(TIME_DIGIT_t *TimeDigit)
{
    if (TimeDigit->DigitSelected > 0)
    {
        TimeDigit->DigitSelected--;
    }
    else
    {
        TimeDigit->DigitSelected = TimeDigit->ActiveDigits - 1;
    }
}

//****************************************************************************//
//Fcn: processInputLeftEvent
//
//Desc: This function processes the left arrow events
//****************************************************************************//
static void processTimeInputLeftArrowEvent(TIME_DIGIT_t *TimeDigit)
{
    if (TimeDigit->DigitSelected < (TimeDigit->ActiveDigits - 1))
    {
        TimeDigit->DigitSelected++;
    }
    else
    {
        TimeDigit->DigitSelected = 0;
    }
}

//****************************************************************************//
//Fcn: processInputDefaultEvent
//
//Desc: This handles all other possible events, not a button do nothing
//****************************************************************************//
static void processTimeInputDefaultEvent(TIME_DIGIT_t *TimeDigit)
{
    asm("nop");
}

