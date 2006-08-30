/**
 * @file IxCryptoAccCodelet.c
 *
 * @date July-13-2005
 *
 * @brief This file contains the implementation of the Crypto Access Codelet.
 *
 * Descriptions of the functions used in this codelet is contained in
 * IxCryptoAccCodelet_p.h
 *
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

/*
 * System include files.
 */


#ifdef __vxworks
#include <end.h>    /* END drivers */
#include <endLib.h> /* END drivers */
#endif /* def __vxworks */

/*
 * User include files.
 */
#include "IxOsal.h" 
#include "IxQMgr.h"
#include "IxNpeDl.h"
#include "IxCryptoAccCodelet.h"
#include "IxCryptoAccCodelet_p.h"

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
#include "IxSerConsole.h"
#define printf ixSerPrintf
#define gets ixSerGets
#endif

/** Bits in byte */
#define BITS_IN_BYTE    8

/** 1 word = 4 bytes */
#define BYTES_IN_WORD   4  

/* digest size */
#define IX_CRYPTOACC_CODELET_DIGEST_LEN 20

/** QMgr dispatcher function pointer */
PRIVATE IxQMgrDispatcherFuncPtr ixCryptoAccCodeletDispatcherFunc;

/** QMgr dispatcher active flag */
PRIVATE volatile BOOL ixCryptoAccCodeletDispatcherPollStop = FALSE;

/** Mbuf Pool */
PRIVATE IX_OSAL_MBUF *ixCryptoAccCodeletCryptoBufPool;
PRIVATE IX_OSAL_MBUF *ixCryptoAccCodeletCryptoFreeBufQHead = NULL;
PRIVATE IX_OSAL_MBUF *ixCryptoAccCodeletCryptoFreeBufQTail = NULL;
PRIVATE UINT8 *ixCryptoAccCodeletCryptoBufData;
PRIVATE UINT32 ixCryptoAccCodeletCryptoFreeBufQSize = 0;

/** Registration results  */
BOOL forwardRegisterCBCalled = FALSE;
BOOL reverseRegisterCBCalled = FALSE;

/** Callback function */
IxCryptoAccRegisterCompleteCallback registerCB;
IxCryptoAccPerformCompleteCallback performCB;

/** Length of MBuf payload (in bytes) */
PRIVATE UINT32 packetLength;

/** Performance log variables declaration 
 * Notes:
 * Throughput of the operation performed in codelets is calculated once every 
 * 1000 requests. (EAU performs 100 operations only). 
 * The result is kept in the performanceLog array and is printed out after all 
 * the crypto perform service requests completed. The number of requests is 
 * determined by the PERFORMANCE_WINDOW_SIZE. 1 unit in the window size 
 * represent 1000 requests except for EAU (100 requests).
 */
PRIVATE UINT32 performanceLog [PERFORMANCE_WINDOW_SIZE];
PRIVATE UINT32 performanceNumEntries = 0;
PRIVATE UINT32 totalPacket = 0;
PRIVATE UINT32 timeStamp = 0;
PRIVATE UINT32 rateRatio;

/** Guard for once-only init code, when initialised this variable is set to 
 * TRUE 
 */
PRIVATE BOOL ixCryptoAccCodeletInitialised = FALSE;

/** Flag used when shutting down services, when TRUE, the function will 
 * shutdown 
 */
PRIVATE BOOL trafficFlowHalt = FALSE;

/* This flag is set when an error is returned by the ixCryptoApi while
 * doing perform operation. If this flag gets set then performance statistics
 * are not printed by the codelet.
 */
PRIVATE BOOL codeletPerformError = FALSE;

/** Initialised both CtxId to an invalid number as 0 (zero) is a valid ID.
 * The ID ranges from 0 to (IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS - 1), hence
 * IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS is invalid.
 */
PRIVATE UINT32 forwardCtxId = IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS;
PRIVATE UINT32 reverseCtxId = IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS;

/** Forward and reverse context declaration */
PRIVATE IxCryptoAccCtx forwardCtx, reverseCtx; 
INT32  codeletSrvIndex;

#ifdef __ixp46X             /* PKE codes only applicable to ixp46x platform */

/** Data/Result pointer to EAU and Hashing operations */
PRIVATE IxCryptoAccPkeEauOpResult opResult;
PRIVATE UINT8 *pDigest = NULL;
PRIVATE UINT8 *pRawData = NULL;

/** Callback for EAU and SHA */
IxCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn;
IxCryptoAccPkeHashPerformCompleteCallback hashPerformCallbackFn;

/** Data length for Hash and Eau operations */
PRIVATE UINT32 dataLen = 0;

/** Total operation to be done in a window size 
 *  (100 for EAU or 1000 for RNG and SHA) */
PRIVATE UINT32 totalOperation = 0;

/** A flag indicating all the operations are completed */
PRIVATE BOOL operationDone = FALSE;

/** EAU operands */
PRIVATE IxCryptoAccPkeEauInOperands operand;

/** First operand values max 64 words */
PRIVATE UINT32 opr1[IX_CRYPTO_ACC_CODELET_MAX_OPR_LEN] 
    = { 0x626d0298, 0x39ea0a13, 0x413163a5, 0x5b4cb500, 
        0x299d5522, 0x956cefcb, 0x3bff10f3, 0x99ce2c2e,  
        0x71cb9de5, 0xfa24babf, 0x58e5b795, 0x21925c9c,
        0xc42e9f6f, 0x464b088c, 0xc572af53, 0xe6d78802, 
        0x626d0278, 0x39ea0a13, 0x413163a5, 0x5b4cb500, 
        0x299d5522, 0x956cefcb, 0x3bff10f3, 0x99ce2c2e,  
        0x71cb9de5, 0xfa24babf, 0x58e5b795, 0x21925c9c,
        0xc42e9f6f, 0x464b088c, 0xc572af53, 0xe6d78802,
        0x626d0278, 0x39ea0a13, 0x413163a5, 0x5b4cb500, 
        0x299d5522, 0x956cefcb, 0x3bff10f3, 0x99ce2c2e,  
        0x71cb9de5, 0xfa24babf, 0x58e5b795, 0x21925c9c,
        0xc42e9f6f, 0x464b088c, 0xc572af53, 0xe6d78802, 
        0x626d0278, 0x39ea0a13, 0x413163a5, 0x5b4cb500, 
        0x299d5522, 0x956cefcb, 0x3bff10f3, 0x99ce2c2e,  
        0x71cb9de5, 0xfa24babf, 0x58e5b795, 0x21925c9c,
        0xc42e9f6f, 0x464b088c, 0xc572af53, 0xe6d78802};

/** Second operand values max 64 words */
PRIVATE UINT32 opr2[IX_CRYPTO_ACC_CODELET_MAX_OPR_LEN] 
    = { 0x8df2a491, 0x492276a3, 0x3d25759b, 0xb06869cb, 
        0xeac0d833, 0xfb8d0cf7, 0xcbb8324f, 0x0d7882e5, 
        0xd0762fc5, 0xb7210eaf, 0xc2e9ada7, 0x32ab7aa9, 
        0x49693dfb, 0xf83724c9, 0xec0736e7, 0x31c80291,
        0x8df2a491, 0x492276a3, 0x3d25759b, 0xb06869cb, 
        0xeac0d833, 0xfb8d0cf7, 0xcbb8324f, 0x0d7882e5, 
        0xd0762fc5, 0xb7210eaf, 0xc2e9adad, 0x32ab7aad, 
        0x49693dfb, 0xf83724c2, 0xec0736ee, 0x31c80291,
        0x8df2a491, 0x492276a3, 0x3d25759b, 0xb06869cb, 
        0xeac0d833, 0xfb8d0cf7, 0xcbb8324f, 0x0d7882e5, 
        0xd0762fc5, 0xb7210eaf, 0xc2e9ada7, 0x32ab7aa9, 
        0x49693dfb, 0xf83724c1, 0xec0736ef, 0x31c80291,
        0x8df2a491, 0x492276a3, 0x3d25759b, 0xb06869cb, 
        0xeac0d833, 0xfb8d0cf7, 0xcbb8324f, 0x0d7882e5, 
        0xd0762fc5, 0xb7210eaf, 0xc2e9ada7, 0x32ab7aa9, 
        0x49693dfb, 0xf83724cf, 0xec0736ef, 0x31c80291};

/** Third operand values max 64 words */
PRIVATE UINT32 opr3[IX_CRYPTO_ACC_CODELET_MAX_OPR_LEN] 
    = { 0x3f655bd0, 0x46f0b35e, 0xc791b004, 0x804afcbb, 
        0x8ef7d69d, 0xbf655bd0, 0x46f0b35e, 0xc791b004, 
        0x804afcbb, 0x8ef7d69d, 0xbf655bd0, 0x46f0b35e, 
        0xc791b004, 0x804afcbb, 0x8ef7d69d, 0xbf655bd0, 
        0x46f0b35e, 0xc791b004, 0x804afcbb, 0x8ef7d69d, 
        0xbf655bd0, 0x46f0b35e, 0xc791b004, 0x804afcbb, 
        0x8ef7d69d, 0xbf655bd0, 0x46f0b35e, 0xc791b004, 
        0x804afcbb, 0x8ef7d69d, 0xbf655bd0, 0x46f0b35e,
        0xc791b004, 0x804afcbb, 0x8ef7d69d, 0xbf655bd0, 
        0x46f0b35e, 0xc791b004, 0x804afcbb, 0x8ef7d69d, 
        0xbf655bd0, 0x46f0b35e, 0xc791b004, 0x804afcbb, 
        0x8ef7d69d, 0xbf655bd0, 0x46f0b35e, 0xc791b004, 
        0x804afcbb, 0x8ef7d69d, 0xbf655bd0, 0x46f0b35e,
        0xc791b004, 0x804afcbb, 0x8ef7d69d, 0xbf655bd0, 
        0x46f0b35e, 0xc791b004, 0x804afcbb, 0x8ef7d69d, 
        0xbf655bd0, 0x46f0b35e, 0xc791b004, 0x804afcbb};

/** PKE service name */
UINT8 ixCryptoAccCodeletPkeService[][IX_CRYPTOACC_CODELET_MAX_STR_SIZE]={
    "PKE RNG",
    "PKE HASH with SHA-1",
    "PKE EAU modular exponential operation",
    "PKE EAU big number modular reduction",
    "PKE EAU big number addition",
    "PKE EAU big number subtraction",
    "PKE EAU big number multiplication"
};

#endif /* ixp46X */


/** Random IV value and IV should remain constant on both
 * directions; encrypt and decrypt.
 */
#define IX_CRYPTO_ACC_CODELET_MAX_IV_LEN     IX_CRYPTO_ACC_AES_CCM_IV_512

PRIVATE UINT8 IV[IX_CRYPTO_ACC_CODELET_MAX_IV_LEN]
        = {  0,2,4,6,1,3,5,7,
             3,4,1,2,6,7,5,1,
             1,7,1,0,0,0,0,3,       
             3,4,3,4,4,5,6,7,       
             10,11,12,13,14,15,16,17,
             18,19,20,21,22,23,24,25,
             26,27,28,29,30,29,28,27,
             26,25,24,23,22,21,20,19};  

/** Random cipher key */
PRIVATE UINT8 cipherKey[IX_CRYPTO_ACC_MAX_CIPHER_KEY_LENGTH] 
        = {0x01,0x02,0x03,0x04,
           0x05,0x06,0x07,0x08,
           0x09,0x0A,0x0B,0x0C,
           0x0D,0x0E,0x0F,0x10,
           0x11,0x12,0x13,0x14,
           0x15,0x16,0x17,0x18,
           0x19,0x1A,0x1B,0x1C,
           0x1D,0x1E,0x1F,0x20};

/** SHA1 key from test case 1 in RFC 2202 */
PRIVATE UINT8 authKey[IX_CRYPTO_ACC_MAX_AUTH_KEY_LENGTH]
        = {0x0B,0x0B,0x0B,0x0B,
           0x0B,0x0B,0x0B,0x0B,
           0x0B,0x0B,0x0B,0x0B,
           0x0B,0x0B,0x0B,0x0B,
           0x0B,0x0B,0x0B,0x0B};

