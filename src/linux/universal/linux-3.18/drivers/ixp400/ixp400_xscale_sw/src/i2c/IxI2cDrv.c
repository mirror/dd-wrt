/**
 * @file IxI2cDrv.c
 *
 * @version File Version: $Revision: 0.1 $
 * 
 * @par
 * IXP400 SW Release version 2.4
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

#if defined (__ixp46X)
#include "IxI2cDrv.h"
#include "IxOsal.h"
#include "IxFeatureCtrl.h"

                                                                               
/**
 * Local #defines
 */
#define IX_I2C_CR_OFFSET    0x0     /* I2C Control Register (ICR)
                                            offset from IXP400_I2C_BASE */
#define IX_I2C_SR_OFFSET    0x4     /* I2C Status Register (ISR)
                                            offset from IXP400_I2C_BASE */
#define IX_I2C_SAR_OFFSET   0x8     /* I2C Slave Address Register (ISAR)
                                            offset from IXP400_I2C_BASE */
#define IX_I2C_DBR_OFFSET   0xC     /* I2C Data Buffer Register (IDBR)
                                            offset from IXP400_I2C_BASE */

#define IX_I2C_MASK_7_BITS  0x7F/* Mask for the least significant 7-bits */

#define IX_I2C_SLAVE_READ   0x1 /* 1 indicates a read in the LSB after the
                                        7-bit slave address */
#define IX_I2C_SLAVE_WRITE  0x0 /* 0 indicates a write in the LSB after the
                                        7-bit slave address */

#define IX_I2C_ENABLE           0x1 /* 1 enables the feature in the ICR */
#define IX_I2C_DISABLE          0x0 /* 0 disables the feature in the ICR */
#define IX_I2C_GEN_CALL_DISABLE 0x1 /* 1 disables response to gen call */

#define IX_I2C_NACK             0x1 /* 1 sends a Nack in the ICR */
#define IX_I2C_ACK              0x0 /* 0 sends an Ack in the ICR */

#define IX_I2C_TRIGGERED        0x1 /* 1 indicates the bit is
                                            triggered in the ISR */
#define IX_I2C_INTERRUPTED      0x1 /* 1 indicates an interrupt has
                                            occured in the ISR */
#define IX_I2C_INTERRUPT_ENABLE 0x1 /* 1 enables the interrupt in the ICR */

#define IX_I2C_SLAVE_READ_MODE  0x0 /* 0 indicates a slave-receive for the
                                            read/write mode bit in the ISR */

#define IX_I2C_MIN_SLAVE_ADDR           0x1 /* The lowest slave addr for I2C */
#define IX_I2C_MAX_7_BIT_SLAVE_ADDR     0x7F/* The max slave addr for 7-bit I2C */

#define IX_I2C_SET_TO_BE_CLEARED        0x1 /* Use for write 1 to clear in
                                                the registers */

#define IX_I2C_US_DELAY_FOR_REG_READ    0x10 /* min delay implemented to avoid
                                                 a tight loop in polling */

#define IX_I2C_INVALID_SLAVE_ADDRESS    0x0 /* 0 value slave address is
                                                invalid as it is reserved
                                                for general calls */

#define IX_I2C_NOT_TO_BE_USED_I2C_ADDR  0x1 /* I2C address of 0x1 is reserved
                                                for the (now obsolete) C-Bus
                                                format */

/* #defines for mask and location of I2C Control Register contents */
#define IX_I2C_SPEEDMODE_LOC                 (0xF)
#define IX_I2C_SPEEDMODE_MASK                (0x1 << IX_I2C_SPEEDMODE_LOC)
#define IX_I2C_UNIT_RESET_LOC                (0xE)
#define IX_I2C_UNIT_RESET_MASK               (0x1 << IX_I2C_UNIT_RESET_LOC)
#define IX_I2C_SLAVE_ADDR_DETECT_ENABLE_LOC  (0xD)
#define IX_I2C_SLAVE_ADDR_DETECT_ENABLE_MASK (0x1 << IX_I2C_SLAVE_ADDR_DETECT_ENABLE_LOC)
#define IX_I2C_ARB_LOSS_INT_ENABLE_LOC       (0xC)
#define IX_I2C_ARB_LOSS_INT_ENABLE_MASK      (0x1 << IX_I2C_ARB_LOSS_INT_ENABLE_LOC)
#define IX_I2C_SLAVE_STOP_DETECT_ENABLE_LOC  (0xB)
#define IX_I2C_SLAVE_STOP_DETECT_ENABLE_MASK (0x1 << IX_I2C_SLAVE_STOP_DETECT_ENABLE_LOC)
#define IX_I2C_BUS_ERROR_INT_ENABLE_LOC      (0xA)
#define IX_I2C_BUS_ERROR_INT_ENABLE_MASK     (0x1 << IX_I2C_BUS_ERROR_INT_ENABLE_LOC)
#define IX_I2C_IDBR_RX_FULL_INT_ENABLE_LOC   (0x9)
#define IX_I2C_IDBR_RX_FULL_INT_ENABLE_MASK  (0x1 << IX_I2C_IDBR_RX_FULL_INT_ENABLE_LOC)
#define IX_I2C_IDBR_TX_EMPTY_INT_ENABLE_LOC  (0x8)
#define IX_I2C_IDBR_TX_EMPTY_INT_ENABLE_MASK (0x1 << IX_I2C_IDBR_TX_EMPTY_INT_ENABLE_LOC)
#define IX_I2C_GEN_CALL_RESPOND_LOC          (0x7)
#define IX_I2C_GEN_CALL_RESPOND_MASK         (0x1 << IX_I2C_GEN_CALL_RESPOND_LOC)
#define IX_I2C_UNIT_ENABLE_LOC               (0x6)
#define IX_I2C_UNIT_ENABLE_MASK              (0x1 << IX_I2C_UNIT_ENABLE_LOC)
#define IX_I2C_SCL_ENABLE_LOC                (0x5)
#define IX_I2C_SCL_ENABLE_MASK               (0x1 << IX_I2C_SCL_ENABLE_LOC)
#define IX_I2C_MASTER_ABORT_LOC              (0x4)
#define IX_I2C_MASTER_ABORT_MASK             (0x1 << IX_I2C_MASTER_ABORT_LOC)
#define IX_I2C_TRANSFER_BYTE_LOC             (0x3)
#define IX_I2C_TRANSFER_BYTE_MASK            (0x1 << IX_I2C_TRANSFER_BYTE_LOC)
#define IX_I2C_ACK_NACK_CTL_LOC              (0x2)
#define IX_I2C_ACK_NACK_CTL_MASK             (0x1 << IX_I2C_ACK_NACK_CTL_LOC)
#define IX_I2C_SEND_STOP_LOC                 (0x1)
#define IX_I2C_SEND_STOP_MASK                (0x1 << IX_I2C_SEND_STOP_LOC)
#define IX_I2C_SEND_START_LOC                (0x0)
#define IX_I2C_SEND_START_MASK               (0x1 << IX_I2C_SEND_START_LOC)

/* #defines for mask and location of I2C Status Register contents */
#define IX_I2C_BUS_ERROR_DETECTED_LOC        (0xA)
#define IX_I2C_BUS_ERROR_DETECTED_MASK       (0x1 << IX_I2C_BUS_ERROR_DETECTED_LOC)
#define IX_I2C_SLAVE_ADDR_DETECTED_LOC       (0x9)
#define IX_I2C_SLAVE_ADDR_DETECTED_MASK      (0x1 << IX_I2C_SLAVE_ADDR_DETECTED_LOC)
#define IX_I2C_GEN_CALL_ADDR_DETECTED_LOC    (0x8)
#define IX_I2C_GEN_CALL_ADDR_DETECTED_MASK   (0x1 << IX_I2C_GEN_CALL_ADDR_DETECTED_LOC)
#define IX_I2C_IDBR_RX_FULL_LOC              (0x7)
#define IX_I2C_IDBR_RX_FULL_MASK             (0x1 << IX_I2C_IDBR_RX_FULL_LOC)
#define IX_I2C_IDBR_TX_EMPTY_LOC             (0x6)
#define IX_I2C_IDBR_TX_EMPTY_MASK            (0x1 << IX_I2C_IDBR_TX_EMPTY_LOC)
#define IX_I2C_ARB_LOSS_DETECTED_LOC         (0x5)
#define IX_I2C_ARB_LOSS_DETECTED_MASK        (0x1 << IX_I2C_ARB_LOSS_DETECTED_LOC)
#define IX_I2C_SLAVE_STOP_DETECTED_LOC       (0x4)
#define IX_I2C_SLAVE_STOP_DETECTED_MASK      (0x1 << IX_I2C_SLAVE_STOP_DETECTED_LOC)
#define IX_I2C_BUS_BUSY_LOC                  (0x3)
#define IX_I2C_BUS_BUSY_MASK                 (0x1 << IX_I2C_BUS_BUSY_LOC)
#define IX_I2C_UNIT_BUSY_LOC                 (0x2)
#define IX_I2C_UNIT_BUSY_MASK                (0x1 << IX_I2C_UNIT_BUSY_LOC)
#define IX_I2C_ACK_NACK_STATUS_LOC           (0x1)
#define IX_I2C_ACK_NACK_STATUS_MASK          (0x1 << IX_I2C_ACK_NACK_STATUS_LOC)
#define IX_I2C_READ_WRITE_MODE_LOC           (0x0)
#define IX_I2C_READ_WRITE_MODE_MASK          (0x1 << IX_I2C_READ_WRITE_MODE_LOC)

/**
 * section for macros
 */
/* Used by most APIs to check if the I2C is already init by checking the
    ixI2cInitComplete flag. Display and return error if I2C is not init */
#define IX_I2C_INIT_SUCCESS_CHECK(funcName)                                 \
if(FALSE == ixI2cInitComplete)                                              \
{                                                                           \
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,               \
        funcName": I2C Driver not initialized\n",                           \
        0,0,0,0,0,0);                                                       \
    return IX_I2C_NOT_INIT;                                                 \
} /* end of FALSE == ixI2cInitComplete */


/* Used in ixI2cDrvSlaveAddrDetectedHdlr, ixI2cDrvIDBRTxEmptyHdlr, and
    ixI2cDrvIDBRRxFullHdlr to check if the client buffer is NULL, then call
    the respective client callback to indicate no buffer. If callback does
    not provide a buffer after a few tries, then disable the I2C unit and log
    an error. *NOTE*: numOfTries has to be declared to use this macro. */
#define IX_I2C_CHECK_SLAVE_OR_GEN_BUFFER_NULL(CBFuncName, CBFuncStr, CallingAPIName, returnVal)\
/* Clear number of tries to check buffer after calling callback */          \
numOfTries = 0;                                                             \
                                                                            \
while(NULL == ixI2cSlaveOrGenBufTracker.bufP)                               \
{                                                                           \
    CBFuncName(IX_I2C_SLAVE_NO_BUFFER,                                      \
        ixI2cSlaveOrGenBufTracker.bufP,                                     \
        ixI2cSlaveOrGenBufTracker.bufSize,                                  \
        ixI2cSlaveOrGenBufTracker.offset);                                  \
                                                                            \
    /* Preventive mechanism when client does not provide buffer */          \
    if(numOfTries++ >= IX_I2C_NUM_OF_TRIES_TO_CALL_CALLBACK_FUNC)           \
    {                                                                       \
        /* Disable the I2C unit and log an error */                         \
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_UNIT_ENABLE_MASK)) |    \
                    (IX_I2C_DISABLE << IX_I2C_UNIT_ENABLE_LOC);             \
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);                    \
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,           \
            CallingAPIName ": " CBFuncStr                                   \
            "not providing buffer! I2C disabled!!\n", 0,0,0,0,0,0);         \
        return returnVal;                                                   \
    } /* end of preventive mechanism for client not providing buffer */     \
} /* end of buffer is NULL */


/* Used in ixI2cDrvIDBRRxFullHdlr to check if the client buffer is filled then
    call the callback to indicate buffer full. If callback does not clear the
    buffer after a few tries, then disable the I2C unit and log an error.
     *NOTE*: numOfTries has to be declared to use this macro. */
#define IX_I2C_CHECK_SLAVE_OR_GEN_BUFFER_FULL(CBFuncName, CBFuncStr, CallingAPIName, returnVal)\
/* Clear number of tries to check buffer after calling callback */          \
numOfTries = 0;                                                             \
                                                                            \
/* Check if the buffer is full */                                           \
while(ixI2cSlaveOrGenBufTracker.bufSize <= ixI2cSlaveOrGenBufTracker.offset)\
{                                                                           \
    CBFuncName(IX_I2C_SLAVE_OR_GEN_READ_BUFFER_FULL,                        \
        ixI2cSlaveOrGenBufTracker.bufP,                                     \
        ixI2cSlaveOrGenBufTracker.bufSize,                                  \
        ixI2cSlaveOrGenBufTracker.offset);                                  \
    /* Preventive mechanism if the client does not have code to             \
        empty the buffer */                                                 \
    if(numOfTries++ >= IX_I2C_NUM_OF_TRIES_TO_CALL_CALLBACK_FUNC)           \
    {                                                                       \
        /* Disable the I2C unit and log an error */                         \
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_UNIT_ENABLE_MASK)) |    \
                    (IX_I2C_DISABLE << IX_I2C_UNIT_ENABLE_LOC);             \
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);                    \
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,           \
            CallingAPIName ": " CBFuncStr                                   \
            "not emptying buffer! I2C disabled!!\n", 0,0,0,0,0,0);          \
        return returnVal;                                                   \
    } /* end of preventive mechanism for client not emptying the buffer */  \
} /* end of buffer is full */


