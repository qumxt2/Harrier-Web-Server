/************************** ghwpixel.c *****************************

   Low-level functions for graphic pixel set and clear
   Absolute coordinates are used.

   The ST7528 controller is assumed to be used with a LCD module.
   The following LCD module characteristics MUST be correctly
   defined in GDISPCFG.H:

      GDISPW  Display width in pixels
      GDISPH  Display height in pixels
      GBUFFER If defined most of the functions operates on
              a memory buffer instead of the LCD hardware.
              The memory buffer content is copied to the LCD
              display with ghw_updatehw().
              (Equal to an implementation of delayed write)

   Creation date:
   Revision date:
   Revision Purpose:

   Version number: 1.0
   Copyright (c) RAMTEX Engineering Aps 2005

*********************************************************************/

#include <gdisphw.h>   /* HW driver prototypes and types */
#include <st7528.h>    /* ST7528 controller specific definements */

#ifdef GGRAPHICS

/*
   Set pixel  == 0 = white, != 0 = black
*/
void ghw_setpixel( GXT x, GYT y, GCOLOR color )
   {
   glcd_err = 0;

   /* Force resonable values */
   GLIMITU(y,GDISPH-1);
   GLIMITU(x,GDISPW-1);

   #ifdef GBUFFER
   GBUF_CHECK();
   invalrect( x, y );
   #endif
   ghw_wr_color_line(x,y,x,pixymsk[y & 0x7],color);
   }

/*
   Get pixel  == 0 = white, != 0 = black
*/
GCOLOR ghw_getpixel(GXT x, GYT y)
   {
   glcd_err = 0;
   /* Force resonable values */
   GLIMITU(y,GDISPH-1);
   GLIMITU(x,GDISPW-1);

   #ifdef GBUFFER
   #ifdef GHW_ALLOCATE_BUF
   if (gbuf == NULL)
      {
      glcd_err = 1;
      return 0;
      }
   #endif
   #else
   ghw_rdbuf(&ghw_tmpb[0],x, y, x);
   #endif
   return ghw_rdbuf_color(x,y);
   }
#endif
