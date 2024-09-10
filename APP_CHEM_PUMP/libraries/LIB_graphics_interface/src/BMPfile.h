// graphics_interface.h
 
// Copyright 2011
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file provides structure typedefs for the bitmap (.bmp) file format

#ifndef BMPFILE_H
#define BMPFILE_H

#include "typedef.h"


#define BF_TYPE 0x4D42				/* "MB" (order is reversed because of little-endian) */
/**** Constants for the biCompression field... ****/
#  define BI_RGB       0			/* No compression - straight BGR data */
#  define BI_RLE8      1			/* 8-bit run-length compression */
#  define BI_RLE4      2			/* 4-bit run-length compression */
#  define BI_BITFIELDS 3			/* RGB bitmap with RGB masks */

/*
 * Bitmap file data structures 
 */

#pragma pack(1)
typedef struct {
	uint16 bfType;					/* Magic number for file */
	uint32 bfSize;					/* Size of file */
	uint16 bfReserved1;				/* Reserved */
	uint16 bfReserved2;				/* ... */
	uint32 bfOffBits;				/* Offset to bitmap data */
} BMPFILEHEADER;

/**** BMP file info structure ****/
typedef struct {
	uint32 biSize;					/* Size of info header */
	sint32 biWidth;					/* Width of image */
	sint32 biHeight;				/* Height of image */
	uint16 biPlanes;				/* Number of color planes */
	uint16 biBitCount;				/* Number of bits per pixel */
	uint32 biCompression;			/* Type of compression to use */
	uint32 biSizeImage;				/* Size of image data */
	sint32 biXPelsPerMeter;			/* X pixels per meter */
	sint32 biYPelsPerMeter;			/* Y pixels per meter */
	uint32 biClrUsed;				/* Number of colors used */
	uint32 biClrImportant;			/* Number of important colors */
} BMPINFOHEADER;


/**** Colormap entry structure ****/
typedef struct {
	uint8 rgbBlue;					/* Blue value */
	uint8 rgbGreen;					/* Green value */
	uint8 rgbRed;					/* Red value */
	uint8 rgbReserved;				/* Reserved */
} RGBQUAD;
#pragma pack()

#endif /* BMPFILE_H */