/* Used in ixI2cDrvSlaveAddrDetectedHdlr and ixI2cDrvIDBRTxEmptyHdlr to check
    if the client buffer is empty then call the callback to indicate buffer
    empty. If callback does not provide a filled buffer after a few tries,
    then disable the I2C unit and log an error.
     *NOTE*: numOfTries has to be declared to use this macro. */
#define IX_I2C_CHECK_SLAVE_BUFFER_EMPTY(CallingAPIName, returnVal)          \
/* Clear number of tries to check buffer after calling callback */          \
numOfTries = 0;                                                             \
                                                                            \
/* Check if the buffer is empty */                                          \
while(ixI2cSlaveOrGenBufTracker.bufSize <= ixI2cSlaveOrGenBufTracker.offset)\
{                                                                           \
    /* Call slave write callback to indicate buffer empty */                \
    ixI2cSlaveWrCallbackP(IX_I2C_SLAVE_WRITE_BUFFER_EMPTY,                  \
        ixI2cSlaveOrGenBufTracker.bufP, ixI2cSlaveOrGenBufTracker.bufSize,  \
        ixI2cSlaveOrGenBufTracker.offset);                                  \
                                                                            \
    /* Preventive mechanism when client does not fill the buffer */         \
    if(numOfTries++ >= IX_I2C_NUM_OF_TRIES_TO_CALL_CALLBACK_FUNC)           \
    {                                                                       \
        /* Disable the I2C unit and log an error */                         \
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_UNIT_ENABLE_MASK)) |    \
                    (IX_I2C_DISABLE << IX_I2C_UNIT_ENABLE_LOC);             \
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);                    \
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,           \
            CallingAPIName "ixI2cSlaveWrCallbackP not emptying buffer! I2C disabled!!\n",  \
            0,0,0,0,0,0);                                                   \
        return returnVal;                                                   \
    } /* end of preventive mechanism for client not emptying the buffer */  \
} /* end of buffer is empty */


/* Used in ixI2cDrvWriteTransfer to poll for the IDBR Tx bit in the I2C SR
    and if it never received it then check if the error is due to an arb loss
    or a transfer error. Return respective error if error else continue.
     *NOTE*: numOfTries has to be declared to use this macro. */
#define IX_I2C_POLL_IDBR_TX_EMPTY_AND_CHECK_FOR_ARB_LOSS(dataXferred)       \
/* Reset numOfTries to zero for polling */                                  \
numOfTries = 0;                                                             \
                                                                            \
/* Poll for IDBR Transmit Empty to determine the data in the IDBR           \
    has bean transmitted */                                                 \
do /* IX_I2C_TRIGGERED != ixI2cStsStored.IDBRTxEmpty */                     \
{                                                                           \
    ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);                        \
    /* Delay before next read */                                            \
    ixI2cDrvSleep(IX_I2C_US_DELAY_FOR_REG_READ);                            \
} while ((IX_I2C_TRIGGERED != ((ixI2cStsStored & IX_I2C_IDBR_TX_EMPTY_MASK) >>\
    IX_I2C_IDBR_TX_EMPTY_LOC)) &&                                           \
    (numOfTries++ < IX_I2C_NUM_TO_POLL_IDBR_TX_EMPTY));                     \
                                                                            \
/* Check if master failed to transmit */                                    \
if(IX_I2C_TRIGGERED != ((ixI2cStsStored & IX_I2C_IDBR_TX_EMPTY_MASK) >>     \
    IX_I2C_IDBR_TX_EMPTY_LOC))                                              \
{                                                                           \
    /* Check if transmit failure is due to arbitration loss */              \
    if(IX_I2C_TRIGGERED == ((ixI2cStsStored &                               \
    IX_I2C_ARB_LOSS_DETECTED_MASK) >> IX_I2C_ARB_LOSS_DETECTED_LOC))        \
    {                                                                       \
        /* Clear the slave address detected bit by writing                  \
            one to the bit location */                                      \
        ixI2cStsStored =                                                    \
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_ARB_LOSS_DETECTED_LOC);     \
        IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);                    \
        ixI2cStatsCounters.ixI2cArbLossCounter++;                           \
        return IX_I2C_MASTER_ARB_LOSS;                                      \
    } /* end of arbitration loss */                                         \
    else /* transmit failure */                                             \
    {                                                                       \
        /* increment the stats for number of bytes master has Tx (includes  \
            slave address) and one byte for master failed to transmit */    \
        if(dataXferred != 0) /* slave addr already sent */                  \
        {                                                                   \
            ixI2cStatsCounters.ixI2cMasterXmitCounter += (dataXferred + 1); \
        } /* end of slave addr already sent */                              \
        ixI2cStatsCounters.ixI2cMasterFailedXmitCounter++;                  \
        return IX_I2C_MASTER_XFER_ERROR;                                    \
    } /* end of transmit failure */                                         \
} /* end of master failed to transmit */

/* Used in ixI2cDrvWriteTransfer and ixI2cDrvReadTransfer to check for bus
    error occurences during transfers. If a bus error occurs, then it will
    send a master abort. */
#define IX_I2C_CHECK_FOR_BUS_ERROR                                          \
/* Check for bus error due to previous transfer before continuing           \
    transfering more data by checking the I2C Status Register */            \
ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);                            \
if(IX_I2C_TRIGGERED == ((ixI2cStsStored & IX_I2C_BUS_ERROR_DETECTED_MASK) >>\
    IX_I2C_BUS_ERROR_DETECTED_LOC))                                         \
{                                                                           \
    /* Clear the bus error detected bit by writing one to the bit           \
        location */                                                         \
    ixI2cStsStored =                                                        \
        (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_BUS_ERROR_DETECTED_LOC) |       \
        (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_TX_EMPTY_LOC);             \
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);                        \
                                                                            \
    /* Send master abort signal */                                          \
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |       \
                    (IX_I2C_ENABLE << IX_I2C_MASTER_ABORT_LOC);             \
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |      \
                    (IX_I2C_DISABLE << IX_I2C_TRANSFER_BYTE_LOC);           \
    IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);                        \
    return IX_I2C_MASTER_BUS_ERROR;                                         \
} /* end of bus error detected */

/*
 * section for enums
 */

/* Used in interrupt mode to determine the operation mode */
typedef enum
{
    IX_I2C_SLAVE_READ_OPERATION = 0x0,
    IX_I2C_SLAVE_WRITE_OPERATION,
    IX_I2C_MASTER_READ_OPERATION,
    IX_I2C_MASTER_WRITE_OPERATION,
    IX_I2C_GENERAL_CALL_OPERATION
} IxI2cOperationMode;

/* Used in interrupt mode by master in sync mode to determine transfer status */
typedef enum
{
    IX_I2C_INTR_XFER_IN_PROGRESS = 0xFF,
    IX_I2C_INTR_XFER_SUCCESSFUL = IX_I2C_SUCCESS,
    IX_I2C_INTR_ARB_LOSS = IX_I2C_MASTER_ARB_LOSS,
    IX_I2C_INTR_XFER_ERROR = IX_I2C_MASTER_XFER_ERROR,
    IX_I2C_INTR_BUS_ERROR = IX_I2C_MASTER_BUS_ERROR
} IxI2cInterruptTransferStatus;


/**
 * section for typedef
 */

/* typedef for I2C buffer tracker */
typedef struct
{
    char* bufP;             /**< pointer to the location of the buffer */
    UINT32 offset;          /**< offset from the pointer head */
    UINT32 bufSize;         /**< size of the buffer */
    IxI2cXferMode XferMode; /**< USED ONLY IN MASTER: to indicate transaction
                                mode */
} IxI2cBufTracker;


/**
 * Private variables defined here
 */

/* Interrupt handler function pointers */
PRIVATE IxI2cMasterReadCallbackP ixI2cMasterRdCallbackP = NULL;
PRIVATE IxI2cMasterWriteCallbackP ixI2cMasterWrCallbackP = NULL;
PRIVATE IxI2cSlaveReadCallbackP ixI2cSlaveRdCallbackP = NULL;
PRIVATE IxI2cSlaveWriteCallbackP ixI2cSlaveWrCallbackP = NULL;
PRIVATE IxI2cGenCallCallbackP ixI2cGenCallCallbackP = NULL;

PRIVATE UINT32 ixI2cBaseAddr = 0;/* Base addr of the I2C registers */
PRIVATE UINT32 ixI2cCRAddr = 0; /* Addr of the I2C control register (CR) */
PRIVATE UINT32 ixI2cSRAddr = 0; /* Addr of the I2C status register (CR) */
PRIVATE UINT32 ixI2cSARAddr = 0;/* Addr of the I2C slave addr register (CR) */
PRIVATE UINT32 ixI2cDBRAddr = 0;/* Addr of the I2C data buffer register (CR) */

PRIVATE UINT32 ixI2cDelayType = IX_I2C_LOOP_DELAY;

/* Storage for the I2C configuration which is used over many functions to
    increase efficiency */
PRIVATE UINT32 ixI2cCfgStored;

/* Storage for the I2C status which is used by many functions to avoid
    declaration of the same struct multiple times */
PRIVATE UINT32 ixI2cStsStored;

/* Storage for the I2C statistics counters */
PRIVATE IxI2cStatsCounters ixI2cStatsCounters;

/* Storage for the buffer info that is used for tracking slave or general
    calls in interrupt mode */
PRIVATE IxI2cBufTracker ixI2cSlaveOrGenBufTracker;

/* Storage for the buffer info that is used for tracking master in
    interrupt mode */
PRIVATE IxI2cBufTracker ixI2cMasterBufTracker;

/* Flag used in interrupt mode to indicate the operation mode - master read,
    master write, slave read, slave write, or general call. By default it is
    set to slave read */
PRIVATE IxI2cOperationMode ixI2cOpMode = IX_I2C_SLAVE_READ_OPERATION;

/* Flag used in interrupt mode to indicate the status of the master transfer -
    in progress, successful, arb loss, or transfer error */
PRIVATE IxI2cInterruptTransferStatus ixI2cIntrXferStatus = IX_I2C_INTR_XFER_SUCCESSFUL;

/* Flag to indicate if the mode is interrupt or poll. */
PRIVATE BOOL ixI2cInterruptMode = FALSE;

/* Flag to indicate if the init has been done and thus not performing some
    instructions that should not be done more than once (please refer to the
    init API. Example: memory mapping). if init is called more than once
    (which is allowed) */
PRIVATE BOOL ixI2cInitComplete = FALSE;

/**
 * Private function declaration
 */
PRIVATE void ixI2cDrvInterruptDetected (void);
PRIVATE void ixI2cDrvBusErrorHdlr (void);
PRIVATE void ixI2cDrvSlaveAddrDetectedHdlr (void);
PRIVATE void ixI2cDrvGenCallAddrDetectedHdlr (void);
PRIVATE void ixI2cDrvIDBRRxFullHdlr (void);
PRIVATE void ixI2cDrvIDBRTxEmptyHdlr (void);
PRIVATE void ixI2cDrvArbLossDetectedHdlr (void);
PRIVATE void ixI2cDrvSlaveStopDetectedHdlr (void);
PRIVATE void ixI2cDrvSleep(UINT32 delay);

