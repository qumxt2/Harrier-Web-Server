/************************** ghwgscrl.c *****************************

   Scrolls the graphics on LCD x lines up.  The empty area in the
   bottom is cleared with a pattern. The fill pattern will be aligned
   to the background independent of the used coordinate parameters.

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
/* <stdlib.h> is included via gdisphw.h */
#include <gdisphw.h>   /* HW driver prototypes and types */
#include <st7528.h>    /* ST7528 controller specific definements */

#if defined( GBASIC_TEXT ) || defined(GSOFT_FONTS) || defined(GGRAPHIC)

/*
   Scrolls the graphics on LCD x lines up.
   The empty area in the bottom is cleared

   lines  =  pixel lines to scroll
*/
void ghw_gscroll(GXT ltx, GYT lty, GXT rbx, GYT rby, GYT lines, SGUINT pattern)
   {
   GYT y,ys,ylim;
   GXT x;
   GFAST SGUCHAR msk,mske,msklim,mskc;
   SGINT shift;
   SGUINT dat;
   GCOLOR color;
   SGUCHAR *p;
   SGUCHAR *p2;
   #ifdef GBUFFER
   SGUCHAR *pd;
   GBUF_CHECK();
   #else
   GXT xc;
   #endif

   /* Select clear color */
   color = (pattern != 0) ? ghw_def_foreground : ghw_def_background;

   glcd_err = 0;

   /* Force resoanable values */
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

   if (lines > rby - lty)
      {
      ghw_fill(ltx, lty, rbx, rby, pattern);   /* clear whole area */
      return;
      }

   /* Prepare patterns and masks */
   ylim = rby - lines;
   msk = startmask[lty & 0x7];
   mske  = stopmask[rby & 0x7];
   msklim = stopmask[ylim & 0x7];

   /* Set shift value (negative = shift left) */
   shift = (SGINT)(((SGUINT)lty + lines) & 0x7) - (SGINT)(lty & 0x7);

   ys = (lty+lines) / GDISPCH; /* First source row for scroll */
   rby /= GDISPCH;
   lty /= GDISPCH;
   ylim /= GDISPCH;


   /* Main y loop */
   for (y = lty; y <= rby; y++, ys++)
      {
      if (y == rby)
         msk &= mske;

      /* Main x loop */
      for (x = ltx; x <= rbx; )
         {
         SGUCHAR GFAST val;
         #ifdef GBUFFER
         p  = &gbuf[ GINDEX(x, (GBUFINT)ys*GDISPCH) ];
         if (((shift > 0) && (ys != rby)) ||
             ((shift < 0) && (ys != lty)))
            p2 = &gbuf[GINDEX(x, ((shift > 0) ? ((GBUFINT)ys+1) : (GBUFINT)(ys-1))*GDISPCH )];
         pd = &gbuf[ GINDEX(x, (GBUFINT)y*GDISPCH) ];
         #else
         GXT xb;
         xb = x;
         /* Read to tmp buffer */
         p  = &ghw_tmpb[0];
         p2 = &ghw_tmpb2[0];
         ghw_rdbuf(p, xb, ys*GDISPCH, GMIN(xb + (TMPBUFPIXSIZE-1),rbx));
         if (((shift > 0) && (ys != rby)) ||
             ((shift < 0) && (ys != lty)))
            {
            ghw_rdbuf(p2, xb, (GYT)( ((shift > 0) ? (ys+1) : (ys-1))*GDISPCH),
                GMIN(xb + (TMPBUFPIXSIZE-1),rbx));
            }

         /* Tmp buffer x loop */
         for (xc = 0; (x <= rbx) && (xc < TMPBUFPIXSIZE); x++, xc++ )
            {
         #endif
            /* Loop color info */
            for (mskc = (1 << (GDISPPIXW-1)); mskc != 0; mskc>>=1, p++, p2++)
               {
               /* Get source data */
               if (y <= ylim)
                  {
                  if (shift != 0)
                     {
                     if (shift > 0)
                        {
                        dat = (SGUINT) (*p);
                        if ((ys != rby))
                           dat |= ((SGUINT) (*p2))*256;
                        dat >>= shift;
                        }
                     else
                        {
                        dat = (y == lty) ? 0 : (SGUINT) (*p2);
                        dat |= ((SGUINT) (*p)) *256;
                        dat <<= abs(shift);
                        dat /= 256;
                        }
                     }
                  else
                     dat = (SGUINT) (*p);

                  if (y == ylim)
                     val = (((SGUCHAR) dat) & msklim) | (((color & mskc) ? 0xff : 0x00) & ~msklim);
                  else
                     val = ((SGUCHAR) dat );
                  }
               else
                  val = ((color & mskc) ? 0xff : 0x00); /* use pattern */

         #ifdef GBUFFER
               if (msk != 0xff)
                  *pd = (*pd & ~msk) | (val & msk);
               else
                  *pd = val;
               pd++;
               }
            x++;
         #else
               *p = val; /* Store result in tmp buffer */
               }
            }
         /* Write tmp buffer (and do masking if required) */
         ghw_wrbuf(&ghw_tmpb[0], xb, y*GDISPCH, x-1, msk);
         #endif
         }
      msk = 0xff;
      }
   }
#endif /* GBASIC_TEXT */


