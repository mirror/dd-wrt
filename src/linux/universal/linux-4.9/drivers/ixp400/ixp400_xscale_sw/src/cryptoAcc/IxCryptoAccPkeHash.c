/**
 * @file IxCryptoAccPkeHash.c
 *
 * @date March-23-2005
 *
 * @brief  Source file for PKE Hash Module
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

/*
* Put the user defined include files required.
*/
#define IX_OSAL_ENFORCED_LE_AC_MAPPING  /**< Force to AC mode when in LE  */
#include "IxOsal.h"
#include "IxCryptoAcc_p.h"
#include "IxCryptoAccUtilities_p.h"
#include "IxCryptoAccPkeCommon_p.h"
#include "IxCryptoAccPkeHash_p.h"

static BOOL initDone = FALSE;           /* flag used to prevent multiple
                                         * initialization
                                         */

static IxCryptoPkeHashStats pkeHashStats;   /* PKE hash operation counters */

static IxCryptoPkeHashRegAddr pkeHashRegAddr; /* PKE hash registers
                                               * address struct
                                               */
                                                       
static IxCryptoPkeHashStateInfo pkeHashStateInfo;   /* State information needed
                                                     * in callback
                                                     */
                                                     
static IxOsalFastMutex *pPkeHashFastMutex;    /* PKE hash fast mutex is used to
                                             * to prevent multiple access
                                             * to PKE hash engine. Only one
                                             * request is allowed each time.
                                             * If fast mutex lock fails, it
                                             * means PKE hash engine is busy
                                             * processing the request, and 
                                             * cannot take in any other request
                                             */


/**
 * @fn ixCryptoPkeHashInit
 * @brief This function is responsible to initialize resources used and bind the 
 *        callback function pointer to interrupt level for PKE Hash module. 
 *
 */
IxCryptoAccStatus
ixCryptoPkeHashInit (void)
{   
    UINT32 pkeHashBaseAddr = 0;
    
    if (!initDone) /* If not initialized */
    {
        /* Get mutex */
        pPkeHashFastMutex = ixCryptoPkeFastMutexGet (IX_CRYPTO_PKE_HASH);
                
        /* Obtain base address for PKE hash MMR */
        pkeHashBaseAddr = ixCryptoPkeAddressGet (IX_CRYPTO_PKE_HASH);
        
        /* Obtain base addrress for each register */
        pkeHashRegAddr.hashConfigReg = pkeHashBaseAddr
            + IX_CRYPTO_PKE_HASH_REG_HCONFIG_OFFSET;  
        pkeHashRegAddr.hashDoReg = pkeHashBaseAddr
            + IX_CRYPTO_PKE_HASH_REG_HDO_OFFSET;
        pkeHashRegAddr.hashInterruptReg = pkeHashBaseAddr
            + IX_CRYPTO_PKE_HASH_REG_HINTR_OFFSET;
        pkeHashRegAddr.hashChainVarReg = pkeHashBaseAddr
            + IX_CRYPTO_PKE_HASH_REG_HCHAIN_OFFSET;
        pkeHashRegAddr.hashDataReg = pkeHashBaseAddr
            + IX_CRYPTO_PKE_HASH_REG_HDATA_OFFSET;

        /* Allocate memory for use of padding in hash data blocks.  
         * Size allocated is 2 blocks of hash data size due to padding 
         * process. Padding might span accross to another hash data block,
         * therefore one extra block is allocated. A flag (is actually offset
         * of data pointer in padded data block) will be used to indicate
         * one or two padded hash data blocks 
         */
        pkeHashStateInfo.pPaddedBlockHashData 
            = IX_CRYPTO_ACC_DRV_DMA_MALLOC(2 * IX_CRYPTO_HASH_DATA_BLOCK_LENGTH);
        
        if (NULL == pkeHashStateInfo.pPaddedBlockHashData)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "Memory allocation for hash padding block failed.\n",
                0, 0, 0, 0, 0, 0);

            /* Memory allocation failed */
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (pkeHashStateInfo.pPaddedBlockHashData) */

        /* Clear interrupt for initialization to ensure no pending interrupt
         * to be serviced before binding callback function to that interrupt
         * level
         */
        IX_CRYPTO_PKE_WRITE_LONG (pkeHashRegAddr.hashInterruptReg, 
            IX_CRYPTO_PKE_HASH_HINTR_CLEAR);

        /* Hook the Hash module callback to the interrupt controller. */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS 
            != IX_CRYPTO_ACC_IRQ_BIND (
                    IX_OSAL_IXP400_SHA_HASHING_DONE_IRQ_LVL,
                    (IxOsalVoidFnVoidPtr) ixCryptoPkeHashDoneIsr,
                    NULL))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "Failed to bind to PKE Hash interrupt\n", 
                0, 0, 0, 0, 0, 0);
                
            /* free the memory allocated */
            IX_CRYPTO_ACC_DRV_DMA_FREE (pkeHashStateInfo.pPaddedBlockHashData);

            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (IX_CRYPTO_ACC_IRQ_BIND() */

        /* Initialize counters */
        pkeHashStats.pkeHashReqCount = 0;
        pkeHashStats.pkeHashReqDoneCount = 0;
        
        /* Set initDone flag to TRUE to indicate intialization has been
         * completed
         */
        initDone = TRUE;

        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else /* PKE Hash module has been initialized */
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if-else (initDone) */     
} /* end of ixCryptoAccPkeHashInit () function */


