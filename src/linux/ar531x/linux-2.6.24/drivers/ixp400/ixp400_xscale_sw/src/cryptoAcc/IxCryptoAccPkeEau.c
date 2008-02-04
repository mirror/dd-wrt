/**
 * @file IxCryptoAccPkeEau.c
 *
 * @date March-18-2006
 *
 * @brief  Source file for EAU
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
#include "IxCryptoAcc.h"
#include "IxCryptoAcc_p.h"
#include "IxCryptoAccPkeEau_p.h"
#include "IxCryptoAccUtilities_p.h"
#include "IxCryptoAccPkeCommon_p.h"

/*
* Variable declarations global to this file only.
*/
static IxOsalFastMutex *pPkeEauMutex;   /**< Protect critical section for EAU */
static UINT32 pkeEauAddr;               /**< Declare EAU address */

static BOOL initDone = FALSE;           /**< TRUE if EAU is initialized */
               
static UINT32 expConfig = 0;            /**< SE and FE option for 
                                            modular exponentiation */

static BOOL chCarry = FALSE;    /**< set if need to check carry/borrow 
                                             status from ram */

static IxCryptoPkeEauStats pkeEauStats;    /**< operation stat counter */
static IxCryptoPkeEauStateInformation pkeEauStateInfo;   /**< operation data */
static IxCryptoPkeEauRamRegAddress pkeEauAddress; /**< EAU reg and ram addresses */

/**
 * @fn void ixCryptoPkeEauRamClear (void)
 *
 * @brief   Set command to clear RAM before operation
 */
IX_CRYPTO_INLINE void 
ixCryptoPkeEauRamClear(void)
{
    UINT32 command;

    /* Assign EAU state to perform operation state */
    pkeEauStateInfo.eauState = IX_CRYPTO_PKE_EAU_PERFORM_OPERATION;

    /* Construct clear RAM command */
    command = (IX_CRYPTO_PKE_EAU_RAM_CLEAR_CMD << IX_CRYPTO_PKE_EAU_CMD_POS) |
              (IX_CRYPTO_PKE_EAU_MAX_N_SIZE << IX_CRYPTO_PKE_EAU_CFG_N_SIZE_POS);
              
    IX_CRYPTO_PKE_WRITE_LONG (pkeEauAddress.eauCommandReg, command);
                       
} /* end of ixCryptoPkeEauRamClear() function */

/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauInit
 * @brief   Initialize EAU (init mutex, mapping, and bind interrupt)
 *
 */
IxCryptoAccStatus
ixCryptoPkeEauInit (void)
{
    UINT32 counter = 0;
    
    /* Check if EAU has been initialized */
    if (initDone) {                  
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (initDone) */
 
    pkeEauAddr = ixCryptoPkeAddressGet (IX_CRYPTO_PKE_EAU);
 
    /* assign eau register and ram address */   
    pkeEauAddress.eauCommandReg = pkeEauAddr + 
        IX_CRYPTO_PKE_EAU_CMD_REG_OFFSET;
    pkeEauAddress.eauStatusReg = pkeEauAddr + 
        IX_CRYPTO_PKE_EAU_STAT_REG_OFFSET;
    pkeEauAddress.eauCounterReg = pkeEauAddr + 
        IX_CRYPTO_PKE_EAU_COUNT_REG_OFFSET;
    pkeEauAddress.eauInterruptReg = pkeEauAddr + 
        IX_CRYPTO_PKE_EAU_INTR_REG_OFFSET;
    
    pkeEauAddress.eauRamReg0 = pkeEauAddr + IX_CRYPTO_PKE_EAU_RAM_REG0_OFFSET;
    pkeEauAddress.eauRamReg1 = pkeEauAddr + IX_CRYPTO_PKE_EAU_RAM_REG1_OFFSET;
    pkeEauAddress.eauRamReg2 = pkeEauAddr + IX_CRYPTO_PKE_EAU_RAM_REG2_OFFSET;
    pkeEauAddress.eauRamReg3 = pkeEauAddr + IX_CRYPTO_PKE_EAU_RAM_REG3_OFFSET;
    pkeEauAddress.eauRamReg4 = pkeEauAddr + IX_CRYPTO_PKE_EAU_RAM_REG4_OFFSET;
    pkeEauAddress.eauRamReg5 = pkeEauAddr + IX_CRYPTO_PKE_EAU_RAM_REG5_OFFSET;
    pkeEauAddress.eauRamReg6 = pkeEauAddr + IX_CRYPTO_PKE_EAU_RAM_REG6_OFFSET;
    pkeEauAddress.eauRamReg7 = pkeEauAddr + IX_CRYPTO_PKE_EAU_RAM_REG7_OFFSET;

    /* Read EAU active bit from status register. Operation is stopped if EAU 
     * is active with 100 read retries */
    while (IX_CRYPTO_PKE_EAU_ACTIVE == (IX_CRYPTO_PKE_EAU_ACTIVE_MASK & 
            IX_CRYPTO_PKE_READ_LONG(pkeEauAddress.eauStatusReg)))
    {       
        /* continue reading if eau is active */
        if (MAX_EAU_ACTIVE_READ < counter++)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR, 
                IX_OSAL_LOG_DEV_STDOUT,
                "EAU is busy.\n", 
                0, 0, 0, 0, 0, 0);
                
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (MAX_EAU_ACTIVE_READ < counter++) */
    } /* end of while loop */
                
    /* Clear the interrupt bit */
    IX_CRYPTO_PKE_WRITE_LONG (pkeEauAddress.eauInterruptReg, 
        IX_CRYPTO_PKE_EAU_CLEAR_INTR);
        
    /* Bind the EAU ISR to interrupt controller */           
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        IX_CRYPTO_ACC_IRQ_BIND (
            IX_OSAL_IXP400_EAU_DONE_IRQ_LVL, 
            (IxOsalVoidFnVoidPtr) ixCryptoPkeEauDoneIsr, 
            NULL))    
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Cannot bind to the interrupt bit in EAU.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* end of if (IX_CRYPTO_ACC_STATUS_FAIL == status) */

    /* Initialize counter for each operation */
    pkeEauStats.modExpReqCount = 0;
    pkeEauStats.bnModReqCount = 0;
    pkeEauStats.bnMulReqCount = 0;
    pkeEauStats.bnAddReqCount = 0;
    pkeEauStats.bnSubReqCount = 0;
    pkeEauStats.totalReqCount = 0;
    pkeEauStats.totalReqDoneCount = 0;
        
    initDone = TRUE;
        
    /* Initialization successfully */
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
} /* end of ixCryptoPkeEauInit() function */


