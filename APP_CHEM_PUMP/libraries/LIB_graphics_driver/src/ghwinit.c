/************************** ghwinit.c *****************************

   Low-level driver functions for the ST7528 LCD display controller
   initialization and error handling.

   The ST7528 controller is assumed to be used with a LCD module.

   The following LCD module characteristics MUST be correctly
   defined in GDISPCFG.H:

      GDISPW  Display width in pixels
      GDISPH  Display height in pixels
      GBUFFER If defined most of the functions operates on
              a memory buffer instead of the LCD hardware.
              The memory buffer content is copied to the LCD
              display with ghw_updatehw().
              (Equal to an implementation of delayed write)

   The ST7528 color information is stored horizontally over 4 or 2
   vertical bytes. This modul contains functions to extract pixel wise
   color information and function for fast read and write of 8x8 pixel
   color information to a temp buffer.

   Version number: 1.1
   Copyright (c) RAMTEX Engineering Aps 2005-2008

*********************************************************************/

#include <stdio.h>
#include <gdisphw.h>  /* HW driver prototypes and types */
#include <st7528.h>   /* ST7528 controller specific definements */

/* #define GREYSCALE_TEST */ /* Enable to generate test bars for greyscale adjustments */

/* Check basic controller settings */
#if (!(defined ( GHW_ST7528_MODE01 ) || defined ( GHW_ST7528_MODE2 ) || defined( GHW_ST7528_MODE3)))
  #error Wrong configuration file used
#endif

#if defined ( GHW_ST7528_MODE01 )
  #if ((GDISPW > 128) || (GDISPH > 129))
  #error Illegal GDISPH, GDISPW settings for controller mode in gdispcfg.h
  #endif
  #define GCTRLW 128
#elif defined ( GHW_ST7528_MODE2 )
  #if ((GDISPW > 132) || (GDISPH > 129))
  #error Illegal GDISPH, GDISPW settings for controller mode in gdispcfg.h
  #define GCTRLW 132
  #endif
#elif defined( GHW_ST7528_MODE3)
  #if ((GDISPW > 160) || (GDISPH > 101))
  #error Illegal GDISPH, GDISPW settings for controller mode in gdispcfg.h
  #endif
  #define GCTRLW 160
#endif

/* Check and clean up some definitions */
#ifndef GHW_NO_HDW_FONT
  #error Illegal configuration file. GHW_NO_HDW_FONT must be defined in GDISPCFG.H
#endif

#ifndef GHW_XOFFSET
  #ifdef GHW_MIRROR_HOR
     #define GHW_XOFFSET (GCTRLW-GDISPW)    /* Use a default x offset */
  #else
     #define GHW_XOFFSET 0
  #endif
#endif


/********************* Chip access definitions *********************/
#ifndef GHW_NOHDW
   #ifdef GHW_SINGLE_CHIP
      #include <bussim.h>
      #define  sgwrby(a,d) simwrby((a),(d))
      #define  sgrdby(a)   simrdby((a))
   #else
      /* Portable I/O functions + hardware port def */
      #include <sgio.h>
   #endif
#else
   #undef GHW_SINGLE_CHIP /* Ignore single chip mode */
#endif

/***********************************************************************/
/** All static LCD driver data is located here in this ghwinit module **/
/***********************************************************************/
#ifdef GBASIC_INIT_ERR

#ifdef GBUFFER
   #ifdef GHW_ALLOCATE_BUF
      /* <stdlib.h> is included via gdisphw.h */
      SGUCHAR *gbuf = NULL;           /* Graphic buffer pointer */
      static SGBOOL gbuf_owner = 0;   /* Identify pointer ownership */
   #else
      SGUCHAR gbuf[ GBUFSIZE ];       /* Graphic buffer */
   #endif
   GXT GFAST iltx,irbx;  /* "Dirty area" speed optimizers in buffered mode */
   GYT GFAST ilty,irby;
   SGBOOL  ghw_upddelay;
#endif

#ifdef GHW_INTERNAL_CONTRAST
static SGUCHAR ghw_contrast;      /* Current contrast value */
#endif

GCOLOR ghw_def_foreground;
GCOLOR ghw_def_background;
SGBOOL  glcd_err;                 /* Internal error */
#ifndef GNOCURSOR
GCURSOR ghw_cursor;               /* Current cursor state */
#endif
GXT ghw_xbase;
GYT ghw_ybase; /* Storrage for last y pos */