/**
 * @fn ixCryptoPkeHashUninit
 * @brief This function is responsible to free resources used and unbind the 
 *        callback function pointer from interrupt level for PKE Hash module. 
 *
 */
IxCryptoAccStatus
ixCryptoPkeHashUninit (void)
{
    UINT32 timer = 0;
    
    if (initDone) /* If initialized */
    {
        /* Check if all request has finished */
        while (pkeHashStats.pkeHashReqCount != pkeHashStats.pkeHashReqDoneCount)
        {
            /* Put the task into sleep mode and wait for the HASH to complete
             * all the operations
             */
            ixOsalSleep(IX_CRYPTO_ACC_DELAY_IN_MS);
        
            /* Timer to avoid infinite lock */
            timer++;
            if (IX_CRYPTO_ACC_TIMER_COUNT < timer )
            {
                /* Log error message in debugging mode */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR, 
                    IX_OSAL_LOG_DEV_STDERR,
                    "HASH Request is not done!\n", 
                    0, 0, 0, 0, 0, 0);
                
                return IX_CRYPTO_ACC_STATUS_FAIL;
            } /* End of if timer loop */
        } /* end of while
           (pkeHashStats.pkeHashReqCount != pkeHashStats.pkeHashReqDoneCount) */
        
        /* If data pointer is not NULL, free it */
        if (NULL != pkeHashStateInfo.pPaddedBlockHashData)
        {
            /* free the memory allocated */
            IX_CRYPTO_ACC_DRV_DMA_FREE (pkeHashStateInfo.pPaddedBlockHashData);
        } /* end of if (pkeHashStateInfo.pPaddedBlockHashData) */
        
        /* 
         * Unhook the hash module callback from the interrupt controller. 
         */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS 
            != IX_CRYPTO_ACC_IRQ_UNBIND (
                   IX_OSAL_IXP400_SHA_HASHING_DONE_IRQ_LVL))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "Failed to unbind to PKE Hash interrupt\n", 
                0, 0, 0, 0, 0, 0);
                
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (IX_CRYPTO_ACC_IRQ_UNBIND) */
        
        /* Clear flag to indicate the PKE Hash submodule has been 
         * uninitialized 
         */
        initDone = FALSE;
        
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    }
    else    /* if not initialized */
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;
    }/* end of if-else (initDone) */   
    
} /* end of ixCryptoAccPkeHashUninit () function */

    
/**
 * @fn ixCryptoPkeHashPrepNStateInfoStore
 * @brief This function is responsible to store the function pointer and data
 *        pointers into state info variable. This function is also responsible
 *        to check if the hash engine is free to perform the requested 
 *        operation.
 *
 */
IxCryptoAccStatus
ixCryptoPkeHashPrepNStateInfoStore (
    UINT8 *pHashData,
    UINT8 *pDigest,
    IxCryptoAccPkeHashPerformCompleteCallback hashPerformCallbackFn)
{
    /* Try to lock mutex. If try lock fails, which mean Hash engine is busy
     * now, client need to retry later */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        IX_CRYPTO_ACC_FAST_MUTEX_TRY_LOCK (pPkeHashFastMutex))
    {
        /* Log message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "PKE hash engine is busy.\n", 
            0, 0, 0, 0, 0, 0);  
        
        return IX_CRYPTO_ACC_STATUS_RETRY;          
    } /* end of if (IX_CRYPTO_ACC_FAST_MUTEX_TRY_LOCK) */
            
    /* store pointers into state info variable */
    pkeHashStateInfo.pBlockHashData = pHashData;
    pkeHashStateInfo.pDigest = pDigest;
    pkeHashStateInfo.hashPerformCallbackFn = hashPerformCallbackFn;
    
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
        
} /* end of ixCryptoPkeHashStateInfoStore () function */


