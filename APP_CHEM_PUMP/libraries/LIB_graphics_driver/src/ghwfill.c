/************************** ghwfill.c *****************************

   Fill box area with a pattern.

   The box area may have any pixel boundary, however the pattern is
   always aligned to the physical background, which makes patching
   of the background easier with when using multiple partial fills.

   The pattern word is used as a 2 character pattern.
   The LSB byte of pattern are used on even pixel lines and the MSB byte
   are used on odd pixel lines making it easy to make a "grey" bit raster
   (for instance when pat = 0x55aa or = 0xaa55)

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
   Copyright (c) RAMTEX Engineering Aps 2005

*********************************************************************/
#include <gdisphw.h>   /* HW driver prototypes and types */
#include <st7528.h>    /* ST7528 controller specific definements */

#ifdef GVIEWPORT

/*
   Prepare ghw_tmpb buffer with a fill pattern
*/
static void ghw_set_pattern(SGUINT pattern, GXT xb)
   {
   GCOLOR col1;
   GXT x,xc;
   SGUCHAR *p = &ghw_tmpb[0];

   /* Use ghw_tmpb buffer to hold fill pattern */
   if ((pattern == 0x0) || (pattern == 0xffff))
      {
      /* Uniform pixel pattern, store whole bytes */
      col1 = (pattern != 0x0) ? ghw_def_foreground : ghw_def_background;
      for (xc=0; xc<TMPBUFSIZE; xc+=GDISPPIXW)
         {
         *p++ = ((col1 & ghw_cmsk[xc & ((1<<GDISPPIXW)-1)]) != 0) ? 0xff : 0x00;
         }
      }
   else
      {
      /* Create 2 line pixel pattern */
      GCOLOR col2;
      SGUCHAR pat1 = (pattern / 256);
      SGUCHAR pat2 = (pattern & 0xff);
      SGUCHAR msk;
      msk = (1 << (7-(xb & 0x7))); /* Calculate start msk for tmp buf*/

      for (x = 0; x < 8; x++)
         {
         col1 = (pat1 & msk) ? ghw_def_foreground : ghw_def_background;
         col2 = (pat2 & msk) ? ghw_def_foreground : ghw_def_background;
         if ((msk >>= 1) == 0)
            msk = 0x80;
         for (xc = 0; xc < GDISPPIXW; xc++, p++)
            {
            *p = (col1 & ghw_cmsk[xc]) ? 0xaa : 0x00; /* Set even rows */
            if (col2 & ghw_cmsk[xc])
               *p |= 0x55;                            /* Set odd rows */
            }
         }
      }
   }

void ghw_fill(GXT ltx, GYT lty, GXT rbx, GYT rby, SGUINT pattern)
   {
   GXT x;
   SGBOOL onecolor;
   GCOLOR col;
   SGUCHAR msk, mske;
   #ifdef GBUFFER
   GXT xc;
   SGUCHAR *p;
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

   /* Prepare patterns and masks */
   if ((pattern == 0x0) || (pattern == 0xffff))
      {
      onecolor = 1;
      col = (pattern != 0x0) ? ghw_def_foreground : ghw_def_background;
      }
   else
      {
      onecolor = 0;
      ghw_set_pattern(pattern,ltx);
      }

   msk = startmask[lty & 0x7];
   mske  = stopmask[rby & 0x7];
   rby &= ~(0x7);
   lty &= ~(0x7);

   /* Main y loop */
   for (; lty <= rby; lty+=GDISPCH)
      {
      if (lty == rby)
         msk &= mske;  /* Use stop mask on last row */

      if (onecolor)
         ghw_wr_color_line(ltx, lty, rbx, msk, col);
      else
         {
         #ifdef GBUFFER
         p = &(gbuf[ GINDEX(ltx, lty) ]);
         #endif
         /* Main x loop, write tmpbuf a number of times */
         for (x = ltx; x <= rbx; )
            {
            #ifdef GBUFFER
            SGUCHAR *pt = &ghw_tmpb[(SGUINT)(x % 8)*GDISPPIXW];
            for (xc = 0; xc < GDISPPIXW; xc++, p++,pt++)
               {
               if (msk != 0xff)
                  *p = (*pt & msk) | (*p  & ~msk);
               else
                  *p = *pt;
               }
            x++;
            #else
            GXT xbe;
            xbe = GMIN(x + (TMPBUFPIXSIZE-1),rbx);
            ghw_wrbuf(&ghw_tmpb[0], x, lty, xbe, msk);
            x = xbe + 1;                /* Move to next 8 pixel columns */
            #endif
            }
         }
      msk = 0xff;
      }
   }

#endif /* GBASIC_TEXT */

