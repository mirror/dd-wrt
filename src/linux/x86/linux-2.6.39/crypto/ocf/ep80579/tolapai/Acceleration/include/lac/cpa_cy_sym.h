/***************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 ***************************************************************************/

/*
 *****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * 
 * @defgroup cpaCySym Symmetric Cipher and Hash Crypto API
 * 
 * @ingroup cpaCy
 * 
 * @description
 *      These functions specify the Cryptographic Component API for 
 *      symmetric cipher, hash, and combined cipher and hash operations. 
 * 
 *****************************************************************************/

#ifndef CPA_CY_SYM_H
#define CPA_CY_SYM_H

/*****************************************************************************/
#include "cpa_cy_common.h"

/*****************************************************************************/

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Cryptographic component symmetric session context handle.  
 * @description
 *      Handle to a cryptographic session context. The memory for this handle
 *      is allocated by the client. The size of the memory that the client needs
 *      to allocate is determined by a call to the @ref 
 *      cpaCySymSessionCtxGetSize function. The session context memory is 
 *      initialized with a call to the @ref cpaCySymInitSession function.
 *      This memory MUST not be freed until a call to @ref 
 *      cpaCySymRemoveSession has completed successfully. 
 *
 *****************************************************************************/
typedef void * CpaCySymSessionCtx;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Packet type for the cpaCySymPerformOp function
 * @description
 *	    Enumeration which is used to indicate to the symmetric crypto perform 
 *      function what type of packet the operation is required to be invoked 
 *      on. The permitted types are a full packet, a partial packet, or the 
 *      last part of a multi-part partial packet. Multi-part cipher and hash 
 *      operations are useful when processing needs to be performed on a 
 *      message which is available to the client in multiple parts (for 
 *      example due to network fragmentation of the packet).
 * @note
 *      Partial packet processing is only supported for in-place cipher or 
 *      in-place hash or in-place authentication operations. It does not apply
 *      to hash mode nested or algorithm chaining operations. The term 
 *      "in-place operations" means that the result of the cipher or hash is
 *      written back into the source buffer. 
 * @see 
 *      cpaCySymPerformOp()
 *
 *****************************************************************************/
typedef enum _CpaCySymPacketType
{
    CPA_CY_SYM_PACKET_TYPE_FULL = 1,
    /**< Perform an operation on a full packet*/
    CPA_CY_SYM_PACKET_TYPE_PARTIAL,
    /**< Perform a partial operation and maintain the state of the partial 
     * operation within the session. This is used for either the first or 
     * subsequent packets within a partial packet flow. */
    CPA_CY_SYM_PACKET_TYPE_LAST_PARTIAL
    /**< Complete the last part of a multi-part operation */
} CpaCySymPacketType;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Types of operations supported by the cpaCySymPerformOp function.
 * @description
 *      This enumeration lists different types of operations supported by the 
 *      cpaCySymPerformOp function. The operation type is defined during 
 *      session registration and cannot be changed for a session once it has 
 *      been setup. 
 * @see
 *      cpaCySymPerformOp 
 *****************************************************************************/
typedef enum _CpaCySymOp
{
    CPA_CY_SYM_OP_CIPHER = 1,
    /**< Cipher only operation on the data */
    CPA_CY_SYM_OP_HASH,
    /**< Hash only operation on the data */
    CPA_CY_SYM_OP_ALGORITHM_CHAINING
    /**< Chain any cipher with any hash operation. The order depends on 
     * the value in the CpaCySymAlgChainOrder enum. */
} CpaCySymOp;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Cipher algorithms.
 * @description
 *      This enumeration lists supported cipher algorithms and modes.
 *
 *****************************************************************************/