/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauUnInit
 * @brief   UnInitialize EAU (unbind interrupt)
 */
IxCryptoAccStatus
ixCryptoPkeEauUninit (void)
{
    UINT32 timer = 0;
    
    if (initDone) /* if (initialized) */
    {
        /* Check if all request has finished */
        while (pkeEauStats.totalReqCount != pkeEauStats.totalReqDoneCount)
        {
            /* Put the task into sleep mode and wait for the EAU to complete
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
                    "EAU Request is not done!\n", 
                    0, 0, 0, 0, 0, 0);
                
                return IX_CRYPTO_ACC_STATUS_FAIL;
            } /* End of if timer loop */
        } /* end of 
           while (pkeEauStats.totalReqCount != pkeEauStats.totalReqDoneCount) */
        
        /* Unbind ISR */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
            IX_CRYPTO_ACC_IRQ_UNBIND (IX_OSAL_IXP400_EAU_DONE_IRQ_LVL))
        {
            /* Log error message in debugging mode if unbind fail */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Unbind irq fail.\n", 
                0, 0, 0, 0, 0, 0);
            
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* end of if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
           * IX_CRYPTO_ACC_IRQ_UNBIND (IX_OSAL_IXP400_EAU_DONE_IRQ_LVL)) 
           */
        
        initDone = FALSE;
        
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
    } 
    else /* not initialized */
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;
    }/* end of if (initDone) */    
    
}/* end of ixCryptoPkeEauUnInit() function */


/**
 * @fn  void 
 *      ixCryptoPkeCryptoEngineEauExpConfigSet (BOOL enableSE, BOOL enableFE)
 *
 * @brief Set SE and FE option for security level
 *
 * @param enableSE BOOL [in] - enable or disable SE option
 * @param enableFE BOOL [in] - enable or disable FE option
 *
 * @note
 * Client has to be aware of these options changes (per component)
 *
 * @return @li  "none"
 * 
 */
void ixCryptoPkeEauExpConfigSet (BOOL enableSE, BOOL enableFE)
{
    expConfig = (enableSE << IX_CRYPTO_PKE_EAU_CFG_SE_POS) |
                (enableFE << IX_CRYPTO_PKE_EAU_CFG_FE_POS);
    
    /* Log message in debugging mode */
    IX_CRYPTO_ACC_LOG (
        IX_OSAL_LOG_LVL_MESSAGE, 
        IX_OSAL_LOG_DEV_STDOUT,
        "SE and FE bits set to %d and %d respectively.\n", 
        enableSE, enableFE, 0, 0, 0, 0);
} /* end of ixCryptoPkeEauExpConfigSet() function */


