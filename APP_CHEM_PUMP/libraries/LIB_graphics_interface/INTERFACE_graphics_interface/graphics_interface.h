//! \file	graphics_interface.h
//! \brief Graphics Interface Library
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//!
//! \addtogroup gca_adcm
//@{
//!
//! \addtogroup gca_adcm_graphics_interface		ADCM Graphics Library
//! \brief ADCM (Advanced Display Module) Graphics Library
//!
//! \b DESCRIPTION:
//!    This module provides an interface for the IO Pin functions
//!    on the ADM (Advanced Display Module)
//!
//!    This file provides function prototypes for interfacing with
//!    the Display Control Monitor LCD display, including controlling input power, drawing screens,
//!    adjusting contrast, and controlling the backlight.
//!
//!    The following image represents the pixel layout of the physical LCD screen.  Use this
//!    image as a reference for the following function prototypes.
//!
//!    SCREEN LAYOUT
//!                    x = LCD_X_MIN   -->   -->   -->   -->   -->   -->   -->   x = LCD_X_MAX
//!    y = LCD_Y_MIN	-----------------------------------------------------------------
//!               		|                                                       		|
//!             |		|                                                               |
//!             |		|                                                   			|
//!             V		|                                                       		|
//!             		|                                                       		|
//!             		|                     LCD Screen Area                   		|
//!             |		|                                                   			|
//!             |		|                                               				|
//!             V		|                                               				|
//!             		|                                                   			|
//!    	y = LCD_Y_MAX	-----------------------------------------------------------------
//!

#ifndef GRAPHICS_INTERFACE_H
#define GRAPHICS_INTERFACE_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"					//!         Compiler specific type definitions
#include "gdisp.h"                      // Low level graphics driver library prototypes
#include "gdisphw.h"					//!         Low level graphics driver library prototypes

// ******************************************************************************************
// * MACROS & CONSTANTS
// ******************************************************************************************

// Constants to be used for absolute LCD screen position
#define LCD_WIDTH								(GDISPW)			//!         160 Pixels
#define LCD_HEIGHT								(GDISPH)			//!         100 Pixels
#define LCD_X_MAX								(LCD_WIDTH - 1)		//!         159
#define LCD_X_MIN								(0)					//!         0
#define LCD_Y_MAX								(LCD_HEIGHT - 1)	//!         99
#define LCD_Y_MIN								(0)					//!         0

// Constants to be used when turning the LCD pixel display on and off
#define DISPLAY_ON								(0x01)
#define DISPLAY_OFF								(0x00)

// Constants to be used when varying the LCD backlight brightness
#define BACKLIGHT_100                           (100)
#define BACKLIGHT_90                            (90)
#define BACKLIGHT_80                            (80)
#define BACKLIGHT_70                            (70)
#define BACKLIGHT_60                            (60)
#define BACKLIGHT_50                            (50)
#define BACKLIGHT_40                            (40)
#define BACKLIGHT_30                            (30)
#define BACKLIGHT_20                            (20)
#define BACKLIGHT_10                            (10)
#define BACKLIGHT_ON                            (1)
#define BACKLIGHT_OFF                           (0)

// Constants to be used as the fillPattern in the fillArea() function
#define FILL_CHECKERBOARD_PATTERN               (0xAA55)
#define FILL_SOLID								(0xFFFF)
#define FILL_VERTICAL_LINES                     (0xAAAA)
#define FILL_HORIZONTAL_LINES                   (0xFF00)

// Possible function return values... 
#define LCD_INIT_OK								(0)
#define LCD_INIT_ERROR                          (-1)
#define GRAPHICS_CALL_OK                        (0x00)
#define GRAPHICS_CALL_ERROR                     (0xFF)
#define CONTRAST_ADJ_ERROR                      (0xFF)

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************

///------------------------------------------------------------------------------------------
//! \fn sint16 lcdInit( void )
//! \brief  This function initializes the LCD hardware.  Call this function on power-up prior to
//!         attempting any graphics manipulation.  In the event of a system or board level power
//!         reset, this function will need to be called again prior to attempting any graphics
//!         manipulation.  This function should be called at the component level only, included
//!         in componentInit().  There is no need to call this function at the application level.
//!
//! \return A return value of LCD_INIT_OK indicates that no errors were detected.
//!         A return value of LCD_INIT_ERROR indicates that a hardware error was detected by the
//!         low level graphics drivers.
///------------------------------------------------------------------------------------------
sint16 lcdInit( void );


