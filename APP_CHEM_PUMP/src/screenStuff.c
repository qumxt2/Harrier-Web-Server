// screenStuff.c

// Copyright 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// The c file for common screen functions

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************

#include "screenStuff.h"
#include "dvinterface_17G721.h"
#include "dvseg_17G721_setup.h"

//Shared Screen objects
__attribute__((section (".bss"))) SELECTION_BOX_t selectBox1 =
{
    0,                                  //index
    0,                                  //numberItems
    FALSE,                              //isEditMode
    FALSE,                              //isFocus
    FALSE,                              //isHidden
    FALSE,                              //isOutlined
    0,                                  //xPos
    0,                                  //yPos
    0,                                  //length
    NULL;                                //selectBoxText
};

__attribute__((section (".bss"))) SELECTION_BOX_t selectBox2 =
{
    0,                                  //index
    0,                                  //numberItems
    FALSE,                              //isEditMode
    FALSE,                              //isFocus
    FALSE,                              //isHidden
    FALSE,                              //isOutlined    
    0,                                  //xPos
    0,                                  //yPos
    0,                                  //length
    NULL;                                //selectBoxText
};

__attribute__((section (".bss"))) SELECTION_BOX_t selectBox3 =
{
    0,                                  //index
    0,                                  //numberItems
    FALSE,                              //isEditMode
    FALSE,                              //isFocus
    FALSE,                              //isHidden
    FALSE,                              //isOutlined
    0,                                  //xPos
    0,                                  //yPos
    0,                                  //length
    NULL;                                //selectBoxText
};

__attribute__((section (".bss"))) SELECTION_BOX_t selectBox4 =
{
    0,                                  //index
    0,                                  //numberItems
    FALSE,                              //isEditMode
    FALSE,                              //isFocus
    FALSE,                              //isHidden
    FALSE,                              //isOutlined
    0,                                  //xPos
    0,                                  //yPos
    0,                                  //length
    NULL;                                //selectBoxText
};

__attribute__((section (".bss"))) SELECTION_BOX_t selectBox5 =
{
    0,                                  //index
    0,                                  //numberItems
    FALSE,                              //isEditMode
    FALSE,                              //isFocus
    FALSE,                              //isHidden
    FALSE,                              //isOutlined    
    0,                                  //xPos
    0,                                  //yPos
    0,                                  //length
    NULL;                                //selectBoxText
};

__attribute__((section (".bss"))) DIGIT_BOX_t digitBox1 =
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) DIGIT_BOX_t digitBox2 =
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) DIGIT_BOX_t digitBox3 =
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) DIGIT_BOX_t digitBox4 =
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) DIGIT_BOX_t digitBox5 =
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) DIGIT_BOX_t digitBox6 =
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) DIGIT_BOX_t digitBox7 =
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) DIGIT_BOX_t digitBox8 =
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) DIGIT_BOX_t digitBox9 =
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) TIME_DIGIT_BOX_t timeBox1 = 
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

__attribute__((section (".bss"))) TIME_DIGIT_BOX_t timeBox2 = 
{
    FALSE,                      //isEditMode
    FALSE,                      //isFocus
    FALSE,                      //isHidden
    { 0 };
};

// **********************************************************************************************************
// selectBoxConfigure - Configures a text box with all the needed parameters
// **********************************************************************************************************

void selectBoxConfigure(SELECTION_BOX_t* pSelectBox, INT8U index, 
        INT8U numberItems, BOOLEAN isEditMode, BOOLEAN isFocus, BOOLEAN isHidden, BOOLEAN isOutlined, INT16U xPos, 
        INT16U yPos, INT16U length, char** selectBoxText)
{
    pSelectBox -> index = index;
    pSelectBox -> numberItems = numberItems;
    pSelectBox -> isEditMode = isEditMode;
    pSelectBox -> isFocus = isFocus;
    pSelectBox -> isHidden = isHidden;
    pSelectBox -> isOutlined = isOutlined;
    pSelectBox -> xPos = xPos;
    pSelectBox -> yPos = yPos;
    pSelectBox -> length = length;
    pSelectBox -> selectBoxText = selectBoxText;
}


// **********************************************************************************************************
// incrementSelectBox - Move focus to the next item in the selection box
// **********************************************************************************************************

void incrementSelectBox(SELECTION_BOX_t* pSelectBoxIndex)
{
    if( pSelectBoxIndex->index < (pSelectBoxIndex->numberItems - 1) )
    {
        pSelectBoxIndex->index++;
    }
    else
    {
        pSelectBoxIndex->index = 0;
    }
}

