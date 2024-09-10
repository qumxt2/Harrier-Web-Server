/* 
 * File:   rtos_internal.h
 * Author: qubkc3
 *
 * Created on August 8, 2012, 11:00 AM
 */

#ifndef RTOS_INTERNAL_H
#define	RTOS_INTERNAL_H

#ifdef	__cplusplus
extern "C" {
#endif

uint8 K_Task_Wait_PreRtosSafe( uint16 ticks );

uint8 K_Resource_Release_PreRtosSafe( uint8 resource );

uint8 K_Resource_Wait_PreRtosSafe( uint8 resource, uint16 time );


#ifdef	__cplusplus
}
#endif

#endif	/* RTOS_INTERNAL_H */