///------------------------------------------------------------------------------------------
//! \fn uint8 displayOnOff( uint8 displayState )
//! \brief  This function can be used to turn the pixel display of the LCD on and off.  This
//!         function does not control power to the LCD.  It simply turns the pixel display
//!         on and off.
//! \param  displayState    Display state, 0 - off, 1 - on.
//!
//! \return The function will return GRAPHICS_CALL_OK if no errors occurred.
//!         The function will return GRAPHICS_CALL_ERROR if an error was detected.
///------------------------------------------------------------------------------------------
uint8 displayOnOff( uint8 displayState );


///------------------------------------------------------------------------------------------
//! \fn void backlightInit( void )
//! \brief  This function initializes the hardware necessary for interfacing with the backlight
//!         of the LCD.  This function should be called at the component level only, included
//!         in ComponentInit(). There is no need to call this function at the application level.
//!
//! \return
///------------------------------------------------------------------------------------------
void backlightInit( void );


///------------------------------------------------------------------------------------------
//! \fn setBacklightOnOff( uint8 backlightValue )
//! \brief  The following function turns the LCD backlight on and off
//!         When backlight is switched off, it will remember the last intensity value,
//!         when it will be back on, it will revert to last intensity level.
//! \param  backlightValue value of BACKLIGHT_OFF will turn the backlight completely off,
//!         A parameter value of BACKLIGHT_ON will turn the backlight on.
//! \note   Use function setBacklightIntensity to set the backlight brightness.
//!
//! \return The function will return GRAPHICS_CALL_OK if no errors occurred.
//!         The function will return GRAPHICS_CALL_ERROR if an error was detected.
///------------------------------------------------------------------------------------------
void setBacklightOnOff( uint8 backlightValue );


///------------------------------------------------------------------------------------------
//! \fn bool getBacklightOnOff( void )
//! \brief  The following function returns the LCD backlight on and off status
//!
//! \return A return value of TRUE will turn the backlight on.
//!         A return value of FALSE will turn if the backlight off
///------------------------------------------------------------------------------------------
bool getBacklightOnOff( void );


///------------------------------------------------------------------------------------------
//! \fn setBacklightIntensity( uint8 backlightValue )
//! \brief  The following function turns the LCD backlight on varying levels of brightness.
//!         When backlight is switched off, it will remember the last intensity value,
//!         when it will be back on, it will revert to last intensity level.
//! \param  backlightValue Parameter values between BACKLIGHT_ON_10 and BACKLIGHT_ON_100
//!         will turn the backlight on to varying degrees of brightness (10%, 20%, and so on).
//!         A parameter value of BACKLIGHT_ON will turn the backlight on.
//!
//! \return
///------------------------------------------------------------------------------------------
void setBacklightIntensity( uint8 backlightValue );


///------------------------------------------------------------------------------------------
//! \fn uint8 getBacklightIntensity( void )
//! \brief  The following function returns the LCD backlight varying levels of brightness.
//!
//! \return Return values between BACKLIGHT_ON_10 and BACKLIGHT_ON_100 will return the backlight
//!         brightness (10%, 20%, and so on).
///------------------------------------------------------------------------------------------
uint8 getBacklightIntensity( void );


///------------------------------------------------------------------------------------------
//! \fn uint8 adjustContrast (sint8 relativeValue)
//! \brief  This function will adjust the contrast setting for the LCD.  Upon LCD initialization,
//!         the default contrast level is set to 48.  This value gives the best appearance at
//!         room temperature conditions with average loading.  The acceptable range of contrast
//!         values is 0 to +99.
//! \param  relativeValue The function takes in a parameter of either +1 or -1.
//!         A +1 increments the contrast level by 1 from its current setting.
//!         A -1 decrements the contrast level by 1 from its current setting.
//! \return The function returns either the new contrast level in a range from 0 to +99,
//!         or CONTRAST_ADJ_ERROR.  If the returned value is CONTRAST_ADJ_ERROR, an error has
//!         occurred in the function.
///------------------------------------------------------------------------------------------
uint8 adjustContrast( sint8 relativeValue );