// **********************************************************************************************************
// decrementSelectBox - Move focus to the previous item in the selection box
// **********************************************************************************************************

void decrementSelectBox(SELECTION_BOX_t* pSelectBoxIndex)
{
    if( pSelectBoxIndex->index > 0 )
    {
        pSelectBoxIndex->index--;
    }
    else
    {
        pSelectBoxIndex->index = pSelectBoxIndex->numberItems - 1;
    }
}


// **********************************************************************************************************
// drawSelectBox - Draw the selection box on the screen
// **********************************************************************************************************

void drawSelectBox(SELECTION_BOX_t* pSelectBox)
{
    if( pSelectBox->isHidden == FALSE)
    {
        if( pSelectBox->isEditMode == FALSE )
        {
            gsetcpos(pSelectBox->xPos, pSelectBox->yPos);
            gputs(pSelectBox->selectBoxText[pSelectBox->index]);
            if( pSelectBox->isFocus == TRUE || pSelectBox->isOutlined == TRUE )
            {
                DrawBox(pSelectBox->xPos, pSelectBox->yPos, pSelectBox->length);
                gsetcpos(pSelectBox->xPos + pSelectBox->length, pSelectBox->yPos);
                ShowPullDownCharacter();
            }
        }
        else
        {
            INT8U i;

            DrawSelectBox(pSelectBox->xPos, pSelectBox->yPos, pSelectBox->length, pSelectBox->numberItems);

            for( i = 0; i < pSelectBox->numberItems; i++ )
            {
                PopulateSelectBox(i, pSelectBox->selectBoxText[i]);
            }
            SelectSelectBox(pSelectBox->index);
        }
    }
}

//****************************************************************************//
//Fcn: anySelectBoxIsEdit
//
//Desc: returns true if any of the select boxes are in edit mode
//****************************************************************************//
BOOLEAN anySelectBoxIsEdit(void)
{
    return(selectBox1.isEditMode | selectBox2.isEditMode | selectBox3.isEditMode | selectBox4.isEditMode | selectBox5.isEditMode);
}

//****************************************************************************//
//Fcn: clearAllSelectBoxIsEdit
//
//Desc: Clears all the edit select box flags
//****************************************************************************//
void clearAllSelectBoxIsEdit(void)
{
    selectBox1.isEditMode = FALSE;
    selectBox2.isEditMode = FALSE;
    selectBox3.isEditMode = FALSE;
    selectBox4.isEditMode = FALSE;
    selectBox5.isEditMode = FALSE;
}

//****************************************************************************//
//Fcn: clearAllSelectBoxIsFocus
//
//Desc: Clears all the focus select box flags
//****************************************************************************//
void clearAllSelectBoxIsFocus(void)
{
    selectBox1.isFocus = FALSE;
    selectBox2.isFocus = FALSE;
    selectBox3.isFocus = FALSE;
    selectBox4.isFocus = FALSE;
    selectBox5.isFocus = FALSE;
}

//****************************************************************************//
//Fcn: hideAllSelectBoxes
//
//Desc: Sets all the hidden select box flags
//****************************************************************************//
void hideAllSelectBoxes(void)
{
    selectBox1.isHidden = TRUE;
    selectBox2.isHidden = TRUE;
    selectBox3.isHidden = TRUE;
    selectBox4.isHidden = TRUE;
    selectBox5.isHidden = TRUE;
}

//****************************************************************************//
//Fcn: upEventForSelectBox
//
//Desc: Check for which select box can have an up event processed on it, and 
//  preforms the operation. Returns true if event is acted upon
//****************************************************************************//
BOOLEAN upEventForSelectBox(void)
{
    BOOLEAN changeOccurred = FALSE;
    
    if(selectBox1.isEditMode == TRUE )
    {
        decrementSelectBox(&selectBox1);
        changeOccurred = TRUE;
    }
    else if(selectBox2.isEditMode == TRUE )
    {
        decrementSelectBox(&selectBox2);
        changeOccurred = TRUE;
    }
    else if(selectBox3.isEditMode == TRUE )
    {
        decrementSelectBox(&selectBox3);
        changeOccurred = TRUE;
    }
    else if(selectBox4.isEditMode == TRUE)
    {
        decrementSelectBox(&selectBox4);
        changeOccurred = TRUE;
    }
    else if(selectBox5.isEditMode == TRUE)
    {
        decrementSelectBox(&selectBox5);
        changeOccurred = TRUE;
    }    
    return changeOccurred;
}

