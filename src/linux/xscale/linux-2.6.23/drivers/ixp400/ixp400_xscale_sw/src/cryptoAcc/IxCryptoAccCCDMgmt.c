/**
 * @file IxCryptoAccCCDMgmt.c
 *
 * @date October-03-2002
 *
 * @brief  Source file for Crypto Context database(CCD) management 
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


/*
 * Put the system defined include files required.
 */


/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxCryptoAccCCDMgmt_p.h"
#include "IxCryptoAccUtilities_p.h"


/*
 * Global variables
 */
IxCryptoAccCryptoCtx ixCryptoCtx[IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS];
                                   /**< Crypto Context database */
IxCryptoAccKeyCryptoParam ixKeyCryptoParam[IX_CRYPTO_ACC_KEY_CRYPTO_PARAM_NUM];
                         /**< Extra NPE Crypto Param for key generation */


/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */
static UINT32 cryptoCtxCount = 0;  /**< Index to Crypto Context List,
                                    * Crypto Context list head pointer
                                    */
static UINT32 cryptoCtxList[IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS];
                                   /**< List of Crypto Context ID in CCD
                                    * this array is used as a FILO push-pop
                                    * list in crypto context get and release
                                    * funciton
                                    */
static INT32 cryptoCtxListLock;    /**< Protect critical sections in this
                                    * module
                                    */
static UINT8 *pCryptoParamPool = NULL;  /**< NPE Crypto Param memory
                                         * pool pointer
                                         */
static UINT32 keyCryptoParamCount = 0;  /**< Index to crypto param List for,
                                         * key generation.
                                         * Key Crypto Param list head pointer
                                         */
static UINT32 keyCryptoParamList[IX_CRYPTO_ACC_KEY_CRYPTO_PARAM_NUM];
                                   /**< List of NPE crypto param structure,
                                    * this array is used as a FILO push-pop
                                    * list in crypto param get and release
                                    * funciton
                                    */
static INT32 keyCryptoParamLock; /**< Protect critical sections in this
                                  * module
                                  */
static BOOL initDone = FALSE;           /**< flag used to prevent multiple
                                         * initialization
                                         */

#ifndef NDEBUG
static UINT32 cryptoCtxDrainCount = 0;  /**< Number of times exceed
                                         * maximum tunnels allowed
                                         */
#endif /* ndef NDEBUG */


/**
 * @fn      ixCryptoCCDMgmtInit
 * @brief   Initialize Crypto Context database
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtInit (void)
{
    UINT8 *pNpeCryptoParamPool = NULL;
    UINT32 i;

    if (!initDone)
    {
        /* Allocate memory to NPE Crypto Parameters Pool */
        pNpeCryptoParamPool = IX_CRYPTO_ACC_DRV_DMA_MALLOC(
                                  (IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS +
                                  IX_CRYPTO_ACC_KEY_CRYPTO_PARAM_NUM)
                                  * IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);

        if (NULL == pNpeCryptoParamPool)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "Memory allocation for NPE Crypto Param Pool failed.\n",
                0, 0, 0, 0, 0, 0);

            /* Memory allocation failed */
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (pNpeCryptoParamPool) */

        /* Store crypto param pool pointer, this pointer will be
         * used  in crypto param pool free function
         */
        pCryptoParamPool = pNpeCryptoParamPool;

        for (i = 0;
            i < IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS;
            i++)
        {
            /* Assign Crypto Context ID to Crypto Context list */
            cryptoCtxList[i] = i;

            /* Initialize valid bit in Crypto Context to FALSE */
            ixCryptoCtx[i].valid = FALSE;

            /* Allocate a NPE Crypto Param structure from the NPE
             * Crypto Parameters Pool and assign NPE Crypto Parameters
             * structure to Crypto Context
             */
            ixCryptoCtx[i].pNpeCryptoParam =
                (IxCryptoNpeCryptoParam *) pNpeCryptoParamPool;

            /* Move the NPE Crypto Param Pool pointer to point to next
             * NPE Crypto Param structure */
            pNpeCryptoParamPool += IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE;
        } /* end of for (i) */

        /* Set cryptoCtxList head pointer to last element in the list */
        cryptoCtxCount = IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS;

        for (i = 0;
            i < IX_CRYPTO_ACC_KEY_CRYPTO_PARAM_NUM;
            i++)
        {
            /* Assign Crypto Context ID to Crypto Context list */
            keyCryptoParamList[i] = i;

            /* Allocate a NPE Crypto Param structure from the NPE
             * Crypto Parameters Pool and assign NPE Crypto Parameters
             * structure to Key Crypto Param
             */
            ixKeyCryptoParam[i].pNpeCryptoParam =
                (IxCryptoNpeCryptoParam *) pNpeCryptoParamPool;

            /* Move the NPE Crypto Param Pool pointer to point to next
             * NPE Crypto Param structure */
            pNpeCryptoParamPool += IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE;
        } /* end of for (i) */

        /* Set keyCryptoParamList head pointer to last element in the list */
        keyCryptoParamCount = IX_CRYPTO_ACC_KEY_CRYPTO_PARAM_NUM;

        /* Set initDone flag to TRUE to indicate intialization has been
         * completed
         */
        initDone = TRUE;

        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* CCD Mgmt module has been initialized */
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (initDone) */
} /* end of ixCryptoCCDMgmtInit ()function */



