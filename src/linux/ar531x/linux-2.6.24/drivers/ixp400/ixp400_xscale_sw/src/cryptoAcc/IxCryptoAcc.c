/**
 * @file IxCryptoAcc.c
 *
 * @date July-13-2005
 *
 * @brief  Source file for Public API (Component Interface module)
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
* Put the user defined include files required.
*/
#include "IxOsal.h"
#include "IxCryptoAcc.h"
#include "IxFeatureCtrl.h"
#include "IxCryptoAcc_p.h"
#include "IxCryptoAccCCDMgmt_p.h"
#include "IxCryptoAccDescMgmt_p.h"
#include "IxCryptoAccQAccess_p.h"
#include "IxCryptoAccCryptoRegister_p.h"
#include "IxCryptoAccCryptoPerform_p.h"
#include "IxCryptoAccUtilities_p.h"
#include "IxCryptoAccPkeCommon_p.h"
#include "IxCryptoAccPkeHash_p.h"
#include "IxCryptoAccPkeEau_p.h"    


#define IX_CRYPTO_ACC_512B_ALIGNED_ADDR_MASK 0xfffffe00 /**< Address mask to get
                                                         * 512-byte aligned 
                                                         * memory address
                                                         */
                                                        
#define IX_CRYPTO_ACC_32B_ALIGNED_ADDR_MASK  0xffffffe0 /**< Address mask to get
                                                         * 32-byte aligned 
                                                         * memory address
                                                         */

#define IX_CRYPTO_MAX_SBOX_INFO_BYTES   1024 /**< Maximum number of bytes 
                                              * needed for Sbox, must be
                                              * 512-byte aligned. Sbox is
                                              * just 256 bytes only.
                                              */

#define IX_CRYPTO_MAX_ARC4_KEY_BYTES      64 /**< Maximum number of bytes 
                                              * needed for ARC4 key, must be
                                              * 32-byte aligned. ARC key is
                                              * just 16 bytes only.
                                              */

#define IX_CRYPTO_SBOX_BLOCK_BYTES       512 /**< Sbox block size with 
                                              * 512-byte aligned.
                                              */

#define IX_CRYPTO_ARC4_KEY_BLOCK_BYTES    32 /**< ARC4 key block size with 
                                              * 32-byte aligned.
                                              */

/*
* Variable declarations global to this file only.  Externs are followed by
* static variables.
*/
static BOOL ixCryptoAccServiceInit = FALSE; /**< TRUE if component 
                                             * initialized 
                                             */
static BOOL ixCryptoAccServiceStop = FALSE; /**< TRUE if crypto service 
                                             * is halt 
                                             */
static BOOL ixCryptoAccCryptoNpeInit = FALSE;  /**< TRUE if crypto hw  
                                                * acclerator initialized 
                                                */
static BOOL ixCryptoAccWepNpeInit = FALSE;     /**< TRUE if WEP component 
                                                * initialized 
                                                */

static BOOL ixCryptoAccCoProcessorExist[IX_CRYPTO_COPROCESSOR_TYPE] = 
                {FALSE, FALSE, FALSE};
                                           /**< TRUE if the coprocessor does 
                                            * exist (DES, AES, HASH)
                                            */    

#ifdef __ixp46X     /* PKE codes only applicable for IXP46X platform */                                                
static BOOL ixCryptoAccPkeCryptoEngineInit = FALSE; /**< TRUE if PKE component 
                                                     * initialized 
                                                     */
#endif
                                                                                         
static IxCryptoAccCfg componentConfig = IX_CRYPTO_ACC_CFG_CRYPTO_NPE_ACC_EN;
                                            /**< Default component config,
                                             * crypto hw accelerator is 
                                             * enabled
                                             */
#ifndef NDEBUG
static INT32 xscaleStatLock;       /**< Protect critical sections in this 
                                    * module
                                    */
#endif

/**
 * @def IX_CRYPTO_IS_SERVICE_INITIALIZED
 * @brief Check if component has been initialized
 *
 * return TRUE / FALSE
 *
 * @note This is to check if either one of the services (crypto hw 
 *       accelerator and WEP) has been initialized.
 */
#define IX_CRYPTO_IS_SERVICE_INITIALIZED() (TRUE == ixCryptoAccServiceInit)



/**
 * @def IX_CRYPTO_IS_SERVICE_STOPPED
 * @brief Check if component service has been stopped
 *
 * return TRUE / FALSE
 *
 * @note This is to check if cryptoAcc access compoenent service has been
 *       stopped (both crypto hw accelerator and WEP component.
 */
#define IX_CRYPTO_IS_SERVICE_STOPPED() (TRUE == ixCryptoAccServiceStop)



/**
 * @def IX_CRYPTO_IS_HW_ACCL_INITIALIZED
 * @brief Check if hw accelerator component has been initialized
 *
 * return TRUE / FALSE
 *
 * @note Need to invoke this check in crypto hardware accelerator perform
 *       operation.
 */
#define IX_CRYPTO_IS_HW_ACCL_INITIALIZED() (TRUE == ixCryptoAccCryptoNpeInit)



/**
 * @def IX_CRYPTO_IS_WEP_NPE_INITIALIZED
 * @brief Check if WEP NPE component has been initialized
 *
 * return TRUE / FALSE
 *
 * @note Need to invoke this check in WEP perform operation request.
 */
#define IX_CRYPTO_IS_WEP_NPE_INITIALIZED() (TRUE == ixCryptoAccWepNpeInit)


/**
 * @def IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED
 * @brief Check if PKE crypto engine has been initialized
 *
 * return TRUE / FALSE
 *
 * @note Need to invoke this check in PKE crypto perform operation.
 */
#define IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED() \
    (TRUE == ixCryptoAccPkeCryptoEngineInit)
    
    
/**
 * @fn IxCryptoAccStatus ixCryptoAccConfig (IxCryptoAccCfg compCfg)
 * @brief Selects which interfaces need to be initialized when crypto-access  
 *        init is called. 
 * 
 */
PUBLIC IxCryptoAccStatus
ixCryptoAccConfig (IxCryptoAccCfg compCfg)
{
    switch (compCfg)
    {
        /* All valid configuration fall through to 
         * IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN case statement */
         
        /* XScale WEP engine only */
        case IX_CRYPTO_ACC_CFG_WEP_XSCALE_ACC_EN:
        
        /* Crypto NPE engine and XSCale WEP engine*/
        case IX_CRYPTO_ACC_CFG_CRYPTO_NPE_ACC_EN: 
                
        /* NPE WEP engine and XSCale WEP engine */
        case IX_CRYPTO_ACC_CFG_WEP_NPE_ACC_EN:
                
        /* CRYPTO NPE, NPE WEP engine and XSCale WEP engine */
        case IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN:
            componentConfig = compCfg;
            return IX_CRYPTO_ACC_STATUS_SUCCESS;
        
        /* Invalid configuration */
        default:
            return IX_CRYPTO_ACC_STATUS_FAIL;            
    }    
} /* end of ixCryptoAccConfig () function */


/**
 * @fn ixCryptoAccInit
 * @brief Initialise the Security Access component
 *
 */
PUBLIC IxCryptoAccStatus
ixCryptoAccInit (void)
{    
    /* Assume DES/AES/HASH coprocessors does not exist */
    ixCryptoAccCoProcessorExist[IX_CRYPTO_DES_COPROCESSOR] = 
    ixCryptoAccCoProcessorExist[IX_CRYPTO_AES_COPROCESSOR] =
    ixCryptoAccCoProcessorExist[IX_CRYPTO_HASH_COPROCESSOR] = FALSE;
        
    /* If crypto hardware accelerator is enabled, check for existence of
     * NPE C, DES, AES and HASH coprocessors
     */       
    if (IX_CRYPTO_ACC_CFG_CRYPTO_NPE_ACC_EN == componentConfig
        || IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN == componentConfig)
    {
        if (IX_FEATURE_CTRL_COMPONENT_DISABLED == 
            ixFeatureCtrlComponentCheck (IX_FEATURECTRL_NPEC))
        {               
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "The NPE C coprocessor does not exist. The configurations"
                " of IX_CRYPTO_ACC_CFG_CRYPTO_NPE_ACC_EN or"
                " IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN is not allowed.\n",
                0,0,0,0,0,0);
             
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* else-if ixFeatureCtrlComponentCheck (IX_FEATURECTRL_NPEC) */
        else
        {
            if (IX_FEATURE_CTRL_COMPONENT_DISABLED == 
                ixFeatureCtrlComponentCheck (IX_FEATURECTRL_DES))
            {
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_WARNING,
                    IX_OSAL_LOG_DEV_STDOUT,
                    "Warning: The DES coprocessor does not exist\n",
                    0,0,0,0,0,0);
            }
            else
            {
                ixCryptoAccCoProcessorExist[IX_CRYPTO_DES_COPROCESSOR] = TRUE;
            }
 
            if ( IX_FEATURE_CTRL_COMPONENT_DISABLED ==
                 ixFeatureCtrlComponentCheck (IX_FEATURECTRL_HASH))
            {
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_WARNING,
                    IX_OSAL_LOG_DEV_STDOUT,
                    "Warning: The HASH coprocessor does not exist\n",
                    0,0,0,0,0,0);
            }
            else
            {
                ixCryptoAccCoProcessorExist[IX_CRYPTO_HASH_COPROCESSOR] = TRUE;
            }
        
            if (IX_FEATURE_CTRL_COMPONENT_DISABLED ==
                ixFeatureCtrlComponentCheck (IX_FEATURECTRL_AES))
            {
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_WARNING,
                    IX_OSAL_LOG_DEV_STDOUT,
                    "Warning: The AES coprocessor does not exist\n",
                    0,0,0,0,0,0);
            }
            else
            {
                ixCryptoAccCoProcessorExist[IX_CRYPTO_AES_COPROCESSOR] = TRUE;
            }
               
            /* Check if DES/HASH/AES coprocessors exist */
            if (!ixCryptoAccCoProcessorExist[IX_CRYPTO_DES_COPROCESSOR] &&
                !ixCryptoAccCoProcessorExist[IX_CRYPTO_HASH_COPROCESSOR] && 
                !ixCryptoAccCoProcessorExist[IX_CRYPTO_AES_COPROCESSOR])
            {
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR,
                    "All cryptographic coprocessor in NPE C does not exist." 
                    " The configurations of IX_CRYPTO_ACC_CFG_CRYPTO_NPE_ACC_EN"
                    " or IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN is not"
                    " allowed.\n",
                    0,0,0,0,0,0);
                  
                return IX_CRYPTO_ACC_STATUS_FAIL;
            }
                
        } /* end of if ixFeatureCtrlComponentCheck (IX_FEATURECTRL_NPEC) */
    } /* end of if (componentConfig) */
        
    /* If WEP NPE access is enabled, check for existence of AAL 
     * coprocessor and NPE A.
     */
    if (IX_CRYPTO_ACC_CFG_WEP_NPE_ACC_EN == componentConfig
        || IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN == componentConfig)
    {  
        if ((IX_FEATURE_CTRL_COMPONENT_DISABLED == 
            ixFeatureCtrlComponentCheck (IX_FEATURECTRL_NPEA)) ||
            (IX_FEATURE_CTRL_COMPONENT_DISABLED == 
                ixFeatureCtrlComponentCheck (IX_FEATURECTRL_AAL)))
        {                
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "The NPEA/AAL coprocessor does not exist and the"
                " configurations of IX_CRYPTO_ACC_CFG_WEP_NPE_ACC_EN or"
                " IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN is not allowed.\n",
                0,0,0,0,0,0);
              
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* else of if ixFeatureCtrlComponentCheck (IX_FEATURECTRL_NPEA) */
    } /* end of if (componentConfig) */
    
    /* Check if the access component has been intialized, if not, 
     * initialize Descriptor Mgmt, CCD Mgmt, Statistics, QAccess 
     * and PKE modules.
     */
    if (!IX_CRYPTO_IS_SERVICE_INITIALIZED ()
        && !IX_CRYPTO_IS_SERVICE_STOPPED ())
    {   
#ifdef __ixp46X     /* PKE codes only applicable for IXP46X platform */
            
        /* Initialize PKE Crypto Engine module if the device is IXP46X. 
         * IXP42X does not has PKE Crypto Engine. If the PKE Crypto
         * Engine is fused out, skip the initialization.
         */
        if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()
            && (IX_FEATURE_CTRL_COMPONENT_DISABLED !=
            ixFeatureCtrlComponentCheck (IX_FEATURECTRL_RSA)))
        {
            if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoPkeInit ())
            {
                /* Log error message in debugging mode */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR,
                    "ixCryptoPkeInit FAILED.\n", 
                    0, 0, 0, 0, 0, 0);
                    
               return IX_CRYPTO_ACC_STATUS_FAIL;     
            } /* end of if (ixCryptoPkeInit () */
           
            /* Set PKE Crypto Engine enabled flag if initialization of PKE
             * Crypto Engine complete successfully
             */
            ixCryptoAccPkeCryptoEngineInit = TRUE;
        }
        else /* PKE Crypto engine not exist */
        {
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_WARNING,
                IX_OSAL_LOG_DEV_STDOUT,
                "Warning: The PKE Crypto Engine does not exist!\n",
                0, 0, 0, 0, 0, 0);   
        } /* end of if-else ixFeatureCtrlDeviceRead () &&
           * ixFeatureCtrlComponentCheck ()
           */

#endif /* ixp46X */        

        /* Initializing Descriptor Management Module */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoDescMgmtInit ())
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "ixCryptoDescMgmtInit FAILED.\n", 
                0, 0, 0, 0, 0, 0);
             
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (ixCryptoDescMgmtInit) */
            
        /* Initializing Crypto Context Database Management Module */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoCCDMgmtInit ())
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "ixCryptoCCDMgmtInit FAILED.\n", 
                0, 0, 0, 0, 0, 0);
            
            /* Release memory pool allocated to Desc pool */
            ixCryptoDescMgmtDescPoolFree ();
                
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (ixCryptoCCDMgmtInit) */
            
        /* Initializing Queue Access Module */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS 
            != ixCryptoQAccessInit (componentConfig))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "ixCryptoQAccessInit FAILED.\n", 
                0, 0, 0, 0, 0, 0);
            
            /* Release memeory pool allocated to NPE Crypto Param */
            ixCryptoCCDMgmtNpeCryptoParamPoolFree ();
            
            /* Release memory pool allocated to Desc pool */
            ixCryptoDescMgmtDescPoolFree ();
                
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (ixCryptoQAccessInit) */
        
        /* Set crypto hw accelerator flag if initialization for crypto
         * hw accelerator complete successfully
         */
        if (IX_CRYPTO_ACC_CFG_CRYPTO_NPE_ACC_EN == componentConfig
            || IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN == componentConfig)
        {
               ixCryptoAccCryptoNpeInit = TRUE;
        }  
        
        /* Set WEP enabled flag if initialization of WEP NPE access
         * complete successfully
         */
        if (IX_CRYPTO_ACC_CFG_WEP_NPE_ACC_EN == componentConfig
            || IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN == componentConfig)
        {
            ixCryptoAccWepNpeInit = TRUE;
        }
        
        /* The initialisation is complete hence set the flag to TRUE */
        ixCryptoAccServiceInit = TRUE;
        
        /* Initialization successfully */
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* end of if (IX_CRYPTO_IS_SERVICE_INITIALIZED) &&
            (IX_CRYPTO_IS_SERVICE_STOPPED) */
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "IxCrypto is initialized or stopped before this \
            function call.\n", 
            0, 0, 0, 0, 0, 0);

        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (IX_CRYPTO_IS_SERVICE_INITIALIZED) &&
     (IX_CRYPTO_IS_SERVICE_STOPPED) */       
} /* end of ixCryptoAccInit () function */