/**
 * @fn void ixCryptoPkeEauRamWrite (
 *  UINT32 *pData,
 *  UINT32 dataLen, 
 *  UINT32 addr)
 * @brief   Write to the EAU RAM at the calculated position. MSB is write at the
 *          highest ram location, thus write to ram is backwarded.
 * 
 */
void ixCryptoPkeEauRamWrite ( 
    UINT32 *pData,
    UINT32 dataLen, 
    UINT32 addr)
{
    UINT32 i;
 
    /* Calculate where to insert operand to the RAM accordingly */
    addr += ((dataLen - 1) * ONE_WORD_TO_BYTE); 
         
    /* write to RAM */
    for (i = 0; i < dataLen; i++)
    {
        IX_CRYPTO_PKE_WRITE_LONG (addr, *pData);
        pData++;
        addr -= ONE_WORD_TO_BYTE;
    }/* end of for (i = 0; i < dataLen; i++) */
    
} /* end of ixCryptoPkeEauRamWrite() function  */


/**
 * @fn void ixCryptoPkeEauRamRead (
 *  UINT32 addr)
 * @brief   Read EAU RAM at the specific register address. Result will be stored
 *          at the pointer location provided by the client. If the result length
 *          is larger than the maximum length, the length is set accordingly.
 */
void ixCryptoPkeEauRamRead (UINT32 addr)
{
    UINT32 len = pkeEauStateInfo.resultLen;
    UINT32 *pTempData = NULL;
    UINT32 i = 0;
 
    /* Advance the result pointer to point to the last word, the result is read
     * from RAM fron least significant to most significant order
     */
    pTempData = pkeEauStateInfo.pOpResult->pData
                    + (pkeEauStateInfo.pOpResult->dataLen - 1);
                    
    /* pkeEauStateInfo.resultLen is the length computed according to the
     * operands and operation. 
     * Read result from the specified address register 
     */
    for (i = 0; i < len; i++)
    {
        *pTempData = IX_CRYPTO_PKE_READ_LONG (addr);
        addr += ONE_WORD_TO_BYTE;
        pTempData--;
    } /* end of for (i = 0; i < len; i++) */
    
} /* end of ixCryptoPkeEauRamRead() function */


/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauModExp (
 *  IxCryptoAccPkeEauInOperands *pOperand, 
 *  IxCryptoAccPkeEauOpResult *pResult,
 *  ixCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn)
 * @brief Execute modular exponentiation operation in EAU
 *
 */
IxCryptoAccStatus ixCryptoPkeEauModExp (   
    IxCryptoAccPkeEauInOperands *pOperand, 
    IxCryptoAccPkeEauOpResult *pResult,
    IxCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn)
{
    UINT32 nLen = (pOperand->modExpOpr).N.dataLen;
    UINT32 mLen = (pOperand->modExpOpr).M.dataLen;
    UINT32 eLen = (pOperand->modExpOpr).e.dataLen;
    UINT32 rLen = pResult->dataLen;
    UINT32 *pN = (pOperand->modExpOpr).N.pData;
    UINT32 *pM = (pOperand->modExpOpr).M.pData;
    UINT32 *pe = (pOperand->modExpOpr).e.pData;
    UINT32 nMSB = 0;
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_SUCCESS;
  
    /* Check invalid data pointer */
    if ((NULL == pN) || (NULL == pM) || (NULL == pe))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Invalid operand data pointer. \n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_NULL_PTR_ERR;
    } /* end of if ((NULL == pN) || (NULL == pM) || (NULL == pe)) */
    
    /* Check the length of operands, where 3 <= N <= 64 words, 
     * 1 <= m <= n words and 1 <= e <= n words */
    if ((nLen < IX_CRYPTO_PKE_EAU_OPR_MIN_N_LEN_IN_WORD ||
        nLen > IX_CRYPTO_PKE_EAU_OPR_MAX_LEN_IN_WORD) ||
        (mLen < IX_CRYPTO_PKE_EAU_OPR_MIN_LEN_IN_WORD || 
        mLen > nLen) ||
        (eLen < IX_CRYPTO_PKE_EAU_OPR_MIN_LEN_IN_WORD ||
        eLen > nLen))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Length of operand is out of range.\n", 
            0, 0, 0, 0, 0, 0);
        
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    }

    /* Get MSB bit */
    nMSB = *pN >> IX_CRYPTO_PKE_EAU_MSB_BIT_SHIFT_OFFSET;  

    /* MSB and LSB of N must be 1 */
    if ((IX_CRYPTO_PKE_EAU_MODULO_BIT_SET != 
        (*(pN + (nLen - 1)) & IX_CRYPTO_PKE_EAU_MODULO_BIT_SET)) || 
        (IX_CRYPTO_PKE_EAU_MODULO_BIT_SET != 
        (nMSB & IX_CRYPTO_PKE_EAU_MODULO_BIT_SET)))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Either the MSB or LSB or both MSB and LSB of N is not 1.\n", 
            0, 0, 0, 0, 0, 0
            );
  
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    } /* end of if statement checking MSB and LSB*/ 

    /* Check if the space provided by the client is big enough to store result */
    if (rLen < nLen)
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "The space allocated is not enough to store the result.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    } /* end of if (rLen < nLen) */
    
    /* Value of M has to be smaller than N */ 
    if (nLen == mLen)
    {  
        status = ixCryptoPkeEauOperandCompare (pN, pM, nLen);
           
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            return status;
        }
    } /* end of if (nLen == mLen) */

    /* Value of e has to be smaller than N */
    if (eLen == nLen)
    {           
        status = ixCryptoPkeEauOperandCompare (pN, pe, nLen);
        
        if (IX_CRYPTO_ACC_STATUS_SUCCESS != status)
        {
            return status;
        }
    } /* end of if (eLen == nLen) */
      
    /* Store operation information to a struct */
    pkeEauStateInfo.clientEauDoneCB = eauPerformCallbackFn; 
    pkeEauStateInfo.pOpResult = pResult;
    pkeEauStateInfo.resultLen = nLen;
    pkeEauStateInfo.pOpr = pOperand;
    pkeEauStateInfo.eauOperation = IX_CRYPTO_ACC_OP_EAU_MOD_EXP;
    
    /* Form the command with operation mask, exponential config, and N size */
    pkeEauStateInfo.command = 
        ((IX_CRYPTO_PKE_EAU_MOD_EXP_CMD << IX_CRYPTO_PKE_EAU_CMD_POS) | 
          expConfig | (nLen << IX_CRYPTO_PKE_EAU_CFG_N_SIZE_POS));

    /* execute clear RAM operation */
    ixCryptoPkeEauRamClear();
    
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
} /* end of ixCryptoAccPkeEauModExp() function */        