/**
 * Function definitions
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvInit (IxI2cInitVars *initVarsSelected)
{
    UINT32 localTempI2cCfg;

    /* Check if the hardware supports I2C. By reading the device type, it can
        be determined if the hardware supports I2C. Currenlty only the IXP46X
        supports I2C. */
  if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X != ixFeatureCtrlDeviceRead())
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixI2cDrvInit: There's no dedicated I2C hardware"
            " support!\n", 0,0,0,0,0,0);
        return IX_I2C_NOT_SUPPORTED;
    } /* end of device detected is not IXP46X */

    /* Valid parameter check section */
    /* Check if the initVarsSelected is NULL */
    if(NULL == initVarsSelected)
    {
        return IX_I2C_NULL_POINTER;
    } /* end of initVarsSelected is NULL */

    /* Check if the I2C speed mode (100Kbps or 400Kbps) selected is valid */
    if(initVarsSelected->I2cSpeedSelect > IX_I2C_FAST_MODE)
    {
        return IX_I2C_INVALID_SPEED_MODE_ENUM_VALUE;
    } /* end of invalid I2cSpeedSelect */

    /* Check if the I2C flow mode (interrupt or poll) selected is valid */
    if(initVarsSelected->I2cFlowSelect > IX_I2C_INTERRUPT_MODE)
    {
        return IX_I2C_INVALID_FLOW_MODE_ENUM_VALUE;
    } /* end of invalid I2cFlowSelect */
    
    /* Check if the I2C Hardware Address is valid (non-zero) */
    if(IX_I2C_INVALID_SLAVE_ADDRESS == initVarsSelected->I2cHWAddr)
    {
        return IX_I2C_INVALID_SLAVE_ADDR;
    } /* end of invalid I2C slave address */

    /* Clear I2C Configurations to zero, only client configurations that
        are non-zero will be set to one in the following */
    localTempI2cCfg = 0;
    
    /* Set the I2C speed mode selected */
    localTempI2cCfg = (localTempI2cCfg & (~IX_I2C_SPEEDMODE_MASK)) |
                    (initVarsSelected->I2cSpeedSelect << IX_I2C_SPEEDMODE_LOC);

    /* If previously Interrupt Mode was set, then unbind the interrupt
        and reset callback pointers to NULL */
    if(TRUE == ixI2cInterruptMode)
    {
        if(IX_SUCCESS != ixOsalIrqUnbind(IX_OSAL_IXP400_I2C_IRQ_LVL))
        {
            return IX_I2C_INT_UNBIND_FAIL;
        } /* end of ixOsalIrqUnbind */

        ixI2cMasterRdCallbackP = NULL;
        ixI2cMasterWrCallbackP = NULL;
        ixI2cSlaveRdCallbackP = NULL;
        ixI2cSlaveWrCallbackP = NULL;
        ixI2cGenCallCallbackP = NULL;

        ixI2cInterruptMode = FALSE;
    } /* end of TRUE == ixI2cInterruptMode */

    /* Check if interrupt mode is selected */
    if(IX_I2C_INTERRUPT_MODE == initVarsSelected->I2cFlowSelect)
    {
        /* Check if configuration is set to respond to slave address */
        if(TRUE == initVarsSelected->I2cSlaveAddrResponseEnable)
        {
            /* Check if slave callbacks are NULL, then return error */
            if((NULL == initVarsSelected->SlaveReadCBP) ||
                (NULL == initVarsSelected->SlaveWriteCBP))
            {
                return IX_I2C_SLAVE_ADDR_CB_MISSING;
            } /* end of slave callback are NULL */

            ixI2cSlaveRdCallbackP = initVarsSelected->SlaveReadCBP;
            ixI2cSlaveWrCallbackP = initVarsSelected->SlaveWriteCBP;
            localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_SLAVE_ADDR_DETECT_ENABLE_MASK)) |
                (IX_I2C_INTERRUPT_ENABLE << IX_I2C_SLAVE_ADDR_DETECT_ENABLE_LOC);
            localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_SLAVE_STOP_DETECT_ENABLE_MASK)) |
                (IX_I2C_INTERRUPT_ENABLE << IX_I2C_SLAVE_STOP_DETECT_ENABLE_LOC);
        } /* end of I2cSlaveAddrResponseEnable == TRUE */
        /* Check if configuration is set to respond to general calls */
        if(TRUE == initVarsSelected->I2cGenCallResponseEnable)
        {
            /* Check gen call callback is NULL, then return error */
            if(NULL == initVarsSelected->GenCallCBP)
            {
                return IX_I2C_GEN_CALL_CB_MISSING;
            } /* end of gen call callback is NULL */

            localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_SLAVE_ADDR_DETECT_ENABLE_MASK)) |
                (IX_I2C_INTERRUPT_ENABLE << IX_I2C_SLAVE_ADDR_DETECT_ENABLE_LOC);
            localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_SLAVE_STOP_DETECT_ENABLE_MASK)) |
                (IX_I2C_INTERRUPT_ENABLE << IX_I2C_SLAVE_STOP_DETECT_ENABLE_LOC);
            ixI2cGenCallCallbackP = initVarsSelected->GenCallCBP;
        } /* end of I2cGenCallResponseEnable == TRUE */
        else /* I2cGenCallResponseEnable = FALSE */
        {
            localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_GEN_CALL_RESPOND_MASK)) |
                (IX_I2C_GEN_CALL_DISABLE << IX_I2C_GEN_CALL_RESPOND_LOC);
        } /* end of I2cGenCallResponseEnable = FALSE */
        
        
        /* Set Master Callbacks whether it is NULL or not */
        ixI2cMasterRdCallbackP = initVarsSelected->MasterReadCBP;
        ixI2cMasterWrCallbackP = initVarsSelected->MasterWriteCBP;
        
        /* Enable all the other interrupt bits */
        localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_ARB_LOSS_INT_ENABLE_MASK)) |
                (IX_I2C_INTERRUPT_ENABLE << IX_I2C_ARB_LOSS_INT_ENABLE_LOC);
        localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_BUS_ERROR_INT_ENABLE_MASK)) |
                (IX_I2C_INTERRUPT_ENABLE << IX_I2C_BUS_ERROR_INT_ENABLE_LOC);
        localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_IDBR_RX_FULL_INT_ENABLE_MASK)) |
                (IX_I2C_INTERRUPT_ENABLE << IX_I2C_IDBR_RX_FULL_INT_ENABLE_LOC);
        localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_IDBR_TX_EMPTY_INT_ENABLE_MASK)) |
                (IX_I2C_INTERRUPT_ENABLE << IX_I2C_IDBR_TX_EMPTY_INT_ENABLE_LOC);

        /* Bind the I2C to the I2C ISR */
        if(IX_SUCCESS != ixOsalIrqBind(IX_OSAL_IXP400_I2C_IRQ_LVL,
                            (IxOsalVoidFnVoidPtr)ixI2cDrvInterruptDetected,
                            NULL))
        {
            ixI2cMasterRdCallbackP = NULL;
            ixI2cMasterWrCallbackP = NULL;
            ixI2cSlaveRdCallbackP = NULL;
            ixI2cSlaveWrCallbackP = NULL;
            ixI2cGenCallCallbackP = NULL;
            return IX_I2C_INT_BIND_FAIL;
        } /* end of ixOsalIrqBind FAIL */
        
        ixI2cInterruptMode = TRUE;
    } /* end of Flow Mode == INTERRUPT MODE */

    else /* I2cFlowSelect == IX_I2C_POLL_MODE */
    {       
        if(TRUE != initVarsSelected->I2cGenCallResponseEnable)
        {
            localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_GEN_CALL_RESPOND_MASK)) |
                (IX_I2C_GEN_CALL_DISABLE << IX_I2C_GEN_CALL_RESPOND_LOC);
        } /* end of TRUE == initVarsSelected->I2cGenCallResponseEnable */
        
        /* Set Interrupt Mode flag to false (poll mode) */
        ixI2cInterruptMode = FALSE;
    } /* end of Flow Mode == POLL MODE */

    /* Set the SCL Enable */
    if(TRUE == initVarsSelected->SCLEnable)
    {
        localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_SCL_ENABLE_MASK)) |
                (IX_I2C_ENABLE << IX_I2C_SCL_ENABLE_LOC);
    } /* end of TRUE == initVarsSelected->SCLEnable */

    /* Check if I2C Init has been called before to avoid multiple instances of
        memory mapping */
    if(FALSE == ixI2cInitComplete)
    {
        /* Memory map the control, status, slave_addrees, and data
            registers of the I2C */
        ixI2cBaseAddr = (UINT32)IX_OSAL_MEM_MAP (IX_OSAL_IXP400_I2C_PHYS_BASE,
                        IX_OSAL_IXP400_I2C_MAP_SIZE);
        ixI2cCRAddr = ixI2cBaseAddr + IX_I2C_CR_OFFSET;
        ixI2cSRAddr = ixI2cBaseAddr + IX_I2C_SR_OFFSET;
        ixI2cSARAddr = ixI2cBaseAddr + IX_I2C_SAR_OFFSET;
        ixI2cDBRAddr = ixI2cBaseAddr + IX_I2C_DBR_OFFSET;

        ixI2cDrvStatsReset(); /* Clear the I2C statistics counters */
        ixI2cInitComplete = TRUE; /* Set the Init Complete flag so that a
                                    call to init will not mem map and clear
                                    the stats again*/
    } /* end of FALSE == ixI2cInitComplete */

    /* Set the I2C Hardware Slave address */
    if(TRUE == initVarsSelected->I2cSlaveAddrResponseEnable)
    {
        IX_OSAL_WRITE_LONG(ixI2cSARAddr,
            initVarsSelected->I2cHWAddr & IX_I2C_MASK_7_BITS);
    }
    else /* Slave address set to an I2C addr that should not be used */
    {
        IX_OSAL_WRITE_LONG(ixI2cSARAddr, IX_I2C_NOT_TO_BE_USED_I2C_ADDR);
    } /* end of setting I2C Hardware Slave address */

    /* Enable the I2C Unit */
    localTempI2cCfg =
                (localTempI2cCfg & (~IX_I2C_UNIT_ENABLE_MASK)) |
                (IX_I2C_ENABLE << IX_I2C_UNIT_ENABLE_LOC);

    /* Check if in interrupt mode */
    if(TRUE == ixI2cInterruptMode)
    {
        /* Clear the slave address detected bit and slave stop detected bit
            by writing 1 to the bit location */
        ixI2cStsStored =
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_ADDR_DETECTED_LOC) |
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_STOP_DETECTED_LOC);
        IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
    } /* end of FALSE == ixI2cInterruptMode */

    /* Copy the localTempI2cCfg to the global ixI2cCfgStored
        and write the client configuration into the I2C Control register */
    ixI2cCfgStored = localTempI2cCfg;
    IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

    return IX_I2C_SUCCESS;
} /* end of ixI2cDrvInit */


PUBLIC IX_I2C_STATUS
ixI2cDrvUninit(void)
{
    if(TRUE == ixI2cInitComplete)
    {
        /* Clear I2C Configurations to zero while disabling the I2C */
        ixI2cCfgStored = 0;
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
        
        /* Unbind the I2C ISR if interrupt mode is enabled */
        if(TRUE == ixI2cInterruptMode)
        {
            if(IX_SUCCESS != ixOsalIrqUnbind(IX_OSAL_IXP400_I2C_IRQ_LVL))
            {
                return IX_I2C_INT_UNBIND_FAIL;
            } /* end of ixOsalIrqUnbind */
            
            /* Set all callback pointers to NULL */
            ixI2cMasterRdCallbackP = NULL;
            ixI2cMasterWrCallbackP = NULL;
            ixI2cSlaveRdCallbackP = NULL;
            ixI2cSlaveWrCallbackP = NULL;
            ixI2cGenCallCallbackP = NULL;

            ixI2cInterruptMode = FALSE;
        } /* end of TRUE == ixI2cInterruptMode */
        
        /* Return the memory that was mapped during init which is the I2C
            control, status, data buffer, and slave address registers. */
        IX_OSAL_MEM_UNMAP(ixI2cBaseAddr);
        ixI2cInitComplete = FALSE;
    } /* end of TRUE == ixI2cInitComplete */
    else /* FALSE == ixI2cInitComplete */
    {
        return IX_I2C_NOT_INIT;
    } /* end of FALSE == ixI2cInitComplete */

    return IX_I2C_SUCCESS;
} /* end of ixI2cDrvUninit */


PUBLIC IX_I2C_STATUS
ixI2cDrvSlaveAddrSet(UINT8 SlaveAddrSet)
{
    /* Disallow this function from running if I2C not initialized */
    IX_I2C_INIT_SUCCESS_CHECK("ixI2cDrvSlaveAddrSet");
    
    /* Check if the I2C Hardware Address is valid (non-zero) */
    if(IX_I2C_INVALID_SLAVE_ADDRESS == SlaveAddrSet)
    {
        return IX_I2C_INVALID_SLAVE_ADDR;
    } /* end of IX_I2C_INVALID_SLAVE_ADDRESS */
        
    /* Set the I2C Hardware Slave address */
    IX_OSAL_WRITE_LONG(ixI2cSARAddr,
            SlaveAddrSet & IX_I2C_MASK_7_BITS);
    return IX_I2C_SUCCESS;
} /* end of ixI2cDrvSlaveAddrSet */


