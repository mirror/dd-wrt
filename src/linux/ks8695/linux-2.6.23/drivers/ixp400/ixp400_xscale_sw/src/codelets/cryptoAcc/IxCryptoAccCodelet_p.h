/**
 * @file IxCryptoAccCodelet_p.h
 *
 * @date April-20-2005
 *
 * @brief This is private header file for the Crypto Access Component Codelet
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


#ifndef IxCryptoAccCodelet_p_H
#define IxCryptoAccCodelet_p_H

#include "IxOsal.h" 
#include "IxNpeDl.h"
#include "IxCryptoAcc.h"

/** Global variables */
extern BOOL forwardRegisterCBCalled;    /**< callback flag */
extern BOOL reverseRegisterCBCalled;    /**< callback flag */
extern IxCryptoAccRegisterCompleteCallback registerCB; /**< register callback 
                                                        * function
                                                        */
extern IxCryptoAccPerformCompleteCallback performCB;   /**< perform callback 
                                                        * function
                                                        */

#define IX_CRYPTOACC_CODELET_MAX_STR_SIZE    128       /**< Maximum size of info
                                                        * string. 
                                                        */

#define IX_CRYPTO_ACC_CODELET_MAX_OPR_LEN    64        /** < Maximum operand len
                                                        * in words.
                                                        */        
                                                        
/**
 * @bried Enum specifying with pke operation to perform.
 *
 */
typedef enum {
    IX_CRYPTO_ACC_CODELET_PKE_RNG,
    IX_CRYPTO_ACC_CODELET_PKE_SHA,
    IX_CRYPTO_ACC_CODELET_PKE_EAU_MOD_EXP,
    IX_CRYPTO_ACC_CODELET_PKE_EAU_BN_MOD,
    IX_CRYPTO_ACC_CODELET_PKE_EAU_BN_ADD,
    IX_CRYPTO_ACC_CODELET_PKE_EAU_BN_SUB,
    IX_CRYPTO_ACC_CODELET_PKE_EAU_BN_MUL
} IxCryptoAccPkeIndex;
 
/**
 * @brief  Structure holding various configuration parameters for 
 *         demonstration purposes.
 * 
 * This structure is only defined and used by this codelet. It holds
 * various parameters that are required to register a context and invoke
 * "perform" requests on the cryptoAcc component.
 */ 
typedef struct {

    IxCryptoAccOperation  frwdOperation ; /**< Opcode for forward operation */
    IxCryptoAccOperation  revOperation ;  /**< Opcode for reverse operation */
    IxCryptoAccCipherAlgo cipherAlgo;     /**< Cipher Algorithm */
    IxCryptoAccCipherMode cipherMode;     /**< Cipher Mode */ 
    UINT32 cipherKeyLen;                  /**< Cipher Key Length */
    UINT32 cipherBlockLen;		          /**< Cipher Algorithm's Block Length */
    UINT32 cipherInitialVectorLen;        /**< Initial vector length */
    IxCryptoAccAuthAlgo  authAlgo;        /**< Authentication Algorithm */   
    UINT32 authKeyLen;                    /**< Authentication Key Length */
    UINT32 authDigestLen;                 /**< Authentication Digest Length */
    UINT32 aadLen;                        /**< Additional Authentication Data
                                           *  Length (used in CCM mode of
                                           *  operation).
                                           */
    BOOL useDifferentSrcAndDestMbufs;     /**< Use different source and
                                           *  destination buffers.
	                                       */
    BOOL invokeXScaleWepPerform;          /**< Perform this request using
                                           * XScaleWepPerform function
                                           * (note only ARC4 and WEP CRC
                                           * requests are valid .
                                           */
    UINT8 infoString[IX_CRYPTOACC_CODELET_MAX_STR_SIZE]; 
                                           /**< Information about the type of
                                            * request.
                                            */
}IxCryptoCodeletServiceParam;


/**************************************************************************
 *                           Function Prototype 
 **************************************************************************/