/* Symbol table entry with fixed sized symbols */
typedef struct
   {
   SGUCHAR b[8];       /* Symbol data, variable length = (cxpix/8+1)*cypix */
   } GSYMDAT;

static struct
   {
   GSYMHEAD sh;        /* Symbol header */
   SGUCHAR b[8];       /* Symbol data, variable length = (cxpix/8+1)*cypix */
   }
GCODE FCODE sysfontsym[0x80] =
   {
   #include <sfst7528.sym>        /* System font symbol table */
   };

/* Default system font */
GCODE GFONT FCODE SYSFONT =
   {
   8,      /* width */
   8,      /* height */
   sizeof(GSYMDAT),        /* number of data bytes in a symbol  (must include any alignment padding)*/
   (PGSYMBOL) sysfontsym,  /* pointer to array of SYMBOLS */
   0x80,   /* num symbols (includes download symbols) */
   NULL    /* SYSFONT must not use codepage */
   };

#ifdef GHW_PCSIM
/* PC simulator declaration */
void ghw_cmd_sim( SGUCHAR cmd );
void ghw_dcmd_sim( SGUCHAR cmd1, SGUCHAR cmd2 );
void ghw_init_sim( SGUINT dispw, SGUINT disph);
void ghw_setxy_sim( SGUINT x, SGUINT y);
void ghw_wr_sim( SGUCHAR val );
SGUCHAR ghw_rd_sim( SGUCHAR inc );
void ghw_exit_sim(void);
#endif

/**********************************************************************/
/** Low level ST7528 interface functions used only by ghw_xxx modules **/
/**********************************************************************/

/*
   Arrays with fast mask values used by the low-level graphic functions
   and view ports.
*/
GCODE SGUCHAR FCODE pixymsk[8] =
    {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};

#if (GDISPPIXW == 1)
  GCODE SGUCHAR FCODE ghw_cmsk[GDISPPIXW] = {0x1};
#elif (GDISPPIXW == 2)
  GCODE SGUCHAR FCODE ghw_cmsk[GDISPPIXW] = {0x2,0x1};
#elif (GDISPPIXW == 4)
  GCODE SGUCHAR FCODE ghw_cmsk[GDISPPIXW] = {0x8,0x4,0x2,0x1};
#else
  #error Illegal GDISPPIXW value defined in gdispcfg.h
#endif

GCODE SGUCHAR FCODE startmask[8] =
    {0xff,0xfe,0xfc,0xf8,0xf0,0xe0,0xc0,0x80};
GCODE SGUCHAR FCODE stopmask[8] =
    {0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff};


/* Tempoary buffer for horizontal byte writing */
SGUCHAR ghw_tmpb[TMPBUFSIZE];
#ifndef GBUFFER
SGUCHAR ghw_tmpb2[TMPBUFSIZE];
#endif

void ghw_setcolor(GCOLOR fore, GCOLOR back)
   {
   ghw_def_foreground = fore;
   ghw_def_background = back;
   }

#ifndef GHW_NOHDW
/*
   Local status check functions for normal commands
   Internal ghw function

   (Note with a slow target processor system busy check may
    not be required because the LCD controller is fast enough.
    In such systems a speed optimization can be made by commenting
    out the body of the ghw_wait() functions below)
*/
/* #define USING_BUSY_WAIT */ /* Define to poll the BUSY flag */

#ifdef USING_BUSY_WAIT
/* Chip wait for not busy */
static void ghw_wait(void)
   {
   #ifndef GHW_SINGLE_CHIP
   if ((sgrdby(GHWSTA) & GSTA_BUSY) != 0)
      {
      /* Controller was busy, execute wait loop */
      SGUINT timeout = 64;
      do {
         if ((sgrdby(GHWSTA) & GSTA_BUSY) == 0)
            return;
         }
      while (timeout-- != 0);
      glcd_err = 1;
      }
   #endif
   }
#else
#define ghw_wait() /* Nothing */
#endif

#endif  /* GHW_NOHDW */


/*
   Send a command to ST7528
   Wait for controller + data ready

   set ghw_err = 0 if Ok
   set ghw_err = 1 if Timeout error

   Internal ghw function
*/
static void ghw_cmd( SGUCHAR cmd )
   {
   #ifdef GHW_PCSIM
   ghw_cmd_sim( cmd );
   #endif

   #ifndef GHW_NOHDW
   ghw_wait();
   sgwrby(GHWCMD, cmd);
   #endif
   }

