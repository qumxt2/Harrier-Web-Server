#ifndef BUSSIM_H
#define BUSSIM_H
/***************************** bussim.c ************************************

   Definitions for the LCD bus simulator access functions.

   The definitions below can be modified to reflect a given access function
   implementation.

   RAMTEX Engineering Aps 2001

****************************************************************************/

/* Simulated port adresses */
#define GHWWR  0x1
#define GHWRD  0x1
#define GHWSTA 0x0
#define GHWCMD 0x0

/*************** Do not modify the definitions below ********************/
#include <sgtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Single chip mode -> access via user supplied access driver functions */
void simwrby( SGUCHAR address, SGUCHAR dat);
SGUCHAR simrdby( SGUCHAR address );
void sim_reset( void );

#ifdef __cplusplus
}
#endif

#endif /* BUSSIM_H */