/**
 * @fn ixCryptoPkeHashDataPad
 * @brief This function is responsible to pad the hash data according to 
 *        padding scheme in RFC3174. The last hash data block from client
 *        will be copied into the pre-allocated padded hash data block for
 *        padding purpose. The padding will be done in padded hash data block
 *        instead of client's hash data block. This is to avoid changing the
 *        content of hash data from client. 
 *
 */
void ixCryptoPkeHashDataPad (UINT32 hashDataLen)
{
    UINT32 hashDataBlockOffset = 0;
    UINT32 hashDataRemainingLen = 0;
    UINT32 padLength = 0;
    UINT32 lengthInBit = 0;
    UINT32 i = 0;
    
    /* Calculate number of hash data block need to be hashed excluding final
     * / padded data block
     */
    pkeHashStateInfo.hashBlockCount 
        = hashDataLen / IX_CRYPTO_HASH_DATA_BLOCK_LENGTH;
    
    /* Calculate the offset of hash data pointer to copy the final hash data
     * block into padded data block and copy the data.
     */
    hashDataBlockOffset = pkeHashStateInfo.hashBlockCount 
                              * IX_CRYPTO_HASH_DATA_BLOCK_LENGTH;
    hashDataRemainingLen = hashDataLen - hashDataBlockOffset;
    
    ixOsalMemCopy (
        pkeHashStateInfo.pPaddedBlockHashData, 
        pkeHashStateInfo.pBlockHashData + hashDataBlockOffset, 
        hashDataRemainingLen);
        
    /* Do padding, if the hash data is not congruent 448 bit 
     * (56 bytes), modulo 512-bit (64 bytes). 
     */

    /* Add 1 to hashBlockCount to include 1 padded block by default */
    pkeHashStateInfo.hashBlockCount++;
    
    /* length of hash data in bits */
    lengthInBit = hashDataLen * BITS_IN_BYTE;

    /* pad hash data with value 0x80 */
    ixOsalMemSet (
        &(pkeHashStateInfo.pPaddedBlockHashData[hashDataRemainingLen]), 
        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE, 
        IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN);

    /* Set padDataBlockOffset to 0, to indicate only 1 padded data 
     * block is used (default)
     */
    pkeHashStateInfo.padDataBlockOffset = 0;
             
    /* There are 3 cases of padding process :-
     * case 1: No padding is needed if the last block of hash data is 
     *         equal to IX_CRYPTO_ACC_HASH_DATA_REMAINDER.
     * case 2: Padding needed, but only 1 paddded hash data block is used.
     *         This case happen if the last block of hash data is 
     *         <= IX_CRYPTO_ACC_HASH_DATA_REMAINDER. The padding bytes and 
     *         length field can fit into 1 padded data block.
     * case 3: Padding needed, but 2 paddded hash data blocks are used.
     *         This case happen if the last block of hash data is 
     *         > IX_CRYPTO_ACC_HASH_DATA_REMAINDER. The padding bytes and 
     *         length field cannot fit into 1 padded data block, one extra
     *         padded data block is used for padding and hold the length field.
     */
    if (IX_CRYPTO_ACC_HASH_DATA_REMAINDER ==
        (hashDataRemainingLen % IX_CRYPTO_HASH_DATA_BLOCK_LENGTH)) /* case 1 */
    {
        /* No 0s padding is needed */
        padLength = 0;
    }
    else /* 2 pad data blocks case */
    {
        /* If the if statement is true, which mean the code fall into case 3,
         * 2 padded data blocks are needed. Otherwise is case 2, only 1 padded
         * data block is used.
         */
        if (IX_CRYPTO_ACC_HASH_DATA_REMAINDER <
            (hashDataRemainingLen % IX_CRYPTO_HASH_DATA_BLOCK_LENGTH))
        {
            /* Set padDataBlockOffset to 64, to indicate 2 padded data 
             * blocks are used
             */
            pkeHashStateInfo.padDataBlockOffset 
                = IX_CRYPTO_HASH_DATA_BLOCK_LENGTH;
                
            /* Inrement hashBlockCount to indicate 2 padded blocks */
            pkeHashStateInfo.hashBlockCount++;
        } /* end of if (hashDataRemainingLen) */
                
        /* calculate zero padding length (bytes) for original 
         * hashDataRemainingLen to make it multiple of 512-bit
         */
        padLength = IX_CRYPTO_HASH_DATA_BLOCK_LENGTH -
                    ((hashDataRemainingLen + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LEN
                    + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN) % 
                    IX_CRYPTO_HASH_DATA_BLOCK_LENGTH);
            
        /* pad remaining hash data with 0s at the back */
        ixOsalMemSet (
            &(pkeHashStateInfo.pPaddedBlockHashData[hashDataRemainingLen 
            + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN]), 
            0, 
            padLength);
        
    } /* end of if-else (hashDataRemainingLen) */
    
    /* set total bit (high 32-bit) to zero, hash data size will
     * not exceed 2^32
     */
    ixOsalMemSet (
        &(pkeHashStateInfo.pPaddedBlockHashData[hashDataRemainingLen
        + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN
        + padLength]),        
        0, 
        IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN);
    
    /* Set total bit field (low 32-bit). For loop is used to shift
     * the total bit value into the last 4-byte of hash data block in order
     * to overcome endianess conversion issue due to different data type 
     * pointer involved in the value copying.
     */
    for (i = 0; i < IX_CRYPTO_ACC_HASH_TOTAL_BIT_LOW_LEN; i++)
    {
        pkeHashStateInfo.pPaddedBlockHashData[hashDataRemainingLen
            + IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN
            + padLength 
            + IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN 
            + i] 
            = (lengthInBit >> ((MSB_BYTE - i) * BITS_IN_BYTE) 
                  & MASK_8_BIT);
    } /* end of for loop (i) */
    
    /* If the hashDataLen <= 63; which mean the padding is needed on the first
     * (the only one) block of hash data. This hash data block will be copied
     * into pre-allocated padded hash data block for padding purpose. Therefore,
     * for this case, the padded hash data block should be used for hashing 
     * instead of original hash data block pointer from client.
     */
    if ((IX_CRYPTO_HASH_DATA_BLOCK_LENGTH - 1) >= hashDataLen)
    {
        pkeHashStateInfo.pBlockHashData = pkeHashStateInfo.pPaddedBlockHashData;
    } /* end of if (hashDataLen) */
    
    /* Log message in debugging mode */
    IX_CRYPTO_ACC_LOG (
        IX_OSAL_LOG_LVL_MESSAGE, 
        IX_OSAL_LOG_DEV_STDOUT,
        "HashBlockCount: %d. HashOffset = %d\n", 
        pkeHashStateInfo.hashBlockCount, 
        pkeHashStateInfo.padDataBlockOffset, 
        0, 0, 0, 0);

} /* end of ixCryptoPkeHashDatapad function */