/**
 * @fn  ixCryptoAccCodeletInit (IxCryptoAccCfg ixCryptoAccCodeletCfg)
 * @brief This function initialises the neccessary components.
 *
 * @note The initialisation sequence is as follows:
 * <UL>
 *  <LI> Initialise Network Processor Engine(s) (NPE)
 *  <LI> Initialise Queue manager
 *  <LI> Start NPE(s)
 *  <LI> Initialise Crypto Access component
 *  <LI> Start Queue manager dispatcher
 *  <LI> Allocate mbuf pool
 * </UL>  
 *
 * @param "IxCryptoAccCfg ixCryptoAccCodeletCfg" - Specifies the interfaces
 *        to the engines that needs to be initialized.
 *
 * @return @li IX_SUCCESS - Successful initialization.
 *         @li IX_FAIL - Initialization failed.
 */
PRIVATE IX_STATUS ixCryptoAccCodeletInit (IxCryptoAccCfg ixCryptoAccCodeletCfg);

/**
 * @fn  ixCryptoAccCodeletUninit (void)
 * @brief This function uninitialize the crypto codelets 
 *
 * @note Uninitialize the crypto by stopping the qmgr dispatcher, uninitializing
 *       the cryptoAcc, unloading qmgr, and releasing allocated mbuf.
 * @note The uninitialisation sequence is as follows:
 * <UL>
 *  <LI> Initialise Crypto Access component
 *  <LI> Free resources
 *  <LI> Stop Queue manager dispatcher
 *  <LI> Unload Queue manager
 *
 * @param None
 *
 * @return @li IX_SUCCESS - Successful Uninitialization.
 *         @li IX_FAIL - Uninitialization failed.
 *
 */
 PRIVATE IX_STATUS ixCryptoAccCodeletUninit (void);

/**
 * @def IX_CRYPTO_ACC_CODELET_DRV_DMA_MALLOC
 *
 * @brief To allocate the memory
 *
 * @note Example of usage: IX_CRYPTO_ACC_CODELET_DRV_DMA_MALLOC(size)
 *
 * @param "UINT32 [in] size" - size of memory block to be freed.
 *
 * @return None
 *
 */
#define IX_CRYPTO_ACC_CODELET_DRV_DMA_MALLOC(size) \
            IX_OSAL_CACHE_DMA_MALLOC(size)

/**
 * @def IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE
 *
 * @brief To free the memory allocated through IX_CRYPTO_ACC_DRV_DMA_MALLOC
 *        function.
 *
 * @note Example of usage: IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE(pData, size)
 *
 * @param "UINT32 [in] *pData" - memory block pointer to be freed.
 *
 * @return None
 *
 */
#define IX_CRYPTO_ACC_CODELET_DRV_DMA_FREE(pData) \
            IX_OSAL_CACHE_DMA_FREE(pData)


/**
 * @fn ixCryptoAccCodeletBufIsQEmpty (void)
 *
 * @brief This inline function is used to check whether the buffer
 *        queue is empty.
 *
 * @param None
 *
 * @return BOOL
 *             - TRUE - the queue is full.
 *             - FALSE - the queue is not full.
 */
INLINE BOOL 
ixCryptoAccCodeletBufIsQEmpty (void);


/**
 * @fn ixCryptoAccCodeletBufIsQFull(void)
 *
 * @brief This inline function is used to check whether the buffer
 *        queue is full.
 *
 * @param None
 *
 * @return BOOL
 *             - TRUE - the queue is full.
 *             - FALSE - the queue is not full.
 */
INLINE BOOL 
ixCryptoAccCodeletBufIsQFull (void);


/**
 * @fn ixCryptoAccCodeletCryptoMbufToQTailAdd (IX_OSAL_MBUF *pMbufToAdd)
 *
 * @brief This function adds a mbuf to the tail of the queue.  
 *
 * @param "IX_OSAL_MBUF* [in] pMbufToAdd" - Pointer of the mbuf to be added  
 *                    to the tail of the queue.
 *
 * @return None.
 */
PRIVATE void ixCryptoAccCodeletCryptoMbufToQTailAdd (IX_OSAL_MBUF *pMbufToAdd);


/**
 * @fn ixCryptoAccCodeletCryptoMbufFromQHeadRemove(IX_OSAL_MBUF **pMbufToRem)
 *
 * @brief This function removes a mbuf from the head of the queue.
 *
 * @param "IX_OSAL_MBUF** [in] pMbufToRem" - Pointer to a mbuf pointer to be
 *                     removed from the head of the queue.
 *
 * @return None.
 */