/**
 * @fn      ixCryptoCCDMgmtCryptoCtxGet
 * @brief   Get a Crypto Context ID from CCD pool. The CryptoCtxId will serve
 *          as index of the array to the CCD database.
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtCryptoCtxGet (UINT32 *pCtxId)
{
    if (cryptoCtxCount > 0)  /* Unused Crypto Context available */
    {
        /* Lock cryptoCtxList to protect critical section */
        IX_CRYPTO_ACC_LOCK (cryptoCtxListLock);

        /* Get a Crypto Context ID from CCD */
        *pCtxId = cryptoCtxList[--cryptoCtxCount];

        /* Unlock */
        IX_CRYPTO_ACC_UNLOCK (cryptoCtxListLock);

        /* Reset statistic count in Crypto Context */
        ixCryptoCtx[*pCtxId].reqCount = 0;
        ixCryptoCtx[*pCtxId].reqDoneCount = 0;

        /* Clear valid flag for reverse AES key */
        ixCryptoCtx[*pCtxId].validAndKeyId = 0;
        
        /* Reset Request Type Field to invalid value */
        ixCryptoCtx[*pCtxId].reqType = IX_CRYPTO_REQ_TYPE;

        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* cryptoCtxCount <= 0) */
    {
#ifndef NDEBUG
        cryptoCtxDrainCount++;
#endif /* ndef NDEBUG */

        /* Exceed Maximum active tunnels allowed */
        return IX_CRYPTO_ACC_STATUS_EXCEED_MAX_TUNNELS;
    } /* end of if-else(cryptoCtxCount) */
} /* end of ixCryptoCCDMgmtCryptoCtxGet () function */


/**
 * @fn      ixCryptoCCDMgmtCryptoCtxRelease
 * @brief   Release Crypto Context back to the CCD pool.
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtCryptoCtxRelease (UINT32 cryptoCtxId)
{
    /* Mark valid bit in Crypto Context to FALSE */
    ixCryptoCtx[cryptoCtxId].valid = FALSE;

    if (cryptoCtxCount < IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS)
    {
        /* Clear NPE Param structure */
        ixOsalMemSet (
            ixCryptoCtx[cryptoCtxId].pNpeCryptoParam->npeCryptoInfo,
            0,
            IX_CRYPTO_MAX_CRYPTO_INFO_BYTES);
            
        /* Lock cryptoCtxList to protect critical section */
        IX_CRYPTO_ACC_LOCK (cryptoCtxListLock);

        /* Push the Crypto Context back to the CCD */
        cryptoCtxList[cryptoCtxCount++] = cryptoCtxId;

        /* Unlock */
        IX_CRYPTO_ACC_UNLOCK (cryptoCtxListLock);
            
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* cryptoCtxCount >= IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS) */
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "Crypto Context %d release failed.\n",
            cryptoCtxId,
            0, 0, 0, 0, 0);

        /* CCD overflow */
        return  IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else(cryptoCtxCount) */
} /* end of ixCryptoCCDMgmtCryptoCtxRelease () function */


