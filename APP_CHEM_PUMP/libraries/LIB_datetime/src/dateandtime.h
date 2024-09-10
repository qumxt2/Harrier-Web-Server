// dateandtime.h

// Copyright 2010
// Graco Inc., Minneapolis, MN
// All Rights Reserved

#ifndef DATEANDTIME_H
#define DATEANDTIME_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

// ******************************************************************************************
// CONSTANTS AND MACROS
// ******************************************************************************************

// A valid day of week number is from 1 through 7, beginning with Monday (1)
// and ending with Sunday (7), as defined by ISO 8601.
typedef enum
{
	RTC_DAY_OF_WEEK_INVALID = 	0,
	RTC_DAY_OF_WEEK_MONDAY = 	1,
	RTC_DAY_OF_WEEK_TUESDAY = 	2,
	RTC_DAY_OF_WEEK_WEDNESDAY =	3,
	RTC_DAY_OF_WEEK_THURSDAY =	4,
	RTC_DAY_OF_WEEK_FRIDAY =	5,
	RTC_DAY_OF_WEEK_SATURDAY = 	6,
	RTC_DAY_OF_WEEK_SUNDAY = 	7
}dayOfWeek_t;

// Use a structure of this type when setting or reading the date and time
typedef struct
{
	uint8 seconds;
	uint8 minutes;
	uint8 hours;
	uint8 dayOfWeek;
	uint8 dayOfMonth;
	uint8 month;
	uint8 year;	
}dateAndTime_t;	

#define CHECK_YEAR_FOR_LEAP_DAY( year )			((!(year%4) && (year%100)) || !(year%400))
#define YEAR_EPOCH								(2000)
#define YEAR_MINIMUM_ALLOWED					(2000)
#define YEAR_MAXIMUM_ALLOWED					(2099)
#define INVALID_DAY_OF_MONTH					(0xFF)
#define MONTH_MINIMUM_ALLOWED					(1)
#define MONTH_MAXIMUM_ALLOWED					(12)

#define HOURS_IN_DAY		(24UL)
#define MINUTES_IN_HOUR		(60UL)
#define MINUTES_IN_DAY		(HOURS_IN_DAY*MINUTES_IN_HOUR)
#define SECONDS_IN_MINUTE	(60UL)
#define DAYS_IN_YEAR		(365UL)

// ******************************************************************************************
// FUNCTION PROTOTYPES
// ******************************************************************************************

//----------------------------------------------------------------------------
//! \brief Returns the day of the week given the date
//!
//! \param dayofmonth	Day of the month
//!	\param month		Month
//! \param year			Year (year 2010 would be entered as 10 (2010 - YEAR_EPOCH) )
//!
//! \return dayOfWeek_t	Day of the week
//----------------------------------------------------------------------------
dayOfWeek_t DateAndTime_GetDayOfWeek( uint8 dayofmonth, uint8 month, uint8 year );

//----------------------------------------------------------------------------
//! \brief Returns the date for Easter for a given year.
//!
//! \param year			Year (year 2010 would be entered as 10 (2010 - YEAR_EPOCH) )
//!
//! \return dateAndTime_t	dayOfMonth, month, year and dayOfWeek contain the
//!							proper data.
//----------------------------------------------------------------------------
dateAndTime_t DateAndTime_GetEasterDate(uint8 year);

//----------------------------------------------------------------------------
//! \brief	Returns the datetime for the n'th instance of a day-of-week in a
//!			given month in a given year.
//!
//! \param month			1 <= month <= 12
//! \param week				1 <= month <= 5; The instance of weekday that the
//!							date falls on (5=last instance in the month).
//! \param weekday			1 <= month <= 7; The day of the week that the
//!							desired date falls on
//! \param year				0 <= year <= 99;
//!							(year 2010 would be entered as 10 (2010 - YEAR_EPOCH) )
//!
//! \return dateAndTime_t	dayOfMonth, month, year and dayOfWeek contain the
//!							proper data.
//----------------------------------------------------------------------------

dateAndTime_t DateAndTime_GetDateByMonthWeekWeekday(int month, int week, int weekday, uint8 year);

