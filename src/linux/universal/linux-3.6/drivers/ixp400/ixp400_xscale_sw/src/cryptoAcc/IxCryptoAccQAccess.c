/**
 * @file IxCryptoAccQAccess.c
 *
 * @date October-03-2002
 *
 * @brief Source file for Queue Access module. This module is the 
 *        only interface to QMgr for IxCryptoAcc software component. This 
 *        module provides function to read from or write to the queue, to 
 *        show the queue status and also the notification callback for QMgr.
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
 * System defined include files required.
 */

/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxCryptoAcc_p.h"
#include "IxCryptoAccQAccess_p.h"
#include "IxCryptoAccDescMgmt_p.h"
#include "IxCryptoAccCCDMgmt_p.h"
#include "IxCryptoAccUtilities_p.h"


#define IX_CRYPTO_ACC_QM_QUEUE_DISPATCH_PRIORITY IX_QMGR_Q_PRIORITY_0
                                        /**< Priority of queue in QMgr 
                                         * dispatcher (higest priority)
                                         */
#define IX_CRYPTO_ACC_CRYPTO_REQ_Q_SOURCE 0
                                        /**< Notification callback trigger 
                                         * source. It is a dummy definition, 
                                         * not used in the code as IxCryptoAcc
                                         * always write to the queue once 
                                         * there is a request from client.
                                         */
#define IX_CRYPTO_ACC_CRYPTO_DONE_Q_SOURCE IX_QMGR_Q_SOURCE_ID_NOT_E
                                        /**< Notification callback trigger
                                         * source
                                         */
#define IX_CRYPTO_ACC_WEP_REQ_Q_SOURCE 0
                                        /**< Notification callback trigger 
                                         * source. It is a dummy definition, 
                                         * not used in the code as IxCryptoAcc
                                         * always write to the queue once 
                                         * there is a request from client.
                                         */
#define IX_CRYPTO_ACC_WEP_DONE_Q_SOURCE IX_QMGR_Q_SOURCE_ID_NOT_E
                                        /**< Notification callback trigger
                                         * source
                                         */                                         

#define IX_CRYPTO_NPE_OP_STATUS_MASK    0x00000001  /**< Q descriptor  
                                                     * authentication status 
                                                     * mask, LSB
                                                     */                                             
#define IX_CRYPTO_NPE_OP_AUTH_FAIL      0x00000001  /**< NPE Operation failed,
                                                     * authentication failed 
                                                     */                        
#define IX_CRYPTO_HW_ACCEL_QUEUE_START_INDEX    0   /**< Start index of queue 
                                                     * configuration for crypto
                                                     * hw accelerator.
                                                     */
#define IX_CRYPTO_HW_ACCEL_QUEUE_END_INDEX                                  \
                                    (IX_CRYPTO_HW_ACCEL_QUEUE_START_INDEX + \
                                    IX_CRYPTO_ACC_NUM_OF_CRYPTO_Q )
                                                    /**< End index of queue 
                                                     * configuration for crypto
                                                     * hw accelerator.
                                                     */
#define IX_CRYPTO_WEP_QUEUE_START_INDEX     IX_CRYPTO_HW_ACCEL_QUEUE_END_INDEX   
                                                    /**< Start index of queue 
                                                     * configuration for WEP
                                                     * NPE.
                                                     */
#define IX_CRYPTO_WEP_QUEUE_END_INDEX       (IX_CRYPTO_WEP_QUEUE_START_INDEX + \
                                             IX_CRYPTO_ACC_NUM_OF_WEP_NPE_Q )
                                                    /**< End index of queue 
                                                     * configuration for WEP
                                                     * NPE.
                                                     */                                                                                                                                                               

/*
* Variable declarations global to this file only.  Externs are followed by
* static variables.
*/
static BOOL initDone = FALSE;      /**< flag used to prevent multiple
                                    * initialization
                                    */
IxCryptoAccStats ixCryptoStats;    /**< Statistics data structure */