//****************************************************************************//
//Fcn: downEventForSelectBox
//
//Desc: Check for which select box can have a down event processed on it, and 
//  preforms the operation. Returns true if event is acted upon
//****************************************************************************//
BOOLEAN downEventForSelectBox(void)
{
    BOOLEAN changeOccurred = FALSE;
    
    if(selectBox1.isEditMode == TRUE )
    {
        incrementSelectBox(&selectBox1);
        changeOccurred = TRUE;
    }
    else if(selectBox2.isEditMode == TRUE )
    {
        incrementSelectBox(&selectBox2);
        changeOccurred = TRUE;
    }
    else if(selectBox3.isEditMode == TRUE )
    {
        incrementSelectBox(&selectBox3);
        changeOccurred = TRUE;
    }
    else if(selectBox4.isEditMode == TRUE)
    {
        incrementSelectBox(&selectBox4);
        changeOccurred = TRUE;
    }
    else if(selectBox5.isEditMode == TRUE)
    {
        incrementSelectBox(&selectBox5);
        changeOccurred = TRUE;
    }    
    return changeOccurred;
}

//****************************************************************************//
//Fcn: drawAllSelectBoxes
//
//Desc: Draws all the non hidden time boxes
//****************************************************************************//
void drawAllSelectBoxes(void)
{
    if(selectBox1.isFocus)
    {
        drawSelectBox(&selectBox2);
        drawSelectBox(&selectBox3);
        drawSelectBox(&selectBox4);
        drawSelectBox(&selectBox5);
        drawSelectBox(&selectBox1);
    }
    else if(selectBox2.isFocus)
    {
        drawSelectBox(&selectBox1);
        drawSelectBox(&selectBox3);
        drawSelectBox(&selectBox4);
        drawSelectBox(&selectBox5);
        drawSelectBox(&selectBox2); 
    }
    else if(selectBox3.isFocus)
    {
        drawSelectBox(&selectBox1);
        drawSelectBox(&selectBox2);
        drawSelectBox(&selectBox4);
        drawSelectBox(&selectBox5);
        drawSelectBox(&selectBox3);
    }
    else if(selectBox4.isFocus)
    {
        drawSelectBox(&selectBox1);
        drawSelectBox(&selectBox2);
        drawSelectBox(&selectBox3);
        drawSelectBox(&selectBox5);
        drawSelectBox(&selectBox4);
    }    
    else
    {
        drawSelectBox(&selectBox1);
        drawSelectBox(&selectBox2);
        drawSelectBox(&selectBox3);
        drawSelectBox(&selectBox4); 
        drawSelectBox(&selectBox5);
    }
}

//****************************************************************************//
//Fcn: anyDigitBoxIsEdit
//
//Desc: This function returns true is any of the digit boxes are in edit mode
//****************************************************************************//
BOOLEAN anyDigitBoxIsEdit(void)
{
    return(digitBox1.isEditMode | digitBox2.isEditMode | digitBox3.isEditMode | digitBox4.isEditMode | digitBox5.isEditMode | digitBox6.isEditMode |
           digitBox7.isEditMode | digitBox8.isEditMode | digitBox9.isEditMode);
}

//****************************************************************************//
//Fcn: clearAllDigitBoxIsEdit
//
//Desc: This function clears all the digit boxes isEditMode
//****************************************************************************//
void clearAllDigitBoxIsEdit(void)
{
    digitBox1.isEditMode = FALSE;
    digitBox2.isEditMode = FALSE;
    digitBox3.isEditMode = FALSE;
    digitBox4.isEditMode = FALSE;
    digitBox5.isEditMode = FALSE;
    digitBox6.isEditMode = FALSE;  
    digitBox7.isEditMode = FALSE;  
    digitBox8.isEditMode = FALSE;  
    digitBox9.isEditMode = FALSE;  
}

//****************************************************************************//
//Fcn: clearAllDigitBoxIsFocus
//
//Desc: This function clears all the digit boxes isFocus
//****************************************************************************//
void clearAllDigitBoxIsFocus(void)
{
    digitBox1.isFocus = FALSE;
    digitBox2.isFocus = FALSE;
    digitBox3.isFocus = FALSE;
    digitBox4.isFocus = FALSE;
    digitBox5.isFocus = FALSE;
    digitBox6.isFocus = FALSE;
    digitBox7.isFocus = FALSE;
    digitBox8.isFocus = FALSE;
    digitBox9.isFocus = FALSE;
}