/** List of services configured for demonstration purposes */
IxCryptoCodeletServiceParam ixCryptoAccCodeletService[]={
    {
        IX_CRYPTO_ACC_OP_ENCRYPT,   /* Encrypt only request */
        IX_CRYPTO_ACC_OP_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_DES,
        IX_CRYPTO_ACC_MODE_ECB,
        IX_CRYPTO_ACC_DES_KEY_64,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        0,                          /* IV is ignored in ECB mode */
        IX_CRYPTO_ACC_AUTH_NULL,    /* Do not use any authentication algo */
        0,                          /* authentication key size is ignored */
        0,                          /* Digest length is ignored */
        0,                          /* aad length is ignored */
        FALSE,                      /* Inplace mode of operation */
        FALSE,
        "DES(ECB) "
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT,   /* Encrypt only request */
        IX_CRYPTO_ACC_OP_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_DES,
        IX_CRYPTO_ACC_MODE_CBC,
        IX_CRYPTO_ACC_DES_KEY_64,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        IX_CRYPTO_ACC_DES_IV_64,
        IX_CRYPTO_ACC_AUTH_NULL,    /* Do not use any authentication algo */
        0,                          /* authentication key size is ignored */
        0,                          /* Digest length is ignored */
        0,                          /* aad length is ignored */
        FALSE,                      /* Inplace mode of operation */
        FALSE,
        "DES(CBC) "
    },

    {
        IX_CRYPTO_ACC_OP_ENCRYPT,   /* Encrypt only request */
        IX_CRYPTO_ACC_OP_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_3DES,
        IX_CRYPTO_ACC_MODE_ECB,
        IX_CRYPTO_ACC_3DES_KEY_192,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        0,                          /* IV is ignored in ECB mode */
        IX_CRYPTO_ACC_AUTH_NULL,    /* Do not use any authentication algo */
        0,                          /* authentication key size is ignored */
        0,                          /* Digest length is ignored */
        0,                          /* aad length is ignored */
        FALSE,                      /* Inplace mode of operation */
        FALSE,
        "3DES(ECB) "
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT,   /* Encrypt only request */
        IX_CRYPTO_ACC_OP_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_3DES,
        IX_CRYPTO_ACC_MODE_CBC,
        IX_CRYPTO_ACC_3DES_KEY_192,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        IX_CRYPTO_ACC_DES_IV_64,
        IX_CRYPTO_ACC_AUTH_NULL,    /* Do not use any authentication algo */
        0,                          /* authentication key size is ignored */
        0,                          /* Digest length is ignored */
        0,                          /* aad length is ignored */
        FALSE,                      /* Inplace mode of operation */
        FALSE,
        "3DES(CBC) "
    },

    {
        IX_CRYPTO_ACC_OP_ENCRYPT,   /* Encrypt only request */
        IX_CRYPTO_ACC_OP_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_ECB,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        0,                          /* IV is ignored in ECB mode */
        IX_CRYPTO_ACC_AUTH_NULL,    /* Do not use any authentication algo */
        0,                          /* authentication key size is ignored */
        0,                          /* Digest length is ignored */
        0,                          /* aad length is ignored */
        FALSE,                      /* Inplace mode of operation */
        FALSE,
        "AES(ECB) "
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT,   /* Encrypt only request */
        IX_CRYPTO_ACC_OP_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_CBC,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        IX_CRYPTO_ACC_AES_CBC_IV_128,
        IX_CRYPTO_ACC_AUTH_NULL,    /* Do not use any authentication algo */
        0,                          /* authentication key size is ignored */
        0,                          /* Digest length is ignored */
        0,                          /* aad length is ignored */
        FALSE,                      /* Inplace mode of operation */
        FALSE,
        "AES(CBC) "
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT,   /* Encrypt only request */
        IX_CRYPTO_ACC_OP_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_CTR,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        IX_CRYPTO_ACC_AES_CTR_IV_128,
        IX_CRYPTO_ACC_AUTH_NULL,    /* Do not use any authentication algo */
        0,                          /* authentication key size is ignored */
        0,                          /* Digest length is ignored */
        0,                          /* aad length is ignored */
        FALSE,                      /* Inplace mode of operation */
        FALSE,
        "AES(CTR) "
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Authentication and encryption */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_CCM,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        IX_CRYPTO_ACC_AES_CCM_IV_512 ,
        IX_CRYPTO_ACC_AUTH_CBC_MAC,
        0,                          /* authentication key size is ignored */
        IX_CRYPTO_ACC_CCM_DIGEST_64,
        IX_CRYPTO_ACC_CCM_AAD_LEN_384,
        FALSE,
        FALSE,
        "AES CCM for 802.11i"
    },

    {
        IX_CRYPTO_ACC_OP_ENCRYPT,   /* Encrypt only request */
        IX_CRYPTO_ACC_OP_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_ARC4,
        IX_CRYPTO_ACC_MODE_NULL,    /* cipher mode is ignored when using ARC4 */
        IX_CRYPTO_ACC_ARC4_KEY_128,
        IX_CRYPTO_ACC_ARC4_BLOCK_8,
        0,                          /* IV length is ignored */
        IX_CRYPTO_ACC_AUTH_NULL,    /* Do not use any authentication algo */
        0,                          /* authentication key size is ignored */
        0,
        0,                          /* aad length is ignored */
        FALSE,
        TRUE,                       /* Codelet should use XScaleWepPerform
                                     * function to service this request.
                                     */
        "ARC4 on XScale"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT,   /* Encrypt only request */
        IX_CRYPTO_ACC_OP_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_ARC4,
        IX_CRYPTO_ACC_MODE_NULL,    /* cipher mode is ignored when using ARC4 */
        IX_CRYPTO_ACC_ARC4_KEY_128,
        IX_CRYPTO_ACC_ARC4_BLOCK_8,
        0,                          /* IV length is ignored */
        IX_CRYPTO_ACC_AUTH_NULL,    /* Do not use any authentication algo */
        0,                          /* authentication key size is ignored */
        0,
        0,                          /* aad length is ignored */
        FALSE,
        FALSE,                      /* Codelet should use NpeWepPerform
                                    * function to service this request.
                                    */
        "ARC4 on WAN-NPE"
    },

    {
        IX_CRYPTO_ACC_OP_AUTH_CALC, /* Authentication only request */
        IX_CRYPTO_ACC_OP_AUTH_CHECK,
        IX_CRYPTO_ACC_CIPHER_NULL,  /* Do not use any cipher algorithm */
        IX_CRYPTO_ACC_MODE_NULL,    /* Cipher mode is ignored */
        0,                          /* Cipher key size is ignored */
        0,                          /* Cipher algorithm block size is ignored*/
        0,                          /* IV is ignored */
        IX_CRYPTO_ACC_AUTH_MD5,     /* Authentication algorithm is MD5 */
        IX_CRYPTO_ACC_MD5_KEY_128,  /* Authentication Key size */
        IX_CRYPTO_ACC_MD5_DIGEST_128,   /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "HMAC using MD5"
    },
    {
        IX_CRYPTO_ACC_OP_AUTH_CALC, /* Authentication only request */
        IX_CRYPTO_ACC_OP_AUTH_CHECK,
        IX_CRYPTO_ACC_CIPHER_NULL,  /* Do not use any cipher algorithm */
        IX_CRYPTO_ACC_MODE_NULL,    /* Cipher mode is ignored */
        0,                          /* Cipher key size is ignored */
        0,                          /* Cipher algorithm block size is ignored*/
        0,                          /* IV is ignored */
        IX_CRYPTO_ACC_AUTH_SHA1,    /* Authentication algorithm is SHA1 */
        IX_CRYPTO_ACC_SHA1_KEY_160,     /* Authentication Key size */
        IX_CRYPTO_ACC_SHA1_DIGEST_160,  /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "HMAC using SHA1"
    },

    {
        IX_CRYPTO_ACC_OP_AUTH_CALC, /* Authentication only request */
        IX_CRYPTO_ACC_OP_AUTH_CHECK,
        IX_CRYPTO_ACC_CIPHER_NULL,  /* Do not use any cipher algorithm */
        IX_CRYPTO_ACC_MODE_NULL,    /* Cipher mode is ignored */
        0,                          /* Cipher key size is ignored */
        0,                          /* Cipher algorithm block size is ignored*/
        0,                          /* IV is ignored */
        IX_CRYPTO_ACC_AUTH_WEP_CRC,
        0,                          /* authentication key size is ignored */
        IX_CRYPTO_ACC_WEP_CRC_DIGEST_32,
        0,                          /* aad length is ignored */
        FALSE,
        TRUE,                       /* Codelet should use XScaleWepPerform
                                     * function to service this request.
                                     */
        "WEP CRC on XScale"
    },
    {
        IX_CRYPTO_ACC_OP_AUTH_CALC, /* Authentication only request */
        IX_CRYPTO_ACC_OP_AUTH_CHECK,
        IX_CRYPTO_ACC_CIPHER_NULL,  /* Do not use any cipher algorithm */
        IX_CRYPTO_ACC_MODE_NULL,    /* Cipher mode is ignored */
        0,                          /* Cipher key size is ignored */
        0,                          /* Cipher algorithm block size is ignored*/
        0,                          /* IV is ignored */
        IX_CRYPTO_ACC_AUTH_WEP_CRC,
        0,                          /* authentication key size is ignored */
        IX_CRYPTO_ACC_WEP_CRC_DIGEST_32,
        0,                          /* aad length is ignored */
        FALSE,
        FALSE,                      /* Codelet should use NpeWepPerform
                                     * function to service this request.
                                     */
        "WEP CRC on WAN-NPE"
    },

    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_DES,
        IX_CRYPTO_ACC_MODE_ECB,
        IX_CRYPTO_ACC_DES_KEY_64,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        0,                              /* IV is ignored in ECB mode */
        IX_CRYPTO_ACC_AUTH_MD5,         /* Authentication algorithm is MD5 */
        IX_CRYPTO_ACC_MD5_KEY_128,      /* Authentication Key size */
        IX_CRYPTO_ACC_MD5_DIGEST_128,   /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "DES(ECB) with MD5 "
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_DES,
        IX_CRYPTO_ACC_MODE_CBC,
        IX_CRYPTO_ACC_DES_KEY_64,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        IX_CRYPTO_ACC_DES_IV_64,
        IX_CRYPTO_ACC_AUTH_MD5,         /* Authentication algorithm is MD5 */
        IX_CRYPTO_ACC_MD5_KEY_128,      /* Authentication Key size */
        IX_CRYPTO_ACC_MD5_DIGEST_128,   /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "DES(CBC) with MD5"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_DES,
        IX_CRYPTO_ACC_MODE_ECB,
        IX_CRYPTO_ACC_DES_KEY_64,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        0,                              /* IV is ignored in ECB mode */
        IX_CRYPTO_ACC_AUTH_SHA1,        /* Authentication algorithm is SHA1 */
        IX_CRYPTO_ACC_SHA1_KEY_160,     /* Authentication Key size */
        IX_CRYPTO_ACC_SHA1_DIGEST_160,  /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "DES(ECB) with SHA1"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_DES,
        IX_CRYPTO_ACC_MODE_CBC,
        IX_CRYPTO_ACC_DES_KEY_64,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        IX_CRYPTO_ACC_DES_IV_64,
        IX_CRYPTO_ACC_AUTH_SHA1,        /* Authentication algorithm is SHA1 */
        IX_CRYPTO_ACC_SHA1_KEY_160,     /* Authentication Key size */
        IX_CRYPTO_ACC_SHA1_DIGEST_160,  /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "DES(CBC) with SHA1"
    },

    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_3DES,
        IX_CRYPTO_ACC_MODE_ECB,
        IX_CRYPTO_ACC_3DES_KEY_192,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        0,                              /* IV is ignored in ECB mode */
        IX_CRYPTO_ACC_AUTH_MD5,         /* Authentication algorithm is MD5 */
        IX_CRYPTO_ACC_MD5_KEY_128,      /* Authentication Key size */
        IX_CRYPTO_ACC_MD5_DIGEST_128,   /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "3DES(ECB) with MD5 "
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_3DES,
        IX_CRYPTO_ACC_MODE_CBC,
        IX_CRYPTO_ACC_3DES_KEY_192,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        IX_CRYPTO_ACC_DES_IV_64,
        IX_CRYPTO_ACC_AUTH_MD5,         /* Authentication algorithm is MD5 */
        IX_CRYPTO_ACC_MD5_KEY_128,      /* Authentication Key size */
        IX_CRYPTO_ACC_MD5_DIGEST_128,   /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "3DES(CBC) with MD5"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_3DES,
        IX_CRYPTO_ACC_MODE_ECB,
        IX_CRYPTO_ACC_3DES_KEY_192,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        0,                              /* IV is ignored in ECB mode */
        IX_CRYPTO_ACC_AUTH_SHA1,        /* Authentication algorithm is SHA1 */
        IX_CRYPTO_ACC_SHA1_KEY_160,     /* Authentication Key size */
        IX_CRYPTO_ACC_SHA1_DIGEST_160,  /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "3DES(ECB) with SHA1"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_3DES,
        IX_CRYPTO_ACC_MODE_CBC,
        IX_CRYPTO_ACC_3DES_KEY_192,
        IX_CRYPTO_ACC_DES_BLOCK_64,
        IX_CRYPTO_ACC_DES_IV_64,
        IX_CRYPTO_ACC_AUTH_SHA1,        /* Authentication algorithm is SHA1 */
        IX_CRYPTO_ACC_SHA1_KEY_160,     /* Authentication Key size */
        IX_CRYPTO_ACC_SHA1_DIGEST_160,  /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "3DES(CBC) with SHA1"
    },

    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_ECB,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        0,                              /* IV is ignored in ECB mode */
        IX_CRYPTO_ACC_AUTH_MD5,         /* Authentication algorithm is MD5 */
        IX_CRYPTO_ACC_MD5_KEY_128,      /* Authentication Key size */
        IX_CRYPTO_ACC_MD5_DIGEST_128,   /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "AES(ECB) with MD5"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_CBC,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        IX_CRYPTO_ACC_AES_CBC_IV_128,
        IX_CRYPTO_ACC_AUTH_MD5,         /* Authentication algorithm is MD5 */
        IX_CRYPTO_ACC_MD5_KEY_128,      /* Authentication Key size */
        IX_CRYPTO_ACC_MD5_DIGEST_128,   /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "AES(CBC) with MD5"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_CTR,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        IX_CRYPTO_ACC_AES_CTR_IV_128,
        IX_CRYPTO_ACC_AUTH_MD5,         /* Authentication algorithm is MD5 */
        IX_CRYPTO_ACC_MD5_KEY_128,      /* Authentication Key size */
        IX_CRYPTO_ACC_MD5_DIGEST_128,   /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "AES(CTR) with MD5"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_ECB,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        0,                              /* IV is ignored in ECB mode */
        IX_CRYPTO_ACC_AUTH_SHA1,        /* Authentication algorithm is SHA1 */
        IX_CRYPTO_ACC_SHA1_KEY_160,     /* Authentication Key size */
        IX_CRYPTO_ACC_SHA1_DIGEST_160,  /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "AES(ECB) with SHA1"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_CBC,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        IX_CRYPTO_ACC_AES_CBC_IV_128,
        IX_CRYPTO_ACC_AUTH_SHA1,        /* Authentication algorithm is SHA1 */
        IX_CRYPTO_ACC_SHA1_KEY_160,     /* Authentication Key size */
        IX_CRYPTO_ACC_SHA1_DIGEST_160,  /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "AES(CBC) with SHA1"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_AES,
        IX_CRYPTO_ACC_MODE_CTR,
        IX_CRYPTO_ACC_AES_KEY_128,
        IX_CRYPTO_ACC_AES_BLOCK_128,
        IX_CRYPTO_ACC_AES_CTR_IV_128,
        IX_CRYPTO_ACC_AUTH_SHA1,        /* Authentication algorithm is SHA1 */
        IX_CRYPTO_ACC_SHA1_KEY_160,     /* Authentication Key size */
        IX_CRYPTO_ACC_SHA1_DIGEST_160,  /* Digest length size */
        0,                              /* aad length is ignored */
        FALSE,                          /* Inplace mode of operation */
        FALSE,
        "AES(CTR) with SHA1"
    },

     {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_ARC4,
        IX_CRYPTO_ACC_MODE_NULL,        /* cipher mode is ignored when using ARC4 */
        IX_CRYPTO_ACC_ARC4_KEY_128,
        IX_CRYPTO_ACC_ARC4_BLOCK_8,
        0,                              /* IV length is ignored */
        IX_CRYPTO_ACC_AUTH_WEP_CRC,
        0,                              /* authentication key size is ignored */
        IX_CRYPTO_ACC_WEP_CRC_DIGEST_32,
        0,                              /* aad length is ignored */
        FALSE,
        TRUE,                           /* Codelet should use XScaleWepPerform
                                         * function to service this request.
                                         */
        "ARC4 + WEP CRC on XScale"
    },
    {
        IX_CRYPTO_ACC_OP_ENCRYPT_AUTH,  /* Encryption and authentication */
        IX_CRYPTO_ACC_OP_AUTH_DECRYPT,
        IX_CRYPTO_ACC_CIPHER_ARC4,
        IX_CRYPTO_ACC_MODE_NULL,        /* cipher mode is ignored when using ARC4 */
        IX_CRYPTO_ACC_ARC4_KEY_128,
        IX_CRYPTO_ACC_ARC4_BLOCK_8,
        0,                              /* IV length is ignored */
        IX_CRYPTO_ACC_AUTH_WEP_CRC,
        0,                              /* authentication key size is ignored */
        IX_CRYPTO_ACC_WEP_CRC_DIGEST_32,
        0,                              /* aad length is ignored */
        FALSE,
        FALSE,                          /* Codelet should use NpeWepPerform
                                         * function to service this request.
                                         */
        "ARC4 + WEP CRC on WAN-NPE"
    }
};