/**
 * @fn ixCryptoAccUninit
 * @brief Uninitialise the Security Access component
 *
 */
PUBLIC IxCryptoAccStatus
ixCryptoAccUninit (void)
{
    UINT32 timer = 0;

    if (FALSE == ixCryptoAccServiceInit)
    {
        return IX_CRYPTO_ACC_STATUS_FAIL; /* Crypto is not initialised  */
    }
    
    ixCryptoAccServiceStop = TRUE;          /* Flag to decline NPE request */
    
#ifdef __ixp46X     /* PKE codes only applicable for IXP46X platform */

    ixCryptoAccPkeCryptoEngineInit = FALSE; /* Flag to decline PKE request */
    
    /* Uninit PKE Crypto Engine module */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoPkeUninit ())
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;  /* Unable to uninitialise the PKE */
    }

#endif /* ixp46X */

    while (IX_CRYPTO_ACC_STATUS_SUCCESS !=
           ixCryptoDescMgmtAllQDescriptorInPoolCheck())
    {
        /* Put the task into sleep mode and wait for the NPE to complete
         * all the tasks in the queue
         */
        ixOsalSleep(IX_CRYPTO_ACC_DELAY_IN_MS);

        /* Timer to avoid infinite lock */
        timer++;
        if (IX_CRYPTO_ACC_TIMER_COUNT < timer )
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_FATAL,
                IX_OSAL_LOG_DEV_STDERR,
                "Stop function failed!\n",
                0, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;

        } /* End of if timer loop */

    } /* end of while (ixCryptoDescMgmtAllQDescriptorInPoolCheck) */

    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoQAccessUninit ())
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;  /* Unable to uninitialise the Queues */
    }

    /* Release all Crypto Contexts and this will not fail */
    ixCryptoCCDMgmtCryptoCtxReleaseAll ();

    /* Release memeory pool allocated to NPE Crypto Param */
    ixCryptoCCDMgmtNpeCryptoParamPoolFree ();

    /* Release memory pool allocated to Desc pool */
    ixCryptoDescMgmtDescPoolFree ();
    
    /* Reset the ixCryptoAccServiceInit flag to FALSE */
    ixCryptoAccServiceInit = FALSE;

    /* Reset crypto hw accelerator flag   */
    ixCryptoAccCryptoNpeInit = FALSE;

    /* Reset WEP enabled flag */
    ixCryptoAccWepNpeInit = FALSE;
    
    /* Reset Service Stop flag */
    ixCryptoAccServiceStop = FALSE;
    
    /* UnInitialization successfully */
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
}

/**
 * @fn ixCryptoAccCtxRegister
 *
 * @brief Crypto context registration. Cryptographic Context ID (cryptoCtxId)
 *        for the registered crypto context obtained from this registration
 *        request will be used in ixCryptoAccAuthCryptPerform requests.
 *
 */
PUBLIC IxCryptoAccStatus 
ixCryptoAccCtxRegister (
    IxCryptoAccCtx *pAccCtx,
    IX_OSAL_MBUF *pMbufPrimaryChainVar,
    IX_OSAL_MBUF *pMbufSecondaryChainVar,
    IxCryptoAccRegisterCompleteCallback registerCallbackFn,
    IxCryptoAccPerformCompleteCallback performCallbackFn,
    UINT32 *pCryptoCtxId)
{
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_SUCCESS;
    UINT32 npeCryptoParamIndex = 0;
    BOOL callbackFlag = FALSE;

    /* Check if the client context and CryptoCtxId pointer are NULL, 
     * return error if either one is NULL 
     */
    if ((NULL == pAccCtx) || (NULL == pCryptoCtxId))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Client's Ctx is %x\n pCryptoCtxId is %x\n", 
            (int) pAccCtx, 
            (int) pCryptoCtxId,
            0, 0, 0, 0);

        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (pAccCtx) || (pCryptoCtxId) */
    
    /* Check if the callback functions provided by client is NULL, return
     * error if either one is NULL
     */
    if ((NULL == registerCallbackFn) || (NULL == performCallbackFn))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Register Callback: %x\nPerform Callback: %x\n", 
            (int) registerCallbackFn,
            (int) performCallbackFn, 
            0, 0, 0, 0);

        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (registerCallbackFn) || (performCallbackFn) */

    /* Check if crypto hw accelerator is enabled for DES/3DES/AES
     * cipher algorithm
     * Note: No check is needed for registration of ARC4 cipher algorithm,
     *       as WEP service request could be processed in two ways, either
     *       through XScale assembly or WAN-NPE. If WEP NPE is not 
     *       enabled, client can send in the request through XScale WEP
     *       perform service request.
     */
    switch (pAccCtx->cipherCtx.cipherAlgo)
    {
        case IX_CRYPTO_ACC_CIPHER_DES:
        case IX_CRYPTO_ACC_CIPHER_3DES:
            if (!ixCryptoAccCoProcessorExist[IX_CRYPTO_DES_COPROCESSOR])
            {
                /* DES co-processor does not exist*/
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR, 
                    "The DES coprocessor does not exist.\n", 
                    0, 0, 0, 0, 0, 0);
                    
                return IX_CRYPTO_ACC_STATUS_CIPHER_ALGO_NOT_SUPPORTED;
            }

            break;
        case IX_CRYPTO_ACC_CIPHER_AES:
            if (!ixCryptoAccCoProcessorExist[IX_CRYPTO_AES_COPROCESSOR])
            {
                /* AES co-processor not exist*/
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR, 
                    "The AES coprocessor does not exist.\n", 
                    0, 0, 0, 0, 0, 0);
                    
                return IX_CRYPTO_ACC_STATUS_CIPHER_ALGO_NOT_SUPPORTED;
            }
            break;
        default:
            break;            
    }

    /* Check if crypto hw accelerator is enabled for SHA1/MD5
     * authentication algorithm
     * Note: No check is needed for registration of WEP_CRC auth algorithm,
     *       as WEP service request could be processed in two ways, either
     *       through XScale assembly or WAN-NPE. If WEP NPE is not 
     *       enabled, client can send in the request through XScale WEP
     *       perform service request.
     */
    switch (pAccCtx->authCtx.authAlgo)
    {
        case IX_CRYPTO_ACC_AUTH_SHA1:
        case IX_CRYPTO_ACC_AUTH_MD5:
            if (!ixCryptoAccCoProcessorExist[IX_CRYPTO_HASH_COPROCESSOR])
            {
                /* HASH co-processor does not exist */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR,
                    "The AAL Hash coprocessor does not exist.\n", 
                    0, 0, 0, 0, 0, 0);
                    
                return IX_CRYPTO_ACC_STATUS_AUTH_ALGO_NOT_SUPPORTED;
            }
            break;
        case IX_CRYPTO_ACC_AUTH_CBC_MAC:
            if (!ixCryptoAccCoProcessorExist[IX_CRYPTO_AES_COPROCESSOR])
            {
                /* AES co-processor does not exist */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR,
                    IX_OSAL_LOG_DEV_STDERR,
                    "The AES coprocessor does not exist.\n", 
                    0, 0, 0, 0, 0, 0);
                    
                return IX_CRYPTO_ACC_STATUS_AUTH_ALGO_NOT_SUPPORTED;
            }
            break;
            
        default:
            break;            
    }

    /* Check if the access component has been intialized and the crypto 
     * service is running 
     */
    if (IX_CRYPTO_IS_SERVICE_INITIALIZED () 
        && !IX_CRYPTO_IS_SERVICE_STOPPED ())
    {
        /* Get an empty Crypto Context from CCD */
        status = ixCryptoCCDMgmtCryptoCtxGet (pCryptoCtxId);
    
        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
        {
            /* Store callback function pointers into Crypto Context */
            ixCryptoCtx[*pCryptoCtxId].registerCallbackFn 
                = registerCallbackFn;
            ixCryptoCtx[*pCryptoCtxId].performCallbackFn
                = performCallbackFn;    
        }
        else /* No Crypto Context is allocated */
        {
            return status;
        } /* end of if-else (status) */
    
        /* Switch case for Operation registered by client */
        switch (pAccCtx->operation)
        {
            /* Encryption operation, notes: the code for the case of 
             * IX_CRYPTO_ACC_OP_ENCRYPT fall through to the case 
             * IX_CRYPTO_ACC_OP_DECRYPT
             */
            case IX_CRYPTO_ACC_OP_ENCRYPT:
            
            /* Deceryption operation */
            case IX_CRYPTO_ACC_OP_DECRYPT:
                /* Register cipher algorithm */
                status = ixCryptoRegisterCipherAlgoRegister (
                             *pCryptoCtxId,
                             pAccCtx, 
                             &npeCryptoParamIndex,
                             IX_CRYPTO_OP_REGISTER);
                             
                /* Check if cipher context registration success */
                if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
                {
                    /* Assign NPE operation */
                    if (IX_CRYPTO_HW_ACCL_REQ == 
                        ixCryptoCtx[*pCryptoCtxId].reqType)
                    {
                        ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                            = IX_CRYPTO_NPE_OP_CRYPT;
                            
                        /* Set direction fo cryption operation, default is 
                         * decrypt, OR the NpeOperation with the mask will set 
                         * the crypt direction bit to 1 and turn it to be 
                         * encrypt operation
                         */
                        if (IX_CRYPTO_ACC_OP_ENCRYPT == pAccCtx->operation)
                        {
                            ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                                = (((ixCryptoCtx[*pCryptoCtxId].npeOperationMode.
                                    npeOperation)
                                    | IX_CRYPTO_NPE_OP_CRYPT_DIR_MASK) 
                                    & IX_CRYPTO_NPE_OP_MASK);
                        } /* end of if (pAccCtx->operation) */
                    }
                    else /* IX_CRYPTO_WEP_REQ */
                    {
                        if (IX_CRYPTO_ACC_OP_ENCRYPT == pAccCtx->operation)
                        {
                            ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                                = IX_CRYPTO_NPE_OP_WEP_ENCRYPT;
                        }
                        else /* decryption */
                        {
                            ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                                = IX_CRYPTO_NPE_OP_WEP_DECRYPT;
                        }
                    } /* end of if-else (ixCryptoCtx[*pCryptoCtxId].reqType) */
                
                    /* Set callbackFlag to TRUE to trigger client's 
                     * callback for the case of DES/3DES/ARC4 encryption and 
                     * decryption operation only
                     */                 
                    if (IX_CRYPTO_ACC_CIPHER_AES 
                        != pAccCtx->cipherCtx.cipherAlgo)
                    {
                        /* Callback is called only if DES/3DES/ARC4, callback 
                         * for AES will be called in QMgr callback context
                         */
                        callbackFlag = TRUE;
                    } /* end of if (pAccCtx->cipherCtx.cipherAlgo) */
                } /* end of if (status) */
                break;
    
            /* Authentication Computation operation, notes: the code for the  
             * case of IX_CRYPTO_ACC_OP_AUTH_CALC fall through to the case 
             * IX_CRYPTO_ACC_OP_AUTH_CHECK 
             */
            case IX_CRYPTO_ACC_OP_AUTH_CALC:
            
            /* Authentication Verification operation */
            case IX_CRYPTO_ACC_OP_AUTH_CHECK: 
                
                /* Register authentication algorithm */
                status = ixCryptoRegisterAuthAlgoRegister (
                             *pCryptoCtxId,
                             pAccCtx, 
                             pMbufPrimaryChainVar,
                             pMbufSecondaryChainVar, 
                             &npeCryptoParamIndex,
                             IX_CRYPTO_OP_REGISTER);
    
                /* Check if authentication context registration
                 * successful 
                 */
                if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
                {
                    /* Assign NPE operation */
                    if (IX_CRYPTO_HW_ACCL_REQ == 
                        ixCryptoCtx[*pCryptoCtxId].reqType)
                    {
                        if (IX_CRYPTO_ACC_OP_AUTH_CALC == pAccCtx->operation)
                        {
                            ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                                = IX_CRYPTO_NPE_OP_HMAC_GEN_ICV;
                        }
                        else /* IX_CRYPTO_ACC_OP_AUTH_CHECK */
                        {
                            ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                                = IX_CRYPTO_NPE_OP_HMAC_VER_ICV;            
                        } /* end of if-else (pAccCtx->Operation) */
                    }
                    else /* IX_CRYPTO_WEP_REQ */
                    {
                        /* Assign NPE operation */
                        if (IX_CRYPTO_ACC_OP_AUTH_CALC == pAccCtx->operation)
                        {
                            ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                                = IX_CRYPTO_NPE_OP_WEP_GEN_ICV;
                        }
                        else /* IX_CRYPTO_ACC_OP_AUTH_CHECK */
                        {
                            ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                                = IX_CRYPTO_NPE_OP_WEP_VER_ICV;
                        } /* end of if-else (pAccCtx->Operation) */                        
                    } /* end of if-else (ixCryptoCtx[*pCryptoCtxId].reqType) */
                } /* end of if (status) */
                
                /* For HMAC-MD5 & HMAC-SHA1, client's callback is called in
                 * QMgr callback context
                 */
                if (IX_CRYPTO_ACC_AUTH_WEP_CRC == pAccCtx->authCtx.authAlgo)    
                {
                    /* Set callbackFlag to TRUE to trigger client's callback */
                    callbackFlag = TRUE;
                }

                break;
    
            /* Combined service, encryption followed by authentication,
             * notes: the code for the case of IX_CRYPTO_ACC_OP_ENCRYPT_AUTH
             * fall through to the case IX_CRYPTO_ACC_OP_AUTH_DECRYPT
             */
            case IX_CRYPTO_ACC_OP_ENCRYPT_AUTH: 
            
            /* Combined service, authentication followed by decryption */
            case IX_CRYPTO_ACC_OP_AUTH_DECRYPT:
                /* Check combination of cipher algorithm and authentication
                 * algorithm. ARC4 cipher algorithm could only pair with 
                 * WEP_CRC authentication algorithm
                 */
                if ((IX_CRYPTO_ACC_CIPHER_ARC4 == pAccCtx->cipherCtx.cipherAlgo)
                    && (IX_CRYPTO_ACC_AUTH_WEP_CRC != pAccCtx->authCtx.authAlgo))
                {
                    /* Wrong combination, operation not supported */
                    return IX_CRYPTO_ACC_STATUS_OPERATION_NOT_SUPPORTED; 
                } /* end of if (pAccCtx->cipherCtx.cipherAlg) */

                /* Check combination of cipher algorithm and authentication
                 * algorithm. WEP_CRC algorithm could only pair with ARC4 
                 * cipher algorithm and not DES, 3DES or AES.
                 */
                if ((IX_CRYPTO_ACC_AUTH_WEP_CRC == pAccCtx->authCtx.authAlgo) 
                    && (IX_CRYPTO_ACC_CIPHER_ARC4 
                        != pAccCtx->cipherCtx.cipherAlgo))
                {
                    /* Wrong combination, operation not supported */
                    return IX_CRYPTO_ACC_STATUS_OPERATION_NOT_SUPPORTED; 
                } /* end of if (pAccCtx->authCtx.authAlg) */
            
                /* Only permissible combination when CCM opearation with cipher
                 * mode =CCM and auth algo =CBC_MAC(2 set of tests done below). 
                 */
                /* Test cipher mode CCM but auth algo not CBC MAC */
                if( (IX_CRYPTO_ACC_MODE_CCM == pAccCtx->cipherCtx.cipherMode)
                    &&(IX_CRYPTO_ACC_AUTH_CBC_MAC != pAccCtx->authCtx.authAlgo))
                {
                    /* Wrong combination, operation not supported */
                    return IX_CRYPTO_ACC_STATUS_OPERATION_NOT_SUPPORTED; 
                }

                /* Test cipher mode not CCM but auth algo is CBC MAC */
                if( (IX_CRYPTO_ACC_MODE_CCM != pAccCtx->cipherCtx.cipherMode)
                    &&(IX_CRYPTO_ACC_AUTH_CBC_MAC == pAccCtx->authCtx.authAlgo))
                {
                    /* Wrong combination, operation not supported */
                    return IX_CRYPTO_ACC_STATUS_OPERATION_NOT_SUPPORTED; 
                }
 
                /* Callback for ARC4 with WEP_CRC and CCM will be called in the same 
                 * context instead of qmgr dispatcher context 
                 */
                if( ((IX_CRYPTO_ACC_CIPHER_ARC4 == pAccCtx->cipherCtx.cipherAlgo)
                          && (IX_CRYPTO_ACC_AUTH_WEP_CRC == pAccCtx->authCtx.authAlgo))
                     ||  (IX_CRYPTO_ACC_MODE_CCM == pAccCtx->cipherCtx.cipherMode))
                    
                {
                    /* Set callbackFlag to TRUE to trigger client's callback */
                    callbackFlag = TRUE;
                }
            
                /* Register cipher algorithm */
                status = ixCryptoRegisterCipherAlgoRegister (
                             *pCryptoCtxId,
                             pAccCtx, 
                             &npeCryptoParamIndex,
                             IX_CRYPTO_OP_WAIT);
    
                /* Check if cipher context registration successful */
                if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
                {
                    /* Register authentication algorithm */
                    status = ixCryptoRegisterAuthAlgoRegister (
                                 *pCryptoCtxId,
                                 pAccCtx, 
                                 pMbufPrimaryChainVar,
                                 pMbufSecondaryChainVar, 
                                 &npeCryptoParamIndex,
                                 IX_CRYPTO_OP_REGISTER);
                
                    /* Check if authentication context registration 
                     * successful 
                     */
                    if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
                    {
                        /* Assign NPE operation */
                        if (IX_CRYPTO_HW_ACCL_REQ == 
                            ixCryptoCtx[*pCryptoCtxId].reqType)
                        {
                            if (IX_CRYPTO_ACC_OP_ENCRYPT_AUTH 
                                == pAccCtx->operation)
                            {
                                /* If the cipher mode was CCM then set 
                                 * NPE operation to CCM GEN else set it to HMAC_GEN 
                                 */
                                if(IX_CRYPTO_ACC_MODE_CCM == 
                                    pAccCtx->cipherCtx.cipherMode)
                                {
                                    ixCryptoCtx[*pCryptoCtxId].npeOperationMode.
                                        npeOperation 
                                            = IX_CRYPTO_NPE_OP_CCM_GEN_MIC;
                                }
                                else /* cipherMode != IX_CRYPTO_ACC_MODE_CCM */
                                {
                                    ixCryptoCtx[*pCryptoCtxId].npeOperationMode.
                                        npeOperation 
                                            = IX_CRYPTO_NPE_OP_ENCRYPT_HMAC_GEN_ICV;
                                }/* end of if-else (pAccCtx->cipherMode) */

                            }
                            else /* IX_CRYPTO_ACC_OP_AUTH_DECRYPT */
                            {                                
                                /* If the cipher mode was CCM then set 
                                 * NPE operation to CCM VER else set it to HMAC_VER.
                                 */
                                if(IX_CRYPTO_ACC_MODE_CCM == 
                                    pAccCtx->cipherCtx.cipherMode)
                                {
                                    ixCryptoCtx[*pCryptoCtxId].npeOperationMode.
                                        npeOperation 
                                            = IX_CRYPTO_NPE_OP_CCM_VER_MIC;
                                }
                                else /* cipherMode != IX_CRYPTO_ACC_MODE_CCM */
                                {
                                     ixCryptoCtx[*pCryptoCtxId].npeOperationMode.
                                         npeOperation
                                             = IX_CRYPTO_NPE_OP_HMAC_VER_ICV_DECRYPT;
                                }
                            }/* end of if-else (pAccCtx->operation) */
                        }
                        else /* IX_CRYPTO_WEP_REQ*/
                        {
                            if (IX_CRYPTO_ACC_OP_ENCRYPT_AUTH 
                                == pAccCtx->operation)
                            {
                                ixCryptoCtx[*pCryptoCtxId].npeOperationMode.
                                    npeOperation 
                                        = IX_CRYPTO_NPE_OP_WEP_ENCRYPT_GEN_ICV;
                            }
                            else /* IX_CRYPTO_ACC_OP_AUTH_DECRYPT */
                            {
                                ixCryptoCtx[*pCryptoCtxId].npeOperationMode.
                                    npeOperation
                                        = IX_CRYPTO_NPE_OP_WEP_VER_ICV_DECRYPT;
                            } /* end of if-else (pAccCtx->operation) */
                        } /* end of if-else (ixCryptoCtx[*pCryptoCtxId].reqType) */
                    } /* end of if (status) */
                } /* end of if (status) */
                break;
            /* all others case */
            default:
                /* operation not supported */
                status = IX_CRYPTO_ACC_STATUS_OPERATION_NOT_SUPPORTED;  

        } /* end of switch (pAccCtx->operation) */
                
        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status) 
        {
            /* Set transfer mode, default is in-place transfer mode, OR
             * the NpeOperation with the mask will set the transfer mode
             * to 1 and turn it to non in-place operation.
             * Set init length for hw accel req also.
             */
            if (IX_CRYPTO_HW_ACCL_REQ == ixCryptoCtx[*pCryptoCtxId].reqType)
            {
                if (pAccCtx->useDifferentSrcAndDestMbufs)
                {
                    ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                        = (((ixCryptoCtx[*pCryptoCtxId].npeOperationMode.
                              npeOperation)
                              | IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK) 
                              & IX_CRYPTO_NPE_OP_MASK);
                } /* end of if (pAccCtx->useDifferentSrcAndDestMbufs) */
        
                /* Assign Init Length for NPE Crypto Parameters structure in 
                 * Crypto Context 
                 */
                ixCryptoCtx[*pCryptoCtxId].npeOperationMode.initLength 
                    = npeCryptoParamIndex;
             }
             else /* IX_CRYPTO_WEP_REQ */
             {
                if (pAccCtx->useDifferentSrcAndDestMbufs)
                {
                    ixCryptoCtx[*pCryptoCtxId].npeOperationMode.npeOperation
                        = (((ixCryptoCtx[*pCryptoCtxId].npeOperationMode.
                              npeOperation)
                              | IX_CRYPTO_NPE_OP_WEP_TRANSFER_MODE_MASK) 
                              & IX_CRYPTO_NPE_OP_MASK);
                } /* end of if (pAccCtx->useDifferentSrcAndDestMbufs) */                
             } /* end of if-else (ixCryptoCtx[*pCryptoCtxId].reqType) */

             /* Check if client's callback need to be called here */
             if (callbackFlag) /* Callback needed */
             {
                    /* Mark valid bit in Crypto Context to indicate the 
                     * Crypto Context has been registered successfully
                     */
                    ixCryptoCtx[*pCryptoCtxId].valid = TRUE;
                        
                    /* Call client's callback */
                    ixCryptoCtx[*pCryptoCtxId].registerCallbackFn(
                        *pCryptoCtxId,
                        NULL, 
                        status);                
            } /* end of if (callbackFlag) */                

            return status;
        }
        else /* Registration failed */
        {
            /* Release Crypto Context allocated previously */
            ixCryptoCCDMgmtCryptoCtxRelease (*pCryptoCtxId);
    
            return status;
        } /* end of if (status) */
    }
    else /* (IX_CRYPTO_IS_SERVICE_INITIALIZED) &&
          * (!IX_CRYPTO_IS_SERVICE_STOPPED)
          */
    {
        
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "Component has not been initialized\n", 
            0, 0, 0, 0, 0, 0);
                
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (IX_CRYPTO_IS_SERVICE_INITIALIZED) &&
       * (!IX_CRYPTO_IS_SERVICE_STOPPED) 
       */
} /* End of ixCryptoAccCtxRegister () function */