//----------------------------------------------------------------------------
//! \brief Returns the number of days in a particular month in a particular year.
//!
//! \param month		1 <= month <= 12
//! \param year			0 <= year <= 99;
//!						(year 2010 would be entered as 10 (2010 - YEAR_EPOCH) )
//!
//! \return uint8		number of days in specified month
//----------------------------------------------------------------------------
uint8 DateAndTime_GetDaysInMonth(uint8 month, uint8 year);

//----------------------------------------------------------------------------
//! \brief	Returns the number of days from epoch until the specified datetime.
//!
//! \param datetime		Datetime to be used for calculation
//!
//! \return uint32		Number of days
//----------------------------------------------------------------------------
uint32 DateAndTime_GetDaysFromEpoch(dateAndTime_t datetime);

//----------------------------------------------------------------------------
//! \brief	Returns the number of days difference between the second and first
//!			datetimes.
//!
//! \param first		Datetime to be used for calculation
//! \param second		Datetime to be used for calculation
//!
//! \return sint32		Number of days
//----------------------------------------------------------------------------
sint32 DateAndTime_GetElapsedDays( dateAndTime_t first, dateAndTime_t second );

//----------------------------------------------------------------------------
//! \brief	Returns the datetime after advancing/decrementing the specified
//!			datetime by 1 month
//!
//! \param indate		Datetime input
//! \param forward		If forward is TRUE, a month will be added to the datetime.
//!						If forward is FALSE, a month with be subtracted from the datetime.
//!
//! \return dateAndTime_t	Resulting datetime.  If original dayOfMonth is invalid
//!							in new month/year, the dayOfMonth will be corrected.
//!	NOTE: The years are limited to 0 - 100 (2000 - 2100)
//----------------------------------------------------------------------------
dateAndTime_t DateAndTime_AdvanceDateByMonth( dateAndTime_t indate, bool forward );

//----------------------------------------------------------------------------
//! \brief	Returns the datetime after advancing/decrementing the specified
//!			datetime by 1 year
//!
//! \param indate		Datetime input
//! \param forward		If forward is TRUE, a year will be added to the datetime.
//!						If forward is FALSE, a year with be subtracted from the datetime.
//!
//! \return dateAndTime_t	Resulting datetime.  If original dayOfMonth is invalid
//!							in new month/year, the dayOfMonth will be corrected.
//!	NOTE: The years are limited to 0 - 100 (2000 - 2100)
//----------------------------------------------------------------------------
dateAndTime_t DateAndTime_AdvanceDateByYear( dateAndTime_t indate, bool forward );

//----------------------------------------------------------------------------
//! \brief	Returns the number of minutes that the specified datetime occurred
//!			after the epoch.
//!
//! \param datetime		Datetime input
//!
//! \return uint32	number of minutes past epoch
//----------------------------------------------------------------------------
uint32 DateAndTime_ConvertDatetimeToMinutesFromEpoch(dateAndTime_t datetime);

//----------------------------------------------------------------------------
//! \brief	Returns the datetime that corresponds to the specified number of
//!			minutes after the epoch.
//!
//! \param in_minutes	Number of minutes after epoch
//!
//! \return dateAndTime_t	Resulting datetime
//----------------------------------------------------------------------------
dateAndTime_t DateAndTime_ConvertMinutesFromEpochToDatetime(uint32 in_minutes);

//----------------------------------------------------------------------------
//! \brief	Compares two datetimes
//!
//! \param a		Datetime to be used for calculation
//! \param b		Datetime to be used for calculation
//!
//! \return sint32	Returns 1 if datetime a occurs after datetime b
//!					Returns 0 if datetime a is the same as datetime b
//!					Returns -1 if datetime a occurs before datetime b
//----------------------------------------------------------------------------
sint32 DateAndTime_CompareDatetimes(dateAndTime_t a, dateAndTime_t b);

//----------------------------------------------------------------------------
//! \brief	Returns the datetime after advancing the specified datetime by one second
//!
//! \param indate		Datetime input
//!
//! \return dateAndTime_t	Resulting datetime.
//!
//!	NOTE: The years are limited to 0 - 100 (2000 - 2100)
//----------------------------------------------------------------------------
dateAndTime_t DateAndTime_AdvanceDateTimeBySecond( dateAndTime_t indate );

#endif 		// DATEANDTIME_H
