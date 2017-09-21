/* wolfssl options.h
 * generated from configure options
 *
 * Copyright (C) 2006-2015 wolfSSL Inc.
 *
 * This file is part of wolfSSL. (formerly known as CyaSSL)
 *
 */

#ifndef WOLFSSL_OPTIONS_H
#define WOLFSSL_OPTIONS_H


#ifdef __cplusplus
extern "C" {
#endif

#undef  OPENSSL_EXTRA
#define OPENSSL_EXTRA

#ifndef WOLFSSL_OPTIONS_IGNORE_SYS
#undef  _POSIX_THREADS
#define _POSIX_THREADS
#endif

#undef  HAVE_THREAD_LS
#define HAVE_THREAD_LS

#undef  TFM_TIMING_RESISTANT
#define TFM_TIMING_RESISTANT

#undef  ECC_TIMING_RESISTANT
#define ECC_TIMING_RESISTANT

#undef  WC_RSA_BLINDING
#define WC_RSA_BLINDING

#undef  HAVE_AESGCM
#define HAVE_AESGCM

#undef  NO_DSA
#define NO_DSA

#undef  NO_RC4
#define NO_RC4

#undef  NO_HC128
#define NO_HC128

#undef  NO_RABBIT
#define NO_RABBIT

#undef  HAVE_POLY1305
#define HAVE_POLY1305

#undef  HAVE_ONE_TIME_AUTH
#define HAVE_ONE_TIME_AUTH

#undef  HAVE_CHACHA
#define HAVE_CHACHA

#undef  HAVE_HASHDRBG
#define HAVE_HASHDRBG

#undef  HAVE_EXTENDED_MASTER
#define HAVE_EXTENDED_MASTER

#undef  NO_PSK
#define NO_PSK

#undef  NO_MD4
#define NO_MD4

#undef  WC_NO_ASYNC_THREADING
#define WC_NO_ASYNC_THREADING

#undef  NO_DES3
#define NO_DES3


#ifdef __cplusplus
}
#endif


#endif /* WOLFSSL_OPTIONS_H */

