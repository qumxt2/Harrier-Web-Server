/* dh.h
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
    \file wolfssl/wolfcrypt/dh.h
*/

#ifndef WOLF_CRYPT_DH_H
#define WOLF_CRYPT_DH_H

#include <wolfssl/wolfcrypt/types.h>

#ifndef NO_DH

#if defined(HAVE_FIPS) && \
    defined(HAVE_FIPS_VERSION) && (HAVE_FIPS_VERSION >= 2)
    #include <wolfssl/wolfcrypt/fips.h>
#endif /* HAVE_FIPS_VERSION >= 2 */

#include <wolfssl/wolfcrypt/integer.h>
#include <wolfssl/wolfcrypt/random.h>

#ifdef __cplusplus
    extern "C" {
#endif

#ifdef WOLFSSL_ASYNC_CRYPT
    #include <wolfssl/wolfcrypt/async.h>
#endif
typedef struct DhParams {
    #ifdef HAVE_FFDHE_Q
    const byte* q;
    wolf_word32      q_len;
    #endif /* HAVE_FFDHE_Q */
    const byte* p;
    wolf_word32      p_len;
    const byte* g;
    wolf_word32      g_len;
} DhParams;

/* Diffie-Hellman Key */
typedef struct DhKey {
    mp_int p, g, q;                         /* group parameters  */
    void* heap;
#ifdef WOLFSSL_ASYNC_CRYPT
    WC_ASYNC_DEV asyncDev;
#endif
} DhKey;


#ifdef HAVE_FFDHE_2048
WOLFSSL_API const DhParams* wc_Dh_ffdhe2048_Get(void);
#endif
#ifdef HAVE_FFDHE_3072
WOLFSSL_API const DhParams* wc_Dh_ffdhe3072_Get(void);
#endif
#ifdef HAVE_FFDHE_4096
WOLFSSL_API const DhParams* wc_Dh_ffdhe4096_Get(void);
#endif
#ifdef HAVE_FFDHE_6144
WOLFSSL_API const DhParams* wc_Dh_ffdhe6144_Get(void);
#endif
#ifdef HAVE_FFDHE_8192
WOLFSSL_API const DhParams* wc_Dh_ffdhe8192_Get(void);
#endif

WOLFSSL_API int wc_InitDhKey(DhKey* key);
WOLFSSL_API int wc_InitDhKey_ex(DhKey* key, void* heap, int devId);
WOLFSSL_API int wc_FreeDhKey(DhKey* key);

WOLFSSL_API int wc_DhGenerateKeyPair(DhKey* key, WC_RNG* rng, byte* priv,
                                 wolf_word32* privSz, byte* pub, wolf_word32* pubSz);
WOLFSSL_API int wc_DhAgree(DhKey* key, byte* agree, wolf_word32* agreeSz,
                       const byte* priv, wolf_word32 privSz, const byte* otherPub,
                       wolf_word32 pubSz);

WOLFSSL_API int wc_DhKeyDecode(const byte* input, wolf_word32* inOutIdx, DhKey* key,
                           wolf_word32);
WOLFSSL_API int wc_DhSetKey(DhKey* key, const byte* p, wolf_word32 pSz, const byte* g,
                        wolf_word32 gSz);
WOLFSSL_API int wc_DhSetKey_ex(DhKey* key, const byte* p, wolf_word32 pSz,
                        const byte* g, wolf_word32 gSz, const byte* q, wolf_word32 qSz);
WOLFSSL_API int wc_DhSetCheckKey(DhKey* key, const byte* p, wolf_word32 pSz,
                        const byte* g, wolf_word32 gSz, const byte* q, wolf_word32 qSz,
                        int trusted, WC_RNG* rng);
WOLFSSL_API int wc_DhParamsLoad(const byte* input, wolf_word32 inSz, byte* p,
                            wolf_word32* pInOutSz, byte* g, wolf_word32* gInOutSz);
WOLFSSL_API int wc_DhCheckPubKey(DhKey* key, const byte* pub, wolf_word32 pubSz);
WOLFSSL_API int wc_DhCheckPubKey_ex(DhKey* key, const byte* pub, wolf_word32 pubSz,
                            const byte* prime, wolf_word32 primeSz);
WOLFSSL_API int wc_DhCheckPrivKey(DhKey* key, const byte* priv, wolf_word32 pubSz);
WOLFSSL_API int wc_DhCheckPrivKey_ex(DhKey* key, const byte* priv, wolf_word32 pubSz,
                            const byte* prime, wolf_word32 primeSz);
WOLFSSL_API int wc_DhCheckKeyPair(DhKey* key, const byte* pub, wolf_word32 pubSz,
                        const byte* priv, wolf_word32 privSz);
WOLFSSL_API int wc_DhGenerateParams(WC_RNG *rng, int modSz, DhKey *dh);
WOLFSSL_API int wc_DhExportParamsRaw(DhKey* dh, byte* p, wolf_word32* pSz,
                       byte* q, wolf_word32* qSz, byte* g, wolf_word32* gSz);


#ifdef __cplusplus
    } /* extern "C" */
#endif

#endif /* NO_DH */
#endif /* WOLF_CRYPT_DH_H */