/**
 * @fn   ixCryptoAccAuthCryptPerform
 * @brief Perform Authentication and Decryption/Encryption functionalities
 *
 */

PUBLIC IxCryptoAccStatus
ixCryptoAccAuthCryptPerform (
    UINT32 cryptoCtxId,
    IX_OSAL_MBUF *pSrcMbuf,
    IX_OSAL_MBUF *pDestMbuf,
    UINT16 authStartOffset,
    UINT16 authDataLen,
    UINT16 cryptStartOffset,
    UINT16 cryptDataLen,
    UINT16 icvOffset,
    UINT8  *pIV)
{
    IxCryptoQDescriptor *pQDesc = NULL;
    UINT32 operationMode;
    UINT32 npeOperation;
    IxCryptoNpeOperationStatus operStatus = IX_CRYPTO_OP_PERFORM;
    UINT32 *pCipherCfg;
    IxCryptoAccStatus status;
    IX_OSAL_MBUF *pMbuf = pSrcMbuf;
    BOOL useDiffBuf = FALSE;
    UINT8 *tempIcvAddr = NULL;
    UINT32 npeCryptCfg;
    UINT32 cipherMode;
    UINT32 u32pQDesc = 0;
    UINT16 shiftOffset = 0;
       
    /* Check if the access component has been intialized and the crypto service
       is running */
    if (IX_CRYPTO_IS_HW_ACCL_INITIALIZED()
        && !IX_CRYPTO_IS_SERVICE_STOPPED())
    {
        /* Check the validity of CryptoCtxId */
        status = ixCryptoCCDMgmtCtxValidCheck (cryptoCtxId);
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "cryptoCtxId %d is not valid or registered.\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);

            return status;          
        }
        
        /* Check the operation requested. If the crypto context was not 
         * registered for crypto hw accelerator service, reject the service
         * request
         */
        if (IX_CRYPTO_HW_ACCL_REQ != ixCryptoCtx[cryptoCtxId].reqType)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "cryptoCtxId %d is not registered for hw acclerator service.\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;
        }
     
        /* Now we know CtxId valid, look up record for the NPE operationMode */
        operationMode = (ixCryptoCtx[cryptoCtxId].npeOperationMode.npeOperation
                            & MASK_8_BIT);
           
        /* Check for NULL MBufs, and the destination mbuf will only be checked 
         * if the transfer mode is non in place */
        if  ((NULL == pSrcMbuf)||
             ((IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE != (operationMode 
             & IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK) &&
             (NULL==pDestMbuf))))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Input Mbufs not valid. pSrcMbuf=0x%X\n \
                pDestMbuf=0x%X\n",
                (int) pSrcMbuf,
                (int) pDestMbuf, 
                0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;       
        }

#ifndef NDEBUG    
        /* Perform boundary check. 
         * Note that the icvOffset is not checked against the mbuf length.
         * The actual length of the mbuf will not be known. Only the length
         * of the data stored in a mbuf is known.
         */
        status = ixCryptoPerformBufferOffsetBoundaryCheck (
                     operationMode,
                     pSrcMbuf,
                     pDestMbuf,
                     authStartOffset,
                     authDataLen,
                     cryptStartOffset,
                     cryptDataLen,
                     icvOffset);
        
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            return status;
        } /* End of Boundary Check */       