//****************************************************************************//
//Fcn: hideAllDigitBoxes
//
//Desc: Sets all the hidden digit box flags
//****************************************************************************//
void hideAllDigitBoxes(void)
{
    digitBox1.isHidden = TRUE;
    digitBox2.isHidden = TRUE;
    digitBox3.isHidden = TRUE;
    digitBox4.isHidden = TRUE;
    digitBox5.isHidden = TRUE;
    digitBox6.isHidden = TRUE;    
    digitBox7.isHidden = TRUE;    
    digitBox8.isHidden = TRUE;    
    digitBox9.isHidden = TRUE;        
}

//****************************************************************************//
//Fcn: upEventForDigitBox
//
//Desc: This function handles up events for the digit boxes. it returns true 
//  if the event is successfully handled
//****************************************************************************//
BOOLEAN upEventForDigitBox(void)
{
    BOOLEAN changeOccurred = FALSE;
    if(digitBox1.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox1.countDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox2.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox2.countDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox3.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox3.countDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox4.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox4.countDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox5.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox5.countDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox6.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox6.countDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }    
    else if(digitBox7.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox7.countDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }  
    else if(digitBox8.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox8.countDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }  
    else if(digitBox9.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox9.countDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }      
    return changeOccurred;
}

//****************************************************************************//
//Fcn: downEventForDigitBox
//
//Desc: This function handles down events for the digit boxes. it returns true 
//  if the event is successfully handled
//****************************************************************************//
BOOLEAN downEventForDigitBox(void)
{
    BOOLEAN changeOccurred = FALSE;
    if(digitBox1.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox1.countDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox2.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox2.countDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox3.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox3.countDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox4.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox4.countDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox5.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox5.countDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox6.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox6.countDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    }    
    else if(digitBox7.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox7.countDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    } 
    else if(digitBox8.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox8.countDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    } 
    else if(digitBox9.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox9.countDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    }     
    return changeOccurred;
}

//****************************************************************************//
//Fcn: leftEventForDigitBox
//
//Desc: This function handles left events for the digit boxes. it returns true 
//  if the event is successfully handled
//****************************************************************************//
BOOLEAN leftEventForDigitBox(void)
{
    BOOLEAN changeOccurred = FALSE;
    if(digitBox1.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox1.countDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox2.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox2.countDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox3.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox3.countDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox4.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox4.countDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox5.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox5.countDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox6.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox6.countDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox7.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox7.countDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox8.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox8.countDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox9.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox9.countDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }    
    return changeOccurred;
}