/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauBNMulAddSub (
 *  IxCryptoAccPkeEauInOperands *operand, 
 *  IxCryptoAccPkeEauOpResult *pResult, 
 *  IxCryptoAccPkeEauOperation operation,
 *  ixCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn);
 * @brief Execute large number multiplication, addition or subtraction operation
 *
 */
IxCryptoAccStatus ixCryptoPkeEauBNMulAddSub (   
    IxCryptoAccPkeEauInOperands *pOperand, 
    IxCryptoAccPkeEauOpResult *pResult,
    IxCryptoAccPkeEauOperation operation,
    IxCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn)
{
    UINT32 nSize = 0;
    UINT32 rLen = pResult->dataLen;
    UINT32 aLen = (pOperand->bnAddSubMulOpr).A.dataLen;
    UINT32 bLen = (pOperand->bnAddSubMulOpr).B.dataLen;
    UINT32 *pA = (pOperand->bnAddSubMulOpr).A.pData;
    UINT32 *pB = (pOperand->bnAddSubMulOpr).B.pData;
            
    /* Check invalid data length and data pointer */
    if ((NULL == pA) || (NULL == pB))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Invalid operand data pointer.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_NULL_PTR_ERR;
    }
    
    /* Check the length of A and B (between 1 word and 64 words) */  
    if ((aLen < IX_CRYPTO_PKE_EAU_OPR_MIN_LEN_IN_WORD ||
        aLen > IX_CRYPTO_PKE_EAU_OPR_MAX_LEN_IN_WORD) ||
        (bLen < IX_CRYPTO_PKE_EAU_OPR_MIN_LEN_IN_WORD ||
        bLen > IX_CRYPTO_PKE_EAU_OPR_MAX_LEN_IN_WORD))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Length of operand not between 1 word and 64 words.\n", 
            0, 0, 0, 0, 0, 0);
        
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    }
     
    /* Check the size of the result provided by client 
    *  Result of multiplication should be addition of the operands A and B
    */
    if (((rLen < aLen) || (rLen < bLen)) || 
        ((operation == IX_CRYPTO_ACC_OP_EAU_BN_MUL) && (rLen < aLen + bLen)))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "The space allocated is not enough to store the result.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    }

    /* Store operation information to a struct */
    pkeEauStateInfo.pOpResult = pResult;
    pkeEauStateInfo.clientEauDoneCB = eauPerformCallbackFn;
    pkeEauStateInfo.pOpr = pOperand;
            
    /* Get the largest size */
    if (aLen >= bLen)    
    {
        nSize = aLen;
        pkeEauStateInfo.resultLen = aLen;
    }
    else
    {
        nSize = bLen;
        pkeEauStateInfo.resultLen = bLen;
    } /* end of if (aLen > bLen) */
    
    /* N operand size to be configured in the command has to be > 2 */
    if (IX_CRYPTO_PKE_EAU_OPR_MIN_N_LEN_IN_WORD > nSize)
    {
        nSize = IX_CRYPTO_PKE_EAU_OPR_MIN_N_LEN_IN_WORD;
    }
    
    /* Form the command for each operation */
    switch (operation)
    {
        /* multiplication */
        case IX_CRYPTO_ACC_OP_EAU_BN_MUL:
            pkeEauStateInfo.eauOperation = IX_CRYPTO_ACC_OP_EAU_BN_MUL;
            pkeEauStateInfo.resultLen = aLen + bLen;
            pkeEauStateInfo.command =
                ((IX_CRYPTO_PKE_EAU_BN_MUL_CMD << IX_CRYPTO_PKE_EAU_CMD_POS) |
                (nSize << IX_CRYPTO_PKE_EAU_CFG_N_SIZE_POS));
            break;
        
        /* addition */
        case IX_CRYPTO_ACC_OP_EAU_BN_ADD:       
            pkeEauStateInfo.eauOperation = IX_CRYPTO_ACC_OP_EAU_BN_ADD;
            pkeEauStateInfo.command = 
                ((IX_CRYPTO_PKE_EAU_BN_ADD_CMD << IX_CRYPTO_PKE_EAU_CMD_POS) | 
                (nSize << IX_CRYPTO_PKE_EAU_CFG_N_SIZE_POS));
            
            /* Keep a pointer to store carry bit position */
            if (IX_CRYPTO_PKE_EAU_OPR_MIN_N_LEN_IN_WORD > 
                pkeEauStateInfo.resultLen)
            {
                pkeEauStateInfo.carryOffset = 
                    pkeEauStateInfo.resultLen * ONE_WORD_TO_BYTE;
                chCarry = TRUE;
            }
            break;
            
        /* subtraction */
        case IX_CRYPTO_ACC_OP_EAU_BN_SUB:               
            pkeEauStateInfo.eauOperation = IX_CRYPTO_ACC_OP_EAU_BN_SUB;
            pkeEauStateInfo.command = 
                ((IX_CRYPTO_PKE_EAU_BN_SUB_CMD << IX_CRYPTO_PKE_EAU_CMD_POS) | 
                (nSize << IX_CRYPTO_PKE_EAU_CFG_N_SIZE_POS));
            break;
            
        default:
            return IX_CRYPTO_ACC_STATUS_OPERATION_NOT_SUPPORTED;
    } /* end of switch (eauOperation) */ 
        
    /* execute clear RAM operation */
    ixCryptoPkeEauRamClear();
    
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
} /* end of ixCryptoPkeEauBNMulAddSub() function */