#define   IX_CRYPTOACC_CODELET_SRV_LIST        0

#ifdef __ixp46X             /* PKE codes only applicable to ixp46x platform */

#define IX_CRYPTOACC_CODELET_MAX_SRV_INDEX       \
            (sizeof(ixCryptoAccCodeletService)   \
            / sizeof(IxCryptoCodeletServiceParam)\
            + (sizeof(ixCryptoAccCodeletPkeService)\
            / IX_CRYPTOACC_CODELET_MAX_STR_SIZE))

#define IX_CRYPTOACC_CODELET_PKE_START_INDEX      \
            (IX_CRYPTOACC_CODELET_MAX_SRV_INDEX - \
            (sizeof(ixCryptoAccCodeletPkeService) \
            / IX_CRYPTOACC_CODELET_MAX_STR_SIZE) + 1)    /**< Start index of PKE 
                                                      * operation (37 - 7 + 1)
                                                      */
#else

#define IX_CRYPTOACC_CODELET_MAX_SRV_INDEX      \
            (sizeof(ixCryptoAccCodeletService) \
            / sizeof(IxCryptoCodeletServiceParam))

#define IX_CRYPTOACC_CODELET_PKE_START_INDEX \
            (IX_CRYPTOACC_CODELET_MAX_SRV_INDEX + 1)

#endif /* ixp46X */

/**
 * @fn ixCryptoAccCodeletBufIsQEmpty (void)
 *
 * @brief This inline function is used to check whether the buffer
 *        queue is empty.
 *
 */
INLINE BOOL
ixCryptoAccCodeletBufIsQEmpty (void)
{
    BOOL status = FALSE;
    if (0 == ixCryptoAccCodeletCryptoFreeBufQSize)
    { 
        status = TRUE;
    }
    return status;
}


/**
 * @fn ixCryptoAccCodeletBufIsQFull (void)
 *
 * @brief This inline function is used to check whether the buffer
 *        queue is full.
 *
 */
INLINE BOOL
ixCryptoAccCodeletBufIsQFull(void)
{
    BOOL status = FALSE;
    if (IX_CRYPTOACC_CODELET_CRYPTO_MBUF_POOL_SIZE 
        == ixCryptoAccCodeletCryptoFreeBufQSize)
    { 
        status = TRUE;
    }
    return status;
}


/**
 * @fn ixCryptoAccCodeletCryptoMbufToQTailAdd (IX_OSAL_MBUF *pMbufToAdd)
 *
 * @brief This function adds a mbuf to the tail of the queue.
 *
 */
PRIVATE void ixCryptoAccCodeletCryptoMbufToQTailAdd (IX_OSAL_MBUF *pMbufToAdd)
{
    IX_OSAL_MBUF *pMbuf;
    
    if (!ixCryptoAccCodeletBufIsQFull())
    {
        pMbuf = ixCryptoAccCodeletCryptoFreeBufQTail;
        if (NULL != ixCryptoAccCodeletCryptoFreeBufQTail) 
        {
            /* Add pMbufToAdd in the last mbuf in the chain */
            IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(pMbuf) = pMbufToAdd;    
        } /* End of if ixCryptoAccCodeletCryptoFreeBufQTail */
    
        /* Update the ixCryptoAccCodeletFreeBufQ with the last mbuf */
        ixCryptoAccCodeletCryptoFreeBufQTail = pMbufToAdd;
        IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(pMbufToAdd) = NULL;
        ixCryptoAccCodeletCryptoFreeBufQSize++;
    } 
    else /* Buffer queue is full */
    {
        printf("\nBuffer Queue is full\n");
    } /* End of if-else ixCryptoAccCodeletBufIsQFull() */
} /* End of ixCryptoAccCodeletCryptoMbufToQTailAdd () function */


/**
 * @fn ixCryptoAccCodeletCryptoMbufFromQHeadRemove (IX_OSAL_MBUF **pMbufToRem)
 *
 * @brief This function removes a mbuf from the head of the queue.
 */
PRIVATE void 
ixCryptoAccCodeletCryptoMbufFromQHeadRemove(IX_OSAL_MBUF **pMbufToRem)
{
    if (!ixCryptoAccCodeletBufIsQEmpty())
    {
        *pMbufToRem = ixCryptoAccCodeletCryptoFreeBufQHead;
        ixCryptoAccCodeletCryptoFreeBufQHead 
            = IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR((*pMbufToRem));
        
        /* Unchain the mbuf */
        IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(*pMbufToRem) = NULL;        

        ixCryptoAccCodeletCryptoFreeBufQSize--;
    } 
    else
    {
        *pMbufToRem = NULL;
        printf ("\nBuffer Queue is empty\n");   
    } /* end of if-else ixCryptoAccCodeletBufIsQEmpty */
} /* End of ixCryptoAccCodeletCryptoMbufFromQHeadRemove () function */


/**
 * @fn IX_STATUS ixCryptoAccCodeletCryptoMemPoolInit (UINT32 bufferSize)
 *
 * @brief Initialise Crypto Access Codelet MBUF pool
 */
PRIVATE IX_STATUS ixCryptoAccCodeletCryptoMemPoolInit (UINT32 bufferSize)
{
    UINT32 bufNo;
    IX_OSAL_MBUF *pMbuf;
    UINT32 mBlkSize = IX_OSAL_CACHE_LINE_SIZE * ((sizeof (IX_OSAL_MBUF) + 
                        (IX_OSAL_CACHE_LINE_SIZE - 1)) /
                        IX_OSAL_CACHE_LINE_SIZE);    
    UINT32 mDataSize = IX_OSAL_CACHE_LINE_SIZE * (( bufferSize + 
                        (IX_OSAL_CACHE_LINE_SIZE - 1)) /
                        IX_OSAL_CACHE_LINE_SIZE);

    ixCryptoAccCodeletCryptoBufPool = NULL;
    ixCryptoAccCodeletCryptoBufData = NULL;
    
    if (NULL == (ixCryptoAccCodeletCryptoBufPool 
                    = IX_CRYPTO_ACC_CODELET_DRV_DMA_MALLOC (
                        IX_CRYPTOACC_CODELET_CRYPTO_MBUF_POOL_SIZE
                        * mBlkSize)))
    {
        printf ("Error allocating mBuf pool for Crypto Codelet!\n");
        return IX_FAIL;
    }

    if (NULL == (ixCryptoAccCodeletCryptoBufData 
                    = IX_CRYPTO_ACC_CODELET_DRV_DMA_MALLOC(
                        IX_CRYPTOACC_CODELET_CRYPTO_MBUF_POOL_SIZE *
                        mDataSize)))
    {
        IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE (ixCryptoAccCodeletCryptoBufPool);

        printf ("Error allocating mBuf data pool for Crypto Codelet!\n");
        return IX_FAIL;
    }

    pMbuf = ixCryptoAccCodeletCryptoBufPool;
    /* Initilise mbuf pool head pointer */
    ixCryptoAccCodeletCryptoFreeBufQHead  = pMbuf;
    
    /* Format our mBufs */
    for (bufNo = 0;
        bufNo < IX_CRYPTOACC_CODELET_CRYPTO_MBUF_POOL_SIZE;
        bufNo++)
    {
        /* Assign memory block to mData pointer in mbuf */ 
        IX_OSAL_MBUF_MDATA (pMbuf) 
            = &ixCryptoAccCodeletCryptoBufData[bufNo * mDataSize];
    
        /* Initialise mbuf length */
        IX_OSAL_MBUF_MLEN(pMbuf) = mDataSize;
        
        /* No chains */
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(pMbuf)= NULL;
    
        /* Add newly formated buffer to our free queue */
        ixCryptoAccCodeletCryptoMbufToQTailAdd (pMbuf);
        pMbuf = (IX_OSAL_MBUF *)((UINT8 *) pMbuf + mBlkSize);
    }

    return IX_SUCCESS;
} /* End of ixCryptoAccCodeletCryptoMemPoolInit () function */


/**
 * @fn IX_STATUS ixCryptoAccCodeletCryptoMemPoolFree (UINT32 bufferSize)
 *
 * @brief Free all resources in Crypto Access Codelet MBUF pool
 *
 */
PRIVATE void ixCryptoAccCodeletCryptoMemPoolFree (UINT32 bufferSize)
{
    IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE (ixCryptoAccCodeletCryptoBufPool);

    IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE (ixCryptoAccCodeletCryptoBufData);
    
    ixCryptoAccCodeletCryptoFreeBufQHead = NULL;
    ixCryptoAccCodeletCryptoFreeBufQTail = NULL;
    ixCryptoAccCodeletCryptoFreeBufQSize = 0;
} /* End of ixCryptoAccCodeletMemPoolFree () function */


/**
 * @fn void ixCryptoAccCodeletDispatcherPoll (void)
 *
 * @brief This function polls the queues when poll mode set in the 
 *        function ixCryptoAccCodeletDispatcherStart. 
 */
PRIVATE void ixCryptoAccCodeletDispatcherPoll (void)
{
    /* Set dispatcher stop flag to FALSE */
    ixCryptoAccCodeletDispatcherPollStop = FALSE;
    
    while (1)
    {
        /* Exit thread if stop flag is set to TRUE */
        if (ixCryptoAccCodeletDispatcherPollStop)
        {
            break;  /* Exit the thread */
        }

#ifndef __vxworks
        /* In linux platform, the QMgr dispatcher thread needs to be put into 
         * sleep mode for 1 milisecond after each loop, in order to allow  
         * others thread to run.
         */
        ixOsalSleep (1);
#endif /* ndef __vxworks */

        /* The crypto NPE uses queues 29 and 30. */
        (*ixCryptoAccCodeletDispatcherFunc) (IX_QMGR_QUELOW_GROUP);
    }
    
} /* End of function ixCryptoAccCodeletDispatcherPoll() */


/**
 * @fn ixCryptoAccCodeletDispatcherStart (BOOL useInterrupt)
 *
 * @brief  This function starts the Queue manager dispatcher.
 *
 * @return IX_STATUS 
 */
PRIVATE IX_STATUS ixCryptoAccCodeletDispatcherStart (BOOL useInterrupt)
{
    IxOsalThread dispatchtid;
    IxOsalThreadAttr threadAttr;
    UINT8 *pThreadName = "QMgr Dispatcher";

    /* Get QMgr dispatcher function pointer */
    ixQMgrDispatcherLoopGet (&ixCryptoAccCodeletDispatcherFunc);
    
    if (useInterrupt)   /* Interrupt mode */
    {
        printf ("\nStarting Interrupt mode for cryptoAcc!\n");
        
        /* Hook the QM QLOW dispatcher to the interrupt controller. 
         * The crypto NPE uses queues 29 and 30.
         */
        if (IX_SUCCESS != ixOsalIrqBind (IX_OSAL_IXP400_QM1_IRQ_LVL,
                              (IxOsalVoidFnVoidPtr)
                              ixCryptoAccCodeletDispatcherFunc,
                              (void *)IX_QMGR_QUELOW_GROUP))
        {
            printf ("Failed to bind to QM1 interrupt\n");
            return IX_FAIL;
        }
    }
    else  /* Poll mode */
    {
        printf ("\nStarting Polling mode for cryptoAcc!\n");
        
        /* Set attribute of thread */
        threadAttr.name = pThreadName;
        threadAttr.stackSize 
            = IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE;
        threadAttr.priority = IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_PRIORITY;
        
        if (IX_SUCCESS != 
            ixOsalThreadCreate (
                &dispatchtid,
                &threadAttr,
                (IxOsalVoidFnVoidPtr) ixCryptoAccCodeletDispatcherPoll,
                NULL))
        {
            printf ("Error spawning dispatch task\n");
            return IX_FAIL;
        }

        if (IX_SUCCESS != 
            ixOsalThreadStart (&dispatchtid))
        {
            printf ("Error starting dispatch task\n");
            return IX_FAIL;
        }
    }

    return IX_SUCCESS;
} /* End of ixCryptoAccCodeletDispatcherStart () function */


