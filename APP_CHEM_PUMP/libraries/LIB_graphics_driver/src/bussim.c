/***************************** bussim.c ************************************

   Generic template file for external bus simulator drivers.

   The functions bodies below must be implemented as described in the comments.
   Use the proper I/O port instructions given by the actual processor
   architecture, the actual LCD connection method and the I/O access
   capabilities of the target C compiler.

   Alternatively this module can be used as a universal stub during initial
   test compilation of the target software.

   The functions in this module is called by single-chip-processor version
   of the ghwinit.c module.

   Copyright (c) RAMTEX Engineering Aps 1998

****************************************************************************/
#ifndef GHW_NOHDW
#ifdef  GHW_SINGLE_CHIP

#include <bussim.h> 

/*#include < Port declaration file for your compiler > */

//******************************************************************************************************
// The following code is added by Graco - 

#include "p32mx795f512l.h"
//#include "ppic32mx.h"

//Define a 100ns delay at 40 MIPS
//#define DELAY_100ns()	(for (int i=0; i < 40 ; i++)Nop()) //Nop(); Nop(); Nop(); Nop()
#define DELAY_100ns()	Nop(); Nop(); Nop(); Nop()

// Define a 250ns delay at 40 MIPS
//#define DELAY_250ns()	(for (int i=0; i < 100 ; i++) Nop()) //Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop()
#define DELAY_250ns()	Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop()

/*
// Define constants used to represent the I/O state registers for the LCD communication lines
#define DATABUS_IOSTATE		TRISE
#define RD_IOSTATE			_TRISD7
#define WR_IOSTATE			_TRISD4
#define A0_IOSTATE			_TRISD5
#define RES_IOSTATE			_TRISD14
#define CS1_IOSTATE			_TRISD6 

// Define constants used to represent the data registers and bit fields associated with the 
// LCD communication lines
#define DATABUS_LATCH		LATE		// Used for a write to the data bus
#define DATABUS_PORT		PORTE		// Used for a read from the data bus
#define RD					_LATD7		// Active low sets the data bus to an output from the LCD
#define WR					_LATD4		// Data bus values are latched on the rising edge
#define A0					_LATD5		// Active high denotes display data, low denotes control data
#define RES					_LATD14		// Pulse active low for 1uSecond to cause an LCD reset
#define CS1					_LATD6		// Active low activates the chip select
*/
// Define constants used to represent the I/O state registers for the LCD communication lines
#define DATABUS_IOSTATE		TRISE
#define RD_IOSTATE			_TRISB11
#define WR_IOSTATE			_TRISB12
#define A0_IOSTATE			_TRISB13
#define RES_IOSTATE			_TRISB15
#define CS1_IOSTATE			_TRISB14 

// Define constants used to represent the data registers and bit fields associated with the 
// LCD communication lines
#define DATABUS_LATCH		LATE		// Used for a write to the data bus
#define DATABUS_PORT		PORTE		// Used for a read from the data bus
#define RD					_LATB11		// Active low sets the data bus to an output from the LCD
#define WR					_LATB12		// Data bus values are latched on the rising edge
#define A0					_LATB13		// Active high denotes display data, low denotes control data
#define RES					_LATB15		// Pulse active low for 1uSecond to cause an LCD reset
#define CS1					_LATB14		// Active low activates the chip select
// End of Graco added code
//******************************************************************************************************

/*
   Simulate a bus write operation for a LCD controller with an Intel
   like bus interface (i.e. use of separate /RD and /WR strobes).

   The address parameter adr is assumed to be either 0 or 1.
*/
void simwrby(SGUCHAR adr, SGUCHAR dat)
   {
   /* A: Set C/D line according to adr, Set /CE line active low */
   
   // The following code is added by Graco - 
   A0 = adr;
   CS1 = 0;
   // End of Graco added code
   
   /* B1: Make data port an output (if required by port architecture) */
   
   // The following code is added by Graco - 
   DATABUS_IOSTATE = DATABUS_IOSTATE & 0xFF00;
   // End of Graco added code
   
   /* B2: Write data to data port */
   
   // The following code is added by Graco - 
   DATABUS_LATCH = (DATABUS_LATCH & 0xFF00) | (SGUINT)dat;
   // End of Graco added code
   
   /* C: Set /WR active low, (Delay min 80 ns), */
   
   // The following code is added by Graco - 
   WR = 0;
   DELAY_100ns();
   // End of Graco added code
   
   /* D: Set /WR passive high */
   
   // The following code is added by Graco -
   WR = 1; 
   // End of Graco added code
   
   /* E: Set /CE passive high */
   
   // The following code is added by Graco - 
   CS1 = 1;
   // End of Graco added code
   
   }