PRIVATE UINT32 startQueueIndex=0;
PRIVATE UINT32 endQueueIndex=0;

                                    
/* Queue configuration info */                                    
static IxCryptoQCfgInfo queueInfo[IX_CRYPTO_ACC_NUM_OF_Q]=
{
  { 
    IX_CRYPTO_ACC_CRYPTO_REQ_Q,       /**< Queue ID */
    "Crypto Req Q",                   /**< Queue name */
    NULL,                             /**< Notification callback */
    IX_CRYPTO_ACC_CRYPTO_REQ_Q_CB_TAG, /**< Callback tag */ 
    IX_QMGR_Q_SIZE64,                 /**< Q Size*/
    IX_QMGR_Q_ENTRY_SIZE1,            /**< Queue Entry Sizes - all Q
                                       * entries are single word entries */
    FALSE,                            /**< Disable Q notification at 
                                       * startup 
                                       */
    IX_CRYPTO_ACC_CRYPTO_REQ_Q_SOURCE,/**< Q Condition to drive callback */
    IX_QMGR_Q_WM_LEVEL0,              /**< Q Low watermark - not used */
    IX_QMGR_Q_WM_LEVEL64,             /**< Q High watermark - not used */    
  },

  { 
    IX_CRYPTO_ACC_CRYPTO_DONE_Q,        /**< Queue ID */
    "Crypto Done Q",                    /**< Queue name */
    ixCryptoQAccessReqDoneQMgrCallback, /**< Notification callback */
    IX_CRYPTO_ACC_CRYPTO_DONE_Q_CB_TAG, /**< Callback tag */ 
    IX_QMGR_Q_SIZE64,                   /**< Q Size*/
    IX_QMGR_Q_ENTRY_SIZE1,              /**< Queue Entry Sizes - all Q
                                         * entries are single word entries */
    TRUE,                               /**< Enable Q notification at 
                                         * startup 
                                         */
    IX_CRYPTO_ACC_CRYPTO_DONE_Q_SOURCE, /**< Q Condition to drive callback */
    IX_QMGR_Q_WM_LEVEL0,                /**< Q Low watermark */
    IX_QMGR_Q_WM_LEVEL64,               /**< Q High watermark - not used */    
  },
  
  { 
    IX_CRYPTO_ACC_WEP_REQ_Q,          /**< Queue ID */
    "WEP Req Q",                      /**< Queue name */
    NULL,                             /**< Notification callback */
    IX_CRYPTO_ACC_WEP_REQ_Q_CB_TAG,   /**< Callback tag */ 
    IX_QMGR_Q_SIZE64,                 /**< Q Size*/
    IX_QMGR_Q_ENTRY_SIZE1,            /**< Queue Entry Sizes - all Q
                                       * entries are single word entries */
    FALSE,                            /**< Disable Q notification at 
                                       * startup 
                                       */
    IX_CRYPTO_ACC_WEP_REQ_Q_SOURCE,   /**< Q Condition to drive callback */
    IX_QMGR_Q_WM_LEVEL0,              /**< Q Low watermark - not used */
    IX_QMGR_Q_WM_LEVEL64,             /**< Q High watermark - not used */    
  },

  { 
    IX_CRYPTO_ACC_WEP_DONE_Q,           /**< Queue ID */
    "WEP Done Q",                       /**< Queue name */
    ixCryptoQAccessWepReqDoneQMgrCallback, /**< Notification callback */
    IX_CRYPTO_ACC_WEP_DONE_Q_CB_TAG,    /**< Callback tag */ 
    IX_QMGR_Q_SIZE64,                   /**< Q Size*/
    IX_QMGR_Q_ENTRY_SIZE1,              /**< Queue Entry Sizes - all Q
                                         * entries are single word entries */
    TRUE,                               /**< Enable Q notification at 
                                         * startup 
                                         */
    IX_CRYPTO_ACC_WEP_DONE_Q_SOURCE,    /**< Q Condition to drive callback */
    IX_QMGR_Q_WM_LEVEL0,                /**< Q Low watermark */
    IX_QMGR_Q_WM_LEVEL64,               /**< Q High watermark - not used */    
  }
};


/**
 * @fn ixCryptoQAccessInit
 * @brief Queue configuration and callback registration function
 *
 */
