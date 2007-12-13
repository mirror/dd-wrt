/**
 * @file IxCryptoAccDescMgmt.c
 *
 * @date October-03-2002
 *
 * @brief Source file for Descriptors Management module
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
#include "IxCryptoAccDescMgmt_p.h"
#include "IxCryptoAccUtilities_p.h"

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */
static IxCryptoQDescriptor* qDescList[MAX_DESCRIPTORS_NUM_IN_POOL];  
                                 /**< array of Q descriptors pointers,
                                  * this array is used as a FILO push-pop 
                                  * list in descriptor get and release 
                                  * funciton
                                  */
static UINT32 cryptoQDescCount = 0;   /**< Q descriptor index to qDescList 
                                         * array (Q descriptor list head 
                                         * pointer)
                                         */
static INT32 descMgmtLock;       /**< Protect critical sections in this 
                                  * module
                                  */
static UINT8 *pDescPool = NULL;  /**< Descriptor pool pointer */
static BOOL initDone = FALSE;    /**< Flag to prevent multiple 
                                  * initialization
                                  */

#ifndef NDEBUG
static UINT32 qDescDrainCount = 0; /**< Number of times run out of Q 
                                    * descriptors 
                                    */
#endif /* ndef NDEBUG */



/**
 * @fn      ixCryptoDescMgmtInit
 * @brief   Initialize descriptor pool
 *
 */
IxCryptoAccStatus
ixCryptoDescMgmtInit (void)
{
    UINT8 *pQDescPool = NULL;
    UINT32 i;
    UINT8  *pAadAddr=NULL;   
    
    if (!initDone)
    {
        /* Allocate memory to Q Descriptors Pool */
        pQDescPool = IX_CRYPTO_ACC_DRV_DMA_MALLOC (
                         (MAX_DESCRIPTORS_NUM_IN_POOL 
                         * (IX_CRYPTO_Q_DESC_SIZE
                         + IX_CRYPTO_Q_DESC_AAD_SIZE))
                         + IX_CRYPTO_Q_DESC_ADDR_WORD_ALIGNED_SIZE);

        if (NULL == pQDescPool) /* if NULL */
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR, 
                "Memory allocation for Descriptors Pool failed\n", 
                0, 0, 0, 0, 0, 0);
    
            /* Memory allocation failed */
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (pQDescPool) */
        
        /* Store descriptor pool pointer, this pointer will be 
         * used  in descriptor pool free function
         */
        pDescPool = pQDescPool;
        
        /* Align queue descriptor address to get word-aligned address.
         */
        pQDescPool = (UINT8 *) (((UINT32) (pQDescPool + 
                         IX_CRYPTO_Q_DESC_ADDR_WORD_ALIGNED_SIZE))
                         & IX_CRYPTO_Q_DESC_ADDR_MASK);
        
        /* Set the start address of AAD pool */
        pAadAddr = pQDescPool + MAX_DESCRIPTORS_NUM_IN_POOL 
                                           * IX_CRYPTO_Q_DESC_SIZE;
        
        for (i = 0;
            i < MAX_DESCRIPTORS_NUM_IN_POOL;
            i++)
        {
            /* Allocate a Q descriptor from the Q Descriptors Pool.
             * Assign Q descriptor to qDescList.
             * Assign pAadAddr from the pool.
             */
            qDescList[i] = (IxCryptoQDescriptor *) pQDescPool;
            qDescList[i]->npeQDesc.aadAddr = (UINT32) pAadAddr;

            /* Move Q descriptor pool pointer to point to next Q 
             * descriptor element 
             */
            pQDescPool += IX_CRYPTO_Q_DESC_SIZE;                        
            pAadAddr += IX_CRYPTO_Q_DESC_AAD_SIZE;

        } /* end of for (i) */

        /* Set qDescList head pointer to last element in the list */
        cryptoQDescCount = MAX_DESCRIPTORS_NUM_IN_POOL;

        /* Set flag to indicate this module has been initialized */
        initDone = TRUE;
        
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* Desc Mgmt module has been initialized */
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (initDone) */
} /* end of function ixCryptoDescMgmtInit () function */



/**
 * @fn      ixCryptoDescMgmtQDescriptorGet
 * @brief   Get a queue descriptor from a descriptor pool
 *
 */