/*
   Send a double byte command to ST7528
   Wait for controller + data ready

   set ghw_err = 0 if Ok
   set ghw_err = 1 if Timeout error

   Internal ghw function
*/
static void ghw_dcmd( SGUCHAR cmd1, SGUCHAR cmd2 )
   {
   #ifdef GHW_PCSIM
   ghw_dcmd_sim( cmd1, cmd2 );
   #endif

   #ifndef GHW_NOHDW
   ghw_wait();
   sgwrby(GHWCMD, cmd1);
   ghw_wait();
   sgwrby(GHWCMD, cmd2);
   #endif
   }

/*
   Repeat a color operation on a vertical byte row
*/
void ghw_wr_color_line(GXT xb, GYT y, GXT xe, SGUCHAR msk, GCOLOR col)
   {
   GXT xc;
   #ifdef GBUFFER

   GBUFINT gbufidx;
   gbufidx = GINDEX(xb, y);

   #else

   /* Set page address ? */
   if ((y & (~0x7)) != ghw_ybase)
      {
      #ifndef GHW_NOHDW
      ghw_wait();
      sgwrby(GHWCMD,((y>>3) & 0xF) | GCTRL_YADR);  /* Set Y koordinate */
      #endif

      ghw_ybase = y & (~0x7);
      }

   #ifdef GHW_PCSIM
   ghw_setxy_sim(xb,y);
   #endif

   /* Set x address */
   #ifndef GHW_NOHDW
   ghw_wait();
   sgwrby(GHWCMD,(((xb+GHW_XOFFSET)>>4) & 0xF) | GCTRL_XADR_H);
   ghw_wait();
   sgwrby(GHWCMD,((xb+GHW_XOFFSET) & 0xF)      | GCTRL_XADR_L);
   #endif

   #endif  /* GBUFFER */

   if (msk != 0xff)
      {
      /* Read-modify-write color line */
      #ifndef GBUFFER
      #ifndef GHW_NOHDW
      ghw_wait();
      sgwrby(GHWCMD, GCTRL_RMW_STRT);
      #endif
      #endif
      do
         {
         for (xc = 0; xc < GDISPPIXW; xc++)
            {
            #ifdef GBUFFER

            gbuf[gbufidx] = (gbuf[gbufidx] & (~msk)) | (((col & ghw_cmsk[xc])!=0) ? msk : 0x00);
            gbufidx++;

            #else

            #ifndef GHW_NOHDW
            register SGUCHAR dat;
            ghw_wait();
            dat = sgrdby(GHWRD);                     /* Dummy read */
            dat = (col & ghw_cmsk[xc]) ? msk : 0x00;
            ghw_wait();
            dat |= (sgrdby(GHWRD) & ~msk);           /* read and insert unmasked bits */
            ghw_wait();
            sgwrby(GHWWR,dat);                       /* Write modified data back */
            #endif

            #ifdef GHW_PCSIM
            ghw_wr_sim(((col & ghw_cmsk[xc]) ? msk : 0x00) | (ghw_rd_sim(0) & (~msk)));
            #endif

            #endif
            }
         }
      while(++xb <= xe);
      #ifndef GBUFFER
      #ifndef GHW_NOHDW
      ghw_wait();
      sgwrby(GHWCMD, GCTRL_RMW_END);
      #endif
      #endif
      }
   else
      {
      /* Write color line */
      do
         {
         for (xc = 0; xc < GDISPPIXW; xc++)
            {
            #ifdef GBUFFER
            gbuf[gbufidx++] = (col & ghw_cmsk[xc]) ? 0xff : 0x00;
            #else

            #ifndef GHW_NOHDW
            ghw_wait();
            sgwrby(GHWWR,(col & ghw_cmsk[xc]) ? 0xff : 0x00);   /* Write data */
            #endif

            #ifdef GHW_PCSIM
            ghw_wr_sim((col & ghw_cmsk[xc]) ? 0xff : 0x00);
            #endif

            #endif
            }
         }
      while(++xb <= xe);
      }
   }