/**
 * @fn      ixCryptoCCDMgmtKeyCryptoParamGet
 * @brief   Get a key ID from key crypto param pool. The keyId will serve
 *          as index of the array to the key crypto param database.
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtKeyCryptoParamGet (UINT32 *pKeyId)
{
    IxCryptoAccStatus status;

    /* Lock keyCryptoParamList to protect critical section */
    IX_CRYPTO_ACC_LOCK (keyCryptoParamLock);

    if (keyCryptoParamCount > 0)  /* Unused Crypto Param available */
    {
        /* Get a key ID from key crypto param pool */
        *pKeyId = keyCryptoParamList[--keyCryptoParamCount];

        status = IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* keyCryptoParamCount <= 0) */
    {
        /* Run out of Key Crypto Param, should not happen as the number
         * of key crypto param prepared is same as maximum og queue
         * entries
         */
        status = IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else(keyCryptoParamCount) */

    /* Unlock given mutex */
    IX_CRYPTO_ACC_UNLOCK (keyCryptoParamLock);

    return status;
} /* end of ixCryptoCCDMgmtKeyCryptoParamGet () function */



/**
 * @fn      ixCryptoCCDMgmtKeyCryptoParamRelease
 * @brief   Release Crypto Param structure back to the key crypto param pool.
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtKeyCryptoParamRelease (UINT32 keyId)
{
    IxCryptoAccStatus status;

    /* Clear NPE Param structure */
    ixOsalMemSet (
        ixKeyCryptoParam[keyId].pNpeCryptoParam->npeCryptoInfo,
        0,
        IX_CRYPTO_MAX_CRYPTO_INFO_BYTES);
            
    /* Lock keyCryptoParamList to protect critical section */
    IX_CRYPTO_ACC_LOCK (keyCryptoParamLock);

    if (keyCryptoParamCount < IX_CRYPTO_ACC_KEY_CRYPTO_PARAM_NUM)
    {
        /* Push the crypto param back to the pool */
        keyCryptoParamList[keyCryptoParamCount++] = keyId;

        status = IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* keyCryptoParamCount >= IX_CRYPTO_ACC_KEY_CRYPTO_PARAM_NUM) */
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "Key Crypto Param %d release failed.\n",
            keyId,
            0, 0, 0, 0, 0);

        /* Key Crypto Param overflow */
        status = IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else(keyCryptoParamCount) */

    /* Unlock given mutex */
    IX_CRYPTO_ACC_UNLOCK (keyCryptoParamLock);

    return status;
} /* end of ixCryptoCCDMgmtKeyCryptoParamRelease () function */



/**
 * @fn      ixCryptoCCDMgmtCryptoCtxReleaseAll
 * @brief   Release ALL Crypto Context back to the CCD pool.
 *
 */
void
ixCryptoCCDMgmtCryptoCtxReleaseAll (void)
{
    UINT32 i;

    for (i = 0;
        i < IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS;
        i++)
    {
        /* Mark valid bit in Crypto Context to FALSE */
        ixCryptoCtx[i].valid = FALSE;

        /* Assign Crypto Context ID to Crypto Context list */
        cryptoCtxList[i] = i;
        
        /* Clear NPE Param structure */
        ixOsalMemSet (
            ixCryptoCtx[i].pNpeCryptoParam->npeCryptoInfo,
            0,
            IX_CRYPTO_MAX_CRYPTO_INFO_BYTES);
    } /* end of for (cryptoCtxCount) */
    
    /* Reset Crypto Context List head pointer */
    cryptoCtxCount = IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS;

} /* end of ixCryptoCCDMgmtCryptoCtxReleaseAll () function */