PUBLIC IX_I2C_STATUS
ixI2cDrvBusScan(void)
{
    UINT8 deviceSlaveAddr = 0;
    UINT32 numOfSlaveDeviceFound = 0;
    UINT32 slaveAddrToScan = 0;
    UINT32 numOfTries = 0;
    UINT32 storedI2cCfg;
    
    /* Disallow this function from running if I2C not initialized */
    IX_I2C_INIT_SUCCESS_CHECK("ixI2cDrvBusScan");

    /* Obtain the IXP's slave address to skip scanning for its own address */
    deviceSlaveAddr = IX_OSAL_READ_LONG(ixI2cSARAddr);

    /* Store previous configuration before running it's own configuration
        for scanning the bus */
    storedI2cCfg = ixI2cCfgStored;

    ixI2cCfgStored = 0; /* all interrupts are disabled */
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SPEEDMODE_MASK)) |
                    (IX_I2C_NORMAL_MODE << IX_I2C_SPEEDMODE_LOC);
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_UNIT_ENABLE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_UNIT_ENABLE_LOC);
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SCL_ENABLE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_SCL_ENABLE_LOC);
    
    IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

    for(slaveAddrToScan = IX_I2C_MIN_SLAVE_ADDR;
        slaveAddrToScan <= IX_I2C_MAX_7_BIT_SLAVE_ADDR;
        slaveAddrToScan++)
    {
        /* Skip the IXP's own slave device address */
        if(slaveAddrToScan == deviceSlaveAddr)
        {
            continue;
        } /* end of skip device's own slave address */

        /* Write SlaveAddr into the IDBR after shifting left 1 bit (LSB is zero
            indicating a write access for the slave that is to be accessed */
        IX_OSAL_WRITE_LONG(ixI2cDBRAddr,
                        (slaveAddrToScan << 1) | IX_I2C_SLAVE_WRITE);

        /* Enable the start signal to control the bus and the Transfer byte
            bit to transmit a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_SEND_START_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_SEND_STOP_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);

        /* Read the I2C Status Register */
        ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);

        /* Check if the I2C bus is busy before starting transfer */
        if(IX_I2C_TRIGGERED == ((ixI2cStsStored & IX_I2C_BUS_BUSY_MASK) >>
            IX_I2C_BUS_BUSY_LOC))
        {
            /* restore to previous configuration */
            ixI2cCfgStored = storedI2cCfg;
            IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
            return IX_I2C_MASTER_BUS_BUSY;
        } /* end of I2C bus is busy */

        /* Write the configuration to the ICR to start the transfer */
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

        numOfTries = 0; /* Reset number of tries to check register */

        /* Poll for IDBR Transmit Empty to determine the data in the IDBR
            has bean transmitted */
        do /* IX_I2C_TRIGGERED != ixI2cStsStored.IDBRTxEmpty */
        {
            ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);
            /* Delay before next read */
            ixI2cDrvSleep(IX_I2C_US_DELAY_FOR_REG_READ);
        } while ((IX_I2C_TRIGGERED !=
            ((ixI2cStsStored & IX_I2C_IDBR_TX_EMPTY_MASK) >>
            IX_I2C_IDBR_TX_EMPTY_LOC)) &&
            (numOfTries++ < IX_I2C_NUM_TO_POLL_IDBR_TX_EMPTY));

        /* Check if master failed to transmit */
        if(IX_I2C_TRIGGERED != ((ixI2cStsStored & IX_I2C_IDBR_TX_EMPTY_MASK) >>
            IX_I2C_IDBR_TX_EMPTY_LOC))
        {
            /* Check if transmit failure is due to arbitration loss */
            if(IX_I2C_TRIGGERED ==
                ((ixI2cStsStored & IX_I2C_ARB_LOSS_DETECTED_MASK) >>
                IX_I2C_ARB_LOSS_DETECTED_LOC))
            {
                /* Clear the slave address detected bit by writing
                    one to the bit location */
                ixI2cStsStored =
                    (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_ARB_LOSS_DETECTED_LOC);
                IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
                ixI2cStatsCounters.ixI2cArbLossCounter++;

                /* restore to previous configuration */
                ixI2cCfgStored = storedI2cCfg;
                IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
                return IX_I2C_MASTER_ARB_LOSS;
            } /* end of arbitration loss */
        }
        else /* master has transmitted */
        {
            /* check if device found by checking Ack reply */
            if(IX_I2C_ACK ==
                ((ixI2cStsStored & IX_I2C_ACK_NACK_STATUS_MASK) >>
                IX_I2C_ACK_NACK_STATUS_LOC))
            {
                ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                    "Slave device detected at :0x%x\n",
                    slaveAddrToScan,0,0,0,0,0);
                numOfSlaveDeviceFound++;
            }
        } /* end of slave device found */

        /* Clear the IDBR transmit empty detected bit by writing
            one to the bit location */
        IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

        /* Send a master abort to stop transfers in order to scan next address */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_MASTER_ABORT_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_TRANSFER_BYTE_LOC);

        /* Write the configuration to the ICR to start the transfer */
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
    }

    /* restore to previous configuration */
    ixI2cCfgStored = storedI2cCfg;
    IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

    /* Check if any slave devices were found */
    if(0 == numOfSlaveDeviceFound)
    {
        return IX_I2C_FAIL;
    } /* end of no slave device found */
    else /* found at least one slave device */
    {
        return IX_I2C_SUCCESS;
    } /* end of found at least one slave device */
} /* end of ixI2cDrvBusScan */

PUBLIC IX_I2C_STATUS
ixI2cDrvWriteTransfer(
    UINT8 SlaveAddr,
    char *bufP,
    UINT32 dataSize,
    IxI2cXferMode XferModeSelect)
{
    UINT32 dataSizeXmtd = 0;
    UINT32 numOfTries = 0;

    /* Disallow this function from running if I2C not initialized */
    IX_I2C_INIT_SUCCESS_CHECK("ixI2cDrvWriteTransfer");

    /* Check if the buffer provided by the client is NULL */
    if(NULL == bufP)
    {
        return IX_I2C_MASTER_NO_BUFFER;
    } /* end of bufP is zero */

    /* Check if dataSize is zero */
    if(0 == dataSize)
    {
        return IX_I2C_DATA_SIZE_ZERO;
    } /* end of dataSize is zero */

    /* Check if XferModeSelect is invalid */
    if(IX_I2C_REPEATED_START < XferModeSelect)
    {
        return IX_I2C_MASTER_INVALID_XFER_MODE;
    } /* end of XferModeSelect is invalid */

    /* Write SlaveAddr into the IDBR after shifting left 1 bit (LSB is zero
        indicating a write access for the slave that is to be accessed */
    IX_OSAL_WRITE_LONG(ixI2cDBRAddr,
                        (SlaveAddr << 1) | IX_I2C_SLAVE_WRITE);

    /* Determine if configured for poll or interrupt mode */
    if(TRUE == ixI2cInterruptMode) /* Interrupt Mode */
    {
        /* Set the flag that will be polled later for transfer complete */
        ixI2cIntrXferStatus = IX_I2C_INTR_XFER_IN_PROGRESS;

        /* Set the Op Mode flag to master write */
        ixI2cOpMode = IX_I2C_MASTER_WRITE_OPERATION;

        /* Fill the Master buffer tracker with the parameters passed in to
            track the buffer information as it moves from one interrupt
            handler to another */
        ixI2cMasterBufTracker.bufP = bufP;
        ixI2cMasterBufTracker.offset = 0;
        ixI2cMasterBufTracker.bufSize = dataSize;
        ixI2cMasterBufTracker.XferMode = XferModeSelect;

        /* Enable the start signal to control the bus and the Transfer byte
            bit to receive a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_SEND_START_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_SEND_STOP_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);

        /* Read the I2C Status Register */
        ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);
        /* Check if the I2C bus is busy before starting transfer */
        if(IX_I2C_TRIGGERED == ((ixI2cStsStored & IX_I2C_BUS_BUSY_MASK) >>
            IX_I2C_BUS_BUSY_LOC))
        {
            return IX_I2C_MASTER_BUS_BUSY;
        } /* end of I2C bus is busy */

        /* Write the configuration to the ICR to start the transfer */
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

        /* Check if master callback is NULL (sync) or not (async) */
        if(NULL == ixI2cMasterWrCallbackP)
        {
            /* Poll the ixI2cIntrXferStatus for status of transfer complete
                or transfer errors. Every 20us is 1 byte transferred at
                400 Kbps and 4 bytes transferred at 100Kbps */
            while(IX_I2C_INTR_XFER_IN_PROGRESS == ixI2cIntrXferStatus)
            {
                ixI2cDrvSleep(IX_I2C_US_POLL_FOR_XFER_STATUS);
            } /* end of while IX_I2C_TRANSFER_IN_PROGRESS */

            return ixI2cIntrXferStatus;
        } /* end of ixI2cMasterWrCallbackP = NULL */
        else /* async: so after transfer is complete (or error) master
                write callback will be called, so return. */
        {
            return IX_I2C_SUCCESS;
        } /* end of async */
    } /* end of interrupt mode */
    else /* Poll Mode */
    {
        /* Enable the start signal to control the bus and the Transfer byte
            bit to transmit a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_SEND_START_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_SEND_STOP_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);

        /* Read the I2C Status Register */
        ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);

        /* Check if the I2C bus is busy before starting transfer */
        if(IX_I2C_TRIGGERED == ((ixI2cStsStored & IX_I2C_BUS_BUSY_MASK) >>
            IX_I2C_BUS_BUSY_LOC))
        {
            return IX_I2C_MASTER_BUS_BUSY;
        } /* end of I2C bus is busy */
            
        do /* while not last byte to transmit */
        {
            /* Write the configuration to the ICR to start the transfer */
            IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

            /* Poll for the IDBR Tx Empty bit and if does not see it after
                a few tries, check if it is due to arb loss and return arb
                loss else return as transfer error. */
            IX_I2C_POLL_IDBR_TX_EMPTY_AND_CHECK_FOR_ARB_LOSS(dataSizeXmtd);
    
            /* Clear the IDBR transmit empty detected bit by writing
                one to the bit location */
            ixI2cStsStored =
                    (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_TX_EMPTY_LOC);
            IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

            /* Write data byte from client buffer into IDBR and increment the
                number of data bytes transmitted */
            IX_OSAL_WRITE_LONG(ixI2cDBRAddr, bufP[dataSizeXmtd++]);
    
            /* Disable the start signal as already got control of the bus
                and enable the Transfer byte bit to transmit a data byte */
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                        (IX_I2C_DISABLE << IX_I2C_SEND_START_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                        (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);

            IX_I2C_CHECK_FOR_BUS_ERROR; /* send master abort if bus error */
        } while (dataSize > dataSizeXmtd); /* not last byte to transmit? */

        /* Last data byte to transfer */
        /* If IX_I2C_NORMAL then send a stop to free the bus */
        if(IX_I2C_NORMAL == XferModeSelect)
        {
            /* Enable the stop signal to free control of the I2C bus. */
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                        (IX_I2C_ENABLE << IX_I2C_SEND_STOP_LOC);
        } /* end of IX_I2C_NORMAL */

        /* Write the configuration to the ICR to start the transfer */
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

        /* Poll for the IDBR Tx Empty bit and if does not see it after
            a few tries, check if it is due to arb loss and return arb
            loss else return as transfer error. */
        IX_I2C_POLL_IDBR_TX_EMPTY_AND_CHECK_FOR_ARB_LOSS(dataSizeXmtd);

        /* Clear the IDBR Tx empty bit by writing one to the bit location */
        ixI2cStsStored =
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_TX_EMPTY_LOC);
        IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

        /* Increment the stats for master transmit */
        ixI2cStatsCounters.ixI2cMasterXmitCounter += dataSizeXmtd;
    } /* end of poll mode */
    return IX_I2C_SUCCESS;
} /* end of ixI2cDrvWriteTransfer */