///------------------------------------------------------------------------------------------
//! \fn uint8 setAbsoluteContrast( uint8 absoluteValue )
//! \brief  This function allows the application to set the absolute value of the LCD contrast
//!         setting in a range from 0 to +99.  Upon LCD initialization, the default contrast level
//!         is set to 48.  This value gives the best appearance at room temperature conditions
//!         with average loading.  The contrast level is very sensitive.  Under normal conditions,
//!         a value below 35 results in a white screen.  A value above 60 causes the screen to be
//!         completely dark.  The application layer may wish to remember the last contrast level
//!         set by the user and reset the contrast to that level upon power up in ApplicationInit().
//!         This function can be used for that purpose.
//! \param  The function takes in a parameter between 0 and +99.
//! \return The function returns either the new contrast level or CONTRAST_ADJ_ERROR.
//!         If the returned value is CONTRAST_ADJ_ERROR, either an error has occurred
//!         in the function, or the input parameter was outside of the valid range.
///------------------------------------------------------------------------------------------
uint8 setAbsoluteContrast( uint8 absoluteValue );


///------------------------------------------------------------------------------------------
//! \fn uint8 drawLine( uint8 xStart, uint8 yStart, uint8 xEnd, uint8 yEnd )
//! \brief  This function will draw a line defined by its start and end coordinates.
//! \param  xStart  Line point A x-coordinates
//! \param  yStart  Line point A y-coordinates
//! \param  xEnd  Line point B x-coordinates
//! \param  yEnd  Line point B y-coordinates
//! \return The function will return GRAPHICS_CALL_OK if no errors occurred.  The function will
//!         return GRAPHICS_CALL_ERROR if an error was detected.
//!
//! \note   Refer to the screen layout image above for details on the orientation of the x and
//!         y coordinates.
///------------------------------------------------------------------------------------------
uint8 drawLine( uint8 xStart, uint8 yStart, uint8 xEnd, uint8 yEnd );


///------------------------------------------------------------------------------------------
//! \fn uint8 drawRectangle (uint8 xUpperLeft, uint8 yUpperLeft,
//							uint8 xLowerRight, uint8 yLowerRight )
//! \brief  This function will draw a rectangular box whose boundaries are defined by the
//!         parameters in the function call.
//! \param  xUpperLeft  Rectangle upper left corner x-coordinates
//! \param  yUpperLeft  Rectangle upper left corner y-coordinates
//! \param  xLowerRight  Rectangle lower right corner x-coordinates
//! \param  yLowerRight  Rectangle lower right corner y-coordinates
//! \return The function will return GRAPHICS_CALL_OK if no errors occurred.
//!         The function will return GRAPHICS_CALL_ERROR if an error was detected.
//!
//! \note   Refer to the screen layout image above for details on the orientation of the x
//!         and y coordinates.
///------------------------------------------------------------------------------------------
uint8 drawRectangle (uint8 xUpperLeft, uint8 yUpperLeft, uint8 xLowerRight, uint8 yLowerRight);


///------------------------------------------------------------------------------------------
//! \fn uint8 placeBitmap( uint8 xPosition, uint8 yPosition, void * data )
//! \brief  This function will place a bitmap image of any size at any location on the LCD.
//! \param  xPosition   Upper left hand corner x-coordinate of the bitmap image will be placed.
//! \param  yPosition   Upper left hand corner y-coordinate of the bitmap image will be placed.
//! \param  data pointer to the bitmap structure.  For information on this
//!         structure, please refer to the comments in bitmaps.c and bitmaps.h.
//! \return The function returns a value of GRAPHICS_CALL_OK if no errors are detected.
//!         Otherwise, the function returns a value of GRAPHICS_CALL_ERROR.
//!
//! \note   Failing to specifically cast the variable "data" to (void*) in the function call
//!         may result in compiler warnings.
///------------------------------------------------------------------------------------------
uint8 placeBitmap( uint8 xPosition, uint8 yPosition, void * data );


///------------------------------------------------------------------------------------------
//! \fn uint8 clearArea (uint8 xUpperLeft, uint8 yUpperLeft, uint8 xLowerRight, uint8 yLowerRight)
//! \brief  This function will clear a rectangular screen area whose boundaries are defined by the
//!         parameters in the function call.
//! \param  xUpperLeft  Clear upper left corner x-coordinates
//! \param  yUpperLeft  Clear upper left corner y-coordinates
//! \param  xLowerRight  Clear lower right corner x-coordinates
//! \param  yLowerRight  Clear lower right corner y-coordinates
//! \return The function will return GRAPHICS_CALL_OK if no errors occurred.
//!         The function will return GRAPHICS_CALL_ERROR if an error was detected.
//!
//! \note   Refer to the screen layout image above for details on the orientation
//!         of the x and y coordinates.
///------------------------------------------------------------------------------------------
uint8 clearArea( uint8 xUpperLeft, uint8 yUpperLeft, uint8 xLowerRight, uint8 yLowerRight );