/**
 * @fn ixCryptoAccCodeletRegisterCtx(
 *         INT32  srvIndex,
 *         IxCryptoAccCtx *forwardCtx,
 *         IxCryptoAccCtx *reverseCtx,
 *         UINT32 *forwardCtxId,
 *         UINT32 *reverseCtxId)
 *
 * @brief  This function registers to the Crypto Access Component for
 *         the specified service index. The srvIndex passed into this 
 *         function is an index into the ixCryptoAccCodeletService array.
 */
PRIVATE IX_STATUS ixCryptoAccCodeletRegisterCtx (
            INT32  srvIndex,
            IxCryptoAccCtx *pForwardCtx,
            IxCryptoAccCtx *pReverseCtx,
            UINT32 *pForwardCtxId,
            UINT32 *pReverseCtxId)
{
    IxCryptoAccStatus status;
    IX_OSAL_MBUF *pPrimaryMbuf = NULL;
    IX_OSAL_MBUF *pSecondaryMbuf = NULL;
    UINT16 i = 0;
    
    if ( (NULL == pForwardCtx)  || 
         (NULL == pReverseCtx)  ||
         (NULL == pForwardCtxId)  ||
         (NULL == pReverseCtxId))
    {
        printf ("\n Input Param is NULL ");
        return IX_FAIL;
    }

    /* Initialize forward and reverse contexts */
    pForwardCtx->operation = ixCryptoAccCodeletService[srvIndex].frwdOperation;
    pReverseCtx->operation = ixCryptoAccCodeletService[srvIndex].revOperation ;

    pForwardCtx->cipherCtx.cipherAlgo 
        = pReverseCtx->cipherCtx.cipherAlgo 
        = ixCryptoAccCodeletService[srvIndex].cipherAlgo;
        
    pForwardCtx->cipherCtx.cipherMode 
        = pReverseCtx->cipherCtx.cipherMode 
        = ixCryptoAccCodeletService[srvIndex].cipherMode;
        
    pForwardCtx->cipherCtx.cipherKeyLen 
        = pReverseCtx->cipherCtx.cipherKeyLen 
        = ixCryptoAccCodeletService[srvIndex].cipherKeyLen;
        
    pForwardCtx->cipherCtx.cipherBlockLen 
        = pReverseCtx->cipherCtx.cipherBlockLen 
        = ixCryptoAccCodeletService[srvIndex].cipherBlockLen;
        
    pForwardCtx->cipherCtx.cipherInitialVectorLen 
        = pReverseCtx->cipherCtx.cipherInitialVectorLen 
        = ixCryptoAccCodeletService[srvIndex].cipherInitialVectorLen;
        
    ixOsalMemCopy (pForwardCtx->cipherCtx.key.cipherKey,
        cipherKey,
        ixCryptoAccCodeletService[srvIndex].cipherKeyLen);

    ixOsalMemCopy (pReverseCtx->cipherCtx.key.cipherKey,
        cipherKey,
        ixCryptoAccCodeletService[srvIndex].cipherKeyLen);
    
    pForwardCtx->authCtx.authAlgo 
        = pReverseCtx->authCtx.authAlgo 
        = ixCryptoAccCodeletService[srvIndex].authAlgo;
        
    pForwardCtx->authCtx.authDigestLen 
        = pReverseCtx->authCtx.authDigestLen 
        = ixCryptoAccCodeletService[srvIndex].authDigestLen;
        
    pForwardCtx->authCtx.authKeyLen 
        = pReverseCtx->authCtx.authKeyLen 
        = ixCryptoAccCodeletService[srvIndex].authKeyLen;
        
    pForwardCtx->authCtx.aadLen
        = pReverseCtx->authCtx.aadLen
        = ixCryptoAccCodeletService[srvIndex].aadLen;

    ixOsalMemCopy (pForwardCtx->authCtx.key.authKey,
        authKey,
        ixCryptoAccCodeletService[srvIndex].authKeyLen);

    ixOsalMemCopy (pReverseCtx->authCtx.key.authKey,
        authKey,
        ixCryptoAccCodeletService[srvIndex].authKeyLen);
    
    pForwardCtx->useDifferentSrcAndDestMbufs 
        = pReverseCtx->useDifferentSrcAndDestMbufs 
        = ixCryptoAccCodeletService[srvIndex].useDifferentSrcAndDestMbufs;

    /* Primary Mbuf and Secondary Mbuf is required for authentication 
     * and combined service for forward context
     */
    if (( IX_CRYPTO_ACC_OP_AUTH_CALC 
        ==ixCryptoAccCodeletService[srvIndex].frwdOperation) || 
        ( IX_CRYPTO_ACC_OP_ENCRYPT_AUTH 
        == ixCryptoAccCodeletService[srvIndex].frwdOperation))
    {
        /* If auth algo is CBC MAC or WEP CRC then the primary and
         * secondary mbufs are not required and should be NULL.
         */
        if( ( IX_CRYPTO_ACC_AUTH_CBC_MAC 
            != ixCryptoAccCodeletService[srvIndex].authAlgo)
            &&( IX_CRYPTO_ACC_AUTH_WEP_CRC 
            !=ixCryptoAccCodeletService[srvIndex].authAlgo))
        {
            ixCryptoAccCodeletCryptoMbufFromQHeadRemove (&pPrimaryMbuf);
            ixCryptoAccCodeletCryptoMbufFromQHeadRemove (&pSecondaryMbuf);

            /* Check if the mbufs are NULL */
            if ((NULL == pPrimaryMbuf) || (NULL == pSecondaryMbuf))
            {
                printf ("\nUnable to allocate mbufs for chaining variables\n");
                return IX_FAIL;
            }
        }
    }

    /* Register the forward context */
    status = ixCryptoAccCtxRegister (
                 pForwardCtx,
                 pPrimaryMbuf,
                 pSecondaryMbuf,
                 registerCB,
                 performCB,
                 pForwardCtxId);
                 
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
    {
        printf ("Forward registration Failed. Status = %d\n", status);
        return IX_FAIL;
    }

    i = 0;

    /* Wait for the forward register callback to return */
    while (!forwardRegisterCBCalled)
    {
        ixOsalSleep (IX_CRYPTOACC_CODELET_REGISTER_WAIT_TIME);
        
        /* Wait for IX_CRYPTOACC_CODELET_MAX_TIME_WAIT times before return 
         * fail status 
         */
        if (IX_CRYPTOACC_CODELET_MAX_TIME_WAIT < i)
        {
            printf ("\nRegistration incomplete, timeout for forward callback\n");
            return IX_FAIL;
        }
        i++;
    } /* end of while (!forwardRegisterCBCalled) */

    /* Primary Mbuf and Secondary Mbuf are required for authentication 
     * and combined service for reverse context
     */
    if (( IX_CRYPTO_ACC_OP_AUTH_CALC 
        ==ixCryptoAccCodeletService[srvIndex].frwdOperation) || 
        ( IX_CRYPTO_ACC_OP_ENCRYPT_AUTH 
        == ixCryptoAccCodeletService[srvIndex].frwdOperation))
    {
        /* If auth algo is CBC MAC or WEP CRC then the primary and
         * secondary mbufs are not required and should be NULL.
         */
        if( ( IX_CRYPTO_ACC_AUTH_CBC_MAC 
            != ixCryptoAccCodeletService[srvIndex].authAlgo)
            &&( IX_CRYPTO_ACC_AUTH_WEP_CRC 
            !=ixCryptoAccCodeletService[srvIndex].authAlgo))
        {
            ixCryptoAccCodeletCryptoMbufFromQHeadRemove (&pPrimaryMbuf);
            ixCryptoAccCodeletCryptoMbufFromQHeadRemove (&pSecondaryMbuf);
    
            /* Check if the mbufs are NULL */
            if ((NULL == pPrimaryMbuf) || (NULL == pSecondaryMbuf))
            {
                printf ("\nUnable to allocate mbufs for chaining variables!\n");
                return IX_FAIL;
            }
        }
    }
    
    /* Register the reverse context */
    status = ixCryptoAccCtxRegister (
                 pReverseCtx,
                 pPrimaryMbuf,
                 pSecondaryMbuf,
                 registerCB,
                 performCB,
                 pReverseCtxId);
                 
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
    {
        printf ("Reverse registration Failed. Status = %d\n", status);
        return IX_FAIL;
    }

    i = 0;

    /* Wait for the reverse register callback to return */
    while (!reverseRegisterCBCalled)
    {
        ixOsalSleep (IX_CRYPTOACC_CODELET_REGISTER_WAIT_TIME);
        
        if (IX_CRYPTOACC_CODELET_MAX_TIME_WAIT < i)
        {
            printf ("\nRegistration incomplete, timeout for reverse callback\n");
            return IX_FAIL;
        }
        i++;
    } /* end of while (!reverseRegisterCBCalled) */
    
    return IX_SUCCESS;
} /* End of ixCryptoAccCodeletRegisterCtx () function */



/**
 * @fn ixCryptoAccCodeletPerform( UINT32 pktLen)
 *
 * @brief This function invokes the "appropriate" perform function
 *        of the cryptoAcc component. XScaleWepPerform function is invoked
 *        if the flag is set for the service index. This function uses 
 *        the global variable "codeletSrvIndex", set by the codeletMain 
 *        function to access the requested service's parameters.
 */
PRIVATE IX_STATUS ixCryptoAccCodeletPerform (UINT32 pktLen)
{
    IxCryptoAccStatus status;
    IX_OSAL_MBUF *pMbuf;
    UINT32 pktNo;
    UINT32 j;
    UINT8  data;

    if (IX_CRYPTO_ACC_CIPHER_AES 
        == ixCryptoAccCodeletService[codeletSrvIndex].cipherAlgo)
    {
        /* Exception: When the cipher mode is CCM, then there is no
         * restriction on the packet length being a multiple of
         * block size. 
         */
        if(IX_CRYPTO_ACC_MODE_CCM 
            != ixCryptoAccCodeletService[codeletSrvIndex].cipherMode)
        {
            /* Check if packet length is multiple of cipher block length */
            if (0 != (pktLen %  IX_CRYPTO_ACC_AES_BLOCK_128))
            {
                printf ("\nPacket length is not mulitple of AES cipher "
                    "block length - 16 bytes.\n\n");
                ixCryptoAccCodeletUsageShow ();
                return IX_FAIL;
            }
        }
    }

    if ( (IX_CRYPTO_ACC_CIPHER_DES 
        ==  ixCryptoAccCodeletService[codeletSrvIndex].cipherAlgo)
        || (IX_CRYPTO_ACC_CIPHER_3DES 
        == ixCryptoAccCodeletService[codeletSrvIndex].cipherAlgo))
    {
        /* Check if packet length is multiple of cipher block length */
        if (0 != (pktLen %  IX_CRYPTO_ACC_DES_BLOCK_64))
        {
            printf ("\nPacket length is not mulitple of DES cipher block "
                "length - 8 bytes.\n\n");
            ixCryptoAccCodeletUsageShow ();
            return IX_FAIL;
        }
    }
        
    /* If the cipher algorithm is ARC4, block length is 1 byte, therefore no
     * check is required on the packet length.
     */

    /* Check, if the request should be performed by XScaleWepPerform 
     * function. Note that the perform done call back is not called if
     * XScaleWepPerform function is invoked.
     */
    if(TRUE == ixCryptoAccCodeletService[codeletSrvIndex].invokeXScaleWepPerform )
    {
        /* Get an mbuf */
        ixCryptoAccCodeletCryptoMbufFromQHeadRemove (&pMbuf);

        /* Check if the mbuf is NULL */
        if (NULL == pMbuf)
        {
            printf ("\nUnable to allocate mbuf for data\n");
            return IX_FAIL;
        }
        
        /* copy data into mBuf from the ASCII table 0 (0x30) to z (0x7a)*/
        data = (INT8) 0x30;
        for (j = 0; j < pktLen; j++)
        {
            * (IX_OSAL_MBUF_MDATA(pMbuf) + j) = data++;             
            if ((INT8) 0x7b == data)
            {
                data =(INT8) 0x30;
            }
        }
       
        /* Call XScaleWepPerform function */
        status = ixCryptoAccCodeletXScaleWepPerform(pMbuf,pktLen);

        if(IX_SUCCESS!= status)
        {
            printf ("\n XScaleWepPeformed failed...!!!!");
            codeletPerformError= TRUE;
        }
        
        /* Added sleep to solve the RPC timeout problem */
        ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
        printf ("\nWaiting for operation done [<10 min for the biggest packet]\n");

        ixCryptoAccCodeletCryptoMbufToQTailAdd(pMbuf);
        
        /* Wait for all buffers being returned in callbacks */
        while (!ixCryptoAccCodeletBufIsQFull())
        {
            ixOsalSleep(IX_CRYPTOACC_CODELET_PERFORM_WAIT_TIME);
            printf (".");
        }
        
    }
    else
    {
        for(pktNo = 0; 
            pktNo < IX_CRYPTOACC_CODELET_CRYPTO_MBUF_POOL_SIZE;
            pktNo++)
        {
            /* Get an mbuf */
            ixCryptoAccCodeletCryptoMbufFromQHeadRemove (&pMbuf);

            /* Check if the mbuf is NULL */
            if (NULL == pMbuf)
            {
                printf ("\nUnable to allocate mbuf for data\n");
                return IX_FAIL;
            }
        
            /* copy data into mBuf from the ASCII table 0 (0x30) to z (0x7a)*/
            data = (INT8) 0x30;
            for (j = 0; j < pktLen; j++)
            {
                * (IX_OSAL_MBUF_MDATA(pMbuf) + j) = data++;             
                if ((INT8) 0x7b == data)
                {
                    data =(INT8) 0x30;
                }
            }
            /* This while loop will always call perform until it is  not 
             * successful but will continue to retry perform if the status 
             * is queue_full. Start with forwardCtxId and the reverseCtxId
             * will be used in the perform callback.
             */
            do
            {
                if (0 == pktNo)
                {
                    /* Start the timeStamp before Perform */
                    timeStamp = ixOsalTimestampGet();
                }
                if( IX_CRYPTO_ACC_CIPHER_ARC4 
                    != ixCryptoAccCodeletService[codeletSrvIndex].cipherAlgo
                    && IX_CRYPTO_ACC_AUTH_WEP_CRC 
                    != ixCryptoAccCodeletService[codeletSrvIndex].authAlgo)
                {
                    status = ixCryptoAccAuthCryptPerform (
                        forwardCtxId,
                        pMbuf,
                        NULL,
                        0,
                        pktLen,    
                        0, 
                        pktLen,
                        pktLen,
                        IV);
                }
                else
                {
                    /* If this request involves ARC4 or WEP_CRC, invoke 
                     * NPE wep perform function 
                     */
                    status= ixCryptoAccNpeWepPerform(forwardCtxId,
                                                     pMbuf,
                                                     NULL,
                                                     0, 
                                                     pktLen,
                                                     pktLen,
                                                     cipherKey);

                }
            
            } while (IX_CRYPTO_ACC_STATUS_QUEUE_FULL == status);
            /* End of do-while loop */
        
            printf ("Placed %dth Packet in to flow\n", pktNo + 1);

        } /* End of for (i) loop */
    
        printf ("\nWaiting for operation done [<10 min for the biggest packet]\n");

        /* Wait crypto services done. The task is put into sleep state while 
         * waiting for all the callbacks returned
         */        
        do
        {
            ixOsalSleep(IX_CRYPTOACC_CODELET_PERFORM_WAIT_TIME);            
        } while (!trafficFlowHalt);
    
        /* Wait for all buffers being returned in callbacks */
        while (!ixCryptoAccCodeletBufIsQFull())
        {
            ixOsalSleep(IX_CRYPTOACC_CODELET_PERFORM_WAIT_TIME);
            printf (".");
        }
    } /* end of else on operation type */
   
    ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
    printf ("\nOperation DONE!\n");

    /* Unregister contexts */
    ixCryptoAccCodeletUnregisterCtx();

    return IX_SUCCESS;
}


