/************************** ghwsymrd.c *****************************

   Graphic symbol read functions for LCD display

   Read graphic area from the display to a GLCD buffer using the general
   symbol format.

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

#ifdef GSOFT_SYMBOLS
/*
   Copy a graphic area from the display to a buffer organized with the
   common symbol and font format.
*/
void ghw_rdsym(GXT ltx, GYT lty, GXT rbx, GYT rby, PGUCHAR dest, SGUINT bw, SGCHAR mode)
   {
   GXT yc,ybeg,ycbeg;
   SGUINT x,xc;
   SGUCHAR *p;
   #ifdef GBUFFER
   GBUF_CHECK();
   #endif

   if (dest == NULL)
      return;

   /* Force reasonable values */
   GLIMITU(ltx,GDISPW-1);
   GLIMITU(lty,GDISPH-1);
   GLIMITD(rby,lty);
   GLIMITU(rby,GDISPH-1);
   GLIMITD(rbx,ltx);
   GLIMITU(rbx,GDISPW-1);
   GLIMITD(bw,1);

   if (mode != GDISPPIXW)
      {
      /* The symbol has more color pallettes than the controller */
      G_WARNING( "ghw_rdsym: color depth not supported" );
      return;
      }

   #ifndef GBUFFER
   #endif
   /* Page y loop */
   for (yc = 0; lty <= rby; lty = (lty & (~0x7))+GDISPCH)
      {
      /* Main x loop */
      ycbeg = yc;

      /* Main x loop */
      for (xc = 0, x = ltx, ybeg = lty; x <= rbx; xc++)
         {
         yc = ycbeg;
         lty = ybeg;
         #ifdef GBUFFER
         p = &gbuf[ GINDEX(x, lty) ];
         #else
         if (((x-ltx) % TMPBUFPIXSIZE) == 0)
            {
            p = &ghw_tmpb[0];
            ghw_rdbuf(p, x, lty, (GMIN(x + (TMPBUFPIXSIZE-1),rbx)));
            }
         else
            p = &p[8]; /* Next symbol byte columns */
         #endif

         /* Loop vertical column of symbol bytes */
         for(;;)
            {
            SGUCHAR xmsk,dat,ymsk;
            SGUCHAR *pb;
            /* (restart byte row scan) */
            ymsk = pixymsk[lty & 0x7];
            pb = p;
            xmsk = 0x80;
            dat = 0;
            /* Symbol byte x row loop */
            do
               {
               if (( *pb & ymsk) != 0)
                  dat |= xmsk;
               pb++;
               }
            while ((xmsk >>= 1) != 0);
            dest[ xc + yc*bw] = dat; /* Store symbol byte */
            yc++;
            if (((lty & 0x7) == 0x7) || (lty == rby))
               break;
            lty++;
            }
         x+=8/GDISPPIXW;
         }
      }
   }

#endif