#endif

        /* Get a free Q descriptor from Q descriptors pool. Return if 
         * unsuccessful */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
            ixCryptoDescMgmtQDescriptorGet (&pQDesc))
        {
            /* Error logging is done in the DescMgmt module */
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* End of ixCryptoDescMgmtQDescriptorGet */

        /* Get NPE operation */
        npeOperation = operationMode & (~IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK);
        
        /* Get operation transfer mode */
        if (IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE != (operationMode 
             & IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK))
        {
            /* set to TRUE if non in-place operation */
            useDiffBuf = TRUE;
        }
        
        /* Switch case based on NPE operation. Two tasks need to be 
         * executed before sending the request to NPE.
         * 1. Check if the ICV is in the middle of data. If the ICV
         *    is in the middle of data and the NPE operation is 
         *    IX_CRYPTO_NPE_OP_HMAC_VER_ICV or 
         *    IX_CRYPTO_NPE_OP_HMAC_VER_ICV_DECRYPT; copy the original 
         *    ICV value into queue descriptor and store the original ICV 
         *    offset into queue descriptor too. Pass in the new ICV address to 
         *    NPE with the ICV field in the source mbuf is cleared to zero.
         * 2. Check if the operation is non in-place operation, if it
         *    is non in-place operation, shift the ixp_data pointer in 
         *    NPE shared structure of destination mbuf to authStartOffset/
         *    cryptStartOffset depending on the NPE operation. As NPE always 
         *    start writing to offset 0 instead of authStartOffset and  
         *    cryptStartOffset. Thus the ixp_data pointer need to be shifted in  
         *    order to get the data write to the correct offset.
         *    Note: 
         *    - Invalidate/flush of buffer will base on mdata pointer in mbuf 
         *      (original data buffer location), not ixp_data.
         *    - Don't have to shift the ixp_data back to its original location
         *      after NPE complete the task, as this pointer will never be 
         *      access after the task complete.
         */        
        switch (npeOperation)
        {

            /* For CCM verify operation, the icvAddr calculation is (almost)
             * identical to HMAC verify operation. The difference is that
             * for CCM processing, authStartOffset and authDataLen are never
             * set. Therefore, intialize authStartOffset to cryptoStartOffset
             * and authDataLen to cryptDataLen and fall through as if it were
             * a HMAC verify request.
             */
            case IX_CRYPTO_NPE_OP_CCM_VER_MIC:
                authStartOffset = cryptStartOffset;
                authDataLen = cryptDataLen;
                /* Fall through  */

            /* HMAC verification only, or combined service with
             * HMAC verification operation. Notes : the code for 
             * HMAC verification fall thorugh to combined service
             * IX_CRYPTO_NPE_OP_HMAC_VER_ICV_DECRYPT
             */
            case IX_CRYPTO_NPE_OP_HMAC_VER_ICV:
            case IX_CRYPTO_NPE_OP_HMAC_VER_ICV_DECRYPT:
                if (useDiffBuf) /* If non in-place operation */
                {
                    /* Need to shift ixp_data pointer to right by 
                     * authStartOffset. ixp_data buffer pointer will be shifted
                     * in ixCryptoUtilMbufToNpeFormatConvert().
                     */
                    shiftOffset = authStartOffset;
                } /* end of if (useDiffBuf) */

                /* Store original ICV offset into queue descriptor */
                pQDesc->value.originalIcvOffset = icvOffset;
                
                /* Get ICV address */
                tempIcvAddr = (UINT8 *)
                                  ixCryptoUtilMbufOffsetToAddressConvert (
                                  pSrcMbuf,
                                  icvOffset,
                                  FALSE);
                                  
                /* Store original ICV value into queue descriptor */
                ixOsalMemCopy (
                    pQDesc->integrityCheckValue, 
                    tempIcvAddr, 
                    ixCryptoCtx[cryptoCtxId].digestLength);
                
                if ((icvOffset >= authStartOffset) &&
                    (icvOffset < (authStartOffset + authDataLen)))
                {       
                    /* Clear ICV field in mbuf to zero */    
                    ixOsalMemSet (
                        tempIcvAddr, 
                        0, 
                        ixCryptoCtx[cryptoCtxId].digestLength);
                }
                
                /* Assign ICV value location to queue descriptor */    
                tempIcvAddr = pQDesc->integrityCheckValue;
                
                /* Convert ICV address to physical address and then 
                   swap to network byte order */
                pQDesc->npeQDesc.address.icvAddr
                    = (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (
                          (UINT32) tempIcvAddr);

                pQDesc->npeQDesc.address.icvAddr
                    = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER 
                        (pQDesc->npeQDesc.address.icvAddr);
 
                break; 

            /* For CCM GEN operation, the icvAddr calculation is (almost)
             * identical to HMAC GEN operation. The difference is that
             * for CCM processing, authStartOffset and authDataLen are never
             * set. Therefore, intialize authStartOffset to cryptoStartOffset
             * and authDataLen to cryptDataLen and fall through as if it were
             * a HMAC verify request.
             */
            case IX_CRYPTO_NPE_OP_CCM_GEN_MIC:
                authStartOffset = cryptStartOffset;
                authDataLen = cryptDataLen;
                /* Fall through  */
            
            /* HMAC generation only, or combined service with
             * HMAC generation operation. Notes : the code for 
             * HMAC generation fall thorugh to combined service
             * IX_CRYPTO_NPE_OP_ENCRYPT_HMAC_GEN_ICV
             */
            case IX_CRYPTO_NPE_OP_HMAC_GEN_ICV:
            case IX_CRYPTO_NPE_OP_ENCRYPT_HMAC_GEN_ICV:
            
                if (useDiffBuf) /* If non in-place */
                {
                    pMbuf = pDestMbuf;
                    shiftOffset = authStartOffset;
                } /* end of if (useDiffBuf) */
                
                /* Get ICV address */
                tempIcvAddr = (UINT8 *)
                    ixCryptoUtilMbufOffsetToAddressConvert (
                        pMbuf,
                        icvOffset,
                        useDiffBuf);
                
                /* Convert ICV address into physical address then
                   swap to network byte order  */
                pQDesc->npeQDesc.address.icvAddr
                    = (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (
                          tempIcvAddr);

                pQDesc->npeQDesc.address.icvAddr
                    = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER 
                      (pQDesc->npeQDesc.address.icvAddr);    

                break; 
           
            /* Encrypt/Decrypt onlyoperation */
            default :
                if (useDiffBuf) /* If non in-place operation */
                {
                    shiftOffset = cryptStartOffset;
                } /* end of if (useDiffBuf) */
                break;
        } /* end of switch case (npeOperation) */ 
        
        /* 
         * If we are doing crypto we need to set up IV. No IV needed for 
         * Authentication. The cipher block length check is only performed 
         * when the operationMode is crypt */   
        if(IX_CRYPTO_NPE_OP_CRYPT_MODE_IS_DISABLED != 
           (operationMode & IX_CRYPTO_NPE_OP_CRYPT_ENABLE_MASK )) 
        { 
            /* 
             * We are doing some Crypto: Now we can be sure that the 
             * NpeCryptCfgWord is at start of UINT8 NpeCryptoInfo array
             */ 
#ifndef NDEBUG
            /* Check for the correct cipher text and plain text length */          
            status = ixCryptoPerformCipherBlkLengthCheck (
                         npeOperation,
                         ixCryptoCtx[cryptoCtxId].cipherBlockLength,
                         cryptStartOffset,
                         cryptDataLen);
                
            if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
            {
                /* Log error message in debugging mode */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_MESSAGE, 
                    IX_OSAL_LOG_DEV_STDOUT,
                    "IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_BLOCK_LEN.\n", 
                    0, 0, 0, 0, 0, 0);

                /* Need to release descriptor before returning. 
                 * Descriptor release will log a message when fail */
                ixCryptoDescMgmtQDescriptorRelease(pQDesc);

                return status;          
            } /* End of invalid block length */ 
#endif

            /* Read NPE Crypto Cfg Word from CCD database. */
            pCipherCfg = (UINT32 *)ixCryptoCtx[cryptoCtxId].
                             pNpeCryptoParam->npeCryptoInfo;

            /* Convert to Network Order (Big Endian) */
            npeCryptCfg = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER (
                              *pCipherCfg);

            /* Extract CipherMode from NPE Crypt Cfg Word */
            cipherMode = ((npeCryptCfg >>
                           IX_CRYPTO_NPE_CRYPT_CFG_CIPHERMODE_LOC) &
                           IX_CRYPTO_NPE_CRYPT_CFG_CIPHERMODE_MASK);

            /* Check that IV needed, for either CBC or CTR  mode */
            if(IX_CRYPTO_NPE_CRYPT_CFG_ECB != cipherMode)
            {
                /* First check IV not NULL */
                if(NULL==pIV)
                {   
                    /* Log error message in debugging mode */
                    IX_CRYPTO_ACC_LOG (
                        IX_OSAL_LOG_LVL_MESSAGE, 
                        IX_OSAL_LOG_DEV_STDOUT,
                        "Input IV not valid == NULL.\n", 
                        0, 0, 0, 0, 0, 0);

                    /* Descriptor release will log a message when fail */
                    ixCryptoDescMgmtQDescriptorRelease(pQDesc);
                    
                    return IX_CRYPTO_ACC_STATUS_FAIL;  
                } /* End of pIV Null check */
                
                /* Copy initialization vector into NPE queue descriptor */
                ixOsalMemCopy (pQDesc->npeQDesc.IV, pIV, 
                        ixCryptoCtx[cryptoCtxId].cipherIvLength);

                /* Check if it's CCM request */
                if( (npeOperation ==  IX_CRYPTO_NPE_OP_CCM_GEN_MIC ) ||
                    (npeOperation ==  IX_CRYPTO_NPE_OP_CCM_VER_MIC)) 
                {
                    /* Copy the aad in IV to the NPE Q descriptor buffer
                     * and flush it. Then do address translation from physical 
                     * to virtual and byte order conversion of the aadAddr.
                     */
                    ixOsalMemCopy ( (UINT8*)
                             pQDesc->npeQDesc.aadAddr ,
                             &pIV[ixCryptoCtx[cryptoCtxId].cipherIvLength], 
                             ixCryptoCtx[cryptoCtxId].aadLen);

                    IX_CRYPTO_DATA_CACHE_FLUSH ((UINT32*)
                        (pQDesc->npeQDesc.aadAddr),
                         ixCryptoCtx[cryptoCtxId].aadLen);

                    pQDesc->npeQDesc.aadAddr 
                        = (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (
                            pQDesc->npeQDesc.aadAddr);                        

                    pQDesc->npeQDesc.aadAddr 
                        = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER (
                            pQDesc->npeQDesc.aadAddr);

                }/* End of check for CCM */
            } /* End of if(ECB)  */      
        } /* End of if (Crypto) */
                  
        /* Construct Q descriptor message */   
        pQDesc->cryptoCtxId = cryptoCtxId;
        pQDesc->operStatus = operStatus;
    
        /* Set to NPE operation and initial values in Crypto Param structure 
         * As npeOperation and initLength are byte-length member fields,
         * conversion is not needed in data coherency endianness. 
         */
        pQDesc->npeQDesc.npeOperationMode.npeOperation
             = ixCryptoCtx[cryptoCtxId].npeOperationMode.npeOperation;

        pQDesc->npeQDesc.npeOperationMode.initLength
             = ixCryptoCtx[cryptoCtxId].npeOperationMode.initLength;
              
        /* Store Src Mbuf pointer into queue descriptor. This pointer will be
         * used in QMgr callback for mbuf conversion and this pointer will be
         * passed back to client as Src Mbuf pointer
         */
        pQDesc->pSrcMbuf = pSrcMbuf;
        
        /* Convert source mbuf into NPE format. Virtual address must be used when
         * flushing to external memory */
        pQDesc->npeQDesc.srcMbufAddr = (UINT32)
            (ixCryptoUtilMbufToNpeFormatConvert(pSrcMbuf, 0));
          
        /* If operationMode is in place, DestMbuf will be empty and don't need to
         * be written to the pQDesc or flushed to memory.
         */
        if (IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE != 
            (operationMode & IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK))
        {
            /* Store Dest Mbuf pointer into queue descriptor. This pointer will 
             * be used in QMgr callback for mbuf conversion and this pointer 
             * will be passed back to client as Dest Mbuf pointer
             */
            pQDesc->pDestMbuf = pDestMbuf;
            
            /* Convert destination mbuf into NPE format */
            pQDesc->npeQDesc.destMbufAddr = (UINT32)
                (ixCryptoUtilMbufToNpeFormatConvert (pDestMbuf, shiftOffset));
        }
        
        /* Offset and length of data */
        pQDesc->npeQDesc.authStartOffset
            = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER(authStartOffset);
    
        pQDesc->npeQDesc.authLength
            = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER(authDataLen);

        pQDesc->npeQDesc.cryptStartOffset
            = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER(cryptStartOffset);

        pQDesc->npeQDesc.cryptLength
            = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER(cryptDataLen);
        
        /* Write Crypto Param Address and flush*/
        pQDesc->npeQDesc.cryptoCtxNpeAddr 
            = (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (
            ixCryptoCtx[cryptoCtxId].pNpeCryptoParam);

        pQDesc->npeQDesc.cryptoCtxNpeAddr 
        = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER 
              (pQDesc->npeQDesc.cryptoCtxNpeAddr);
        
        /* Flush Q descriptor message from cache */
        IX_CRYPTO_DATA_CACHE_FLUSH (pQDesc, sizeof (IxCryptoQDescriptor));
        
        /* translate pQDesc after flush */    
        pQDesc = (IxCryptoQDescriptor*) 
            IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (pQDesc);

        /* Enqueue request to hardware queue using QMgr */
        u32pQDesc = (UINT32) pQDesc;
        status = ixCryptoQAccessQueueWrite (IX_CRYPTO_ACC_CRYPTO_REQ_Q,
                     &u32pQDesc);               
    
        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
        {
            ixCryptoCtx[cryptoCtxId].reqCount++;
        }
        else /* End of status check for Queue Write */ 
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "ixCryptoQAccessQueueWrite failed.\n", 
                0, 0, 0, 0, 0, 0);

            /* Convert Q descriptor address back to virtual address 
             * before releasing it
             */
            pQDesc = (IxCryptoQDescriptor *) 
                         IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION(
                         (UINT32) pQDesc);
                           
            /* Convert mbuf back to host format from NPE format */
            pSrcMbuf = ixCryptoUtilMbufFromNpeFormatConvert (
                           pQDesc->pSrcMbuf);
            pDestMbuf = ixCryptoUtilMbufFromNpeFormatConvert (
                            pQDesc->pDestMbuf);

            /* If it's a CCM encrypt or decrypt operation convert 
             * back the aadAddr conversion.
             */
           if( (npeOperation ==  IX_CRYPTO_NPE_OP_CCM_GEN_MIC ) ||
               (npeOperation ==  IX_CRYPTO_NPE_OP_CCM_VER_MIC)) 
             {
                /* Convert the aadAddr to host byte order*/
                pQDesc->npeQDesc.aadAddr 
                        = IX_CRYPTO_CONVERT_WORD_TO_HOST_ORDER (
                          pQDesc->npeQDesc.aadAddr);

                /* Convert the aadAddr from physical to virtual address */
                pQDesc->npeQDesc.aadAddr 
                        = (UINT32)IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION (
                          pQDesc->npeQDesc.aadAddr);                          
             }

            /* When the Queue write fails, pQDesc needs to be released */
            ixCryptoDescMgmtQDescriptorRelease(pQDesc);
     
        } /* End of if-else for Queue Write check */
        return status;
    }
    else /* end of if (IX_CRYPTO_IS_HW_ACCL_INITIALIZED) &&
        (IX_CRYPTO_IS_SERVICE_STOPPED) */
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "IxCrypto is not initialized or it is stopped.\n", 
            0, 0, 0, 0, 0, 0);   

        return IX_CRYPTO_ACC_STATUS_FAIL;
    }/* end of if-else (IX_CRYPTO_IS_HW_ACCL_INITIALIZED) &&
    (IX_CRYPTO_IS_SERVICE_STOPPED) */  
} /* end of ixCryptoAccAuthCryptPerform () function */


/**
 * @fn IxCryptoAccStatus ixCryptoAccNpeWepPerform
 * @brief Function to invoke ARC4 and WEP ICV computations on NPE.
 *
 */
PUBLIC IxCryptoAccStatus
ixCryptoAccNpeWepPerform (
    UINT32 cryptoCtxId,
    IX_OSAL_MBUF *pSrcMbuf,
    IX_OSAL_MBUF *pDestMbuf,
    UINT16 startOffset,   
    UINT16 dataLen,
    UINT16 icvOffset,
    UINT8 *pKey)
{
    IxCryptoQDescriptor *pQDesc = NULL;
    UINT32 operationMode;
    UINT32 npeOperation;
    IxCryptoNpeOperationStatus operStatus = IX_CRYPTO_OP_WEP_PERFORM;
    IxCryptoAccStatus status;
    UINT8 *tempIcvAddr = NULL;
    UINT8* srcIcvAddr;
    UINT32 u32pQDesc;
           
    /* Check if the access component has been intialized and the crypto service
       is running */
    if (IX_CRYPTO_IS_WEP_NPE_INITIALIZED()
        && !IX_CRYPTO_IS_SERVICE_STOPPED())
    {
        /* Check the validity of CryptoCtxId */
        status = ixCryptoCCDMgmtCtxValidCheck (cryptoCtxId);
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "cryptoCtxId %d is not valid or registered.\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);

            return status;          
        }
        
        /* Check the operation requested. If the crypto context was not 
         * registered for WEP service, reject the service request
         */
        if (IX_CRYPTO_WEP_REQ != ixCryptoCtx[cryptoCtxId].reqType)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "cryptoCtxId %d is not registered for WEP service.\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;
        }
     
        /* Now we know CtxId valid, look up record for the NPE operationMode */
        operationMode = (ixCryptoCtx[cryptoCtxId].npeOperationMode.npeOperation
                            & MASK_8_BIT);
           
        /* Check for NULL MBufs, and the destination mbuf will only be checked 
         * if the transfer mode is non in place */
        if  ((NULL == pSrcMbuf)||
             ((IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE != (operationMode 
             & IX_CRYPTO_NPE_OP_WEP_TRANSFER_MODE_MASK) &&
             (NULL == pDestMbuf))))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Input Mbufs not valid. pSrcMbuf=0x%X\n \
                pDestMbuf=0x%X\n",
                (int) pSrcMbuf,
                (int) pDestMbuf, 
                0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;       
        }
    
#ifndef NDEBUG    
        /* Perform boundary check. 
         * The actual length of the mbuf will not be known. Only the length
         * of the data stored in a mbuf is known.
         */
        status = ixCryptoPerformWepBufferOffsetBoundaryCheck (
                     operationMode,
                     pSrcMbuf,
                     pDestMbuf,
                     startOffset,
                     dataLen,
                     icvOffset,
                     ixCryptoCtx[cryptoCtxId].digestLength);

        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            return status;
        } /* End of Boundary Check */       