PUBLIC IX_I2C_STATUS
ixI2cDrvReadTransfer(
    UINT8 SlaveAddr,
    char *bufP,
    UINT32 dataSize,
    IxI2cXferMode XferModeSelect)
{
    UINT32 dataSizeRcvd = 0;
    UINT32 numOfTries = 0;

    /* Disallow this function from running if I2C not initialized */
    IX_I2C_INIT_SUCCESS_CHECK("ixI2cDrvReadTransfer");

    /* Check if the buffer provided by the client is NULL */
    if(NULL == bufP)
    {
        return IX_I2C_MASTER_NO_BUFFER;
    } /* end of bufP is NULL */

    /* Check if dataSize is zero */
    if(0 == dataSize)
    {
        return IX_I2C_DATA_SIZE_ZERO;
    } /* end of dataSize is zero */

    /* Check if XferModeSelect is invalid */
    if(IX_I2C_REPEATED_START < XferModeSelect)
    {
        return IX_I2C_MASTER_INVALID_XFER_MODE;
    } /* end of XferModeSelect is invalid */

    /* Check if SlaveAddr is invalid (zero) */
    if(IX_I2C_INVALID_SLAVE_ADDRESS == SlaveAddr)
    {
        return IX_I2C_INVALID_SLAVE_ADDR;
    } /* end of SlaveAddr is invalid (zero) */

    /* Write SlaveAddr into the IDBR after shifting left 1 bit and OR with 1
        (LSB is one indicating a read access for the slave that is to be
        accessed */
    IX_OSAL_WRITE_LONG(ixI2cDBRAddr,
                        ((SlaveAddr << 1) | IX_I2C_SLAVE_READ));

    /* Determine if configured for poll or interrupt mode */
    if(TRUE == ixI2cInterruptMode) /* Interrupt Mode */
    {
        /* Set the flag that will be polled later for transfer complete */
        ixI2cIntrXferStatus = IX_I2C_INTR_XFER_IN_PROGRESS;

        /* Set the Op Mode flag to master read */
        ixI2cOpMode = IX_I2C_MASTER_READ_OPERATION;

        /* Fill the Master buffer tracker with the parameters passed in to
            track the buffer information as it moves from one interrupt
            handler to another */
        ixI2cMasterBufTracker.bufP = bufP;
        ixI2cMasterBufTracker.offset = 0;
        ixI2cMasterBufTracker.bufSize = dataSize;
        ixI2cMasterBufTracker.XferMode = XferModeSelect;

        /* Enable the start signal to control the bus and the Transfer byte
            bit to receive a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_SEND_START_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_SEND_STOP_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);

        /* Read the I2C Status Register */
        ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);

        /* Check if the I2C bus is busy before starting transfer */
        if(IX_I2C_TRIGGERED == ((ixI2cStsStored & IX_I2C_BUS_BUSY_MASK) >>
            IX_I2C_BUS_BUSY_LOC))
        {
            return IX_I2C_MASTER_BUS_BUSY;
        } /* end of I2C bus is busy */

        /* Write the configuration to the ICR to start the transfer */
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

        /* Check if master callback is NULL (sync) or not (async) */
        if(NULL == ixI2cMasterRdCallbackP)
        {
            /* Poll the ixI2cIntrXferStatus for status of transfer complete
                or transfer errors. Every 20us is 1 byte transferred at
                400 Kbps and 4 bytes transferred at 100Kbps */
            while(IX_I2C_INTR_XFER_IN_PROGRESS == ixI2cIntrXferStatus)
            {
                ixI2cDrvSleep(IX_I2C_US_POLL_FOR_XFER_STATUS);
            } /* end of while IX_I2C_TRANSFER_IN_PROGRESS */

            return ixI2cIntrXferStatus;
        } /* end of ixI2cMasterRdCallbackP = NULL */
        else /* async: so after transfer is complete (or error) master
                read callback will be called, so return. */
        {
            return IX_I2C_SUCCESS;
        } /* end of async */
    } /* end of interrupt mode */
    else /* Poll Mode */
    {
        /* Enable the start signal to control the bus and the Transfer byte
            bit to transmit a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_SEND_START_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_SEND_STOP_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                    (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);

        /* Read the I2C Status Register */
        ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);

        /* Check if the I2C bus is busy before starting transfer */
        if(IX_I2C_TRIGGERED == ((ixI2cStsStored & IX_I2C_BUS_BUSY_MASK) >>
            IX_I2C_BUS_BUSY_LOC))
        {
            return IX_I2C_MASTER_BUS_BUSY;
        } /* end of I2C bus is busy */

        /* Write the configuration to the ICR to start the transfer */
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

        /* Poll for the IDBR Tx Empty bit and if does not see it after
            a few tries, check if it is due to arb loss and return arb
            loss else return as transfer error. */
        IX_I2C_POLL_IDBR_TX_EMPTY_AND_CHECK_FOR_ARB_LOSS(0);

        /* Clear the IDBR transmit empty detected bit by writing
            one to the bit location */
        ixI2cStsStored =
                    (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_TX_EMPTY_LOC);
        IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

        do /* more bytes to receive */
        {
            /* Check if last byte to receive */
            if(1 == (dataSize - dataSizeRcvd))
            {
                /* Check if in normal transfer (ends with a stop) */
                if(IX_I2C_NORMAL == XferModeSelect)
                {
                    /* Enable the stop signal to free control of the I2C
                        bus. */
                    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                                    (IX_I2C_ENABLE << IX_I2C_SEND_STOP_LOC);
                } /* end of IX_I2C_NORMAL */
                /* Send a Nack to end the transfer */
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                                (IX_I2C_NACK << IX_I2C_ACK_NACK_CTL_LOC);
            } /* end of last byte to receive */
            else /* not last byte to receive */
            {
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                                (IX_I2C_ACK << IX_I2C_ACK_NACK_CTL_LOC);
            } /* end of not last byte to receive */

            /* Disable the start signal as already got control of the bus
                and enable the Transfer byte bit to transmit a data byte */
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_SEND_START_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                           (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                           (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);

            IX_I2C_CHECK_FOR_BUS_ERROR; /* send master abort if bus error */

            /* Write the configuration to the ICR to start the transfer */
            IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

            /* Reset numOfTries to zero for polling */
            numOfTries = 0;

            /* Poll for IDBR Receive Full to determine data is received into
                the IDBR */
            do /* IX_I2C_TRIGGERED != ixI2cStsStored.IDBRRxFull */
            {
                ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);
                ixI2cDrvSleep(IX_I2C_US_DELAY_FOR_REG_READ);
            } while ((IX_I2C_TRIGGERED !=
                ((ixI2cStsStored & IX_I2C_IDBR_RX_FULL_MASK) >>
                IX_I2C_IDBR_RX_FULL_LOC)) &&
                (numOfTries++ < IX_I2C_NUM_TO_POLL_IDBR_RX_FULL));

            /* Check if master failed to received */
            if(IX_I2C_TRIGGERED !=
                ((ixI2cStsStored & IX_I2C_IDBR_RX_FULL_MASK) >>
                IX_I2C_IDBR_RX_FULL_LOC))
            {
                /* Read the I2C status register */
                ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);
                /* Check if receive failure is due to arbitration loss */
                if(IX_I2C_TRIGGERED ==
                    ((ixI2cStsStored & IX_I2C_ARB_LOSS_DETECTED_MASK) >>
                    IX_I2C_ARB_LOSS_DETECTED_LOC))
                {
                    /* Clear the slave address detected bit by writing
                        one to the bit location */
                    ixI2cStsStored =
                        (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_ARB_LOSS_DETECTED_LOC);
                    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
                    ixI2cStatsCounters.ixI2cArbLossCounter++;
                    return IX_I2C_MASTER_ARB_LOSS;
                } /* end of arbitration loss */
                else /* receive failure */
                {
                    /* increment the stats for number of bytes master has
                        received and one byte for master failed to
                        received and one byte for slave address transmitted */
                    ixI2cStatsCounters.ixI2cMasterRcvCounter +=
                                                            dataSizeRcvd;
                    ixI2cStatsCounters.ixI2cMasterFailedRcvCounter++;
                    ixI2cStatsCounters.ixI2cMasterXmitCounter++;
                    return IX_I2C_MASTER_XFER_ERROR;
                } /* end of receive failure */
            } /* end of master failed to receive */

            /* Clear the IDBR Receive Full detected bit by writing one to the
                bit location */
            ixI2cStsStored =
                (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_RX_FULL_LOC);
            IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

            /* Read the data from IDBR into client buffer */
            bufP[dataSizeRcvd++] = IX_OSAL_READ_LONG(ixI2cDBRAddr);
        }while (dataSize > dataSizeRcvd); /* more bytes to receive? */

        /* increment the stats for number of bytes master has received and one
            byte for slave address transmitted */
        ixI2cStatsCounters.ixI2cMasterRcvCounter += dataSizeRcvd;
        ixI2cStatsCounters.ixI2cMasterXmitCounter++;
    } /* end of poll mode */

    return IX_I2C_SUCCESS;
} /* end of ixI2cDrvReadTransfer */


PUBLIC IX_I2C_STATUS
ixI2cDrvSlaveAddrAndGenCallDetectedCheck(void)
{
    /* Disallow this function from running if I2C not initialized */
    IX_I2C_INIT_SUCCESS_CHECK("ixI2cDrvSlaveAddrAndGenCallDetectedCheck");

    /* Read the I2C Status Register */
    ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);

    /* Check if a Slave address detected was triggered */
    if(IX_I2C_TRIGGERED !=
        ((ixI2cStsStored & IX_I2C_SLAVE_ADDR_DETECTED_MASK) >>
        IX_I2C_SLAVE_ADDR_DETECTED_LOC))
    {
        return IX_I2C_SLAVE_ADDR_NOT_DETECTED;
    } /* end of slave address triggered */

    /* Check if a general call address detected was triggered */
    if(IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_GEN_CALL_ADDR_DETECTED_MASK) >>
        IX_I2C_GEN_CALL_ADDR_DETECTED_LOC))
    {
        /* Clear the slave address detected and general call address
            detected bits by writing one to bit locations */
        ixI2cStsStored =
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_ADDR_DETECTED_LOC) |
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_GEN_CALL_ADDR_DETECTED_LOC);
        IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
        return IX_I2C_GEN_CALL_ADDR_DETECTED;
    } /* end of General Call detected */

    /* Check if a Slave write detected was triggered */
    if(IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_READ_WRITE_MODE_MASK) >>
        IX_I2C_READ_WRITE_MODE_LOC))
    {
        /* Clear the slave address detected bit by writing one to the bit
            location */
        ixI2cStsStored =
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_ADDR_DETECTED_LOC);
        IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
        return IX_I2C_SLAVE_WRITE_DETECTED;
    } /* end of Slave Write detected */

    /* Clear the slave address detected bit by writing one to the bit location */
    ixI2cStsStored =
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_ADDR_DETECTED_LOC);
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
    return IX_I2C_SLAVE_READ_DETECTED;
} /* end of ixI2cDrvSlaveAddrAndGenCallDetectedCheck */


PUBLIC IX_I2C_STATUS
ixI2cDrvSlaveOrGenDataReceive(
    char *bufP,
    const UINT32 bufSize,
    UINT32 *dataSizeRcvd)
{
    UINT32 numOfTries = 0;

    /* Disallow this function from running if I2C not initialized */
    IX_I2C_INIT_SUCCESS_CHECK("ixI2cDrvSlaveOrGenDataReceive");


    /* Check if buffer pointer is NULL */
    if(NULL == bufP)
    {
        return IX_I2C_SLAVE_NO_BUFFER;
    } /* end of buffer pointer is NULL */

    /* Check if dataSize is zero */
    if(0 == bufSize)
    {
        return IX_I2C_DATA_SIZE_ZERO;
    } /* end of bufSize is zero */

    /* Check if dataSizeXmtd pointer is NULL */
    if(NULL == dataSizeRcvd)
    {
        return IX_I2C_NULL_POINTER;
    } /* end of dataSizeXmtd pointer is NULL */
    
    dataSizeRcvd[0] = 0; /* Reset the number of data received to zero */

    /* Keep receiving data until slave stop is detected */
    do /* while not slave stop detected or error */
    {
        /* Reset the numOfTries to zero */
        numOfTries = 0;

        /* Enable the Transfer byte bit to receive a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                    (IX_I2C_ACK << IX_I2C_ACK_NACK_CTL_LOC);
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
    
        /* Poll for IDBR Receive Full to determine there's data in the IDBR
            to be read */
        do /* IX_I2C_TRIGGERED != ixI2cStsStored.IDBRRxFull */
        {
            ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);
            ixI2cDrvSleep(IX_I2C_US_DELAY_FOR_REG_READ);
        } while ((IX_I2C_TRIGGERED !=
            ((ixI2cStsStored & IX_I2C_IDBR_RX_FULL_MASK) >>
            IX_I2C_IDBR_RX_FULL_LOC)) &&
            (IX_I2C_TRIGGERED !=
            ((ixI2cStsStored & IX_I2C_SLAVE_STOP_DETECTED_MASK) >>
            IX_I2C_SLAVE_STOP_DETECTED_LOC)) &&
            (numOfTries++ < IX_I2C_NUM_TO_POLL_IDBR_RX_FULL));

        /* Check if slave received */
        if(IX_I2C_TRIGGERED ==
            ((ixI2cStsStored & IX_I2C_IDBR_RX_FULL_MASK) >>
            IX_I2C_IDBR_RX_FULL_LOC))
        {
            /* Check if exceeded buffer size */
            if(dataSizeRcvd[0] >= bufSize)
            {
                /* increment the stats for number of bytes slave has received */
                ixI2cStatsCounters.ixI2cSlaveRcvCounter += bufSize;
                return IX_I2C_SLAVE_OR_GEN_READ_BUFFER_FULL;
            } /* end of check before write into buffer */

            /* Read data byte from IDBR into client buffer and increment the
                number of data bytes received counter */
            bufP[dataSizeRcvd[0]++] = IX_OSAL_READ_LONG(ixI2cDBRAddr);
            /* Clear the IDBR receive full bit by writing one
                to the bit location */
            ixI2cStsStored =
                (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_RX_FULL_LOC);
            IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
        } /* end of slave receive */
        else if (IX_I2C_TRIGGERED ==
            ((ixI2cStsStored & IX_I2C_SLAVE_STOP_DETECTED_MASK) >>
            IX_I2C_SLAVE_STOP_DETECTED_LOC))
        {
            /* Clear the slave stop bit by writing one to the bit location */
            ixI2cStsStored =
                (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_STOP_DETECTED_LOC);
            IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
            ixI2cStatsCounters.ixI2cSlaveRcvCounter += dataSizeRcvd[0];
            return IX_I2C_SUCCESS;
        } /* end of SlaveStopDetected */
        else /* slave receive fail */
        {
            /* increment the stats for number of bytes slave has received
                and one byte for slave failed to received */
            ixI2cStatsCounters.ixI2cSlaveRcvCounter += dataSizeRcvd[0];
            ixI2cStatsCounters.ixI2cSlaveFailedRcvCounter++;
            return IX_I2C_SLAVE_OR_GEN_READ_ERROR;
        } /* end of slave receive fail */
    } while (1);
} /* end of ixI2cDrvSlaveOrGenDataReceive */