/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauBNMod (   
 *  IxCryptoAccPkeEauInOperands *operand, 
 *  IxCryptoAccPkeEauOpResult *pResult,
 *  ixCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn)
 * @brief Execute large number modular reduction operation in EAU
 *
 */
IxCryptoAccStatus ixCryptoPkeEauBNMod (   
    IxCryptoAccPkeEauInOperands *pOperand, 
    IxCryptoAccPkeEauOpResult *pResult,
    IxCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn)
{
    UINT32 nMSB = 0;
    UINT32 aLen = (pOperand->bnModOpr).A.dataLen;
    UINT32 nLen = (pOperand->bnModOpr).N.dataLen;
    UINT32 *pA = (pOperand->bnModOpr).A.pData;
    UINT32 *pN = (pOperand->bnModOpr).N.pData;
        
    /* Check invalid data length and data pointer */
    if ((NULL == pA) || (NULL == pN))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Invalid operand data pointer. \n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_NULL_PTR_ERR;
    }
       
    /* Check the length of A and N. N is between 3 word and 64 words, A can be
     * between 1 word and 128 word, but sizeof(A) <= 2 * sizeof(N).
     */
    if ((nLen < IX_CRYPTO_PKE_EAU_OPR_MIN_N_LEN_IN_WORD ||
        nLen > IX_CRYPTO_PKE_EAU_OPR_MAX_LEN_IN_WORD)   ||
        (aLen < IX_CRYPTO_PKE_EAU_OPR_MIN_LEN_IN_WORD   || 
        aLen > 2 * nLen))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Length of operand is out of range.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    }
    
    /* Check if the space provided by client is enough */
    if (pResult->dataLen < nLen)
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "The space allocated is not enough to store the result.\n", 
            0, 0, 0, 0, 0, 0);
            
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    }
    
    /* Get MSB bit */
    nMSB = *pN >> IX_CRYPTO_PKE_EAU_MSB_BIT_SHIFT_OFFSET;  

    /* MSB of N must be 1 */
    if (IX_CRYPTO_PKE_EAU_MODULO_BIT_SET != 
        (nMSB & IX_CRYPTO_PKE_EAU_MODULO_BIT_SET))
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "The MSB of N is not 1.\n", 
            0, 0, 0, 0, 0, 0
            );
            
        return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
    } /* end of if statement checking LSB and MSB */   
        
    /* Store operation information to a struct */
    pkeEauStateInfo.pOpr = pOperand;
    pkeEauStateInfo.pOpResult = pResult;
    pkeEauStateInfo.clientEauDoneCB = eauPerformCallbackFn;
    pkeEauStateInfo.resultLen = nLen;
    pkeEauStateInfo.eauOperation = IX_CRYPTO_ACC_OP_EAU_BN_MOD;
    
    /* Form the command register option by shifting nSize to bit 16:22 */
    pkeEauStateInfo.command = 
        ((IX_CRYPTO_PKE_EAU_MOD_RED_CMD << IX_CRYPTO_PKE_EAU_CMD_POS) | 
        (nLen << IX_CRYPTO_PKE_EAU_CFG_N_SIZE_POS));
         
    /* execute clear RAM operation */
    ixCryptoPkeEauRamClear();
    
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
} /* end of ixCryptoPkeEauBNMod() function */

