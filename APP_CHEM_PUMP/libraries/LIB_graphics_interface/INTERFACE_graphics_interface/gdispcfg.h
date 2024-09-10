#ifndef GDISPCFG_H
#define GDISPCFG_H
/******************* gdispcfg.h *****************************

   CONFIGURATION FILE FOR THE GRAPHIC LCD LIBRARY
   This header file define the driver feature set used by your
   application and the abstract LCD hardware properties.

   As a programmer you set these definements in order to adjust
   the LCD driver code to the needs of your application.
   In many cases the definements in this file is used to remove,
   modify or replace sections of the underlying library source code.

   This header is included by the Graphic LCD library files.
   It should normally not be included by the user application.

   Revision data:    080808
   Revision Purpose: Configuration header simplified
                     Virtual font support added.

   Version number: 1.1
   Copyright (c) RAMTEX Engineering Aps 2004-2008

************************************************************/

/* Size of display module in pixels */
#define GDISPW  160     /* Width */
#define GDISPH  100     /* Height */

/* LCD controller type (selects initialization for internal video RAM) */
/* #define GHW_ST7528_MODE01 */ /* 128x128 segment output (4-level gray scale) */
/* #define GHW_ST7528_MODE2  */ /* 132x128 segment output (16-level gray scale) */
   #define GHW_ST7528_MODE3     /* 160x100 segment output (16-level gray scale) */

/* Define number of view-ports supported,
   See the function SGUCHAR gselvp( SGUCHAR s );
   At least 1 view-port must be defined */
#define GNUMVP 5

/* Feature optimization compilation keywords */
#define GBASIC_INIT_ERR      /* Enable Basic initalization and error handling */
#define GVIEWPORT            /* Enable high-level (viewport) functions */
#define GGRAPHICS            /* Enable Graphics */
#define GSOFT_SYMBOLS        /* Enable Software symbols */
#define GSOFT_FONTS          /* Enable Soft fonts */
/*#define GVIRTUAL_FONTS*/   /* Enable virtual font support */
#define GBASIC_TEXT          /* Enable Basic text */
#define GS_ALIGN             /* Enable extended string alignment */
/*#define GMULTIBYTE */      /* Enable multibyte support */
/*#define GMULTIBYTE_UTF8 */ /* Enable UTF-8 multibyte support */
/*#define GWIDECHAR */       /* Enable wide-char support */
/*#define GFUNC_VP */        /* Enable named viewport functions xxx_vp()*/
/*#define GSCREENS*/         /* Enable screens */
/*#define GEXTMODE*/         /* Enable application specific viewport data extentions */
                             /* (viewport data extensions are defined in gvpapp.h) */
#define GNOCURSOR            /* Turn visual cursor handling off or on*/
                             /* Define for max speed, undefine to have cursor support */
#define GNOTXTSPACE          /* Turn extra character or line space handling off and on */
                             /* Define for max speed, undefine to have line and character spacing */

/* Tabulator definitions */
#define GCONSTTAB            /* Tab table contain constants (undefine to use variable tabs) */

#ifdef  GCONSTTAB
 /* Define const tab increments in number of pixel */
 #define GTABSIZE  (GDISPW/6)
#else
 /* variable tab tabel is used */
 #define GMAXTABS 10           /* Max. number of tab positions (defines tabulator tabel size) */
#endif

/* Define value for switch between normal and multi-byte string chars
   If the char is above this value then this char plus the next char is
   used to form a 16 bit wide char (ex in the range 0x8000 to 0xffff) */
#define G_MULTIBYTE_LIMIT 0x80

/* Select buffered implementation (speed optimization with
   external display RAM buffer or use direct operation on
   module RAM. Define or undefine GBUFFER */
/*#define GBUFFER*/ /* Extern buffer for data manipulation, fast */

/* If GHW_ALLOCATE_BUF is defined the graphich buffer is allocated using malloc.
   instead of using a (faster) static buffer */
#ifdef GBUFFER
  /* #define GHW_ALLOCATE_BUF */ /* Allocate buffer on heap */
#endif

/* If GWARNING is defined, illegal runtime values will cause
   issue of a display message and stop of the system.
   The soft error handler function G_WARNING(str) defined in
   gdisphw.h is used for message output.
   If undefined parameters will be forced within a legal range
   and used afterwards. */
#define GWARNING

/* If GERROR is defined, states and situations which may result
   in a fatal runtime state will cause a display message to be
   issued and the system stopped. The soft error handler function
   G_ERROR(str) defined in gdisphw.h is used for message output.
   If undefined the situation is ignored or an exit is performed. */
#define GERROR

/* If GDATACHECK is defined the internal data is checked
   for errors. Maybe some faulty part of the main code overwrites
   the internal data of the LCD driver, such and error will be
   catched with this define. Undefine for max speed. */
#define GDATACHECK

/* Variable used for X and W */
#if (GDISPW <= 255)
#define GXT   unsigned char
#else
#define GXT   unsigned short
#endif
/* Variable used for Y and H */
#if (GDISPH <= 255)
#define GYT   unsigned char
#else
#define GYT   unsigned short
#endif

#ifdef GHW_PCSIM
   /* Simulator mode only switches */
   /* Define to minimize a console application when the LCD simulator is used */
   #define GHW_MINIMIZE_CONSOLE
   /* Define to limit simulator updates to the highlevel functions.
      The simulator operations is faster when defined */
   #define GHW_FAST_SIM_UPDATE