/*
   Write buffer to screen. If msk != 0xff do read modify write
   operations so only bytes where msk bit has 1 is changed.
   Writes vertical bytes
      Cnt is whole number pixels.
*/
void ghw_wrbuf(SGUCHAR *buf, GXT xb, GYT y, GXT xe, SGUCHAR msk)
   {
   SGUINT cnt;
   /* Set page address ? */
   if ((y & (~0x7)) != ghw_ybase)
      {
      #ifndef GHW_NOHDW
      ghw_wait();
      sgwrby(GHWCMD,((y>>3) & 0xF) | GCTRL_YADR);  /* Set Y koordinate */
      #endif
      ghw_ybase = y & (~0x7);
      }

   /* Set x address */
   cnt = ((SGUINT)xe-xb+1)*GDISPPIXW; /* loop count */

   #ifdef GHW_PCSIM
   ghw_setxy_sim(xb,y);
   #endif

   #ifndef GHW_NOHDW
   xb+=GHW_XOFFSET;
   ghw_wait();
   sgwrby(GHWCMD,((xb>>4) & 0xF) | GCTRL_XADR_H);
   ghw_wait();
   sgwrby(GHWCMD,(xb & 0xF)      | GCTRL_XADR_L);
   #endif

   if (msk != 0xff)
      {
      #ifndef GHW_NOHDW
      ghw_wait();
      sgwrby(GHWCMD, GCTRL_RMW_STRT);
      #endif
      do
         {
         #ifndef GHW_NOHDW
         register SGUCHAR dat;
         ghw_wait();
         dat = sgrdby(GHWRD);                               /* Dummy read */
         ghw_wait();
         dat = (*buf & msk) | (sgrdby(GHWRD) & ~msk); /* add bits not to be modifed */
         ghw_wait();
         sgwrby(GHWWR,dat);                           /* Write data */
         #endif

         #ifdef GHW_PCSIM
         ghw_wr_sim((*buf & msk)  | (ghw_rd_sim(0) & ~msk));
         #endif

         buf++;
         }
      while(--cnt != 0);
      #ifndef GHW_NOHDW
      ghw_wait();
      sgwrby(GHWCMD, GCTRL_RMW_END);
      #endif
      }
   else
      {
      /* Write buffer */
      do
         {
         #ifndef GHW_NOHDW
         ghw_wait();
         sgwrby(GHWWR,*buf);   /* Write data */
         #endif

         #ifdef GHW_PCSIM
         ghw_wr_sim(*buf);
         #endif

         buf++;
         }
      while(--cnt != 0);
      }
   }

/*
   Read buffer from screen.
   The read data is and'ed with msk to clear any bits which should
   later be updated.
   Cnt is whole number of pixels.
*/
#ifndef GBUFFER
void ghw_rdbuf(SGUCHAR *buf, GXT xb, GYT y, GXT xe)
   {
   SGUINT cnt;
   /* Set page address ? */
   if ((y & (~0x7)) != ghw_ybase)
      {
      #ifndef GHW_NOHDW
      ghw_wait();
      sgwrby(GHWCMD,((y>>3) & 0xF) | GCTRL_YADR);  /* Set Y page*/
      #endif
      ghw_ybase = y & (~0x7);
      }

   /* Set x address */
   ghw_xbase = xb;           /* Save buffer base */
   cnt = ((SGUINT)xe-xb+1)*GDISPPIXW; /* loop count incl pixel resolution */

   #ifndef GHW_NOHDW
   xb += GHW_XOFFSET;
   ghw_wait();
   sgwrby(GHWCMD,((xb>>4) & 0xF) | GCTRL_XADR_H);
   ghw_wait();
   sgwrby(GHWCMD,(xb & 0xF)      | GCTRL_XADR_L);
   ghw_wait();
   *buf = sgrdby(GHWRD);   /* Dummy read */
   #else

   #ifdef GHW_PCSIM
   ghw_setxy_sim(xb,y);
   #endif

   #endif

   do
      {
      #ifndef GHW_NOHDW
      ghw_wait();
      *buf = sgrdby(GHWRD);
      #else
      #ifdef GHW_PCSIM
      *buf = ghw_rd_sim(1);
      #endif
      #endif
      buf++;
      }
   while(--cnt != 0);
   }
#endif


/*
   Return bit color at x,y position in tmp buffer
   x,y is pixels coordinates

   Note for non buffered mode the ghw_xbase must have been
   initiated in advance by a call to ghw_rdbuf, or directly.
*/
GCOLOR ghw_rdbuf_color(GXT x, GYT y)
   {
   SGUCHAR *p;
   GCOLOR col = 0;
   #ifdef GBUFFER
   p = &(gbuf[ GINDEX(x, y)]);
   #else
   p = &ghw_tmpb[(SGUINT)((x-ghw_xbase) % TMPBUFPIXSIZE) * GDISPPIXW];
   #endif
   y = pixymsk[y & 0x7];
   #if (GDISPPIXW == 4)
   if (*p++ & y) col |= 0x8;
   if (*p++ & y) col |= 0x4;
   #endif
   if (*p++ & y) col |= 0x2;
   if (*p   & y) col |= 0x1;
   return col;
   }

