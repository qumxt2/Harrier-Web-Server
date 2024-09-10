//! \file datetime.h
//! \breif The module for maintaining date and time
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \addtogroup gca_adcm
//@{
//!
//! \addtogroup gca_adcm_datetime		ADCM Datetime Library
//! \brief ADCM (Advanced Display Module) Datetime Library
//!
//! \b DESCRIPTION:
//!    This module provides an interface for the date and time
//!    on the ADCM (Advanced Display Control Module)
#ifndef DATETIME_H
#define DATETIME_H

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************
#include "typedef.h"
#include "dvar.h"

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

/// The type that is to be returned by a function (application defined) that will be
/// called by this module to retrieve the current date
typedef struct
{
	uint8 year;			// (0-99)
	uint8 month;		// (1-12)
	uint8 day;			// day of month (1-31)
} DATETIME_DATE_t;

/// The type that is to be returned by a function (application defined) that will be
/// called by this module to retrieve the current time
typedef struct
{
	uint8 hour;
	uint8 minute;
	uint8 second;	
} DATETIME_TIME_t;

/// The enumerated type for specifying the display format of the date
typedef enum
{
	DATETIME_DATE_FORMAT_MM_DD_YY = 0,
	DATETIME_DATE_FORMAT_DD_MM_YY,
	DATETIME_DATE_FORMAT_YY_MM_DD,

	DATETIME_DATE_FORMAT_NUM_FORMATS,
	DATETIME_DATE_FORMAT_UNDEFINED = DATETIME_DATE_FORMAT_NUM_FORMATS
} DATETIME_DATE_FORMAT_t;

/// The type for a function (application defined) that will be called by this module
/// in order to retrieve current date information
typedef DATETIME_DATE_t (*DATETIME_DATE_FUNCTION_t)( void );

/// The type for a function (application defined) that will be called by this module
/// in order to retrieve current time information
typedef DATETIME_TIME_t (*DATETIME_TIME_FUNCTION_t)( void );

// *****************************************************************************
// * MARCROS
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTION PROTOTYPES
// *****************************************************************************

///------------------------------------------------------------------------------------------
//! \fn DATETIME_GetDateFormat( void )
//! \brief Retrieve the date display format
//!
//! \return The current date display format setting: MM_DD_YY or DD_MM_YY or YY_MM_DD
//----------------------------------------------------------------------------
DATETIME_DATE_FORMAT_t DATETIME_GetDateFormat( void );


///------------------------------------------------------------------------------------------
//! \fn DATETIME_SetDateFormat( void )
//! \brief Set the date display format
//!
//! \param date_format	The desired date display format of DATETIME_DATE_FORMAT_t type
//----------------------------------------------------------------------------
void DATETIME_SetDateFormat( DATETIME_DATE_FORMAT_t date_format );


///------------------------------------------------------------------------------------------
//! \fn DATETIME_CurrentDate( void )
//! \brief Set or Get current date
//! \param set	set flag. If TRUE, time will be set, if FALSE current date will be returned
//! \param newVal   pointer to DATETIME_DATE_t. If set flag is TRUE, function will use
//!                 passed structure pointer, to set date from that structre.
//!
//! \return Function returns current date as a pointer to a DATETIME_DATE_t structure
//----------------------------------------------------------------------------
DistVarType DATETIME_CurrentDate( bool set, uint32 newVal );


///------------------------------------------------------------------------------------------
//! \fn DATETIME_CurrentDate( void )
//! \brief Set or Get current date
//! \param set	set flag. If TRUE, time will be set, if FALSE current time will be returned
//! \param newVal   pointer to DATETIME_DATE_t. If set flag is TRUE, function will use
//!                 passed structure pointer, to set time from that structre.
//!
//! \return Function returns current time as a pointer to a DATETIME_DATE_t structure
//----------------------------------------------------------------------------
DistVarType DATETIME_CurrentTime( bool set, uint32 newVal );


#endif // DATETIME_H

//@}
