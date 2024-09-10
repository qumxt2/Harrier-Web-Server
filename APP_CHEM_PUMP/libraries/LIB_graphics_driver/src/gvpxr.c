/************************* gvpxr.c ********************************

   Creation date: 24-04-2004

   Revision date:
   Revision Purpose:

   Version number: 1.0
   Copyright (c) RAMTEX Engineering Aps 2004

*********************************************************************/

#include <gi_disp.h> /* gLCD prototypes */

#ifdef GVIEWPORT

/********************************************************************
   Segment: View-port
   Level: View-port
   Returns the right edge coordinate of the specified view-port
   GNUMVP in gdispcfg.h must
   be set to number of VP's to support.
*/
GXT gvpxr( SGUCHAR vp )
   {
   GCHECKVP( vp );
   return gdata.viewports[vp].rb.x;
   }

#endif

