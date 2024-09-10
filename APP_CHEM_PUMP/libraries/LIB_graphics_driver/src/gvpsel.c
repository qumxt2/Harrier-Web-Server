/************************* gvpsel.c ********************************

   Creation date: 980220

   Revision date:
   Revision Purpose:

   Version number: 2.0
   Copyright (c) RAMTEX Engineering Aps 1998-2004

*********************************************************************/

#include <gi_disp.h> /* gLCD prototypes */

#ifdef GVIEWPORT

/********************************************************************
   Segment: View-port
   Level: View-port
   Returns current view-port
   GNUMVP in gdispcfg.h must
   be set to number of VP's to support.
   After ginit() view-port 0 is set as default.
*/
SGUCHAR ggetvpnum( void )
   {
   SGUCHAR i;
   gi_datacheck(); /* check internal data for errors */
   /* find current vp index */
   for( i=0; i<GNUMVP; i++ )
      {
      if( gcurvp == &gdata.viewports[i] )
         break;
      }
   return ( i<GNUMVP ) ? i : 0;
   }

/* VP select function without returning old vp (selects viewport faster)*/
void gi_selvp( SGUCHAR vp )
   {
   if (vp < GNUMVP)
      gcurvp = &gdata.viewports[ vp ];
   else
      {
      G_WARNING("Viewport number >= GNUMVP definition, Defaults to GNUMVP-1");
      gcurvp = &gdata.viewports[ GNUMVP-1 ];
      }
   #ifdef GHW_USING_COLOR
   /* Let active colors track palette*/
   ghw_setcolor(gcurvp->foreground,gcurvp->background);
   #endif
   }

/********************************************************************
   Segment: View-port
   Level: View-port
   Selects the view-port to become current, GNUMVP in gdispcfg.h must
   be set to number of VP's to support.
   After ginit() view-port 0 is set as default.
   Returns previous current view-port
*/
SGUCHAR gselvp( SGUCHAR vp /* view-port to select */ )
   {
   SGUCHAR vpold;
   vpold = ggetvpnum();   /* Get old viewport number (and do data check) */
   gi_selvp(vp);
   return vpold;
   }

#endif