/**
 * @fn ixCryptoAccCodeletUnregisterCtx (void)
 *
 * @brief  This function unregisters all the contexts registered.
 */
PRIVATE void ixCryptoAccCodeletUnregisterCtx (void)
{
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        ixCryptoAccCtxUnregister (forwardCtxId))
    {
        printf ("\nFailed to unregister forward context %d\n", forwardCtxId);
    }

    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        ixCryptoAccCtxUnregister (reverseCtxId))
    {
        printf ("\nFailed to unregister reverse context %d\n", reverseCtxId);
    }

    forwardRegisterCBCalled = FALSE;
    reverseRegisterCBCalled = FALSE;
    
    /* Initialised to an invalid number, zero is a valid ID */
    forwardCtxId = IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS;
    reverseCtxId = IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS;
}

/**
 * @fn  ixCryptoAccCodeletInit (IxCryptoAccCfg ixCryptoAccCodeletCfg)
 * @brief This function initialises the required components depending upon
 *        the ixCryptoAccCodeletCfg.
 *
 */
PRIVATE IX_STATUS ixCryptoAccCodeletInit (IxCryptoAccCfg ixCryptoAccCodeletCfg)
{
    if (ixCryptoAccCodeletInitialised) 
    {
        printf ("\nCrypto codelet already initialised\n");
        return IX_SUCCESS;
    }
    
#ifdef __vxworks
    /* When the ixe drivers are running, the codelets
     * cannot run.
     */
    if (NULL != endFindByName ("ixe", 0))
    {
        printf ("FAIL : Driver ixe0 detected\n");
        return IX_FAIL;
    }
    if (NULL != endFindByName ("ixe", 1))
    {
        printf ("FAIL : Driver ixe1 detected\n");
        return IX_FAIL;
    }
#endif

    /* Initialise stop flag to TRUE */
    ixCryptoAccCodeletDispatcherPollStop = TRUE;

    /* Load NPE C image if required */
    if( (IX_CRYPTO_ACC_CFG_CRYPTO_NPE_ACC_EN == ixCryptoAccCodeletCfg) ||
        (IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN == ixCryptoAccCodeletCfg))
    {
        printf ("\nInitializing NPE C...");
        /* Initialise NPE C */
        if (IX_SUCCESS != ixNpeDlNpeInitAndStart (
            IX_CRYPTOACC_CODELET_NPEC_IMAGE_ID))
        {
            printf ("Error initialising NPE C for crypto processing\n");
            return IX_FAIL;
        }
        printf ("..done\n");
    }

    /* Load NPE A image if required */
    if( (IX_CRYPTO_ACC_CFG_WEP_NPE_ACC_EN == ixCryptoAccCodeletCfg ) ||
        (IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN == ixCryptoAccCodeletCfg))
    {
        printf ("\nInitializing NPE A...");
        /* Initialise NPE A */
        if (IX_SUCCESS != ixNpeDlNpeInitAndStart (
            IX_CRYPTOACC_CODELET_NPEA_IMAGE_ID))
        {
            printf ("Error initialising NPE A for WEP processing\n");
            return IX_FAIL;
        }
        printf ("..done\n");
    }

    /* Initialise Queue Manager */
    if (IX_SUCCESS != ixQMgrInit ())
    {
        printf ("Error initialising queue manager!\n");
        return IX_FAIL;
    }

    if(IX_SUCCESS != ixCryptoAccConfig (ixCryptoAccCodeletCfg))
    {
        ixQMgrUnload();
        printf ("Error failed to configure crypto acc component");
        return IX_FAIL;
    }

    /* Initialise cryptoAcc component */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoAccInit ())
    {
        ixQMgrUnload();
        printf ("Error initialising Crypto access component!\n");
        return IX_FAIL;
    }

    /* Start the Queue Manager dispatcher */   
    if ( IX_SUCCESS != ixCryptoAccCodeletDispatcherStart (
        IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_MODE))
    {
        ixCryptoAccUninit();
        ixQMgrUnload();
        printf ("Error starting queue manager dispatcher!\n");
        return IX_FAIL;
    }

    /* set flag to TRUE to indicate codelet has been initialised successfully */
    ixCryptoAccCodeletInitialised = TRUE;
    printf ("\nIxCryptoAcc Codelet Initialization complete!\n\n");

    return IX_SUCCESS;
} /* End of ixCryptoAccCodeletInit () function */

/**
 * @fn  ixCryptoAccCodeletUninit (void)
 * @brief This function uninitialize the crypto codelets 
 *
 */
PRIVATE IX_STATUS ixCryptoAccCodeletUninit (void)
{  
    /* Uninitialize cryptoAcc */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoAccUninit())
    {
        printf ("Uninitialize cryptoAcc failed\n");
        return IX_FAIL;
    }

    /* PKE does not need to free this resource */
    if (IX_CRYPTOACC_CODELET_PKE_START_INDEX > (UINT32)(codeletSrvIndex+1))
    {
        /* Free resources */
        ixCryptoAccCodeletCryptoMemPoolFree(
            packetLength + IX_CRYPTOACC_CODELET_DIGEST_LEN);
    }
    
    /* Stop qmgr dispatcher */
    if ( IX_SUCCESS != ixCryptoAccCodeletDispatcherStop 
        (IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_MODE))
    {
        printf ("Unable to stop qmgr dispatcher\n");
        return IX_FAIL;
    }
        
    /* Unload qmgr */
    if (IX_SUCCESS != ixQMgrUnload())
    {
        printf ("Uninitialize qmgr failed\n");
        return IX_FAIL;
    }
    
     /* Reset initialize flag */
    ixCryptoAccCodeletInitialised = FALSE;       
    printf ("\nCrypto Codelet Uninitialzed Successfully\n");
    return IX_SUCCESS;
}


/** 
 * @fn ixCryptoAccCodeletRegisterCB(
 *         UINT32 cryptoCtxId,
 *         IX_OSAL_MBUF *pMbuf, 
 *         IxCryptoAccStatus status)
 *
 * @brief  Cryptographic Context registration callback function to be called
 *         in QMgr Dispatcher context when the registration completed.
 *
 */