#endif

        /* Get a free Q descriptor from Q descriptors pool. Return if 
         * unsuccessful */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
            ixCryptoDescMgmtQDescriptorGet (&pQDesc))
        {
            /* Error logging is done in the DescMgmt module */
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* End of ixCryptoDescMgmtQDescriptorGet */

        /* Get NPE operation */
        npeOperation = operationMode & 
                           (~IX_CRYPTO_NPE_OP_WEP_TRANSFER_MODE_MASK);

        /* If transfer mode is non in-place, shift the ixp_data pointer, as
         * NPE will always write to mbuf starting from the first byte 
         * regardless of the startOffset
         */
        if (IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE != (operationMode 
             & IX_CRYPTO_NPE_OP_WEP_TRANSFER_MODE_MASK))
        {
            /* If its gen ICV then the ICV ptr comes from 
             * pDestMbuf. 
             * If its verify ICV then the ICV is first copied 
             * from the source to the destination and the ICV ptr
             * passed to the npe is from the pDestMbuf.
             */
            if ((IX_CRYPTO_NPE_OP_WEP_GEN_ICV == npeOperation)
                || (IX_CRYPTO_NPE_OP_WEP_ENCRYPT_GEN_ICV == npeOperation))
            {
                tempIcvAddr = (UINT8 *)
                                ixCryptoUtilMbufOffsetToAddressConvert (
                                   pDestMbuf,
                                   icvOffset,
                                   FALSE); 

            }
            else
            {
                if ((IX_CRYPTO_NPE_OP_WEP_VER_ICV == npeOperation)
                    || (IX_CRYPTO_NPE_OP_WEP_VER_ICV_DECRYPT == npeOperation))
                {
                    srcIcvAddr = (UINT8 *)
                                    ixCryptoUtilMbufOffsetToAddressConvert (
                                        pSrcMbuf,
                                        icvOffset,
                                        FALSE); 

                    tempIcvAddr = (UINT8*) 
                                     ixCryptoUtilMbufOffsetToAddressConvert (
                                         pDestMbuf,
                                         icvOffset,
                                         FALSE); 
                     
                    /* Copy the ICV from source to destination */
                    ixOsalMemCopy(tempIcvAddr,srcIcvAddr, 
                            ixCryptoCtx[cryptoCtxId].digestLength);

                    IX_CRYPTO_DATA_CACHE_FLUSH (
                        tempIcvAddr,
                        ixCryptoCtx[cryptoCtxId].digestLength);
                }
            } /* end of if-else (npeOperation) */            
        }
        else /* Inplace mode of operation - ICV always fetched from src */
        { 
            if ((IX_CRYPTO_NPE_OP_WEP_GEN_ICV == npeOperation)
                || (IX_CRYPTO_NPE_OP_WEP_ENCRYPT_GEN_ICV == npeOperation)
                || (IX_CRYPTO_NPE_OP_WEP_VER_ICV == npeOperation)
                || (IX_CRYPTO_NPE_OP_WEP_VER_ICV_DECRYPT == npeOperation))
            {
        
                tempIcvAddr = (UINT8*) 
                                ixCryptoUtilMbufOffsetToAddressConvert (
                                    pSrcMbuf,
                                    icvOffset,
                                    FALSE); 
            }
        }

        /* Switch case based on NPE operation. Two tasks need to be 
         * executed before sending the request to NPE.
         * 1. Convert ICV offset into physical address for NPE if 
         *    authentication / combined service operation is selected.
         * 2. Copy ARC4 per packet key into Q descriptor in IV field to
         *    pass the key to NPE if encryption / combined service operation
         *    is selected.
         */        
        switch (npeOperation)
        {
            /* Encryption or decryption only.
             * Decryption fall through to encryption case statement.
             */
            case IX_CRYPTO_NPE_OP_WEP_DECRYPT:
            case IX_CRYPTO_NPE_OP_WEP_ENCRYPT:
                /* First check ARC4 key not NULL */
                if (NULL == pKey)
                {   
                    /* Log error message in debugging mode */
                    IX_CRYPTO_ACC_LOG (
                        IX_OSAL_LOG_LVL_MESSAGE, 
                        IX_OSAL_LOG_DEV_STDOUT,
                        "Input ARC4 key not valid == NULL.\n", 
                        0, 0, 0, 0, 0, 0);

                    /* Descriptor release will log a message when fail */
                    ixCryptoDescMgmtQDescriptorRelease(pQDesc);
                    
                    return IX_CRYPTO_ACC_STATUS_FAIL;  
                } /* End of ARC4 NULL check */
         
                /* Copy ARC4 per packet key into NPE queue descriptor */
                ixOsalMemCopy (pQDesc->npeQDesc.IV, pKey, 
                    ixCryptoCtx[cryptoCtxId].cipherKeyLength);  
                break;

            /* WEP CRC-32 generation and verification only.
             * WEP CRC-32 verification fall through to WEP CRC-32 generation
             * case statement.
             */
            case IX_CRYPTO_NPE_OP_WEP_VER_ICV:
            case IX_CRYPTO_NPE_OP_WEP_GEN_ICV:

                /* Convert ICV address to physical address and then 
                 * swap to network byte order 
                 */
                pQDesc->npeQDesc.address.icvAddr
                    = (UINT32) IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (
                          (UINT32) tempIcvAddr);

                pQDesc->npeQDesc.address.icvAddr
                    = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER 
                          (pQDesc->npeQDesc.address.icvAddr);
                break;
            
            /* Combined service only.
             * Authentication & decryption fall through encryption &
             * authentication case statement.
             */
            case IX_CRYPTO_NPE_OP_WEP_VER_ICV_DECRYPT:
            case IX_CRYPTO_NPE_OP_WEP_ENCRYPT_GEN_ICV:
                /* First check ARC4 key not NULL */
                if (NULL == pKey)
                {   
                    /* Log error message in debugging mode */
                    IX_CRYPTO_ACC_LOG (
                        IX_OSAL_LOG_LVL_MESSAGE, 
                        IX_OSAL_LOG_DEV_STDOUT,
                        "Input ARC4 key not valid == NULL.\n", 
                        0, 0, 0, 0, 0, 0);

                    /* Descriptor release will log a message when fail */
                    ixCryptoDescMgmtQDescriptorRelease(pQDesc);
                    
                    return IX_CRYPTO_ACC_STATUS_FAIL;  
                } /* End of ARC4 NULL check */            
                
                /* Copy ARC4 per packet key into NPE queue descriptor */
                ixOsalMemCopy (pQDesc->npeQDesc.IV, pKey, 
                    ixCryptoCtx[cryptoCtxId].cipherKeyLength);  

                /* Convert ICV address to physical address and then 
                 * swap to network byte order 
                 */
                pQDesc->npeQDesc.address.icvAddr
                    = (UINT32) IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (
                          (UINT32) tempIcvAddr);

                pQDesc->npeQDesc.address.icvAddr
                    = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER 
                          (pQDesc->npeQDesc.address.icvAddr);
                break; 
           
            /* Should not reach this case, something is wrong */
            default :
                return IX_CRYPTO_ACC_STATUS_FAIL;                
        } /* end of switch case (npeOperation) */
                  
        /* Construct Q descriptor message */   
        pQDesc->cryptoCtxId = cryptoCtxId;
        pQDesc->operStatus = operStatus;
    
        /* Set to NPE operation in Crypto Param structure. 
         * Init length is not needed in WEP NPE perform function call.
         * As npeOperation is byte-length member fields,
         * conversion is not needed in data coherency endianness. 
         */
         pQDesc->npeQDesc.npeOperationMode.npeOperation
             = ixCryptoCtx[cryptoCtxId].npeOperationMode.npeOperation;
              
        /* Store Src Mbuf pointer into queue descriptor. This pointer will be
         * used in QMgr callback for mbuf conversion and this pointer will be
         * passed back to client as Src Mbuf pointer
         */
        pQDesc->pSrcMbuf = pSrcMbuf;
        
        /* Convert source mbuf into NPE format. Virtual address must be used 
         * when flushing to external memory 
         */
        pQDesc->npeQDesc.srcMbufAddr = (UINT32)
            (ixCryptoUtilMbufToNpeFormatConvert (pSrcMbuf, 0));
          
        /* If operationMode is in place, pDestMbuf will be NULL and don't need 
         * to be written to the pQDesc or flushed to memory.
         */
        if (IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE != 
            (operationMode & IX_CRYPTO_NPE_OP_WEP_TRANSFER_MODE_MASK))
        {
            /* Store Dest Mbuf pointer into queue descriptor. This pointer will 
             * be used in QMgr callback for mbuf conversion and this pointer 
             * will be passed back to client as Dest Mbuf pointer
             */
            pQDesc->pDestMbuf = pDestMbuf;
            
            /* Convert destination mbuf into NPE format */
            pQDesc->npeQDesc.destMbufAddr = (UINT32)
                (ixCryptoUtilMbufToNpeFormatConvert (pDestMbuf, startOffset));
        }
        
        /* Offset and length of data 
         * Combined service for WEP does not require authStartOffset and
         * authLength to be set. These two fields are ignored as both
         * authentication and encryption share the same offset and data length.
         * Note : NPE Crypto Param is not used for WEP perform
         */
        pQDesc->npeQDesc.cryptStartOffset
            = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER (startOffset);

        pQDesc->npeQDesc.cryptLength
            = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER (dataLen);
       
        /* Flush Q descriptor message from cache */
        IX_CRYPTO_DATA_CACHE_FLUSH (pQDesc, sizeof (IxCryptoQDescriptor));
        
        /* translate pQDesc after flush */    
        pQDesc = (IxCryptoQDescriptor*) 
            IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (pQDesc);
    
        /* Enqueue request to hardware queue using QMgr */
        u32pQDesc = (UINT32) pQDesc;
        status = ixCryptoQAccessQueueWrite (IX_CRYPTO_ACC_WEP_REQ_Q,
                     &u32pQDesc);               
    
        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
        {
            ixCryptoCtx[cryptoCtxId].reqCount++;
        }
        else /* End of status check for Queue Write */ 
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "ixCryptoQAccessQueueWrite failed.\n", 
                0, 0, 0, 0, 0, 0);

            /* Convert Q descriptor address back to virtual address 
             * before releasing it
             */
            pQDesc = (IxCryptoQDescriptor *) 
                         IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION(
                         (UINT32) pQDesc);
                           
            /* Convert mbuf back to host format from NPE format */
            pSrcMbuf = ixCryptoUtilMbufFromNpeFormatConvert (
                           pQDesc->pSrcMbuf);
            pDestMbuf = ixCryptoUtilMbufFromNpeFormatConvert (
                            pQDesc->pDestMbuf);
            
            /* When the Queue write fails, pQDesc needs to be released */
            ixCryptoDescMgmtQDescriptorRelease(pQDesc);
     
        } /* End of if-else for Queue Write check */
        return status;
    }
    else /* end of if (IX_CRYPTO_IS_WEP_NPE_INITIALIZED) &&
        (IX_CRYPTO_IS_SERVICE_STOPPED) */
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "IxCrypto is not initialized or it is stopped.\n", 
            0, 0, 0, 0, 0, 0);   

        return IX_CRYPTO_ACC_STATUS_FAIL;
    }/* end of if-else (IX_CRYPTO_IS_WEP_NPE_INITIALIZED) &&
        (IX_CRYPTO_IS_SERVICE_STOPPED) */  
} /* end of ixCryptoAccWepNpePerform () function */
                 
                 
/**
 * @fn IxCryptoAccStatus ixCryptoAccXScaleWepPerform 
 * @brief Function to support ARC4 and WEP ICV computations on XScale.
 *
 * @note  A big amount of data will be pushed into stack while this function
 *        is called. This is because chunk of memory for sbox will need to be
 *        allocated for XScale assembly code to store the ARC4 computation
 *        state and this memory block need to be 512-byte aligned.
 * 
 */
PUBLIC IxCryptoAccStatus
ixCryptoAccXScaleWepPerform (
    UINT32 cryptoCtxId,
    IX_OSAL_MBUF *pSrcMbuf,
    IX_OSAL_MBUF *pDestMbuf,
    UINT16 startOffset,   
    UINT16 dataLen,
    UINT16 icvOffset,
    UINT8 *pKey)
{
    UINT32 wepOperation;
    IxCryptoAccStatus status;
    UINT8 sbox[IX_CRYPTO_MAX_SBOX_INFO_BYTES];
    UINT8 *pSbox = sbox;
    UINT8 cipherKey[IX_CRYPTO_MAX_ARC4_KEY_BYTES];
    UINT8 *pCipherKey = cipherKey;
    
    /* Check if the access component has been intialized and the crypto service
       is running */
    if (IX_CRYPTO_IS_SERVICE_INITIALIZED()
        && !IX_CRYPTO_IS_SERVICE_STOPPED())
    {
        /* Check the validity of CryptoCtxId */
        status = ixCryptoCCDMgmtCtxValidCheck (cryptoCtxId);
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "cryptoCtxId %d is not valid or registered.\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);

            return status;          
        }
        
        /* Check the operation requested. If the crypto context was not 
         * registered for WEP service, reject the service request
         */
        if (IX_CRYPTO_WEP_REQ != ixCryptoCtx[cryptoCtxId].reqType)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "cryptoCtxId %d is not registered for WEP service.\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;
        }
     
        /* Now we know CtxId valid, look up record for the WEP operation */
        wepOperation = (ixCryptoCtx[cryptoCtxId].npeOperationMode.npeOperation
                            & MASK_8_BIT);
                            
        /* Get transfer mode bit in opcode to determine whether the context 
         * is registered for in-place operation as non in-place operation is 
         * not supported in XScale assembly. If the context is registered for 
         * non in-place, return error.
         */
        if (IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE 
            != (wepOperation & IX_CRYPTO_NPE_OP_WEP_TRANSFER_MODE_MASK))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Non in-place operation is supported in XScale WEP perform.\n", 
                0, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;
        }
           
        /* Check for NULL MBuf, and the destination mbuf will not be checked 
         * since transfer mode is always in place */
        if  (NULL == pSrcMbuf)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Input Mbufs not valid. pSrcMbuf=0x%X\n",
                (int) pSrcMbuf,
                0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;       
        }        
    
#ifndef NDEBUG    
        /* Perform boundary check. 
         * The actual length of the mbuf will not be known. Only the length
         * of the data stored in a mbuf is known.
         */
        status = ixCryptoPerformWepBufferOffsetBoundaryCheck (
                     wepOperation,
                     pSrcMbuf,
                     pDestMbuf,
                     startOffset,
                     dataLen,
                     icvOffset,
                     ixCryptoCtx[cryptoCtxId].digestLength);
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            return status;
        } /* End of Boundary Check */       
