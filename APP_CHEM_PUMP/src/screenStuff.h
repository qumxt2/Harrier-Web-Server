// screenStuff.h

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// The header file for screen variables and functions

#ifndef _SCREEN_STUFF_H    /* Guard against multiple inclusion */
#define _SCREEN_STUFF_H

#include "CountDigit.h"
#include "typedef.h"
#include "TimeDigit.h"

typedef struct
{
    INT8U           index;
    INT8U           numberItems;
    BOOLEAN         isEditMode;
    BOOLEAN         isFocus;
    BOOLEAN         isHidden;
    BOOLEAN         isOutlined;
    INT16U          xPos;
    INT16U          yPos;
    INT16U          length;
    char**          selectBoxText;
} SELECTION_BOX_t;

typedef struct
{
    BOOLEAN         isEditMode;
    BOOLEAN         isFocus;
    BOOLEAN         isHidden;
    COUNT_DIGIT_t   countDigit;
} DIGIT_BOX_t;

typedef struct
{
    BOOLEAN         isEditMode;
    BOOLEAN         isFocus;
    BOOLEAN         isHidden;
    TIME_DIGIT_t    timeDigit;
} TIME_DIGIT_BOX_t;

typedef struct
{
    BOOLEAN         isEditMode;
    BOOLEAN         isFocus;
    BOOLEAN         isHidden;
    COUNT_DIGIT_t   countDigit;
} COUNT_DIGIT_BOX_t;


//Function Prototypes
void incrementSelectBox(SELECTION_BOX_t* pSelectBoxIndex);
void decrementSelectBox(SELECTION_BOX_t* pSelectBoxIndex);
void drawSelectBox(SELECTION_BOX_t* pSelectBox);
void selectBoxConfigure(SELECTION_BOX_t* pSelectBox, INT8U index, 
        INT8U numberItems, BOOLEAN isEditMode, BOOLEAN isFocus, BOOLEAN isHidden, BOOLEAN isOutlined, INT16U xPos, 
        INT16U yPos, INT16U length, char** selectBoxText);

BOOLEAN anySelectBoxIsEdit(void);
void clearAllSelectBoxIsEdit(void);
void clearAllSelectBoxIsFocus(void);
void hideAllSelectBoxes(void);
BOOLEAN upEventForSelectBox(void);
BOOLEAN downEventForSelectBox(void);
void drawAllSelectBoxes(void);

BOOLEAN anyDigitBoxIsEdit(void);
void clearAllDigitBoxIsEdit(void);
void clearAllDigitBoxIsFocus(void);
void hideAllDigitBoxes(void);
BOOLEAN upEventForDigitBox(void);
BOOLEAN downEventForDigitBox(void);
BOOLEAN leftEventForDigitBox(void);
BOOLEAN rightEventForDigitBox(void);
void drawDigitBox(DIGIT_BOX_t *);
void drawAllDigitBoxes(void);

BOOLEAN anyTimeBoxIsEdit(void);
void clearAllTimeBoxIsEdit(void);
void clearAllTimeBoxIsFocus(void);
void hideAllTimeBoxes(void);
BOOLEAN upEventForTimeBox(void);
BOOLEAN downEventForTimeBox(void);
BOOLEAN leftEventForTimeBox(void);
BOOLEAN rightEventForTimeBox(void);
void drawTimeBox(TIME_DIGIT_BOX_t *);
void drawAllTimeBoxes(void);

void hideAllBoxes(void);
void clearAllIsFocus(void);
void clearAllIsEdit(void);
//Function prototypes for button presses that can be shared
void processInputRightArrowEvent(void);
void processInputLeftArrowEvent(void);
void processInputDefaultEvent(void);

uint16 incrementGenericFocusIndex(uint16 focusIndex, uint16 numberItems);
uint16 decrementGenericFocusIndex(uint16 focusIndex, uint16 numberItems);

void validateVertHorzInput(uint32 maxVolume, uint32 fillVolume, uint32 sensorVolume);

extern SELECTION_BOX_t __attribute__((section(".bss"))) selectBox1, selectBox2, selectBox3, selectBox4, selectBox5;
extern DIGIT_BOX_t __attribute__((section(".bss"))) digitBox1, digitBox2, digitBox3, digitBox4, digitBox5, digitBox6, digitBox7, digitBox8, digitBox9;
extern TIME_DIGIT_BOX_t __attribute__((section(".bss"))) timeBox1, timeBox2;

#endif