PUBLIC IX_I2C_STATUS
ixI2cDrvSlaveDataTransmit(
    char *bufP,
    const UINT32 dataSize,
    UINT32 *dataSizeXmtd)
{
    UINT32 numOfTries = 0;
    BOOL ixI2cSlaveStopExpected = FALSE;

    /* Disallow this function from running if I2C not initialized */
    IX_I2C_INIT_SUCCESS_CHECK("ixI2cDrvSlaveDataTransmit");

    /* Check if buffer pointer is NULL */
    if(NULL == bufP)
    {
        return IX_I2C_SLAVE_NO_BUFFER;
    } /* end of buffer pointer is NULL */

    /* Check if dataSize is zero */
    if(0 == dataSize)
    {
        return IX_I2C_DATA_SIZE_ZERO;
    } /* end of dataSize is zero */

    /* Check if dataSizeXmtd pointer is NULL */
    if(NULL == dataSizeXmtd)
    {
        return IX_I2C_NULL_POINTER;
    } /* end of datSizeXmtd pointer is NULL */
    
    dataSizeXmtd[0] = 0;
    ixI2cSlaveStopExpected = FALSE;

    /* Keep transmitting data until slave stop is detected */
    do /* start of ixI2cSlaveStopExpected = FALSE */
    {
        /* Reset the numOfTries to zero */
        numOfTries = 0;

        /* Check if the client buffer is already emptied */
        if(dataSizeXmtd[0] >= dataSize)
        {
            /* increment the stats for number of bytes slave has transmitted */
            ixI2cStatsCounters.ixI2cSlaveXmitCounter += dataSize;
            return IX_I2C_SLAVE_WRITE_BUFFER_EMPTY;
        } /* end of dataSizeXmtd[0] >= dataSize */

        /* Write data byte from client buffer into IDBR and increment the
            number of data bytes transmitted */
        IX_OSAL_WRITE_LONG(ixI2cDBRAddr, bufP[dataSizeXmtd[0]++]);

        /* Enable the Transfer byte bit to transmit a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
    
        /* Poll for IDBR Transmit Empty to determine the data in the IDBR
            has bean transmitted */
        do /* IX_I2C_TRIGGERED != ixI2cStsStored.IDBRTxEmpty */
        {
            ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);
            ixI2cDrvSleep(IX_I2C_US_DELAY_FOR_REG_READ);
        } while ((IX_I2C_TRIGGERED !=
            ((ixI2cStsStored & IX_I2C_IDBR_TX_EMPTY_MASK) >>
            IX_I2C_IDBR_TX_EMPTY_LOC)) &&
            (numOfTries++ < IX_I2C_NUM_TO_POLL_IDBR_TX_EMPTY));

        /* Check if slave successfully transmitted */
        if(IX_I2C_TRIGGERED ==
            ((ixI2cStsStored & IX_I2C_IDBR_TX_EMPTY_MASK) >>
            IX_I2C_IDBR_TX_EMPTY_LOC))
        {
            if(IX_I2C_NACK ==
                ((ixI2cStsStored & IX_I2C_ACK_NACK_STATUS_MASK) >>
                IX_I2C_ACK_NACK_STATUS_LOC))
            {
                ixI2cSlaveStopExpected = TRUE;
            } /* end of AckNackStatus = IX_I2C_NACK */
            /* Clear the IDBR transmit empty bit by writing 1 to the bit
                location */
            ixI2cStsStored =
                (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_TX_EMPTY_LOC);
            IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
        } /* end of slave successfully transmitted */
        else /* failed to transmit */
        {
            /* increment the stats for number of bytes slave has transmitted
                and one byte for slave failed to transmit */
            ixI2cStatsCounters.ixI2cSlaveXmitCounter += dataSizeXmtd[0];
            ixI2cStatsCounters.ixI2cSlaveFailedXmitCounter++;
            return IX_I2C_SLAVE_WRITE_ERROR;
        } /* end of failed to transmit */
    } while (FALSE == ixI2cSlaveStopExpected);

    ixI2cStatsCounters.ixI2cSlaveXmitCounter += dataSizeXmtd[0];

    /* Reset the numOfTries to zero */
    numOfTries = 0;

    /* Enable the Transfer byte bit to transmit a data byte */
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
    IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
    
    /* Poll for Slave Stop to determine end of transmission */
    do /* IX_I2C_TRIGGERED != ixI2cStsStored.SlaveStopDetected */
    {
        ixI2cStsStored = IX_OSAL_READ_LONG(ixI2cSRAddr);
        ixI2cDrvSleep(IX_I2C_US_DELAY_FOR_REG_READ);
    } while ((IX_I2C_TRIGGERED !=
        ((ixI2cStsStored & IX_I2C_SLAVE_STOP_DETECTED_MASK) >>
        IX_I2C_SLAVE_STOP_DETECTED_LOC)) &&
        (numOfTries++ < IX_I2C_NUM_TO_POLL_IDBR_TX_EMPTY));

    if(IX_I2C_TRIGGERED !=
        ((ixI2cStsStored & IX_I2C_SLAVE_STOP_DETECTED_MASK) >>
        IX_I2C_SLAVE_STOP_DETECTED_LOC))
    {
        return IX_I2C_SLAVE_WRITE_ERROR;
    } /* end of SlaveStopDetected != IX_I2C_TRIGGERED */

    /* Clear the Slave Stop Detected bit by writing 1 to the bit location */
    ixI2cStsStored =
        (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_STOP_DETECTED_LOC);
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

    return IX_I2C_SUCCESS;
} /* end of ixI2cDrvSlaveDataTransmit */


PUBLIC void
ixI2cDrvSlaveOrGenCallBufReplenish(
    char *bufP,
    UINT32 bufSize)
{
    /* Replenish the buffer with new buffer */
    ixI2cSlaveOrGenBufTracker.bufP = bufP;
    ixI2cSlaveOrGenBufTracker.offset = 0;
    ixI2cSlaveOrGenBufTracker.bufSize = bufSize;

} /* end of ixI2cDrvSlaveOrGenCallBufReplenish */

    
PUBLIC IX_I2C_STATUS
ixI2cDrvStatsGet(IxI2cStatsCounters *I2cStats)
{
    if(NULL == I2cStats)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixI2cDrvStatsGet: stats pointer is NULL.\n",
            0,0,0,0,0,0);
        return IX_I2C_NULL_POINTER;
    } /* end of I2cStats = NULL */
    
    I2cStats->ixI2cMasterXmitCounter =
                ixI2cStatsCounters.ixI2cMasterXmitCounter;
    I2cStats->ixI2cMasterFailedXmitCounter =
                ixI2cStatsCounters.ixI2cMasterFailedXmitCounter;
    I2cStats->ixI2cMasterRcvCounter =
                ixI2cStatsCounters.ixI2cMasterRcvCounter;
    I2cStats->ixI2cMasterFailedRcvCounter =
                ixI2cStatsCounters.ixI2cMasterFailedRcvCounter;
    I2cStats->ixI2cSlaveXmitCounter =
                ixI2cStatsCounters.ixI2cSlaveXmitCounter;
    I2cStats->ixI2cSlaveFailedXmitCounter =
                ixI2cStatsCounters.ixI2cSlaveFailedXmitCounter;
    I2cStats->ixI2cSlaveRcvCounter =
                ixI2cStatsCounters.ixI2cSlaveRcvCounter;
    I2cStats->ixI2cSlaveFailedRcvCounter =
                ixI2cStatsCounters.ixI2cSlaveFailedRcvCounter;
    I2cStats->ixI2cGenAddrCallSucceedCounter =
                ixI2cStatsCounters.ixI2cGenAddrCallSucceedCounter;
    I2cStats->ixI2cGenAddrCallFailedCounter =
                ixI2cStatsCounters.ixI2cGenAddrCallFailedCounter;
    I2cStats->ixI2cArbLossCounter =
                ixI2cStatsCounters.ixI2cArbLossCounter;

    return IX_I2C_SUCCESS;
} /* end of ixI2cDrvStatsGet */


PUBLIC void
ixI2cDrvStatsReset(void)
{
    /* Clear the I2C stats counters to zero */
    ixI2cStatsCounters.ixI2cMasterXmitCounter = 0;
    ixI2cStatsCounters.ixI2cMasterFailedXmitCounter = 0;
    ixI2cStatsCounters.ixI2cMasterRcvCounter = 0;
    ixI2cStatsCounters.ixI2cMasterFailedRcvCounter = 0;
    ixI2cStatsCounters.ixI2cSlaveXmitCounter = 0;
    ixI2cStatsCounters.ixI2cSlaveFailedXmitCounter = 0;
    ixI2cStatsCounters.ixI2cSlaveRcvCounter = 0;
    ixI2cStatsCounters.ixI2cSlaveFailedRcvCounter = 0;
    ixI2cStatsCounters.ixI2cGenAddrCallSucceedCounter = 0;
    ixI2cStatsCounters.ixI2cGenAddrCallFailedCounter = 0;
    ixI2cStatsCounters.ixI2cArbLossCounter = 0;
} /* end of ixI2cDrvStatsReset */


PUBLIC IX_I2C_STATUS
ixI2cDrvShow(void)
{
    /* Disallow this function from running if I2C not initialized */
    IX_I2C_INIT_SUCCESS_CHECK("ixI2cDrvShow");

    /* Read and display the I2C register status */
    ixI2cStsStored = IX_OSAL_READ_LONG (ixI2cSRAddr);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "1 = YES, 0 = NO\n",0,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Bus Error Detected        : %d\n",
        ((ixI2cStsStored & IX_I2C_BUS_ERROR_DETECTED_MASK) >>
        IX_I2C_BUS_ERROR_DETECTED_LOC),0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Slave Address Detected     : %d\n",
        ((ixI2cStsStored & IX_I2C_SLAVE_ADDR_DETECTED_MASK) >>
        IX_I2C_SLAVE_ADDR_DETECTED_LOC),0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Gen Call Address Detected  : %d\n",
        ((ixI2cStsStored & IX_I2C_GEN_CALL_ADDR_DETECTED_MASK) >>
        IX_I2C_GEN_CALL_ADDR_DETECTED_LOC),0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "IDBR Rx Full Detected      : %d\n",
        ((ixI2cStsStored & IX_I2C_IDBR_RX_FULL_MASK) >>
        IX_I2C_IDBR_RX_FULL_LOC),0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "IDBR Tx Empty Detected     : %d\n",
        ((ixI2cStsStored & IX_I2C_IDBR_TX_EMPTY_MASK) >>
        IX_I2C_IDBR_TX_EMPTY_LOC),0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Arbitration Loss Detected  : %d\n",
        ((ixI2cStsStored & IX_I2C_ARB_LOSS_DETECTED_MASK) >>
        IX_I2C_ARB_LOSS_DETECTED_LOC),0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Slave Stop Detected        : %d\n",
        ((ixI2cStsStored & IX_I2C_SLAVE_STOP_DETECTED_MASK) >>
        IX_I2C_SLAVE_STOP_DETECTED_LOC),0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Bus is Busy            : %d\n",
        ((ixI2cStsStored & IX_I2C_BUS_BUSY_MASK) >>
        IX_I2C_BUS_BUSY_LOC),0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Unit is Busy           : %d\n",
        ((ixI2cStsStored & IX_I2C_UNIT_BUSY_MASK) >>
        IX_I2C_UNIT_BUSY_LOC),0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C last sent or received a: ",0,0,0,0,0,0);
    if(IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_ACK_NACK_STATUS_MASK) >>
        IX_I2C_ACK_NACK_STATUS_LOC))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "Nack\n",0,0,0,0,0,0);
    } /* end of Ack status last sent or received */
    else /* Nack status last sent or received */
    {
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "Ack\n",0,0,0,0,0,0);
    } /* end of Nack status last sent or received */
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C mode is                : ",0,0,0,0,0,0);
    if(IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_READ_WRITE_MODE_MASK) >>
        IX_I2C_READ_WRITE_MODE_LOC))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "master-rx or slave-tx\n",0,0,0,0,0,0);
    } /* end of ReadWriteMode triggered */
    else /* ReadWriteMode not triggered */
    {
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "master-tx or slave-rx\n",0,0,0,0,0,0);
    } /* end of ReadWriteMode not triggered */

    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C delay type selected    : ",0,0,0,0,0,0);
    if(IX_I2C_LOOP_DELAY == ixI2cDelayType)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "Loop delay\n",0,0,0,0,0,0);
    } /* end of loop delay */
    else /* scheduled delay */
    {
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "Scheduled delay\n",0,0,0,0,0,0);
    } /* end of scheduled delay */

    /* Display the I2C stats counters */
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Master transmitted       : %d\n",
        ixI2cStatsCounters.ixI2cMasterXmitCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Master transmitted failed: %d\n",
        ixI2cStatsCounters.ixI2cMasterFailedXmitCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Master received          : %d\n",
        ixI2cStatsCounters.ixI2cMasterRcvCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Master received failed   : %d\n",
        ixI2cStatsCounters.ixI2cMasterFailedRcvCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Slave transmitted        : %d\n",
        ixI2cStatsCounters.ixI2cSlaveXmitCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Slave transmitted failed : %d\n",
        ixI2cStatsCounters.ixI2cSlaveFailedXmitCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Slave received           : %d\n",
        ixI2cStatsCounters.ixI2cSlaveRcvCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Slave received failed    : %d\n",
        ixI2cStatsCounters.ixI2cSlaveFailedRcvCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C General call succeeded   : %d\n",
        ixI2cStatsCounters.ixI2cGenAddrCallSucceedCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C General call failed      : %d\n",
        ixI2cStatsCounters.ixI2cGenAddrCallFailedCounter,0,0,0,0,0);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "I2C Arbitration Loss         : %d\n",
        ixI2cStatsCounters.ixI2cArbLossCounter,0,0,0,0,0);

    return IX_I2C_SUCCESS;
} /* end of ixI2cDrvShow */

PUBLIC void
ixI2cDrvDelayTypeSelect (IxI2cDelayMode delayTypeSelect)
{
    if(IX_I2C_LOOP_DELAY == delayTypeSelect)
    {
        ixI2cDelayType = IX_I2C_LOOP_DELAY;
    }
    else if(IX_I2C_SCHED_DELAY == delayTypeSelect)
    {
        ixI2cDelayType = IX_I2C_SCHED_DELAY;
    }
    else
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixI2cDrvDelayTypeSelect: Invalid IxI2cDelayMode selected.\n",
            0,0,0,0,0,0);
    }
    return;
}

/**
 * Private functions definitions
 */
