/**
 * @file IxCryptoAccCryptoRegister_p.h
 *
 * @date October-03-2002
 *
 * @brief  Private header file for Crypto Register Module
 *
 *
 * Design Notes:
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
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


#ifndef IxCryptoAccCryptoRegister_p_H
#define IxCryptoAccCryptoRegister_p_H


/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxCryptoAcc_p.h"


#define IX_CRYPTO_HMAC_IPAD_VALUE     0x36    /**< ipad value specified in 
                                               * RFC 2104 
                                               */
#define IX_CRYPTO_HMAC_OPAD_VALUE     0x5C    /**< opad value specified in 
                                               * RFC 2104 
                                               */
#define IX_CRYPTO_AES128_KEY_LEN_IN_WORDS  4  /**< 128-bit key (4 words)
                                               */
#define IX_CRYPTO_AES192_KEY_LEN_IN_WORDS  6  /**< 192-bit key (6 words)
                                               */
#define IX_CRYPTO_AES256_KEY_LEN_IN_WORDS  8  /**< 256-bit key (8 words)
                                               */
#define IX_CRYPTO_NPE_CRYPT_CFG_CIPHER_MODE_POS  12  /**< Cipher mode position
                                                      * in NPE Crypt Config
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_CIPHER_ALGO_POS  11  /**< Cipher Algo position
                                                      * in NPE Crypt Config
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT_POS      10  /**< Encrypt dir position
                                                      * in NPE Crypt Config
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_CRYPT_MODE_POS   8   /**< Crypt mode position
                                                      * in NPE Crypt Config
                                                      */ 
                                                

/**
 * @fn ixCryptoRegisterCipherAlgoRegister
 *
 * @brief This function is responsible to register the cipher algorithm 
 *        selected by client to the IxCryptoAcc software component. 
 *        Parameters to the algorithm will be checked against the cipher 
 *        algorithm standard. If the parameters provided are not compliant
 *        to the standard or not supported by IxCryptoAcc software 
 *        component, error will be reported to the client.
 *
 * @param UINT32 [in] cryptoCtxId - Crypto Context ID points to Crypto
 *        Context in CCD databse.
 * @param IxCryptoAccCtx* [in] pAccCtx - Crypto context parameters needed 
 *        in registration
 * @param UINT32* [inout] npeCryptoParamIndex - Index to the array of NPE 
 *        Crypto Param structure array.
 * @param IxCryptoNpeOperationStatus [in] operStatus - NPE operation status 
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *             - IX_CRYPTO_ACC_STATUS_CIPHER_ALGO_NOT_SUPPORTED
 *             - IX_CRYPTO_ACC_STATUS_CIPHER_MODE_NOT_SUPPORTED
 *             - IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_KEY_LEN
 *             - IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_IV_LEN
 *             - IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_BLOCK_LEN
 *
 */
IxCryptoAccStatus
ixCryptoRegisterCipherAlgoRegister (
    UINT32 cryptoCtxId,
    IxCryptoAccCtx *pAccCtx,
    UINT32 *npeCryptoParamIndex,
    IxCryptoNpeOperationStatus operStatus); 
 
 
 
/**
 * @fn ixCryptoRegisterAuthAlgoRegister
 *
 * @brief This function is responsible to register the authentication 
 *        algorithm selected by client to the IxCryptoAcc software 
 *        component. Parameters to the algorithm will be checked against 
 *        the authentication algorithm standard. If the parameters provided
 *        are not compliant to the standard or not supported by IxCryptoAcc
 *        software component, error will be reported to the client.
 *
 * @param UINT32 [in] cryptoCtxId - Crypto Context ID points to Crypto
 *        Context in CCD databse.
 * @param IxCryptoAccCtx* [in] pAccCtx - Crypto context parameters needed 
 *        in registration
 * @param IX_OSAL_MBUF* [in] pMbufPrimaryChainVar - Empty mbuf for the use of
 *        primary chaining variable generation. This mbuf cannot be NULL
 *        if selected authentication algorithm is HMAC-SHA1 or HMAC-MD5. 
 *        Otherwise, the mbuf pointer must be NULL if WEP-CRC is selected.
 * @param IX_OSAL_MBUF* [in] pMbufSecondaryChainVar - Empty mbuf for the use of
 *        secondary chaining variable generation. This mbuf cannot be NULL
 *        if selected authentication algorithm is HMAC-SHA1 or HMAC-MD5. 
 *        Otherwise, the mbuf pointer must be NULL if WEP-CRC is selected.
 * @param UINT32* [inout] npeCryptoParamIndex - Index to the array of NPE 
 *        Crypto Param structure array.
 * @param IxCryptoNpeOperationStatus [in] operStatus - NPE operation status
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *             - IX_CRYPTO_ACC_STATUS_AUTH_ALGO_NOT_SUPPORTED
 *             - IX_CRYPTO_ACC_STATUS_AUTH_INVALID_DIGEST_LEN
 *             - IX_CRYPTO_ACC_STATUS_AUTH_INVALID_KEY_LEN
 *
 */