PRIVATE void 
ixCryptoAccCodeletCryptoMbufFromQHeadRemove (IX_OSAL_MBUF **pMbufToRem);


/**
 * @fn ixCryptoAccCodeletCryptoMemPoolInit(UINT32 pktLen)
 *
 * @brief This function initialises mbuf pool for the Crypto
 *        Access Codelet.
 *
 * @param "UINT32 [in] pktLen" - packet length size for mbuf
 *
 * @return @li IX_SUCCESS - Successful mbuf pool initialisation.
 *         @li IX_FAIL - Mbuf pool initialisation failed.
 */
PRIVATE IX_STATUS ixCryptoAccCodeletCryptoMemPoolInit (UINT32 pktLen);


/**
 * @fn IX_STATUS ixCryptoAccCodeletCryptoMemPoolFree (UINT32 pktLen)
 *
 * @brief Free all resources in Crypto Access Codelet MBUF pool
 *
 * @param "UINT32 [in] pktLen" - packet length size for mbuf
 *
 * @return None
 */
PRIVATE void ixCryptoAccCodeletCryptoMemPoolFree (UINT32 pktLen);

/**
 * @fn ixCryptoAccCodeletDispatcherPoll(void)
 *
 * @brief This function polls the queues when poll mode set in the 
 *        function ixCryptoAccCodeletDispatcherStart.
 *
 * @param None
 * 
 * @return None
 */
PRIVATE void ixCryptoAccCodeletDispatcherPoll (void);


/**
 * @fn ixCryptoAccCodeletDispatcherStart(BOOL useInterrupt)
 *
 * @brief  This function starts the Queue manager dispatcher.
 *
 * @param "BOOL [in] useInterrupt" - start in interrupt or poll mode. TRUE is 
 *                                   interrupt mode while FALSE is poll mode.
 * 
 * @return  @li IX_SUCCESS - Dispatcher successfully started.
 *          @li IX_FAIL - Error starting dispatcher.
 */
PRIVATE IX_STATUS ixCryptoAccCodeletDispatcherStart (BOOL useInterrupt);


/**
 * @fn ixCryptoAccCodeletRegisterCtx(
 *         INT32 srvIndex,
 *         IxCryptoAccCtx *pForwardCtx,
 *         IxCryptoAccCtx *pReverseCtx,
 *         UINT32 *pForwardCtxId,
 *         UINT32 *pReverseCtxId)
 *
 * @brief  This function registers forward and reverse context for the 
 *         specified service index.
 *
 * @param "INT32 [in] srvIndex" - Index into the array defining various 
 *                                configured services.
 * @param "IxCryptoAccCtx* [in] pForwardCtx" - pointer to Crypto Context for
 *                                            forward direction, e.g encryption,
 *                                            auth_calc.
 * @param "IxCryptoAccCtx* [in] pReverseCtx" - pointer to Crypto Context for 
 *                                            reverse direction, e.g decryption,
 *                                            auth_check.
 * @param "UINT32* [in] pForwardCtxId" - pointer to Crypto Context ID
 * @param "UINT32* [in] pReverseCtxId" - pointer to Crypto Context ID
 * 
 * @return @li IX_SUCCESS - Register successfully
 *         @li IX_FAIL - Error in registering
 */
PRIVATE IX_STATUS ixCryptoAccCodeletRegisterCtx (
            INT32  srvIndex,
            IxCryptoAccCtx *pForwardCtx,
            IxCryptoAccCtx *pReverseCtx,
            UINT32 *pForwardCtxId,
            UINT32 *pReverseCtxId);


/**
 * @fn ixCryptoAccCodeletPeform(UINT32 pktLen)
 *
 *
 * @brief This function invokes the "appropriate" perform function
 *        of the cryptoAcc component. XScaleWepPerform function is invoked
 *        if the invokeXScaleWepPerform flag is set for the service index.
 *        This function uses the global variable "codeletSrvIndex", 
 *        set by the codeletMain  to access the requested service's parameters.
 *
 * @param  "UINT32 pktLen  [in] pktLen"   - Packet length.
 * @return @li IX_SUCCESS - If the operation invocation was successful. 
 *         @li IX_FAIL    - If operation failed for some reason.
 */