#endif

#ifndef GHW_PCSIM
   /* Memory type qualifiers only for target compilation mode (dont care in PC simulation mode) */
   /* The following definements allow you to optimize the parameter passing and to use target compiler specific
      memory allocation keywords (default will be used for PC simulator mode) */

   /* Keyword used for fast variables optimization (critical params)*/
   #define GFAST  /* nothing */
   /* Keyword used for very fast variables optimization (critical params)*/
   #define GVFAST /* nothing */
   /* Type qualifier used on pointer (parameter) to strings when the object
      is not modified by the function (part of C strong proto typing */
   #define GCONSTP const
   /* type qualifier used for fixed data (graphic tables etc) */
   #define GCODE  const
   /* Memory type qualifier used for fixed data (if GCODE setting is not enough) */
   #define FCODE  /* nothing */
   /* Memory type qualifier used for pointer to fixed data (if GCODE var * is not enough) */
   #define PFCODE /* nothing */
   /* Keyword used for generic pointers to data strings, if generic pointers is not default */
   #define PGENERIC /* nothing */
#endif

/** Color buffer handling types **/

/* Define number of bits pr pixel in the graphic buffer */
#if (defined( GHW_ST7528_MODE01 ))
   #define GDISPPIXW 2
#else
   #define GDISPPIXW 4
#endif

/* Define type to hold color information for a pixel */
#define GCOLOR  SGUCHAR

/* Define integer optimized for buffer indexing and buffer size values */
#define GBUFINT SGUINT

/****************** LOW-LEVEL DRIVER CONFIGURATIONS *******************/

/* Adapt library to scan layout selected by LCD module vendor */
#define GHW_MIRROR_VER  /* Mirror the display vertically */
/*#define GHW_MIRROR_HOR  */     /* Mirror the display horizontally */
#define GHW_XOFFSET  0       /* Set display start offset in on-chip video ram */

/* Must be defined if the LCD controller chip does not support hardware
   fonts or download font.
   When defined SYSFONT is a soft-font and code for hard font support
   in COMMON modules are optimized away.*/
#define GHW_NO_HDW_FONT

/* When defined the contrast regulation on LCD controller chip is enabled */
#define GHW_INTERNAL_CONTRAST

#define GHW_USING_VBYTE /* This controller uses vertical bytes */

/* Enable code generation for color and gray-shade support */
#define GHW_USING_COLOR
/*#define GHW_INVERTGRAPHIC_SYM*/  /* Define to accept symbols created with using 0 as black */

/****************** COLOR DEFINITION *******************/

/* Define the default colors used for text foreground and background
  G_BLACK, G_RED ,G_GREEN, G_YELLOW, G_BLUE,
  G_MAGENTA, G_CYAN, G_WHITE, G_LIGHTGRAY, or G_DARKGRAY
  Note that the number of possible colors is limited by both on the
  palette mode and the display screen type
*/
#define GHW_PALETTE_BACKGROUND  G_WHITE
#define GHW_PALETTE_FOREGROUND  G_BLACK

/*
   Map "pure color names" to palette indexes.
   These definitions should mach the default palette file (*.pal) and
   should be updated if the color ordering in the palette files is
   changed.
   If a direct match is not possible (ex because of a small palette)
   then the closest color match should be defined.
*/
#if (GDISPPIXW == 1)
   /* Basic colors (mapping to a 1 bit grey scale (b&w)) */
   #define G_BLACK       1
   #define G_RED         1
   #define G_GREEN       1
   #define G_YELLOW      0
   #define G_BLUE        1
   #define G_MAGENTA     0
   #define G_CYAN        0
   #define G_WHITE       0
   /* Extended colors */
   #define G_LLIGHTGREY  0
   #define G_LIGHTGREY   0
   #define G_GREY        0
   #define G_DARKGREY    1
   #define G_DDARKGREY   1
   #define G_ORANGE      0
   #define G_BROWN       1
   #define G_DARKGREEN   1
   #define G_LIGHTBLUE   0
#elif (GDISPPIXW == 2)
   /* Basic colors (mapping to a 2 bit grey scale) */
   #define G_BLACK       3
   #define G_RED         2
   #define G_GREEN       2
   #define G_YELLOW      1
   #define G_BLUE        2
   #define G_MAGENTA     1
   #define G_CYAN        1
   #define G_WHITE       0
   /* Extended colors */
   #define G_LLIGHTGREY  3
   #define G_LIGHTGREY   2
   #define G_GREY        2
   #define G_DARKGREY    2
   #define G_DDARKGREY   0
   #define G_ORANGE      1
   #define G_BROWN       2
   #define G_DARKGREEN   3
   #define G_LIGHTBLUE   1
#elif (GDISPPIXW == 4)
  /* Names for basic colors (can be used on all color displays) */
  #define G_BLACK       15
  #define G_RED         12
  #define G_GREEN       6
  #define G_YELLOW      2
  #define G_BLUE        9
  #define G_MAGENTA     5
  #define G_CYAN        3
  #define G_WHITE       0
  /* Names for extended colors and grey-scales*/
  #define G_LLIGHTGREY  5
  #define G_LIGHTGREY   6
  #define G_GREY        7
  #define G_DARKGREY    9
  #define G_DDARKGREY   10
  #define G_ORANGE      4
  #define G_BROWN       9
  #define G_DARKGREEN   8
  #define G_LIGHTBLUE   5
#endif

/* End of gdispcfg.h */
#endif
