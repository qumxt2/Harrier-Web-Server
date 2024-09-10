/************************** ghwretgl.c *****************************

   Low-level function for drawing rectangles or straight vertical or
   horizontal lines.

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
   Revision date:     061005
   Revision Purpose:  GBUFFER mode speed optimized
   Revision date:
   Revision Purpose:

   Version number: 1.1
   Copyright (c) RAMTEX Engineering Aps 2005

*********************************************************************/

#include <gdisphw.h>   /* HW driver prototypes and types */
#include <st7528.h>    /* ST7528 controller specific definements */

#ifdef GGRAPHICS

extern SGBOOL ghw_upddelay;

/*
   Draw vertical line
*/
static void ghw_linev(GXT xb, GYT yb, GYT ye, GCOLOR color)
   {
   SGUCHAR msk,stpmsk;
   #ifdef GBUFFER
   invalx( xb );
   invaly( yb );
   invaly( ye );
   #endif
   msk = startmask[yb & 0x7];
   stpmsk = stopmask[ye & 0x7];
   ye &= ~(0x7);
   yb &= ~(0x7);

   for (; yb <= ye; yb+=GDISPCH)
      {
      if (yb == ye)
         msk &= stpmsk;
      ghw_wr_color_line(xb,yb,xb,msk,color);
      msk = 0xff;
      }
   #ifdef GBUFFER
   if (ghw_upddelay == 0)
      ghw_updatehw();
   #endif
   }

/*
   Provides accelerated line drawing for horizontal/vertical lines.

   If left-top and right-bottom is on a single vertical or horizontal
   line a single line is drawn.

   All coordinates are absolute coordinates.
*/
void ghw_rectangle(GXT ltx, GYT lty, GXT rbx, GYT rby, GCOLOR color)
   {
   glcd_err = 0;

   /* Force resonable values */
   GLIMITU(ltx,GDISPW-1);
   GLIMITU(lty,GDISPH-1);
   GLIMITD(rby,lty);
   GLIMITU(rby,GDISPH-1);
   GLIMITD(rbx,ltx);
   GLIMITU(rbx,GDISPW-1);

   #ifdef GBUFFER
   GBUF_CHECK();
   if (ghw_upddelay)
      {
      invalrect(ltx,lty);
      invalrect(rbx,rby);
      }
   #endif

   if (ltx != rbx)
      {
      if ((lty & ~(0x7)) == (rby & ~(0x7)))
         /* The two horizontal lines are within the same page row */
         {
         ghw_wr_color_line(ltx,lty,rbx,pixymsk[lty & 0x7] | pixymsk[rby & 0x7],color);
         #ifdef GBUFFER
         if (ghw_upddelay == 0)
            {
            invalx( ltx );
            invalx( rbx );
            invaly( lty );
            ghw_updatehw();
            }
         #endif
         }
      else
         {
         ghw_wr_color_line(ltx,lty,rbx,pixymsk[lty & 0x7],color);
         #ifdef GBUFFER
         if (ghw_upddelay == 0)
            {
            invalx( ltx );
            invalx( rbx );
            invaly( lty );
            ghw_updatehw();
            }
         #endif
         }
      }

   if (lty != rby)
      {
      ghw_linev(ltx, lty, rby, color );      /* Draw vertical line */
      if (ltx != rbx)
         {                                   /* It is box coordinates */
         if ((lty & ~(0x7)) != (rby & ~(0x7)))
            {
            /* The two horizontal lines are not within the same page row */
            ghw_wr_color_line(ltx,rby,rbx,pixymsk[rby & 0x7],color);
            #ifdef GBUFFER
            if (ghw_upddelay == 0)
               {
               invalx( ltx );
               invalx( rbx );
               invaly( rby );
               ghw_updatehw();
               }
            #endif
            }
         ghw_linev(rbx, lty, rby, color );   /* Draw right vertical line */
         }
      }
   }
#endif /* GGRAPHICS */

