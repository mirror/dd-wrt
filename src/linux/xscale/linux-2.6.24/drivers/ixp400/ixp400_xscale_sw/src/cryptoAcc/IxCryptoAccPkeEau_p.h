/**
 * @file IxCryptoAccPkeEau_p.h
 *
 * @date March-18-2005
 *
 * @brief  Header file for EAU module
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

#if defined(__ixp46X)     /* PKE codes only applicable to IXP46X platform */

#ifndef IXCRYPTOACCPKEEAU_P_H
#define IXCRYPTOACCPKEEAU_P_H

#define IX_OSAL_ENFORCED_LE_AC_MAPPING      /* Force mapping to AC when in LE */

#include "IxOsal.h"
#include "IxCryptoAcc.h"
#include "IxCryptoAcc_p.h"

#define OPERAND_SIZE_N                  8       /* Operand size in words */
#define ONE_WORD_TO_BYTE                4       /* 1 word to byte */

#define MAX_EAU_ACTIVE_READ             100     /* read eau status max count */

#define IX_CRYPTO_PKE_EAU_OPR_MIN_LEN_IN_WORD       1   /* Min operand length */
#define IX_CRYPTO_PKE_EAU_OPR_MIN_N_LEN_IN_WORD     3   /* Min N length */
#define IX_CRYPTO_PKE_EAU_OPR_MAX_LEN_IN_WORD       64  /* Max operand length */
#define IX_CRYPTO_PKE_EAU_RESULT_MAX_LEN_IN_WORD    64  /* Max result length */

/* EAU offsets for RAM registers */
#define IX_CRYPTO_PKE_EAU_RAM_REG0_OFFSET   0x0
#define IX_CRYPTO_PKE_EAU_RAM_REG1_OFFSET   0x100
#define IX_CRYPTO_PKE_EAU_RAM_REG2_OFFSET   0x200
#define IX_CRYPTO_PKE_EAU_RAM_REG3_OFFSET   0x300
#define IX_CRYPTO_PKE_EAU_RAM_REG4_OFFSET   0x400
#define IX_CRYPTO_PKE_EAU_RAM_REG5_OFFSET   0x500
#define IX_CRYPTO_PKE_EAU_RAM_REG6_OFFSET   0x600
#define IX_CRYPTO_PKE_EAU_RAM_REG7_OFFSET   0x700

/*  EAU registers offset */
#define IX_CRYPTO_PKE_EAU_CMD_REG_OFFSET    0x2000  /* command register */

#define IX_CRYPTO_PKE_EAU_STAT_REG_OFFSET \
    (IX_CRYPTO_PKE_EAU_CMD_REG_OFFSET + 0x4)        /* status register  */ 


#define IX_CRYPTO_PKE_EAU_COUNT_REG_OFFSET \
    (IX_CRYPTO_PKE_EAU_CMD_REG_OFFSET + 0x8)        /* count register   */
    
#define IX_CRYPTO_PKE_EAU_INTR_REG_OFFSET \
    (IX_CRYPTO_PKE_EAU_CMD_REG_OFFSET + 0xC)        /* interrupt register */

/*
 *  Table showing where operands and result are stored in RAM
 *
 *   Address        
 * Offset from RAM  modExp          BN_MOD          BN_MUL      BN_ADD/SUB 
 *   -----------------------------------------------------------------------
 * 700H - 7FFH      unused          unused          unused      unused  Reg7
 * 600H - 6FFH      unused          unused          unused      unused  Reg6
 * 500H - 5FFH      Modulus(n)      Modulus(n)      unused      unused  Reg5
 * 400H - 4FFH      temp            temp            nused       unused  Reg4
 * 300H - 3FFH      Op1(M)/Result   A_hi            Result_hi   unused  Reg3
 * 200H - 2FFH      Temp            A_lo/Result     Result_lo   Result  Reg2
 * 100H - 1FFH      Temp            temp            B           B       Reg1
 * 000H - 0FFH      Exponent (e)    temp            A           A       Reg0
 *
 *
 *  EAU registers and masking
 *
 *  Command register
 *  Bit 31:30   -   reserved
 *      29      -   SE
 *      28      -   FE
 *      27:24   -   operation command (0000 - Idle, 
 *                  0001 Modular exponentiation (A^e mod N),
 *                  0010 Large Number Multiplication (A*B),
 *                  0011 Large Number Addition (A+B)
 *                  0100 Modular Reduction (A mod N)
 *                  0101 Large Number Subtraction (A-B)
 *                  0110 Clear EAU RAM
 *      23      -   reserved
 *      22:16   -   N size
 *      15      -   reserved
 *      14:12   -   Destination field (register number)
 *      11      -   reserved
 *      10:8    -   source field (register number)
 *      7:0     -   fill value
 */