typedef enum _CpaCySymCipherAlgorithm
{
    CPA_CY_SYM_CIPHER_NULL = 1,
    /**< NULL cipher algorithm. No mode applies to the NULL algorithm. */
    CPA_CY_SYM_CIPHER_ARC4,
    /**< (A)RC4 cipher algorithm */
    CPA_CY_SYM_CIPHER_AES_ECB,
    /**< AES algorithm in ECB mode */
    CPA_CY_SYM_CIPHER_AES_CBC,
    /**< AES algorithm in CBC mode */
    CPA_CY_SYM_CIPHER_AES_CTR,
    /**< AES algorithm in Counter mode */
    CPA_CY_SYM_CIPHER_AES_CCM,
    /**< AES algorithm in CCM mode. This authenticated cipher is only supported
     * when the hash mode is also set to CPA_CY_SYM_HASH_MODE_AUTH. When this 
     * cipher algorithm is used the CPA_CY_SYM_HASH_AES_CCM element of the 
     * CpaCySymHashAlgorithm enum MUST be used to set up the related 
     * CpaCySymHashSetupData structure in the session context. */
    CPA_CY_SYM_CIPHER_AES_GCM,
    /**< AES algorithm in GCM mode. This authenticated cipher is only supported
     * when the hash mode is also set to CPA_CY_SYM_HASH_MODE_AUTH. When this 
     * cipher algorithm is used the CPA_CY_SYM_HASH_AES_GCM element of the 
     * CpaCySymHashAlgorithm enum MUST be used to set up the related 
     * CpaCySymHashSetupData structure in the session context. */
    CPA_CY_SYM_CIPHER_DES_ECB,
    /**< DES algorithm in ECB mode */
    CPA_CY_SYM_CIPHER_DES_CBC,
    /**< DES algorithm in CBC mode */
    CPA_CY_SYM_CIPHER_3DES_ECB,
    /**< Triple DES algorithm in ECB mode */
    CPA_CY_SYM_CIPHER_3DES_CBC,
    /**< Triple DES algorithm in CBC mode */
    CPA_CY_SYM_CIPHER_3DES_CTR
    /**< Triple DES algorithm in CTR mode */
} CpaCySymCipherAlgorithm;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Symmetric Cipher Direction
 * @description
 *      This enum indicates the cipher direction (encryption or decryption).
 * 
 *****************************************************************************/
typedef enum _CpaCySymCipherDirection
{
    CPA_CY_SYM_CIPHER_DIRECTION_ENCRYPT = 1,
    /**< Encrypt Data */
    CPA_CY_SYM_CIPHER_DIRECTION_DECRYPT
    /**< Decrypt Data */
} CpaCySymCipherDirection;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Symmetric Cipher Setup Data.
 * @description
 *      This structure contains data relating to Cipher (Encryption and 
 *      Decryption) to set up a session. 
 * 
 *****************************************************************************/
typedef struct _CpaCySymCipherSetupData {
    CpaCySymCipherAlgorithm cipherAlgorithm;
    /**< Cipher algorithm and mode */
    Cpa32U cipherKeyLenInBytes;
    /**< Cipher key length in bytes. For AES it can be 128 bits (16 bytes),
     * 192 bits (24 bytes) or 256 bits (32 bytes).
     * For the CCM mode of operation, the only supported key length is 128 bits
     * (16 bytes). */
    Cpa8U *pCipherKey;
    /**< Cipher key */
    CpaCySymCipherDirection cipherDirection;
    /**< This parameter determines if the cipher operation is an encrypt or
     * a decrypt operation. */
} CpaCySymCipherSetupData;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Symmetric Hash mode
 * @description
 *      This enum indicates the Hash Mode.
 * 
 *****************************************************************************/
typedef enum _CpaCySymHashMode
{
    CPA_CY_SYM_HASH_MODE_PLAIN = 1,
    /**< Plain hash */
    CPA_CY_SYM_HASH_MODE_AUTH,
    /**< Authenticated hash - HMAC & AES_XCBC_MAC algorithms. This mode MUST 
     * also be set to make use of the AES GCM and AES CCM algorithms. */
    CPA_CY_SYM_HASH_MODE_NESTED
    /**< Nested hash */
} CpaCySymHashMode;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Hash algorithms.
 * @description
 *      This enumeration lists supported hash algorithms.
 *
 *****************************************************************************/