IxCryptoAccStatus 
ixCryptoQAccessInit (IxCryptoAccCfg compCfg)
{
    UINT32 i;

    if (initDone) /* If the module has been initialized */
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (initDone) */
    
    /* Initialize all the member in the ixCryptoStats struct */
    ixCryptoStats.cryptoSuccessCounter = 0;
    ixCryptoStats.cryptoFailCounter = 0;
    ixCryptoStats.qOverflowCounter = 0;
    ixCryptoStats.qUnderflowCounter = 0;
    ixCryptoStats.qDescAddrInvalidCounter = 0;
    ixCryptoStats.wepNpeSuccessCounter = 0;
    ixCryptoStats.wepNpeFailCounter = 0;
    ixCryptoStats.wepNpeQOverflowCounter = 0;
    ixCryptoStats.wepNpeQUnderflowCounter = 0;
    ixCryptoStats.wepNpeQDescAddrInvalidCounter = 0;
    ixCryptoStats.wepXScaleSuccessCounter = 0;
    ixCryptoStats.wepXScaleFailCounter = 0;
    
    /* Check component configuration to determine which queues should be 
     * initialized
     */
    switch (compCfg)
    {
        /* XScale WEP engine only, no hw queues need to be initialized,
         * thus the function is forced return here 
         */
        case IX_CRYPTO_ACC_CFG_WEP_XSCALE_ACC_EN:
            /* Set flag to indicate this module has been initializaed */
            initDone = TRUE;
            return IX_CRYPTO_ACC_STATUS_SUCCESS;
        
        /* Crypto NPE engine only */
        case IX_CRYPTO_ACC_CFG_CRYPTO_NPE_ACC_EN: 
            startQueueIndex = IX_CRYPTO_HW_ACCEL_QUEUE_START_INDEX;
            endQueueIndex = IX_CRYPTO_HW_ACCEL_QUEUE_END_INDEX;
            break;
        
        /* WEP NPE engine only */
        case IX_CRYPTO_ACC_CFG_WEP_NPE_ACC_EN:
            startQueueIndex = IX_CRYPTO_WEP_QUEUE_START_INDEX;
            endQueueIndex = IX_CRYPTO_WEP_QUEUE_END_INDEX;
            break;
        
        /* CRYPTO NPE and WEP NPE engine only */
        case IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN:
            startQueueIndex = IX_CRYPTO_HW_ACCEL_QUEUE_START_INDEX;
            endQueueIndex = IX_CRYPTO_WEP_QUEUE_END_INDEX;
            break;
        
        /* Invalid configuration */
        default:
            return IX_CRYPTO_ACC_STATUS_FAIL;        
    }
    
    for (i = startQueueIndex; i < endQueueIndex; i++)
    {
        /* Configure each Q */
        if (IX_SUCCESS != (ixQMgrQConfig (
                              queueInfo[i].qName,
                              queueInfo[i].qId,
                              queueInfo[i].qSize,
                              queueInfo[i].qWords)))
        {
            /* configuration failed */
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (ixQMgrQConfig) */

        /* Set notification condition for Q */  
        if (queueInfo[i].qNotificationEnableAtStartup) 
        {
            /* Set Q watermarks for each Q */
            if (IX_SUCCESS != ixQMgrWatermarkSet (
                                  queueInfo[i].qId,
                                  queueInfo[i].AlmostEmptyThreshold,
                                  queueInfo[i].AlmostFullThreshold))
            {
                /* configuration failed */
                return IX_CRYPTO_ACC_STATUS_FAIL;
            } /* end of if (ixQMgrWatermarkSet) */
            
            /* Set dispatcher priority for the Q */
            if (IX_SUCCESS != ixQMgrDispatcherPrioritySet ( 
                                  queueInfo[i].qId, 
                                  IX_CRYPTO_ACC_QM_QUEUE_DISPATCH_PRIORITY))
            {
                /* configuration failed */
                return IX_CRYPTO_ACC_STATUS_FAIL;
            } /* end of if (ixQMgrDispatcherPrioritySet) */
       
            /* Register callbacks for each Q */ 
            if (IX_SUCCESS != ixQMgrNotificationCallbackSet (
                                  queueInfo[i].qId,
                                  queueInfo[i].qCallback,
                                  queueInfo[i].callbackTag))
            {
                /* callback registration failed */
                return IX_CRYPTO_ACC_STATUS_FAIL;
            } /* end of if (ixQMgrNotificationCallbackSet) */

            if (IX_SUCCESS != ixQMgrNotificationEnable (
                                   queueInfo[i].qId,
                                   queueInfo[i].qConditionSource))
            {
                /* configuration failed */
                return IX_CRYPTO_ACC_STATUS_FAIL;
            } /* end of if (ixQMgrNotificationEnable) */
        } /* end of if (queueInfo[i].qNotificationEnableAtStartup) */        
    } /* end of for (i) */
        
    /* Set flag to indicate this module has been initializaed */
    initDone = TRUE;
    
    return IX_CRYPTO_ACC_STATUS_SUCCESS;

} /* end of ixCryptoQAccessInit () function */

/**
 * @fn ixCryptoQAccessUninit
 * @brief Queue configuration uninitialisation function
 *
 */
IxCryptoAccStatus
ixCryptoQAccessUninit (void)
{
    UINT32 i;
    /* If the module has not been initialized */
    if (FALSE == initDone)
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (initDone) */

    for (i = startQueueIndex; i < endQueueIndex; i++)
    {
        if(IX_SUCCESS != ixQMgrNotificationDisable (queueInfo[i].qId))
        {
            IX_CRYPTO_ACC_LOG(
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "ixQMgrNotificationDisable Failed for %d Queue-ID\n",
                queueInfo[i].qId, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;
        }
        if(IX_SUCCESS != ixQMgrNotificationCallbackSet (
                                  queueInfo[i].qId,
                                  NULL,
                                  0))
        {
            IX_CRYPTO_ACC_LOG(
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "ixQMgrNotificationCallbackSet Failed for %d Queue-ID\n",
                queueInfo[i].qId, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;
        }
    }
    initDone = FALSE;
    return IX_CRYPTO_ACC_STATUS_SUCCESS;

} /* end of ixCryptoQAccessUninit () function */

/**
 * @fn ixCryptoQAccessReqDoneQMgrCallback
 * @brief Notification callback registered with QManager. Once the 
 *        threshold of the interrupt source is reached, the notification
 *        callback is triggered and is executed.
 *
 */
void 
ixCryptoQAccessReqDoneQMgrCallback (
    IxQMgrQId queueId, 
    IxQMgrCallbackId callbackId)
{
    UINT32 messageCount = 0;
    IxCryptoQDescriptor *pQDesc = NULL;
    IX_OSAL_MBUF *pSrcMbuf;
    IX_OSAL_MBUF *pDestMbuf;
    UINT32 operationResult;
    UINT32 cryptoCtxId;
    IX_STATUS qStatus;
    IxCryptoAccStatus status;
    UINT32 npeOperation;
    BOOL useDiffBuf = FALSE;
    UINT8 *tempIcvAddr = NULL;
    UINT32 keyId;

    do
    {
        /* Read queue entry from hardware queue */
        qStatus = ixQMgrQRead (queueId, &operationResult);
        
        /* Check if the queue read operation pass */
        if (IX_SUCCESS == qStatus)
        {
            /* Count how many messages got this callback */
            messageCount++;
    
            /* Reset src mbuf and dest mbuf pointers to NULL */
            pSrcMbuf = NULL;
            pDestMbuf = NULL;
            
            pQDesc = (IxCryptoQDescriptor *)
                        (operationResult & IX_CRYPTO_Q_DESC_ADDR_MASK);
    
            /* Convert Q descriptor pointer from physical address to virtual
             * address
             */
            pQDesc = (IxCryptoQDescriptor *) 
                        IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION(
                            (UINT32) pQDesc);
                         
            if (NULL != pQDesc) /* if pQDesc not NULL */
            {       
                /* Invalidate Q descriptor message in cache */
                IX_CRYPTO_DATA_CACHE_INVALIDATE (pQDesc, 
                                  sizeof (IxCryptoQDescriptor));
            
                cryptoCtxId = pQDesc->cryptoCtxId; /* Get Crypto Context ID */
        
                /* Check if the source mbuf is NULL */
                if (NULL != pQDesc->pSrcMbuf)
                {
                    /* Convert mbuf header and its contents from NPE format
                     * to host format
                     */
                    pSrcMbuf = ixCryptoUtilMbufFromNpeFormatConvert (
                                   pQDesc->pSrcMbuf);
                } /* end of if (pQDesc->pSrcMbuf) */
            
                /* Check if the destination mbuf is NULL */ 
                if (NULL != pQDesc->pDestMbuf)
                {
                    /* Convert mbuf header and its contents from NPE 
                     * format to host format
                     */
                    pDestMbuf = ixCryptoUtilMbufFromNpeFormatConvert (
                                    pQDesc->pDestMbuf);
                } /* end of if (pQDesc->pDestMbuf) */

                /* Get NPE operation from Q descriptor */
                npeOperation 
                    = ((pQDesc->npeQDesc.npeOperationMode.npeOperation
                           & (~IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK)) 
                           & MASK_8_BIT);
                
                if (IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE 
                    != (pQDesc->npeQDesc.npeOperationMode.npeOperation
                    & IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK))
                {
                    /* set to TRUE if non in-place operation */
                    useDiffBuf = TRUE;
                }
            
                /* Switch case based on NPE operation. Tasks need to be 
                 * executed in the callback context depending on the NPE 
                 * operation completed.
                 * 1. Check if the ICV is in the middle of data. If the ICV
                 *    is in the middle of data and the NPE operation is 
                 *    IX_CRYPTO_NPE_OP_HMAC_VER_ICV or 
                 *    IX_CRYPTO_NPE_OP_HMAC_VER_ICV_DECRYPT; copy the original 
                 *    ICV value from queue descriptor and write it back to 
                 *    source and/or destination mbuf (depends on transfer 
                 *    mode) based on the original ICV offset stored in queue 
                 *    descriptor.
                 */                
                switch (npeOperation)
                {
                    /* HMAC verification only, or combined service with
                     * HMAC verification operation. Notes : the code for 
                     * HMAC verification fall thorugh to combined service
                     * IX_CRYPTO_NPE_OP_HMAC_VER_ICV_DECRYPT
                     */
                    case IX_CRYPTO_NPE_OP_HMAC_VER_ICV:
                    case IX_CRYPTO_NPE_OP_HMAC_VER_ICV_DECRYPT:
                        
                        pQDesc->npeQDesc.authStartOffset
                           = IX_CRYPTO_CONVERT_SHORT_TO_HOST_ORDER (
                             pQDesc->npeQDesc.authStartOffset);
  
                        pQDesc->npeQDesc.authLength 
                           = IX_CRYPTO_CONVERT_SHORT_TO_HOST_ORDER (
                             pQDesc->npeQDesc.authLength);

                        /* If ICV in the middle of data */
                        if ((pQDesc->value.originalIcvOffset >= 
                            (UINT32)(pQDesc->npeQDesc.authStartOffset)) &&
                            (pQDesc->value.originalIcvOffset <
                            ((UINT32)pQDesc->npeQDesc.authStartOffset + 
                            (UINT32)pQDesc->npeQDesc.authLength)))
                        {
                            /* Get ICV address from source mbuf */
                            tempIcvAddr = (UINT8 *)
                                ixCryptoUtilMbufOffsetToAddressConvert (
                                pSrcMbuf,
                                pQDesc->value.originalIcvOffset,
                                FALSE);
                                    
                            /* Restore ICV value into source mbuf */        
                            ixOsalMemCopy (
                                tempIcvAddr, 
                                pQDesc->integrityCheckValue,
                                ixCryptoCtx[cryptoCtxId].digestLength);
                        } /* end of if (pQDesc->value.originalIcvOffset) */
                        
                        if (useDiffBuf) /* If non in-place operation */
                        {
                            /* Restore ICV value into destination mbuf */
                            ixOsalMemCopy (
                                (UINT8 *) (IX_OSAL_MBUF_MDATA (pDestMbuf) + 
                                pQDesc->value.originalIcvOffset), 
                                pQDesc->integrityCheckValue,
                                ixCryptoCtx[cryptoCtxId].digestLength);
                        }                        
                        break;
                        
                    /* Convert back the aadAddr */
                    case  IX_CRYPTO_NPE_OP_CCM_GEN_MIC:
                    case  IX_CRYPTO_NPE_OP_CCM_VER_MIC:

                        /* Convert the aadAddr to host byte order*/
                        pQDesc->npeQDesc.aadAddr 
                                = IX_CRYPTO_CONVERT_WORD_TO_HOST_ORDER (
                                       pQDesc->npeQDesc.aadAddr);

                        /* Convert the aadAddr from physical to virtual address */
                        pQDesc->npeQDesc.aadAddr 
                            = (UINT32)IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION (
                                   pQDesc->npeQDesc.aadAddr);   
                        break;

                    /* Encrypt/Decrypt only operation, HMAC generation only, 
                     * or combined service with HMAC generation operation 
                     * - Do nothing
                     */
                    default :
                        break;
                } /* end of switch case (npeOperation) */                    
                        
                /* Check Authentication status */
                if (IX_CRYPTO_NPE_OP_AUTH_FAIL 
                    != (operationResult & IX_CRYPTO_NPE_OP_STATUS_MASK)) /* pass */
                {                
                    /* Increment counter for number of requests completed
                     * successfully without error 
                     */
                    ixCryptoStats.cryptoSuccessCounter++;

                    /* Check NPE operation status flag to determine which
                     * callback function to be called
                     */
                    switch (pQDesc->operStatus)
                    {
                        /* Crypto Perform operation */
                        case IX_CRYPTO_OP_PERFORM:
            
                            /* Crypto Perform operation completed successfully */
                            status =  IX_CRYPTO_ACC_STATUS_SUCCESS;
                                    
                            /* Crypto Perform operation, call the perform 
                             * complete callback function
                             */
                            ixCryptoCtx[cryptoCtxId].performCallbackFn(
                                cryptoCtxId,
                                pSrcMbuf, 
                                pDestMbuf,
                                status);
                                        
                            break;
                
                            /* Crypto Register operation */
                        case IX_CRYPTO_OP_REGISTER:

                            /* If reverse AES key generated, need to copy the
                             * AES key into the crypto param in ixCryptoCtx for
                             * crypto perform service
                             */
                            if (IX_CRYPTO_ACC_REV_AES_KEY_VALID ==
                                (ixCryptoCtx[cryptoCtxId].validAndKeyId &
                                IX_CRYPTO_ACC_REV_AES_KEY_VALID))
                            {
                                /* Invalidate crypto context to get updated primary
                                 * and secondary chaining variables
                                 */
                                IX_CRYPTO_DATA_CACHE_INVALIDATE (
                                    ixCryptoCtx[cryptoCtxId].pNpeCryptoParam,
                                    IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);

                                keyId = ixCryptoCtx[cryptoCtxId].validAndKeyId & MASK_8_BIT;

                                /* Invalidate key crypto param to get reverse
                                 * AES key
                                 */
                                IX_CRYPTO_DATA_CACHE_INVALIDATE (
                                    ixKeyCryptoParam[keyId].pNpeCryptoParam,
                                    IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);

                                /* Copy rever AES key from key Crypto Param into
                                 * ixCryptoCtx
                                 */
                                ixOsalMemCopy (
                                    &((ixCryptoCtx[cryptoCtxId].pNpeCryptoParam)
                                    ->npeCryptoInfo[IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN]),
                                    &((ixKeyCryptoParam[keyId].pNpeCryptoParam)
                                    ->npeCryptoInfo[IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN]),
                                    ixKeyCryptoParam[keyId].keyLength);

                                /* Flush NPE Crypto Param structure from cache
                                 * into SDRAM
                                 */
                                IX_CRYPTO_DATA_CACHE_FLUSH (
                                    ixCryptoCtx[cryptoCtxId].pNpeCryptoParam,
                                    IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);

                                 /* Release key crypto param */
                                 status = ixCryptoCCDMgmtKeyCryptoParamRelease (
                                            keyId);
                            }

                            /* Crypto Register operation completed successfully */
                            status =  IX_CRYPTO_ACC_STATUS_SUCCESS;

                            /* Mark valid bit in Crypto Context to indicate the
                             * Crypto Context has been registered successfully
                             */
                            ixCryptoCtx[cryptoCtxId].valid = TRUE;

                            /* Crypto Register operation, call the register
                             * complete callback function
                             */
                            ixCryptoCtx[cryptoCtxId].registerCallbackFn(
                                cryptoCtxId,
                                pSrcMbuf,
                                status);
                                    
                            break;
                                 
                            /* Crypto Register Operation */
                        case IX_CRYPTO_OP_WAIT:
                                 
                            /* Crypto Register operation, part of the operation 
                             * completed only, wait for next completion 
                             * indication, and release the buffer
                             */
                            status = IX_CRYPTO_ACC_STATUS_WAIT;
                
                            /* Crypto Register operation, call the register 
                             * complete callback function
                             */
                            ixCryptoCtx[cryptoCtxId].registerCallbackFn(
                                cryptoCtxId,
                                pSrcMbuf,
                                status);

                            break;

                        /* Hash key generation operation */
                        case IX_CRYPTO_OP_HASH_GEN_KEY:

                            /* get keyId from QDesc */
                            keyId = pQDesc->value.keyId;

                            /* Hash key generation operation completed 
                             * successfully 
                             */
                            status =  IX_CRYPTO_ACC_STATUS_SUCCESS;

                            /* Hash key generation operation, call the hash key
                             * generation complete callback function
                             */
                            ixKeyCryptoParam[keyId].hashKeyCallbackFn(
                                keyId,
                                pSrcMbuf,
                                status);

                            /* Release key crypto param */
                            status = ixCryptoCCDMgmtKeyCryptoParamRelease (
                                         keyId);

                            break;

                            /* Default error case, the code should not reach this
                             * default case in normal operation
                             */
                        default:
                            status = IX_CRYPTO_ACC_STATUS_FAIL;

                            /* Log error message in debugging mode */
                            IX_CRYPTO_ACC_LOG(
                                IX_OSAL_LOG_LVL_ERROR,
                                IX_OSAL_LOG_DEV_STDERR,
                                "Wrong NPE Operation status for CryptoCtx %d\n",
                                cryptoCtxId,
                                0, 0, 0, 0, 0);
                                    
                            break;
                                                    
                    } /* end of switch case (pQDesc->OperStatus) */
                }
                else /* Authentication operation failed */
                {
                    status = IX_CRYPTO_ACC_STATUS_AUTH_FAIL;
                        
                    /* Increment counter for number of requests completed 
                     * but operation failed 
                     */
                    ixCryptoStats.cryptoFailCounter++;
                        
                    /* Logging error message in debugging mode */
                    IX_CRYPTO_ACC_LOG(
                        IX_OSAL_LOG_LVL_MESSAGE, 
                        IX_OSAL_LOG_DEV_STDOUT,
                        "Authentication operation failed.\n",
                        0, 0, 0, 0, 0, 0);
                            
                    /* Call client's perform complete callback function,
                     * the callback function is registered during the Crypto 
                     * Context registration process.
                     * Note: Perform compleet callback is always called for the 
                     * case with AuthStatus == Q_DESC_OPERATION_FAIL as only the
                     * crypto perform request with operation authentication will
                     * have this status.
                     */
                    ixCryptoCtx[cryptoCtxId].performCallbackFn(
                        cryptoCtxId,
                        pSrcMbuf, 
                        pDestMbuf,
                        status);
                                  
                } /* end of if-else (AuthStatus) */
                    
                /* Release Q descriptor back to descriptor pool */
                if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
                    ixCryptoDescMgmtQDescriptorRelease (pQDesc))
                {
                    /* Log error message in debugging mode */
                    IX_CRYPTO_ACC_LOG(
                        IX_OSAL_LOG_LVL_ERROR,
                        IX_OSAL_LOG_DEV_STDERR, 
                        "Q descriptor release failed for Crypto Ctx %d.\n",
                        cryptoCtxId,
                        0, 0, 0, 0, 0);
                }
                        
                /* Increment counter in the Crypto Context to keep track the 
                 * number of requests have been completed, log the number even 
                 * if operation failed
                 * Note : Only Increment the counter for crypto register and
                 *        crypto perform request, as hash key generation will
                 *        not have a valid cryptoCtxId
                 */
                if (IX_CRYPTO_OP_HASH_GEN_KEY != pQDesc->operStatus)
                {
                    ixCryptoCtx[cryptoCtxId].reqDoneCount++;
                }
            }
            else /* if pQDesc is NULL */
            {
                /* Increment counter for number of Q descriptor address received
                 * is NULL
                 */
                ixCryptoStats.qDescAddrInvalidCounter++;
                    
                /* Logging error message in debugging mode */
                IX_CRYPTO_ACC_LOG(
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR,
                    "Q desc read from Crypto Done Q is NULL\n",
                    0, 0, 0, 0, 0, 0);
            } /* end of if-else (pQDesc) */
        }
        else /* Else for if (qStatus != IX_SUCCESS) */
        {
            if (IX_QMGR_Q_UNDERFLOW == qStatus )
            {
                /* 
                 * Underflow. Only error if this first time in loop
                 * Log with statistics
                 */
                if (0 == messageCount)
                {
                    ixCryptoStats.qUnderflowCounter++;
                }
            }
            else /* queue read failed */
            {
                /* Logging error message in debugging mode */
                IX_CRYPTO_ACC_LOG(
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR,
                    "Queue read failed with status %d.\n",
                    qStatus,
                    0, 0, 0, 0, 0);
            }
        } /* End of if (qStatus != IX_SUCCESS) */        
    } while (IX_SUCCESS == qStatus);

} /* end of ixCryptoQAccessReqDoneQMCallback () function */


/**
 * @fn      ixCryptoQAccessQueueWrite
 * @brief   Write queue entry to hardware queue through QMgr
 * @note    There is no corresponding Read function, as no statistic
 *          collection for reads (inderflow is common occurance using
 *          mechanism used in callback).
 *
 */
IxCryptoAccStatus
ixCryptoQAccessQueueWrite (
    IxQMgrQId queueId,
    UINT32 *pQEntry)
{
    if (IX_SUCCESS == ixQMgrQWrite (queueId, pQEntry))
    {
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* queue overflow */
    {
        if (IX_CRYPTO_ACC_CRYPTO_REQ_Q == queueId)
        {
            /* Crypto Req Q overflow, increase the counter */
            ixCryptoStats.qOverflowCounter++;
        }
        else
        {
            /* WEP Req Q overflow, increase the counter */
            ixCryptoStats.wepNpeQOverflowCounter++;
        }
        return IX_CRYPTO_ACC_STATUS_QUEUE_FULL;
    } /* end of if-else (ixQMgrWrite) */
} /* end of ixCryptoQAccessQueueWrite () function */



/**
 * @fn      ixCryptoQAccessQueueStatusShow
 * @brief   Show queue status
 *
 */
IxCryptoAccStatus
ixCryptoQAccessQueueStatusShow (IxQMgrQId queueId)
{
    if (IX_SUCCESS == ixQMgrQShow (queueId))
    {
        printf ("\n\nNumber of successful crypto request          : %d \n\n", 
            ixCryptoStats.cryptoSuccessCounter);
                
        printf ("Number of unsuccessful crypto request        : %d \n\n", 
            ixCryptoStats.cryptoFailCounter);
        
        printf ("Number of successful NPE WEP request         : %d \n\n", 
            ixCryptoStats.wepNpeSuccessCounter);  
 
        printf ("Number of unsuccessful NPE WEP request       : %d \n\n", 
            ixCryptoStats.wepNpeFailCounter);
            
        printf ("Number of successful XScale WEP request      : %d \n\n", 
            ixCryptoStats.wepXScaleSuccessCounter);  
 
        printf ("Number of unsuccessful XScale WEP request    : %d \n\n", 
            ixCryptoStats.wepXScaleFailCounter);
                   
        switch (queueId)
        {
            case IX_CRYPTO_ACC_CRYPTO_REQ_Q:
                printf ("Number of times Crypto Req Q overflow        : %d \n\n", 
                    ixCryptoStats.qOverflowCounter);
                break;
        
            case IX_CRYPTO_ACC_CRYPTO_DONE_Q:
                printf ("Number of times Crypto Done Q underflow      : %d \n\n", 
                    ixCryptoStats.qUnderflowCounter);
                printf ("Number of times Q descriptor received is NULL: %d \n\n", 
                    ixCryptoStats.qDescAddrInvalidCounter);
                break;
                
            case IX_CRYPTO_ACC_WEP_REQ_Q:
                printf ("Number of times WEP Req Q overflow           : %d \n\n", 
                    ixCryptoStats.wepNpeQOverflowCounter);          
                break;
                
            case IX_CRYPTO_ACC_WEP_DONE_Q:
                printf ("Number of times WEP Done Q underflow         : %d \n\n", 
                    ixCryptoStats.wepNpeQUnderflowCounter);
                    
                printf ("Number of times Q descriptor received is NULL: %d \n\n", 
                    ixCryptoStats.wepNpeQDescAddrInvalidCounter);    
                break;
                
            default:
                printf ("Invalid queue ID for cryptoAcc software component.\n");
                break;        
        } /* end of switch case (queueId) */
        
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* Q not configured */
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (ixQMgrShow) */
} /* end of ixCryptoQAccessQueueStatusShow () function*/


/**
 * @fn ixCryptoQAccessWepReqDoneQMgrCallback
 * @brief Notification callback registered with QManager for WEP service. Once  
 *        the threshold of the interrupt source is reached, the notification
 *        callback is triggered and is executed.
 *
 */
void 
ixCryptoQAccessWepReqDoneQMgrCallback (
    IxQMgrQId queueId, 
    IxQMgrCallbackId callbackId)
{
    UINT32 messageCount = 0;
    IxCryptoQDescriptor *pQDesc = NULL;
    IX_OSAL_MBUF *pSrcMbuf;
    IX_OSAL_MBUF *pDestMbuf;
    UINT32 operationResult;
    UINT32 cryptoCtxId;
    IX_STATUS qStatus;
    
    do
    {
        /* Read queue entry from hardware queue */
        qStatus = ixQMgrQRead (queueId, &operationResult);
        
        /* Check if the queue read operation pass */
        if (IX_SUCCESS == qStatus)
        {
            /* Count how many messages got this callback */
            messageCount++;
    
            /* Reset src mbuf and dest mbuf pointers to NULL */
            pSrcMbuf = NULL;
            pDestMbuf = NULL;
            
            pQDesc = (IxCryptoQDescriptor *)
                         (operationResult & IX_CRYPTO_Q_DESC_ADDR_MASK);
    
            /* Convert Q descriptor pointer from physical address to virtual
             * address
             */
            pQDesc = (IxCryptoQDescriptor *) 
                         IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION(
                         (UINT32) pQDesc);
                         
            if (NULL != pQDesc) /* if pQDesc not NULL */
            {       
                /* Invalidate Q descriptor message in cache */
                IX_CRYPTO_DATA_CACHE_INVALIDATE (
                    pQDesc, 
                    sizeof (IxCryptoQDescriptor));
            
                cryptoCtxId = pQDesc->cryptoCtxId; /* Get Crypto Context ID */
        
                /* Check if the source mbuf is NULL */
                if (NULL != pQDesc->pSrcMbuf)
                {
                    /* Convert mbuf header and its contents from NPE format
                     * to host format
                     */
                    pSrcMbuf = ixCryptoUtilMbufFromNpeFormatConvert (
                                   pQDesc->pSrcMbuf);
                } /* end of if (pQDesc->pSrcMbuf) */
            
                /* Check if the destination mbuf is NULL */ 
                if (NULL != pQDesc->pDestMbuf)
                {
                    /* Convert mbuf header and its contents from NPE 
                     * format to host format, shift mdata pointer back to
                     * original location
                     */
                    pDestMbuf = ixCryptoUtilMbufFromNpeFormatConvert (
                                    pQDesc->pDestMbuf);
                } /* end of if (pQDesc->pDestMbuf) */
                        
                /* Check Authentication status */
                if (IX_CRYPTO_NPE_OP_AUTH_FAIL 
                    != (operationResult & IX_CRYPTO_NPE_OP_STATUS_MASK)) 
                {                
                    /* Increment counter for number of requests completed
                     * successfully without error 
                     */
                    ixCryptoStats.wepNpeSuccessCounter++;

                    /* WEP Perform operation completed successfully.
                     * Call the perform complete callback function
                     */
                    ixCryptoCtx[cryptoCtxId].performCallbackFn(
                        cryptoCtxId,
                        pSrcMbuf, 
                        pDestMbuf,
                        IX_CRYPTO_ACC_STATUS_SUCCESS);
                }
                else /* Authentication operation failed */
                {                        
                    /* Increment counter for number of requests completed 
                     * but operation failed 
                     */
                    ixCryptoStats.wepNpeFailCounter++;
                        
                    /* Logging error message in debugging mode */
                    IX_CRYPTO_ACC_LOG(
                        IX_OSAL_LOG_LVL_MESSAGE,
                        IX_OSAL_LOG_DEV_STDOUT, 
                        "WEP Authentication operation failed.\n",
                        0, 0, 0, 0, 0, 0);
                            
                    /* WEP Perform operation, call the perform 
                     * complete callback function with status
                     * IX_CRYPTO_ACC_STATUS_AUTH_FAIL
                     */
                    ixCryptoCtx[cryptoCtxId].performCallbackFn(
                        cryptoCtxId,
                        pSrcMbuf, 
                        pDestMbuf,
                        IX_CRYPTO_ACC_STATUS_AUTH_FAIL);
                                  
                } /* end of if-else (AuthStatus) */
                    
                /* Release Q descriptor back to descriptor pool */
                if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
                    ixCryptoDescMgmtQDescriptorRelease (pQDesc))
                {
                    /* Log error message in debugging mode */
                    IX_CRYPTO_ACC_LOG(
                        IX_OSAL_LOG_LVL_ERROR,
                        IX_OSAL_LOG_DEV_STDERR,
                        "Q descriptor release failed for Crypto Ctx %d.\n",
                        cryptoCtxId,
                        0, 0, 0, 0, 0);
                }
                        
                /* Increment counter in the Crypto Context to keep track the 
                 * number of requests have been completed, log the number even 
                 * if operation failed                 
                 */
                ixCryptoCtx[cryptoCtxId].reqDoneCount++;
            }
            else /* if pQDesc is NULL */
            {
                /* Increment counter for number of Q descriptor address received
                 * is NULL
                 */
                ixCryptoStats.wepNpeQDescAddrInvalidCounter++;
                    
                /* Logging error message in debugging mode */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR, 
                    "Q desc read from WEP Done Q is NULL\n",
                    0, 0, 0, 0, 0, 0);
            } /* end of if-else (pQDesc) */
        }
        else /* Else for if (qStatus != IX_SUCCESS) */
        {
            if (IX_QMGR_Q_UNDERFLOW == qStatus )
            {
                /* 
                 * Underflow. Only error if this first time in loop
                 * Log with statistics
                 */
                if (0 == messageCount)
                {
                    ixCryptoStats.wepNpeQUnderflowCounter++;
                }
            }
            else /* queue read failed */
            {
                /* Logging error message in debugging mode */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR, 
                    "WEP Queue read failed with status %d.\n",
                    qStatus,
                    0, 0, 0, 0, 0);
            }
        } /* End of if (qStatus != IX_SUCCESS) */        
    } while (IX_SUCCESS == qStatus);   
} /* end of function ixCryptoQAccessWepReqDoneQMgrCallback () */