/**
 * @fn void ixCryptoPkeEauDoneIsr (void)
 * @brief   ISR function invoked when EAU has completed the operation, including
 *          calling client callback function.
 *
 */
void ixCryptoPkeEauDoneIsr (void)
{
    /* Clear the interrupt bit */
    IX_CRYPTO_PKE_WRITE_LONG (pkeEauAddress.eauInterruptReg, 
        IX_CRYPTO_PKE_EAU_CLEAR_INTR);

    /* Check which state eau is in */
    switch (pkeEauStateInfo.eauState)
    {
        /* RAM clear operation done, execute EAU operation */
        case IX_CRYPTO_PKE_EAU_PERFORM_OPERATION:
        
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "IX_CRYPTO_PKE_EAU_PERFORM_OPERATION state \n",  
                0, 0, 0, 0, 0, 0);
            
            /* Change to next state to wait for operation done and get result */
            pkeEauStateInfo.eauState = IX_CRYPTO_PKE_EAU_PERFORM_DONE;
            
            /* Increment EAU total request counter */
            pkeEauStats.totalReqCount++;
            
            /* Execute EAU operation by writing into RAM and command register */
            ixCryptoPkeEauOperationPerform();
            break;    
            
        /* operation done and read result from RAM */
        case IX_CRYPTO_PKE_EAU_PERFORM_DONE:
        
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "IX_CRYPTO_PKE_EAU_PERFORM_DONE state \n",  
                0, 0, 0, 0, 0, 0);
            
            /* Increment EAU total request done counter */
            pkeEauStats.totalReqDoneCount++;
            
            /* Read result from RAM */
            ixCryptoPkeEauResultRead();
            
            break;        
            
        default :         
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR, 
                IX_OSAL_LOG_DEV_STDERR,
                "IX_CRYPTO_PKE_EAU default state, should not enter this state.\n",  
                0, 0, 0, 0, 0, 0);
            break;       
    } /* end of switch (pkeEauStateInfo.eauState) */
} /* end of ixCryptoPkeEauDoneIsr() function */

/**
 * @fn void ixCryptoPkeEauOperationPerform (void)
 *
 * @brief   Call ixCryptoPkeEauRamWrite that writes operands to the associate 
 *          RAM registers and executes operation
 */