/*
   Simulate a bus read operation for a LCD controller with an Intel
   like bus interface (i.e. use of separate /RD and /WR strobes).

   The address parameter adr is assumed to be either 0 or 1.
*/
SGUCHAR simrdby(SGUCHAR adr)
   {
   SGUCHAR dat = 0;
   /* a: Set C/D line according to adr. Set /CE line active low */
   
   // The following code is added by Graco - 
   A0 = adr;
   CS1 = 0;   
   // End of Graco added code
   
   /* b: Make data port an input (if required by port architecture) */
   
   // The following code is added by Graco - 
   DATABUS_IOSTATE = (DATABUS_IOSTATE & 0xFF00) | (SGUINT)0x00FF;
   // End of Graco added code
   
   /* c: Set /RD active low, (Delay min 150ns), */
   
   // The following code is added by Graco - 
   RD = 0;
   DELAY_100ns();
   DELAY_100ns();
   // End of Graco added code
   
   /* d: Read data from data port */
   
   // The following code is added by Graco - 
   dat = (SGUCHAR)(DATABUS_PORT & 0x00FF);
   // End of Graco added code
   
   /* e1:Set /RD passive high, */
   
   // The following code is added by Graco - 
   RD = 1;
   // End of Graco added code
   
   /* e2:Set /CE passive high (could be ignored) */
   
   // The following code is added by Graco - 
   CS1 = 1;
   // End of Graco added code
   
   return dat;
   }

/*
  Initialize and reset LCD display.
  Is called before simwrby() and simrdby() is invoked for the first time

  The SED1335 reset line is toggled here if it connected to a bus port.
  (it may alternatively be hard-wired to the reset signal to the processors
  in the target system).

  The sim_reset() function is invoked automatically via the ginit() function.
*/
void sim_reset( void )
   {
	int i;
   /* 1. Init data port setup (if required by port architecture) */
   
   SGUINT currentRegisterConfig;
   
   // The following code is added by Graco -
   // Set PORTE bits 0 through 7 (multiplexed with AN24 through AN31)
   // to digital mode and enable the port read functionality.
   
	//currentRegisterConfig = AD1PCFGH;
   	//AD1PCFGH = currentRegisterConfig | 0b1111111100000000;

   // Set initial LCD communication line values
   DATABUS_LATCH = 0x0000;
   RD = 1;
   WR = 0;
   A0 = 0;
   RES = 1;						 
   CS1 = 1;
   // End of Graco added code
   
   /* 2. Make C/D, /RD, /WR, /CE to outputs (if required by port architecture) */
   
   // The following code is added by Graco - 
   // RES also set to an output
   
   A0_IOSTATE = 0;
   RD_IOSTATE = 0;
   WR_IOSTATE = 0;
   CS1_IOSTATE = 0;
   RES_IOSTATE = 0;
   
   // End of Graco added code
   
   /* 3. Set LCD reset line /RST active low   (if /RST is connected to a port bit) */
   
   // The following code is added by Graco -
   // Originally reset for 4 cycles of DELAY_250ns. Not reliable.
   RES = 0;
   for(i = 0; i < 320; i++) {
      DELAY_250ns();
   }
   // End of Graco added code
   
   /* 4. Set LCD reset line /RST passive high (if /RST is connected to a port bit) */
   
   // The following code is added by Graco -
   RES = 1;
   for(i = 0; i < 320; i++) {
      DELAY_250ns();
   }
   // End of Graco added code
   
   }

#endif /* GHW_SINGLE_CHIP */
#endif /* GHW_NOHDW */