typedef enum _CpaCySymHashAlgorithm
{
    CPA_CY_SYM_HASH_MD5 = 1,
    /**< MD5 algorithm. Supported in all 3 hash modes */
    CPA_CY_SYM_HASH_SHA1,
    /**< 128 bit SHA algorithm. Supported in all 3 hash modes */
    CPA_CY_SYM_HASH_SHA224,
    /**< 224 bit SHA algorithm. Supported in all 3 hash modes */
    CPA_CY_SYM_HASH_SHA256,
    /**< 256 bit SHA algorithm. Supported in all 3 hash modes */
    CPA_CY_SYM_HASH_SHA384,
    /**< 384 bit SHA algorithm. Supported in all 3 hash modes */
    CPA_CY_SYM_HASH_SHA512,
    /**< 512 bit SHA algorithm. Supported in all 3 hash modes */
    CPA_CY_SYM_HASH_AES_XCBC,
    /**< AES XCBC algorithm. This is only supported in the hash mode 
     * CPA_CY_SYM_HASH_MODE_AUTH. */
    CPA_CY_SYM_HASH_AES_CCM,
    /**< AES algorithm in CCM mode. This authenticated cipher requires that the
     * hash mode is set to CPA_CY_SYM_HASH_MODE_AUTH. When this hash algorithm
     * is used, the CPA_CY_SYM_CIPHER_AES_CCM element of the 
     * CpaCySymCipherAlgorithm enum MUST be used to set up the related
     * CpaCySymCipherSetupData structure in the session context. */
    CPA_CY_SYM_HASH_AES_GCM
    /**< AES algorithm in GCM mode. This authenticated cipher requires that the
     * hash mode is set to CPA_CY_SYM_HASH_MODE_AUTH. When this hash algorithm
     * is used, the CPA_CY_SYM_CIPHER_AES_GCM element of the 
     * CpaCySymCipherAlgorithm enum MUST be used to set up the related
     * CpaCySymCipherSetupData structure in the session context. */
} CpaCySymHashAlgorithm;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Hash Mode Nested Setup Data.
 * @description
 *      This structure contains data relating to a hash session in 
 *      CPA_CY_SYM_HASH_MODE_NESTED mode 
 * 
 *****************************************************************************/
typedef struct _CpaCySymHashNestedModeSetupData {
    Cpa8U *pInnerPrefixData;
    /**< A pointer to a buffer holding the Inner Prefix data. For optimal
     * performance the prefix data SHOULD be 8-byte aligned. This data is 
     * prepended to the data being hashed before the inner hash operation is
     * performed. */
    Cpa32U innerPrefixLenInBytes;
    /**< The inner prefix length in bytes. The maximum size the prefix data
     * can be is 255 bytes. */
    CpaCySymHashAlgorithm outerHashAlgorithm;
    /**< The hash algorithm used for the outer hash. Note: The inner hash
     * algorithm is provided in the hash context.  */
    Cpa8U *pOuterPrefixData;
    /**< A pointer to a buffer holding the Outer Prefix data. For optimal
     * performance the prefix data SHOULD be 8-byte aligned. This data is 
     * prepended to the output from the inner hash operation before the outer 
     * hash operation is performed.*/
    Cpa32U outerPrefixLenInBytes;
    /**< The outer prefix length in bytes. The maximum size the prefix data
 * can be is 255 bytes. */
} CpaCySymHashNestedModeSetupData;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Hash Auth Mode Setup Data.
 * @description
 *      This structure contains data relating to a hash session in 
 *      CPA_CY_SYM_HASH_MODE_AUTH mode 
 * 
 *****************************************************************************/
typedef struct _CpaCySymHashAuthModeSetupData {
    Cpa8U *authKey;
    /**< Authentication key pointer. */
    Cpa32U authKeyLenInBytes;
    /**< Length of the authentication key in bytes. The key length MUST be
     * less than or equal to the block size of the algorithm. It is the clients
     * responsibility to ensure that the key length is compliant with the 
     * standard being used. For example RFC 2104, FIPS 198a.
     * For the CCM mode of operation, the only supported key length is 128 bits
     * (16 bytes). */
    Cpa32U aadLenInBytes;
    /**< The length of the additional authenticated data (AAD) in bytes.
     * This is only required for CCM and GCM modes of operation.
     * For CCM, this is the length of the B blocks (including B0) that contain
     * l(a) encoded, a itself, and any necessary padding.
     * For GCM, this is the length of A.
     * In all cases, the maximum permitted value is 240 bytes. */
} CpaCySymHashAuthModeSetupData;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Hash Setup Data.
 * @description
 *      This structure contains data relating to a hash session. The fields
 *      hashAlgorithm, hashMode and digestResultLenInBytes are common to all
 *      three hash modes and MUST be set for each mode.
 * 
 *****************************************************************************/
