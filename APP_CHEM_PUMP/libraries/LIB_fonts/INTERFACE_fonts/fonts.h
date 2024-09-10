//! \file	fonts.h
//! \brief  ADCM Fonts Library
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//!
//! \addtogroup gca_adcm
//@{
//!
//! \addtogroup gca_adcm_fonts		ADCM Fonts Library
//! \brief ADCM (Advanced Display Module) Fonts Library
//!
//! \b DESCRIPTION:
//!    This module The defines for font usage
//!

#ifndef FONTS_H
#define FONTS_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "gdisphw.h"
#include "gdispcfg.h"

// ******************************************************************************************
// * MACROS & CONSTANTS
// ******************************************************************************************
#define XTREME_HEIGHT                                       (12) //pixels
#define TOTALIZER_HEIGHT                                    (20) //pixels

#define TITLE_FONT                                          (&Totalizer)
#define TITLE_FONT_HEIGHT                                   (TOTALIZER_HEIGHT)

#define STANDARD_FONT                                       (&Xtreme)
#define STANDARD_FONT_HEIGHT                                (XTREME_HEIGHT)

// ******************************************************************************************
// * PUBLIC VARIABLES
// ******************************************************************************************
extern GCODE GFONT FCODE Xtreme;
extern GCODE GFONT FCODE Totalizer;
extern GCODE GFONT FCODE legacy;

#endif //FONTS_H
