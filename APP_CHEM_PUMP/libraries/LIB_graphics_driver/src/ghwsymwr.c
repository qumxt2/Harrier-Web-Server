/************************** ghwsymwr.c *****************************

   Graphic symbol write functions for LCD display

   Write graphic symbol buffer using the general symbol format
   to LCD display.

   The byte ordering for a symbol is horizontal byte(s) containing the
   first pixel row at the lowest address followed by the byte(s) in
   the pixel row below etc. The symbol is left aligned in the byte buffer.

   All coordinates are absolute pixel coordinate.

   ---------

   The ST7528 controller is assumed to be used with a LCD module.

   The following LCD module characteristics MUST be correctly
   defined in GDISPCFG.H:

      GDISPW  Display width in pixels
      GDISPH  Display height in pixels
      GBUFFER If defined most of the functions operates on
              a memory buffer instead of the LCD hardware.
              The memory buffer content is complied to the LCD
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
#include <st7528.h>    /* LCD controller specific definements */

#ifdef GVIRTUAL_FONTS
#include <gvfont.h>
#endif

#ifdef GSOFT_SYMBOLS

/*
   Copy a graphic area from a buffer using the common symbol and font format
   to the LCD memory or the graphic buffer
*/
void ghw_wrsym(GXT ltx, GYT lty, GXT rbx, GYT rby, PGSYMBYTE src, SGUINT bw, SGUCHAR mode)
   {
   GCOLOR fore = 0;
   GCOLOR back = 0;
   SGUCHAR spshift_start = 0;
   SGUCHAR spmask;
   SGUCHAR spshift;
   GYT y,yc,cy;
   GXT x,xc;
   SGUCHAR msk;

   #ifdef GBUFFER
   GBUF_CHECK();
   #endif

   #ifdef GVIRTUAL_FONTS
   if (bw == 0)
      return;
   #else
   if ((src == NULL) || (bw == 0))
      return;
   #endif

   /* Force reasonable values */
   GLIMITU(ltx,GDISPW-1);
   GLIMITU(lty,GDISPH-1);
   GLIMITD(rby,lty);
   GLIMITU(rby,GDISPH-1);
   GLIMITD(rbx,ltx);
   GLIMITU(rbx,GDISPW-1);
   GLIMITD(bw,1);

   #ifdef GBUFFER
   invalrect( ltx, lty );
   invalrect( rbx, rby );
   #endif

   /* Is it a color/gray scale symbol or a 'b&w' symbol ? */
   if ((mode & GHW_PALETTEMASK) != 0)
      {
      mode &= GHW_PALETTEMASK;
      if ((mode == 4) || (mode == 2))
         spshift_start = 8-mode;
      else
         {
         /* The symbol has more color pallettes than the controller */
         G_WARNING( "ghwsymw: symbol color pixel resolution not supported" );
         return;
         }
      }
   else
      {
      /* "B&W" symbol */
      if ((mode & GHW_INVERSE) == 0)
         {
         fore = ghw_def_foreground;  /* Normal 'b&w' */
         back = ghw_def_background;
         }
      else
         {
         fore = ghw_def_background;  /* Inverse 'b&w' */
         back = ghw_def_foreground;
         }
      mode = 1;
      spshift_start = 0x80;
      }

   /* initiate spmsk */
   spmask =  (1<<mode)-1; /* 1 -> 0x01, 2->0x3 4->0x0f, 8 -> 0xff */

   msk = startmask[lty & 0x7];

   /* Main y loop, symbol start from row 0 */
   for (yc = 0; lty <= rby;)
      {
      /* Main x loop */
      if ((lty & ~0x7) == (rby & ~0x7))
         msk &= stopmask[rby & 0x7];  /* Use stop mask on last row */

      #ifndef GBUFFER
      ghw_xbase = ltx; /* Set x base for tmp buffer*/
      #endif
      spshift = spshift_start;  /* Shift for normalize first pixel */

      for (x = ltx, xc = 0; x <= rbx; x++)
         {
         /* Init page loop indexes */
         y = lty;
         cy = yc;
         /* Loop vertical columns for the y page */
         do
            {
            register GCOLOR dat;
            /* Load symbol byte */
            #ifdef GVIRTUAL_FONTS
            if (src == NULL)
               /* Load new symbol byte from virtual memory */
               dat = gi_symv_by(xc+cy*bw);
            else
            #endif
               dat = src[xc+cy*bw];       /* Load symbol byte */
            if (mode == 1)
               {
               /* Convert B&W symbol to foreground / background color */
               if ((dat & spshift) != 0)
                  dat = fore;
               else
                  dat = back;
               }
            else
               {
               /* Convert and map color pixels */
               #ifdef GHW_INVERTGRAPHIC_SYM
               if (colorsym)
                  dat ^= 0xff; /* Symbols was created with 0 as black, invert */
               #endif
               dat = ((dat >> spshift) & spmask);
               if (mode != GDISPPIXW)
                  {
                  #if (GDISPPIXW == 2)
                  /* Convert from 4 to 2 bit pr pixel */
                  dat >>= 2;
                  #else
                  /* Convert from 2 to 4 bits pr pixel */
                  dat <<= 2;
                  if (dat & 0x4)  /* Exapand LSB to new lower bits */
                     dat |= 0x3;
                  else
                     dat &= ~0x3;
                  #endif
                  }
               }
            ghw_wrbuf_color(x,y++,dat);
            cy++;
            }
         while (((y & 0x7) != 0) && (y <= rby));
         /* Move to next symbol pixel column */
         if (mode == 1)
            {
            if (spshift == 1)
               {
               spshift = spshift_start;
               xc++; /* Next symbol byte column*/
               }
            else
               spshift >>= 1;
            }
         else
            {
            if (spshift == 0)
               {
               spshift = spshift_start;
               xc++; /* Next symbol byte column*/
               }
            else
               spshift -= mode;
            }
         #ifndef GBUFFER
         if ((x == rbx) || (((x-ltx)%TMPBUFPIXSIZE) == (TMPBUFPIXSIZE-1)))
            {
            /* write tmp buffer to screen */
            ghw_wrbuf(&ghw_tmpb[0], ghw_xbase, lty, x, msk);
            ghw_xbase += (x-ghw_xbase)+1;
            }
         #endif
         }
      /* Move to next page */
      lty = y;
      yc = cy;
      msk = 0xff;
      }
   }

#endif