PRIVATE IX_STATUS ixCryptoAccCodeletPerform (UINT32 pktLen);



/**
 * @fn ixCryptoAccCodeletUnregisterCtx (void)
 *
 * @brief  This function unregisters all the contexts registered.
 *
 * @param None
 * 
 * @return None
 */
PRIVATE void ixCryptoAccCodeletUnregisterCtx (void);


/**
 * @fn ixCryptoAccCodeletXScaleWepPerform ( IX_OSAL_MBUF *pMbuf ,UINT32 pktLen)
 * @brief This codelet function calls ixCryptoAccXScaleWepPerform function
 *        to perform ARC4 or WEP_CRC processing on XScale.
 * @param  IX_OSAL_MBUF [in] *pMbuf : pointer to the mbuf 
 * 
 * @param  UINT32 [in] pktLen: length of data to be worked on.
 * @return  @li IX_SUCCESS - All invocations of ixCryptoAccXScaleWepPerform
 *                           were succesful.
 *          @li IX_FAIL - There was some kind of error while processing the
 *                        request. This should not happen, if the codelet
 *                        has been configured correctly.
 *                        
 */
PRIVATE IX_STATUS ixCryptoAccCodeletXScaleWepPerform( 
                    IX_OSAL_MBUF *pMbuf,
                    UINT32 pktLen);

/**
 * @fn ixCryptoAccCodeletDispatcherStop (BOOL useInterrupt)
 *
 * @brief Stop QMgr dispatcher thread if QMgr dispatcher runs in poll mode 
 *        or unbind QMgr dispatcher from interrupt if it runs in interrupt mode.
 *
 * @param "BOOL [in] useInterrupt" - start in interrupt or poll mode. TRUE is 
 *                                   interrupt mode while FALSE is poll mode.
 * 
 * @return  @li IX_SUCCESS - Dispatcher successfully stopped.
 *          @li IX_FAIL - Error stopping dispatcher.
 */
PRIVATE IX_STATUS ixCryptoAccCodeletDispatcherStop (BOOL useInterrupt);


/**
 * @fn ixCryptoAccCodeletUsageShow (void)
 *
 * @brief Display user guide for Linux* platform or VxWorks* platform
 *
 * @param None
 *
 * @return None
 */
PRIVATE void ixCryptoAccCodeletUsageShow (void);

/**
 * @fn ixCryptoAccCodeletPerformMain (UINT32 packetLen)
 *
 * @brief Invoke cryptographic operations
 *
 * @param UINT32 [in] packetLen: packet length specified by client.
 * @return  @li IX_SUCCESS - All invocations of operations were succesful.
 *          @li IX_FAIL - There was some kind of error while processing the
 *                        request. This should not happen, if the codelet
 *                        has been configured correctly.
 */
PRIVATE IX_STATUS ixCryptoAccCodeletPerformMain (UINT32 packetLen);

/** 
 * @fn ixCryptoAccCodeletRegisterCB(
 *         UINT32 cryptoCtxId,
 *         IX_OSAL_MBUF *pMbuf, 
 *         IxCryptoAccStatus status)
 *
 * @brief  Cryptographic Context registration callback function to be called
 *         in QMgr Dispatcher context when the registration completed.
 *
 * @param "UINT32 [in] cryptoCtxId" - Crypto Context ID
 * @param "IX_OSAL_MBUF* [in] pMbuf" - mbuf pointer
 * @param "IxCryptoAccStatus [in] status" - status of callback
 *
 * @return None
 */
PRIVATE void ixCryptoAccCodeletRegisterCB (
                UINT32 cryptoCtxId,
                IX_OSAL_MBUF *pMbuf, 
                IxCryptoAccStatus status);

/**
 * @fn ixCryptoAccCodeletPerformCB (
 *        UINT32 cryptoCtxId,
 *        IX_OSAL_MBUF *pSrcMbuf, 
 *        IX_OSAL_MBUF *pDestMbuf,
 *        IxCryptoAccStatus status)
 *
 * @brief  Crypto services request callback function to be called when the 
 *         crypto service request completed. Performance rate is calculated 
 *         in this function.
 *
 * @param "UINT32 [in] cryptoCtxId" - Crypto Context ID
 * @param "IX_OSAL_MBUF* [in] pSrcMbuf" - source mbuf pointer
 * @param "IX_OSAL_MBUF* [in] pDestMbuf" - destination mbuf pointer
 * @param "IxCryptoAccStatus [in] status" - status of callback
 *
 * @return None
 */
