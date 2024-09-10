/*
   Draw circle.
   This functions uses integer coordinates so the center optionally
   can be placed outside the current viewport (and screen).

   This implementation does not use the math.h library nor the
   trigometric functions and is therefore relative fast.

   Revision date:     28-06-07
   Revision Purpose:  Drawing color corrected to actual fore_ground color

   Version number: 1.1
   Copyright (c) RAMTEX Engineering Aps 2007
*/
#include <gi_disp.h>

#ifdef GGRAPHICS

static SGINT xb,xe,yb,ye; /* Fast check coordinates to prevent illegal pointer operations */
static SGUCHAR fil;
static GCOLOR col;

#define  G_IS_INSIDE_VP( x, y ) ((((x) < xb) || ((x) > xe) || ((y) < yb) || ((y) > ye)) ? 0 : 1)

#define  G_SETPIXEL(x,y) \
   { \
   register SGUINT xa; \
   register SGUINT ya; \
   xa = (x); \
   ya = (y); \
   if (G_IS_INSIDE_VP( xa, ya )) \
   ghw_setpixel((GXT)xa,(GYT)ya,col); \
   }

#ifdef GHW_USING_VBYTE
static void setlinev( SGUINT xa, SGUINT ya1, SGUINT ya2 )
   {
   if (!((xa < xb) || (xa > xe) || (ya2 < yb) || (ya1 > ye)))
      {
      /* Some part inside vp */
      if (ya1 < yb) ya1 = yb;
      if (ya2 > ye) ya2 = ye;
      ghw_rectangle((GXT)xa,(GYT)ya1,(GXT)xa,(GYT)ya2,col);
      }
   }

#else
static void setlineh( SGUINT xa1, SGUINT xa2, SGUINT ya)
   {
   if (!((xa2 < xb) || (xa1 > xe) || (ya < yb) || (ya > ye)))
      {
      /* Some part inside vp */
      if (xa1 < xb) xa1 = xb;
      if (xa2 > xe) xa2 = xe;
      ghw_rectangle((GXT)xa1,(GYT)ya,(GXT)xa2,(GYT)ya,col);
      }
   }
#endif

/*
  Draw mirror points
  (reduces calculations to a 0-45 degree arc)
*/
static void plotarcs(int xc, int yc, int x1, int y1)
   {
   if (fil == 0)
      {
      G_SETPIXEL( xc+x1, yc-y1);
      G_SETPIXEL( xc+y1, yc-x1);
      G_SETPIXEL( xc-x1, yc-y1);
      G_SETPIXEL( xc-y1, yc-x1);
      G_SETPIXEL( xc+x1, yc+y1);
      G_SETPIXEL( xc+y1, yc+x1);
      G_SETPIXEL( xc-x1, yc+y1);
      G_SETPIXEL( xc-y1, yc+x1);
      }
   else
      {
      #ifdef GHW_USING_VBYTE
      setlinev( xc+x1, yc-y1, yc+y1 );
      setlinev( xc-x1, yc-y1, yc+y1 );
      setlinev( xc+y1, yc-x1, yc+x1 );
      setlinev( xc-y1, yc-x1, yc+x1 );
      #else
      setlineh( xc-x1, xc+x1, yc-y1 );
      setlineh( xc-y1, xc+y1, yc-x1 );
      setlineh( xc-x1, xc+x1, yc+y1 );
      setlineh( xc-y1, xc+y1, yc+x1 );
      #endif
      }
   }

/*
   Fast draw circle.
   This functions uses signed integer coordinates so the center optionally
   can be placed outside the current viewport (and screen).
*/
void gcircle(SGINT xc, SGINT yc, SGINT r, SGUCHAR fill)
   {
   SGINT x, y;
   float p;
   if (r <= 0)
      return;
   /* preset view-port limits */
   xb = gcurvp->lt.x;
   yb = gcurvp->lt.y;
   xe = gcurvp->rb.x;
   ye = gcurvp->rb.y;
   /* To absolute coordinates */
   xc += xb;
   yc += yb;

   /* module scope parameters for faster handling */
   col = G_IS_INVERSE() ? ghw_def_background : ghw_def_foreground;
   fil=fill;

   x = 0;
   y = r;
   p = (float)(1 - r);

   /* Plot on axis */
   plotarcs(xc,yc,x,y);

   /* Plot a 45 angle, and mirror the rest */
   while(x < y)
      {
      if(p < 0.0)
         {
         x = x + 1;
         p = p + (float) ((SGLONG)2 * x + 1);
         }
      else
         {
         x = x + 1;
         y = y - 1;
         p = p + (float)((SGLONG)2*(x - y) + 1);
         }
      plotarcs(xc,yc,x,y);
      }
   ghw_updatehw();
   }

#ifdef GFUNC_VP

void gcircle_vp( SGUCHAR vp, SGINT xc, SGINT yc, SGINT r, SGUCHAR fill )
   {
   GSETFUNCVP(vp, gcircle(xc, yc, r, fill));
   }

#endif /* GFUNC_VP */
#endif /* GGRAPHICS */
