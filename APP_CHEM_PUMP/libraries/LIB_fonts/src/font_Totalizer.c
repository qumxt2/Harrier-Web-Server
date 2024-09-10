/**************************** Totalizer.c ***********************

   Totalizer font table and code page structure definitions.
   This file has been auto-generated with the IconEdit / FontEdit tool.

   Copyright(c) RAMTEX 1998-2006

*****************************************************************/
#include "gdisphw.h"

/* Code page entry (one codepage range element) */
static struct
   {
   GCPHEAD chp;
   GCP_RANGE cpr[1];     /* Adjust this index if more codepage segments are added */
   } GCODE FCODE Totalizercp =
   {
   #include "Totalizer.cp" /* Codepage table */
   };

typedef struct          /* Structure used for automatic word alignment */
   {
   SGUCHAR b[40];       /* Symbol data, "variable length" array */
   } GSYMDAT40;

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;         /* Symbol header */
   SGUCHAR b[40];       /* Symbol data, "variable length" */
   } GCODE FCODE Totalizersym[96] =
   {
   #include "Totalizer.sym" /* Include symbols */
   };

/* Font structure */
GCODE GFONT FCODE Totalizer =
   {
   11,       /* width */
   20,       /* height */
   sizeof(GSYMDAT40),    /* number of bytes in a symbol  (must include any alignment padding)*/
   (PGSYMBOL)Totalizersym, /* pointer to array of SYMBOLS */
   96,      /* num symbols */
   (PGCODEPAGE)&Totalizercp
   };