typedef struct _CpaCySymHashSetupData {
    CpaCySymHashAlgorithm hashAlgorithm;
    /**< Hash algorithm. For mode CPA_CY_SYM_MODE_HASH_NESTED, this is the
     * inner hash algorithm. */
    CpaCySymHashMode hashMode;
    /**< Mode of the hash operation. Valid options include plain, auth or 
     * nested hash mode. */
    Cpa32U digestResultLenInBytes;
    /**< Length of the digest to be returned. If the verify option is set
     * this specifies the length of the digest to be compared for the session*/
    CpaCySymHashAuthModeSetupData authModeSetupData;
    /**< Authentication Mode Setup Data. 
     * Only valid for mode CPA_CY_SYM_MODE_HASH_AUTH */
    CpaCySymHashNestedModeSetupData nestedModeSetupData;
    /**< Nested Hash Mode Setup Data 
     * Only valid for mode CPA_CY_SYM_MODE_HASH_NESTED */
} CpaCySymHashSetupData;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Algorithm Chaining Operation Ordering
 * @description
 *      This enum defines the ordering of operations for algorithm chaining. 
 *
 ****************************************************************************/
typedef enum _CpaCySymAlgChainOrder
{
    CPA_CY_SYM_ALG_CHAIN_ORDER_HASH_THEN_CIPHER = 1,
    /**< Perform the hash operation followed by the cipher operation. If it is
     * required that the result of the hash (i.e. The digest) is going to be 
     * included in the data to be ciphered, then:
     * a) The digest MUST be placed in the destination buffer at the location
     *    corresponding to the end of the data region to be hashed
     *    (hashStartSrcOffsetInBytes + messageLenToHashInBytes),
     *    i.e.  there must be no gaps between the start of the digest and the
     *    end of the data region to be hashed.
     * b) The messageLenToCipherInBytes member of the CpaCySymOpData 
     *    structure must be equal to the overall length of the plain text, the 
     *    digest length and any (optional) trailing data that is to be included.
     * c) The messageLenToCipherInBytes must be a multiple to the block size if
     *    a block cipher is being used. 
     * 
     * +-------------------------+
     * |         Plaintext       |
     * +-------------------------+
     * <-messageLenToHashInBytes->
     * 
     * +-------------------------+--------+------+
     * |         Plaintext       | Digest | Tail |
     * +-------------------------+--------+------+
     * <--------messageLenToCipherInBytes-------->
     * 
     * +-----------------------------------------+
     * |               Cipher Text               |
     * +-----------------------------------------+ 
     * 
     * */
    CPA_CY_SYM_ALG_CHAIN_ORDER_CIPHER_THEN_HASH
    /**< Perform the cipher operation followed by the hash operation, i.e. The
     * hash operation will work on the cipher text result of the cipher 
     * operation */
} CpaCySymAlgChainOrder;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Session Setup Data.
 * @description
 *      This structure contains data relating to setting up a session. The 
 *      client needs to complete the information in this structure in order to 
 *      setup a session. 
 *
 ****************************************************************************/
typedef struct _CpaCySymSessionSetupData  {
    CpaCyPriority sessionPriority;
    /**< Priority of this session */
    CpaCySymOp symOperation;
    /**< Operation: cipher, hash, auth cipher or chained */
    CpaCySymCipherSetupData cipherSetupData;
    /**< Cipher Setup Data for the session. This member is ignored for the 
     * CPA_CY_SYM_OP_HASH operation. */
    CpaCySymHashSetupData hashSetupData;
    /**< Hash Setup Data for a session. This member is ignored for the 
     * CPA_CY_SYM_OP_CIPHER operation*/
    CpaCySymAlgChainOrder algChainOrder;
    /**<  If this operation data structure relates to an algorithm chaining
     * session then this parameter determines the order the chained operations
     * are performed in. If this structure does not relate to an algorithm 
     * chaining session then this parameter will be ignored. */
} CpaCySymSessionSetupData ;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Cryptographic Component Operation Data.
 * @description
 *      This structure contains data relating to performing cryptographic
 *      processing on a data buffer. This request is used with
 *      cpaCySymPerformOp() call for performing cipher, hash, auth cipher
 *      or a combined hash and cipher operation.
 * 
 * @see
 *      CpaCySymPacketType
 *
 * @note
 *      If the client modifies or frees the memory referenced in this structure
 *      after it has been submitted to the cpaCySymPerformOp function, and 
 *      before it has been returned in the callback, undefined behavior will
 *      result.
 *         
 ****************************************************************************/