#define IX_CRYPTO_PKE_EAU_IDLE_CMD          0x0         /* all the commands */
#define IX_CRYPTO_PKE_EAU_MOD_EXP_CMD       0x1
#define IX_CRYPTO_PKE_EAU_BN_MUL_CMD        0x2
#define IX_CRYPTO_PKE_EAU_BN_ADD_CMD        0x3
#define IX_CRYPTO_PKE_EAU_MOD_RED_CMD       0x4
#define IX_CRYPTO_PKE_EAU_BN_SUB_CMD        0x5
#define IX_CRYPTO_PKE_EAU_RAM_CLEAR_CMD     0x6
#define IX_CRYPTO_PKE_EAU_MAX_N_SIZE        0x40
#define IX_CRYPTO_PKE_EAU_MODULO_BIT_SET    0x1         /* MSB and LSB of N=1 */
#define IX_CRYPTO_PKE_EAU_MSB_BIT_SHIFT_OFFSET  31      /* Shift offset of MSB */

#define IX_CRYPTO_PKE_EAU_CMD_POS           24
#define IX_CRYPTO_PKE_EAU_CFG_FE_POS        28          /* SE and FE position */      
#define IX_CRYPTO_PKE_EAU_CFG_SE_POS        29  
#define IX_CRYPTO_PKE_EAU_CFG_N_SIZE_POS    16          /* bit 16:22 */


/*
 *  Status register
 *  Bit 31:5    -   reserved
 *      4       -   Equal bit for subtraction
 *      3       -   Carry/Borrow bit for addition and subtraction
 *      2       -   reserved
 *      1       -   EAU done bit (zero to clear)
 *      0       -   EAU status bit (zero is inactive, one is active)
 *
 */

#define IX_CRYPTO_PKE_EAU_ACTIVE_MASK       (1 << 0)    /* bit 0 */
#define IX_CRYPTO_PKE_EAU_DONE_MASK         (1 << 1)    /* bit 1 */
#define IX_CRYPTO_PKE_EAU_CARRY_BORROW_MASK (1 << 3)    /* bit 3 */
#define IX_CRYPTO_PKE_EAU_ACTIVE            0x1
 
/*
 *  Interrupt register
 *  Bit 31:1    -   reserved
 *      0       -   interrupt bit (0 = inactive, 1 = active, write 1 to clear)
 *
**/
#define IX_CRYPTO_PKE_EAU_CLEAR_INTR        0x1     /* clear interrupt bit */

/**
 * @enum    IxCryptoPkeEauState
 *
 * @brief   EAU (Exponentiation Acceleration Unit) operation states. 
 *      
 
 * @note    There are two states:
 *      1. IX_CRYPTO_PKE_EAU_PERFORM_OPERATION - Ram clear done and execute EAU 
 *                                               operation
 *      2. IX_CRYPTO_PKE_EAU_PERFORM_DONE - collect result and call 
 *                                          client's callback
 *
 */ 
typedef enum 
{
    IX_CRYPTO_PKE_EAU_PERFORM_OPERATION,    /* perform operation */
    IX_CRYPTO_PKE_EAU_PERFORM_DONE,         /* perform done */
    IX_CRYPTO_PKE_INVALID_STATE             /* Invalid EAU operation state */
} IxCryptoPkeEauState;

/**
 * @struct  IxCryptoPkeEauStats
 * @brief   Structure storing the counter of each operation performed by EAU and
 *          the time of an operation.
 */
typedef struct {
    UINT32 modExpReqCount;
    UINT32 bnModReqCount;
    UINT32 bnMulReqCount;
    UINT32 bnAddReqCount;
    UINT32 bnSubReqCount;
    UINT32 totalReqCount;
    UINT32 totalReqDoneCount;
} IxCryptoPkeEauStats;

/**
 * @struct  IxCryptoPkeEauStateInformation
 * @brief   Structure storing client callback function (when operation is done),
 *          operation state, operation being performed, operands pointer, 
 *          result pointer, the carry offset in result ram, the result length,
 *          operation time, and command to execute the operation.
 */
typedef struct {
    IxCryptoAccPkeEauPerformCompleteCallback clientEauDoneCB;
    IxCryptoPkeEauState eauState;
    IxCryptoAccPkeEauOperation eauOperation;                                                        
    IxCryptoAccPkeEauInOperands *pOpr;
    IxCryptoAccPkeEauOpResult *pOpResult;
    UINT32 carryOffset;
    UINT32 resultLen;
    UINT32 opTime;
    UINT32 command;
} IxCryptoPkeEauStateInformation;

/**
 * @struct pkeEauAddress
 *
 * @brief EAU (Exponentiation Acceleration Unit) registers and ram addresses  
 *
 */ 