IxCryptoAccStatus
ixCryptoRegisterAuthAlgoRegister (
    UINT32 cryptoCtxId,
    IxCryptoAccCtx *pAccCtx,
    IX_OSAL_MBUF *pMbufPrimaryChainVar,
    IX_OSAL_MBUF *pMbufSecondaryChainVar,
    UINT32 *npeCryptoParamIndex,
    IxCryptoNpeOperationStatus operStatus);

 
 
/**
 * @fn ixCryptoRegisterChainVarMbufPrepare
 *
 * @brief Prepare input for NPE to generate initial chaining variables,
 *        the input are stored in the mbufs. Hash key is padded to 64 bytes 
 *        and each byte is XORed with 0x36 for primary chaining variables. 
 *        (key XORed with ipad, where ipad is the byte 0x36 repeated 64 
 *        times). Hash key is padded to 64 bytes and each byte is XORed with 
 *        0x5C for secondary chaining variables. (key XORed with opad, where 
 *        opad is the byte 0x5C repeated 64 times).Please refer to RFC2104
 *        (HMAC: Keyed-Hashing for Message Authentication) for details.
 *
 * @param IX_OSAL_MBUF* [in] pMbufPrimaryChainVar - Empty mbuf for the use of
 *        primary chaining variable generation.
 * @param IX_OSAL_MBUF* [in] pMbufSecondaryChainVar - Empty mbuf for the use of
 *        secondary chaining variable generation. 
 * @param IxCryptoAccCtx* [in] pAccCtx - Crypto context parameters needed 
 *        in registration
 *
 * @return none
 *
 * 
 */
void
ixCryptoRegisterChainVarMbufPrepare (
    IX_OSAL_MBUF *pMbufPrimaryChainVar,
    IX_OSAL_MBUF *pMbufSecondaryChainVar,
    IxCryptoAccCtx *pAccCtx);

 
 /**
 * @fn ixCryptoRegisterChainVariablesGenerate
 *
 * @brief Generate primary or secondary chaining variables (depends on the
 *        input of mbuf) through NPE. The result of computation will be 
 *        stored in data structure IxCryptoNpeCryptoParam, which is 
 *        associated with the cryptoCtxId. Only one initial chaining 
 *        variable is computed per each function call.
 *
 * @param UINT32 [in] cryptoCtxId - Crypto Context ID points to Crypto
 *        Context in CCD databse.
 * @param IX_OSAL_MBUF* [in] pMbufChainVar - Empty mbuf for the use of chaining
 *        variable generation.
 * @param UINT32 [in] initLength - Length of initial values in NPE Crypto
 *        Param structure
 * @param UINT32 [in] npeCryptoParamAddr - Start address of NPE Crypto 
 *        Param structure
 * @param UINT32 [in] chainVarAddr - Address for NPE to store the result of
 *        chaining variable computation
 * @param IxCryptoNpeOperationStatus [in] operStatus - NPE operation status
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_QUEUE_FULL
 *
 * 
 */
IxCryptoAccStatus
ixCryptoRegisterChainVariablesGenerate (
    UINT32 cryptoCtxId,
    IX_OSAL_MBUF *pMbufChainVar,
    UINT32 initLength,
    UINT32 npeCryptoParamAddr,
    UINT32 chainVarAddr,
    IxCryptoNpeOperationStatus operStatus);


 
 /**
 * @fn ixCryptoRegisterRevAesKeyGenerate
 *
 * @brief Generate reverse AES key needed for AES decryption operation 
 *        through the NPE. The result of computation will be stored in data
 *        structure IxCryptoNpeCryptoParam, which is associated with the 
 *        cryptoCtxId.
 *
 * @param UINT32 [in] cryptoCtxId - Crypto Context ID points to Crypto
 *        Context in CCD databse.
 * @param UINT32 [in] initLength - Length of initial values in NPE Crypto
 *        Param structure
 * @param UINT32 [in] fwdAesKeyAddr - Address for forward AES key
 * @param UINT32 [in] keyLength - AES key length in bytes
 * @param IxCryptoNpeOperationStatus [in] operStatus - NPE operation status
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_QUEUE_FULL
 *
 * 
 */
IxCryptoAccStatus
ixCryptoRegisterRevAesKeyGenerate (
    UINT32 cryptoCtxId,
    UINT32 initLength,
    UINT32 fwdAesKeyAddr,
    UINT32 keyLength,
    IxCryptoNpeOperationStatus operStatus);



#endif /* IxCryptoAccCryptoRegister_p_H */