PRIVATE void ixI2cDrvInterruptDetected (void)
{
    /* Read the I2C status register */
    ixI2cStsStored = IX_OSAL_READ_LONG (ixI2cSRAddr);

    /* Check the source of the interrupt in the order starting with
        Bus Error, Arb Loss, IDBR Rx Full, IDBR Tx Full, General Call detected,
        Slave address detected, and finally slave stop detected */
    if(IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_BUS_ERROR_DETECTED_MASK) >>
        IX_I2C_BUS_ERROR_DETECTED_LOC))
    {
        ixI2cDrvBusErrorHdlr();
    } /* end of BusErrorDetected */
    else if(IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_ARB_LOSS_DETECTED_MASK) >>
        IX_I2C_ARB_LOSS_DETECTED_LOC))
    {
        ixI2cDrvArbLossDetectedHdlr();
    } /* end of ArbLossDetected */
    else if(IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_IDBR_RX_FULL_MASK) >>
        IX_I2C_IDBR_RX_FULL_LOC))
    {
        ixI2cDrvIDBRRxFullHdlr();
    } /* end of IDBRRxFull */
    else if(IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_IDBR_TX_EMPTY_MASK) >>
        IX_I2C_IDBR_TX_EMPTY_LOC))
    {
        ixI2cDrvIDBRTxEmptyHdlr();
    } /* end of IDBRTxEmpty */
    else if((IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_GEN_CALL_ADDR_DETECTED_MASK) >>
        IX_I2C_GEN_CALL_ADDR_DETECTED_LOC)) &&
        (ixI2cGenCallCallbackP != NULL))
    {
        ixI2cDrvGenCallAddrDetectedHdlr();
        } /* end of GenCallAddrDetected */
    else if((IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_SLAVE_ADDR_DETECTED_MASK) >>
        IX_I2C_SLAVE_ADDR_DETECTED_LOC)) &&
        (ixI2cSlaveRdCallbackP != NULL) && (ixI2cSlaveWrCallbackP != NULL))
    {
        ixI2cDrvSlaveAddrDetectedHdlr();
    } /* end of SlaveAddrDetected */
    else if (IX_I2C_TRIGGERED ==
        ((ixI2cStsStored & IX_I2C_SLAVE_STOP_DETECTED_MASK) >>
        IX_I2C_SLAVE_STOP_DETECTED_LOC))
    {
        ixI2cDrvSlaveStopDetectedHdlr();
    } /* end of SlaveStopDetected */
    else /* invalid interrupt state */
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "Invalid interrupt state. Disabling I2C\n",
            0,0,0,0,0,0);
        if(IX_I2C_SUCCESS != ixI2cDrvUninit())
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Calling of ixI2cDrvUninit is unsuccessful.\n",
                0,0,0,0,0,0);
        } /* end of IX_I2C_SUCCESS != ixI2cDrvUninit() */
    } /* end of invalid interrupt state */
    return;
} /* end of ixI2cDrvInterruptDetected */


PRIVATE void ixI2cDrvBusErrorHdlr (void)
{
    /* Set the flag to indicate a bus error */
    ixI2cIntrXferStatus = IX_I2C_INTR_BUS_ERROR;
    
    if(IX_I2C_MASTER_READ_OPERATION == ixI2cOpMode)
    {
        if(NULL != ixI2cMasterRdCallbackP)
        {
            ixI2cMasterRdCallbackP(IX_I2C_MASTER_BUS_ERROR,
                        ixI2cMasterBufTracker.XferMode,
                        ixI2cMasterBufTracker.bufP,
                        ixI2cMasterBufTracker.bufSize);
        } /* end of NULL != ixI2cMasterRdCallbackP */

        /* increment the stats for number of bytes master has transmitted
            and one byte for master failed to transmit */
        ixI2cStatsCounters.ixI2cMasterRcvCounter +=
            ixI2cMasterBufTracker.offset;
        ixI2cStatsCounters.ixI2cMasterFailedRcvCounter++;
    } /*end of IX_I2C_MASTER_READ_OPERATION */

    else if(IX_I2C_MASTER_WRITE_OPERATION == ixI2cOpMode)
    {
        if(NULL != ixI2cMasterWrCallbackP)
        {
            ixI2cMasterWrCallbackP(IX_I2C_MASTER_BUS_ERROR,
                        ixI2cMasterBufTracker.XferMode,
                        ixI2cMasterBufTracker.bufP,
                        ixI2cMasterBufTracker.bufSize);
        } /* end of NULL != ixI2cMasterWrCallbackP */

        /* increment the stats for number of bytes master has transmitted
            and one byte for master failed to transmit */
        ixI2cStatsCounters.ixI2cMasterXmitCounter+=
            ixI2cMasterBufTracker.offset;
        ixI2cStatsCounters.ixI2cMasterFailedXmitCounter++;
    } /* end of IX_I2C_MASTER_WRITE_OPERATION */

    else if(IX_I2C_SLAVE_READ_OPERATION == ixI2cOpMode)
    {
        ixI2cStatsCounters.ixI2cSlaveRcvCounter +=
            ixI2cSlaveOrGenBufTracker.offset;
        ixI2cStatsCounters.ixI2cSlaveFailedRcvCounter++;
        return;
    } /* end of IX_I2C_SLAVE_READ_OPERATION */

    else if(IX_I2C_GENERAL_CALL_OPERATION == ixI2cOpMode)
    {
        ixI2cStatsCounters.ixI2cGenAddrCallSucceedCounter +=
            ixI2cSlaveOrGenBufTracker.offset;
        ixI2cStatsCounters.ixI2cGenAddrCallFailedCounter++;
        return;
    } /* end of IX_I2C_GENERAL_CALL_OPERATION */

    else /* IX_I2C_SLAVE_WRITE_OPERATION */
    {/* This shouldn't happen, log an error */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixI2cDrvBusErrorHdlr:"
            " occured during slave write!\n", 0,0,0,0,0,0);
        return;
    } /* end of IX_I2C_SLAVE_WRITE_OPERATION */

    /* Send a master abort to abort transfers */    
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                (IX_I2C_ENABLE << IX_I2C_MASTER_ABORT_LOC);
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                (IX_I2C_DISABLE << IX_I2C_SEND_START_LOC);
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                (IX_I2C_DISABLE << IX_I2C_TRANSFER_BYTE_LOC);
    IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

    /* Clear the bus error detected bit and IDBR Tx Empty bit by writing one
        to the bit location. The IDBR Tx Empty bit is also cleared because
        the bus error happens on a master Tx. */
    ixI2cStsStored =
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_BUS_ERROR_DETECTED_LOC) |
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_TX_EMPTY_LOC);
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

    return;
} /* end of ixI2cDrvBusErrorHdlr */


PRIVATE void ixI2cDrvSlaveAddrDetectedHdlr (void)
{
    UINT32 numOfTries = 0;

    ixI2cStsStored = IX_OSAL_READ_LONG (ixI2cSRAddr);

    /* Check the I2C status register's read/write mode bit to determine
        whether it is a slave read or slave write requested */
    if(IX_I2C_SLAVE_READ_MODE ==
        ((ixI2cStsStored & IX_I2C_READ_WRITE_MODE_MASK) >>
        IX_I2C_READ_WRITE_MODE_LOC))
    {
        /* Set the I2C op mode flag to indicate a slave read operation */
        ixI2cOpMode = IX_I2C_SLAVE_READ_OPERATION;
    } /* end of IX_I2C_SLAVE_READ_MODE */
    else /* IX_I2C_SLAVE_WRITE_MODE */
    {
        /* Set the I2C op mode flag to indicate a slave write operation */
        ixI2cOpMode = IX_I2C_SLAVE_WRITE_OPERATION;

        /* Check if buffer is NULL and buffer empty */
        IX_I2C_CHECK_SLAVE_OR_GEN_BUFFER_NULL(ixI2cSlaveWrCallbackP,
                                    "ixI2cSlaveWrCallbackP",
                                    "ixI2cDrvSlaveAddrDetectedHdlr",);
        IX_I2C_CHECK_SLAVE_BUFFER_EMPTY("ixI2cDrvSlaveAddrDetectedHdlr",);

        IX_OSAL_WRITE_LONG(ixI2cDBRAddr,
            ixI2cSlaveOrGenBufTracker.bufP[ixI2cSlaveOrGenBufTracker.offset++]);
    } /* end of IX_I2C_SLAVE_WRITE_MODE */

    /* Enable the Transfer byte bit to transfer a data byte */
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                    (IX_I2C_ACK << IX_I2C_ACK_NACK_CTL_LOC);
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
    IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

    /* Clear the slave address detected bit by writing 1 to the bit location */
    ixI2cStsStored =
        (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_ADDR_DETECTED_LOC);
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

    return;
} /* end of ixI2cDrvSlaveAddrDetectedHdlr */

PRIVATE void ixI2cDrvGenCallAddrDetectedHdlr (void)
{
    /* Set the I2C Operation Mode flag to indicate a general call operation */
    ixI2cOpMode = IX_I2C_GENERAL_CALL_OPERATION;

    /* Enable the Transfer byte bit to receive a data byte */
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                    (IX_I2C_ACK << IX_I2C_ACK_NACK_CTL_LOC);
    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                    (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
    IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);

    /* Clear the slave address detected and general call address detected bits
        by writing one to the bit locations */
    ixI2cStsStored =
        (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_ADDR_DETECTED_LOC) |
        (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_GEN_CALL_ADDR_DETECTED_LOC);
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

    return;
} /* end of ixI2cDrvGenCallAddrDetectedHdlr */