typedef struct _CpaCySymOpData {
    CpaCySymSessionCtx pSessionCtx;
    /**< Handle for the initialized session context*/
    CpaCySymPacketType packetType;
    /**< Selects the perform operation packet type, i.e. Complete packet, a
     * partial packet, or the final packet in a multi-part partial packet. */
    Cpa8U *pIv;
    /**< Initialization Vector or Counter. For block ciphers in CBC, CCM, 
     * and GCM mode this contains a pointer to the Initialization Vector 
     * (IV) value. For CCM this is the A0 block, while for GCM this is the Y0 
     * block. For block ciphers in CTR mode this contains a pointer to the 
     * Counter. For optimum performance, the data pointed to SHOULD be 8-byte 
     * aligned. 
     * The IV/Counter will be updated after every partial crypto operation */
    Cpa32U ivLenInBytes;
    /**< Cipher IV length in bytes.  Determines the amount of valid IV data
     * pointed to by the pIv parameter. */
    Cpa32U cryptoStartSrcOffsetInBytes;
    /**< Starting point for cipher processing - given as number of bytes 
     * from start of data in the source buffer. The result of the cipher 
     * operation will be written back into the output buffer starting
     * at this location. */
    Cpa32U messageLenToCipherInBytes;
    /**< The message length, in bytes, of the source buffer that the crypto
     * operation will be computed on. This must be a multiple to the block size
     * if a block cipher is being used. This is also the same as the result 
     * length.
     * Note: There are limitations on this length for partial 
     * operations. Refer to the cpaCySymPerformOp function description for
     * details.
     * Note: On some implementations, this length may be limited to a 16-bit
     * value (65535 bytes). */
    Cpa32U hashStartSrcOffsetInBytes;
    /**< Starting point for hash processing - given as number of bytes 
     * from start of packet in source buffer. Note: for CCM and GCM modes of
     * operation set the pAdditionalAuthData field instead. */
    Cpa32U messageLenToHashInBytes;
    /**< The message length, in bytes, of the source buffer that the hash
     * will be computed on. 
     * Note: There are limitations on this length for partial operations. 
     * Refer to the cpaCySymPerformOp function description for details.
     * Note: for CCM and GCM modes of operation set the pAdditionalAuthData
     * field instead.
     * Note: On some implementations, this length may be limited to a 16-bit
     * value (65535 bytes). */
    Cpa8U *pDigestResult;
    /**<  Pointer to the location where the digest result either exists or 
     * will be inserted. At session registration time, the client specified the
     * digest result length with the digestResultLenInBytes member of the 
     * CpaCySymHashSetupData structure. The client must allocate at least 
     * digestResultLenInBytes of physically contiguous memory at this location.
     * For partial packet processing this pointer will be ignored for all but
     * the final partial operation.
     * NOTE: The digest result will overwrite any data at this location. */
    Cpa8U *pAdditionalAuthData;
    /**< Pointer to Additional Authenticated Data (AAD) needed for 
     * authenticated cipher mechanisms - CCM and GCM. For other authentication 
     * mechanisms this pointer is ignored. The length of the data pointed to by
     * this field is set up for the session in the CpaCySymHashAuthModeSetupData
     * structure as part of the cpaCySymInitSession function call. */
    CpaBoolean digestVerify;
    /**< Compute the digest and compare it to the digest contained at the
     * location pointed to by pDigestResult. This option is only valid 
     * for full packets and for final partial packets. The number of bytes to
     * be compared is indicated by the digest output length for the session. 
     * The digest computed will not be written back to the buffer. 
     * NOTE: This is not supported for hash mode CPA_CY_SYM_HASH_MODE_NESTED*/
} CpaCySymOpData;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Cryptographic Component Statistics.
 * @description
 *      This structure contains statistics on the Symmetric Cryptographic 
 *      operations. Statistics are set to zero when the component is 
 *      initialized. 
 * 
 ****************************************************************************/
