//! \file	screen_bitmaps.h
//! \brief  Standard Bitmaps Library
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//!
//! \addtogroup gca_adcm
//@{
//!
//! \addtogroup gca_adcm_screen_bitmaps		ADCM Standard Bitmap Library
//! \brief ADCM (Advanced Display Module) Bitmap Library
//!
//! \b DESCRIPTION:
//!    This module provides a set of standard bitmap images used
//!    in the ADM (Advanced Display Module).

#ifndef SCREEN_BITMAPS_H
#define SCREEN_BITMAPS_H

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"
#include "gdisp.h"

// ******************************************************************************************
// * MACROS & CONSTANTS
// ******************************************************************************************
#define BMPSYM(x) typedef struct { GSYMHEAD sh; SGUCHAR b[x]; } BMPSYM##x;

BMPSYM(12)
BMPSYM(18)
BMPSYM(20)
BMPSYM(24)
BMPSYM(28)
BMPSYM(45)
BMPSYM(60)
BMPSYM(66)
BMPSYM(69)
BMPSYM(72)
BMPSYM(87)
BMPSYM(128)
BMPSYM(144)
BMPSYM(800)

// ******************************************************************************************
// * PUBLIC VARIABLES
// ******************************************************************************************
extern GCODE BMPSYM24 FCODE BMP_Screen_ArrowDown_12x12[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_ArrowUpDown_12x12[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_ArrowUp_12x12[1];
extern GCODE BMPSYM60 FCODE BMP_Screen_Arrow_Right_23x23[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_Blank_12x12[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_CheckMark_10x10[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_Dropdown_12x12[1];
extern GCODE BMPSYM69 FCODE BMP_Screen_Enter_23x23[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_Number_12x12[51];
extern GCODE BMPSYM24 FCODE BMP_Screen_PinLock_12x12[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_UpDown_12x12[1];
extern GCODE BMPSYM69 FCODE BMP_Screen_WindowIn_23x23[1];
extern GCODE BMPSYM69 FCODE BMP_Screen_WindowOut_23x23[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_Arrow_Down_Small_12x12[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_Arrow_Up_Small_12x12[1];
extern GCODE BMPSYM24 FCODE BMP_Screen_EventBell_12x12[3];
extern GCODE BMPSYM72 FCODE BMP_Screen_Date_Format_41x12[3];
extern GCODE BMPSYM12 FCODE BMP_Lightning_6x12[1];
extern GCODE BMPSYM24 FCODE BMP_Pressure_12x12[1];
extern GCODE BMPSYM128 FCODE BMP_Tank_10x17[1];
extern GCODE BMPSYM800 FCODE BMP_Screen_GracoLogo_74x80[1];
extern GCODE BMPSYM128 FCODE BMP_Screen_Modeicons_32x32[9];
extern GCODE BMPSYM144 FCODE BMP_Screen_pumpIcons_32x36[6];
extern GCODE BMPSYM128 FCODE BMP_Thermometer_9x14[1];

extern GCODE BMPSYM69 FCODE BMP_dcmCmpAppContinue_23x23[1];
extern GCODE BMPSYM69 FCODE BMP_dcmCmpAppEnter_23x23[1];
extern GCODE BMPSYM28 FCODE BMP_dcmCmpCheckNoCANBL_14x14[1];
extern GCODE BMPSYM28 FCODE BMP_dcmCmpCheckNoChange_14x14[1];
extern GCODE BMPSYM28 FCODE BMP_dcmCmpCheck_14x14[1];
extern GCODE BMPSYM18 FCODE BMP_dcmCmpRightArrow_11x9[1];
extern GCODE BMPSYM45 FCODE BMP_dcmCmpSerialNum_20x15[1];
extern GCODE BMPSYM60 FCODE BMP_dcmCmpSingleApp_20x20[1];
extern GCODE BMPSYM66 FCODE BMP_dcmCmpSingleApp_22x22[1];
extern GCODE BMPSYM87 FCODE BMP_dcmCmpToken_17x29[1];
extern GCODE BMPSYM69 FCODE BMP_dcmCmpToken_17x23[1];
extern GCODE BMPSYM28 FCODE BMP_dcmCmpX_14x14[1];

#endif      //! BITMAPS_H

//@}