void ixCryptoPkeEauOperationPerform (void)
{
    /* write operands to RAM */
    switch (pkeEauStateInfo.eauOperation)
    {
         /* modular exponentiation */ 
        case IX_CRYPTO_ACC_OP_EAU_MOD_EXP:
    
            /* Write the operands (M, N and e) to RAM accordingly */
            ixCryptoPkeEauRamWrite (
                (pkeEauStateInfo.pOpr->modExpOpr).M.pData, 
                (pkeEauStateInfo.pOpr->modExpOpr).M.dataLen, 
                pkeEauAddress.eauRamReg3);
        
            ixCryptoPkeEauRamWrite (
                (pkeEauStateInfo.pOpr->modExpOpr).e.pData, 
                (pkeEauStateInfo.pOpr->modExpOpr).e.dataLen, 
                pkeEauAddress.eauRamReg0);
            
            ixCryptoPkeEauRamWrite (
                (pkeEauStateInfo.pOpr->modExpOpr).N.pData, 
                (pkeEauStateInfo.pOpr->modExpOpr).N.dataLen, 
                pkeEauAddress.eauRamReg5);
    
            break;    
            
        /* big number addition, subtraction and multiplication, code fall 
         * through to case IX_CRYPTO_ACC_OP_EAU_BN_MUL 
         */ 
        case IX_CRYPTO_ACC_OP_EAU_BN_ADD:
        case IX_CRYPTO_ACC_OP_EAU_BN_SUB:
        case IX_CRYPTO_ACC_OP_EAU_BN_MUL:
        
            /* Write the operands (A, B) to RAM accordingly */
            ixCryptoPkeEauRamWrite (
                (pkeEauStateInfo.pOpr->bnAddSubMulOpr).A.pData, 
                (pkeEauStateInfo.pOpr->bnAddSubMulOpr).A.dataLen, 
                pkeEauAddress.eauRamReg0);
                
            ixCryptoPkeEauRamWrite (
                (pkeEauStateInfo.pOpr->bnAddSubMulOpr).B.pData, 
                (pkeEauStateInfo.pOpr->bnAddSubMulOpr).B.dataLen, 
                pkeEauAddress.eauRamReg1);
      
            break;
        
        /* big number modular reduction */
        case IX_CRYPTO_ACC_OP_EAU_BN_MOD:
        
            /* Write the operands (A, N) to RAM accordingly */
            ixCryptoPkeEauRamWrite (
                (pkeEauStateInfo.pOpr->bnModOpr).A.pData, 
                (pkeEauStateInfo.pOpr->bnModOpr).A.dataLen, 
                pkeEauAddress.eauRamReg2);
    
            ixCryptoPkeEauRamWrite (
                (pkeEauStateInfo.pOpr->bnModOpr).N.pData,
                (pkeEauStateInfo.pOpr->bnModOpr).N.dataLen,
                pkeEauAddress.eauRamReg5);
            break;
            
        default :
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR, 
                IX_OSAL_LOG_DEV_STDERR,
                "Invalid EAU operation.\n",  
                0, 0, 0, 0, 0, 0);
            break;       
    } /* end of switch (pkeEauStateInfo.eauOperation)*/
    
    /* Write command to the register to start operation */
    IX_CRYPTO_PKE_WRITE_LONG (pkeEauAddress.eauCommandReg, pkeEauStateInfo.command);   
    
} /* end of ixCryptoPkeEauOperationPerform() function */


/**
 * @fn void ixCryptoPkeEauResultRead (void)
 *
 * @brief   Read result from the RAM by calling ixCryptoPkeEauRamRead according 
 *          to the operation
 */
