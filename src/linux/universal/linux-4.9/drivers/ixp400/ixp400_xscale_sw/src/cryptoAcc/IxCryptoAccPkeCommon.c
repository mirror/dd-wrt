/**
 * @file IxCryptoPkeCommon.c
 *
 * @date March-18-2006
 *
 * @brief  Source file for PKE component 
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

#if defined(__ixp46X)     
/* PKE is not supported in IXP42X platform */
#define IX_OSAL_ENFORCED_LE_AC_MAPPING  /**< Force to AC mode when in LE  */
#include "IxOsal.h"
#include "IxCryptoAcc_p.h"
#include "IxCryptoAccPkeCommon_p.h"
#include "IxCryptoAccUtilities_p.h"
#include "IxCryptoAccPkeEau_p.h"
#include "IxCryptoAccPkeHash_p.h"

static BOOL initDone = FALSE;   /* TRUE if PKE engine is initialized */

static BOOL pkeBaseAddrMappedFlag [IX_CRYPTO_PKE_ENGINE_TYPE]
                  = {FALSE, FALSE, FALSE}; /* TRUE if the PKE engine address
                                            * (EAU, RNG, HASH) has been mapped.
                                            * Note: Hash Map flag is not used.
                                            */
                                            
static BOOL pkeFastMutexInitFlag[IX_CRYPTO_PKE_ENGINE_TYPE]
                  = {FALSE, FALSE, FALSE}; /* TRUE if the mutex for PKE engine
                                            * (EAU, RNG, HASH) has been 
                                            * initialized 
                                            */    

/* Static variables used in inline function */
static UINT32 pkeBaseAddress [IX_CRYPTO_PKE_ENGINE_TYPE]; 
                                            /* 3 different base address 
                                             * - EAU, RNG, HASH 
                                             */
static IxOsalFastMutex pkeFastMutex[IX_CRYPTO_PKE_ENGINE_TYPE];     
                                            /* 3 fast mutexes - EAU, RNG, HASH*/
                                                                                       
/**
 * @fn IxCryptoAccStatus ixCryptoPkeInit
 * @brief   Initialize RNG, HASH, and EAU uints.
 */
