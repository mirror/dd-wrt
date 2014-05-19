/**
 * @file IxCryptoAccPkeHash_p.h
 *
 * @date March-23-2005
 *
 * @brief  Private header file for PKE Hash Module
 *
 *
 * Design Notes: None
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

#if defined(__ixp46X)     
/* PKE is not supported in IXP42X platform */
#ifndef IXCRYPTOACCPKEHASH_P_H
#define IXCRYPTOACCPKEHASH_P_H

/*
 * Put the user defined include files required.
 */
#define IX_OSAL_ENFORCED_LE_AC_MAPPING  /**< Force to AC mode when in LE  */
#include "IxOsal.h"
#include "IxCryptoAcc_p.h"

/* PKE HASH engine register map (offset from base address) */
#define IX_CRYPTO_PKE_HASH_REG_HCONFIG_OFFSET   0x00    /* hash config register
                                                         * offset 
                                                         */
#define IX_CRYPTO_PKE_HASH_REG_HDO_OFFSET       0x04    /* hash do register
                                                         * offset 
                                                         */
#define IX_CRYPTO_PKE_HASH_REG_HINTR_OFFSET     0x08    /* hash interrupt 
                                                         * register offset 
                                                         */
#define IX_CRYPTO_PKE_HASH_REG_HCHAIN_OFFSET    0x0C    /* hash chaining 
                                                         * variable register 
                                                         * offset 
                                                         */
#define IX_CRYPTO_PKE_HASH_REG_HDATA_OFFSET     0x10    /* hash data register
                                                         * offset 
                                                         */

/* State of hashing */
#define IX_CRYPTO_PKE_HASH_TERMINAL_STATE           0   /* Terminal processing,
                                                         * need to return digest
                                                         * and call client's 
                                                         * callback
                                                         */
#define IX_CRYPTO_PKE_HASH_FINAL_DATA_BLOCK_STATE   1   /* Final hash data block
                                                         * state / padded block 
                                                         * data processing
                                                         */
#define IX_CRYPTO_PKE_HASH_SECOND_LAST_DATA_BLOCK_STATE     2   
                                                        /* Second last hash data 
                                                         * block state / padded 
                                                         * or hash block data 
                                                         * processing
                                                         */
/* Hashing operation definition */
#define IX_CRYPTO_PKE_HASH_SHA1_CHAIN_VAR_WLEN      5   /* Length of SHA1 
                                                         * chaining variables
                                                         * in words
                                                         */
#define IX_CRYPTO_HASH_DATA_BLOCK_LENGTH \
    IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH          /* Hash data block
                                                         * length in bytes
                                                         */
#define IX_CRYPTO_PKE_HASH_DATA_BLOCK_WLEN          16  /* Length of hash 
                                                         * cdata block
                                                         * in words
                                                         */
#define IX_CRYPTO_PKE_HASH_SHA1_CHAIN_VAR_LEN       20  /* Length of SHA1 
                                                         * chaining variables
                                                         * in bytes
                                                         */
#define IX_CRYPTO_PKE_HASH_HDO_START            0x01    /* Hash Do register 
                                                         * value, by writing 1 
                                                         * will initiate the 
                                                         * hashing process
                                                         */
#define IX_CRYPTO_PKE_HASH_HINTR_CLEAR          0x01    /* Hash Interrupt  
                                                         * register value, by  
                                                         * writing 1 will clear
                                                         * the interrupt
                                                         */
#define IX_CRYPTO_PKE_HASH_HCFG_SHA1                 0  /* SHA1 algorithm */
#define IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_NO_SWAP       0  /* No byte swapping */
#define IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_BYTE_SWAP     2  /* Byte swapping */


/* Hashing engine configuration register definition
 *  Bits
 *  ------
 *  31:30   WEV - Write endianess for chaining variables   
 *  29:28   REV - Read endianess for chaining variables
 *  27:26   WED - Write endianess for data
 *  25:24   RED - Read endianess for data
 *  23:17   Reserved
 *  16      Mode (0: SHA1)
 *  15:0    Reserved
 */

/* Hash config register field offset */
#define IX_CRYPTO_PKE_HASH_HCFG_REG_WEV_LOC          30
#define IX_CRYPTO_PKE_HASH_HCFG_REG_REV_LOC          28
#define IX_CRYPTO_PKE_HASH_HCFG_REG_WED_LOC          26
#define IX_CRYPTO_PKE_HASH_HCFG_REG_RED_LOC          24
#define IX_CRYPTO_PKE_HASH_HCFG_REG_MODE_LOC         16


/* Note: Byte swapping for read/write chaining variables, read/wrute data into
 * hash engine is enabled in LE mode. This is to overcome the endianess 
 * conversion issue due to different type of data pointer pointers are used in
 * the memory read/write access. Use input is in byte pointer, while writing
 * into hash engine is word pointer.
 */
#ifndef __LITTLE_ENDIAN

#define IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_MODE \
    IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_NO_SWAP
    
#else

#define IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_MODE \
    IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_BYTE_SWAP
    
#endif

/* Hash configuration for hash config register */
#define IX_CRYPTO_PKE_HASH_HCFG_REG_SHA1_CONF \
    ((IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_MODE     \
    << IX_CRYPTO_PKE_HASH_HCFG_REG_WEV_LOC) | \
    (IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_MODE      \
    << IX_CRYPTO_PKE_HASH_HCFG_REG_REV_LOC) | \
    (IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_MODE      \
    << IX_CRYPTO_PKE_HASH_HCFG_REG_WED_LOC) | \
    (IX_CRYPTO_PKE_HASH_HCFG_ENDIAN_MODE      \
    << IX_CRYPTO_PKE_HASH_HCFG_REG_RED_LOC) | \
    (IX_CRYPTO_PKE_HASH_HCFG_SHA1             \
    << IX_CRYPTO_PKE_HASH_HCFG_REG_MODE_LOC))


    
