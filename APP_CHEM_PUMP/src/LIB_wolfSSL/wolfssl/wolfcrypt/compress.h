/* compress.h
 *
 * Copyright (C) 2006-2018 wolfSSL Inc.  All rights reserved.
 *
 * This file is part of wolfSSL.
 *
 * Contact licensing@wolfssl.com with any questions or comments.
 *
 * https://www.wolfssl.com
 */


/*!
    \file wolfssl/wolfcrypt/compress.h
*/


#ifndef WOLF_CRYPT_COMPRESS_H
#define WOLF_CRYPT_COMPRESS_H

#include <wolfssl/wolfcrypt/types.h>

#ifdef HAVE_LIBZ

#ifdef __cplusplus
    extern "C" {
#endif


#define COMPRESS_FIXED 1

#define LIBZ_WINBITS_GZIP 16


WOLFSSL_API int wc_Compress(byte*, wolf_word32, const byte*, wolf_word32, wolf_word32);
WOLFSSL_API int wc_Compress_ex(byte* out, wolf_word32 outSz, const byte* in,
    wolf_word32 inSz, wolf_word32 flags, wolf_word32 windowBits);
WOLFSSL_API int wc_DeCompress(byte*, wolf_word32, const byte*, wolf_word32);
WOLFSSL_API int wc_DeCompress_ex(byte* out, wolf_word32 outSz, const byte* in,
    wolf_word32 inSz, int windowBits);

#ifdef __cplusplus
    } /* extern "C" */
#endif


#endif /* HAVE_LIBZ */
#endif /* WOLF_CRYPT_COMPRESS_H */