PRIVATE void ixCryptoAccCodeletPerformCB (
                UINT32 cryptoCtxId,
                IX_OSAL_MBUF *pSrcMbuf, 
                IX_OSAL_MBUF *pDestMbuf,
                IxCryptoAccStatus status);

#ifdef __ixp46X             /* PKE codes only applicable to ixp46x platform */

/**
 * @fn ixCryptoAccCodeletPkeHashPerformCB (
 *          UINT8 *pDigestCB,
 *          IxCryptoAccStatus status)
 *
 * @brief  Crypto Hash perform callback function to be called when the 
 *         operation completed. Performance rate is calculated in this function.
 *
 * @param "UINT8 [in] *pDigestCB" - digest from hashing result
 * @param "IxCryptoAccStatus [in] status" - status of callback
 *
 * @return None
 */
PRIVATE void ixCryptoAccCodeletPkeHashPerformCB (
            UINT8 *pDigestCB, 
            IxCryptoAccStatus status);
/**
 * @fn ixCryptoAccCodeletPkeEauPerformCB (
 *          IxCryptoAccPkeEauOperation operation,
 *          IxCryptoAccPkeEauOpResult *pResultCB,
 *          BOOL carryOrBorrow,
 *          IxCryptoAccStatus status)
 *
 * @brief  Crypto EAU perform callback function to be called when the 
 *         operation completed. Performance rate is calculated in this function.
 *
 * @param "IxCryptoAccPkeEauOperation [in] operation" - EAU arithmetic operation
 * @param "IxCryptoAccPkeEauOpResult [in] *pResultCB" - result pointer
 * @param "BOOL [in] carryOrBorrow" - carry or borrow check
 * @param "IxCryptoAccStatus [in] status" - status of callback
 *
 * @return None
 */
PRIVATE void ixCryptoAccCodeletPkeEauPerformCB (
            IxCryptoAccPkeEauOperation operation,
            IxCryptoAccPkeEauOpResult *pResultCB,
            BOOL carryOrBorrow,
            IxCryptoAccStatus status);  
            
/**
 * @fn ixCryptoAccCodeletPkePerformMain (IxCryptoAccPkeIndex opIndex, UINT32 len)
 *
 * @brief Invoke PKE operations
 *
 * @param UINT32 [in] opIndex: index of an operation.
 * @param UINT32 [in] len: data/operand length sepcified by client
 * @return  @li IX_SUCCESS - All invocations of pke operations were successful. 
 *          @li IX_FAIL - There was some kind of error while processing the 
 *              request. This should not happen, if the codelet has been 
 *              configured correctly.
 */
PRIVATE IX_STATUS 
ixCryptoAccCodeletPkePerformMain (IxCryptoAccPkeIndex opIndex, UINT32 len);

/**
 * @fn ixCryptoAccCodeletPkeEauPerform (IxCryptoAccPkeEauOperation operation)
 *
 * @brief Perform EAU operations
 *
 * @param IxCryptoAccPkeEauOperation [in] operation: eau operation.
 * @return @li  IX_SUCCESS - EAU operation start successfully            
 * @return @li  IX_FAIL - EAU operation request failed.
 *
 */
PRIVATE IX_STATUS 
ixCryptoAccCodeletPkeEauPerform (IxCryptoAccPkeEauOperation operation);


/**
 * @fn ixCryptoAccCodeletPkeRngPerform (void)
 *
 * @brief Get pseudo-random number operation. Each window output is 1000 times,
 *        and has 20 windows. Thus, 20000 operations are performed and time is
 *        calculated for each window (1000 operations).
 *
 * @param none
 * @return @li  IX_SUCCESS - Pseudo random number get operation complete 
 *              successfully.
 * @return @li  IX_FAIL - There was some kind of error while processing the
 *              request. This should not happen, if the codelet has been 
 *              configured correctly.
 */
PRIVATE IX_STATUS ixCryptoAccCodeletPkeRngPerform (void);
#endif 

#endif /* IxCryptoAccCodelet_p_H */


