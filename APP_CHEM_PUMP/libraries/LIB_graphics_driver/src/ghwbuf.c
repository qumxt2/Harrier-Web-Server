/************************** ghwbuf.c *****************************

   Low-level driver functions for the ST7528 LCD display buffer
   handling. Contains low-level functions for optimized buffer
   handling in buffered mode (GBUFFER defined) when softfonts and
   graphic is used.

   NOTE: These functions are only called by the GDISP high-level
   functions. They should not be used directly from user programs.

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

#ifdef GBASIC_INIT_ERR
/****************************************************************
 ** functions for internal implementation
 ****************************************************************/

/*
   Update HW with buffer content if buffered driver else nothing
*/
#ifdef GBUFFER

void ghw_updatehw(void)
   {
   if (ghw_upddelay)
      return;

   GBUF_CHECK();
   glcd_err = 0;

   if( irby < ilty ) return;
   if( irbx < iltx ) return;
   if( irby >= GDISPH ) irby = GDISPH-1;
   if( irbx >= GDISPW ) irbx = GDISPW-1;

   /* Mirror buffer */
   /* Loop byte rows */
   irby &= ~(0x7);
   ilty &= ~(0x7);
   for (; ilty <= irby; ilty+=GDISPCH)
      {
      ghw_wrbuf(&gbuf[GINDEX(iltx, ilty)], iltx, ilty, irbx, 0xff);
      }

   #if (defined(_WIN32) && defined(GHW_PCSIM))
   GSimFlush();
   #endif
   iltx = 1;
   ilty = 1;
   irbx = 0;
   irby = 0;
   }

/*
   Set updatehw to instant update or delayed update
     1 = Normal screen update from buffer
     0 = Update from buffer stopped until normal update is selected again

  Activated from gsetupdate(on);
*/
GUPDATE ghw_setupdate( GUPDATE update )
   {
   GUPDATE old_update;
   old_update = (GUPDATE) (ghw_upddelay == 0);
   if ((update != 0) && (ghw_upddelay != 0))
      {
      /* Update is re-activated, make a screen update. */
      ghw_upddelay = 0;  /* Flag = 0 -> no delayed update */
      ghw_updatehw();
      }
   else
      ghw_upddelay = (update == 0);
   return old_update;
   }

#endif  /* GBUFFER */

#endif /* GBASIC_INIT_ERR */