typedef struct _CpaCySymStats {
    Cpa32U numSessionsInitialized;
    /**<  Number of session initialized */
    Cpa32U numSessionsRemoved;
    /**<  Number of sessions removed */
    Cpa32U numSessionErrors;
    /**<  Number of session initialized and removed errors. */
    Cpa32U numSymOpRequests;
    /**<  Number of successful symmetric crypto operation requests. */
    Cpa32U numSymOpRequestErrors;
    /**<  Number of crypto operation requests that had an error and could 
     * not be processed. */
    Cpa32U numSymOpCompleted;
    /**<  Number of crypto operations that completed successfully. */
    Cpa32U numSymOpCompletedErrors;
    /**<  Number of crypto operations that could not be completed 
     * successfully due to errors. */
    Cpa32U numSymOpVerifyFailures;
    /**<  Number of crypto operations that completed successfully, but the 
     * result of the digest verification test was that it failed. N.B. This 
     * does not indicate an "error" condition. */
} CpaCySymStats;

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Definition of callback function  
 * 
 * @description
 *      This is the callback function prototype. The callback function is 
 *      registered by the application using the cpaCySymInitSession()
 *      function call. 
 * 
 * @context
 *      This callback function can be executed in a context that DOES NOT
 *      permit sleeping to occur. 
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in] pCallbackTag      Opaque value provided by user while making 
 *                              individual function call.
 * @param[in] status            Status of the operation. Valid values are
 *                              CPA_STATUS_SUCCESS and CPA_STATUS_FAIL.
 * @param[in] operationType     Identifies the operation type that was requested
 *                              in the cpaCySymPerformOp function.
 * @param[in] pOpData           Pointer to structure with input parameters.
 * @param[in] pDstBuffer        Caller MUST allocate a sufficiently sized 
 *                              destination buffer to hold the data output. For
 *                              out-of-place processing the data outside the
 *                              crypto regions in the source buffer are copied
 *                              into the destination buffer. To perform 
 *                              "in-place" processing set the pDstBuffer
 *                              parameter in cpaCySymPerformOp function to point
 *                              at the same location as pSrcBuffer. For optimum 
 *                              performance, the data pointed to SHOULD be 
 *                              8-byte aligned.
 * @param[in] verifyResult      This parameter is valid when the digestVerify
 *                              option is set in the CpaCySymOpData structure.
 *                              A value of CPA_TRUE indicates that the compare
 *                              succeeded. A value of CPA_FALSE indicates that 
 *                              the compare failed for an unspecified reason. 
 *
 * @retval
 *      None
 * @pre
 *      Component has been initialized.
 * @post
 *      None
 * @note
 *      None
 * @see
 *      cpaCySymInitSession(),
 *      cpaCySymRemoveSession()
 *
 *****************************************************************************/
typedef void (*CpaCySymCbFunc)(void *pCallbackTag,
        CpaStatus status,
        const CpaCySymOp operationType,
        void *pOpData,
        CpaBufferList *pDstBuffer,
        CpaBoolean verifyResult);

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Cryptographic Symmetric Session Context Size Get Function.
 * 
 * @description
 *      This function is used by the client to determine the size of the memory
 *      it must allocate in order to store the session context. This MUST be
 *      called before the client allocates the memory for the session context
 *      and before the client calls the @ref cpaCySymInitSession function. 
 * 
 * @context
 *      This is a synchronous function that can not sleep. It can be 
 *      executed in a context that does not permit sleeping.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in]  instanceHandle            Instance handle.
 * @param[in]  pSessionSetupData         Pointer to session setup data which
 *                                       contains parameters which are static
 *                                       for a given cryptographic session such
 *                                       as operation type, mechanisms, and keys
 *                                       for cipher and/or hash operations.
 * @param[out] pSessionCtxSizeInBytes    The amount of memory in bytes required
 *                                       to hold the Session Context. 
 *
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resources.
 * 
 * @pre
 *      The component has been initialized via cpaCyStartInstance function.
 * @post
 *      None
 * @note
 *      This is a synchronous function and has no completion callback 
 *      associated with it.
 * @see
 *      CpaCySymSessionSetupData
 *      cpaCySymInitSession(),
 *      cpaCySymPerformOp()
 *
 *****************************************************************************/