IxCryptoAccStatus
ixCryptoPkeInit (void)
{
    if (initDone)        /* Check if PKE has been initialized */
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (initDone) */
    
    /* Mapping EAU base address */
    pkeBaseAddress[IX_CRYPTO_PKE_EAU] = (UINT32) IX_CRYPTO_ACC_MEM_MAP (
        IX_CRYPTO_PKE_EAU_BASE_ADDR, 
        IX_CRYPTO_PKE_EAU_SIZE);
        
    /* Assert if NULL */
    IX_OSAL_ASSERT (pkeBaseAddress[IX_CRYPTO_PKE_EAU]);
    
    /* Set flag to TRUE to indicate mem map success */
    pkeBaseAddrMappedFlag[IX_CRYPTO_PKE_EAU] = TRUE;
    
    /* Mapping RNG and HASH engine base address */
    pkeBaseAddress[IX_CRYPTO_PKE_RNG] = (UINT32) IX_CRYPTO_ACC_MEM_MAP (
        IX_CRYPTO_PKE_RNG_HASH_BASE_ADDR, 
        IX_CRYPTO_PKE_RNG_HASH_SIZE);
    
    /* Assert if NULL */
    IX_OSAL_ASSERT (pkeBaseAddress[IX_CRYPTO_PKE_RNG]);
   
    /* Set flag to TRUE to indicate mem map success */
    pkeBaseAddrMappedFlag[IX_CRYPTO_PKE_RNG] = TRUE;
    
    /* Mapping Hash engine base address */
    pkeBaseAddress[IX_CRYPTO_PKE_HASH] = pkeBaseAddress[IX_CRYPTO_PKE_RNG] 
        + IX_CRYPTO_PKE_HASH_BASE_ADDR_OFFSET;

    /* Initialize fast mutex for EAU */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        IX_CRYPTO_ACC_FAST_MUTEX_INIT (&pkeFastMutex[IX_CRYPTO_PKE_EAU]))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Cannot init EAU fast mutex.\n", 
            0, 0, 0, 0, 0, 0);

        /* Release all resources which has been allocated.
         * Note: return status of this function is ignore, no atter it is 
         * success or fail, we will still return fail since the init failed. 
         */
        ixCryptoPkeResourcesRelease ();
       
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (IX_CRYPTO_ACC_FAST_MUTEX_INIT) */
    
    /* Set flag to TRUE to indicate mutex initialization success */   
    pkeFastMutexInitFlag[IX_CRYPTO_PKE_EAU] = TRUE;
    
    /* Initialize fast mutex for RNG */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        IX_CRYPTO_ACC_FAST_MUTEX_INIT (&(pkeFastMutex[IX_CRYPTO_PKE_RNG])))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Cannot init RNG fast mutex.\n", 
            0, 0, 0, 0, 0, 0);
        
        /* Release all resources which has been allocated.
         * Note: return status of this function is ignore, no atter it is 
         * success or fail, we will still return fail since the init failed. 
         */
        ixCryptoPkeResourcesRelease ();
                
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (IX_CRYPTO_ACC_FAST_MUTEX_INIT) */

    /* Set flag to TRUE to indicate mutex initialization success */   
    pkeFastMutexInitFlag[IX_CRYPTO_PKE_RNG] = TRUE;
   
    /* Initialize fast mutex for HASH engine */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        IX_CRYPTO_ACC_FAST_MUTEX_INIT (&(pkeFastMutex[IX_CRYPTO_PKE_HASH])))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Cannot init Hash fast mutex.\n", 
            0, 0, 0, 0, 0, 0);
        
        /* Release all resources which has been allocated.
         * Note: return status of this function is ignore, no atter it is 
         * success or fail, we will still return fail since the init failed. 
         */
        ixCryptoPkeResourcesRelease ();
                
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (IX_CRYPTO_ACC_FAST_MUTEX_INIT) */

    /* Set flag to TRUE to indicate mutex initialization success */   
    pkeFastMutexInitFlag[IX_CRYPTO_PKE_HASH] = TRUE;
    
    /* Initialize PKE EAU sub-module */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoPkeEauInit())
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Cannot initialize EAU module.\n", 
            0, 0, 0, 0, 0, 0);

        /* Release all resources which has been allocated.
         * Note: return status of this function is ignore, no atter it is 
         * success or fail, we will still return fail since the init failed. 
         */
        ixCryptoPkeResourcesRelease ();
                    
        return IX_CRYPTO_ACC_STATUS_FAIL;    
    } /* end of if(ixCryptoPkeEauInit()) */
    
        /* Initialize PKE EAU sub-module */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoPkeHashInit())
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Cannot initialize HASH module.\n", 
            0, 0, 0, 0, 0, 0);
            
        /* Uninit PKE EAU sub-module if failed */
        ixCryptoPkeEauUninit ();

        /* Release all resources which has been allocated.
         * Note: return status of this function is ignore, no atter it is 
         * success or fail, we will still return fail since the init failed. 
         */
        ixCryptoPkeResourcesRelease ();
        
        return IX_CRYPTO_ACC_STATUS_FAIL;    
    } /* end of if (ixCryptoPkeHashInit ()) */
    
    initDone = TRUE;
   
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
} /* end of ixCryptoPkeInit() function */


/**
 * @fn      IxCryptoAccStatus ixCryptoPkeResourcesRelease
 * @brief   Release all the resources allocated and mapped.
 * @param   "none"
 *
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operation performed successfully
 * @return @li  IX_CRYPTO_ACC_STATUS_FAIL - Operation failed
 */