/*
   Set bit color at x,y position in tmp buffer
   x,y is pixels coordinates
*/
void ghw_wrbuf_color(GXT x, GYT y, GCOLOR col)
   {
   SGUCHAR *p;
   SGUCHAR msk,mskc;
   #ifdef GBUFFER
   p = &(gbuf[ GINDEX(x, y) ]);
   #else
   p = &(ghw_tmpb[(SGUINT)((x-ghw_xbase) % TMPBUFPIXSIZE) * GDISPPIXW]);
   #endif
   msk = pixymsk[y & 0x7];
   mskc = (1 << (GDISPPIXW-1));
   do
      {
      if (col & mskc)
         *p |= msk;
      else
         *p &= ~msk;
      p++;
      }
   while((mskc >>= 1) != 0);
   }


/***********************************************************************/
/**        ST7528 Initialization and error handling functions         **/
/***********************************************************************/


/*
   Fast set or clear of grapic buffer (in LCD module or RAM)

   Internal ghw function
*/
static void ghw_bufset( GCOLOR color )
   {
   GYT y;
   ghw_ybase = 0xff; /* Force update */
   for (y = 0; y < GDISPH; y += 8)
      {
      ghw_wr_color_line(0, y, GDISPW-1, 0xff, color);
      #ifdef GBUFFER
      ghw_wrbuf(&gbuf[GINDEX(0, y)], 0, y, GDISPW-1, 0xff);
      #endif
      }
   }


/*
   Grey scale values.
   Note the grey scale levels are not visually linear.
   There is usually more variance on the higher values.
   The acually "look" also depends highly on the contrast
   voltage settings.

   These values may need to be adjusted individually for each
   display module type.
   Enable the GREYSCALE_TEST definition in the beginning of
   this module so ghw_draw_greyscale() is activated. This function
   generates a test pattern to facilitate adjustment of the
   grey-scale palette values below.
*/
#ifndef GHW_NOHDW
static GCODE SGUCHAR FCODE grey_pal[64] =
    {
    00,00,00,00,     /* 0   */
    06,06,06,06,     /* 1 6 */
    11,11,11,11,     /* 2 5 */
    16,16,16,16,     /* 3 5 */
    21,21,21,21,     /* 4 5 */
    26,26,26,26,     /* 5 5 */
    31,31,31,31,     /* 6 4 */
    35,35,35,35,     /* 7 4 */
    39,39,39,39,     /* 8 4 */
    43,43,43,43,     /* 9 4 */
    47,47,47,47,     /*10 3 */
    50,50,50,50,     /*11 3 */
    53,53,53,53,     /*12 3 */
    56,56,56,56,     /*13 2 */
    58,58,58,58,     /*14 2 */
    60,60,60,60      /*15   */
    };
#endif

void ghw_load_palette( void )
   {
   #ifndef GHW_NOHDW
   SGUCHAR pos = 0;

   sgwrby(GHWCMD, GCTRL_MODESET);   /* Mode set */
   sgwrby(GHWCMD, 0x05);

   ghw_cmd(GCTRL_PALETTE_START | pos);
   for (; pos < sizeof(grey_pal); pos++)
      {
      ghw_dcmd(GCTRL_PALETTE_START | pos, grey_pal[pos]);
      }

   sgwrby(GHWCMD, GCTRL_MODESET);   /* Mode set */
   sgwrby(GHWCMD, 0x04);
   #endif
   }


#ifdef GREYSCALE_TEST
/*
   Test functions
   Fills screen with a number of gray scales.bars.
   Can be used when fine tuning the grey_pal table
   values for the given display module
*/
void ghw_draw_greyscale(void)
   {
   GYT y;
   SGUCHAR msk;
   GCOLOR col;
   for (y=0, msk = 0xf, col = 0; y < 16*4; y+=4, col = (col+1) & 0xf)
      {
      ghw_wr_color_line(0, y , GDISPW-1, msk, col);
      if (((msk <<= 4) & 0xff) == 0)
         {
         msk = 0xf;
         }
      }
   }
#endif

/*
   Wait here for LCD power to stabilize
   If this condition is violated then the rest of the initialization may fail.
   The required time depends on the actual display and power circuit.
   Here multiple writes is used as delay.

   For some displays the for(;;) loop can be optimized away (commented out)
*/
static void ghw_cmdw( SGUCHAR cmd )
   {
   SGUINT tmp;
   for (tmp = 0; tmp < 255; tmp++)
      ghw_cmd( cmd );
   }

