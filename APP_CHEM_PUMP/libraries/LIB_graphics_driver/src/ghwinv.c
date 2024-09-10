/************************** ghwinv.c *****************************

   Invert box area

   The box area may have any pixel boundary.

   ---------

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
   Copyright (c) RAMTEX Engineering Aps 2002

*********************************************************************/
#include <gdisphw.h>   /* HW driver prototypes and types */
#include <st7528.h>    /* ST7528 controller specific definements */

#if (!defined( GNOCURSOR ) && defined (GSOFT_FONTS )) || defined (GGRAPHICS)

void ghw_invert(GXT ltx, GYT lty, GXT rbx, GYT rby)
   {
   GXT x;
   #ifndef GBUFFER
   GXT xc;
   GYT yc;
   #else
   GBUF_CHECK();
   #endif

   glcd_err = 0;

   /* Force reasonable values */
   GLIMITU(ltx,GDISPW-1);
   GLIMITU(lty,GDISPH-1);
   GLIMITD(rby,lty);
   GLIMITU(rby,GDISPH-1);
   GLIMITD(rbx,ltx);
   GLIMITU(rbx,GDISPW-1);

   #ifdef GBUFFER
   invalrect( ltx, lty );
   invalrect( rbx, rby );
   #endif

   /* Main y loop */
   #ifdef GBUFFER
   /* Main y loop */
   for (;lty <= rby; lty++)
      {
      /* Main x loop */
      for (x = ltx; x <= rbx; x++)
         {
   #else
   /* Main y loop */
   for (; lty <= rby;)
      {
      /* Main x loop */
      yc = lty;
      for (x = ltx, xc = 0; x <= rbx; x++)
         {
         lty = yc;
         if (xc == 0)
            {
            /* Read to tmp buffer (and init ghw_xbase) */
            ghw_rdbuf(&ghw_tmpb[0],x, yc, GMIN(x + (TMPBUFPIXSIZE-1),rbx));
            }
         /* Byte column loop */
         do
            {
            #endif

            GCOLOR col = ghw_rdbuf_color(x,lty);
            /* Swap back ground / foreground colors, leave rest unchanged */
            if (col == ghw_def_foreground)
               ghw_wrbuf_color(x,lty,ghw_def_background);
            else if (col == ghw_def_background)
               ghw_wrbuf_color(x,lty,ghw_def_foreground);

            #ifndef GBUFFER
            lty++;
            }
         while ((lty <= rby) && ((lty & 0x7) != 0));

         if (((++xc % TMPBUFPIXSIZE) == 0) || (x == rbx))
            /* write buffer (with x start at ghw_xbase set by ghw_rdbuf) */
            {
            ghw_wrbuf(&ghw_tmpb[0], ghw_xbase, yc, x, 0xff);
            xc = 0;
            }
         #endif
         }
      }
   }

#endif /* GBASIC_TEXT */

