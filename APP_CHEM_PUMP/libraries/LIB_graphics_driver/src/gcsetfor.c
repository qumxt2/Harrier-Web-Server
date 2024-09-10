/************************* Gcsetfor.c *******************************

   Creation date: 031010

   Revision date:
   Revision Purpose:

   Version number: 1
   Copyright (c) RAMTEX Engineering Aps 2003

*********************************************************************/
#include <gi_disp.h> /* gLCD prototypes */

#ifdef GVIEWPORT
#ifdef GHW_USING_COLOR

GCOLOR gsetcolorf(GCOLOR fore)
   {
   GCOLOR oldfore = gcurvp->foreground;
   #if ((GDISPPIXW != 8) && (GDISPPIXW != 16))
   GLIMITU(fore,((1<<GDISPPIXW)-1));
   #endif
   gcurvp->foreground = fore;
   ghw_setcolor(gcurvp->foreground, gcurvp->background);
   return oldfore;
   }

#ifdef GFUNC_VP

GCOLOR gsetcolorf_vp(SGUCHAR vp, GCOLOR fore)
   {
   GCOLOR retp;
   GGETFUNCVP(vp, gsetcolorf(fore) );
   return retp;
   }

#endif /* GFUNC_VP */

#endif
#endif