CpaStatus
cpaCySymSessionCtxGetSize(const CpaInstanceHandle instanceHandle,
        const CpaCySymSessionSetupData *pSessionSetupData,
        Cpa32U *pSessionCtxSizeInBytes);

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Cryptographic Component Symmetric Session Initialization Function.
 * 
 * @description
 *      This function is used by the client to initialize an asynchronous 
 *      completion callback function for the symmetric crypto operations.
 *      Clients MAY register multiple callback functions using this function. 
 *      The callback function is identified by the combination of userContext,
 *      pSymCb and session context, pSessionCtx. The session context is the
 *      handle to the session and needs to be be passed when processing calls.
 *      Callbacks on completion of operations within a session are guaranteed
 *      to be in the same order they were submitted in.
 * 
 * @context
 *      This is a synchronous function and it cannot sleep. It can be 
 *      executed in a context that does not permit sleeping.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in]  instanceHandle       Instance handle.
 * @param[in]  pSymCb               Pointer to callback function to be 
 *                                  registered. Set to NULL if the 
 *                                  cpaCySymPerformOp function is required to
 *                                  work in a synchronous manner. 
 * @param[in]  pSessionSetupData    Pointer to session setup data which contains
 *                                  parameters which are static for a given 
 *                                  cryptographic session such as operation 
 *                                  type, mechanisms, and keys for cipher and/or
 *                                  hash operations.
 * @param[out] pSessionCtx          Pointer to the memory allocated by the 
 *                                  client to store the session context. This
 *                                  will be initialized with this function. This
 *                                  value needs to be passed to subsequent 
 *                                  processing calls.
 *
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_RETRY          Resubmit the request.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resources.
 * 
 * @pre
 *      The component has been initialized via cpaCyStartInstance function.
 * @post
 *      None
 * @note
 *      This is a synchronous function and has no completion callback 
 *      associated with it.
 * @see
 *      CpaCySymSessionCtx,
 *      CpaCySymCbFunc,
 *      CpaCySymSessionSetupData,
 *      cpaCySymRemoveSession(),
 *      cpaCySymPerformOp()
 *
 *****************************************************************************/
CpaStatus
cpaCySymInitSession(const CpaInstanceHandle instanceHandle,
        const CpaCySymCbFunc pSymCb,
        const CpaCySymSessionSetupData *pSessionSetupData,
        CpaCySymSessionCtx pSessionCtx);

/**
 *****************************************************************************
  * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Cryptographic Component Symmetric Session Remove Function.
 * 
 * @description
 *      This function will remove a previously initialized session context
 *      and the installed callback handler function. Removal will fail if 
 *      outstanding calls still exist for the initialized session handle. 
 *      The client needs to retry the remove function at a later time.
 *      The memory for the session context MUST not be freed until this call 
 *      has completed successfully. 
 * 
 * @context
 *      This is a synchronous function that can not sleep. It can be 
 *      executed in a context that does not permit sleeping.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in]      instanceHandle    Instance handle.
 * @param[in,out]  pSessionCtx       Session context to be removed..
 *
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_RETRY          Resubmit the request.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resources.
 * 
 * @pre
 *      The component has been initialized via cpaCyStartInstance function.
 * @post
 *      None
 * @note
 *      Note that this is a synchronous function and has no completion callback
 *      associated with it.
 * 
 * @see
 *      CpaCySymSessionCtx,
 *      cpaCySymInitSession()
 *
 *****************************************************************************/