typedef struct 
{
    UINT32 eauCommandReg;       /* Command register */
    UINT32 eauStatusReg;        /* Status register */
    UINT32 eauCounterReg;       /* Counter register */
    UINT32 eauInterruptReg;     /* Interrupt register */
    UINT32 eauRamReg0;          /* RAM reg0 */
    UINT32 eauRamReg1;          /* RAM reg1 */
    UINT32 eauRamReg2;          /* RAM reg2 */
    UINT32 eauRamReg3;          /* RAM reg3 */
    UINT32 eauRamReg4;          /* RAM reg4 */
    UINT32 eauRamReg5;          /* RAM reg5 */
    UINT32 eauRamReg6;          /* RAM reg6 */
    UINT32 eauRamReg7;          /* RAM reg7 */
} IxCryptoPkeEauRamRegAddress;


/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauModExp (
 *  IxCryptoAccPkeEauInOperands *pOperand, 
 *  IxCryptoAccPkeEauOpResult *pResult,
 *  ixCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn))
 *
 * @brief Execute modular exponentiation operation in EAU
 *
 * @param *pOperand IxCryptoAccPkeEauInOperands [in] - data pointer for operands
 * @param *pResult IxCryptoAccPkeEauOpResult [in] - data pointer for result
 * @param eauPerformCallbackFn ixCryptoAccPkeEauPerformCompleteCallback [in|out]
 *        - eau done callback function pointer
 *
 * Global Data  : 
 *  - pOpResult     - pointer to the operation result
 *  - eauOperation  - operation being perform
 *
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operation performed successfully
 * @return @li  IX_CRYPTO_ACC_STATUS_FAIL - Operation failed
 * @return @li  IX_CRYPTO_ACC_STATUS_NULL_PTR_ERR - Operation return null pointer
 *                                                  error
 * @return @li  IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR - Opeartion return operand 
 *                                                      is out of range
 *
 */
IxCryptoAccStatus 
ixCryptoPkeEauModExp (   
    IxCryptoAccPkeEauInOperands *pOperand, 
    IxCryptoAccPkeEauOpResult *pResult,
    IxCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn);

/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauBNMulAddSub (
 *  IxCryptoAccPkeEauInOperands *pOperand, 
 *  IxCryptoAccPkeEauOpResult *pResult, 
 *  IxCryptoAccPkeEauOperation operation,
 *  ixCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn);
 *
 * @brief Execute large number multiplication, addition or subtraction operation
 *
 * @param *pOperand IxCryptoAccPkeEauInOperands [in] - data pointer for operands
 * @param *pResult IxCryptoAccPkeEauOpResult [in] - data pointer for result
 * @param operation IxCryptoAccPkeEauOperation [in] - operation to be performed 
 * @param eauPerformCallbackFn ixCryptoAccPkeEauPerformCompleteCallback [in|out]
 *        - eau done callback function pointer
 *
 * Global Data  : 
 *  - pOpResult     - pointer to the operation result
 *  - eauOperation  - operation being perform
 *
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operation performed successfully
 * @return @li  IX_CRYPTO_ACC_STATUS_FAIL - Operation failed
 *
 */ 
IxCryptoAccStatus 
ixCryptoPkeEauBNMulAddSub (   
    IxCryptoAccPkeEauInOperands *pOperand, 
    IxCryptoAccPkeEauOpResult *pResult, 
    IxCryptoAccPkeEauOperation operation,
    IxCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn);

/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauBNMod (   
 *  IxCryptoAccPkeEauInOperands *pOperand, 
 *  IxCryptoAccPkeEauOpResult *pResult,
 *  ixCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn)
 *
 * @brief Execute large number modular reduction operation in EAU
 *
 * @param *pOperand IxCryptoAccPkeEauInOperands [in] - data pointer for operands
 * @param *pResult IxCryptoAccPkeEauOpResult [in] - data pointer for result
 * @param eauPerformCallbackFn ixCryptoAccPkeEauPerformCompleteCallback [in|out]
 *        - eau done callback function pointer
 *
 * Global Data  : 
 *  - pOpResult     - pointer to the operation result
 *  - eauOperation  - operation being perform
 *
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operation performed successfully
 * @return @li  IX_CRYPTO_ACC_STATUS_FAIL - Operation failed
 *
 */ 
IxCryptoAccStatus 
ixCryptoPkeEauBNMod (   
    IxCryptoAccPkeEauInOperands *pOperand, 
    IxCryptoAccPkeEauOpResult *pResult,
    IxCryptoAccPkeEauPerformCompleteCallback eauPerformCallbackFn);

/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauInit (void)
 *
 * @brief Initialize EAU
 *
 * @param "none"
 *
 * Global Data  : 
 *  - eauAddr     - EAU register address
 *  - eauLock     - fast mutex lock
 *
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operation performed successfully
 * @return @li  IX_CRYPTO_ACC_STATUS_FAIL - Operation failed
 *
 */
