/************************* GHWIOINI.C ******************************

   Inittialize and deinitialize target specific LCD resources.

   Creation date: 011111

   Revision date:
   Revision Purpose:

   Version number: 2.00
   Copyright (c) RAMTEX Engineering Aps 2001

*********************************************************************/
#ifndef GHW_NOHDW
//#include <sgio.h>      /* Portable I/O functions + hardware port def */
#endif
#include <gdisphw.h>

#ifdef GHW_SINGLE_CHIP
void sim_reset( void );
#endif

#ifdef GBASIC_INIT_ERR
/*
   ghw_io_init()

   This function is called once by ginit() via ghw_init() before any LCD
   controller registers is addressed. Any target system specific
   initialization like I/O port initialization can be placed here.

*/
void ghw_io_init(void)
   {
   #ifndef GHW_NOHDW
   #ifdef GHW_SINGLE_CHIP
   sim_reset();  /* Initiate LCD bus simulation ports */
   #endif

   /* Insert required target specific code here, if any */
   
   	// Done during the sim_reset() call performed above - Graco.

   /* Set LCD reset line /RST active low   (if /RST is connected to a port bit) */
   
   	// Done during the sim_reset() call performed above - Graco.
   	
   /* Set LCD reset line /RST passive high (if /RST is connected to a port bit) */
   
   	// Done during the sim_reset() call performed above - Graco.
   	
   #endif
   }

/*
  This function is called once by gexit() via ghw_exit() as the last operation
  after all LCD controller operations has stopped.
  Any target system specific de-initialization, like I/O port deallocation
  can be placed here. In most embedded systems this function can be empty.
*/
void ghw_io_exit(void)
   {
   #ifndef GHW_NOHDW
   /* Insert required code here, if any */
   
    // Nothing necessary - Graco.
      	
   #endif
   }

#endif /* GBASIC_INIT_ERR */
