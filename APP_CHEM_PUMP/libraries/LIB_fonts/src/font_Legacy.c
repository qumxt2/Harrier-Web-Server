/****************************  legacy.c ***********************

   ariel14 font table and code page structure definitions.
   This file has been auto-generated with the IconEdit / FontEdit tool.

   Copyright(c) RAMTEX 1998-2006

*****************************************************************/
#include <gdisphw.h>

/* Code page entry (one codepage range element) */
static struct
   {
   GCPHEAD chp;
   GCP_RANGE cpr[4];     /* Adjust this index if more codepage segments are added */
   }
GCODE FCODE legacycp =
   {
   #include "legacy.cp" /* Codepage table */
   };

typedef struct          /* Structure used for automatic word alignment */
   {
   SGUCHAR b[28];       /* Symbol data, "variable length" array */
   } GSYMDATa14;

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;         /* Symbol header */
   SGUCHAR b[28];       /* Symbol data, "variable length" */
   }
GCODE FCODE legacysym[39] =
   {
   #include "legacy.sym" /* Include symbols */
   };

/* Font structure */
GCODE GFONT FCODE legacy =
   {
   10,       /* width */
   14,       /* height */
   sizeof(GSYMDATa14),    /* number of bytes in a symbol  (must include any alignment padding)*/
   (PGSYMBOL)legacysym, /* pointer to array of SYMBOLS */
   39,      /* num symbols */
   (PGCODEPAGE)&legacycp
   };

