//! \file	debug.h
//! \brief The debug module header file. \sa debug_module
//!
//! Copyright 2006
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \addtogroup gca_portal
//!
//! \addtogroup gca_debugprint		DEBUG_PRINT Interface
//! \brief A group of simple data logging functions that provide a means
//! to print debug information with a minimum amount of overhead.
//!
//! This is a fast debug interface, with very specialized operations.
//! Little stack space is required (roughly 30 bytes), and the
//! operations execute with speed and efficiency as the top priority.

		
#ifndef DEBUG_H
#define DEBUG_H

//! \ingroup debug_debugprint
//@{

// Include basic platform types
#include "typedef.h"

// Eliminate lint warnings of a constant value boolean during expansion
// of the following macros
//lint -emacro(506, DEBUG_PRINT_STRING)
//lint -emacro(506, DEBUG_PRINT_SIGNED_DECIMAL)
//lint -emacro(506, DEBUG_PRINT_UNSIGNED_DECIMAL)
//lint -emacro(506, DEBUG_PRINT_UINT8)
//lint -emacro(506, DEBUG_PRINT_UINT16)
//lint -emacro(506, DEBUG_PRINT_UINT32)

#ifdef NO_DEBUG_PRINT
#define	DEBUG_PRINT_STRING(mask, string)		do { } while (0)
#define	DEBUG_PRINT_SIGNED_DECIMAL(mask, value)	do { } while (0)
#define	DEBUG_PRINT_UNSIGNED_DECIMAL(mask, value)	do { } while (0)
#define	DEBUG_PRINT_UINT8(mask, value)			do { } while (0)
#define	DEBUG_PRINT_UINT16(mask, value)			do { } while (0)
#define	DEBUG_PRINT_UINT32(mask, value)			do { } while (0)
#else

/// Prints a constant string if \c mask contains a bit that overlaps with
/// the global debug mask ::g_DebugMask.
/// \hideinitializer
#define	DEBUG_PRINT_STRING(mask, string)									\
		do {																\
			if( (DBUG_ALWAYS & (mask)) || (g_DebugMask & (mask)) )			\
				DebugPrintString( string );									\
		} while (0)

/// Prints a signed decimal value if \c mask contains a bit that overlaps with
/// the global debug mask ::g_DebugMask.
/// \hideinitializer
#define	DEBUG_PRINT_SIGNED_DECIMAL(mask, value)								\
		do {																\
			if( (DBUG_ALWAYS & (mask)) || (g_DebugMask & (mask)) )			\
				DebugPrintSignedDecimal( value );							\
		} while (0)

/// Prints an unsigned decimal value if \c mask contains a bit that overlaps
/// with the global debug mask ::g_DebugMask.
/// \hideinitializer
#define	DEBUG_PRINT_UNSIGNED_DECIMAL(mask, value)							\
		do {																\
			if( (DBUG_ALWAYS & (mask)) || (g_DebugMask & (mask)) )			\
				DebugPrintUnsignedDecimal( value );							\
		} while (0)

/// Prints the file name and line of code reached if \c mask contains a bit that
/// overlaps with the global debug mask ::g_DebugMask.
/// \hideinitializer
#define DEBUG_PRINT_FILE_LINE(mask)                                         \
        do {                                                                \
            DEBUG_PRINT_STRING( mask, __FILE__ );                           \
            DEBUG_PRINT_STRING( mask, " line " );                           \
            DEBUG_PRINT_UNSIGNED_DECIMAL( mask, __LINE__ );                 \
            DEBUG_PRINT_STRING( mask, "\n" );                               \
        } while (0)

/// Prints a byte value in hexadecimal value if \c mask contains a bit
/// that overlaps with the global debug mask ::g_DebugMask.
/// \hideinitializer
#define	DEBUG_PRINT_UINT8(mask, value)										\
		do {																\
			if( (DBUG_ALWAYS & (mask)) || (g_DebugMask & (mask)) )			\
				DebugPrintUInt8( value );									\
		} while (0)

/// Prints a word value in hexadecimal value if \c mask contains a bit
/// that overlaps with the global debug mask ::g_DebugMask.
/// \hideinitializer
#define	DEBUG_PRINT_UINT16(mask, value)										\
		do {																\
			if( (DBUG_ALWAYS & (mask)) || (g_DebugMask & (mask)) )			\
				DebugPrintUInt16( value );									\
		} while (0)

/// Prints a longword value in hexadecimal value if \c mask contains a bit
/// that overlaps with the global debug mask ::g_DebugMask.
/// \hideinitializer
#define	DEBUG_PRINT_UINT32(mask, value)										\
		do {																\
			if( (DBUG_ALWAYS & (mask)) || (g_DebugMask & (mask)) )			\
				DebugPrintUInt32( value );									\
		} while (0)

//-------------------------

#endif	// NO_DEBUG_PRINT

/// Debug mask bit: For debug messages that should \b always be displayed.
/// \hideinitializer
#define	DBUG_ALWAYS							((uint16) (1 << 0))