IxCryptoAccStatus
ixCryptoDescMgmtQDescriptorGet (IxCryptoQDescriptor **pQDescriptor)
{
    IxCryptoAccStatus status;
    
    /* Lock qDescList to protect critical section */
    IX_CRYPTO_ACC_LOCK (descMgmtLock);

    if (cryptoQDescCount > 0) /* Unused Q descriptor available */
    {
        /* Get a Q descriptor from the pool */
        *pQDescriptor = qDescList[--cryptoQDescCount];
        
        /* Set NPE src and dest buffer to NULL */
        (*pQDescriptor)->npeQDesc.srcMbufAddr = (UINT32) NULL;
        (*pQDescriptor)->npeQDesc.destMbufAddr = (UINT32) NULL;
        
        /* Set XScale src and dest mbuf to NULL */
        (*pQDescriptor)->pSrcMbuf = NULL;
        (*pQDescriptor)->pDestMbuf = NULL;

        status = IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* cryptoQDescCount <= 0 */
    {
#ifndef NDEBUG
        qDescDrainCount++;
#endif /* ndef NDEBUG */
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG(
            IX_OSAL_LOG_LVL_MESSAGE,
            IX_OSAL_LOG_DEV_STDOUT,
            "Run out of Q descriptor.\n",
            0, 0, 0, 0, 0, 0);
            
        /* Run out of Q descriptor in the pool */
        status = IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (cryptoQDescCount) */

    /* Unlock given mutex */
    IX_CRYPTO_ACC_UNLOCK (descMgmtLock);
    
    return status;
} /* end of ixCryptoDescMgmtQDescriptorGet () function */



/**
 * @fn      ixCryptoDescMgmtQDescriptorRelease
 * @brief   Release a queue descriptor to a descriptor pool
 *
 */
IxCryptoAccStatus
ixCryptoDescMgmtQDescriptorRelease (IxCryptoQDescriptor *pQDescriptor)
{
    IxCryptoAccStatus status;

    /* Lock qDescList to protect critical section */
    IX_CRYPTO_ACC_LOCK (descMgmtLock);

    if (cryptoQDescCount < MAX_DESCRIPTORS_NUM_IN_POOL)
    {
        /* Push the Q descriptor back to the pool */
        qDescList[cryptoQDescCount++] = pQDescriptor;

        status = IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* cryptoQDescCount >= MAX_DESCRIPTORS_NUM_IN_POOL */
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDERR,
            "Q descriptor release failed.\n",
            0, 0, 0, 0, 0, 0);

        /* Q Descriptor Pool overflow */
        status = IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (cryptoQDescCount) */

    /* Unlock given mutex */
    IX_CRYPTO_ACC_UNLOCK (descMgmtLock);

    return status;
} /* end of ixCryptoDescMgmtQDescriptorRelease () function */



/**
 * @fn      ixCryptoDescMgmtShow
 * @brief   Show the numbers of descriptor have been used
 *
 */
void
ixCryptoDescMgmtShow (void)
{
    /* Descriptor Management statistics display */
    printf ("\n\nQ Descriptors Pool Statistics\n");
    printf ("-----------------------------\n");
    printf ("Pool size in bytes ......... : %10u bytes\n",
        (UINT32) (IX_CRYPTO_Q_DESC_SIZE * MAX_DESCRIPTORS_NUM_IN_POOL));
    printf ("Q Descriptor size .......... : %10u bytes\n",
        (UINT32) IX_CRYPTO_Q_DESC_SIZE);
    printf ("Pool depth ................. : %10u descriptors\n",
        MAX_DESCRIPTORS_NUM_IN_POOL);
    printf ("Number of Q descriptors used : %10u\n",
        (MAX_DESCRIPTORS_NUM_IN_POOL - cryptoQDescCount));

#ifndef NDEBUG
    printf ("No of times run out of Q descriptors : %10u\n", 
        qDescDrainCount);
#endif /* ndef NDEBUG */
} /* end of ixCryptoDescMgmtShow () function */



/**
 * @fn ixCryptoDescMgmtAllQDescriptorInPoolCheck
 *
 * @brief This inline function is used to check whether all the allocated 
 *        descriptors have been pushed back to the descriptors pool. If all 
 *        descriptors are unused, it indicates that there is no pending task 
 *        in either Crypto Req Q or Crypto Done Q.
 *
 */
IxCryptoAccStatus
ixCryptoDescMgmtAllQDescriptorInPoolCheck (void)
{
    /* default to fail */
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_FAIL;
    
    if (MAX_DESCRIPTORS_NUM_IN_POOL == cryptoQDescCount)
    {
        status = IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    
    return status;
} /* end of ixCryptoDescMgmtAllQDescriptorInPoolCheck () function */



/**
 * @fn ixCryptoDescMgmtDescPoolFree
 *
 * @brief To free the memory allocated to descriptor pool through malloc 
 *        function.
 *
 */
void
ixCryptoDescMgmtDescPoolFree (void)
{
    /* Check if the pool has been allocated */
    if (NULL != pDescPool)
    {
        /* free memory allocated */
        IX_CRYPTO_ACC_DRV_DMA_FREE (pDescPool);
        
        /* Set flag to FALSE to indicate the Desc pool has been freed */
        initDone = FALSE;
        
        /* Reset pool pointer to NULL */
        pDescPool = NULL;
        
        printf ("Descriptor pool has been freed.\n");
    } /* end of (pDescPool) */
} /* end of ixCryptoDescMgmtDescPoolFree () function */
