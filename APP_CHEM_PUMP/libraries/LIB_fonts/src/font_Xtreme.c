/****************************   Xtreme.c ***********************

   Xtreme font table and code page structure definitions.
   This file has been auto-generated with the IconEdit / FontEdit tool.

   Copyright(c) RAMTEX 1998-2006

*****************************************************************/
#include "gdisphw.h"

/* Code page entry (one codepage range element) */
static struct
   {
   GCPHEAD chp;
   GCP_RANGE cpr[2];     /* Adjust this index if more codepage segments are added */
   } GCODE FCODE Xtremecp =
   {
   #include "Xtreme.cp" /* Codepage table */
   };

typedef struct          /* Structure used for automatic word alignment */
   {
   SGUCHAR b[12];       /* Symbol data, "variable length" array */
   } GSYMDAT12;

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;         /* Symbol header */
   SGUCHAR b[12];       /* Symbol data, "variable length" */
   } GCODE FCODE Xtremesym[97] =
   {
   #include "Xtreme.sym" /* Include symbols */
   };

/* Font structure */
GCODE GFONT FCODE Xtreme =
   {
   7,       /* width */
   12,       /* height */
   sizeof(GSYMDAT12),    /* number of bytes in a symbol  (must include any alignment padding)*/
   (PGSYMBOL)Xtremesym, /* pointer to array of SYMBOLS */
   97,      /* num symbols */
   (PGCODEPAGE)&Xtremecp
   };
