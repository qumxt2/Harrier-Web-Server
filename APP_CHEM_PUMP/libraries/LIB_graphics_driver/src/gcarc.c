/************************* gcarc.c ********************************

   Draw corner arc (a 90 degree arc aligned to x,y axis)
   Drawing possiblities:
     GLINE  Perimeter line with foreground color (default)
     GFILL  Inner area filled with background info
     GFRAME Combination of above
  The arc orinetation is selected with these parameters
     GCARC_LT   left-top corner
     GCARC_LB   left-bottom corner
     GCARC_RT   right top corner 
     GCARC_RB   right bottom corner.
  One or more arc orientations is drawn in one call.

   Creation date: 04-12-2008

   Version number: 1.0
   Copyright (c) RAMTEX Engineering Aps 2008

*********************************************************************/
#include <gi_disp.h>

#ifdef GGRAPHICS

/*
*/
void gcarc(GXT xc, GYT yc, GXYT r, SGUCHAR arctype)
   {
   /* Check and limit to prevent negative coordinates */
   if (((arctype & (GCARC_LT|GCARC_LB))!= 0) && ( r > xc))
      {
      r=xc;
      G_WARNING( "gcarc: radius turncated" );
      }
   if (((arctype & (GCARC_LT|GCARC_RT))!= 0) && ( r > yc))
      {
      r=yc;
      G_WARNING( "gcarc: radius turncated" );
      }

   xc += gcurvp->lt.x;
   yc += gcurvp->lt.y;

   /* Limit center to be within viewport */
   LIMITTOVP( "gcarc",xc,yc,xc,yc );

   /* Check and limit radius to prevent viewport overflow */
   if (((arctype & (GCARC_RT|GCARC_RB))!= 0)&&(xc+r > gcurvp->rb.x))
      {
      r = gcurvp->rb.x-xc;
      G_WARNING( "gcarc: radius turncated" );
      }
   if (((arctype & (GCARC_LB|GCARC_RB))!= 0)&& (yc+r > gcurvp->rb.y))
      {
      r = gcurvp->rb.y-yc;
      G_WARNING( "gcarc: radius turncated" );
      }

   if( r == 0)
      return;

   gi_carc(xc, yc, r, arctype);
   ghw_updatehw();
   }

#ifdef GFUNC_VP
void gcarc_vp(SGUCHAR vp, GXT xc, GYT yc, GXYT r, SGUCHAR arcflags)
   {
   GSETFUNCVP(vp, gcarc( xc, yc, r, arcflags));
   }
#endif /* GFUNC_VP */

/**************************************************
   Internal function (also used by groundrect)
*/
/* Convenient short hand macros for image coordinate mirroring */
#define  ADD(xy1,xy2) (((GXYT)(xy1))+((GXYT)(xy2)))
#define  SUB(xy1,xy2) (((GXYT)(xy1))-((GXYT)(xy2)))
#define  MIRX( e1, e2) ((mx!= 0) ? SUB(e1,e2) : ADD(e1,e2))
#define  MIRY( e1, e2) ((my!= 0) ? SUB(e1,e2) : ADD(e1,e2))
#define  SWPX( e1, e2) ((mx!= 0) ? (e1) : (e2))


void gi_carc(GXT xc, GYT yc, GXYT r, SGUCHAR arctype)
   {
   GXYT x,x1;
   GXYT y,y1;
   SGUCHAR mx,my,fill,line;
   GCOLOR colf,coll;
   float p;

   line = ((arctype & 0x1)!=0) ? 0 : 1; /* GLINE or GFRAME */
   fill = ((arctype & 0x2)!=0) ? 1 : 0; /* GFILL or GFRAME */
   coll = G_IS_INVERSE() ? ghw_def_background : ghw_def_foreground;
   colf = G_IS_INVERSE() ? ghw_def_foreground : ghw_def_background;

   /* Process one or more arc types */
   for(;;)
      {
      if ((arctype & GCARC_LT)!=0)
         {
         arctype &= ~GCARC_LT;
         my=1;
         mx=1;
         }
      else
      if ((arctype & GCARC_LB)!=0)
         {
         arctype &= ~GCARC_LB;
         my=0;
         mx=1;
         }
      else
      if ((arctype & GCARC_RT)!=0)
         {
         arctype &= ~GCARC_RT;
         my=1;
         mx=0;
         }
      else
      if ((arctype & GCARC_RB)!=0)
         {
         arctype &= ~GCARC_RB;
         my=0;
         mx=0;
         }
      else
         return;

      /* Start on new arc */
      x = 0;
      y = r;
      p = (float)(1 - (SGINT)r);
      x1 = x;
      y1 = y;

      /* Calculate a 45 degree angle, and mirror the rest */
      for(;;)
         {
         if (line)
            {
            if (y != x)
               {
               ghw_setpixel((GXT)MIRX(xc,x),(GYT)MIRY(yc,y),coll);
               if (fill && (y != y1))
                  {
                  /* Fill using horizontal lines, so check on vertical move */
                  ghw_rectangle((GXT)SWPX(SUB(xc,x-1),xc),
                                (GYT)MIRY(yc,y),
                                (GXT)SWPX(xc,ADD(xc,x-1)),
                                (GYT)MIRY(yc,y),colf);
                  y1=y;
                  }
               }
            ghw_setpixel((GXT)MIRX(xc,y),(GYT)MIRY(yc,x), coll);
            if (fill)
               ghw_rectangle((GXT)SWPX(SUB(xc,y-1),xc),
                             (GYT)MIRY(yc,x),
                             (GXT)SWPX(xc,ADD(xc,y-1)),
                             (GYT)MIRY(yc,x),colf);
            if (x+1 >= y)
               break;
            }
         else
            {
            ghw_rectangle((GXT)SWPX(SUB(xc,y),xc),
                          (GYT)MIRY(yc,x),
                          (GXT)SWPX(xc,ADD(xc,y)),
                          (GYT)MIRY(yc,x),colf);
            if (y != y1)
               {
               finalize:
               ghw_rectangle((GXT)SWPX(SUB(xc,x1),xc),
                          (GYT)MIRY(yc,y1),
                          (GXT)SWPX(xc,ADD(xc,x1)),
                          (GYT)MIRY(yc,y1),colf);
               y1=y;
               }
            x1=x;
            if (x+1 >= y)
               {
               if (x!=y)
                  {
                  x=y;
                  goto finalize;
                  }
               break;
               }
            }

         /* Calculate next arc point */
         if( p < 0.0 )
            {
            x = x + 1;
            p = p + (float) ((SGLONG)2 * x + 1);
            }
         else
            {
            x = x + 1;
            y = y - 1;
            p = p + (float)((SGLONG)2*((SGINT)x - (SGINT)y) + 1);
            }
         }
      }
   }

#endif /* GGRAPHICS */