#endif

        /* Mask the sbox address with lower 9-bit set to zero and point to 
         * next block memory to have the sbox aligned to 512-bytes boundary
         */ 
        pSbox = (UINT8 *)((IX_CRYPTO_ACC_512B_ALIGNED_ADDR_MASK 
                     & (UINT32) pSbox)
                     + IX_CRYPTO_SBOX_BLOCK_BYTES);

        /* Switch case based on WEP operation opcode.
         * Check if ARC4 per packet key is not NULL if ARC4 encryption is 
         * required. If NULL, return error.
         * Note: ARC4 per packet key need to be stored in memory with
         *       32 byte aligned.
         */        
        switch (wepOperation)
        {
            /* All cases fall through to same case statement */
            case IX_CRYPTO_NPE_OP_WEP_DECRYPT:
            case IX_CRYPTO_NPE_OP_WEP_ENCRYPT:
            case IX_CRYPTO_NPE_OP_WEP_VER_ICV_DECRYPT:
            case IX_CRYPTO_NPE_OP_WEP_ENCRYPT_GEN_ICV:

                /* First check ARC4 key not NULL */
                if (NULL == pKey)
                {   
                    /* Log error message in debugging mode */
                    IX_CRYPTO_ACC_LOG (
                        IX_OSAL_LOG_LVL_MESSAGE, 
                        IX_OSAL_LOG_DEV_STDOUT,
                        "Input ARC4 key not valid == NULL.\n", 
                        0, 0, 0, 0, 0, 0);
                    
                    return IX_CRYPTO_ACC_STATUS_FAIL;  
                } /* End of ARC4 NULL check */
                
                /* Mask the key address with lower 5-bit set to zero and point
                 * to the next block of memory to have the cipher key aligned 
                 * to 32-bytes boundary
                 */ 
                pCipherKey = (UINT8 *) ((IX_CRYPTO_ACC_32B_ALIGNED_ADDR_MASK 
                                 & (UINT32) pCipherKey)
                                 + IX_CRYPTO_ARC4_KEY_BLOCK_BYTES);
                
                /* copy ARC4 per packet key into 32-byte aligned memory */
                ixOsalMemCopy (pCipherKey, 
                    pKey, 
                    ixCryptoCtx[cryptoCtxId].cipherKeyLength);
                   
                break; 
        } /* end of switch case (wepOperation) */
         
        /* Increment Req counter in crypto context */
        ixCryptoCtx[cryptoCtxId].reqCount++;

        /* Call WEP XScale function to perform ARC4/WEP_CRC computation */
        status = ixCryptoPerformXScaleWepPerform (
                     wepOperation,
                     pSrcMbuf,
                     startOffset,
                     dataLen,
                     icvOffset,
                     pCipherKey,
                     pSbox);
                
#ifndef NDEBUG
        /* XSCale statistics collection lock, to protect critical section */
        IX_CRYPTO_ACC_LOCK (xscaleStatLock);

        /* Increment counters accordingly to the status returned */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
        {
            ixCryptoStats.wepXScaleSuccessCounter++;            
        }
        else /* failed or auth failed */
        {
            ixCryptoStats.wepXScaleFailCounter++;
        } /* end of if-else (status) */
        
        /* unlock */
        IX_CRYPTO_ACC_UNLOCK (xscaleStatLock);
#endif /* ndef NDEBUG */
           
        /* Increment req done counters in the same context, as the perform 
         * function is executed synchronously
         */
        ixCryptoCtx[cryptoCtxId].reqDoneCount++;
        
        return status;
    }
    else /* end of if (IX_CRYPTO_IS_SERVICE_INITIALIZED) &&
        (IX_CRYPTO_IS_SERVICE_STOPPED) */
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "IxCrypto is not initialized or it is stopped.\n", 
            0, 0, 0, 0, 0, 0);   

        return IX_CRYPTO_ACC_STATUS_FAIL;
    }/* end of if-else (IX_CRYPTO_IS_SERVICE_INITIALIZED) &&
        (IX_CRYPTO_IS_SERVICE_STOPPED) */  
} /* end of ixCryptoAccXScaleWepPerform() function */


/**
 * @fn ixCryptoAccCtxUnregister
 * @brief Unregister the crypto context from Cryptographic Context Database.
 *
 */
PUBLIC IxCryptoAccStatus
ixCryptoAccCtxUnregister (UINT32 cryptoCtxId)
{
    IxCryptoAccStatus status;

    /* Check if the access component has been intialized and the crypto 
     * service is running 
     */
    if (IX_CRYPTO_IS_SERVICE_INITIALIZED()
        && !IX_CRYPTO_IS_SERVICE_STOPPED())
    {
        /* Check if the Crypto Context ID provided points to a valid Crypto
         * Context 
         */
        status = ixCryptoCCDMgmtCtxValidCheck (cryptoCtxId);
        
        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status) 
        {
            /* Check if all requests from this cryptoCtxId have been
             * completed 
             */
            if (IX_CRYPTO_CTX_IS_ALL_TASK_DONE (
                ixCryptoCtx[cryptoCtxId].reqCount,
                ixCryptoCtx[cryptoCtxId].reqDoneCount))
            {
                /* Unregister the Crypto Context */
                return ixCryptoCCDMgmtCryptoCtxRelease (cryptoCtxId);              
            }
            else /* IX_CRYPTO_CTX_IS_ALL_TASK_DONE == FALSE */
            {
                /* There are requests pending in the queue for this Crypto 
                 * Context, cancel the unregistration and send Retry 
                 * message back to the client 
                 */
                return IX_CRYPTO_ACC_STATUS_RETRY;
            } /* end of if-else (IX_CRYPTO_CTX_IS_ALL_TASK_DONE) */
        }
        else /* Crypto context does not exist */
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Crypto Context %d does not exist.\n", 
                cryptoCtxId, 
                0, 0, 0, 0, 0);
                
            return status;
        } /*end of if (status) for (ixCryptoCCDMgmtCtxValidCheck) */
    }
    else /* (IX_CRYPTO_IS_SERVICE_INITIALIZED) &&
          * (!IX_CRYPTO_IS_SERVICE_STOPPED) 
          */
    {
        /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "Component has not been initialized\n", 
                0, 0, 0, 0, 0, 0);
                
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (IX_CRYPTO_IS_SERVICE_INITIALIZED) &&
       * (!IX_CRYPTO_IS_SERVICE_STOPPED) 
       */
} /* End of ixCryptoAccCtxUnregister () */


/**
 * @fn   ixCryptoAccCryptoServiceStop
 * @brief API to stop the security hardware accelerator services.
 *
 * @note THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.
 *       It will be removed in a future release.
 *       See API header file IxCryptoAcc.h for more information.  
 */
PUBLIC IxCryptoAccStatus
ixCryptoAccCryptoServiceStop (void)
{
    UINT32 timer = 0;
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_SUCCESS;
        
    ixCryptoAccServiceStop = TRUE;          /* Flag to decline NPE request */
    
#ifdef __ixp46X     /* PKE codes only applicable for IXP46X platform */

    ixCryptoAccPkeCryptoEngineInit = FALSE; /* Flag to decline PKE request */
    
    /* Uninit PKE Crypto Engine module */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoPkeUninit ())
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;  /* Unable to uninitialise the PKE */
    }

#endif /* ixp46X */
    
    while (IX_CRYPTO_ACC_STATUS_SUCCESS 
        != ixCryptoDescMgmtAllQDescriptorInPoolCheck())
    {
        /* Put the task into sleep mode and wait for the NPE to complete
         * all the tasks in the queue 
         */
        ixOsalSleep(IX_CRYPTO_ACC_DELAY_IN_MS);

        /* Timer to avoid infinite lock */
        timer++;
        if (IX_CRYPTO_ACC_TIMER_COUNT < timer )
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_FATAL,
                IX_OSAL_LOG_DEV_STDERR,
                "Stop function failed!\n", 
                0, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;
    
        } /* End of if timer loop */

    } /* end of while (ixCryptoDescMgmtAllQDescriptorInPoolCheck) */
    
    /* Release all Crypto Contexts and this will not fail */
    ixCryptoCCDMgmtCryptoCtxReleaseAll ();

    /* Release memeory pool allocated to NPE Crypto Param */
    ixCryptoCCDMgmtNpeCryptoParamPoolFree ();
    
    /* Release memory pool allocated to Desc pool */
    ixCryptoDescMgmtDescPoolFree ();
   
    return status;
} /* end of ixCryptoAccCryptoServiceStop () function */



/**
 * @fn   ixCryptoAccShow
 * @brief API for printing statistic and status.
 *
 */
PUBLIC void
ixCryptoAccShow (void)
{
    /* Show request queue and done queue status */
    ixCryptoQAccessQueueStatusShow (IX_CRYPTO_ACC_CRYPTO_REQ_Q); 
    ixCryptoQAccessQueueStatusShow (IX_CRYPTO_ACC_CRYPTO_DONE_Q);
    ixCryptoQAccessQueueStatusShow (IX_CRYPTO_ACC_WEP_REQ_Q); 
    ixCryptoQAccessQueueStatusShow (IX_CRYPTO_ACC_WEP_DONE_Q);
       
    /* To show the number of descriptors have been used, and the pool 
     * usage
     */
    ixCryptoDescMgmtShow ();

    /* To show number of Crypto Contexts have been registered */
    ixCryptoCCDMgmtShow ();

#ifdef __ixp46X     /* PKE codes only applicable for IXP46X platform */    

    /* To show number of requests on PKE Crypto Engine */
    ixCryptoPkeEauShow (); 
    ixCryptoPkeHashShow (); 

#endif /* ixp46X */

} /* end of ixCryptoAccShow () function */


/**
 * @fn   ixCryptoAccShowWithId
 * @brief API for printing statistic and status for particular Context.
 *
 */

PUBLIC void
ixCryptoAccShowWithId (UINT32 cryptoCtxId)
{
    if (IX_CRYPTO_ACC_STATUS_SUCCESS == 
    ixCryptoCCDMgmtCtxValidCheck (cryptoCtxId))
    {
        /* Show request queue and done queue status, number of descriptors have 
         * been used, and the pool usage and show number of Crypto Contexts
         * that have been registered */
        ixCryptoAccShow ();

        /* To show contents of Crypto Context which has been registered */
        ixCryptoCCDMgmtCryptoCtxShow (cryptoCtxId);
    }
    else
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Invalid cryptoCtxId.\n", 
            0, 0, 0, 0, 0, 0);
    } /* End of if-else Ctx Check */
    
    
} /* end of ixCryptoAccShowWithId () */



/**
 * @fn ixCryptoAccHashKeyGenerate
 *
 * @brief This function is used to generate authentication key needed in HMAC
 *        authentication if the authentication key is greater than 64 bytes.
 *        New authentication key of L bytes size will be generated in this 
 *        function (L = 20 for SHA1, L = 16 for MD5). Please refer to RFC2104 
 *        for more details on the key size. The authentication key is padded 
 *        (extended) so that its length (in bits) is congruent to 448, 
 *        modulo 512 (please refer to RFC1321). 
 *        This function has been extended to become a generic hashing function
 *        other than above mentioned functionality. It could be used to hash 
 *        any key size or data that are greater than 0 and less than 65399 
 *        bytes
 *
 * @note  
 * 1. This API will need to be called if the authentication key > 64 bytes. If
 *    key size <= 64 bytes, authentication key is used directly in Crypto
 *    context for registration, no further hashing is needed.
 * 2. This function must be called prior the Crypto Context registration if 
 *    the key size > 64. Result from this function will be used as 
 *    authentication key in the Crypto Context registration.
 * 3. It is client's responsibility to ensure the mbuf is big enough to
 *    hold the result (generated authentication key). No checking on mbuf 
 *    length will be done.
 *
 */