/**
 * @struct  IxCryptoPkeHashStats
 * @brief   Data structure for PKE hash operation statistics 
 *          Database
 */
typedef struct
{
    UINT32 pkeHashReqCount;         /* Counter for hash request on PKE */
    UINT32 pkeHashReqDoneCount;     /* Counter for hash request done on PKE */
} IxCryptoPkeHashStats;


/**
 * @struct  IxCryptoPkeHashStateInfo
 * @brief   Data structure to store information needed in PKE hash state 
 *          machine.
 */
typedef struct
{
    
    UINT32 hashBlockCount;          /* Hash data block count left to be hashed.
                                     * If hashBlockCount == 1, it is final
                                     * block of data, send in final hash data
                                     * block stored in pPaddedBlockHashData for
                                     * hashing. 
                                     * If hashBlockCount == 0, hash operation
                                     * done, read final digest and
                                     * call client's callback.
                                     */
    UINT8  *pBlockHashData;         /* Hash data blocks pointer */
    UINT8  *pPaddedBlockHashData;   /* Padded hash data block pointer */
    UINT32 padDataBlockOffset;      /* Padded hash data blocks at maximum are
                                     * 2 blocks. Offset to the first padded
                                     * has data block is 0. While second padded
                                     * hash data block offset is 64. Based on 
                                     * the offset indicated, the hash state 
                                     * machine will decide which padded hash 
                                     * data block to be used in particular state. 
                                     */
    
    UINT8  *pDigest;                /* Pointer to store final digest */
    IxCryptoAccPkeHashPerformCompleteCallback hashPerformCallbackFn;
                                    /* Client's callback when operation done */
} IxCryptoPkeHashStateInfo;


/**
 * @struct  IxCryptoPkeHashRegBaseAddr
 * @brief   Data structure to store base address of registers in PKE hash
 *          engine
 */
typedef struct
{
    UINT32 hashConfigReg;           /* hash config register base address */
    UINT32 hashDoReg;               /* hash DO register base address */
    UINT32 hashInterruptReg;        /* hash interrupt register base address */
    UINT32 hashChainVarReg;         /* hash chain variables register base 
                                     * address 
                                     */
    UINT32 hashDataReg;             /* hash data register base address */
} IxCryptoPkeHashRegAddr;



/**
 * @fn ixCryptoPkeHashInit
 *
 * @brief This function is responsible to initialize resources used and bind the 
 *        callback function pointer to interrupt level for PKE Hash module. 
 *
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoPkeHashInit (void);


/**
 * @fn ixCryptoPkeHashUninit
 *
 * @brief This function is responsible to free resources used and unbind the 
 *        callback function pointer from interrupt level for PKE Hash module. 
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoPkeHashUninit (void);


/**
 * @fn ixCryptoPkeHashPrepNStateInfoStore
 *
 * @brief This function is responsible to store the function pointer and data
 *        pointers into state info variable. This function is also responsible
 *        to check if the hash engine is free to perform the requested 
 *        operation.
 *
 * @return IxCryptoAccStatus
 *           - IX_CRYPTO_ACC_STATUS_SUCCESS
 *           - IX_CRYPTO_ACC_STATUS_RETRY
 *
 */
IxCryptoAccStatus
ixCryptoPkeHashPrepNStateInfoStore (
    UINT8 *pHashData,
    UINT8 *pDigest,
    IxCryptoAccPkeHashPerformCompleteCallback hashPerformCallbackFn);


/**
 * @fn ixCryptoPkeHashDataPad
 *
 * @brief This function is responsible to pad the hash data according to 
 *        padding scheme in RFC3174. The last hash data block from client
 *        will be copied into the pre-allocated padded hash data block for
 *        padding purpose. The padding will be done in padded hash data block
 *        instead of client's hash data block. This is to avoid changing the
 *        content of hash data from client. 
 *        
 * @return None
 *
 */
void ixCryptoPkeHashDataPad (UINT32 hashDataLen);


/**
 * @fn ixCryptoPkeHashConfigStart
 *
 * @brief This function will configure the hash engine to operate in SHA1
 *        mode, initialize hash engine with initial chaining variables and
 *        then start hashing on first block of hash data
 *        
 * @return None
 *
 */
void ixCryptoPkeHashConfigStart (void);


/**
 * @fn ixCryptoPkeHashDoneIsr
 *
 * @brief Callback which is binded to PKE Hash Done interrupt. 
 *        Once the interrupt is triggered, the callback is executed.
 *        This function acts like a state machine to continue from where it left
 *        off for the hashing operation, and call the client's callback in
 *        terminal state to return the digest computed.
 *
 * @return None
 *
 */
void
ixCryptoPkeHashDoneIsr (void);


/**
 * @fn ixCryptoPkeHashShow
 *
 * @brief This function is responsible to show the statistic counter for Hashing
 *        operation and also PKE hash engine status.
 *
 * @return none
 *
 */
void ixCryptoPkeHashShow (void);

#endif /* IXCRYPTOACCPKEHASH_P_H */
#endif /* __ixp46X */