PRIVATE void ixCryptoAccCodeletRegisterCB (
        UINT32 cryptoCtxId,
        IX_OSAL_MBUF *pMbuf, 
        IxCryptoAccStatus status)
{
    BOOL *pFlagToUse;

    /* Determine callback flag to be used from the context crypto registered. 
     * If forward context, forwardRegisterCBCalled is used; otherwise 
     * reverseRegisterCBCalled is used.  
     */
    if (cryptoCtxId == forwardCtxId)
    {
        pFlagToUse = &forwardRegisterCBCalled;
    }
    else /* (cryptoCtxId != forwardCtxId) */
    {   
        if (cryptoCtxId == reverseCtxId)
        {
            pFlagToUse = &reverseRegisterCBCalled;
        }
        else
        {
            ixOsalLog (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "\nInvalid ContextId %d\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);
            return;
        } /* end of if-else (cryptoCtxId == reverseCtxId) */
    } /* end of if-else (cryptoCtxId == forwardCtxId) */

    /* We are expecting two callbacks for authentication and combined service 
     * The pMbuf could be null for encrypt/decrypt ONLY operation.
     */
    if (NULL != pMbuf)
    {
        /* Must return the buffer to the pool irrespective of the status */
        ixCryptoAccCodeletCryptoMbufToQTailAdd (pMbuf);
    }

    if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
    {
        *pFlagToUse = TRUE;
        ixOsalLog (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "RegistrationCB is successful. Context %d\n", 
            cryptoCtxId, 
            0, 0, 0, 0, 0);
    } /* End of if (IX_CRYPTO_ACC_STATUS_SUCCESS == status) */
    else 
    {
        if (IX_CRYPTO_ACC_STATUS_WAIT == status)
        {
            ixOsalLog (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Register not completed yet; Context %d\n", 
                cryptoCtxId, 
                0, 0, 0, 0, 0);
        }
        else
        {
            ixOsalLog (
                IX_OSAL_LOG_LVL_WARNING,
                IX_OSAL_LOG_DEV_STDOUT, 
                "Registration failed; Context %d\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);
        }
    } /* End of if else (IX_CRYPTO_ACC_STATUS_SUCCESS == status) */
} /* End of ixCryptoAccCodeletRegisterCB () function */


/**
 * @fn ixCryptoAccCodeletPerformCB (
 *        UINT32 cryptoCtxId,
 *        IX_OSAL_MBUF *pSrcMbuf, 
 *        IX_OSAL_MBUF *pDestMbuf,
 *        IxCryptoAccStatus status)
 *
 * @brief  Crypto services request callback function to be called when the 
 *         crypto service s request completed. Performance rate is calculated 
 *         in this function.
 *
 */
PRIVATE void ixCryptoAccCodeletPerformCB (
        UINT32 cryptoCtxId,
        IX_OSAL_MBUF *pSrcMbuf, 
        IX_OSAL_MBUF *pDestMbuf,
        IxCryptoAccStatus status)
{
    IxCryptoAccStatus retStatus;
    UINT32 timeNow;     /**< Current timestamp */
    UINT32 timeDiff;    /**< Time taken to process 1000 packets in XSCALE TICK 
                         * unit
                         */ 
    switch (status)
    {
        case IX_CRYPTO_ACC_STATUS_SUCCESS:
            if (IX_CRYPTOACC_CODELET_BATCH_LEN <= totalPacket)
            {
                timeNow = ixOsalTimestampGet();
                
                /* Check if the timer wrap over, get the time taken and  
                 * divided it by 66 to get microseconds as XScale tick is 66MHz 
                 */
                if (timeNow < timeStamp)
                {
                    timeDiff = ((0xffffffff - timeStamp + timeNow + 1)/
                        IX_CRYPTOACC_CODELET_XSCALE_TICK);
                }
                else
                {
                    timeDiff = ((timeNow - timeStamp) /
                        IX_CRYPTOACC_CODELET_XSCALE_TICK);
                } /* end of if-else (timeNow < timeStamp) */
                
                /* Store performance rate for every 1000 packets */
                if (PERFORMANCE_WINDOW_SIZE > performanceNumEntries)
                {
                    performanceLog[performanceNumEntries] = rateRatio / timeDiff;
                    performanceNumEntries++;
                }
        
                /* Reset totalPacket for another measurement */
                totalPacket = 0;
        
                /* Start the timeStamp */
                timeStamp = timeNow;
            }
            else
            {
                totalPacket++;
            } /* End of if-else totalPacket */
            break;
      
        case IX_CRYPTO_ACC_STATUS_AUTH_FAIL:
            ixOsalLog (
                IX_OSAL_LOG_LVL_WARNING,
                IX_OSAL_LOG_DEV_STDOUT,
                "Authentication Failed\n",
                0, 0, 0, 0, 0, 0);
            codeletPerformError = TRUE;
            break;
    
        case IX_CRYPTO_ACC_STATUS_FAIL:
            ixOsalLog (
                IX_OSAL_LOG_LVL_WARNING,
                IX_OSAL_LOG_DEV_STDOUT,
                "Operation Failed\n", 
                0, 0, 0, 0, 0, 0);
            codeletPerformError = TRUE;
            break;
    
        default:
            ixOsalLog (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "Error in status message, %d\n", 
                status, 0, 0, 0, 0, 0);
            codeletPerformError = TRUE;
    } /* End of switch (status) */

    /* Sample size more than user-specified size, halt the traffic */    
    if (PERFORMANCE_WINDOW_SIZE <= performanceNumEntries)
    {
        trafficFlowHalt = TRUE;
    }

    /* If we are shutting down, return mbuf to the pool and return */
    if (trafficFlowHalt)
    {
        /* Return the buffer(s) to the pool */
        if (NULL != pSrcMbuf)
        {
            ixCryptoAccCodeletCryptoMbufToQTailAdd (pSrcMbuf);
        }
        if (NULL != pDestMbuf)
        {
            ixCryptoAccCodeletCryptoMbufToQTailAdd (pDestMbuf);
        }
        return;
    }

    /* Switch Contexts ID, based on the context ID to determine what operation 
     * should be done on the packet. If cryptoCtxId is a forwardCtxId, the 
     * packet is encrypted / authenticated, send the packet to NPE again for
     * decryption / auth_check, and vise-versa.
     */
    if (cryptoCtxId == forwardCtxId)
    {
        /* The perform will keep on trying to write to the queue and will wait
         * if the queue is full
         */
        do
        {
            if(IX_CRYPTO_ACC_CIPHER_ARC4 
                != ixCryptoAccCodeletService[codeletSrvIndex].cipherAlgo
                && IX_CRYPTO_ACC_AUTH_WEP_CRC 
                != ixCryptoAccCodeletService[codeletSrvIndex].authAlgo)
            {
                retStatus = ixCryptoAccAuthCryptPerform (
                    reverseCtxId,
                    pSrcMbuf,
                    pDestMbuf,
                    0,
                    packetLength,
                    0,
                    packetLength,
                    packetLength,
                    IV);
            }
            else
            {
               /* If this request involves ARC4 or WEP_CRC, invoke 
                * NPE wep perform function 
                */
               retStatus= ixCryptoAccNpeWepPerform(reverseCtxId, 
                                                   pSrcMbuf,
                                                   pDestMbuf,
                                                   0, 
                                                   packetLength,
                                                   packetLength,
                                                   cipherKey);
            }
        } while (IX_CRYPTO_ACC_STATUS_QUEUE_FULL == retStatus);
        /* End of do-while loop */
    }
    else    /* cryptoCtxId == reverseCtxId */
    {
        do
        {
            if( IX_CRYPTO_ACC_CIPHER_ARC4 
                != ixCryptoAccCodeletService[codeletSrvIndex].cipherAlgo
                && IX_CRYPTO_ACC_AUTH_WEP_CRC 
                != ixCryptoAccCodeletService[codeletSrvIndex].authAlgo)
            {
                retStatus = ixCryptoAccAuthCryptPerform (
                            forwardCtxId,
                            pSrcMbuf,
                            pDestMbuf,
                            0,
                            packetLength,
                            0,
                            packetLength,
                            packetLength,
                            IV);    
           }else
           {
               /* If this request involves ARC4 or WEP_CRC, invoke 
                * NPE wep perform function 
                */
               retStatus= ixCryptoAccNpeWepPerform(forwardCtxId,
                                                   pSrcMbuf,
                                                   pDestMbuf,
                                                   0, 
                                                   packetLength,
                                                   packetLength,
                                                   cipherKey);

           }
                 
        } while (IX_CRYPTO_ACC_STATUS_QUEUE_FULL == retStatus);
        /* End of while loop */
    } /* End of if else (cryptoCtxId == forwardCtxId) */        
} /* End of ixCryptoAccCodeletPerformCB () function */


/**
 * @fn ixCryptoAccCodeletMain (
 *          INT32 srvIndex,
 *          UINT32 dataBLenOrOprWLen)
 *
 * @brief  This is the entry point function to the codelet to choose the 
 *         operation for the codelet and packet length to be used. 
 *         This is the main function of the codelet where crypto contexts
 *         registration and crypto perform services are done. Based on the  
 *         selected operation and selected packet length, packets are sent to 
 *         cryptoAcc for processing. 
 */
IX_STATUS ixCryptoAccCodeletMain (
               INT32 srvIndex,
               UINT32 dataBLenOrOprWLen)
{    
    IX_STATUS status = IX_SUCCESS;
    packetLength = dataBLenOrOprWLen;
    
    if(IX_CRYPTOACC_CODELET_SRV_LIST == srvIndex) 
    {
        ixCryptoAccCodeletUsageShow ();
        return IX_SUCCESS;
    }
    
    if( (IX_CRYPTOACC_CODELET_MAX_SRV_INDEX + 1) <= (UINT32)srvIndex)
    {
        printf ("\nInvalid operation type selected!\n\n");
        ixCryptoAccCodeletUsageShow ();
        return IX_FAIL;
    }    

    /* Initialize NPE's, Qmgr and IxCryptoAcc component */
    if (IX_SUCCESS !=  ixCryptoAccCodeletInit(IX_CRYPTOACC_CODELET_CRYPTOACC_CFG))
    {
        printf ("Initialisation Failed.\n");
        return IX_FAIL;
    }
    
    ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
    
    printf ("************************************************************\n");
    printf ("\tSecurity Hardware Accelerator Codelet\n");
    printf ("************************************************************\n\n");
    
    ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);

    /* Reset performance entries counter */
    performanceNumEntries = 0;
    
    /* Reset stop flag */
    trafficFlowHalt = FALSE;
    
    /* Initialize codeletPerformError flag */
    codeletPerformError = FALSE;
    
    codeletSrvIndex = srvIndex - 1;
    /* Cryptographic operations */
    if (IX_CRYPTOACC_CODELET_PKE_START_INDEX > (UINT32)srvIndex)
    {
        
        if ((IX_CRYPTOACC_CODELET_MAX_CRYPTO_PKT_SIZE < dataBLenOrOprWLen) 
            || (IX_CRYPTOACC_CODELET_MIN_CRYPTO_PKT_SIZE > dataBLenOrOprWLen))
        {
           printf ("\n Packetlength should  %d <= packetLength <= %d",
                IX_CRYPTOACC_CODELET_MIN_CRYPTO_PKT_SIZE,
                IX_CRYPTOACC_CODELET_MAX_CRYPTO_PKT_SIZE);
    
           status = IX_FAIL;
        }  
        else
        {
            if (IX_SUCCESS != ixCryptoAccCodeletPerformMain (dataBLenOrOprWLen))
            {
                printf ("ixCryptoAccCodeletPerformMain failed!\n");
                status = IX_FAIL;
            }
        }
    } /* else-if (IX_CRYPTOACC_CODELET_PKE_START_INDEX > (UINT32)srvIndex) */
#ifdef __ixp46X     /* PKE codes only applicable to ixp46x platform */
    else /* else-if (IX_CRYPTOACC_CODELET_PKE_START_INDEX > srvIndex) */
    {    
        /* PKE operations */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
                ixCryptoAccCodeletPkePerformMain (srvIndex, dataBLenOrOprWLen))
        {
            printf ("ixCryptoAccCodeletPkePerformMain failed! \n");
            status = IX_FAIL;
        }

    } /* end of if (IX_CRYPTOACC_CODELET_PKE_START_INDEX > srvIndex) */
#endif /* ixp46X */

    /* Uninitialize the codelet */
    if (IX_SUCCESS != ixCryptoAccCodeletUninit())
    {
        printf ("Fail to uninitialize crypto codelets.\n");
        status = IX_FAIL;
    }

    return status;
} /* end of ixCryptoAccCodeletMain() function */

#ifdef __ixp46X             /* PKE codes only applicable to ixp46x platform */

/**
 * @fn ixCryptoAccCodeletPkeRngPerform (void)
 *
 * @brief Get pseudo-random number operation. Each window output is 1000 times,
 *        and has 20 windows. Thus, 20000 operations are performed and time is
 *        calculated for each window (1000 operations).
 *            
 */
PRIVATE IX_STATUS 
ixCryptoAccCodeletPkeRngPerform (void)
{
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_FAIL;
    UINT32 opNum = 0;
    UINT32 timeNow = 0;
    UINT32 timeDiff = 0;
    UINT32 count = 0;
    UINT32 *pPseudoNumber = 
        IX_CRYPTO_ACC_CODELET_DRV_DMA_MALLOC (packetLength * BYTES_IN_WORD);
    
    /* Calculate 20 window output times, each with 1000 operations */
    for (performanceNumEntries = 0; 
         performanceNumEntries < PERFORMANCE_WINDOW_SIZE; 
         performanceNumEntries++) 
    {   
        /* Start the timeStamp before Perform */
        timeStamp = ixOsalTimestampGet();
        
        /* Get pseudo random number for 1000 times */
        for (opNum = 0; opNum < IX_CRYPTO_ACC_CODELET_PKE_TOTAL_OP; opNum++)
        {
            count = 0;
            do
            {
                status = ixCryptoAccPkePseudoRandomNumberGet (dataLen, pPseudoNumber);
            } while ((IX_CRYPTO_ACC_STATUS_RETRY == status) && 
                    (IX_CRYPTO_ACC_CODELET_MAX_COUNT > count++));
            /* end of do-while */             
                    
            if (IX_CRYPTO_ACC_STATUS_SUCCESS != status) 
            {
                printf ("Pke RNG perform failed with error code %d! \n", 
                    status);
                codeletPerformError = TRUE;
                return IX_FAIL;
            } /* end of if (IX_CRYPTO_ACC_STATUS_SUCCESS != status) */

        } /* end of 
           * for (opNum = 0; opNum < IX_CRYPTO_ACC_CODELET_PKE_TOTAL_OP; opNum++)
           */
        timeNow = ixOsalTimestampGet();
            
        /* Check if the timer wrap over, get the time taken and  
        * divided it by 66 to get microseconds as XScale tick is 66MHz 
        */
        if (timeNow < timeStamp)
        {
            timeDiff = ((0xffffffff - timeStamp + timeNow + 1)/
                IX_CRYPTOACC_CODELET_XSCALE_TICK);
        }
        else
        {
            timeDiff = ((timeNow - timeStamp) /
                IX_CRYPTOACC_CODELET_XSCALE_TICK);
        } /* end of if-else (timeNow < timeStamp) */
                    
        /* Store performance rate for every 1000 numbers */
        if (PERFORMANCE_WINDOW_SIZE > performanceNumEntries)
        {
            performanceLog[performanceNumEntries] = 
                timeDiff / IX_CRYPTO_ACC_CODELET_PKE_TOTAL_OP;
        }   
    } /* end of for (performanceNumEntries = 0; 
                     performanceNumEntries < PERFORMANCE_WINDOW_SIZE; 
                     performanceNumEntries++) 
       */
       
    if (NULL != pPseudoNumber)
    {
        IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE (pPseudoNumber);
    }
    operationDone = TRUE;
    return IX_SUCCESS;
} /* end of ixCryptoAccCodeletPkeRngPerform() function */

/**
 * @fn ixCryptoAccCodeletPkeEauPerform (IxCryptoAccPkeEauOperation operation)
 *
 * @brief Perform EAU operations
 */
PRIVATE IX_STATUS 
ixCryptoAccCodeletPkeEauPerform (IxCryptoAccPkeEauOperation operation)
{
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_FAIL;
    UINT32 count = 0;
    eauPerformCallbackFn = ixCryptoAccCodeletPkeEauPerformCB;
         
    /* Start the timeStamp before Perform */
    timeStamp = ixOsalTimestampGet();                
    
    /* Continue to call eau perform if status is retry (mutex is locked). 
       Fail if status is not retry or counter exceed maximum count */            
    do 
    {
        status = ixCryptoAccPkeEauPerform (
                    operation,
                    &operand,
                    eauPerformCallbackFn,
                    &opResult);
                    
    } while ((IX_CRYPTO_ACC_STATUS_RETRY == status) && 
                (IX_CRYPTO_ACC_CODELET_MAX_COUNT > count++));
    /* End of do-while loop */
    
    /* Return error if not success and set global variables of operationDone and
       codeletPerformError to true. */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
    {
        printf ("Pke Eau perform failed with error code %d! \n", status);
        codeletPerformError = TRUE;
        return IX_FAIL;        
     } 

    return IX_SUCCESS;    
} /* end of ixCryptoAccCodeletPkeEauPerform() function */

/**
 * @fn ixCryptoAccCodeletPkePerformMain (IxCryptoAccPkeIndex opIndex, UINT32 len)
 *
 * @brief Invoke PKE operations
 *
 */