/**
 * @fn ixCryptoPkeHashConfigStart
 * @brief This function will configure the hash engine to operate in SHA1
 *        mode, initialize hash engine with initial chaining variables and
 *        then start hashing on first block of hash data
 *
 * Note: Byte swapping for read/write chaining variables, read/wrute data into
 * hash engine is enabled in LE mode. This is to overcome the endianess 
 * conversion issue due to different type of data pointer pointers are used in
 * the memory read/write access. Use input is in byte pointer, while writing
 * into hash engine is word pointer.
 */
void ixCryptoPkeHashConfigStart (void)
{
    UINT8 sha1ChainVar[IX_CRYPTO_PKE_HASH_SHA1_CHAIN_VAR_LEN] = SHA1_CHAIN_VAR;
    UINT32 *pTempChainVar = (UINT32 *) &sha1ChainVar[0];
    UINT32 *pTempData = (UINT32 *) pkeHashStateInfo.pBlockHashData;
    UINT32 i = 0;
    
    /* Write configuration value into hash config register to reset internal 
     * pointer of Hash engine
     */
    IX_CRYPTO_PKE_WRITE_LONG (
        pkeHashRegAddr.hashConfigReg,
        IX_CRYPTO_PKE_HASH_HCFG_REG_SHA1_CONF); 
        
    /* Log message in debugging mode */
    IX_CRYPTO_ACC_LOG (
        IX_OSAL_LOG_LVL_MESSAGE, 
        IX_OSAL_LOG_DEV_STDOUT,
        "PKE Hash Config : %8X.\n", 
        IX_CRYPTO_PKE_HASH_HCFG_REG_SHA1_CONF, 
        0, 0, 0, 0, 0);

    /* Write initial chaining variables
     * Note: hashChainVarReg address is auto-increment in the hash engine
     * itself, there is no need to increment the register address after each 
     * write into that register.
     */
    for (i = 0; i < IX_CRYPTO_PKE_HASH_SHA1_CHAIN_VAR_WLEN; i++)
    {
        /* Write 4 bytes of CV each time */
        IX_CRYPTO_PKE_WRITE_LONG (
            pkeHashRegAddr.hashChainVarReg,
            *pTempChainVar);
        
        /* Advanced the chaining variables pointer */
        pTempChainVar++;
    } /* end of for loop (i) */

    /* Write first block of hashing data into data registers in PKE Hash 
     * engine.
     * Note: hashDataReg address is auto-increment in the hash engine
     * itself, there is no need to increment the register address after each 
     * write into that register. 
     */
    for (i = 0; i < IX_CRYPTO_PKE_HASH_DATA_BLOCK_WLEN; i++)
    {
        /* Write 4 bytes of data each time */
        IX_CRYPTO_PKE_WRITE_LONG (
            pkeHashRegAddr.hashDataReg,
            *pTempData);
        
        /* Advanced the data pointer */
        pTempData++;                
    } /* end of for loop (i) */
    
    /* Update hashBlockCount to indicate remaining of data blocks to be hashed 
     */
    pkeHashStateInfo.hashBlockCount--;
    
    /* Update hash data block pointer */
    pkeHashStateInfo.pBlockHashData += IX_CRYPTO_HASH_DATA_BLOCK_LENGTH;

    /* Update statistic */    
    pkeHashStats.pkeHashReqCount++;
        
    /* Start hashing */
    IX_CRYPTO_PKE_WRITE_LONG (
        pkeHashRegAddr.hashDoReg,
        IX_CRYPTO_PKE_HASH_HDO_START);
    
} /* end of ixCryptoPkeHashConfigStart () function */