IxCryptoAccStatus 
ixCryptoPkeResourcesRelease (void)
{
    /* Destroy EAU mutex if initialized */
    if (pkeFastMutexInitFlag[IX_CRYPTO_PKE_EAU])
    {
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
            IX_CRYPTO_ACC_FAST_MUTEX_DESTROY (&pkeFastMutex[IX_CRYPTO_PKE_EAU]))
         {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Cannot destroy EAU mutex.\n", 
                0, 0, 0, 0, 0, 0);
            
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (IX_CRYPTO_ACC_FAST_MUTEX_DESTROY) */
        
        /* Reset the flag to false to indicate the mutex has been destroyed */
        pkeFastMutexInitFlag[IX_CRYPTO_PKE_EAU] = FALSE;
    } /* end of if (pkeFastMutexInitFlag[IX_CRYPTO_PKE_EAU]) */
    
    /* Destroy RNG mutex if initialized */
    if (pkeFastMutexInitFlag[IX_CRYPTO_PKE_RNG])
    {    
        /* Destroy RNG mutex */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS !=
            IX_CRYPTO_ACC_FAST_MUTEX_DESTROY (&pkeFastMutex[IX_CRYPTO_PKE_RNG]))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Cannot destroy RNG mutex.\n", 
                0, 0, 0, 0, 0, 0);
            
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (IX_CRYPTO_ACC_FAST_MUTEX_DESTROY) */
        
        /* Reset the flag to false to indicate the mutex has been destroyed */
        pkeFastMutexInitFlag[IX_CRYPTO_PKE_RNG] = FALSE;
    } /* end of if (pkeFastMutexInitFlag[IX_CRYPTO_PKE_RNG]) */
    
    /* Destroy Hash mutex if initialized */
    if (pkeFastMutexInitFlag[IX_CRYPTO_PKE_HASH])
    { 
        if (IX_CRYPTO_ACC_STATUS_SUCCESS !=
            IX_CRYPTO_ACC_FAST_MUTEX_DESTROY (&pkeFastMutex[IX_CRYPTO_PKE_HASH]))
         {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Cannot destroy HASH mutex.\n", 
                0, 0, 0, 0, 0, 0);
            
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (IX_CRYPTO_ACC_FAST_MUTEX_DESTROY) */
        
        /* Reset the flag to false to indicate the mutex has been destroyed */
        pkeFastMutexInitFlag[IX_CRYPTO_PKE_HASH] = FALSE;
    } /* end of if (pkeFastMutexInitFlag[IX_CRYPTO_PKE_HASH]) */
    
    /* Unmap PKE EAU engine address and reset the flag */
    if (pkeBaseAddrMappedFlag[IX_CRYPTO_PKE_EAU])
    {
        IX_CRYPTO_ACC_MEM_UNMAP (pkeBaseAddress[IX_CRYPTO_PKE_EAU]);
        pkeBaseAddrMappedFlag[IX_CRYPTO_PKE_EAU] = FALSE; 
    } /* end of if (pkeBaseAddrMappedFlag[IX_CRYPTO_PKE_EAU]) */
    
    /* Unmap PKE RNG engine address and reset the flag */
    if (pkeBaseAddrMappedFlag[IX_CRYPTO_PKE_EAU])
    {
        IX_CRYPTO_ACC_MEM_UNMAP (pkeBaseAddress[IX_CRYPTO_PKE_RNG]);
        pkeBaseAddrMappedFlag[IX_CRYPTO_PKE_RNG] = FALSE;
    } /* end of if (pkeBaseAddrMappedFlag[IX_CRYPTO_PKE_RNG]) */
    
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
} /* end of function ixCryptoPkeResourcesRelease () */


/**
 * @fn IxCryptoAccStatus ixCryptoPkeUnInit
 * @brief   UnInitialize RNG, HASH, and EAU uints.
 */
IxCryptoAccStatus ixCryptoPkeUninit (void)
{
    if (!initDone) /* If not initialized */
    {
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    } /* end of if (!initDone) */
    
    /* Uninit PKE EAU sub-module */    
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoPkeEauUninit())
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Uninitialize EAU sub-module failed.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_FAIL;    
    } /* end of if (ixCryptoPkeEauUninit ()) */
    
    /* Uninit PKE Hash sub-module */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoPkeHashUninit())
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Uninitialize Hash sub-module failed.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_FAIL;    
    } /* end of if (ixCryptoPkeHashUninit ()) */

    /* Release all resources which has been allocated */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != ixCryptoPkeResourcesRelease ())
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Failed to release all the resources allocated.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (ixCryptoPkeResourcesRelease ()) */
    
    /* Reset flag to indicate uninit successful */
    initDone = FALSE;
        
    return IX_CRYPTO_ACC_STATUS_SUCCESS; 
} /* end of ixCryptoPkeUnInit() function */

/**
 * @fn      IxOsalFastMutex ixCryptoPkeFastMutexGet (IxCryptoPkeEngineType type)
 * @brief   Get mutex for the specified component
 *
 */
IxOsalFastMutex *
ixCryptoPkeFastMutexGet (IxCryptoPkeEngineType type)
{
    return &pkeFastMutex[type];
} /* end of ixCryptoPkeFastMutexGet() function */


/**
 * @fn      UINT32 ixCryptoPkeAddressGet (IxCryptoPkeEngineType type)
 * @brief   Get the base address for specified Pke crypto engine component
 *
 */
UINT32 
ixCryptoPkeAddressGet (IxCryptoPkeEngineType type)
{
    return pkeBaseAddress[type];
} /* end of ixCryptoPkeAddressGet() function */

#endif /* __ixp46X */