/**
 * @fn      ixCryptoCCDMgmtCtxValidCheck
 * @brief   Check whether the Crypto Contex is valid
 *
 */
IxCryptoAccStatus
ixCryptoCCDMgmtCtxValidCheck (UINT32 cryptoCtxId)
{
    if ((IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS > cryptoCtxId)
        && ixCryptoCtx[cryptoCtxId].valid) /* Crypto Context valid */
    {
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* invalid Crypto Context */
    {
        return IX_CRYPTO_ACC_STATUS_CRYPTO_CTX_NOT_VALID;
    } /* end of if-else (ixCryptoCtx[cryptoCtxId].valid) */
} /* end of ixCryptoCCDMgmtCtxValidCheck () function */



/**
 * @fn      ixCryptoCCDMgmtCryptoCtxShow
 * @brief   Show the contents of Crypto Context which has been registered
 *
 */
void
ixCryptoCCDMgmtCryptoCtxShow (UINT32 cryptoCtxId)
{
    printf ("\n\nCrypto Context %5d \n", cryptoCtxId);
    printf ("---------------------\n");
    printf ("NPE operation ............. : %10x\n",
        ixCryptoCtx[cryptoCtxId].npeOperationMode.npeOperation);
    printf ("Init length ............... : %10x\n",
        ixCryptoCtx[cryptoCtxId].npeOperationMode.initLength);
    printf ("Requests Count ............ : %10u\n", 
        ixCryptoCtx[cryptoCtxId].reqCount);
    printf ("Requests Done Count ....... : %10u\n\n", 
        ixCryptoCtx[cryptoCtxId].reqDoneCount);

} /* end of ixCryptoCCDMgmtCryptoCtxShow () function */



/**
 * @fn      ixCryptoCCDMgmtShow
 * @brief   Show the numbers of Crypto Context have been registered.
 *
 */
void
ixCryptoCCDMgmtShow (void)
{
    /* CCD Management module display */
    printf ("\n\nCrypto Context Database  memory allocation\n");
    printf ("------------------------------------------\n");
    printf (
    	"CCD size in bytes ........................ : %10u bytes\n",
    	(UINT32) (IX_CRYPTO_CTX_SIZE * IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS));
    printf (
    	"Crypto Context size ...................... : %10u bytes\n", 
    	(UINT32) IX_CRYPTO_CTX_SIZE);
    printf ("Maximum Crypto Context allowed ........... : %10u\n",
        IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS);
    printf ("Number of Crypto Context registered ...... : %10u\n",
        (IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS - cryptoCtxCount));

#ifndef NDEBUG
    printf ("No of times exceed maximum tunnels allowed : %10u\n",
        cryptoCtxDrainCount);
#endif /* ndef NDEBUG */
} /* end of ixCryptoCCDMgmtShow () function */



/**
 * @fn ixCryptoCCDMgmtNpeCryptoParamPoolFree
 *
 * @brief To free the memory allocated to NPE crypto param pool through 
 *        malloc function
 *
 */
void 
ixCryptoCCDMgmtNpeCryptoParamPoolFree (void)
{
    /* Check if the pool has been allocated */
    if(NULL != pCryptoParamPool)
    {
        /* free the memory allocated */
        IX_CRYPTO_ACC_DRV_DMA_FREE (pCryptoParamPool);
        
        /* Set flag to FALSE to indicate the CCD Crypto Param pool has 
         * been freed 
         */
        initDone = FALSE;
        
        /* Reset NPE param pool pointer to NULL */
        pCryptoParamPool = NULL;
        
        printf ("CCD Crypto Param pool has been freed.\n");
    }/* end of if (pNpeCryptoParamPool) */ 
} /* end of ixCryptoCCDMgmtNpeCryptoParamPoolFree () function */
