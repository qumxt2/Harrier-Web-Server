/* misc.h
 *
 * Copyright (C) 2006-2018 wolfSSL Inc.  All rights reserved.
 *
 * This file is part of wolfSSL.
 *
 * Contact licensing@wolfssl.com with any questions or comments.
 *
 * https://www.wolfssl.com
 */




#ifndef WOLF_CRYPT_MISC_H
#define WOLF_CRYPT_MISC_H


#include <wolfssl/wolfcrypt/types.h>


#ifdef __cplusplus
    extern "C" {
#endif


#ifdef NO_INLINE
WOLFSSL_LOCAL
wolf_word32 rotlFixed(wolf_word32, wolf_word32);
WOLFSSL_LOCAL
wolf_word32 rotrFixed(wolf_word32, wolf_word32);

WOLFSSL_LOCAL
wolf_word32 ByteReverseWord32(wolf_word32);
WOLFSSL_LOCAL
void   ByteReverseWords(wolf_word32*, const wolf_word32*, wolf_word32);

WOLFSSL_LOCAL
void XorWords(wolfssl_word*, const wolfssl_word*, wolf_word32);
WOLFSSL_LOCAL
void xorbuf(void*, const void*, wolf_word32);

WOLFSSL_LOCAL
void ForceZero(const void*, wolf_word32);

WOLFSSL_LOCAL
int ConstantCompare(const byte*, const byte*, int);

#ifdef WORD64_AVAILABLE
WOLFSSL_LOCAL
word64 rotlFixed64(word64, word64);
WOLFSSL_LOCAL
word64 rotrFixed64(word64, word64);

WOLFSSL_LOCAL
word64 ByteReverseWord64(word64);
WOLFSSL_LOCAL
void   ByteReverseWords64(word64*, const word64*, wolf_word32);
#endif /* WORD64_AVAILABLE */

#ifndef WOLFSSL_HAVE_MIN
    #if defined(HAVE_FIPS) && !defined(min) /* so ifdef check passes */
        #define min min
    #endif
    WOLFSSL_LOCAL wolf_word32 min(wolf_word32 a, wolf_word32 b);
#endif

#ifndef WOLFSSL_HAVE_MAX
    #if defined(HAVE_FIPS) && !defined(max) /* so ifdef check passes */
        #define max max
    #endif
    WOLFSSL_LOCAL wolf_word32 max(wolf_word32 a, wolf_word32 b);
#endif /* WOLFSSL_HAVE_MAX */


void c32to24(wolf_word32 in, word24 out);
void c16toa(word16 u16, byte* c);
void c32toa(wolf_word32 u32, byte* c);
void c24to32(const word24 u24, wolf_word32* u32);
void ato16(const byte* c, word16* u16);
void ato24(const byte* c, wolf_word32* u24);
void ato32(const byte* c, wolf_word32* u32);
wolf_word32 btoi(byte b);


WOLFSSL_LOCAL byte ctMaskGT(int a, int b);
WOLFSSL_LOCAL byte ctMaskGTE(int a, int b);
WOLFSSL_LOCAL byte ctMaskLT(int a, int b);
WOLFSSL_LOCAL byte ctMaskLTE(int a, int b);
WOLFSSL_LOCAL byte ctMaskEq(int a, int b);
WOLFSSL_LOCAL byte ctMaskNotEq(int a, int b);
WOLFSSL_LOCAL byte ctMaskSel(byte m, byte a, byte b);
WOLFSSL_LOCAL int  ctMaskSelInt(byte m, int a, int b);
WOLFSSL_LOCAL byte ctSetLTE(int a, int b);

#endif /* NO_INLINE */


#ifdef __cplusplus
    }   /* extern "C" */
#endif


#endif /* WOLF_CRYPT_MISC_H */