void ixCryptoPkeEauResultRead (void)
{
    BOOL carryOrBorrow = FALSE;
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_SUCCESS;
    pPkeEauMutex = ixCryptoPkeFastMutexGet(IX_CRYPTO_PKE_EAU);
    
#ifndef NDEBUG   
    pkeEauStateInfo.opTime = 
        IX_CRYPTO_PKE_READ_LONG (pkeEauAddress.eauCounterReg);
#endif           
        
    /*  When operation is done, read result according to operation, and 
        increment statistic for the specified operation */
    switch (pkeEauStateInfo.eauOperation)
    {      
        /* modular exponentiation */    
        case IX_CRYPTO_ACC_OP_EAU_MOD_EXP:      

            ixCryptoPkeEauRamRead (pkeEauAddress.eauRamReg3);
            pkeEauStats.modExpReqCount++;            
            break;
        
        /* multiplication */        
        case IX_CRYPTO_ACC_OP_EAU_BN_MUL:       
                           
            ixCryptoPkeEauRamRead (pkeEauAddress.eauRamReg2);
            pkeEauStats.bnMulReqCount++;
            break;
        
        /* addition */        
        case IX_CRYPTO_ACC_OP_EAU_BN_ADD:               
            /* Check carry bit. If chCarry flag is set, 
             * determine carry at (result register + resultLen) location.
             * Otherwise, just read from the status register. 
             */            
            if (chCarry)
            {
                if (0 != IX_CRYPTO_PKE_READ_LONG (pkeEauAddress.eauRamReg2 + 
                    pkeEauStateInfo.carryOffset))
                {
                    carryOrBorrow = TRUE;
                }
            } /* else-if (chCarry) */
            else
            {
                if (IX_CRYPTO_PKE_READ_LONG (pkeEauAddress.eauStatusReg) & 
                    IX_CRYPTO_PKE_EAU_CARRY_BORROW_MASK) 
                {
                    carryOrBorrow = TRUE;
                }
            } /* end of if (chCarry) */
            
            chCarry = FALSE;
            pkeEauStats.bnAddReqCount++;
            ixCryptoPkeEauRamRead (pkeEauAddress.eauRamReg2); 
            break;
        
        /* modular reduction */        
        case IX_CRYPTO_ACC_OP_EAU_BN_MOD:       
        
            pkeEauStats.bnModReqCount++;
            ixCryptoPkeEauRamRead (pkeEauAddress.eauRamReg2);            
            break;
                
        /* subtraction */        
        case IX_CRYPTO_ACC_OP_EAU_BN_SUB:       
                
            if (IX_CRYPTO_PKE_READ_LONG (pkeEauAddress.eauStatusReg) & 
                IX_CRYPTO_PKE_EAU_CARRY_BORROW_MASK) 
            {
                carryOrBorrow = TRUE;
            }
           
            ixCryptoPkeEauRamRead (pkeEauAddress.eauRamReg2);
            pkeEauStats.bnSubReqCount++;
            break;              

        default:
            /* Log message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_ERROR, 
                IX_OSAL_LOG_DEV_STDERR,
                "Operation not supported in Callback.\n", 
                0, 0, 0, 0, 0, 0);
                
                status = IX_CRYPTO_ACC_STATUS_FAIL; 
            break;                        
    } /* end of switch (pkeEauStateInfo.eauOperation) */
        
    /* Unlock fast mutex */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
        IX_CRYPTO_ACC_FAST_MUTEX_UNLOCK (pPkeEauMutex))
    {
        /* Log message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Cannot unlock fast mutex.\n", 
            0, 0, 0, 0, 0, 0);
    } /* end of if (IX_CRYPTO_ACC_STATUS_SUCCESS != 
       * IX_CRYPTO_ACC_FAST_MUTEX_UNLOCK (pPkeEauMutex)
       */
     
    /* Log message when finished operation in debugging mode */
    IX_CRYPTO_ACC_LOG (
        IX_OSAL_LOG_LVL_MESSAGE, 
        IX_OSAL_LOG_DEV_STDOUT,
        "Operation cycle count ................... : %u. \n",  
        pkeEauStateInfo.opTime, 
        0, 0, 0, 0, 0);
           
    /* call client callback function */
    pkeEauStateInfo.clientEauDoneCB (pkeEauStateInfo.eauOperation,
        pkeEauStateInfo.pOpResult, carryOrBorrow, status); 

} /* end of ixCryptoPkeEauResultRead() function */


/**
 * @fn void ixCryptoPkeEauShow (void)
 * @brief   Show counter for each operation being performed by EAU
 * 
 */
void ixCryptoPkeEauShow(void)
{
    /* Eau Statistic display */
    printf ("\n\nCrypto EAU Performance Statistic\n");
    printf ("------------------------------------------\n");
    printf ("Modular exponentiation ..................... : %d\n",
    	    pkeEauStats.modExpReqCount);
    printf ("Large number multiplication ................ : %d\n", 
    	    pkeEauStats.bnMulReqCount);
    printf ("Large number addition ...................... : %d\n", 
            pkeEauStats.bnAddReqCount);
    printf ("Large number modular reduction ............. : %d\n", 
            pkeEauStats.bnModReqCount);
    printf ("Large number subtraction ................... : %d\n", 
            pkeEauStats.bnSubReqCount);
    printf ("Total EAU operation request ................ : %d\n",
    	    pkeEauStats.totalReqCount);
    printf ("Total EAU operation request done ........... : %d\n",
    	    pkeEauStats.totalReqDoneCount);

} /* end of ixCryptoPkeEauShow() function */

/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauOperandCompare (
 *      UINT32 *pOprN, 
 *      UINT32 *pOprMorE, 
 *      UINT32 length)
 *
 * @brief Compare operands (N with M or N with e) for modular exponential
 */
IxCryptoAccStatus ixCryptoPkeEauOperandCompare (
    UINT32 *pOprN, 
    UINT32 *pOprMorE,
    UINT32 length)
{
    UINT32 cmpLoop = 0;
        
    for (cmpLoop = 0; cmpLoop < length; cmpLoop++)
    {
        if (pOprN[cmpLoop] != pOprMorE[cmpLoop])
        {
            if (pOprN[cmpLoop] > pOprMorE[cmpLoop])
            {
                return IX_CRYPTO_ACC_STATUS_SUCCESS;
            }
            else
            {
                /* Log error message in debugging mode */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_ERROR, 
                    IX_OSAL_LOG_DEV_STDOUT,
                    "M or e value is bigger than N value.\n", 
                    0, 0, 0, 0, 0, 0);
                      
                return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;
            } /* end of if (pOprN[cmpLoop] > pOprMorE[cmpLoop]) */
        } /* end of if (pOprN[cmpLoop] != pOprMorE[cmpLoop]) */        
    } /* end of for (cmpLoop = 0; cmpLoop < length; cmpLoop++) */ 
    
    /* if both operands are equal, return error */
    return IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR;       
} /* end of ixCryptoPkeEauOperandCompare() function*/

#endif /* __ixp46X */