PRIVATE IX_STATUS 
ixCryptoAccCodeletPkePerformMain (IxCryptoAccPkeIndex opIndex, UINT32 len)
{
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_FAIL;
    UINT32 i = 0;
    UINT32 count = 0;
    UINT8 data = 0;
    UINT8 opr1Len = 0;
    UINT8 opr2Len = 0;
    UINT8 opr3Len = 0;

    /* For EAU operation, each operand length allocates 8 bits. RNG and Hash
     * read the whole WLen of 32 bits.
     * For example: WLen = 0x00ccbbaa. 
     *      modular exponential: aa is M, bb is N, and cc is e
     *      large number modular reduction: aa is A, bb is N
     *      large number add/sub/mul: aa is A, bb is B
     *      RNG and Hash: 0x00ccbbaa
     */
    opr1Len = len >> 0;
    opr2Len = len >> BITS_IN_BYTE;
    opr3Len = len >> (BITS_IN_BYTE * 2);
    
    /* Set length for RNG and SHA */
    dataLen = len;
    
    /* Set result length to be 128 words especially for multiplication */
    opResult.dataLen = IX_CRYPTO_ACC_CODELET_MAX_OPR_LEN * 2;
    
    /* Allocate memory for result in EAU operation */
    opResult.pData = IX_CRYPTO_ACC_CODELET_DRV_DMA_MALLOC (
                        opResult.dataLen * BYTES_IN_WORD);
    
    IX_OSAL_ASSERT (opResult.pData);
    
    /* Reset operationDone flag and totalOperation */
    operationDone = FALSE;
    totalOperation = 0;
    
    /* Set the operation index to zero to match the enum */
    opIndex -= IX_CRYPTOACC_CODELET_PKE_START_INDEX;
    
    /* Perform operation according to the index */
    switch (opIndex)
    {
        /* RNG operation */
        case IX_CRYPTO_ACC_CODELET_PKE_RNG:
    
            printf ("PKE RNG pseudo-random number\n");
            printf ("number length = %d \n\n", dataLen);
            status = ixCryptoAccCodeletPkeRngPerform ();
            break;
            
        /* Hash operation */                    
        case IX_CRYPTO_ACC_CODELET_PKE_SHA:
            
            printf ("PKE SHA1 hashing operation\n");
            printf ("data length = %d \n\n", dataLen);
            ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
            hashPerformCallbackFn = ixCryptoAccCodeletPkeHashPerformCB;
            
            /* Allocate memory for data and digest */
            pRawData = IX_CRYPTO_ACC_CODELET_DRV_DMA_MALLOC (dataLen);       
            pDigest = 
                IX_CRYPTO_ACC_CODELET_DRV_DMA_MALLOC (IX_CRYPTO_ACC_SHA1_DIGEST_160); 
            
            /* Check malloc return a NULL pointer */
            IX_OSAL_ASSERT (pRawData);
            IX_OSAL_ASSERT (pDigest);
                       
            /* copy data into array from the ASCII table 0 (0x30) to z (0x7a)*/
            data = (INT8) 0x30;
            for (i = 0; i < dataLen; i++)
            {
                *(pRawData + i) = data++;             
                if ((INT8) 0x7b == data)
                {
                    data =(INT8) 0x30;
                }
            }

            /* Start the timeStamp before Perform */
            timeStamp = ixOsalTimestampGet();
            
            count = 0;
            /* Call perform again if retry return (mutex is lock). Exit if 
             * return status is not retry and report error.
             */
            do {
                status = ixCryptoAccPkeHashPerform (
                            IX_CRYPTO_ACC_AUTH_SHA1,
                            pRawData,
                            dataLen,
                            hashPerformCallbackFn,
                            pDigest);
                            
            } while ((IX_CRYPTO_ACC_STATUS_RETRY == status) && 
                    (IX_CRYPTO_ACC_CODELET_MAX_COUNT > count++)); 
            /* End of do-while loop */
                                                
            if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
            {
                printf ("Pke Hash perform failed with error code %d! \n", status);
                codeletPerformError = TRUE; 
                status = IX_CRYPTO_ACC_STATUS_FAIL;
            }         
            break;
        
        /* EAU modular exponential operation */
        case IX_CRYPTO_ACC_CODELET_PKE_EAU_MOD_EXP:
            
            printf ("PKE EAU modular exponential operation \n");
            printf ("M length = %d, N length = %d, e length = %d\n\n", 
                opr1Len, opr2Len, opr3Len);
            ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
            
            /* Store operands */
            operand.modExpOpr.M.pData = &opr1[0];
            operand.modExpOpr.M.dataLen = opr1Len;
            operand.modExpOpr.N.pData = &opr2[0];
            operand.modExpOpr.N.dataLen = opr2Len;
            operand.modExpOpr.e.pData = &opr3[0];
            operand.modExpOpr.e.dataLen = opr3Len;

            /* Configure exponential option before calling perform */
            ixCryptoAccPkeEauExpConfig (IX_CRYPTOACC_PKE_EAU_EXP_CONFIG_SE_MODE, 
                IX_CRYPTOACC_PKE_EAU_EXP_CONFIG_FE_MODE);
                
            status = 
                ixCryptoAccCodeletPkeEauPerform (IX_CRYPTO_ACC_OP_EAU_MOD_EXP);
            break;
            
        /* EAU big number modular reduction operation */            
        case IX_CRYPTO_ACC_CODELET_PKE_EAU_BN_MOD:
            
            printf ("PKE EAU large number modular reduction operation\n");
            printf ("A length = %d, N length = %d\n\n", opr1Len, opr2Len);
            ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
            
            /* Store operands */
            operand.bnModOpr.A.pData = &opr1[0];
            operand.bnModOpr.A.dataLen = opr1Len;
            operand.bnModOpr.N.pData = &opr2[0];
            operand.bnModOpr.N.dataLen = opr2Len;
            
            status = 
                ixCryptoAccCodeletPkeEauPerform (IX_CRYPTO_ACC_OP_EAU_BN_MOD);
            break;
        
        /* EAU big number addition operation */    
        case IX_CRYPTO_ACC_CODELET_PKE_EAU_BN_ADD:
            
            printf ("PKE EAU large number addition operation\n");
            printf ("A length = %d, B length = %d\n\n", opr1Len, opr2Len);
            ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
            
            /* Store operands */
            operand.bnAddSubMulOpr.A.pData = &opr1[0];
            operand.bnAddSubMulOpr.A.dataLen = opr1Len;
            operand.bnAddSubMulOpr.B.pData = &opr2[0];
            operand.bnAddSubMulOpr.B.dataLen = opr2Len;
            
            status = 
                ixCryptoAccCodeletPkeEauPerform (IX_CRYPTO_ACC_OP_EAU_BN_ADD);
            break;
            
        /* EAU big number subtraction operation */        
        case IX_CRYPTO_ACC_CODELET_PKE_EAU_BN_SUB:
            
            printf ("PKE EAU large number subtraction operation\n");
            printf ("A length = %d, B length = %d\n\n", opr1Len, opr2Len);
            ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
            
            /* Store operands */
            operand.bnAddSubMulOpr.A.pData = &opr1[0];
            operand.bnAddSubMulOpr.A.dataLen = opr1Len;
            operand.bnAddSubMulOpr.B.pData = &opr2[0];
            operand.bnAddSubMulOpr.B.dataLen = opr2Len;
            
            status = 
                ixCryptoAccCodeletPkeEauPerform (IX_CRYPTO_ACC_OP_EAU_BN_SUB);
            break;
            
        /* EAU big number multiplication */    
        case IX_CRYPTO_ACC_CODELET_PKE_EAU_BN_MUL:            
            
            printf ("PKE EAU large number multiplication operation\n");
            printf ("A length = %d, B length = %d\n\n", opr1Len, opr2Len);
            ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);    
                
            /* Store operands */
            operand.bnAddSubMulOpr.A.pData = &opr1[0];
            operand.bnAddSubMulOpr.A.dataLen = opr1Len;
            operand.bnAddSubMulOpr.B.pData = &opr2[0];
            operand.bnAddSubMulOpr.B.dataLen = opr2Len;
            
            status = 
                ixCryptoAccCodeletPkeEauPerform (IX_CRYPTO_ACC_OP_EAU_BN_MUL);
            break;
         
        /* Invalid operation */                
        default:
            printf ("\n Invalid operation selection \n");
            
            /* Free the memory allocated and return */
            if (NULL != opResult.pData)
            {
                IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE (opResult.pData);
            }
            return IX_FAIL;
    } /* end of switch (opIndex) */
    
    /* Wait crypto services done or if error occurred. The task is put into 
     * sleep state while waiting for all the operation callbacks to return.
     * If counter value exceed wait time, exit the loop.
     */
    count = 0;       
    do
    {
        if (0 == (count % (IX_CRYPTOACC_CODELET_PERFORM_WAIT_TIME *
            IX_CRYPTO_ACC_CODELET_SLEEP_WAIT)))  
        {
            printf ("Waiting for operation done\n");
        }
        ixOsalSleep(IX_CRYPTO_ACC_CODELET_PKE_PERFORM_WAIT_TIME);
        
    } while ((!operationDone) && (!codeletPerformError) &&
            ((IX_CRYPTOACC_CODELET_PERFORM_WAIT_TIME * 
            IX_CRYPTOACC_CODELET_MAX_TIME_WAIT) > count++));
    /* end of do-while loop */
    
    /* set status to fail if operation is not done that indicates errors or 
       timed out */
    if (!operationDone)
    {
        printf ("\nOperation timed out or operation has error.\n");
        status = IX_CRYPTO_ACC_STATUS_FAIL;
    }
    
    /* check and free memory allocated */
    if (NULL != opResult.pData)
    {
        IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE (opResult.pData);
    }
    
    /* Check and free memory allocated after all operations for sha */
    if (IX_CRYPTO_ACC_CODELET_PKE_SHA == opIndex)
    {
        if ((NULL != pDigest) && (NULL != pRawData))
        {
            IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE (pDigest);
            IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE (pRawData);
        }
    } /* end of if (IX_CRYPTO_ACC_CODELET_PKE_SHA == opIndex) */ 
                          
    /* Print result if all operations are successful */
    if(IX_CRYPTO_ACC_STATUS_SUCCESS == status)
    {
        if(!codeletPerformError)
        {
            printf ("\nOperation DONE!\n");
            ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
            /* Dump performance data (20) */
            for (i = 0; i < PERFORMANCE_WINDOW_SIZE; i++)
            {
                printf ("[%2d] The rate is %d usec per operation\n", 
                    i, performanceLog[i]);
            } /* end of for loop */
        } 
        else /* else-if (!codeletPerformError) */
        {
            printf ("\n PKE operations had errors. Statistics not printed\n");
        } /* end of if(!codeletPerformError) */
                   
        return IX_SUCCESS;
    } /* else-if(IX_CRYPTO_ACC_STATUS_SUCCESS == status) */
    else
    {
        printf ("\n PKE operations failed. Statistics not printed\n");
    } /* end of if(IX_CRYPTO_ACC_STATUS_SUCCESS == status)  */

    return IX_FAIL;
} /* end of ixCryptoAccCodeletPkePerformMain() function */

/**
 * @fn ixCryptoAccCodeletPkeHashPerformCB (
 *          UINT8 *pDigestCB,
 *          IxCryptoAccStatus status)
 *
 * @brief  Crypto Hash perform callback function to be called when the 
 *         operation completed. Performance rate is calculated in this function.
 *
 */
PRIVATE void 
ixCryptoAccCodeletPkeHashPerformCB (UINT8 *pDigestCB, IxCryptoAccStatus status)
{
    UINT32 timeNow = 0;
    UINT32 timeDiff = 0;
    UINT32 count = 0;

    /* Continue of return status is success */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
    {     
        /* Get and calculate the time after each 1000 operations */
        if (IX_CRYPTO_ACC_CODELET_PKE_TOTAL_OP <= ++totalOperation)
        {
            timeNow = ixOsalTimestampGet();
            
            /* Check if the timer wrap over, get the time taken and  
            * divided it by 66 to get microseconds as XScale tick is 66MHz 
            */
            if (timeNow < timeStamp)
            {
                timeDiff = ((0xffffffff - timeStamp + timeNow + 1)/
                    IX_CRYPTOACC_CODELET_XSCALE_TICK);
            }
            else
            {
                timeDiff = ((timeNow - timeStamp) /
                    IX_CRYPTOACC_CODELET_XSCALE_TICK);
            } /* end of if (timeNow < timeStamp) */
        
            totalOperation = 0;
        
            /* Store performance rate for every 1000 operations */
            if (PERFORMANCE_WINDOW_SIZE > performanceNumEntries)
            {
                performanceLog[performanceNumEntries] = 
                    timeDiff / IX_CRYPTO_ACC_CODELET_PKE_TOTAL_OP;
                    
                performanceNumEntries++;
                
                /* Start the timeStamp before Perform */
                timeStamp = ixOsalTimestampGet();
            }
            else /* else-if (PERFORMANCE_WINDOW_SIZE > performanceNumEntries) */
            {
                /* Finished collecting performance of all operations */    
                operationDone = TRUE;
                return;
            } /* end of if (PERFORMANCE_WINDOW_SIZE > performanceNumEntries) */
        } /* end of if (IX_CRYPTO_ACC_CODELET_PKE_TOTAL_OP == ++totalOperation) */
        
        count = 0;        
        /* call hash perform again */            
        do {
            status = ixCryptoAccPkeHashPerform (
                        IX_CRYPTO_ACC_AUTH_SHA1,
                        pRawData,
                        dataLen,
                        hashPerformCallbackFn,
                        pDigest);
                        
        } while ((IX_CRYPTO_ACC_STATUS_RETRY == status) && 
                (IX_CRYPTO_ACC_CODELET_MAX_COUNT > count++)); 
        /* End of do-while loop */                                                       
        
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            ixOsalLog (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "Pke Hash perform failed with error code %d! \n", 
                status, 0, 0, 0, 0, 0);
                    
            codeletPerformError = TRUE;
        } /* end of if (IX_CRYPTO_ACC_STATUS_SUCCESS != status) */
    } /* if-else (IX_CRYPTO_ACC_STATUS_SUCCESS == status) */
    else
    {
        ixOsalLog (
            IX_OSAL_LOG_LVL_WARNING,
            IX_OSAL_LOG_DEV_STDOUT,
            "PKE Hash Operation Failed\n", 
            0, 0, 0, 0, 0, 0);
            
        codeletPerformError = TRUE;
    } /* end of if (IX_CRYPTO_ACC_STATUS_SUCCESS == status) */
} /* end of ixCryptoAccCodeletPkeHashPerformCB() function */