/*
   Initialize display, clear ram  (low-level)
   Clears glcd_err status before init

   Return 0 if no error,
   Return != 0 if some error
*/

SGBOOL ghw_init(void)
   {
   #ifdef GBUFFER
   iltx = 0;
   ilty = 0;
   irbx = GDISPW-1;
   irby = GDISPH-1;
   ghw_upddelay = 0;
   #endif


   #ifdef GHW_PCSIM
   /* Tell simulator about the visual LCD screen organization */
   ghw_init_sim( GDISPW, GDISPH);
   #endif

   glcd_err = 0;
   ghw_io_init();                 /* Perform target specific initialization */

   #if (defined( GHW_ALLOCATE_BUF) && defined( GBUFFER ))
   if (gbuf == NULL)
      {
      /* Allocate graphic ram buffer */
      if ((gbuf = calloc(ghw_gbufsize(),1)) == NULL)
         glcd_err = 1;
      else
         gbuf_owner = 1;
      }
   #endif

   if (glcd_err)
      return 1;                   /* Some lowlevel io error detected */

   ghw_setcolor(GHW_PALETTE_FOREGROUND,GHW_PALETTE_BACKGROUND);
   ghw_dcmd(GCTRL_MODESET, 0x04);   /* Mode set */
   ghw_cmd(GCTRL_RESET);            /* soft reset */

   #ifndef GHW_NOHDW
   #ifndef GHW_SINGLE_CHIP
   /* Wait for soft reset to complete */

   if ((sgrdby(GHWSTA) & (GSTA_BUSY | GSTA_RESET)) != 0)
      {
      /* Controller was busy, execute wait loop */
      SGUCHAR timeout = 127;
      do {
         if (--timeout == 0)
            {
            glcd_err = 1;
            return 1; /* Some lowlevel io error detected */
            }
         }
      while ((sgrdby(GHWSTA) & (GSTA_BUSY | GSTA_RESET)) != 0);
      }
   #endif /* GHW_SINGLE_CHIP */

   ghw_dcmd(GCTRL_DSTART,0);      /* LCD start line = 0 */
   ghw_dcmd(GCTRL_DUTY,0);        /* Partial duty = 0 */

   #ifdef GHW_MIRROR_VER
   ghw_cmd(GCTRL_ADC | 1);       /* Inverse vertical direction */
   #else
   ghw_cmd(GCTRL_ADC | 0);       /* Normal vertical direction */
   #endif

   #ifdef GHW_MIRROR_HOR
   ghw_cmd(GCTRL_SHL | 8);        /* Inverse horizontal direction */
   #else
   ghw_cmd(GCTRL_SHL | 0);        /* Normal horizontal direction */
   #endif
   ghw_dcmd(GCTRL_INI_COM0,0);    /* Initial Com */

   ghw_cmd(GCTRL_OSC_ON);         /* Turn oscillator on */
   ghw_cmd(GCTRL_DC_DC_STEPUP|2); /* DC-DC stepup | (0-3)*/
   ghw_cmd(GCTRL_REG_RES  | 7);   /* Regulator resistor ratio select | (0-7) */
   ghw_cont_set(40);              /* Set default contrast level (0-99) */
   ghw_cmd(GCTRL_LCD_BIAS | 5);   /* LCD bias ratio | (0-7) */

   ghw_cmd(GCTRL_FRAME | 2);      /* FRC and PWM mode | FRC | PWM1,PWM0 */


   ghw_cmdw(GCTRL_POWER | 0x4);   /* Turn stepup generator on */
   ghw_cmdw(GCTRL_POWER | 0x6);   /* Turn resistor ladder on */
   ghw_cmdw(GCTRL_POWER | 0x7);   /* Turn buffers on */


   ghw_dcmd(GCTRL_NLINEINV, 0);

   #endif  /* GHW_NOHDW */

   if (glcd_err)
      return 1;                   /* Some initial error detected */

   ghw_load_palette();

   ghw_ybase = 0xff;              /* Force coordinate setting*/
   ghw_xbase = 0;

   ghw_bufset(ghw_def_background); /* Clear graphic area */
   #ifdef GREYSCALE_TEST
   ghw_draw_greyscale();
   #endif
   ghw_cmd(GCTRL_ON);             /* Display On */
   #ifndef GNOCURSOR
   ghw_cursor = GCURSIZE1;        /* Cursor is off initially */
   /* ghw_cursor = GCURSIZE1 | GCURON; */  /* Uncomment to set cursor on initially */
   #endif

   ghw_updatehw();  /* Flush to display hdw or simulator */

   return (glcd_err != 0) ? 1 : 0;
   }

