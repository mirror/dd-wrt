/**
 * @file IxCryptoPkeCommon_p.h
 *
 * @date March-18-2005
 *
 * @brief  Header file for PKE component
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
#ifndef IXCRYPTOPKECRYPTOENGINECOMMON_P_H
#define IXCRYPTOPKECRYPTOENGINECOMMON_P_H

#define IX_OSAL_ENFORCED_LE_AC_MAPPING  /**< Force to AC mode when in LE  */
#include "IxOsal.h"
#include "IxCryptoAcc.h"
#include "IxCryptoAcc_p.h"


/* type of the pke crypto unit */
typedef enum {
    IX_CRYPTO_PKE_EAU = 0,
    IX_CRYPTO_PKE_RNG,
    IX_CRYPTO_PKE_HASH,
    IX_CRYPTO_PKE_ENGINE_TYPE
} IxCryptoPkeEngineType;

/* Memory mapping wrapper */
#define IX_CRYPTO_ACC_MEM_MAP(physAddr,size) IX_OSAL_MEM_MAP(physAddr,size)
#define IX_CRYPTO_ACC_MEM_UNMAP(virtAddr) IX_OSAL_MEM_UNMAP(virtAddr)

/* Fast mutex wrapper */
#define IX_CRYPTO_ACC_FAST_MUTEX_INIT(mutex)    ixOsalFastMutexInit(mutex)
#define IX_CRYPTO_ACC_FAST_MUTEX_TRY_LOCK(mutex) ixOsalFastMutexTryLock(mutex)
#define IX_CRYPTO_ACC_FAST_MUTEX_UNLOCK(mutex)  ixOsalFastMutexUnlock(mutex)
#define IX_CRYPTO_ACC_FAST_MUTEX_DESTROY(mutex)  ixOsalFastMutexDestroy(mutex)

/* IRQ bind/unbind wrapper */
#define IX_CRYPTO_ACC_IRQ_BIND(level,funcPtr,parameter) \
    ixOsalIrqBind(level,funcPtr,parameter) 
#define IX_CRYPTO_ACC_IRQ_UNBIND(level) ixOsalIrqUnbind(level)

/* PKE crypto engine base address */
#define IX_CRYPTO_PKE_EAU_BASE_ADDR \
    IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_EAU_PHYS_BASE

#define IX_CRYPTO_PKE_RNG_HASH_BASE_ADDR \
    IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_RNG_SHA_PHYS_BASE

/* PKE crypto engine map size */
#define IX_CRYPTO_PKE_EAU_SIZE IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_EAU_MAP_SIZE      
#define IX_CRYPTO_PKE_RNG_HASH_SIZE \
    IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_RNG_SHA_MAP_SIZE

/* Offset of Hash Engine base address to RNG base address */
#define IX_CRYPTO_PKE_HASH_BASE_ADDR_OFFSET  0x100

/* Wrapper for register read / write to PKE submodule */
#define IX_CRYPTO_PKE_READ_LONG(wAddr) IX_OSAL_READ_LONG(wAddr)
#define IX_CRYPTO_PKE_WRITE_LONG(wAddr, data) \
    IX_OSAL_WRITE_LONG(wAddr, data)

/**
 * @fn      IxCryptoAccStatus ixCryptoPkeInit
 * @brief   Initialize RNG, Hash, and EAU units.
 * @param   "none"
 *
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operation performed successfully
 * @return @li  IX_CRYPTO_ACC_STATUS_FAIL - Operation failed
 *
 */
IxCryptoAccStatus 
ixCryptoPkeInit (void);


/**
 * @fn      IxCryptoAccStatus ixCryptoPkeResourcesRelease
 * @brief   Release all the resources allocated and mapped.
 * @param   "none"
 *
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operation performed successfully
 * @return @li  IX_CRYPTO_ACC_STATUS_FAIL - Operation failed
 */
IxCryptoAccStatus 
ixCryptoPkeResourcesRelease (void);


/**
 * @fn      IxCryptoAccStatus ixCryptoPkeUninit
 * @brief   UnInitialize RNG, Hash, and EAU units.
 * @param   "none"
 *
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operation performed successfully
 * @return @li  IX_CRYPTO_ACC_STATUS_FAIL - Operation failed
 */
IxCryptoAccStatus 
ixCryptoPkeUninit (void);


/**
 * @fn      IxOsalFastMutex * ixCryptoPkeFastMutexGet (IxCryptoPkeEngineType type)
 * @brief   Get mutex for the specified component
 * @param   type IxCryptoPkeEngineType - index to obtain PKE sub-component 
 *          fast mutex
 *
 * @return @li  IxOsalFastMutex - Fast mutex address
 */
IxOsalFastMutex *
ixCryptoPkeFastMutexGet (IxCryptoPkeEngineType type);


/**
 * @fn      UINT32 ixCryptoPkeAddressGet (IxCryptoPkeEngineType type)
 * @brief   Get the base address for specified Pke crypto engine component
 * @param   type IxCryptoPkeEngineType - index to obtain PKE sub-component 
 *          base address
 *
 * @return @li  address UNIT32 - address
 */
UINT32 
ixCryptoPkeAddressGet (IxCryptoPkeEngineType type);

#endif  /* IXCRYPTOPKECRYPTOENGINECOMMON_P_H */
#endif  /* __ixp46X */
