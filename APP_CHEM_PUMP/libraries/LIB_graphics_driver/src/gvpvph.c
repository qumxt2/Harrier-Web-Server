/************************* gvpvph.c ********************************

   Creation date: 980220

   Revision date:     13-08-04
   Revision Purpose:  Named viewport function _vp added

   Revision date:
   Revision Purpose:

   Version number: 2.0
   Copyright (c) RAMTEX Engineering Aps 1998-2004

*********************************************************************/

#include <gi_disp.h> /* gLCD prototypes */

#ifdef GVIEWPORT

/********************************************************************
   Segment: Viewport
   Level: Viewport
   return viewport height
*/
GYT ggetvph(void)
   {
   gi_datacheck(); /* check internal data for errors */
   return (gcurvp->rb.y - gcurvp->lt.y) + 1;
   }

GXT ggetvph_vp(SGUCHAR vp)
   {
   GCHECKVP( vp );
   return gdata.viewports[vp].rb.y - gdata.viewports[vp].lt.y + 1;
   }

#endif

