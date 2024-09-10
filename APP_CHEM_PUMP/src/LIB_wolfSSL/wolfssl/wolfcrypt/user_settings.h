/* 
 * File:   custom.h
 *
 * Custom WolfSSL options for the GCA project
 *
 */

#ifndef USER_SETTINGS_H
#define	USER_SETTINGS_H

#ifdef	__cplusplus
extern "C" {
#endif

// The WolfSSL types are alread defined in our project. Unfortunately, CMX defines wolf_word32
// incorrectly (it defines it as a 64-bit type, since longs are 64 bits on PIC32)
#include "Cpdefine.h"
typedef unsigned int wolf_word32;
// end type touchup


// Minimizing footprint
#define NO_OLD_TLS
#define NO_RC4
#define NO_MD4
#define NO_MD2
#define NO_DES3
#define NO_RABBIT
#define NO_MD5
#undef WOLFSSL_RIPEMD
#define NO_WOLFSSL_SERVER
#undef HAVE_HC128
#define NO_PWDBASED
#define NO_DH
#define NO_DSA
#define NO_PSK
#define WOLFSSL_SHA384
#define WOLFSSL_SHA512

// Need SNI to hit the web-based API
#define HAVE_SNI
#define HAVE_TLS_EXTENSIONS

// Reduce memory usage ~3KB
#define NO_SESSION_CACHE

// Slight reduciton in memory usage, but slightly slower
#define RSA_LOW_MEM

#define WOLFSSL_STATIC_RSA

// We don't care about cert expirations
#define NO_ASN_TIME

#define WOLFSSL_SMALL_STACK

// For porting to the GCA4400
#define USER_TICKS
#define DISABLE_CERTIFICATE_DATE_CHECK

// This will turn on all sorts of WolfSSL debug messages that will be
// sent to the debug portal
//#define DEBUG_WOLFSSL_MSG

#ifdef	__cplusplus
}
#endif

#endif	/* USER_SETTINGS_H */