/*
   Return last error state. Called from applications to
   check for LCD HW or internal errors.
   The error state is reset by ghw_init and all high_level
   LCD functions.

   Return == 0 : No errors
   Return != 0 : Some errors
*/
SGUCHAR ghw_err(void)
   {
   #if (defined(_WIN32) && defined( GHW_PCSIM))
   if (GSimError())
      return 1;
   #endif
   return (glcd_err == 0) ? 0 : 1;
   }

void ghw_exit(void)
   {
   #if (defined( GHW_ALLOCATE_BUF) && defined( GBUFFER ))
   if (gbuf != NULL)
      {
      ghw_cmd(GCTRL_OFF);  /* Blank display */
      if (gbuf_owner != 0)
         {
         /* Buffer is allocated by ginit, so release graphic buffer here */
         free(gbuf);
         gbuf_owner = 0;
         }
      gbuf = NULL;
      }
   #else
   ghw_cmd(GCTRL_OFF);  /* Blank display */
   #endif
   ghw_io_exit();       /* Release any LCD hardware resources, if required */
   #ifdef GHW_PCSIM
   ghw_exit_sim();      /* Release simulator resources */
   #endif
   }

/*
   Display a (fatal) error message.
   The LCD display module is always cleared and initialized to
   the system font in advance.
   The error message is automatically centered on the screen
   and any \n characters in the string is processed.

   str = ASCII string to write at display center
*/
void ghw_puterr( PGCSTR str)
   {
   PGCSTR idx=str;
   SGUCHAR xcnt,i,mskc;
   PGSYMBYTE psym;
   GYT y,h;
   GXT x,xc;
   SGBOOL centering = 1;

   if (ghw_init() != 0)  /* (Re-) initialize display */
      return;            /* Some initialization error */

   /* Count number of xcnt in string */
   if (idx == NULL)
      return;
   x = 0;
   for (x = 0, y = 1; *idx != 0; idx++)
      {
      if (*idx == '\n')
         {
         y++;
         x = 0;
         }
      else
         {
         x++;
         if (x >= (GDISPW/SYSFONT.symwidth))
            {
            y++;
            x = 0;         /* Some text longer than line, start on new line */
            centering = 0; /* and skip horizontal centering */
            }
         }
      }

   /* Set start line */
   y = ((y > ((GDISPH/8)-1)) ? 0 : ((GDISPH/8-1)-y)/2) * 8;
   idx=str;
   do
      {
      xcnt=0;  /* Set start x position so line is centered */
      while ((idx[xcnt]!=0) && (xcnt < GDISPW/8) &&
            ((idx[xcnt]!='\n') || (centering == 0)) )
         {
         xcnt++;
         }

      /* Set start for centered message */
      if ((GDISPW > xcnt*SYSFONT.symwidth) && centering)
         x = ((GDISPW-xcnt*SYSFONT.symwidth)/2) & ~(0x7);
      else
         x = 0;
      h = (SYSFONT.symheight > 8) ? 8 : SYSFONT.symheight;

      while (xcnt-- > 0)
         {
         /* Point to graphic content for character symbol */
         psym = &(sysfontsym[(*idx == '\n') ? ' ' : ((*idx) & 0x7f)].b[0]);

         #ifdef GBUFFER
         invalrect(x,y);
         invalrect(x+7,y);
         #else
         ghw_xbase = x;
         #endif

         /* Place symbol data in temp buffer and on display */
         for (i = 0; i < h; i++)
            {
            for (xc = 0, mskc = 0x80; (xc < SYSFONT.symwidth) || (mskc != 0); xc++, mskc >>= 1)
               {
               ghw_wrbuf_color(x+xc, y+i, (*psym & mskc) ? ghw_def_foreground : ghw_def_background);
               }
            psym++;
            }

         #ifndef GBUFFER
         /* Flush tmp buffer */
         ghw_wrbuf(&ghw_tmpb[0],x,y,x+xc-1,0xff);
         #endif
         idx++;
         if ((x+=8) > GDISPW)
            break; /* Line overrun, continue on next line */
         }
      if (centering && (*idx == '\n'))
         idx++;
      y+=8;
      }
   while ((*idx != 0) && (y < GDISPH));

   ghw_updatehw();  /* Flush to display hdw or simulator */
   }

