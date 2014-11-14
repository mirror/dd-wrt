/**
 * @file IxCryptoAccCCDMgmt_p.h
 *
 * @date October-03-2002
 *
 * @brief  Private header file for Crypto Context database(CCD) management 
 *         module
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


#ifndef IxCryptoAccCCDMgmt_p_H
#define IxCryptoAccCCDMgmt_p_H


/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxCryptoAcc_p.h"
#include "IxCryptoAccQAccess_p.h"


/* NPE Crypto Param structure size */
#define IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE \
            (((sizeof (IxCryptoNpeCryptoParam) +  \
            (IX_OSAL_CACHE_LINE_SIZE - 1)) /    \
            IX_OSAL_CACHE_LINE_SIZE) *          \
            IX_OSAL_CACHE_LINE_SIZE )

/* Crypto Context Size */
#define IX_CRYPTO_CTX_SIZE  ((sizeof (IxCryptoAccCryptoCtx)) + \
                                IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE)

/* Extra Crypto param structure needed for hash key and reverse AES key
 * generation
 */
#define IX_CRYPTO_ACC_KEY_CRYPTO_PARAM_NUM (IX_CRYPTO_ACC_NUM_OF_Q *\
                                                 IX_QMGR_Q_SIZE64)

/*
 * Global variables
 */
extern IxCryptoAccCryptoCtx ixCryptoCtx[IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS];
                                            /**< Crypto Context database */
extern IxCryptoAccKeyCryptoParam ixKeyCryptoParam[IX_CRYPTO_ACC_KEY_CRYPTO_PARAM_NUM];
                         /**< Extra NPE Crypto Param for key generation */
                                                 

/**
 * @fn ixCryptoCCDMgmtInit
 *
 * @brief Initialize CCD management module. A pool of memory will be 
 *        allocated in this function to hold the information 
 *        (IxCryptoNpeCryptoParam) shared across NPE and IxCryptoAcc 
 *        software component. The CCD database will be initialized. 
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtInit (void);



/**
 * @fn ixCryptoCCDMgmtCryptoCtxGet
 *
 * @brief Get a Crypto Context buffer from the CCD pool. The CryptoCtxId
 *        will be returned via the pCtxId pointer. CryptoCtxId will serve
 *        as index of the array to the CCD database.
 *
 * @param UINT32* [out] pCtxId - Crypto Context ID pointer
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_EXCEED_MAX_TUNNELS
 *
 */
IxCryptoAccStatus 
ixCryptoCCDMgmtCryptoCtxGet (UINT32 *pCtxId);



/**
 * @fn ixCryptoCCDMgmtCryptoCtxRelease
 *
 * @brief Release Crypto Context
 *
 * @param UINT32 [in] cryptoCtxId - Crypto Context ID
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtCryptoCtxRelease (UINT32 cryptoCtxId);



/**
 * @fn      ixCryptoCCDMgmtKeyCryptoParamGet
 *
 * @brief   Get a key ID from key crypto param pool. The keyId will serve
 *          as index of the array to the key crypto param database.
 *
 * @param UINT32* [out] pKeyId - Key ID pointer
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtKeyCryptoParamGet (UINT32 *pKeyId);



/**
 * @fn      ixCryptoCCDMgmtKeyCryptoParamRelease
 *
 * @brief   Release Crypto Param structure back to the key crypto param pool.
 *
 * @param UINT32 [in] keyId - Key ID
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtKeyCryptoParamRelease (UINT32 keyId);



/**
 * @fn ixCryptoCCDMgmtCryptoCtxReleaseAll
 *
 * @brief Release all Crypto Contexts
 *
 * @return none
 *
 */
void
ixCryptoCCDMgmtCryptoCtxReleaseAll (void);



/**
 * @fn ixCryptoCCDMgmtCtxValidCheck
 *
 * @brief To check whether the CryptoCtxId provided is valid and whether 
 *        the Crypto Context has been registered.
 *
 * @param UINT32 [in] cryptoCtxId - Crypto Context ID
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_CRYPTO_CTX_NOT_VALID
 *
 *
 * Remove INLINE keyword to resolve cannot inline warning (vxWorks). SCR1421
 */
IxCryptoAccStatus 
ixCryptoCCDMgmtCtxValidCheck (UINT32 cryptoCtxId);



/**
 * @fn ixCryptoCCDMgmtShow
 *
 * @brief To show number of Crypto Contexts have been registered
 *
 * @return none
 *
 */
void
ixCryptoCCDMgmtShow (void);



/**
 * @fn ixCryptoCCDMgmtCryptoCtxShow
 *
 * @brief To show contents of Crypto Context which has been registered
 *
 * @param UINT32 [in] cryptoCtxId - Crypto Context ID
 *
 * @return none
 *
 */
void
ixCryptoCCDMgmtCryptoCtxShow (UINT32 cryptoCtxId);



/**
 * @fn ixCryptoCCDMgmtNpeCryptoParamPoolFree
 *
 * @brief To free the memory allocated to NPE crypto param pool through 
 *        malloc function
 *
 * @return none
 *
 */
void 
ixCryptoCCDMgmtNpeCryptoParamPoolFree (void);



/**
 * @def IX_CRYPTO_CTX_IS_ALL_TASK_DONE
 *
 * @brief To check if there is any pending task in the queues for the 
 *        Crypto Context specified. If TRUE, no pending tasks.
 *
 * @param UINT32 [in] reqCount - Number of requests have been issued
 * @param UINT32 [in] reqDoneCount - Number of requests have been completed
 *
 * @return TRUE/FALSE
 */
#define IX_CRYPTO_CTX_IS_ALL_TASK_DONE(reqCount, reqDoneCount)\
            (reqCount == reqDoneCount)


#endif /* IxCryptoAccCCDMgmt_p_H */
