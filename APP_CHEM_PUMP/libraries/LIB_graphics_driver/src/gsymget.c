/************************* gsymget.c *******************************

   Creation date: 980223

   Revision date:     03-01-26
   Revision Purpose:  Bit mask on GINVERSE mode

   Revision date:     13-08-04
   Revision Purpose:  Named viewport function _vp added

   Revision date:     7-01-05
   Revision Purpose:  Pointer changed from PGSYMBOL to a GSYMBOL* type
                      (PGSYMBOL is now equal to 'const GSYMBOL* ')

   Revision date:     05-04-05
   Revision Purpose:  B&W or 2 color symbol mode now respect GINVERSE settings

   Revision date:     270405
   Revision Purpose:  The bw parameter to ghw_rdrsym(..) changed to SGUINT
                      to accomondate all combinations of display size and
                      pixel resolutions.

   Revision date:     290707
   Revision Purpose:  bw calculation corrected for odd pixel widths > 8

   Revision date:     16-11-07
   Revision Purpose:  Vfont adaption

   Version number: 2.6
   Copyright (c) RAMTEX Engineering Aps 1998-2007

*********************************************************************/
#include <gi_disp.h> /* gLCD prototypes */

#ifdef GSOFT_SYMBOLS

/*******************************************************************
   Segment: Software symbols
   Level: Graphics
   Get box area into symbol buffer.
   The area defined by the input parameters is read from
   the display into sym. The area allocated for symbol must
   be large enough to hold all the data + the GSYMHEAD struct.
   The GSYMHEAD is updated to reflect the actual size
   (which may have been limited by view-port settings or the display size)
   Display information fetched with ggetsym can be written back with
   gfillsym or gputsym.
   See also gsymsize()
*/
#ifndef GHW_NO_LCD_READ_SUPPORT
void ggetsym(GXT xs, GYT ys, GXT xe, GYT ye, GSYMBOL *psym, GBUFINT size )
   {
   SGUINT bw;  /* Byte width, May range from sym width/8 to sym width*3 */
   GYT h;
   gi_datacheck(); /* check internal data for errors */
   /* normalize to view-port */
   xs += gcurvp->lt.x;
   ys += gcurvp->lt.y;
   xe += gcurvp->lt.x;
   ye += gcurvp->lt.y;

   /* limit values to view-port */
   LIMITTOVP( "ggetsym",xs,ys,xe,ye );
   if( psym == NULL )
      {
      G_WARNING( "ggetsym: parameter, psym == NULL" );
      return;
      }

   glcd_err = 0; /* Reset HW error flag */
   h = (ye-ys)+1;

   #if (GDISPPIXW < 8)
   bw = (((SGUINT)xe)*GDISPPIXW)/GDISPCW - (((SGUINT)xs)*GDISPPIXW)/GDISPCW + 1;
   #else
   bw = (((SGUINT)(xe-xs)) + 1)*((GDISPPIXW+(GDISPCW-1))/GDISPCW);
   #endif

   #if (GDISPPIXW > 1)

   if( size < (GBUFINT)(((GBUFINT)bw * h) + (GBUFINT) sizeof(GCSYMHEAD)/* size of buffer */))
      {
      G_WARNING( "ggetsym: parameter, buffer too small" );
      return;
      }
   psym->csh.colorid = 0;
   psym->csh.pbits = GDISPPIXW;
   psym->csh.cypix = h;
   psym->csh.cxpix = (xe-xs)+1;

   ghw_rdsym(xs, ys, xe, ye, (SGUCHAR*)&(((GCSYMBOL*)psym)->b), bw, GDISPPIXW );

   #else

   if( size < (GBUFINT)( ((GBUFINT)bw)*h + ((GBUFINT)sizeof(GSYMHEAD))/* size of buffer */) )
      {
      G_WARNING( "ggetsym: parameter, buffer too small" );
      return;
      }
   /* save w,h */
   psym->sh.cypix = h;
   psym->sh.cxpix = (xe-xs)+1;
   ghw_rdsym(xs, ys, xe, ye, (SGUCHAR*)&(((GBWSYMBOL *)psym)->b), bw, (SGUCHAR) (G_IS_INVERSE() ? 0x80 : 0));

   #endif
   }

#endif /* GHW_NO_LCD_READ_SUPPORT */

/********************************************************************
   Segment: Software symbols
   Level: Graphics
   Returns the minimum size to allocate to hold a symbol.
*/
GBUFINT gsymsize(GXT xs, GYT ys, GXT xe, GYT ye )
   {
   GXT w;
   GYT h;
   GBUFINT bw;

   gi_datacheck(); /* check internal data for errors */
   /* normalize to view-port */
   xs += gcurvp->lt.x;
   ys += gcurvp->lt.y;
   xe += gcurvp->lt.x;
   ye += gcurvp->lt.y;

   /* limit values to view-port */
   LIMITTOVP( "gsymsize",xs,ys,xe,ye );

   w = (xe-xs)+1;
   h = (ye-ys)+1;

   #if (GDISPPIXW > 1)
   bw = ((GBUFINT)w*GDISPPIXW+7)/8; /* width of char in bytes */
   return (GBUFINT) ((GBUFINT)bw * h) + sizeof(GCSYMHEAD);
   #else
   bw = (w+7)/8;           /* width of char in bytes */
   return (GBUFINT) ((GBUFINT)bw * h) + sizeof(GSYMHEAD);
   #endif
   }

#ifdef GFUNC_VP

#ifndef GHW_NO_LCD_READ_SUPPORT
void ggetsym_vp( SGUCHAR vp, GXT xs, GYT ys, GXT xe, GYT ye, GSYMBOL * psym, GBUFINT size )
   {
   GSETFUNCVP(vp, ggetsym(xs,ys,xe,ye,psym,size) );
   }
#endif /* GHW_NO_LCD_READ_SUPPORT */

GBUFINT  gsymsize_vp( SGUCHAR vp, GXT xs, GYT ys, GXT xe, GYT ye )
   {
   GBUFINT retp;
   GGETFUNCVP(vp, gsymsize(xs,ys,xe,ye) );
   return retp;
   }

#endif /* GFUNC_VP */

#endif /* GSOFT_SYMBOLS */