/**
 * @fn ixCryptoPkeHashDoneIsr
 * @brief Callback which is binded to PKE Hash Done interrupt. 
 *        Once the interrupt is triggered, the callback is executed.
 *        This function acts like a state machine to continue from where it left
 *        off for the hashing operation, and call the client's callback in
 *        terminal state to return the digest computed.
 *
 */
void
ixCryptoPkeHashDoneIsr (void)
{
    UINT32 *pTempDigest = NULL;
    UINT32 *pTempData = (UINT32 *) pkeHashStateInfo.pBlockHashData;
    UINT32 i = 0;
    
    /* Clear interrupt */
    IX_CRYPTO_PKE_WRITE_LONG (
        pkeHashRegAddr.hashInterruptReg,
        IX_CRYPTO_PKE_HASH_HINTR_CLEAR);
        
    /* Decide on which of the following state to enter based on the number
     * of hash data block remaining 
     */
    switch (pkeHashStateInfo.hashBlockCount)
    {
        /* Terminal processing for hash operation. Task need be done in terminal
         * processing are retrieving digest from hash engine and return the
         * digest to client by calling client's callback. Mutex need to be 
         * released in terminal processing too.
         */
        case IX_CRYPTO_PKE_HASH_TERMINAL_STATE:
            /* Log message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "IX_CRYPTO_PKE_HASH_TERMINAL_STATE\n", 
                0, 0, 0, 0, 0, 0);
        
            pTempDigest = (UINT32 *) pkeHashStateInfo.pDigest;
            
            /* Read chaining variables / digest 
             * Note: hashChainVarReg address is auto-increment in the hash engine
             * itself, there is no need to increment the register address after  
             * each read from that register.*/
            for (i = 0; i < IX_CRYPTO_PKE_HASH_SHA1_CHAIN_VAR_WLEN; i++)
            {
                /* Read 4 bytes of digest each time */
                *pTempDigest = IX_CRYPTO_PKE_READ_LONG (
                                   pkeHashRegAddr.hashChainVarReg);
                
                /* Advanced the digest pointer */
                pTempDigest++;
            } /* end of for loop (i) */
            
            /* Increment hash request done counter */
            pkeHashStats.pkeHashReqDoneCount++;
            
            /* Release mutex to indicate Hash engine is free now */
            if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
                IX_CRYPTO_ACC_FAST_MUTEX_UNLOCK (pPkeHashFastMutex))
            {
                /* Log message in debugging mode */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_MESSAGE, 
                    IX_OSAL_LOG_DEV_STDOUT,
                    "Failed to release mutex in callback\n", 
                    0, 0, 0, 0, 0, 0);                
            } /* end of for loop (i) */
            
            /* Call client's callback */
            pkeHashStateInfo.hashPerformCallbackFn (
                pkeHashStateInfo.pDigest,
                IX_CRYPTO_ACC_STATUS_SUCCESS);
            
            return;
        
        /* Final hash data block processing, data pointer is set to padded
         * data block pointer instead of original hash data block. Last block
         * of hash data should be padded to be congruent to 448-bit and 
         * modulo 512-bit.
         * Note: the code fall through to default case (middle data block
         * processing) 
         */
        case IX_CRYPTO_PKE_HASH_FINAL_DATA_BLOCK_STATE: 
            /* Log message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "IX_CRYPTO_PKE_HASH_FINAL_DATA_BLOCK_STATE\n", 
                0, 0, 0, 0, 0, 0);
                
            pTempData = (UINT32 *) (pkeHashStateInfo.pPaddedBlockHashData
                            + pkeHashStateInfo.padDataBlockOffset);
            break;
            
        /* Second last data block processing, if padded data blocks count is 2,
         * then the data pointer must point to the padded data block. Otherwise,
         * if the padded data block count is 1 only, then the data pointer
         * will point to the user hash data block pointer
         */
        case IX_CRYPTO_PKE_HASH_SECOND_LAST_DATA_BLOCK_STATE:
            /* Log message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "IX_CRYPTO_PKE_HASH_SECOND_LAST_DATA_BLOCK_STATE : %x\n", 
                pkeHashStateInfo.padDataBlockOffset, 
                0, 0, 0, 0, 0);
                            
            if (0 != pkeHashStateInfo.padDataBlockOffset)
            {
                pTempData = (UINT32 *) pkeHashStateInfo.pPaddedBlockHashData;
            }
            break;
            
        /* Default case is middle hash data block processing, write 512-bit
         * of hash data into hash engine for processing.
         */
        default:
            /* Update hash data block pointer
             * Note: only advance pBlockHashData pointer and not the padded
             * block pointer. Padded block pointer advancement is taken care by
             * the offset
             */
            pkeHashStateInfo.pBlockHashData += IX_CRYPTO_HASH_DATA_BLOCK_LENGTH;       
            break;
    } /* end of switch (pkeHashStateInfo.hashBlockCount) */
    
    /* Update hashBlockCount to indicate remaining of data blocks to  
     * be hashed
     */
    pkeHashStateInfo.hashBlockCount--;

    /* Write hash data into hash engine data register 
     * Note: hashDataReg address is auto-increment in the hash engine
     * itself, there is no need to increment the register address after each 
     * write into that register.
     */
    for (i = 0; i < IX_CRYPTO_PKE_HASH_DATA_BLOCK_WLEN; i++)
    {
        /* Write 4 bytes of data each time */
        IX_CRYPTO_PKE_WRITE_LONG (
            pkeHashRegAddr.hashDataReg,
            *pTempData);
        
        /* Advanced the data pointer */
        pTempData++;                
    } /* end of for loop (i) */
    
    /* Start hashing */
    IX_CRYPTO_PKE_WRITE_LONG (
        pkeHashRegAddr.hashDoReg,
        IX_CRYPTO_PKE_HASH_HDO_START);
} /* end of function ixCryptoPkeHashDoneIsr() */


/**
 * @fn ixCryptoPkeHashShow
 *
 * @brief This function is responsible to show the statistic counter for Hashing
 *        operation and also PKE hash engine status.
 *
 * @return none
 *
 */
void ixCryptoPkeHashShow (void)
{
    /* PKE Hash Statistic display */
    printf ("\n\nCrypto PKE Hash Statistic\n");
    printf ("------------------------------------------\n");
    printf ("PKE Hash Request Count ..................... : %d\n",
    	    pkeHashStats.pkeHashReqCount);
    printf ("PKE Hash Request Done Count ................ : %d\n", 
             pkeHashStats.pkeHashReqDoneCount); 
} /* end of function ixCryptoPkeHashShow() */

#endif /* __ixp46X  */