#ifndef GNOCURSOR
/*
   Replace cursor type data (there is no HW cursor support in ST7528)
*/
void ghw_setcursor( GCURSOR type)
   {
   ghw_cursor = type;
   }
#endif

/*
   Turn display off
   (Minimize power consumption)
*/
void ghw_dispoff(void)
   {
   GBUF_CHECK();
   ghw_cmd(GCTRL_OFF);
   }

/*
   Turn display on
*/
void ghw_dispon(void)
   {
   GBUF_CHECK();
   ghw_cmd(GCTRL_ON);
   }

#ifdef GHW_INTERNAL_CONTRAST
/*
   Set contrast (Normalized value range [0 : 99] )
   Returns the old value.
*/
SGUCHAR ghw_cont_set(SGUCHAR contrast)
   {
   SGUCHAR tmp;
   GLIMITU(contrast,99);
   tmp = ghw_contrast;
   ghw_contrast = contrast;

   #if (defined( GHW_ALLOCATE_BUF) && defined( GBUFFER ))
   if (gbuf == NULL) {glcd_err = 1; return contrast;}
   #endif

   ghw_dcmd(GCTRL_REF_VOLT , (SGUCHAR)((((SGUINT) contrast) *16) / 25));

   return tmp;
   }

/*
   Change contrast (Normalized value range [-99 : +99] )
   Returns the old value.
*/
SGUCHAR ghw_cont_change(SGCHAR contrast_diff)
   {
   SGINT tmp = (SGINT) ((SGUINT) ghw_contrast);
   tmp += (SGINT) contrast_diff;
   GLIMITU(tmp,99);
   GLIMITD(tmp,0);
   return ghw_cont_set((SGUCHAR)tmp);
   }
#endif /* GHW_INTERNAL_CONTRAST */

#if (defined( GHW_ALLOCATE_BUF) && defined( GBUFFER ))
/*
  Size of buffer requied to save the whole screen state
*/
SGUINT ghw_gbufsize( void )
   {
   return GBUFSIZE + sizeof(GHW_STATE);
   }

#ifdef GSCREENS
/*
   Check if screen buf owns the screen ressources.
*/
SGUCHAR ghw_is_owner( SGUCHAR *buf )
   {
   return ((buf == gbuf) && (gbuf != NULL)) ? 1 : 0;
   }

/*
  Save the current state to the screen buffer
*/
SGUCHAR *ghw_save_state( SGUCHAR *buf )
   {
   GHW_STATE *ps;
   if (!ghw_is_owner(buf))
      return NULL;
   ps = (GHW_STATE *)(&gbuf[GBUFSIZE]);
   ps->upddelay = (ghw_upddelay != 0);
   #ifndef GNOCURSOR
   ps->cursor = ghw_cursor;
   #endif
   ps->foreground = ghw_def_foreground; /* Palette may vary, save it */
   ps->background = ghw_def_background;
   return gbuf;
   }

/*
   Set state to buf.
   If buffer has not been initiated by to a screen before, only
   the pointer is updated. Otherwise the buffer
*/
void ghw_set_state(SGUCHAR *buf, SGUCHAR doinit)
   {
   if (gbuf != NULL)
      {
      /* The LCD controller has been initiated before */
      if (gbuf_owner != 0)
         {
         /* Buffer was allocated by ginit, free it so screen can be used instead*/
         free(gbuf);
         gbuf_owner = 0;
         gbuf = NULL;
         }
      }

   if (doinit != 0)
      {
      /* First screen initialization, just set buffer pointer and
         leave rest of initialization to a later call of ghw_init() */
      gbuf = buf;
      gbuf_owner = 0;
      }
   else
      {
      if ((gbuf = buf) != NULL)
         {
         GHW_STATE *ps;
         ps = (GHW_STATE *) &buf[GBUFSIZE];
         #ifndef GNOCURSOR
         ghw_cursor = ps->cursor;
         #endif

         ghw_upddelay = 0;        /* Force update of whole screen */
         iltx = 0;
         ilty = 0;
         irbx = GDISPW-1;
         irby = GDISPH-1;
         ghw_ybase = 0xff; /* Force coordinate setting*/
         ghw_xbase = 0x0;
         ghw_updatehw();
         ghw_upddelay = (ps->upddelay != 0) ? 1 : 0;
         ghw_def_foreground = ps->foreground; /* Palette may vary, save it */
         ghw_def_background = ps->background;
         }
      }
   }
#endif  /* GSCREENS */
#endif  /* GHW_ALLOCATE_BUF */

#endif /* GBASIC_INIT_ERR */