///------------------------------------------------------------------------------------------
//! \fn uint8 ditherArea (uint8 xUpperLeft, uint8 yUpperLeft,
//						uint8 xLowerRight, uint8 yLowerRight)
//! \brief  This function will dither a rectangular screen area whose boundaries are defined by the
//!         parameters in the function call.  
//! \param  xUpperLeft  Dither upper left corner x-coordinates
//! \param  yUpperLeft  Dither upper left corner y-coordinates
//! \param  xLowerRight  Dither lower right corner x-coordinates
//! \param  yLowerRight  Dither lower right corner y-coordinates
//! \return The function will return GRAPHICS_CALL_OK if no errors occurred.  
//!         The function will return GRAPHICS_CALL_ERROR if an error was detected.
//!
//! \note   Refer to the screen layout image above for details on the orientation
//!         of the x andy coordinates.
///------------------------------------------------------------------------------------------
uint8 ditherArea( uint8 xUpperLeft, uint8 yUpperLeft, uint8 xLowerRight, uint8 yLowerRight );


///------------------------------------------------------------------------------------------
//! \fn uint8 invertArea (uint8 xUpperLeft, uint8 yUpperLeft,
//						 uint8 xLowerRight, uint8 yLowerRight)
//! \brief  This function will invert a rectangular screen area, black -> white and white -> black,
//!         whose boundaries are defined by the parameters in the function call.
//! \param  xUpperLeft  Invert upper left corner x-coordinates
//! \param  yUpperLeft  Invert upper left corner y-coordinates
//! \param  xLowerRight  Invert lower right corner x-coordinates
//! \param  yLowerRight  Invert lower right corner y-coordinates
//! \return The function will return GRAPHICS_CALL_OK if no errors occurred.  The function will
//!         return GRAPHICS_CALL_ERROR if an error was detected.
//!
//! \note   Refer to the screen layout image above for details on the orientation of the x and
//!         y coordinates.
///------------------------------------------------------------------------------------------
uint8 invertArea( uint8 xUpperLeft, uint8 yUpperLeft, uint8 xLowerRight, uint8 yLowerRight );


///------------------------------------------------------------------------------------------
//! \fn uint8 fillArea( uint8 xUpperLeft, uint8 yUpperLeft, uint8 xLowerRight,
//					   uint8 yLowerRight, uint16 fillPattern )
//! \brief  This function will fill a rectangular screen area whose boundaries are defined by the
//!         parameters in the function call with the pattern defined by the final parameter in
//!         the function call.  Pre-defined fill patterns are available as defined above.
//!         Other fill patterns can be created if necessary.  The LSB of the fillPattern word is
//!         used on even pixel lines.  The MSB of the fillPattern word is used on odd pixel lines.
//!         A '1' represents a dark pixel, and a '0' represents a light pixel.
//! \param  xUpperLeft  Fill area upper left corner x-coordinates
//! \param  yUpperLeft  Fill area upper left corner y-coordinates
//! \param  xLowerRight  Fill area lower right corner x-coordinates
//! \param  yLowerRight  Fill area lower right corner y-coordinates
//! \param  fillPattern Pre-defined fill pattern
//! \return The function will  return GRAPHICS_CALL_OK if no errors occurred.
//!         The function will return GRAPHICS_CALL_ERROR if an error was detected.
//!
//! \note   Refer to the screen layout image above for details on the orientation
//!         of the x and y coordinates.
///------------------------------------------------------------------------------------------
uint8 fillArea( uint8 xUpperLeft, uint8 yUpperLeft, uint8 xLowerRight,
				uint8 yLowerRight, uint16 fillPattern );


///------------------------------------------------------------------------------------------
//! \fn void GFX_dumpBMP (void)
//! \brief  This function will dump a bitmap image file to the CCA Debug Portal.
//!
//! \return
///------------------------------------------------------------------------------------------
void GFX_dumpBMP( void );

#endif	//! GRAPHICS_INTERFACE_H

//@}