/**
 * @fn ixCryptoAccCodeletPkeEauPerformCB (
 *          IxCryptoAccPkeEauOperation operation,
 *          IxCryptoAccPkeEauOpResult *pResultCB,
 *          BOOL carryOrBorrow,
 *          IxCryptoAccStatus status)
 *
 * @brief  Crypto EAU perform callback fuction to be called when the 
 *         operation completed. Performance rate is calculated in this function.
 *
 */
PRIVATE void 
ixCryptoAccCodeletPkeEauPerformCB (
    IxCryptoAccPkeEauOperation operation,
    IxCryptoAccPkeEauOpResult *pResultCB,
    BOOL carryOrBorrow,
    IxCryptoAccStatus status)
{
    UINT32 timeNow = 0;
    UINT32 timeDiff = 0;
    UINT32 count = 0;
     
    /* Continue of return status is success */                   
    if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
    {
        /* Get and calculate the time after each 100 operations */
        if (IX_CRYPTO_ACC_CODELET_PKE_EAU_TOTAL_OP <= ++totalOperation)
        {
            timeNow = ixOsalTimestampGet();

            /* Check if the timer wrap over, get the time taken and  
            * divided it by 66 to get microseconds as XScale tick is 66MHz 
            */
            if (timeNow < timeStamp)
            {
                timeDiff = ((0xffffffff - timeStamp + timeNow + 1)/
                    IX_CRYPTOACC_CODELET_XSCALE_TICK);
            }
            else
            {
                timeDiff = ((timeNow - timeStamp) /
                    IX_CRYPTOACC_CODELET_XSCALE_TICK);
            } /* end of if (timeNow < timeStamp) */
            
            totalOperation = 0;
            
            /* Store performance rate for every 100 operations */
            if (PERFORMANCE_WINDOW_SIZE > performanceNumEntries)
            {
                performanceLog[performanceNumEntries] = 
                    timeDiff / IX_CRYPTO_ACC_CODELET_PKE_EAU_TOTAL_OP;
                    
                performanceNumEntries++;
                        
                /* Start the timeStamp before Perform */
                timeStamp = ixOsalTimestampGet();
            }
            else /* else-if (PERFORMANCE_WINDOW_SIZE > performanceNumEntries) */
            {
                /* Set the flag all operations done */    
                operationDone = TRUE;
                return;
            } /* end of if (PERFORMANCE_WINDOW_SIZE > performanceNumEntries) */
        } /* end of 
           * if (IX_CRYPTO_ACC_CODELET_PKE_EAU_TOTAL_OP <= ++totalOperation)
           */
        
        count = 0;
        /* Call eau perform function again */                    
        do 
        {
            status = ixCryptoAccPkeEauPerform (
                        operation,
                        &operand,
                        eauPerformCallbackFn,
                        &opResult);
                        
        } while ((IX_CRYPTO_ACC_STATUS_RETRY == status) && 
                    (IX_CRYPTO_ACC_CODELET_MAX_COUNT > count++)); 
        /* End of do-while loop */
                            
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            ixOsalLog (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "Pke Eau perform failed with error code %d! \n", 
                status, 0, 0, 0, 0, 0);

            codeletPerformError = TRUE;
        } /* end of if (IX_CRYPTO_ACC_STATUS_SUCCESS != status) */
    }
    else /* else-if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)*/
    {
        ixOsalLog (
            IX_OSAL_LOG_LVL_WARNING,
            IX_OSAL_LOG_DEV_STDOUT,
            "Eau Operation Failed\n", 
            0, 0, 0, 0, 0, 0);
            
        codeletPerformError = TRUE; 
    } /* end of if (IX_CRYPTO_ACC_STATUS_SUCCESS == status) */
} /* end of ixCryptoAccCodeletPkeEauPerformCB() function */

#endif /* ixp46X */

/**
 * @fn ixCryptoAccCodeletPerformMain (UINT32 packetLen)
 *
 * @brief Invoke cryptographic operations
 */
PRIVATE IX_STATUS ixCryptoAccCodeletPerformMain (UINT32 packetLen)
{
    UINT32 i;
    UINT32 bufferSize; 
            
    registerCB = ixCryptoAccCodeletRegisterCB;
    performCB = ixCryptoAccCodeletPerformCB;
    
    /* Store packet length into global variable for the use in perform CB */
    packetLength = packetLen;
    
    /* Calculate the buffer size. It's the sum of packet length and
     * auth digest length (if it's combined mode of operation).
     */
    bufferSize = packetLen;

    if (IX_CRYPTO_ACC_OP_ENCRYPT !=
          ixCryptoAccCodeletService[codeletSrvIndex].frwdOperation)
    {
        bufferSize += ixCryptoAccCodeletService[codeletSrvIndex].authDigestLen;
    }
    
    /* Precompute the rate ratio used performance calculation */
    rateRatio = (bufferSize * BITS_IN_BYTE * IX_CRYPTOACC_CODELET_BATCH_LEN);

    /* Alloc Mbufs pool (extra buffer is allocated for digest by default) */
    if (IX_SUCCESS != 
        ixCryptoAccCodeletCryptoMemPoolInit (
            packetLength + IX_CRYPTOACC_CODELET_DIGEST_LEN))
    {
        ixCryptoAccCodeletDispatcherStop(IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_MODE);
        ixCryptoAccUninit();
        ixQMgrUnload();
        printf ("Mbuf pool init failed\n");
        return IX_FAIL;
    }
       
    /* Registration */
    if (IX_SUCCESS != ixCryptoAccCodeletRegisterCtx(
        codeletSrvIndex,
        &forwardCtx,
        &reverseCtx,
        &forwardCtxId,
        &reverseCtxId))
    {
        printf ("\nRegistration Failed.\n");
        return IX_FAIL;
    }

    if( IX_SUCCESS == ixCryptoAccCodeletPerform(packetLength))
    {
        if( TRUE != codeletPerformError)
        {
            ixOsalSleep (IX_CRYPTO_ACC_CODELET_SLEEP_WAIT);
            /* Dump performance data */
            for (i = 0; i < PERFORMANCE_WINDOW_SIZE; i++)
            {
                printf ("[%2d] The rate is %d Mbits/sec\n", 
                    i, performanceLog[i]);
            }
        }
        else
        {
            printf ("\n Perform callback had errors.Statistics not printed");
        }
    }else
    {
        printf ("\n ixCryptoAccCodeletPerform failed. Statistics not printed");
    }
   
    return IX_SUCCESS;

} /* end of function ixCryptoAccCodeletPerformMain () */


/**
 * @fn ixCryptoAccCodeletXScaleWepPerform 
 * @brief This function calls the ixCryptoAccXScaleWepPerform
 * function for doing ARC4/WEP_CRC requests on XScale. It also gathers
 * performance statistics.
 */

PRIVATE IX_STATUS ixCryptoAccCodeletXScaleWepPerform(
                    IX_OSAL_MBUF *pMbuf,
                    UINT32 wepPacketLength)
{
    UINT32 timeNow;     /**< Current timestamp */
    UINT32 timeDiff;    /**< Time taken to process 1000 packets in XSCALE TICK 
                         * unit
                         */ 
    IxCryptoAccStatus status ;

    /* Check if pMbuf is NULL */
    if(NULL == pMbuf )
    {
        printf ("\n Mbuf is NULL");
        return IX_CRYPTO_ACC_STATUS_FAIL;
    }

    /* Fill all the entries in the performance window */
    for(performanceNumEntries=0;
        performanceNumEntries< PERFORMANCE_WINDOW_SIZE;
        performanceNumEntries++)
    {
        /* Get the current time stamp before starting the batch */
        timeStamp = ixOsalTimestampGet();

        for( totalPacket=0; totalPacket < IX_CRYPTOACC_CODELET_BATCH_LEN; 
             totalPacket+=2)
        {
            status= ixCryptoAccXScaleWepPerform(forwardCtxId,
                                                pMbuf,
                                                NULL,
                                                0, 
                                                wepPacketLength,
                                                wepPacketLength,
                                                cipherKey);
            
            /* Check whether the request invocation was success or not */
            if(IX_CRYPTO_ACC_STATUS_SUCCESS == status)
            {
                status= ixCryptoAccXScaleWepPerform(reverseCtxId,
                                                pMbuf,
                                                NULL,
                                                0, 
                                                wepPacketLength,
                                                wepPacketLength,
                                                cipherKey);

                /* Check whether the request invocation was success or not */
                if(IX_CRYPTO_ACC_STATUS_SUCCESS != status)
                {
                    printf ("\n XScaleWepPerform failed in the reverse direction ");
                    return status;
                }
            }
            else
            {
                printf ("\n XScaleWepPerform failed with status = %d", status);
                return status;
            }
        }/* end of For(totalPacket) */
        timeNow = ixOsalTimestampGet();
                
        /* Check if the timer wrap over, get the time taken and  
         * divided it by 66 to get microseconds as XScale tick is 66MHz 
         */
        if (timeNow < timeStamp)
        {
            timeDiff = ((0xffffffff - timeStamp + timeNow + 1)/
                        IX_CRYPTOACC_CODELET_XSCALE_TICK);
        }
        else
        {
            timeDiff = ((timeNow - timeStamp) /
                        IX_CRYPTOACC_CODELET_XSCALE_TICK);
        } /* end of if-else (timeNow < timeStamp) */
                
        /* Store performance rate for every 1000 packets */
        if (PERFORMANCE_WINDOW_SIZE > performanceNumEntries)
        {
            performanceLog[performanceNumEntries] = rateRatio / timeDiff;
        }

    }/* end of performanceEntries */
    
    return IX_SUCCESS;
}

/**
 * @fn ixCryptoAccCodeletDispatcherStop (BOOL useInterrupt)
 *
 * @brief Stop QMgr dispatcher thread if QMgr dispatcher runs in poll mode 
 *        or unbind QMgr dispatcher from interrupt if it runs in interrupt mode.
 *
 */
PRIVATE IX_STATUS ixCryptoAccCodeletDispatcherStop (BOOL useInterrupt)
{
    if (useInterrupt)   /* Interrupt mode */
    {
        /* 
         * Unhook the QM QLOW dispatcher from the interrupt controller. 
         */
        if (IX_SUCCESS != ixOsalIrqUnbind (IX_OSAL_IXP400_QM1_IRQ_LVL))
        {
            printf ("Failed to unbind to QM1 interrupt\n");
            return IX_FAIL;
        }
    }
    else /* poll mode */
    {
        if (!ixCryptoAccCodeletDispatcherPollStop)
        {
            /* Set stop flag to TRUE to stop the thread */
            ixCryptoAccCodeletDispatcherPollStop = TRUE;
        }
    } /* end of if (useInterrupt) */
    return (IX_SUCCESS);
} /* end of ixCryptoAccCodeletDispatcherStop () */


/**
 * @fn ixCryptoAccCodeletUsageShow (void)
 *
 * @brief Display user guide for Linux* platform or VxWorks* platform
 */
PRIVATE void ixCryptoAccCodeletUsageShow (void)
{
    UINT32 i;
    printf ("\nCrypto Access Component Codelet User Guide\n");
    printf ("==========================================\n\n");
    
#ifdef __vxworks    
    printf ("  >ixCryptoAccCodeletMain (serviceIndex, dataBLenOrOprWLen)\n\n");
#elif __wince
    printf ("  Provide operationType and dataBLenOrOprWLen\n\n");
#else
    printf ("  >insmod ixp400_codelets_cryptoAcc.o serviceIndex=<serviceIndex> "
        "dataBLenOrOprWLen=<dataBLenOrOprWLen>\n\n");
#endif
  
    printf ("\n%4d : List configurations available\n", IX_CRYPTOACC_CODELET_SRV_LIST);

    for (i = 0 ; i < IX_CRYPTOACC_CODELET_MAX_SRV_INDEX; i++)
    { 
        if ((IX_CRYPTOACC_CODELET_PKE_START_INDEX - 1) > i)
        {
            printf ("%4d : %s\n", i+1, ixCryptoAccCodeletService[i].infoString);
        }
#ifdef __ixp46X         /* PKE codes only applicable to ixp46x platform */
        else
        {
            printf ("%4d : %s\n", i+1,
                ixCryptoAccCodeletPkeService[i-IX_CRYPTOACC_CODELET_PKE_START_INDEX+1]);
        }
#endif  /* ixp46X */
    }
    
    printf ("\n\n");
    
} /* end of ixCryptoAccCodeletUsageShow () */


#ifdef __wince
int readNumber(void)
{
    INT8 line[256];
    gets(line);
    return atoi(line);
}

int    wmain(int argc, WCHAR **argv)
{
    int cryptoAccCodeletOperationNr,cryptoAccCodeletPacketLength;
    BOOL cryptoAccCodeletRun = TRUE;
    
    ixCryptoAccCodeletUsageShow ();
    
    while(cryptoAccCodeletRun)
    {
        printf("\n");
        printf("Enter operation number: ");
        cryptoAccCodeletOperationNr = readNumber();
    
        /* Lists the set of services demonstrated */
        if (IX_CRYPTOACC_CODELET_SRV_LIST == cryptoAccCodeletOperationNr)
        {
            ixCryptoAccCodeletMain(cryptoAccCodeletOperationNr, 
                IX_CRYPTOACC_CODELET_SRV_LIST);
        }
        /* Execute services between service Index 1 to 30 */
        else
        if ((IX_CRYPTOACC_CODELET_SRV_LIST < cryptoAccCodeletOperationNr) &&
            (IX_CRYPTOACC_CODELET_MAX_SRV_INDEX >= cryptoAccCodeletOperationNr))
        {
            printf("Enter packet length: ");
            cryptoAccCodeletPacketLength = readNumber();
            ixCryptoAccCodeletMain(cryptoAccCodeletOperationNr, 
                cryptoAccCodeletPacketLength);
        }
        else
        {
            printf("\nInvalid service index -- exit the codelet!\n");
            cryptoAccCodeletRun = FALSE;
        }
    }
}
#endif

/* End of IxCryptoAccCodelet.c */

