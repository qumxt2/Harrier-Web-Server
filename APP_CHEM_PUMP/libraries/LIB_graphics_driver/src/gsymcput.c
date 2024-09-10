/************************* gsymcput.c ********************************

   Creation date: 980223

   Revision date:     02-01-23
   Revision Purpose:  symsize parameter added to gi_putsymbol(..)
   Revision date:     03-01-26
   Revision Purpose:  Bit mask on GINVERSE mode
   Revision date:     03-05-20
   Revision Purpose:  gi_putsymbol parameter types updated for large font support.
   Revision date:     13-08-04
   Revision Purpose:  Named viewport function _vp added
   Revision date:     18-02-07
   Revision Purpose:  GLINECUT GNOSCROLL now respected.
                      Symbol start position (lower-left symbol corner) can now
                      be closer to viewport edge, so top of symbol is skipped.
   Revision date:     31-03-08
   Revision Purpose:  Now uses same character and line spacing handling
                      as a normal text char

   Version number: 2.7
   Copyright (c) RAMTEX Engineering Aps 1998-2008

*********************************************************************/

#include <gi_disp.h> /* gLCD prototypes */

#if defined( GSOFT_SYMBOLS ) && defined( GBASIC_TEXT )

/* Define convinent short hand notations */
#define  ltx gcurvp->lt.x
#define  lty gcurvp->lt.y
#define  rbx gcurvp->rb.x
#define  rby gcurvp->rb.y
#define  cposx gcurvp->cpos.x
#define  cposy gcurvp->cpos.y

/* Support functions in gfputch.c externally referenced from this module */
void gi_clr_chsp(GYT h);
SGUCHAR gi_process_newline(GYT fh);
void gi_put_prepare(void);
void gi_put_complete(void);

/********************************************************************
   Segment: Software symbols
   Level: Fonts
   Draws a symbol in view-port at next char pos. and update char pos.
   Can be used to embed symbol in text lines.
*/
void gputcsym( PGSYMBOL psymbol )
   {
   GXT w;
   GYT h;

   if (psymbol == NULL) 
      return;

   gi_put_prepare();
   w = gsymw(psymbol);
   h = gsymh(psymbol);

   #ifndef GNOTXTSPACE
   if ((SGUINT) cposx + w + gcurvp->chln.x > (SGUINT) rbx)
   #else
   if ((SGUINT) cposx + w > (SGUINT) rbx)
   #endif
      {
      if (G_IS_NOWRAP())
         {
         if (!G_IS_PARTIAL_CHAR())
            goto skip_symbol; /* no room for symbol */
         }
      else
         {
         /* Make a character wrap in advance */
         if (gi_process_newline(h) != 0)
            goto skip_symbol; /* a no-scroll condition reached, no room for character */
         }
      }

   gi_putsymbol( cposx,
                 (GYT)((cposy+1)-h),
                 rbx, rby,
                 psymbol,
                 (GYT)(( h <= cposy-lty ) ? 0 : h - ((cposy-lty)+1)), /* yoffset*/
                 0);


   #ifndef GNOTXTSPACE
   if ((SGUINT) cposx + w + gcurvp->chln.x > (SGUINT) rbx)
   #else
   if ((SGUINT) cposx + w > (SGUINT) rbx)
   #endif
      cposx = rbx;
   else
      {
      cposx += w;
      #ifndef GNOTXTSPACE
      if (gcurvp->chln.x > 0)
         gi_clr_chsp(h); /* Clear extra char spacing if needed (spacing is assumed to be equal to symbol)*/
      #endif
      }

   skip_symbol:
   gi_put_complete();
   }

#ifdef GFUNC_VP

void gputcsym_vp( SGUCHAR vp, PGSYMBOL psymbol )
   {
   GSETFUNCVP(vp, gputcsym(psymbol) );
   }

#endif /* GFUNC_VP */

#endif /* GSOFT_SYMBOLS && GBASIC_TEXT */