PRIVATE void ixI2cDrvIDBRRxFullHdlr (void)
{
    UINT32 numOfTries = 0;

    if(IX_I2C_MASTER_READ_OPERATION == ixI2cOpMode)
    {
        ixI2cMasterBufTracker.bufP[ixI2cMasterBufTracker.offset++] =
            IX_OSAL_READ_LONG(ixI2cDBRAddr);

        /* Check if master has received all its data */
        if(ixI2cMasterBufTracker.bufSize <=
                ixI2cMasterBufTracker.offset)
        {
            /* Call the master Read callback (async) indicating a transfer
            complete if it is not NULL else indicate a transfer complete
            through the XferStatus flag */
            if(NULL == ixI2cMasterRdCallbackP)
            {
                ixI2cIntrXferStatus = IX_I2C_INTR_XFER_SUCCESSFUL;
            } /* end of NULL == ixI2cMasterRdCallbackP */
            else /* NULL != ixI2cMasterRdCallbackP */
            {
                ixI2cMasterRdCallbackP(IX_I2C_MASTER_XFER_COMPLETE,
                        ixI2cMasterBufTracker.XferMode,
                        ixI2cMasterBufTracker.bufP,
                        ixI2cMasterBufTracker.bufSize);
            } /* end of NULL != ixI2cMasterRdCallbackP */

            /* increment the stats for number of bytes master has received
                and one byte for slave address transmitted */
            ixI2cStatsCounters.ixI2cMasterRcvCounter +=
                                        ixI2cMasterBufTracker.offset;
            ixI2cStatsCounters.ixI2cMasterXmitCounter++;
        } /* end of master has received all its data */
        /* Check if there's more than one byte of data to receive */
        else if(((ixI2cMasterBufTracker.bufSize - ixI2cMasterBufTracker.offset)
                > 1))
        {
            /* Enable the Transfer byte bit to receive a data byte */
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_SEND_START_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                            (IX_I2C_ACK << IX_I2C_ACK_NACK_CTL_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                            (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
            IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
        } /* end of more than one byte of data to receive */
        else /* last byte to receive */
        {
            /* Enable the Transfer byte bit to receive a data byte */
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_SEND_START_LOC);

            /* If IX_I2C_NORMAL then send a stop to free the bus */
            if(IX_I2C_NORMAL == ixI2cMasterBufTracker.XferMode)
            {
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                                (IX_I2C_ENABLE << IX_I2C_SEND_STOP_LOC);
            } /* end of IX_I2C_NORMAL */
            /* Send a Nack to end transfer */
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                            (IX_I2C_NACK << IX_I2C_ACK_NACK_CTL_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                            (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
            IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
        } /* end of last byte to receive in normal transfer (ends with stop) */
    } /* end of IX_I2C_MASTER_READ_OPERATION */
    else if(IX_I2C_SLAVE_READ_OPERATION == ixI2cOpMode)
    {
        /* Check if buffer is null and buffer is full */
        IX_I2C_CHECK_SLAVE_OR_GEN_BUFFER_NULL(ixI2cSlaveRdCallbackP,
                                        "ixI2cSlaveRdCallbackP",
                                        "ixI2cDrvIDBRRxFullHdlr",);
        IX_I2C_CHECK_SLAVE_OR_GEN_BUFFER_FULL(ixI2cSlaveRdCallbackP,
                                        "ixI2cSlaveRdCallbackP",
                                        "ixI2cDrvIDBRRxFullHdlr",);

        /* Read the IDBR data into the slave buffer */
        ixI2cSlaveOrGenBufTracker.bufP[ixI2cSlaveOrGenBufTracker.offset++] =
            IX_OSAL_READ_LONG(ixI2cDBRAddr);

        /* Enable the Transfer byte bit to receive a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                        (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
    }/* end of IX_I2C_SLAVE_READ_OPERATION */
    else if(IX_I2C_GENERAL_CALL_OPERATION == ixI2cOpMode)
    {
        /* Check if buffer is null and buffer full */
        IX_I2C_CHECK_SLAVE_OR_GEN_BUFFER_NULL(ixI2cGenCallCallbackP,
                                        "ixI2cGenCallCallbackP",
                                        "ixI2cDrvIDBRRxFullHdlr",);
        IX_I2C_CHECK_SLAVE_OR_GEN_BUFFER_FULL(ixI2cGenCallCallbackP,
                                        "ixI2cGenCallCallbackP",
                                        "ixI2cDrvIDBRRxFullHdlr",);

        /* Read the IDBR data into the general call buffer */
        ixI2cSlaveOrGenBufTracker.bufP[ixI2cSlaveOrGenBufTracker.offset++] =
            IX_OSAL_READ_LONG(ixI2cDBRAddr);

        /* Enable the Transfer byte bit to receive a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                        (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
    } /* end of IX_I2C_GENERAL_CALL_OPERATION */
    else /* It shouldn't reach here. Log an error */
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixI2cDrvIDBRRxFullHdlr:"
            " occured during a write operation!\n", 0,0,0,0,0,0);
    } /* end of code that should not be reached */

    /* Clear the IDBR Rx Full detected bit by writing one to the bit location */
    ixI2cStsStored =
        (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_RX_FULL_LOC);
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

    return;
} /* end of ixI2cDrvIDBRRxFullHdlr */


PRIVATE void ixI2cDrvIDBRTxEmptyHdlr (void)
{
    UINT32 numOfTries = 0;

    /* Check if it is a master read, master write, or a slave write. A master
        read still requires a first master write for the slave address it is
        going to access. */
    if(IX_I2C_MASTER_WRITE_OPERATION == ixI2cOpMode)
    {
        /* Check if there's data to transmit */
        if(ixI2cMasterBufTracker.offset >= ixI2cMasterBufTracker.bufSize)
        { /* No more data to transmit */
            /* Check if Master Write callback is NULL. If NULL, then set the
                Xfer status to indicate a successful transfer. Else, call the
                Master Write Callback indicating transfer complete */
            if(NULL == ixI2cMasterWrCallbackP)
            {
                ixI2cIntrXferStatus = IX_I2C_INTR_XFER_SUCCESSFUL;
            } /* end of NULL == ixI2cMasterWrCallbackP */
            else /* ixI2cMasterWrCallbackP != NULL */
            {
                ixI2cMasterWrCallbackP(IX_I2C_MASTER_XFER_COMPLETE,
                    ixI2cMasterBufTracker.XferMode,
                    ixI2cMasterBufTracker.bufP,
                    ixI2cMasterBufTracker.bufSize);
            } /* end of ixI2cMasterWrCallbackP != NULL */

            /* increment the stats for number of bytes master has transmitted
                and one byte for slave address transmitted */
            ixI2cStatsCounters.ixI2cMasterXmitCounter +=
                                        (ixI2cMasterBufTracker.offset + 1);
        } /* end of no more data to transmit */
        else /* More data to transmit */
        {
            IX_OSAL_WRITE_LONG(ixI2cDBRAddr,
                ixI2cMasterBufTracker.bufP[ixI2cMasterBufTracker.offset]);

            /* check if it is last data byte to transmit */
            if(1 == 
                (ixI2cMasterBufTracker.bufSize - ixI2cMasterBufTracker.offset))
            {
                /* If IX_I2C_NORMAL then send a stop to free the bus */
                if(IX_I2C_NORMAL == ixI2cMasterBufTracker.XferMode)
                {
                    /* Enable sending a stop signal */
                    ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                                    (IX_I2C_ENABLE << IX_I2C_SEND_STOP_LOC);
                } /* end of IX_I2C_NORMAL */
    
                /* Enable the Transfer byte bit to receive a data byte */
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                                (IX_I2C_DISABLE << IX_I2C_SEND_START_LOC);
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                                (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                                (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
                IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
            } /* end of last data byte to transmit */
            else /* more data bytes to transmit (not last data byte) */
            {
                /* Enable the Transfer byte bit to receive a data byte */
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                                (IX_I2C_DISABLE << IX_I2C_SEND_START_LOC);
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                                (IX_I2C_DISABLE << IX_I2C_SEND_STOP_LOC);
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                                (IX_I2C_ACK << IX_I2C_ACK_NACK_CTL_LOC);
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                                (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                                (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
                IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
            } /* end of more data bytes to transmit (not last data byte) */
            ixI2cMasterBufTracker.offset++;
        } /* end of more data to transmit */
    } /* end of IX_I2C_MASTER_WRITE_OPERATION */

    else if(IX_I2C_MASTER_READ_OPERATION == ixI2cOpMode)
    {
        /* Check if one data byte to read only */
        if(1 == ixI2cMasterBufTracker.bufSize)
        {
            /* If IX_I2C_NORMAL then send a stop to free the bus */
            if(IX_I2C_NORMAL == ixI2cMasterBufTracker.XferMode)
            {
                /* Send a stop to free the I2C bus */
                ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                                (IX_I2C_ENABLE << IX_I2C_SEND_STOP_LOC);
            } /* end of IX_I2C_NORMAL */

            /* Enable the Transfer byte bit to receive a data byte */
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_SEND_START_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                            (IX_I2C_NACK << IX_I2C_ACK_NACK_CTL_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                            (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
            IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
        } /* end of one data byte to read only */
        else /* there's more than one data byte to read */
        {
            /* Enable the Transfer byte bit to receive a data byte */
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_STOP_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_SEND_STOP_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_SEND_START_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_SEND_START_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_ACK_NACK_CTL_MASK)) |
                            (IX_I2C_ACK << IX_I2C_ACK_NACK_CTL_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                            (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
            ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_MASTER_ABORT_MASK)) |
                            (IX_I2C_DISABLE << IX_I2C_MASTER_ABORT_LOC);
            IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
        }/* end of there's more than one data byte to read */
    } /* end of IX_I2C_MASTER_READ_OPERATION */

    else if(IX_I2C_SLAVE_WRITE_OPERATION == ixI2cOpMode)
    {
        /* Check if buffer is NULL */
        IX_I2C_CHECK_SLAVE_OR_GEN_BUFFER_NULL(ixI2cSlaveWrCallbackP,
                                    "ixI2cSlaveWrCallbackP",
                                    "ixI2cDrvIDBRTxEmptyHdlr",);

        ixI2cStsStored = IX_OSAL_READ_LONG (ixI2cSRAddr);

        /* Check the AckNackStatus to determine a stop is expected
            next. ixI2cStsStored is already read on interrupt */
        if(IX_I2C_ACK ==
            ((ixI2cStsStored & IX_I2C_ACK_NACK_STATUS_MASK) >>
            IX_I2C_ACK_NACK_STATUS_LOC))
        {
            IX_I2C_CHECK_SLAVE_BUFFER_EMPTY("ixI2cDrvIDBRTxEmptyHdlr",);
        
            IX_OSAL_WRITE_LONG(ixI2cDBRAddr,
                ixI2cSlaveOrGenBufTracker.bufP[ixI2cSlaveOrGenBufTracker.offset++]);
        } /* end of IX_I2C_ACK */
        /* Enable the Transfer byte bit to receive a data byte */
        ixI2cCfgStored = (ixI2cCfgStored & (~IX_I2C_TRANSFER_BYTE_MASK)) |
                        (IX_I2C_ENABLE << IX_I2C_TRANSFER_BYTE_LOC);
        IX_OSAL_WRITE_LONG(ixI2cCRAddr, ixI2cCfgStored);
    } /* end of IX_I2C_SLAVE_WRITE_MODE */
    else /* It shouldn't reach here. Log an error */
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixI2cDrvIDBRTxEmptyHdlr:"
            " occured during a read operation!\n", 0,0,0,0,0,0);
    } /* end of code that should not be reached */

    /* Clear the IDBR Transmit Empty detected bit by writing one to the bit
        location */
    ixI2cStsStored =
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_IDBR_TX_EMPTY_LOC);
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);
    return;
} /* end of ixI2cDrvIDBRTxEmptyHdlr */


PRIVATE void ixI2cDrvArbLossDetectedHdlr (void)
{
    /* Set the flag to indicate an arbitration loss */
    ixI2cIntrXferStatus = IX_I2C_INTR_ARB_LOSS;

    /* Check if current Operation is a master read  or a master write */
    if(IX_I2C_MASTER_READ_OPERATION == ixI2cOpMode)
    {
        /* Call the Master Read callback (async) indicating arb loss if it
            is not NULL else ixI2cIntrXferStatus flag is used */
        if(NULL != ixI2cMasterRdCallbackP)
        {
            ixI2cMasterRdCallbackP(IX_I2C_MASTER_ARB_LOSS,
                        ixI2cMasterBufTracker.XferMode,
                        ixI2cMasterBufTracker.bufP,
                        ixI2cMasterBufTracker.bufSize);
        } /* end of NULL != ixI2cMasterRdCallbackP */
    } /*end of IX_I2C_MASTER_READ_OPERATION */
    else /* IX_I2C_MASTER_WRITE_OPERATION */
    {
        /* Call the Master Write callback (async) indicating arb loss if it
            is not NULL else ixI2cIntrXferStatus flag is used */
        if(NULL != ixI2cMasterWrCallbackP)
        {
            ixI2cMasterWrCallbackP(IX_I2C_MASTER_ARB_LOSS,
                        ixI2cMasterBufTracker.XferMode,
                        ixI2cMasterBufTracker.bufP,
                        ixI2cMasterBufTracker.bufSize);
        } /* end of NULL != ixI2cMasterWrCallbackP */
    } /* end of IX_I2C_MASTER_WRITE_OPERATION */

    /* Increment the stats for arbitration loss occurence */
    ixI2cStatsCounters.ixI2cArbLossCounter++;

    /* Clear the arbitration loss detected bit by writing one to the bit
        location */
    ixI2cStsStored =
        (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_ARB_LOSS_DETECTED_LOC);
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

    return;
} /* end of ixI2cDrvArbLossDetectedHdlr */


PRIVATE void ixI2cDrvSlaveStopDetectedHdlr (void)
{
    if(IX_I2C_SLAVE_READ_OPERATION == ixI2cOpMode)
    {
        /* Check if an error occured during data transfer */
        if(IX_I2C_INTR_XFER_ERROR == ixI2cIntrXferStatus)
        {
            /* Call the slave read callback indicating a transfer error, while
                passing the buffer pointer, the buffer size, and the buffer
                filled */
            ixI2cSlaveRdCallbackP(IX_I2C_SLAVE_OR_GEN_READ_ERROR,
                ixI2cSlaveOrGenBufTracker.bufP,
                ixI2cSlaveOrGenBufTracker.bufSize,
                ixI2cSlaveOrGenBufTracker.offset);

            /* increment the stats for number of bytes slave has
                failed to transmit */
            ixI2cStatsCounters.ixI2cSlaveFailedRcvCounter++;
        } /* end of slave read IX_I2C_TRANSFER_ERROR */
        else /* read completed successfully */
        {
            /* Call the slave read callback indicating a read complete, while
                passing the buffer pointer, the buffer size, and the buffer
                filled */
            ixI2cSlaveRdCallbackP(IX_I2C_SUCCESS,
                ixI2cSlaveOrGenBufTracker.bufP,
                ixI2cSlaveOrGenBufTracker.bufSize,
                ixI2cSlaveOrGenBufTracker.offset);

            /* increment the stats for number of bytes slave has received */
            ixI2cStatsCounters.ixI2cSlaveRcvCounter +=
                ixI2cSlaveOrGenBufTracker.offset;
        } /* end of slave read IX_I2C_TRANSFER_SUCCESSFUL */
    } /* end of IX_I2C_SLAVE_READ_OPERATION */
    else if(IX_I2C_SLAVE_WRITE_OPERATION == ixI2cOpMode)
    {
        /* Call the slave write callback indicating a write complete,
            while passing the buffer pointer, the buffer size, and
            the buffer transmitted */
        ixI2cSlaveWrCallbackP(IX_I2C_SUCCESS,
            ixI2cSlaveOrGenBufTracker.bufP,
            ixI2cSlaveOrGenBufTracker.bufSize,
            ixI2cSlaveOrGenBufTracker.offset);

        /* increment the stats for number of bytes slave has transmitted */
        ixI2cStatsCounters.ixI2cSlaveXmitCounter +=
            ixI2cSlaveOrGenBufTracker.offset;
    } /* end of IX_I2C_SLAVE_WRITE_OPERATION */
    else /* IX_I2C_GENERAL_CALL_OPERATION */
    {
        /* Check if an error occured during data transfer */
        if(IX_I2C_INTR_XFER_ERROR == ixI2cIntrXferStatus)
        {
            /* Call the general call callback indicating a transfer error,
                while passing the buffer pointer, the buffer size, and
                the buffer filled */
            ixI2cGenCallCallbackP(IX_I2C_SLAVE_OR_GEN_READ_ERROR,
                ixI2cSlaveOrGenBufTracker.bufP,
                ixI2cSlaveOrGenBufTracker.bufSize,
                ixI2cSlaveOrGenBufTracker.offset);
            /* increment the stats for number of bytes general call has
                failed to receive */
            ixI2cStatsCounters.ixI2cGenAddrCallFailedCounter++;
        } /* end of general call IX_I2C_TRANSFER_ERROR */
        else /* general call completed successfully */
        {
            /* Call the general call callback indicating a read complete,
                while passing the buffer pointer, the buffer size, and
                the buffer filled */
            ixI2cGenCallCallbackP(IX_I2C_SUCCESS,
                ixI2cSlaveOrGenBufTracker.bufP,
                ixI2cSlaveOrGenBufTracker.bufSize,
                ixI2cSlaveOrGenBufTracker.offset);

            /* increment the stats for number of bytes general call has
                received */
            ixI2cStatsCounters.ixI2cGenAddrCallSucceedCounter +=
                ixI2cSlaveOrGenBufTracker.offset;
        } /* end of general call IX_I2C_TRANSFER_SUCCESSFUL */
    } /* end of IX_I2C_GENERAL_CALL_OPERATION */

    /* Clear the slave stop detected bit by writing one to the bit
        location */
    ixI2cStsStored =
            (IX_I2C_SET_TO_BE_CLEARED << IX_I2C_SLAVE_STOP_DETECTED_LOC);
    IX_OSAL_WRITE_LONG(ixI2cSRAddr, ixI2cStsStored);

    return;
} /* end of ixI2cDrvSlaveStopDetectedHdlr */

PRIVATE void ixI2cDrvSleep(UINT32 delay)
{
	if(IX_I2C_LOOP_DELAY == ixI2cDelayType)
	{
		ixOsalBusySleep(delay);
	}
	else /* IX_I2C_SCHED_DELAY == ixI2cDelayType */
	{
		ixOsalSleep(delay);
	}

	return;
}


#endif /* __ixp46X */

