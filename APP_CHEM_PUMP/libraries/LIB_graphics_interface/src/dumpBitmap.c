//dumpBitmap.c

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// SCREEN LAYOUT
//  				x = LCD_X_MIN   -->   -->   -->   -->   -->   -->   -->   x = LCD_X_MAX
//	y = LCD_Y_MIN	-----------------------------------------------------------------
//					|																|
//			|		|																|
//			|		|																|
//			V		|																|
//					|																|
//					|						 LCD Screen Area						|
//			|		|																|
//			|		|																|
//			V		|																|
//					|																|
//	y = LCD_Y_MAX	-----------------------------------------------------------------
// 

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisp.h"						// Graphics driver library prototypes
#include "typedef.h"					// Compiler specific type definitions
#include "graphics_interface.h"			// Interface library prototypes
#include "rtos.h"
#include "BMPfile.h"

#include "ccaportal.h"
#include "debug.h"

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************
// This function will dump a 2-color bitmap.  The code would need to be modified slightly
// in order to work with other color depths.
void GFX_dumpBMP(void)
{
	
	(void) K_Task_Wait(1000);
	#if ( ((LCD_WIDTH / 8) + ((4 - ((LCD_WIDTH / 8) % 4)) & 0x03)) < 40 ) 
		// minimum size needed to hold BMPINFOHEADER
		uint8 buffer[40];
	#else
		// minimum size is determined by number of bytes needed to contain a single row
		uint8 buffer[ ((LCD_WIDTH / 8) + ((4 - ((LCD_WIDTH / 8) % 4)) & 0x03)) ];
	#endif
	uint16 x, y;
	// each row in .bmp file format must have an even multiple of 4 bytes. Any padding
	// in each row should be filled with 0.
	uint16 xByteCount = ((LCD_WIDTH / 8) + ((4 - ((LCD_WIDTH / 8) % 4)) & 0x03));

	//uint16 xByteCount = 1;//BYTES_IN_ROW_OF_PIXELS(LCD_WIDTH);	
	// Create the .bmp file header
	
	((BMPFILEHEADER *)buffer)->bfType = BF_TYPE;
	((BMPFILEHEADER *)buffer)->bfReserved1 = 0x0000;
	((BMPFILEHEADER *)buffer)->bfReserved2 = 0x0000;
	((BMPFILEHEADER *)buffer)->bfOffBits = 
		sizeof(BMPFILEHEADER) +
		sizeof(BMPINFOHEADER) + 
		(2 * sizeof(RGBQUAD));
	((BMPFILEHEADER *)buffer)->bfSize = 
		((BMPFILEHEADER *)buffer)->bfOffBits +
		( (uint32)xByteCount * LCD_HEIGHT );
	
	CcaPortalWriteByteArray( buffer, sizeof(BMPFILEHEADER) );
	(void) K_Task_Wait(1000);	
	// Create the .bmp info header	
	
	((BMPINFOHEADER *)buffer)->biSize = sizeof(BMPINFOHEADER);
	((BMPINFOHEADER *)buffer)->biWidth = LCD_WIDTH;
	((BMPINFOHEADER *)buffer)->biHeight = LCD_HEIGHT;
	((BMPINFOHEADER *)buffer)->biPlanes = 1;
	((BMPINFOHEADER *)buffer)->biBitCount = 1;
	((BMPINFOHEADER *)buffer)->biCompression = BI_RGB;
	((BMPINFOHEADER *)buffer)->biSizeImage = 
		( (uint32)xByteCount * LCD_HEIGHT );
	((BMPINFOHEADER *)buffer)->biXPelsPerMeter = 0;
	((BMPINFOHEADER *)buffer)->biYPelsPerMeter = 0;
	((BMPINFOHEADER *)buffer)->biClrUsed = 2;
	((BMPINFOHEADER *)buffer)->biClrImportant = 2;

	CcaPortalWriteByteArray( buffer, sizeof(BMPINFOHEADER) );
	(void) K_Task_Wait(1000);
	
	// Create color index 0 (black)
	((RGBQUAD *)buffer)->rgbBlue = 0x00;
	((RGBQUAD *)buffer)->rgbGreen = 0x00;
	((RGBQUAD *)buffer)->rgbRed = 0x00;
	((RGBQUAD *)buffer)->rgbReserved = 0x00;
	CcaPortalWriteByteArray( buffer, sizeof(RGBQUAD) );


	// Create color index 1 (while)
	((RGBQUAD *)buffer)->rgbBlue = 0xFF;
	((RGBQUAD *)buffer)->rgbGreen = 0xFF;
	((RGBQUAD *)buffer)->rgbRed = 0xFF;
	((RGBQUAD *)buffer)->rgbReserved = 0x00;
	CcaPortalWriteByteArray( buffer, sizeof(RGBQUAD) );
	(void) K_Task_Wait(1000);

	// Create the bitmap data
	// bitmap data is stored bottom row first, left to right.  Each row must
	// contain even multiple of 4 bytes. Any padding at end of row should be written with 0s.
	//for( y = LCD_HEIGHT; y > 0; y-- )
	for( y = 100; y > 0; y-- )
	{
		// clear out the buffer (assume everything is 0 unless proven otherwise)
		for( x = 0; x < xByteCount; x++ )
		{
			buffer[ x ] = 0x00;
		}
		// parse the row bit by bit and set corresponding bit in buffer as appropriate
		for( x = 0; x < LCD_WIDTH; x++ )
		{
			
			if( !ggetpixel( x, (y-1) ) )
			//if( x%3 )
			{
				buffer[ (x / 8) ] |= ( 1 << (7 - (x % 8)) );
			}
		}
		CcaPortalWriteByteArray( buffer, xByteCount );
	}
}
