#ifndef ST7528_H
#define ST7528_H
/****************************** KS07XX.H *****************************

   Definitions specific for the KS07XX Graphic LCD controller.

   The KS07XX controller is assumed to be used with a LCD module.
   The LCD module characteristics (width, height etc) must be correctly
   defined in GDISPCFG.H

   This header should only be included by the low-level LCD drivers

   Creation date:

   Revision date
   Revision Purpose:

   Version number: 1.0
   Copyright (c) RAMTEX Engineering Aps 2005

***********************************************************************/

#include <gdisphw.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Controller status bits */
#define GSTA_BUSY        0x80
#define GSTA_OFF         0x40
#define GSTA_RESET       0x20

/* Controller commands ( 1 byte ) */
#define GCTRL_MODESET    0x38  /* Mode selection */
#define GMODE_EXT        0x01

#define GCTRL_PS_ON      0xA8  /* Power save mode | (0-1) Normal,sleep */
#define GCTRL_PS_OFF     0xE1  /* Release Power save mode */
#define GCTRL_FRAME      0x90  /* FRC and PWM mode | FRC | PWM1,PWM0 */

#define GCTRL_OFF        0xAE  /* Display off */
#define GCTRL_ON         0xAF  /* Display on */
#define GCTRL_XADR_H     0x10  /* Set X adr | (132-0) MSB */
#define GCTRL_XADR_L     0x00  /* Set X adr | (132-0) LSB */

#define GCTRL_YADR       0xB0  /* Set Y adr | (15-0)*/
#define GCTRL_DSTART     0x40  /* Set Initial display line (0)*/
#define GCTRL_RESET      0xE2  /* Reset controller */

#define GCTRL_SHL        0xC0  /* Norm/reverse SHL | (0x00 or 0x08) */

#define GCTRL_POWER      0x28  /* Power-Ctrl VC,VR,VF | (0-7) */
#define GCTRL_REG_RES    0x20  /* Regulator resistor ration select | (0-7) */
#define GCTRL_NORM_REV   0xA6  /* Normal/Reverse display | (0-1) */

#define GCTRL_ADC        0xA0  /* Set Alternative DireCtion | (0-1) */
#define GCTRL_E_ON       0xA4  /* Normal display / Entire display on  | (0-1) */

#define GCTRL_OSC_ON     0xAB  /* Turn oscillator on */
#define GCTRL_LCD_BIAS   0x50  /* Set bias | (0-7) */
#define GCTRL_INI_LINE   0x40  /* Double command 2:byte (0-128)*/
#define GCTRL_INI_COM0   0x44  /* Double command 2:byte (0-128)*/
#define GCTRL_DUTY       0x48  /* Double command 2:byte (0-128)*/
#define GCTRL_NLINEINV   0x4C  /* Double command 2:byte (0-32) */
#define GCTRL_DC_DC_STEPUP 0x64  /* DC-DC_stepup (0-3) */
#define GCTRL_REF_VOLT   0x81  /* Set reference voltage, next byte = (0-63) */

#define GCTRL_RMW_STRT   0xE0  /* Set reference voltage, next byte = (0-63) */
#define GCTRL_RMW_END    0xEE  /* Set reference voltage, next byte = (0-63) */
/* Controller commands (2 byte) */
#define GCTRL_INDICATOR  0xAC  /* Set static indicator mode (0=off,1=On)
                                  next byte : (0=Off 1,2= Blinking 3=On) */

/* Extended mode command */
#define GCTRL_PALETTE_START 0x80  /* Mode 0 pallette command | 0x8 | (0-7)*/
                                  /* Mode 1 pallette command | (0-63)*/

/* Internal functions and types used only by other ghw_xxx functions */
#ifdef GBASIC_INIT_ERR
extern GCODE SGUCHAR FCODE pixymsk[8];     /* Bit mask values */
void ghw_wr_color_line(GXT xb, GYT y, GXT xe, SGUCHAR msk, GCOLOR col);
void ghw_wrbuf(SGUCHAR *buf, GXT xb, GYT y, GXT xe, SGUCHAR msk);
void ghw_wrbuf_color(GXT x, GYT y, GCOLOR col);
GCOLOR ghw_rdbuf_color(GXT x, GYT y);
#ifndef GBUFFER
void ghw_rdbuf(SGUCHAR *buf, GXT xb, GYT y, GXT xe);
#endif

#endif /* GBASIC_INIT_ERR */

#define  TMPBUFPIXSIZE 8
#define  TMPBUFSIZE (TMPBUFPIXSIZE*GDISPPIXW)
#define  GMIN(x1,xlow) (((xlow) < (x1)) ? (xlow) : (x1))

extern SGUCHAR ghw_tmpb[TMPBUFSIZE];
extern SGUCHAR ghw_tmpb2[TMPBUFSIZE];
extern GXT ghw_xbase;
extern GCODE SGUCHAR FCODE ghw_cmsk[GDISPPIXW];
extern GCODE SGUCHAR FCODE startmask[8];
extern GCODE SGUCHAR FCODE stopmask[8];

/*
   Internal data types used only by ghw_xxx functions
   The data types are located in ghw_init
*/

extern SGBOOL glcd_err;        /* Internal hdw error */

#define  GBUFSIZE (((GDISPH+7)/8) * GDISPW * GDISPPIXW)

#ifdef GBUFFER
   extern SGBOOL ghw_upddelay;

   /* "Dirty area" buffer controls for ghw_update speed optimization */
   extern GXT GFAST iltx,irbx;
   extern GYT GFAST ilty,irby;

   #define invalx( irx ) { \
      register GXT rirx; \
      rirx = (GXT)(irx); \
      if(  irbx < iltx ) iltx = irbx = rirx; \
      else if( rirx < iltx ) iltx = rirx; \
      else if( rirx > irbx ) irbx = rirx; \
      }

   #define invaly( iry ) { \
      register GYT riry; \
      riry = (GYT)(iry); \
      if( irby < ilty) ilty = irby = riry; \
      else if( riry < ilty ) ilty = riry; \
      else if( riry > irby ) irby = riry; \
      }

   #define invalrect( irx, iry ) { \
      invalx( irx ); \
      invaly( iry ); \
      }

   #ifdef GHW_ALLOCATE_BUF
      extern   SGUCHAR *gbuf;                  /* Graphic buffer pointer */
      #define GBUF_CHECK()  {if (gbuf == NULL) {glcd_err=1;return;}}
   #else
      extern   SGUCHAR gbuf[GBUFSIZE];         /* Graphic buffer */
      #define GBUF_CHECK()  { /* Nothing */ }
   #endif

   /* Structure to save the low-level state information */
   typedef struct _GHW_STATE
      {
      SGUCHAR upddelay;  /* Store for ghw_update */
      #ifndef GNOCURSOR
      GCURSOR cursor;    /* Store for ghw_cursor */
      #endif
      GCOLOR foreground; /* Store for current foreground and background color */
      GCOLOR background;
      } GHW_STATE;

   #define  GINDEX(x,y) ((((GBUFINT)(x)) + ((GBUFINT)((y)/GDISPCH))*GDISPW) * GDISPPIXW )

#else  /* GBUFFER */

   #ifdef GHW_ALLOCATE_BUF
     #undef GHW_ALLOCATE_BUF /* Allocation must only be active in buffered mode */
   #endif
   #define GBUF_CHECK()  { /* Nothing */ }

#endif /* GBUFFER */


#ifdef __cplusplus
}
#endif


#endif /* KS07XX_H */
