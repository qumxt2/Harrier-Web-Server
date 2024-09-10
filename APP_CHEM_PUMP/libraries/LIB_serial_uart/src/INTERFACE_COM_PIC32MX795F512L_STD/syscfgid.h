// syscfgid.h
 
// Copyright 2006
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// DESCRIPTION

#ifndef SYSCFGID_H
#define SYSCFGID_H

#include "typedef.h"


typedef	uint32 SysConfigID_t;


#define SYS_CONFIG_ID_INVALID			(0xFFFFFFFFUL)
#define	SYS_COMPONENT_CLASS_ID_INVALID	(0xFF)
#define SYS_SOFTWARE_APP_ID_INVALID		(0xFFFF)
#define SYS_PURPOSE_ID_INVALID			(0xFF)


SysConfigID_t SYS_GetConfigurationID( void );

uint8	SYS_GetComponentClassID( void );
uint16 	SYS_GetSoftwareAppID( void );
uint8 	SYS_GetPurposeID( void );

uint8	SYS_StripComponentClassID( SysConfigID_t sysconfigid );
uint16	SYS_StripSoftwareAppID( SysConfigID_t sysconfigid );
uint8	SYS_StripPurposeID( SysConfigID_t sysconfigid );

#define _SYS_COMPONENT_CLASS_ID(x) __attribute__((section("__APP_COMPCLS.sec"))) \
								   const uint8 sysComponentClassID_ROM = (x);
#define _SYS_SOFTWARE_APP_ID(x) __attribute__((section("__APP_SOFTID.sec"))) \
						  		const uint16 sysSoftwareAppID_ROM = (x);
#define _SYS_PURPOSE_ID(x) __attribute__((section("__APP_PURPID.sec"))) \
						   const uint8 sysPurposeID_ROM = (x);

void SYS_SetComponentClassID(uint8 compclassid);
void SYS_SetSoftwareAppID(uint16 softwareappid);
void SYS_SetPurposeID(uint8 purposeid);


#endif // SYSCFGID_H