PUBLIC IxCryptoAccStatus
ixCryptoAccHashKeyGenerate (
    IxCryptoAccAuthAlgo hashAlgo,
    IX_OSAL_MBUF *pMbufHashKey,
    IxCryptoAccHashKeyGenCompleteCallback hashKeyCallbackFn,
    UINT16 hashKeyStartOffset,
    UINT16 hashKeyLen,
    UINT16 hashKeyDestOffset,
    UINT32 *pHashKeyId)
{
    union
    {
        IxCryptoNpeHashCfgWord hashCfgWord;
        UINT32 npeHashCfgWord;
    } cfgWord;

    UINT32 npeHashConfigWord;
    UINT8 *pHashKey;
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_SUCCESS;
    IxCryptoQDescriptor *pQDesc = NULL;
    UINT32 initLength;
    UINT32 destAddr;
    UINT32 padLength;
    UINT32 lengthInBit;
    UINT32 i;
    UINT8 md5ChainVar[IX_CRYPTO_NPE_CRYPTO_PARAM_MD5_CHAIN_VAR_LEN] =
        MD5_CHAIN_VAR;
    UINT8 sha1ChainVar[IX_CRYPTO_NPE_CRYPTO_PARAM_SHA1_CHAIN_VAR_LEN] =
        SHA1_CHAIN_VAR;
    UINT32 u32pQDesc;

    /* Check if the mbuf and hashKeyID pointer are NULL,
     * return error if either one is NULL
     */
    if ((NULL == pMbufHashKey) || (NULL == pHashKeyId))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Mbuf is %x\n pHashKeyId is %x\n",
            (int) pMbufHashKey,
            (int) pHashKeyId,
            0, 0, 0, 0);

        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (pMbufHashKey) || (pHashKeyId) */

    /* Check if the mbuf provided is chained mbufs */
    if (NULL != IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pMbufHashKey))
    {
        /* chained mbuf, return  error */
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pMbufHashKey) */

    /* Check if the callback function provided by client is NULL, return
     * error if NULL
     */
    if (NULL == hashKeyCallbackFn)
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Hash key Callback: %x\n",
            (int) hashKeyCallbackFn,
            0,0, 0, 0, 0);

        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (hashKeyCallbackFn) */

    /* Check if the access component has been intialized and the crypto
     * service is running
     */
    if (IX_CRYPTO_IS_HW_ACCL_INITIALIZED()
        && !IX_CRYPTO_IS_SERVICE_STOPPED())
    {
        /* Check hash key length */
        if (IX_CRYPTO_ACC_INVALID_HASH_DATA_LEN == hashKeyLen
            || IX_CRYPTO_ACC_MAX_HASH_DATA_LEN < hashKeyLen)
        {
            /* invalid key length / hash data length */
            return IX_CRYPTO_ACC_STATUS_AUTH_INVALID_KEY_LEN;
        }

        /* get mData pointer of mbuf */
        pHashKey = IX_OSAL_MBUF_MDATA (pMbufHashKey);

        /* Clear NPE hash config word to zero */
        cfgWord.npeHashCfgWord = 0;
        
        /* Hash algorithm switch case options
         * For each case of valid hash algorithm, hash config will be created.
         * Hash key will be padded to multiple of 512-bit according to the 
         * padding scheme in RFC1321 (MD5 hash algorithm) and RFC3174 (SHA1
         * hash algorithm).
         */
        switch (hashAlgo)
        {
            /* SHA1 algorithm */
            case IX_CRYPTO_ACC_AUTH_SHA1:

                /* Set authentication digest length to key length */
                cfgWord.hashCfgWord.digestLength
                    = (IX_CRYPTO_ACC_SHA1_KEY_160 / WORD_IN_BYTES);

                /* Set authentication algorithm */
                cfgWord.hashCfgWord.hashAlgo = IX_CRYPTO_NPE_HASH_CFG_SHA1;

                /* Set chaining variable length */
                cfgWord.hashCfgWord.CVLength
                    = (IX_CRYPTO_NPE_CRYPTO_PARAM_SHA1_CHAIN_VAR_LEN /
                        WORD_IN_BYTES);

                /* Init length for chaining variables computation is the length
                 * of hash config word plus length of primary chaining variable
                 */
                initLength = IX_CRYPTO_NPE_CRYPTO_PARAM_HASH_CFG_WORD_LEN
                                + IX_CRYPTO_NPE_CRYPTO_PARAM_SHA1_CHAIN_VAR_LEN;
                                
                /* Configure endian mode of NPE, by default is Big endian.
                 * No swapping is required
                 */
                cfgWord.hashCfgWord.chnWr = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
                cfgWord.hashCfgWord.chnRd = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
                cfgWord.hashCfgWord.hdWr = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
                cfgWord.hashCfgWord.hdRd = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
                
                /* Do padding, if the hash key is not congruent 448 bit 
                 * (56 bytes), modulo 512-bit (64 bytes). 
                 */
                if (IX_CRYPTO_ACC_HASH_DATA_REMAINDER ==
                    (hashKeyLen % IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH))
                {
                    /* length of original hash key in bits */
                    lengthInBit = hashKeyLen * BITS_IN_BYTE;
                    
                    /* pad hash key with value 0x80 */
                    ixOsalMemSet (&pHashKey[hashKeyStartOffset + hashKeyLen], 
                        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE, 
                        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN);
                    
                    /* set total bit (high 32-bit) to zero, key size will
                     * not exceed 2^32
                     */
                    ixOsalMemSet (
                        &pHashKey[hashKeyStartOffset + hashKeyLen
                        + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN], 
                        0, 
                        IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN);
                    
                    /* set total bit field (low 32-bit) */
                    for (i = 0; i < IX_CRYPTO_ACC_HASH_TOTAL_BIT_LOW_LEN; i++)
                    {
                        pHashKey[hashKeyStartOffset + hashKeyLen 
                            + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN
                            + IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN + i] 
                            = (lengthInBit >> ((MSB_BYTE - i) * BITS_IN_BYTE) 
                   & MASK_8_BIT);
                    }
                    
                    /* total hash key length after padding */
                    hashKeyLen = hashKeyLen 
                                 + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LEN
                                 + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN;
                }
                else
                {
                    /* length of original hash key in bits */
                    lengthInBit = hashKeyLen * BITS_IN_BYTE;
                    
                    /* calculate zero padding length (bytes) for original 
                     * hash key to make it multiple of 512-bit
                     */
                    padLength = IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH -
                                ((hashKeyLen + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LEN
                                + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN) % 
                                IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH);
                    
                    /* pad hash key with value 0x80 */
                    ixOsalMemSet (&pHashKey[hashKeyStartOffset + hashKeyLen], 
                        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE, 
                        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN);
                        
                    /* pad original hash key with 0s at the back */
                    ixOsalMemSet (
                        &pHashKey[hashKeyStartOffset + hashKeyLen 
                        + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN], 
                        0, padLength);
                    
                    /* set total bit (high 32-bit) to zero, key size will
                     * not exceed 2^32
                     */
                    ixOsalMemSet (&pHashKey[hashKeyStartOffset + hashKeyLen
                        + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN
                        + padLength], 
                        0, 
                        IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN);
                    
                    /* set total bit field (low 32-bit) */
                    for (i = 0; i < IX_CRYPTO_ACC_HASH_TOTAL_BIT_LOW_LEN; i++)
                    {
                        pHashKey[hashKeyStartOffset + hashKeyLen 
                            + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN
                            + padLength 
                            + IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN
                            + i] 
                            = (lengthInBit >> ((MSB_BYTE - i) * BITS_IN_BYTE) 
                                  & MASK_8_BIT);
                    }
                    
                    /* total hash key length after padding */
                    hashKeyLen = hashKeyLen 
                                 + padLength
                                 + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LEN
                                 + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN;
                }
                
                break;

            /* MD5 algorithm */
            case IX_CRYPTO_ACC_AUTH_MD5:

                /* Set authentication digest length */
                cfgWord.hashCfgWord.digestLength
                    = (IX_CRYPTO_ACC_MD5_KEY_128 / WORD_IN_BYTES);

                /* Set authentication algorithm */
                cfgWord.hashCfgWord.hashAlgo = IX_CRYPTO_NPE_HASH_CFG_MD5;

                /* Set chaining variable length */
                cfgWord.hashCfgWord.CVLength
                    = (IX_CRYPTO_NPE_CRYPTO_PARAM_MD5_CHAIN_VAR_LEN /
                        WORD_IN_BYTES);

                /* Init length for chaining variables computation is the length
                 * of hash config word plus length of primary chaining variable
                 */
                initLength = IX_CRYPTO_NPE_CRYPTO_PARAM_HASH_CFG_WORD_LEN
                                + IX_CRYPTO_NPE_CRYPTO_PARAM_MD5_CHAIN_VAR_LEN;
                                
                /* Configure endian mode of NPE, MD5 hashing co-processor
                 * operate in Little Endian, byte swapping is required
                 */
                cfgWord.hashCfgWord.chnWr = IX_CRYPTO_NPE_CFG_BYTE_SWAP;
                cfgWord.hashCfgWord.chnRd = IX_CRYPTO_NPE_CFG_BYTE_SWAP;
                cfgWord.hashCfgWord.hdWr = IX_CRYPTO_NPE_CFG_BYTE_SWAP;
                cfgWord.hashCfgWord.hdRd = IX_CRYPTO_NPE_CFG_BYTE_SWAP;
                
                /* Do padding, if the hash key is not congruent 448 bit 
                 * (56 bytes), modulo 512-bit (64 bytes). 
                 */
                if (IX_CRYPTO_ACC_HASH_DATA_REMAINDER == 
                    (hashKeyLen % IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH))
                {
                    /* length of original hash key in bits */
                    lengthInBit = hashKeyLen * BITS_IN_BYTE;
                    
                    /* pad hash key with value 0x80 */
                    ixOsalMemSet (&pHashKey[hashKeyStartOffset + hashKeyLen], 
                        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE, 
                        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN);
                        
                    /* set total bit field (low 32-bit) 
                     * Note :  MD5 operate in little endian, byte swap 
                     * needed while setting the total bit field
                     */
                    for (i = 0; i < IX_CRYPTO_ACC_HASH_TOTAL_BIT_LOW_LEN; i++)
                    {
                        pHashKey[hashKeyStartOffset + hashKeyLen + 
                            IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN +i] 
                                = (lengthInBit >> (i * BITS_IN_BYTE) 
                                      & MASK_8_BIT);
                    }
                    
                    /* set total bit (high 32-bit) to zero, key size will
                     * not exceed 2^32
                     */
                    ixOsalMemSet (
                        &pHashKey[hashKeyStartOffset + hashKeyLen
                        + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN
                        + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LOW_LEN], 
                        0, 
                        IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN);
                    
                    /* total hash key length after padding */
                    hashKeyLen = hashKeyLen 
                                 + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LEN
                                 + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN;
                }
                else
                {
                    /* length of original hash key in bits */
                    lengthInBit = hashKeyLen * BITS_IN_BYTE;
                    
                    /* calculate zero padding length (bytes) for original 
                     * hash key to make it multiple of 512-bit
                     */
                    padLength = IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH -
                                ((hashKeyLen + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LEN
                                + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN) % 
                                IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH);
                    
                    /* pad hash key with value 0x80 */
                    ixOsalMemSet (&pHashKey[hashKeyStartOffset + hashKeyLen], 
                        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE, 
                        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN);
                    
                    /* pad original hash key with 0s at the back */
                    ixOsalMemSet (
                        &pHashKey[hashKeyStartOffset + hashKeyLen 
                        + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN], 
                        0, 
                        padLength);
                    
                    /* set total bit field (low 32-bit) 
                     * Note :  MD5 operate in little endian, byte swap 
                     * needed while setting the total bit field
                     */
                    for (i = 0; i < IX_CRYPTO_ACC_HASH_TOTAL_BIT_LOW_LEN; i++)
                    {
                        pHashKey[hashKeyStartOffset + hashKeyLen + 
                            IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN 
                            + padLength +i] 
                                = (lengthInBit >> (i * BITS_IN_BYTE) 
                                      & MASK_8_BIT);
                    }
                    
                    /* set total bit (high 32-bit) to zero, key size will
                     * not exceed 2^32
                     */
                    ixOsalMemSet (
                        &pHashKey[hashKeyStartOffset + hashKeyLen
                        + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN
                        + padLength 
                        + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LOW_LEN],
                        0, 
                        IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN);
                    
                    /* total hash key length after padding */
                    hashKeyLen = hashKeyLen 
                                 + padLength
                                 + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LEN
                                 + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN;
                }
                
                break;

            /* invalid authentication algorithm */
            default:
                return IX_CRYPTO_ACC_STATUS_AUTH_ALGO_NOT_SUPPORTED;

        } /* end of switch case (hashAlgo) */

        /* Get Crypto Param for hash key generation */
        status = ixCryptoCCDMgmtKeyCryptoParamGet (pHashKeyId);

        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
        {
            /* Store callback function pointers into Crypto Context */
            ixKeyCryptoParam[*pHashKeyId].hashKeyCallbackFn
                = hashKeyCallbackFn;
        }
        else /* No Crypto Context is allocated */
        {
            return status;
        } /* end of if-else (status) */

        /* Generate NPE Hash Config word in NPE format (Big Endian) */
        ixCryptoUtilNpeHashCfgGenerate (
            &npeHashConfigWord, 
            &cfgWord.hashCfgWord);

        /* Copy Hash Config word into NPE Crypto Param structure */
        for ( i = 0;
            i < IX_CRYPTO_NPE_CRYPTO_PARAM_HASH_CFG_WORD_LEN;
            i++)
        {
            /* Shift Hash Config word into the array of UINT8,
             * most significant byte is shifted into the array with
             * smaller index.
             * Ex :
             *   -------------------
             *  | b0 | b1 | b2 | b3 |
             *   -------------------
             *  array[n] = b0
             *  array[n+1] = b1
             */
            ixKeyCryptoParam[*pHashKeyId].pNpeCryptoParam->npeCryptoInfo[i]
                = ((npeHashConfigWord >> ((MSB_BYTE - i) * BITS_IN_BYTE))
                  & MASK_8_BIT);
        } /* end of for (i) */

        /* Log NPE Crypt Config word in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Hash Cfg word is %x\n",
            npeHashConfigWord,
            0, 0, 0, 0, 0);
        
        /* Initialize NPE crypto param structure with algorithm dependent
         * initial chaining variables value
         */
        if (IX_CRYPTO_ACC_AUTH_MD5 == hashAlgo)
        {
            /* Copy MD5 initial chaining variable value into NPE Crypto
             * param structure
             */
            ixOsalMemCopy (&(ixKeyCryptoParam[*pHashKeyId].pNpeCryptoParam->
                npeCryptoInfo[IX_CRYPTO_NPE_CRYPTO_PARAM_HASH_CFG_WORD_LEN]),
                md5ChainVar,
                IX_CRYPTO_NPE_CRYPTO_PARAM_MD5_CHAIN_VAR_LEN);
        }
        else /* IX_CRYPTO_ACC_AUTH_SHA1 */
        {
            /* Copy SHA1 initial chaining variable value into NPE Crypto
             * param structure
             */
            ixOsalMemCopy (&(ixKeyCryptoParam[*pHashKeyId].pNpeCryptoParam->
                npeCryptoInfo[IX_CRYPTO_NPE_CRYPTO_PARAM_HASH_CFG_WORD_LEN]),
                sha1ChainVar,
                IX_CRYPTO_NPE_CRYPTO_PARAM_SHA1_CHAIN_VAR_LEN);
        } /* end of if-else (hashAlgo) */

        /* Flush NPE Crypto Param structure from cache into SDRAM */
        IX_CRYPTO_DATA_CACHE_FLUSH (
            ixKeyCryptoParam[*pHashKeyId].pNpeCryptoParam,
            IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);
        
        /* Get a free Q descriptor from Q descriptors pool
         * Notes: Q descriptor is pushed back to the descriptor pool if the
         *        queue write operation failed. If the queue write operation
         *        pass, the Q descriptor will be processed by NPE and being
         *        release in QMgr dispatcher context through registered
         *        ixCryptoQAccessReqDoneQMgrCallback function
         */
        status =ixCryptoDescMgmtQDescriptorGet (&pQDesc);

        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
        {
            /* Store hashKeyID & operStatus for the use in QMGr callback */
            pQDesc->value.keyId = *pHashKeyId;
            pQDesc->operStatus = IX_CRYPTO_OP_HASH_GEN_KEY;

            /* Get destination address */
            destAddr = ixCryptoUtilMbufOffsetToAddressConvert (
                           pMbufHashKey,
                           hashKeyDestOffset,
                           FALSE);

            /* Convert destination address to physical address
               and swap to network byte order */
            pQDesc->npeQDesc.address.icvAddr
                = (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (
                      destAddr);
 
            pQDesc->npeQDesc.address.icvAddr
                = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER (
                      pQDesc->npeQDesc.address.icvAddr);  
            
            /* Store Mbuf pointer into queue descriptor. This pointer will be
             * used in QMgr callback for mbuf conversion and this pointer will 
             * be passed back to client as Src Mbuf pointer
             */
            pQDesc->pSrcMbuf = pMbufHashKey;

            /* Convert mbuf into NPE format */
            pQDesc->npeQDesc.srcMbufAddr
                = (UINT32) ixCryptoUtilMbufToNpeFormatConvert(pMbufHashKey, 0);

            /* Set to chaining variables generation operation */
            pQDesc->npeQDesc.npeOperationMode.npeOperation
                = IX_CRYPTO_NPE_OP_HASH_GEN_ICV;

            /* Set length of initial values in Crypto Param structure */
            pQDesc->npeQDesc.npeOperationMode.initLength = initLength;

            /* Offset and length of data */
            pQDesc->npeQDesc.authStartOffset 
                 = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER (hashKeyStartOffset);

            pQDesc->npeQDesc.authLength 
                 = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER (hashKeyLen);

            /* Convert virtual address to physical address, NPE only takes
             * physical address
             */

            /* NPE Crypto Param structure start address */
            pQDesc->npeQDesc.cryptoCtxNpeAddr
                = (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION
                  ((UINT32) ixKeyCryptoParam[*pHashKeyId].pNpeCryptoParam
                  ->npeCryptoInfo);

            pQDesc->npeQDesc.cryptoCtxNpeAddr 
                = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER 
                      (pQDesc->npeQDesc.cryptoCtxNpeAddr);

            /* Flush Q descriptor message from cache */
            IX_CRYPTO_DATA_CACHE_FLUSH (pQDesc, sizeof (IxCryptoQDescriptor));

            /* Convert Q descriptor address to physical address */
            pQDesc = (IxCryptoQDescriptor *)
                         IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION(
                         (UINT32) pQDesc);

            /* Enqueue request to hardware queue */
            u32pQDesc = (UINT32) pQDesc;
            status = ixCryptoQAccessQueueWrite (
                         IX_CRYPTO_ACC_CRYPTO_REQ_Q,
                         &u32pQDesc);

            /* Check if operation enqueued successfully */
            if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
            {
                /* Relaese NPE crypto param back to the pool */
                ixCryptoCCDMgmtKeyCryptoParamRelease (*pHashKeyId);

                /* Convert Q descriptor address back to virtual address
                 * before releasing it
                 */
                pQDesc = (IxCryptoQDescriptor *)
                             IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION(
                             (UINT32) pQDesc);

                /* Convert mbuf back to host format from NPE format */
                pMbufHashKey = ixCryptoUtilMbufFromNpeFormatConvert (
                                   pQDesc->pSrcMbuf);

                /* Release Q descriptor back to the descriptor pool */
                ixCryptoDescMgmtQDescriptorRelease (pQDesc);
            }/* end of if-else (status) */
        } /* end of if (status) for (ixCryptoDescMgmtQDescriptorGet) */

        return status;
    }
    else /* if (IX_CRYPTO_IS_HW_ACCL_INITIALIZED() &&
          * (!IX_CRYPTO_IS_SERVICE_STOPPED)
          */
    {

        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "Component has not been initialized\n",
            0, 0, 0, 0, 0, 0);

        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (IX_CRYPTO_IS_HW_ACCL_INITIALIZED() &&
       * (!IX_CRYPTO_IS_SERVICE_STOPPED)
       */
} /* end of ixCryptoAccHashKeyGenerate () function */


/**
 * @fn   ixCryptoAccCtxCipherKeyUpdate
 * @brief The function allows the caller to the change the keys
 * of a context registered for CCM processing. The caller of this
 * function has to make sure that there are no pending requests 
 * on the context.
 */

PUBLIC IxCryptoAccStatus 
ixCryptoAccCtxCipherKeyUpdate (
    UINT32 cryptoCtxId,
    UINT8 *pCipherKey)
{
   UINT32 operationMode;
   UINT32 npeOperation;
   IxCryptoAccStatus status;

   if( IX_CRYPTO_IS_HW_ACCL_INITIALIZED()
        && !IX_CRYPTO_IS_SERVICE_STOPPED())
    {
       /* Check the validity of CryptoCtxId */
        status = ixCryptoCCDMgmtCtxValidCheck (cryptoCtxId);
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "cryptoCtxId %d is not valid or registered.\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);

            return status;          
        }

        /* Check whether the input pointer is NULL */
        if( pCipherKey == NULL)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Input cipherKey ptr is NULL", 
                0, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_FAIL;          
            
        }

        /* Check if the crypto context is registered for doing HW ACC service.
         */
        if (IX_CRYPTO_HW_ACCL_REQ != ixCryptoCtx[cryptoCtxId].reqType)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "cryptoCtxId %d is not registered for CRYPTO NPE service.\n", 
                cryptoCtxId, 0, 0, 0, 0, 0);

            return IX_CRYPTO_ACC_STATUS_OPERATION_NOT_SUPPORTED;
        }
     
        operationMode = (ixCryptoCtx[cryptoCtxId].npeOperationMode.npeOperation
                            & MASK_8_BIT);

        /* Get NPE operation */
        npeOperation = operationMode & (~IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK);

        /* Update the keys only if the context is registered to perform 
         * CCM operations.
         */
        switch (npeOperation)
        {
            case IX_CRYPTO_NPE_OP_CCM_GEN_MIC:
            case IX_CRYPTO_NPE_OP_CCM_VER_MIC:

                /* Update the keys for the context */       
                ixOsalMemCopy( &(ixCryptoCtx[cryptoCtxId].pNpeCryptoParam->
                npeCryptoInfo[IX_CRYPTO_NPE_CRYPT_PARAM_CIPHER_KEY_START_OFFSET]),
                         pCipherKey,
                         ixCryptoCtx[cryptoCtxId].cipherKeyLength);
                
                /* Flush NPE Crypto Param structure from cache into SDRAM */
                IX_CRYPTO_DATA_CACHE_FLUSH (
                    ixCryptoCtx[cryptoCtxId].pNpeCryptoParam,
                    IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);
                
                break;
            default:
                 return IX_CRYPTO_ACC_STATUS_OPERATION_NOT_SUPPORTED; 
        }

    }else /* (IX_CRYPTO_IS_HW_ACCL_INITIALIZED) &&
           * (!IX_CRYPTO_IS_SERVICE_STOPPED)
           */
    {

        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "Component has not been initialized\n",
            0, 0, 0, 0, 0, 0);

        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (IX_CRYPTO_IS_HW_ACCL_INITIALIZED) &&
       * (!IX_CRYPTO_IS_SERVICE_STOPPED)
       */

    return IX_CRYPTO_ACC_STATUS_SUCCESS;
}

