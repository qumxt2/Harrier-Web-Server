/************************* ggetfsym.c *******************************

   Get a pointer to symbol in a font identified by the c character.
   If font = NULL the current viewport font is used.
   If a code page is used with the font then codepage lookup is done.

   PGSYMBOL ggetfsym(SGUCHAR c, PGFONT fp) is mapped to this function.

   Creation date: 10-10-04

   Revision date:     16-11-07
   Revision Purpose:  Vfont adaption

   Revision date:
   Revision Purpose:

   Version number: 2.1
   Copyright (c) RAMTEX Engineering Aps 2004-2008

*********************************************************************/
#include <gi_disp.h> /* gLCD prototypes */

#ifdef GSOFT_FONTS
/* Assosiate a (wide) character with a symbol in a font */
PGSYMBOL ggetfsymw(GWCHAR c, PGFONT fp)
   {
   PGCODEPAGE cp;
   if (fp == NULL)
      {
      fp = gcurvp->pfont;     /* Current font */
      cp = gcurvp->codepagep; /* Use font codepage or overrided codepage */
      #ifndef GHW_NO_HDW_FONT
      if( fp == NULL )
         return (PGSYMBOL) NULL;  /* Current fon is SYSFONT, It is a hardware font, so No symbol can be found */
      #endif
      }
   else
      cp = (PGCODEPAGE) NULL; /* Use font codepage if any */
   #ifdef GVIRTUAL_FONTS
   if (gisfontv(fp))
      {
      static GSYMHEADV vsymbol;
      PGSYMBOL psym;
      psym = gi_getsymbol(c, fp, cp);
      if (psym == NULL)
         return NULL;
      /* Take local copy of virtual symbol header so
         gputsym output can be mixed with gputs output for virtual fonts */
      vsymbol.id0 = psym->vsh.id0;
      vsymbol.id1 = psym->vsh.id1;
      vsymbol.type_id = psym->vsh.type_id;
      vsymbol.numbits = psym->vsh.numbits;
      vsymbol.cxpix = psym->vsh.cxpix;
      vsymbol.cypix = psym->vsh.cypix;
      vsymbol.bidx = psym->vsh.bidx;
      vsymbol.device_id = gvfdevice(fp);
      return (PGSYMBOL)(&vsymbol);
      }
   #endif
   return gi_getsymbol(c, fp, cp);
   }
#endif

