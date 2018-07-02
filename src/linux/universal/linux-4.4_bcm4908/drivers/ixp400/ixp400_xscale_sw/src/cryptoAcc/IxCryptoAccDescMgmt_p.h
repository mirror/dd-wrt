/**
 * @file IxCryptoAccDescMgmt_p.h
 *
 * @date October-03-2002
 *
 * @brief Private header file for Descriptors Management module
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


#ifndef IxCryptoAccDescMgmt_p_H
#define IxCryptoAccDescMgmt_p_H


/*
 * Put the user defined include files required.
 */
#include "IxCryptoAcc_p.h"


/*
 * #defines for function return types, etc.
 */

/* Maximum numbers of queue descriptor created in the descriptor pool. 278 
 * descriptors are created in the pool. This number ONLY be changed to a 
 * bigger number and not smaller. This is because the queue depth is 64 
 * entries per queue, 4 queues are used for this access component and we  
 * need to have extra 16 descriptors for NPE as NPE may hold up to 16 
 * descriptor at the same time. Furthermore, 6 descriptors are reserved for 
 * XScale processing contexts
 */
#define MAX_DESCRIPTORS_NUM_IN_POOL   278

#define IX_CRYPTO_Q_DESC_ADDR_WORD_ALIGNED_SIZE   3   
                                                /**< Extra bytes needed in 
                                                 * memory allocation to ensure 
                                                 * the queue descriptor address
                                                 * is word aligned
                                                 */ 

/* Q Descriptor size for IxCryptoAcc software component */
#define IX_CRYPTO_Q_DESC_SIZE (((sizeof (IxCryptoQDescriptor) +       \
                                  (IX_OSAL_CACHE_LINE_SIZE - 1)) /  \
                                  IX_OSAL_CACHE_LINE_SIZE) *        \
                                  IX_OSAL_CACHE_LINE_SIZE )

/* Additional Authentication Data size */
#define IX_CRYPTO_Q_DESC_AAD_SIZE ((( (IX_CRYPTO_ACC_CCM_MAX_AAD_LEN ) +  \
                                  (IX_OSAL_CACHE_LINE_SIZE - 1)) /  \
                                  IX_OSAL_CACHE_LINE_SIZE)       *  \
                                  IX_OSAL_CACHE_LINE_SIZE )

/**
 * @fn ixCryptoDescMgmtInit
 *
 * @brief Initialize descriptor management module. Pool of descriptors 
 *        shared across NPE and IxCryptoAcc software component is allocated
 *        and initialized.
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoDescMgmtInit (void);



/**
 * @fn ixCryptoDescMgmtQDescriptorGet
 *
 * @brief Get queue descriptor from the descriptor pool.
 *
 * @param IxCryptoQDescriptor** [out] pQDescriptor - Q descriptor pointer
 * 
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoDescMgmtQDescriptorGet (IxCryptoQDescriptor **pQDescriptor);



/**
 * @fn ixCryptoDescMgmtQDescriptorRelease
 *
 * @brief Release queue descriptor previously allocated back to the 
 *        descriptor pool
 *
 * @param IxCryptoQDescriptor* [in] pQDescriptor - Q descriptor pointer
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoDescMgmtQDescriptorRelease (IxCryptoQDescriptor *pQDescriptor);



/**
 * @fn ixCryptoDescMgmtShow
 *
 * @brief To show the number of descriptors that have been used, and the pool
 *        usage.
 *
 * @return none
 *
 */
void
ixCryptoDescMgmtShow (void);



/**
 * @fn ixCryptoDescMgmtDescPoolFree
 *
 * @brief To free the memory allocated to descriptor pool through malloc 
 *        function. 
 *
 * @return none
 *
 */
void
ixCryptoDescMgmtDescPoolFree (void);



/**
 * @fn ixCryptoDescMgmtAllQDescriptorInPoolCheck
 *
 * @brief This inline function is used to check whether all the allocated 
 *        descriptors have been pushed back to the descriptors pool. If all 
 *        descriptors are unused, it indicates that there is no pending task 
 *        in the queues.
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 * Remove INLINE keyword to resolve cannot inline warning (VxWorks). SCR1421
 *
 */
IxCryptoAccStatus
ixCryptoDescMgmtAllQDescriptorInPoolCheck (void);


#endif /* def IxCryptoAccDescMgmt_p_H */
