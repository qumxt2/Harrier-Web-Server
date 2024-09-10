// version.h
 
// Copyright 2006
// Graco Inc., Minneapolis, MN
// All Rights Reserved

/*
** This file contains macros and functions for setting and accessing all 
** of the constants that are hardcoded into the VERSION_BLOCK of memory
**   (both the BOOTLOADER version block and APPLICATION version block 
**  - Refer to document xxxxxx for information on the standardized 
**      Graco AFTD Memory structure (as defined in project A110M).
**  - Refer to document xxxxxx for proper usage of the VERSION constants.
**
** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
** ! Use only the precompiled 'version.a' library to access these values  ! 
** ! within an application.  Attempting to access the variables directly  ! 
** ! will NOT generate proper results.                                    !
** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#ifndef VERSION_H
#define VERSION_H


#include "typedef.h"

uint8 VersionGetVersionString (uint8 *versionBuffer, uint8 bufferLength);
uint16 VersionGetMajorVersion (void);
uint16 VersionGetMinorVersion (void);
uint16 VersionGetBuildVersion (void);

uint8 VersionGetBuildDateString (uint8 *dateBuffer, uint8 bufferLength);
void VersionGetBuildDateNumeric (uint8 *month, uint8 *day, uint16 *year);

uint8 VersionGetBuildTimeString (uint8 *timeBuffer, uint8 bufferLength);
void VersionGetBuildTimeNumeric (uint8 *hour, uint8 *minute, uint8 *second);

uint8 VersionGetPartNoString (uint8 *partNoBuffer, uint8 bufferLength);

uint8 VersionGetCopyrightString (uint8 *copyrightBuffer, uint8 bufferLength);


uint8 VersionGetVersionString_BOOT (uint8 *versionBuffer, uint8 bufferLength);
uint16 VersionGetMajorVersion_BOOT (void);
uint16 VersionGetMinorVersion_BOOT (void);
uint16 VersionGetBuildVersion_BOOT (void);

uint8 VersionGetBuildDateString_BOOT (uint8 *dateBuffer, uint8 bufferLength);
void VersionGetBuildDateNumeric_BOOT (uint8 *month, uint8 *day, uint16 *year);

uint8 VersionGetBuildTimeString_BOOT (uint8 *timeBuffer, uint8 bufferLength);
void VersionGetBuildTimeNumeric_BOOT (uint8 *hour, uint8 *minute, uint8 *second);

uint8 VersionGetPartNoString_BOOT (uint8 *partNoBuffer, uint8 bufferLength);


/*
** Refer to document xxxxxx for information on standardized 
** Graco AFTD Memory structure (as defined in project A110M)
*/
//lint -emacro(19, _MAJOR_VERSION)
#define _MAJOR_VERSION(x) __attribute__((section("__APP_MAJVERS.sec"))) \
const uint16 appMajorVersion = (x);
//lint -emacro(19, _MINOR_VERSION)
#define _MINOR_VERSION(x) __attribute__((section("__APP_MINVERS.sec"))) \
const uint16 appMinorVersion = (x);
//lint -emacro(19, _BUILD_VERSION)
#define _BUILD_VERSION(x) __attribute__((section("__APP_BLDVERS.sec"))) \
const uint16 appBuildVersion = (x);
//lint -emacro(19, _BUILD_DATE)
#define _BUILD_DATE() __attribute__((section("__APP_BLDDATE.sec"))) \
const uint8 appBuildDate[] = {__DATE__};
//lint -emacro(19, _BUILD_TIME)
#define _BUILD_TIME() __attribute__((section("__APP_BLDTIME.sec"))) \
const uint8 appBuildTime[] = {__TIME__};
//lint -emacro(19, _COPYRIGHT_STR)
#define _COPYRIGHT_STR(x) __attribute__((section("__APP_CPYRSTR.sec"))) \
const uint8 copyrightStr[] = {x};

//lint -emacro(19, _SOFTWARE_PARTNO)
#define _SOFTWARE_PARTNO(x) __attribute__((section("__APP_SOFTPN.sec"))) \
const uint8 app_partno[] = {x};

//lint -emacro(19, _MAJOR_VERSION_BOOT)
#define _MAJOR_VERSION_BOOT(x) __attribute__((section("__BOOT_MAJVERS.sec"))) \
const uint16 bootMajorVersion = (x);
//lint -emacro(19, _MINOR_VERSION_BOOT)
#define _MINOR_VERSION_BOOT(x) __attribute__((section("__BOOT_MINVERS.sec"))) \
const uint16 bootMinorVersion = (x);
//lint -emacro(19, _BUILD_VERSION_BOOT)
#define _BUILD_VERSION_BOOT(x) __attribute__((section("__BOOT_BLDVERS.sec"))) \
const uint16 bootBuildVersion = (x);
//lint -emacro(19, _BUILD_DATE_BOOT)
#define _BUILD_DATE_BOOT() __attribute__((section("__BOOT_BLDDATE.sec"))) \
const uint8 bootBuildDate[] = {__DATE__};
//lint -emacro(19, _BUILD_TIME_BOOT)
#define _BUILD_TIME_BOOT() __attribute__((section("__BOOT_BLDTIME.sec"))) \
const uint8 bootBuildTime[] = {__TIME__};

//lint -emacro(19, _SOFTWARE_PARTNO_BOOT)
#define _SOFTWARE_PARTNO_BOOT(x) __attribute__((section("__BOOT_SOFTPN.sec"))) \
const uint8 boot_partno[] = {x};

#endif /* _VERSION_H */
