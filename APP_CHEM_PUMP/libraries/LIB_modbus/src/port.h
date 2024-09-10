/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: port.h,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

#ifndef _PORT_H
#define _PORT_H

//#include "assert.h"
//#include "inttypes.h"
//#include "GenericTypeDefs.h"
#include "Cpfuncs.h"


#include "typedef.h"
#include "stdio.h"

//#include "plib.h"

#define	INLINE                      inline
#define PR_BEGIN_EXTERN_C           extern "C" {
#define	PR_END_EXTERN_C             }

#define ENTER_CRITICAL_SECTION( )   K_OS_Disable_Interrupts()//INTDisableInterrupts()
#define EXIT_CRITICAL_SECTION( )    K_OS_Enable_Interrupts()//INTEnableSystemMultiVectoredInt()


//#define ChkAssert(x)				if(!x) printf("Modbus Assert\n");
//#define ChkAssert(x) fprintf(stderr, "%s:%d:%s(): " x, __FILE__, __LINE__, __func__, __VA_ARGS__)
//#define ChkAssert(x) if(!x) printf("%s:%u", __FILE__, __LINE__)
#define ChkAssert(x) //if(!x) printf("%d", __LINE__)


//typedef uint8_t bool;
//typedef bool bool;

typedef unsigned char uchar;

//typedef char char;

//typedef uint16_t uint16;
//typedef uint16 uint16;
//typedef int16_t sint16;
//typedef sint16 sint16;

//typedef uint32_t uint32;
//typedef int32_t sint32;
//typedef uint32 uint32;
//typedef sint32 sint32;

//#ifndef TRUE
//#define TRUE            1
//#endif

//#ifndef FALSE
//#define FALSE           0
//#endif

#define SYS_FREQ 				(40000000L)
#define DESIRED_BAUDRATE    	(115200)      //The desired BaudRate
#define PRESCALE       			64
#define TOGGLES_PER_SEC			1
#define T4_TICK       			(SYS_FREQ/PRESCALE/TOGGLES_PER_SEC)


#endif

