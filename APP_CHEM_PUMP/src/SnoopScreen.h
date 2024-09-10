// SnoopScreen.h

// Copyright 2015 - 2017
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// Implements the header file for the modem snoop screen

#ifndef _SNOOPSCREEN_H_
#define _SNOOPSCREEN_H_

// **********************************************************************************************************
// Public functions
// **********************************************************************************************************

INPUT_MODE_t SnoopScreen(INPUT_EVENT_t InputEvent);
void SNOOP_putc(char c);

#endif