CpaStatus
cpaCySymRemoveSession(const CpaInstanceHandle instanceHandle,
        CpaCySymSessionCtx pSessionCtx);

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Cryptographic Component Symmetric Operation Perform Function. 
 *  
 * @description
 *      Performs a cipher, hash or combined (cipher and hash) operation on
 *      the source data buffer using supported symmetric key algorithms and 
 *      modes. This function maintains cryptographic state between calls for
 *      the partial crypto operations. If a partial crypto operation is being 
 *      performed, then on a per-session basis, the next part of the multi-part
 *      message can be submitted prior to previous parts being completed, the
 *      only limitation being that all parts must be performed in sequential
 *      order.
 *      If for any reason a client wishes to terminate the partial packet
 *      processing on the session (for example if a packet fragment was lost)
 *      then the client MUST remove the session. 
 * 
 *      When performing block based operations on a partial packet (excluding
 *      the final partial packet), the data that is to be operated on MUST be
 *      a multiple of the block size of the algorithm being used.
 * 
 *      Partial packet processing is only supported for in-place cipher or 
 *      in-place hash operations. It does not apply to nested hash mode
 *      or algorithm chaining.
 *      The data on which the partial packet operation is to be performed MUST 
 *      NOT be chained. There MUST be sufficient space in the buffer to store
 *      the result of the partial packet operation.
 * 
 *      The term "in-place" means that the result of the crypto operation is
 *      written into the source buffer. The term "out-of-place" means that the
 *      result of the crypto operation is written into the destination buffer.
 *      To perform "in-place" processing set the pDstBuffer parameter to point
 *      at the same location as the pSrcBuffer parameter
 * 
 * @context
 *      When called as an asynchronous function it cannot sleep. It can be 
 *      executed in a context that does not permit sleeping.
 *      When called as a synchronous function it may sleep. It MUST NOT be 
 *      executed in a context that DOES NOT permit sleeping.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      Yes when configured to operate in synchronous mode. 
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in]  instanceHandle   Instance handle.
 * @param[in]  pCallbackTag     Opaque data that will be returned to the client
 *                              in the callback.
 * @param[in]  pOpData          Pointer to a structure containing request 
 *                              parameters. The client code allocates the memory
 *                              for this structure. This component takes 
 *                              ownership of the memory until it is returned in
 *                              the callback.
 * @param[in]  pSrcBuffer       Caller MUST allocate source buffer and populate 
 *                              with data. For optimum performance, the data 
 *                              pointed to SHOULD be 8-byte aligned. For
 *                              block ciphers, the data passed in MUST be 
 *                              a multiple of the relevant block size. 
 *                              i.e. Padding WILL NOT be applied to the data.
 * @param[out] pDstBuffer       Caller MUST allocate a sufficiently sized 
 *                              destination buffer to hold the data output. For
 *                              out-of-place processing the data outside the
 *                              crypto regions in the source buffer are copied
 *                              into the destination buffer. To perform 
 *                              "in-place" processing set the pDstBuffer
 *                              parameter in cpaCySymPerformOp function to point
 *                              at the same location as pSrcBuffer. For optimum 
 *                              performance, the data pointed to SHOULD be 
 *                              8-byte aligned.
 * @param[out] pVerifyResult    In synchronous mode, this parameter is returned 
 *                              when the digestVerify option is set in the 
 *                              CpaCySymOpData structure. A value of CPA_TRUE 
 *                              indicates that the compare succeeded. A value of
 *                              CPA_FALSE indicates that the compare failed for
 *                              an unspecified reason. 
 *
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_RETRY          Resubmit the request.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resource.
 *  
 * @pre
 *      The component has been initialized via cpaCyStartInstance function.
 *      A Cryptographic session has been previously setup using the
 *      cpaCySymInitSession() function call. 
 * @post
 *      None
 * @note
 *      When in asynchronous mode, a callback of type CpaCySymCbFunc is 
 *      generated in response to this function call. Any errors generated during 
 *      processing are reported as part of the callback status code.
 *      Partial packet processing is only supported for in-place cipher or 
 *      in-place hash operations (meaning when the output of the cipher or
 *      hash is written back into the source buffer).
 * 
 * @see
 *      CpaCySymOpData,
 *      cpaCySymInitSession(),
 *      cpaCySymRemoveSession()
 *****************************************************************************/
CpaStatus
cpaCySymPerformOp(const CpaInstanceHandle instanceHandle,
        void *pCallbackTag,
        const CpaCySymOpData *pOpData,
        const CpaBufferList *pSrcBuffer,
        CpaBufferList *pDstBuffer,
        CpaBoolean *pVerifyResult);

/**
 *****************************************************************************
 * @file cpa_cy_sym.h
 * @ingroup cpaCySym
 *      Query symmetric cryptographic statistics for a specific instance.
 * 
 * @description
 *      This function will query a specific instance for statistics. The 
 *      user MUST allocate the CpaCySymStats structure and pass the 
 *      reference to that into this function call. This function will write 
 *      the statistic results into the passed in CpaCySymStats 
 *      structure.
 * 
 *      Note: statistics returned by this function do not interrupt current data
 *      processing and as such can be slightly out of sync with operations that
 *      are in progress during the statistics retrieval process
 *
 * @context
 *      This is a synchronous function and it can sleep. It MUST NOT be 
 *      executed in a context that DOES NOT permit sleeping.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      Yes
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in] instanceHandle         Instance handle.
 * @param[out] pSymStats              Pointer to memory into which the 
 *                                   statistics will be written.
 *
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resources.
 * 
 * @pre
 *      Component has been initialized.
 * @post
 *      None
 * @note
 *      This function operates in a synchronous manner, i.e. No asynchronous 
 *      callback will be generated.
 * @see
 *      CpaCySymStats
 *****************************************************************************/
CpaStatus
cpaCySymQueryStats(const CpaInstanceHandle instanceHandle,
        CpaCySymStats *pSymStats);

/*****************************************************************************/
#endif /* CPA_CY_SYM_H */
/*****************************************************************************/