/// Debug mask bit: For notable events that occur during system initialization
/// \hideinitializer
#define	DBUG_INIT							((uint16) (1 << 1))

/// Debug mask bit: For notable events in icbp.c
/// \hideinitializer
#define	DBUG_ICB							((uint16) (1 << 2))

/// Debug mask bit: For notable events in canbuffer.c
/// \hideinitializer
#define	DBUG_CAN_BUFFER						((uint16) (1 << 3))

/// Debug mask bit: For notable events in candrv.c
/// \hideinitializer
#define	DBUG_CAN_DRV						((uint16) (1 << 4))

/// Debug mask bit: For notable events in dvar.c
/// \hideinitializer
#define	DBUG_DVAR							((uint16) (1 << 5))

/// Debug mask bit: For notable events in spi.c
/// \hideinitializer
#define	DBUG_SPI							((uint16) (1 << 6))

/// General application-level debug mask bits
#define	DBUG_APP_CATEGORY(n)				((uint16) (1 << (7 + n)))

/// Default setting for ::g_DebugMask on system start-up:  ::DBUG_INIT and
/// ::DBUG_ALWAYS.
/// \hideinitializer
#define	DEFAULT_DEBUG_MASK					(								\
												DBUG_INIT				|	\
												DBUG_ALWAYS				|	\
											0)

/// \brief The global debug mask.
///
/// The \c mask parameter of the various \c DEBUG macros is compared to this
/// variable in order to determine whether or not a certain debug message
/// should be generated.
/// \note Do \b not modify directly; use the debug console
/// command "debugmask" instead.
extern uint16 g_DebugMask;

/// This macro returns the present value of the global debug mask.
/// \hideinitializer
#define	GET_GLOBAL_DEBUG_MASK()				((uint16) (g_DebugMask))


//----------------------------------------------------------------------------
//! \brief Initialization function.
//!
//! Must be invoked prior to using the remainder of this interface.
//!
//! \return A negative value if the initialization failed, 0 on success.
//----------------------------------------------------------------------------
sint16 DebugInit( void );


//----------------------------------------------------------------------------
//! \brief	Set the global debug mask.
//!
//! This function will set the global debug mask to the specified value.
//!
//! \param	newMask			The new debug mask setting.
//!
//! \b Notes:
//!	\li	The debug mask bit ::DBUG_ALWAYS will be automatically OR'd into
//!	the new debug mask.  There is no way to deactivate that mask bit.
//!
//! \return	 void
//----------------------------------------------------------------------------
void DebugSetMask( uint16 newMask );


//----------------------------------------------------------------------------
//! \brief	Get the amount of free space in the DEBUG_PRINT buffer.
//!
//! \return	 The amount of free space in the DEBUG_PRINT buffer in bytes
//----------------------------------------------------------------------------
uint16 DebugGetBufferStatus( void );


// ***************************************************
// * MACROS
// ***************************************************
#define TRAP_RESET_OCCURED(rcon)			(rcon & 0x8000)
#define ILLEGAL_OPCODE_RESET_OCCURED(rcon)	(rcon & 0x4000)
#define EXTERNAL_RESET_OCCURED(rcon)		(rcon & 0x0080)
#define SOFTWARE_RESET_OCCURED(rcon)		(rcon & 0x0040)
#define WATCHDOG_RESET_OCCURED(rcon)		(rcon & 0x0010)
#define BROWNOUT_RESET_OCCURED(rcon)		(rcon & 0x0002)
#define POWERON_RESET_OCCURED(rcon)			(rcon & 0x0001)


//----------------------------------------------------------------------------
//! \brief Prep the system to allow for information dumps out the CCA Debug
//!        Portal during the startup_pre_datainit function.
//!
//! \return value of RCON special function register before this function 
//!         clears the reset flags.  This return value can be used with
//!         the macros defined in debug.h to determin which type(s) of 
//!         reset occured
//----------------------------------------------------------------------------
uint16  DebugPreDatainitPrep (void);


//----------------------------------------------------------------------------
//! \brief Dump information to the CCA Debug Portal based on the value passed
//!        in rcon_value.
//!
//! \param	rcon_value		The value of RCON to evaluate for reset flags.
//----------------------------------------------------------------------------
void   DebugDumpResetData (uint16 rcon_value);	
				

//----------------------------------------------------------------------------
//! \brief Wait for serial communications to wrap up before exiting the 
//!        startup_pre_datainit function.
//----------------------------------------------------------------------------
void  DebugPreDatainitFinish (void);



/// @cond

/// These functions are for internal use only.  Do not include in the
/// interface documentation.
void DebugPrintString( const char *string );
void DebugPrintSignedDecimal( sint32 decimalValue );
void DebugPrintUnsignedDecimal( uint32 decimalValue );
void DebugPrintUInt8( uint8 hexValue );
void DebugPrintUInt16( uint16 hexValue );
void DebugPrintUInt32( uint32 hexValue );

/// @endcond

//@}

#endif // DEBUG_H

