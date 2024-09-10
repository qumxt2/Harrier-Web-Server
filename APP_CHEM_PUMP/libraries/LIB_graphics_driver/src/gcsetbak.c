/************************* Gcsetbak.c *******************************

   Creation date: 031010

   Revision date:
   Revision Purpose:

   Version number: 1
   Copyright (c) RAMTEX Engineering Aps 2003

*********************************************************************/
#include <gi_disp.h> /* gLCD prototypes */

#ifdef GVIEWPORT
#ifdef GHW_USING_COLOR

GCOLOR gsetcolorb(GCOLOR back)
   {
   GCOLOR oldback = gcurvp->background;
   #if ((GDISPPIXW != 8) && (GDISPPIXW != 16))
   GLIMITU(back,((1<<GDISPPIXW)-1));
   #endif
   gcurvp->background = back;
   ghw_setcolor(gcurvp->foreground, gcurvp->background);
   return oldback;
   }

#ifdef GFUNC_VP

GCOLOR gsetcolorb_vp(SGUCHAR vp, GCOLOR back)
   {
   GCOLOR retp;
   GGETFUNCVP(vp, gsetcolorb(back) );
   return retp;
   }

#endif /* GFUNC_VP */

#endif
#endif