//****************************************************************************//
//Fcn: rightEventForDigitBox
//
//Desc: This function handles right events for the digit boxes. it returns true 
//  if the event is successfully handled
//****************************************************************************//
BOOLEAN rightEventForDigitBox(void)
{
    BOOLEAN changeOccurred = FALSE;
    if(digitBox1.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox1.countDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox2.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox2.countDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox3.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox3.countDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox4.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox4.countDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox5.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox5.countDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox6.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox6.countDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }    
    else if(digitBox7.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox7.countDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox8.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox8.countDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }
    else if(digitBox9.isEditMode == TRUE )
    {
        (void)ProcessCountDigitEvent(&digitBox9.countDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }    
    return changeOccurred;
}

//****************************************************************************//
//Fcn: drawDigitBox
//
//Desc: Draws a digit boxes
//****************************************************************************//
void drawDigitBox(DIGIT_BOX_t *dB)
{
    //only draw if the box is not hidden
    if(dB->isHidden == FALSE)
    {
        DisplayCountDigit(&(dB->countDigit));
        //draw a box around a single digit if the box is being edited 
        if(dB->isEditMode == TRUE)
        {
            DrawCountDigitDigitBox(&(dB->countDigit));
        }
        else if(dB->isFocus == TRUE)
        {
            DrawCountDigitBox(&(dB->countDigit));
        }
    }
}

//****************************************************************************//
//Fcn: drawAllDigitBoxes
//
//Desc: Draws all the non hidden digit boxes
//****************************************************************************//
void drawAllDigitBoxes(void)
{
    drawDigitBox(&digitBox1);
    drawDigitBox(&digitBox2);
    drawDigitBox(&digitBox3);
    drawDigitBox(&digitBox4);
    drawDigitBox(&digitBox5);
    drawDigitBox(&digitBox6);
    drawDigitBox(&digitBox7);
    drawDigitBox(&digitBox8);
    drawDigitBox(&digitBox9);    
}

//****************************************************************************//
//Fcn: anyTimeBoxIsEdit
//
//Desc: This function returns true is any of the Time boxes are in edit mode
//****************************************************************************//
BOOLEAN anyTimeBoxIsEdit(void)
{
    return(timeBox1.isEditMode | timeBox2.isEditMode);
}

//****************************************************************************//
//Fcn: clearAllTimeBoxIsEdit
//
//Desc: This function clears all the time boxes iiEditMode
//****************************************************************************//
void clearAllTimeBoxIsEdit(void)
{
    timeBox1.isEditMode = FALSE;
    timeBox2.isEditMode = FALSE;
}

//****************************************************************************//
//Fcn: clearAllTimeBoxIsFocus
//
//Desc: This function clears all the time boxes isFocus
//****************************************************************************//
void clearAllTimeBoxIsFocus(void)
{
    timeBox1.isFocus = FALSE;
    timeBox2.isFocus = FALSE;
}

//****************************************************************************//
//Fcn: hideAllTimeBoxes
//
//Desc: Sets all the hidden time box flags
//****************************************************************************//
void hideAllTimeBoxes(void)
{
    timeBox1.isHidden = TRUE;
    timeBox2.isHidden = TRUE;
}

//****************************************************************************//
//Fcn: drawTimeBox
//
//Desc: Draws a time box
//****************************************************************************//
void drawTimeBox(TIME_DIGIT_BOX_t *tB)
{
    //only draw if the box is not hidden
    if(tB->isHidden == FALSE)
    {
        DisplayTimeDigit(&(tB->timeDigit));
        //draw a box around a single digit if the box is being edited 
        if(tB->isEditMode == TRUE)
        {
            DrawTimeDigitDigitBox(&(tB->timeDigit));
        }
        else if(tB->isFocus == TRUE)
        {
            DrawTimeDigitBox(&(tB->timeDigit));
        }
    }
}

//****************************************************************************//
//Fcn: drawAllTimeBoxes
//
//Desc: Draws all the non hidden time boxes
//****************************************************************************//
void drawAllTimeBoxes(void)
{
    drawTimeBox(&timeBox1);
    drawTimeBox(&timeBox2);
}

//****************************************************************************//
//Fcn: hideAllBoxes
//
//Desc: Sets all the hidden flags
//****************************************************************************//
void hideAllBoxes(void)
{
    hideAllSelectBoxes();
    hideAllDigitBoxes();
    hideAllTimeBoxes();
}

//****************************************************************************//
//Fcn: clearAllIsFocus
//
//Desc: clears all the isFocus flags
//****************************************************************************//
void clearAllIsFocus(void)
{
    clearAllSelectBoxIsFocus();
    clearAllDigitBoxIsFocus();
    clearAllTimeBoxIsFocus();
}

//****************************************************************************//
//Fcn: clearAllIsEdit
//
//Desc: clears all the is edit flags
//****************************************************************************//
void clearAllIsEdit(void)
{
    clearAllSelectBoxIsEdit();
    clearAllDigitBoxIsEdit();
    clearAllTimeBoxIsEdit();
}
//****************************************************************************//
//Fcn: upEventForTimeBox
//
//Desc: This function handles up events for the time boxes. it returns true 
//  if the event is successfully handled
//****************************************************************************//
BOOLEAN upEventForTimeBox(void)
{
    BOOLEAN changeOccurred = FALSE;
    
    if(timeBox1.isEditMode == TRUE )
    {
        (void)ProcessTimeDigitEvent(&timeBox1.timeDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }
    else if(timeBox2.isEditMode == TRUE )
    {
        (void)ProcessTimeDigitEvent(&timeBox2.timeDigit, INPUT_EVENT_UP_ARROW);
        changeOccurred = TRUE;
    }
    return changeOccurred;
    
}

//****************************************************************************//
//Fcn: downEventForTimeBox
//
//Desc: This function handles down events for the time boxes. it returns true 
//  if the event is successfully handled
//****************************************************************************//
BOOLEAN downEventForTimeBox(void)
{
    BOOLEAN changeOccurred = FALSE;
    if(timeBox1.isEditMode == TRUE )
    {
        (void)ProcessTimeDigitEvent(&timeBox1.timeDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    }
    else if(timeBox2.isEditMode == TRUE )
    {
        (void)ProcessTimeDigitEvent(&timeBox2.timeDigit, INPUT_EVENT_DOWN_ARROW);
        changeOccurred = TRUE;
    }
    return changeOccurred;
}

//****************************************************************************//
//Fcn: leftEventForTimeBox
//
//Desc: This function handles left events for the time boxes. it returns true 
//  if the event is successfully handled
//****************************************************************************//
BOOLEAN leftEventForTimeBox(void)
{
    BOOLEAN changeOccurred = FALSE;
if( timeBox1.isEditMode == TRUE )
    {
        (void)ProcessTimeDigitEvent(&timeBox1.timeDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    else if( timeBox2.isEditMode == TRUE )
    {
        (void)ProcessTimeDigitEvent(&timeBox2.timeDigit, INPUT_EVENT_LEFT_ARROW);
        changeOccurred = TRUE;
    }
    return changeOccurred;
}

//****************************************************************************//
//Fcn: rightEventForTimeBox
//
//Desc: This function handles right events for the time boxes. it returns true 
//  if the event is successfully handled
//****************************************************************************//
BOOLEAN rightEventForTimeBox(void)
{
    BOOLEAN changeOccurred = FALSE;
    
    if( timeBox1.isEditMode == TRUE )
    {
        (void)ProcessTimeDigitEvent(&timeBox1.timeDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }
    else if( timeBox2.isEditMode == TRUE )
    {
        (void)ProcessTimeDigitEvent(&timeBox2.timeDigit, INPUT_EVENT_RIGHT_ARROW);
        changeOccurred = TRUE;
    }
    
    return changeOccurred;
}

//****************************************************************************//
//Fcn: processInputRightArrowEvent
//
//Desc: This function processes the right arrow events
//****************************************************************************//
void processInputRightArrowEvent(void)
{
    if(rightEventForDigitBox() == FALSE)
    {
        (void)rightEventForTimeBox();
    }
}

//****************************************************************************//
//Fcn: processInputLeftEvent
//
//Desc: This function processes the left arrow events
//****************************************************************************//
void processInputLeftArrowEvent(void)
{
    if(leftEventForDigitBox() == FALSE)
    {
        (void)leftEventForTimeBox();
    }
}

//****************************************************************************//
//Fcn: processInputDefaultEvent
//
//Desc: This handles all other possible events, not a button do nothing
//****************************************************************************//
void processInputDefaultEvent(void)
{
    asm("nop");
}

// **********************************************************************************************************
// incrementGenericFocusIndex - Move focus to the next field
// **********************************************************************************************************
uint16 incrementGenericFocusIndex(uint16 focusIndex, uint16 numberItems)
{
    if( focusIndex < (numberItems - 1) )
    {
        return focusIndex + 1;
    }
    else
    {
        return 0;
    }
}

// **********************************************************************************************************
// decrementGenericFocusIndex - Move focus to the previous field
// **********************************************************************************************************
uint16 decrementGenericFocusIndex(uint16 focusIndex, uint16 numberItems)
{
    if( focusIndex > 0 )
    {
        return focusIndex - 1;
    }
    else
    {
        return (numberItems - 1);
    }
}


// **********************************************************************************************************
// validateVertHorzInput - Validate the input for the horizontal and vertical modes.  Assumes inputs are
//                         in gallons & are in the format XXX.XX * 100
// **********************************************************************************************************
void validateVertHorzInput(uint32 maxVolume, uint32 fillVolume, uint32 sensorVolume)
{   
    if(maxVolume == 0)
    {
        maxVolume = gSetup.MaxTankVolume;
    }
    if(maxVolume < fillVolume)  //Entered fill volume cannot be larger than the entered max volume
    {
        fillVolume = maxVolume;
    }
    if(fillVolume < sensorVolume)   //sensor volume must be less than or equal to the fill volume
    {
        fillVolume = gSetup.TankFillVolume;     //no change for invalid fill volume
        sensorVolume = gSetup.TankSensorVolume;
    }
    (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, MaxTankVolume), maxVolume);
    (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, TankFillVolume), fillVolume);
    (void)DVAR_SetPointLocal_wRetry(DVA17G721_SS(gSetup, TankSensorVolume), sensorVolume);    
}