#ifdef __ixp46X     /* PKE codes only applicable for IXP46X platform */

/**
 * @fn IxCryptoAccStatus ixCryptoAccPseudoRandomNumberGet (
    UINT32 pseudoRandomNumberLen,
    UINT32 *pPseudoRandomNumber)
 * @brief This function will return the pseudo-random number generated in  
 *        hardware to the client via the data pointer supplied. The length of 
 *        pseudo-random number requested must be in words count 
 *        (pseudoRandomNumberLen >= 1 word). If bigger pseudo-random number is 
 *        requested, then this function will take longer time to complete.
 *
 */
PUBLIC IxCryptoAccStatus
ixCryptoAccPkePseudoRandomNumberGet (
    UINT32 pseudoRandomNumberLen,
    UINT32 *pPseudoRandomNumber)
{
    UINT32 i = 0;
    IxOsalFastMutex *pPkeRngMutex;
    UINT32 pkeRngAddress;
        
    /* Check if length is bigger than 0 */
    if (0 == pseudoRandomNumberLen)
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Length of pseudo-random number has to be greater than 0. \n", 
            0, 0, 0, 0, 0, 0);
    
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    } /*  end of if ((0 == pseudoRandomNumberLen) */

    /* Check if data pointer is NULL */
    if (NULL == pPseudoRandomNumber)
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Data pointer point to NULL.\n", 
            0, 0, 0, 0, 0, 0);
        
        return IX_CRYPTO_ACC_STATUS_NULL_PTR_ERR;
    } /* end of if (NULL == pPseudoRandomNumber) */
    
    /* Check if PKE engine is exist and initialized */
    if (!IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED())
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "PKE engine does not exist or is not initialized.",
            0, 0, 0, 0, 0, 0);
            
         return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (!IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED()) */
    
    pPkeRngMutex = ixCryptoPkeFastMutexGet (IX_CRYPTO_PKE_RNG);
    pkeRngAddress = ixCryptoPkeAddressGet (IX_CRYPTO_PKE_RNG);
            
    /* Try to lock mutex. If try lock fails, which mean RNG engine is busy
     * now, client need to retry later */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS 
        != IX_CRYPTO_ACC_FAST_MUTEX_TRY_LOCK (pPkeRngMutex))
    {
        /* Log message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "PKE RNG engine is busy.\n", 
            0, 0, 0, 0, 0, 0);
        
        return IX_CRYPTO_ACC_STATUS_RETRY;
    } /* end of if (IX_CRYPTO_ACC_STATUS_SUCCESS != status) */

    /* Read RNG FIFO */
    for (i = 0; i < pseudoRandomNumberLen; i++){
        *pPseudoRandomNumber = IX_CRYPTO_PKE_READ_LONG(pkeRngAddress);
        pPseudoRandomNumber++;
    } /* end of for (i=0; i<pseudoRandomNumberLen; i++) */

    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        IX_CRYPTO_ACC_FAST_MUTEX_UNLOCK (pPkeRngMutex))
    {
        /* Log message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Failed to unlock mutex for RNG.\n", 
            0, 0, 0, 0, 0, 0);
                     
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        IX_CRYPTO_ACC_MUTEX_UNLOCK (&rngLock)) */
        
    return IX_CRYPTO_ACC_STATUS_SUCCESS;

} /* end of ixCryptoAccPkePseudoRandomNumberGet() function */


/**
 * @fn IxCryptoAccStatus ixCryptoAccEauPerform (
    IxCryptoAccEauOperation operation,
    IxCryptoAccEauInOperands *pOpr,
    IxCryptoAccEauPerformCompleteCallback eauPerformCallbackFn,
    IxCryptoAccEauOpResult *pResult)
 * @brief This function is used to performed large number exponential 
 *        arithmetic operation requested by client by using EAU 
 *        (Exponentiation Acceleration Unit) on PKE (Public Key Exchange) 
 *        bridge. The result computed will be returned via callback function 
 *        supplied by client. All the operand size must be specified in words
 *        (32-bit), the output data pointer must be big enough to hold the 
 *        result.
 *
 */
PUBLIC IxCryptoAccStatus 
ixCryptoAccPkeEauPerform (
    IxCryptoAccPkeEauOperation operation,
    IxCryptoAccPkeEauInOperands *pOpr,
    IxCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn,
    IxCryptoAccPkeEauOpResult *pResult)
{
    IxOsalFastMutex *pPkeEauMutex;
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_SUCCESS;

    /* Check if data pointer or callback funtion pointer is NULL*/
    if (NULL == eauPerformCallbackFn || NULL == pResult)
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Result pointer point to NULL or callback function is NULL.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_NULL_PTR_ERR;
    } /* end of if (NULL == eauPerformCallbackFn || NULL == pResult) */
    
    /* Check if PKE engine is exist and initialized */
    if (!IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED())
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "PKE engine does not exist or is not initialized.",
            0, 0, 0, 0, 0, 0);
            
         return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (!IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED()) */
    
    /* Obtain EAU Mutex */    
    pPkeEauMutex = ixCryptoPkeFastMutexGet (IX_CRYPTO_PKE_EAU);
    
    /* Try to lock mutex. If try lock fails, which mean EAU engine is busy
     * now, client need to retry later */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        IX_CRYPTO_ACC_FAST_MUTEX_TRY_LOCK (pPkeEauMutex))
    {
        /* Log message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "PKE EAU engine is busy.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_RETRY;
    } /* end of if (IX_CRYPTO_ACC_FAST_MUTEX_TRY_LOCK) */
      
    /* check which operation to be performed */
    switch (operation)
    {
        /* Modular exponential operation */
        case IX_CRYPTO_ACC_OP_EAU_MOD_EXP:
            status = ixCryptoPkeEauModExp (
                         pOpr, 
                         pResult, 
                         eauPerformCallbackFn);
            break;
            
        /* Multiplication operation. Note: the code fall through to
         * case IX_CRYPTO_ACC_OP_EAU_BN_SUB */
        case IX_CRYPTO_ACC_OP_EAU_BN_MUL:     
        
        /* Addtion operation. Note: the code fall through to
         * case IX_CRYPTO_ACC_OP_EAU_BN_SUB */
        case IX_CRYPTO_ACC_OP_EAU_BN_ADD:
        
        /* Subtraction operation. Note: the code fall through to
         * case IX_CRYPTO_ACC_OP_EAU_BN_SUB */
        case IX_CRYPTO_ACC_OP_EAU_BN_SUB:
            status = ixCryptoPkeEauBNMulAddSub (
                         pOpr, 
                         pResult, 
                         operation, 
                         eauPerformCallbackFn);
            break;

        /* Modular reduction operation*/
        case IX_CRYPTO_ACC_OP_EAU_BN_MOD:
            status = ixCryptoPkeEauBNMod (
                         pOpr, 
                         pResult, 
                         eauPerformCallbackFn);
            break;

        default:
            status = IX_CRYPTO_ACC_STATUS_OPERATION_NOT_SUPPORTED;                        
    } /* end of switch statement */     
        
    /* If any of the operation above fail, the mutex locked at the beginning of 
     * code should be released. If the operation above return success status,
     * the mutex will be released in EAU done interrupt callback after the 
     * operation is complete.
     */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
    {
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
            IX_CRYPTO_ACC_FAST_MUTEX_UNLOCK (pPkeEauMutex)) 
        {
            /* Log message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Cannot unlock fast mutex.\n", 
                0, 0, 0, 0, 0, 0); 
        }
    } /* end of if(IX_CRYPTO_ACC_STATUS_SUCCESS != status) */   
           
    return status; 

} /* end of ixCryptoAccPkeEauPerform() function */


/**
 * @fn IxCryptoAccStatus ixCryptoAccPkeEauExpConfig (
    BOOL enableSE,
    BOOL enableFE)
 * @brief This function is used to configure short exponent setting (SE - 
 *        skipping leading 0s while performing modular exponential) and fast 
 *        exponent setting (FE - skipping all 0s while performing modular
 *        exponential) for EAU modular exponential operation. By default,
 *        these 2 settings are DISABLED to prevent timing attacks on keys
 *        and also ensure best protection on keys. These 2 setting will
 *        have direct impact on function call to @ref ixCryptoAccPkeEauPerform.
 *        Client may change it accordingly to achieve different level of 
 *        protection and also performance.
 *
 */
PUBLIC IxCryptoAccStatus 
ixCryptoAccPkeEauExpConfig (BOOL enableSE, BOOL enableFE)
{
    /* Check if PKE engine is exist and initialized */
    if (!IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED())
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "PKE engine does not exist or is not initialized.",
            0, 0, 0, 0, 0, 0);
            
         return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (!IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED()) */
    
    ixCryptoPkeEauExpConfigSet (enableSE, enableFE);
    
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
} /* end of ixCryptoAccPkeEauExpConfig() function */


/**
 * @fn IxCryptoAccStatus ixCryptoAccPkeHashPerform (
    IxCryptoAccAuthAlgo hashAlgo,
    UINT8 *pHashData,
    UINT32 hashDataLen,
    ixCryptoAccPkeHashPerformCompleteCallback hashPerformCallbackFn,
    UINT8 *pDigest)
 * @brief This function is used to hash the input data and return the digest
 *        generated via callback function supplied by client. L bytes size of 
 *        digest will be generated (L = 20 for SHA1). The hash data is padded 
 *        inside the function so that its length (in bits) is congruent to 448, 
 *        modulo 512 (please refer to RFC3174). The length of data in bytes to 
 *        be hashed must be greater than 0.
 *
 */ 
PUBLIC IxCryptoAccStatus
ixCryptoAccPkeHashPerform (
    IxCryptoAccAuthAlgo hashAlgo,
    UINT8 *pHashData,
    UINT32 hashDataLen,
    IxCryptoAccPkeHashPerformCompleteCallback hashPerformCallbackFn,
    UINT8 *pDigest)
{
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_SUCCESS;
    
    /* Check hash algorithm type, currently just support SHA1 algorithm. 
     * This check is for future expansion
     */
    if (IX_CRYPTO_ACC_AUTH_SHA1 != hashAlgo)
    {
        /* invalid hash algorithm requested */
        return IX_CRYPTO_ACC_STATUS_AUTH_ALGO_NOT_SUPPORTED;   
    }
    
    /* Check hash data length */
    if (IX_CRYPTO_ACC_INVALID_HASH_DATA_LEN == hashDataLen
        || IX_CRYPTO_ACC_MAX_HASH_DATA_LEN < hashDataLen)
    {
        /* invalid hash data length */
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    }
    
    /* Check if the data pointers are NULL, return error if NULL
     */
    if ((NULL == pHashData) || (NULL == pDigest))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "pHashData is %x\n pDigest is %x\n",
            (int) pHashData,
            (int) pDigest,
            0, 0, 0, 0);

        return IX_CRYPTO_ACC_STATUS_NULL_PTR_ERR;
    } /* end of if (pHashData) || (pDigest) */

    /* Check if the callback function provided by client is NULL, return
     * error if NULL
     */
    if (NULL == hashPerformCallbackFn)
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "PKE Hash Perform Callback: %x\n",
            (int) hashPerformCallbackFn,
            0,0, 0, 0, 0);

        return IX_CRYPTO_ACC_STATUS_NULL_PTR_ERR;
    } /* end of if (hashPerformCallbackFn) */
    
    /* Check if PKE engine is exist and initialized */
    if (!IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED())
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "PKE engine does not exist or is not initialized.\n",
            0, 0, 0, 0, 0, 0);
            
         return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (!IX_CRYPTO_IS_PKE_CRYPTO_ENGINE_INITIALIZED()) */
    
    /* Store information for use of hashing in different state  and 
     * try lock mutex 
     */
    status = ixCryptoPkeHashPrepNStateInfoStore (
        pHashData,
        pDigest, 
        hashPerformCallbackFn);
        
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
    {
        /* return if PKE hash engine is busy */
        return status;   
    }
    
    /* Hash data will be padded to multiple of 512-bit according to the 
     * padding scheme in RFC3174 (SHA1 hash algorithm).
     */
    ixCryptoPkeHashDataPad (hashDataLen);
    
    /* Start first block of data hashing, the rest of the data blocks 
     * hashing processing and terminal processing will be called in 
     * PKE hash interrupt callback function.
     */
    ixCryptoPkeHashConfigStart ();
    
    return IX_CRYPTO_ACC_STATUS_SUCCESS;

} /* end of ixCryptoAccPkeHashPerform () function */

#endif /* ixp46X */