IxCryptoAccStatus
ixCryptoPkeEauInit (void);

/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauUnInit (void)
 *
 * @brief Uninitialize EAU
 *
 * @param "none" 
 *
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operation performed successfully
 * @return @li  IX_CRYPTO_ACC_STATUS_FAIL - Operation failed
 *
 */
IxCryptoAccStatus
ixCryptoPkeEauUninit (void);

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
void 
ixCryptoPkeEauExpConfigSet (BOOL enableSE, BOOL enableFE);


/**
 * @fn void ixCryptoPkeEauRamWrite (
 *  UINT32 *pData,
 *  UINT32 dataLen, 
 *  UINT32 addr)
 *
 * @brief   Write to the EAU RAM at the calculated position
 *
 * @param *pData UINT32 [in] - data pointer for operands
 * @param dataLen UINT32 [in] - data length for operands
 * @param addr UINT32 [in] - address of the specific register to write operand
 *
 * @note
 * RAM is divided into 8 registers with 256-bits (8 words)each with 1 word access.
 *
 * @return @li  none
 * 
 */
void ixCryptoPkeEauRamWrite ( 
    UINT32 *pData, 
    UINT32 dataLen,
    UINT32 addr);

/**
 * @fn void ixCryptoPkeEauRamRead (
 *  UINT32 addr)
 *
 * @brief   Read EAU RAM at the specific register address. Result will be stored
 *          at the pointer location provided by the client. If the result length
 *          is larger than the maximum length, pad the remaining with zero.
 *          Max length for BN_MOD and BN_MUL are 128 words, others are 64 words.
 *          Result are read with LSB at the bottom address location.
 *
 * @param addr UINT32 [in] - address of the specific register to be read
 *
 * Global Data  : 
 *  - pOpResult     - pointer to the operation result
 *
 * @note
 * RAM is divided into 8 registers with 256-bits (8 words)each with 1 word access.
 * 
 */
void 
ixCryptoPkeEauRamRead (UINT32 addr);


/**
 * @fn void ixCryptoPkeEauDoneIsr (void)
 *
 * @brief   ISR function invoked when EAU has completed the operation, including
 *          calling client callback function.
 *
 * @note    1. When enter IX_CRYPTO_PKE_EAU_PERFORM_OPERATION state, it means  
 *             RAM clear operation completed, it will then continue to execute 
 *             EAU operation requested by client.
 *          2. When enter IX_CRYPTO_PKE_EAU_PERFORM_DONE state, it indicates 
 *             that EAU operation requested is completed, result will be read
 *             from the RAM and return to the client via client's callback.
 *
 * @param "none"
 * @return @li "none"
 */
void 
ixCryptoPkeEauDoneIsr (void);

/**
 * @fn void ixCryptoPkeEauRamClear (void)
 *
 * @brief   Clear all the RAM registers before each operation. 
 *
 * @param "none"
 * @return @li "none"
 * 
 */
IX_CRYPTO_INLINE void
ixCryptoPkeEauRamClear(void);

/**
 * @fn void ixCryptoPkeEauShow (void)
 *
 * @brief   Show counter for each operation being performed by EAU
 *
 * @param "none"
 * @return @li "none"
 * 
 */
void 
ixCryptoPkeEauShow(void);

/**
 * @fn void ixCryptoPkeEauOperationPerform (void)
 *
 * @brief   Call ixCryptoPkeEauRamWrite that writes operands to the associate 
 *          RAM registers and executes operation
 *
 * @param "none"
 * @return @li "none"
 * 
 */
void ixCryptoPkeEauOperationPerform (void);

/**
 * @fn void ixCryptoPkeEauResultRead (void)
 *
 * @brief   Read result from the RAM by calling ixCryptoPkeEauRamRead according
 *          to the operation
 *
 * @param "none"
 * @return @li "none"
 * 
 */
void ixCryptoPkeEauResultRead (void);

/**
 * @fn IxCryptoAccStatus ixCryptoPkeEauOperandCompare (
 *      UINT32 *pOprN, 
 *      UINT32 *pOprMorE, 
 *      UINT32 length)
 *
 * @brief Compare operands (N with M or N with e) for modular exponential
 *
 * @param *pOprN    UINT32 [in] - data pointer for first operand
 * @param *pOprMorE UINT32 [in] - data pointer for second operand
 * @param length    UINT32 [in] - length to compare
 *s
 * @return @li  IX_CRYPTO_ACC_STATUS_SUCCESS - Operands meet the requirement
 * @return @li  IX_CRYPTO_ACC_STATUS_OUT_OF_RANGE_ERR - Operands out of range
 *
 */ 
IxCryptoAccStatus 
ixCryptoPkeEauOperandCompare (UINT32 *pOprN, UINT32 *pOprMorE, UINT32 length);

#endif 

#endif /* __ixp46X */
