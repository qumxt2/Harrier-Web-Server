// datetime.c
//
// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved
//
// DESCRIPTION

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "typedef.h"
#include "datetime.h"
//#include "rtc_m41t81sm6f.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

static void UpdateTime( void );

// *****************************************************************************
// * STATIC VARIABLES
// *****************************************************************************

//static DATETIME_DATE_FORMAT_t sDateFormat = DATETIME_DATE_FORMAT_MM_DD_YY;
static dateAndTime_t m_dateAndTime;

// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************

DATETIME_DATE_FORMAT_t DATETIME_GetDateFormat( void )
{
    //  APP_Library_DataFormat defined in app_library.c
    //	return (DATETIME_DATE_FORMAT_t) APP_Library_DateFormat( FALSE, 0 );
    return 0;
}

void DATETIME_SetDateFormat( DATETIME_DATE_FORMAT_t date_format )
{
	if( date_format < DATETIME_DATE_FORMAT_NUM_FORMATS )
	{
            //          see app_library.c
            //		(void) APP_Library_DateFormat ( TRUE, date_format );
	}
}

DistVarType DATETIME_CurrentTime( bool set, uint32 newVal )
{
    uint32 val;
    DATETIME_TIME_t *rVal = (DATETIME_TIME_t*)&val;
    DATETIME_TIME_t *new = (DATETIME_TIME_t*)&newVal;
    UpdateTime();
    
    if (set)
    {
        m_dateAndTime.hours = new->hour;
        m_dateAndTime.minutes = new->minute;
        m_dateAndTime.seconds = new->second;
//        setDateAndTime(&m_dateAndTime);
    } 
    
    rVal->hour = m_dateAndTime.hours;
    rVal->minute = m_dateAndTime.minutes;
    rVal->second = m_dateAndTime.seconds;
    return val;
}

DistVarType DATETIME_CurrentDate( bool set, uint32 newVal )
{
    uint32 val;
    DATETIME_DATE_t *rVal = (DATETIME_DATE_t*)&val;
    DATETIME_DATE_t *new = (DATETIME_DATE_t*)&newVal;
	UpdateTime();    

    if( set )
    {
        m_dateAndTime.year = new->year;
        m_dateAndTime.month = new->month;
        m_dateAndTime.dayOfMonth = new->day;
//        setDateAndTime(&m_dateAndTime);
    }
    
    rVal->year = m_dateAndTime.year;
    rVal->month = m_dateAndTime.month;
    rVal->day = m_dateAndTime.dayOfMonth;
    
    return val;
}

// *****************************************************************************
// * PRIVATE FUNCTIONS
// *****************************************************************************

static void UpdateTime( void )
{
//    bool set = FALSE;
    
//    if( checkHaltFlag() )
//    {
//        (void) resetHaltFlag();
//    }
//
//    #define GETTIMEVERIFY(x,y) if (x > y) { set = TRUE; x = y;}
//
//	if( !getDateAndTime(&m_dateAndTime) )
//	{
//		GETTIMEVERIFY(m_dateAndTime.seconds,59);
//		GETTIMEVERIFY(m_dateAndTime.minutes,59);
//		GETTIMEVERIFY(m_dateAndTime.hours,23);
//		GETTIMEVERIFY(m_dateAndTime.dayOfMonth,31);
//		GETTIMEVERIFY(m_dateAndTime.month,12);
//		GETTIMEVERIFY(m_dateAndTime.year,99);
//		GETTIMEVERIFY(m_dateAndTime.dayOfWeek,7);
//
//        if( set )
//        {
//            setDateAndTime(&m_dateAndTime);
//        }
//	}
}
