/**
 * IxSspAcc.c
 *
 * File Version: $Revision: 0.1 $
 * 
 * Description: C file for Synchronous Serial Port (SSP) Access component (IxSspAcc)
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

#if defined (__ixp46X) || defined (__ixp43X)

#include "IxOsal.h"
#include "IxSspAcc.h"
#include "IxFeatureCtrl.h"

/**
 * Local #defines
 */
#define IX_SSP_CR0_OFFSET   0x0     /* SSP Control Register 0 (SSCR0)
                                            offset from IXP400_SSP_BASE */
#define IX_SSP_CR1_OFFSET   0x4     /* SSP Control Register 1 (SSCR1)
                                            offset from IXP400_SSP_BASE */
#define IX_SSP_SR_OFFSET    0x8     /* SSP Status Register (SSSR)
                                            offset from IXP400_SSP_BASE */
#define IX_SSP_DR_OFFSET    0x10    /* SSP Data Register (SSDR)
                                            offset from IXP400_SSP_BASE */

#define IX_SSP_FIFO_EMPTY   0x00        /* SSP FIFO empty value is
                                                zero */
#define IX_SSP_FIFO_FULL    0x10        /* SSP FIFO full value is 16 */

#define IX_SSP_FIFO_FULL_OR_EMPTY   0x00/* FIFO level indicates it can
                                                either be empty of full */
#define IX_SSP_TX_FIFO_EXCEED_THLD  0x0 /* zero indicates a Tx FIFO
                                                exceed threshold for the
                                                Tx FIFO svc request bit of
                                                the SSSR */
#define IX_SSP_RX_FIFO_BELOW_THLD   0x0 /* zero indicates a Rx FIFO
                                                below threshold for the Rx
                                                FIFO svc request bit of the
                                                SSSR */
#define IX_SSP_IS_BUSY              0x1 /* One indicates a SSP busy for
                                                the SSP busy bit of the SSSR */
#define IX_SSP_TX_FIFO_FULL         0x0 /* zero indicates a TX FIFO Full
                                                in the Tx FIFO not full bit
                                                of the SSSR */
#define IX_SSP_RX_FIFO_EMPTY        0x0 /* zero indicates a RX FIFO Empty
                                                in the Rx FIFO not empty bit
                                                of the SSSR */
#define IX_SSP_OVERRUN_HAS_OCCURRED 0x1 /* one indicates an overrun has
                                                occurred in the overrun bit
                                                of the SSSR */
#define IX_SSP_INTERRUPT_ENABLE     0x1 /* one enables the interrupt in
                                                the SSCR1 */
#define IX_SSP_INTERRUPT_DISABLE    0x0 /* one disables the interrupt in
                                                the SSCR1 */
#define IX_SSP_INTERRUPTED          0x1 /* one indicates an interrupt has
                                                occured in the SSSR */

#define IX_SSP_SET_TO_BE_CLEARED     0x1 /* Use for write 1 to clear in
                                                the registers */

/* #defines for mask and location of SSP Control and Status Registers contents */
#define IX_SSP_SERIAL_CLK_RATE_LOC      0x8
#define IX_SSP_SERIAL_CLK_RATE_MASK     (0xFF << IX_SSP_SERIAL_CLK_RATE_LOC)
#define IX_SSP_PORT_STATUS_LOC          0x7
#define IX_SSP_PORT_STATUS_MASK         (0x1 << IX_SSP_PORT_STATUS_LOC)
#define IX_SSP_CLK_SRC_LOC              0x6
#define IX_SSP_CLK_SRC_MASK             (0x1 << IX_SSP_CLK_SRC_LOC)
#define IX_SSP_FRAME_FORMAT_LOC         0x4
#define IX_SSP_FRAME_FORMAT_MASK        (0x3 << IX_SSP_FRAME_FORMAT_LOC)
#define IX_SSP_DATA_SIZE_LOC            0x0
#define IX_SSP_DATA_SIZE_MASK           (0xF << IX_SSP_DATA_SIZE_LOC)
#define IX_SSP_RX_FIFO_THLD_LOC         0xA
#define IX_SSP_RX_FIFO_THLD_MASK        (0xF << IX_SSP_RX_FIFO_THLD_LOC)
#define IX_SSP_TX_FIFO_THLD_LOC         0x6
#define IX_SSP_TX_FIFO_THLD_MASK        (0xF << IX_SSP_TX_FIFO_THLD_LOC)
#define IX_SSP_MICROWIRE_CTL_WORD_LOC   0x5
#define IX_SSP_MICROWIRE_CTL_WORD_MASK  (0x1 << IX_SSP_MICROWIRE_CTL_WORD_LOC)
#define IX_SSP_SPI_SCLK_PHASE_LOC       0x4
#define IX_SSP_SPI_SCLK_PHASE_MASK      (0x1 << IX_SSP_SPI_SCLK_PHASE_LOC)
#define IX_SSP_SPI_SCLK_POLARITY_LOC    0x3
#define IX_SSP_SPI_SCLK_POLARITY_MASK   (0x1 << IX_SSP_SPI_SCLK_POLARITY_LOC)
#define IX_SSP_LOOPBACK_ENABLE_LOC      0x2
#define IX_SSP_LOOPBACK_ENABLE_MASK     (0x1 << IX_SSP_LOOPBACK_ENABLE_LOC)
#define IX_SSP_TX_FIFO_INT_ENABLE_LOC   0x1
#define IX_SSP_TX_FIFO_INT_ENABLE_MASK  (0x1 << IX_SSP_TX_FIFO_INT_ENABLE_LOC)
#define IX_SSP_RX_FIFO_INT_ENABLE_LOC   0x0
#define IX_SSP_RX_FIFO_INT_ENABLE_MASK  (0x1 << IX_SSP_RX_FIFO_INT_ENABLE_LOC)
#define IX_SSP_RX_FIFO_LVL_LOC          0xC
#define IX_SSP_RX_FIFO_LVL_MASK         (0xF << IX_SSP_RX_FIFO_LVL_LOC)
#define IX_SSP_TX_FIFO_LVL_LOC          0x8
#define IX_SSP_TX_FIFO_LVL_MASK         (0xF << IX_SSP_TX_FIFO_LVL_LOC)
#define IX_SSP_RX_FIFO_OVERRUN_LOC      0x7
#define IX_SSP_RX_FIFO_OVERRUN_MASK     (0x1 << IX_SSP_RX_FIFO_OVERRUN_LOC)
#define IX_SSP_RX_FIFO_SVC_REQ_LOC      0x6
#define IX_SSP_RX_FIFO_SVC_REQ_MASK     (0x1 << IX_SSP_RX_FIFO_SVC_REQ_LOC)
#define IX_SSP_TX_FIFO_SVC_REQ_LOC      0x5
#define IX_SSP_TX_FIFO_SVC_REQ_MASK     (0x1 << IX_SSP_TX_FIFO_SVC_REQ_LOC)
#define IX_SSP_BUSY_LOC                 0x4
#define IX_SSP_BUSY_MASK                (0x1 << IX_SSP_BUSY_LOC)
#define IX_SSP_RX_FIFO_NOT_EMPTY_LOC    0x3
#define IX_SSP_RX_FIFO_NOT_EMPTY_MASK   (0x1 << IX_SSP_RX_FIFO_NOT_EMPTY_LOC)
#define IX_SSP_TX_FIFO_NOT_FULL_LOC     0x2
#define IX_SSP_TX_FIFO_NOT_FULL_MASK    (0x1 << IX_SSP_TX_FIFO_NOT_FULL_LOC)


/**
 * macros
 */
#define IX_SSP_INIT_SUCCESS_CHECK(funcName, returnType)                 \
    if(FALSE == ixSspAccInitComplete)                                   \
    {                                                                   \
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,       \
            funcName": SSP Access not initialized\n",                   \
            0,0,0,0,0,0);                                               \
        return returnType;                                              \
    } /* end of FALSE == ixSspAccInitComplete */


/**
 * typedef
 */

/* typedef to contain both SSP Control Register 0 and 1 */
typedef struct
{
    UINT32   sscr0;
    UINT32   sscr1;
} IxSspAccConfig;

/**
 * Static variables defined here
 */

/* Interrupt handler function pointers */
static RxFIFOOverrunHandler ixRxFIFOOverrunHdlr = NULL;
static RxFIFOThresholdHandler ixRxFIFOThsldHdlr = NULL;
static TxFIFOThresholdHandler ixTxFIFOThsldHdlr = NULL;

/* The addresses to be used to access the SSP Control Register 0 (CR0),
    SSP Control Register 1 (CR1), SSP Status Register (SR), and SSP
    Data Register (DR). The address is assigned on init. */
static UINT32 ixSspCR0Addr = 0;
static UINT32 ixSspCR1Addr = 0;
static UINT32 ixSspSRAddr = 0;
static UINT32 ixSspDRAddr = 0;

/* Storage for the SSP configuration which is used over many functions to
increase efficiency */
static IxSspAccConfig ixSspAccCfgStored;

/* Storage for the SSP status which is used by many functions to avoid
    declaration of the same struct multiple times */
static UINT32 ixSspAccStsStored;

/* Storage for the SSP statistics counters */
static IxSspAccStatsCounters ixSspAccStatsCounters;

/* Flag to indicate if the mode is interrupt or poll. */
static BOOL ixSspAccInterruptMode = FALSE;

/* Flag to indicate if the init has been done and thus not performing some
    instructions that should not be done more than once (please refer to the init
    API. Example: memory mapping). if init is called more than once (which is
    allowed) */
static BOOL ixSspAccInitComplete = FALSE;

/**
 * static function declaration
 */
PRIVATE void ixSspAccInterruptDetected (void);

/**
 * Function definitions
 */
PUBLIC IX_SSP_STATUS
ixSspAccInit (
    IxSspInitVars *initVarsSelected)
{
    IX_SSP_STATUS temp_return = 0;
    
    /* Check if the hardware supports SSP. By reading the device type, it can be
        determined if the hardware supports SSP. Currenlty only the IXP46X and
        IXP43X support SSP. */
    if((IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X != ixFeatureCtrlDeviceRead()) &&
       (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X != ixFeatureCtrlDeviceRead()))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixSspAccInit: This hardware does not support SSP!\n",
            0,0,0,0,0,0);
        return IX_SSP_NOT_SUPORTED;
    } /* end of device detected is not IXP46X or IXP43X */

    /* Check if the initVarsSelected is NULL */
    if(NULL == initVarsSelected)
        return IX_SSP_NULL_POINTER;

    /* Check if SSP Init has been called before to avoid multiple instances of
        memory mapping */
    if(FALSE == ixSspAccInitComplete)
    {
        /* Memory map the control, status, and data registers of the SSP */
        ixSspCR0Addr = (UINT32)IX_OSAL_MEM_MAP (IX_OSAL_IXP400_SSP_PHYS_BASE,
                        IX_OSAL_IXP400_SSP_MAP_SIZE);
        ixSspCR1Addr = ixSspCR0Addr + IX_SSP_CR1_OFFSET;
        ixSspSRAddr = ixSspCR0Addr + IX_SSP_SR_OFFSET;
        ixSspDRAddr = ixSspCR0Addr + IX_SSP_DR_OFFSET;

        ixSspAccStatsReset(); /* Clear the SSP statistics counters */
        ixSspAccInitComplete = TRUE; /* Set the Init Complete flag so that a
                                    call to init will not mem map and clear
                                    the stats again*/
    } /* end of FALSE == ixSspAccInitComplete */

    /* Set the SSP frame format (SPI, SSP, or Microwire) if format is valid */
    if(IX_SSP_SUCCESS != ixSspAccFrameFormatSelect(
                        initVarsSelected->FrameFormatSelected))
        return IX_SSP_INVALID_FRAME_FORMAT_ENUM_VALUE;

    /* Set the data size if range is valid and FIFOs empty */
    temp_return = ixSspAccDataSizeSelect(initVarsSelected->DataSizeSelected);
    if(IX_SSP_SUCCESS != temp_return)
    {
        return temp_return;
    }

    /* Set the clock source if source is valid */
    if(IX_SSP_SUCCESS != ixSspAccClockSourceSelect(
                        initVarsSelected->ClkSourceSelected))
        return IX_SSP_INVALID_CLOCK_SOURCE_ENUM_VALUE;

    /* Set the Tx FIFO Threshold if level is valid */
    if(IX_SSP_SUCCESS != ixSspAccTxFIFOThresholdSet(
                        initVarsSelected->TxFIFOThresholdSelected))
        return IX_SSP_INVALID_TX_FIFO_THRESHOLD_ENUM_VALUE;

    /* Set the Rx FIFO Threshold if level is valid */
    if(IX_SSP_SUCCESS != ixSspAccRxFIFOThresholdSet(
                        initVarsSelected->RxFIFOThresholdSelected))
        return IX_SSP_INVALID_RX_FIFO_THRESHOLD_ENUM_VALUE;

    /* Unbind the SSP ISR if interrupt mode was enabled previously */
    if(TRUE == ixSspAccInterruptMode)
    {
        if(IX_SUCCESS != ixOsalIrqUnbind(IX_OSAL_IXP400_SSP_IRQ_LVL))
        {
            return IX_SSP_INT_UNBIND_FAIL;
        } /* end of ixOsalIrqUnbind Fail */
        ixSspAccRxFIFOIntDisable();
        ixSspAccTxFIFOIntDisable();
        ixSspAccInterruptMode = FALSE;
    } /* end of ixSspAccInterruptMode == TRUE */

    /* Check if either the Rx FIFO or the Tx FIFO interrupt is selected to be
        enabled, then enable interrupt mode */
    if((TRUE == initVarsSelected->TxFIFOIntrEnable) ||
        (TRUE == initVarsSelected->RxFIFOIntrEnable))
    {
        /* Check if the Rx FIFO Overrun handler is NULL */
        if(NULL == initVarsSelected->RxFIFOOverrunHdlr)
            return IX_SSP_RX_FIFO_OVERRUN_HANDLER_MISSING;

        /* Set the Rx FIFO Overrun handler */
        ixRxFIFOOverrunHdlr = initVarsSelected->RxFIFOOverrunHdlr;

        /* Bind the SSP to the SSP ISR */
        if(IX_SUCCESS != ixOsalIrqBind(IX_OSAL_IXP400_SSP_IRQ_LVL,
                            (IxOsalVoidFnVoidPtr)ixSspAccInterruptDetected,
                            NULL))
        {
            return IX_SSP_INT_BIND_FAIL;
        }
        ixSspAccInterruptMode = TRUE; /* Set the Interrupt Mode flag to TRUE */
    } /* end of interrupt mode selected */
    else /* start of polling mode selected */
    {
        /* Set the Rx FIFO Overrun handler to NULL */
        ixRxFIFOOverrunHdlr = NULL;
        
        ixSspAccInterruptMode = FALSE; /* Set the Interrupt Mode flag to FALSE */
    } /* end of polling mode selected */

    /* Check if the Rx FIFO interrupt is selected to be enabled */
    if(TRUE == initVarsSelected->RxFIFOIntrEnable)
    {
        /* Enable the Rx FIFO and set the Rx FIFO handler if handler pointer is
        not NULL */
        if(IX_SSP_SUCCESS != ixSspAccRxFIFOIntEnable(
                            initVarsSelected->RxFIFOThsldHdlr))
            return IX_SSP_RX_FIFO_HANDLER_MISSING;
    } /* end of Rx FIFOIntrEnable Selected */

    /* Check if the Tx FIFO interrupt is selected to be enabled */
    if(TRUE == initVarsSelected->TxFIFOIntrEnable)
    {   
        /* Enable the Tx FIFO and set the Tx FIFO handler if handler pointer is
            not NULL */
        if(IX_SSP_SUCCESS != ixSspAccTxFIFOIntEnable(
                            initVarsSelected->TxFIFOThsldHdlr))
            return IX_SSP_TX_FIFO_HANDLER_MISSING;
    } /* end of Tx FIFOIntrEnable Selected */
    
    /* Enable/disable the loopback */
    ixSspAccLoopbackEnable(initVarsSelected->LoopbackEnable);

    if(SPI_FORMAT == initVarsSelected->FrameFormatSelected)
    {
        /* Set the SPI SCLK phase if phase selected is valid */
        if(IX_SSP_SUCCESS != ixSspAccSpiSclkPhaseSet(
                            initVarsSelected->SpiSclkPhaseSelected))
        {
            return IX_SSP_INVALID_SPI_PHASE_ENUM_VALUE;
        }
        
        /* Set the SPI SCLK polarity if polarity selected is valid */
        if(IX_SSP_SUCCESS != ixSspAccSpiSclkPolaritySet(
                            initVarsSelected->SpiSclkPolaritySelected))
        {
            return IX_SSP_INVALID_SPI_POLARITY_ENUM_VALUE;
        }
    } /* end of SPI_FORMAT */

    if(MICROWIRE_FORMAT == initVarsSelected->FrameFormatSelected)
    {
        /* Set the Microwire control word size if size is valid and Tx FIFO empty */
        temp_return = ixSspAccMicrowireControlWordSet(
                            initVarsSelected->MicrowireCtlWordSelected);
        if(IX_SSP_SUCCESS != temp_return)
        {
            if(IX_SSP_TX_FIFO_NOT_EMPTY == temp_return)
            {
                return IX_SSP_FIFO_NOT_EMPTY_FOR_SETTING_CTL_CMD;
            }
            else
            {
                return IX_SSP_INVALID_MICROWIRE_CTL_CMD_ENUM_VALUE;
            }
        } /* end of temp_return != IX_SSP_SUCCESS */
    } /* end of MICROWIRE_FORMAT */
    
    /* Set the Serial clock rate to the selected */
    ixSspAccSerialClockRateConfigure(initVarsSelected->SerialClkRateSelected);

    /* Enable the SSP Port to start receiving and transmitting data */
    ixSspAccSSPPortStatusSet(SSP_PORT_ENABLE);

    return IX_SSP_SUCCESS;
} /* endo of ixSspAccInit */

PUBLIC IX_SSP_STATUS
ixSspAccUninit (
    void)
{
    if(TRUE == ixSspAccInitComplete)
    {
        /* Disable the SSP hardware */
        ixSspAccSSPPortStatusSet(SSP_PORT_DISABLE);
        
        /* Unbind the SSP ISR if interrupt mode is enabled */
        if(TRUE == ixSspAccInterruptMode)
        {
            if(IX_SUCCESS != ixOsalIrqUnbind(IX_OSAL_IXP400_SSP_IRQ_LVL))
            {
                return IX_SSP_INT_UNBIND_FAIL;
            } /* end of ixOsalIrqUnbind Fail */
            ixSspAccRxFIFOIntDisable();
            ixSspAccTxFIFOIntDisable();
            /* Set all Handler pointers to NULL */
            ixRxFIFOOverrunHdlr = NULL;
            ixSspAccInterruptMode = FALSE;
        } /* end of TRUE == ixSspAccInterruptMode */
        
        /* Return the memory that was mapped during init which is the SSP control
            and status registers and the SSP data register. */
        IX_OSAL_MEM_UNMAP(ixSspCR0Addr);
        ixSspAccInitComplete = FALSE;
    } /* end of TRUE == ixSspAccInitComplete */

    return IX_SSP_SUCCESS;
} /* end of ixSspAccUninit */

PUBLIC IX_SSP_STATUS
ixSspAccFIFODataSubmit (
    UINT16* data,
    UINT32 amtOfData)
{
    UINT32 dataLoc = 0;

    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccFIFODataSubmit", IX_SSP_NOT_INIT);

    /* Check if the data pointer provided is NULL */
    if(NULL == data)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixSspAccFIFODataSubmit: data pointer is NULL.\n",
            0,0,0,0,0,0);
        return IX_SSP_NULL_POINTER;
    } /* end of NULL == data */

    /* Check if the Tx FIFO has sufficient space to store the number of data
        specified by amtOfData */
    if((ixSspAccTxFIFOLevelGet() + amtOfData) > IX_SSP_FIFO_FULL)
        return IX_SSP_FAIL;

    /* Copy the data from the data buffer pointer into the SSP Data Register */
    while(amtOfData > dataLoc)
        IX_OSAL_WRITE_LONG(ixSspDRAddr, data[dataLoc++]);

    /* Increment the SSP stats counter for data transmitted */
    ixSspAccStatsCounters.ixSspXmitCounter+=amtOfData;
    
    return IX_SSP_SUCCESS;
} /* end of ixSspAccFIFODataSubmit */

PUBLIC IX_SSP_STATUS
ixSspAccFIFODataReceive (
    UINT16* data,
    UINT32 amtOfData)
{
    UINT32 dataLoc = 0;

    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccFIFODataReceive", IX_SSP_NOT_INIT);
    
    /* Check if the data pointer provided is NULL */
    if(NULL == data)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixSspAccFIFODataReceive: data pointer is NULL.\n",
            0,0,0,0,0,0);
        return IX_SSP_NULL_POINTER;
    } /* end of NULL == data */

    /* Check if the Rx FIFO has the number of data specified by amtOfData to be
        retrieved */
    if(ixSspAccRxFIFOLevelGet() < amtOfData)
        return IX_SSP_FAIL;

    /* Overrun Check is called to increment the overrun counter if it occured
        and not used to determine whether an overrun occured therefore no checking
        of the return value is necessary */
    ixSspAccRxFIFOOverrunCheck();

    /* Copy the data from the SSP Data Register into the data buffer pointer */
    while(amtOfData > dataLoc)
        data[dataLoc++] = IX_OSAL_READ_LONG(ixSspDRAddr);

    /* Increment the SSP stats counter for data received */
    ixSspAccStatsCounters.ixSspRcvCounter+=amtOfData;

    return IX_SSP_SUCCESS;
} /* end of ixSspAccFIFODataReceive */

PUBLIC IX_SSP_STATUS
ixSspAccTxFIFOHitOrBelowThresholdCheck (
    void)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccTxFIFOHitOrBelowThresholdCheck",
                                IX_SSP_NOT_INIT);

    /* Read the SSP status register, SSSR */
    ixSspAccStsStored = IX_OSAL_READ_LONG (ixSspSRAddr);

    /* Check the Tx FIFO Service Request bit has been set to determine if the
        threshold has been hit or is below */
    if(IX_SSP_TX_FIFO_EXCEED_THLD ==
        ((ixSspAccStsStored & IX_SSP_TX_FIFO_SVC_REQ_MASK) >>
        IX_SSP_TX_FIFO_SVC_REQ_LOC))
    {
        return IX_SSP_TX_FIFO_EXCEED_THRESHOLD;
    }
    else
    {
        return IX_SSP_TX_FIFO_HIT_BELOW_THRESHOLD;
    }
} /* end of ixSspAccTxFIFOHitOrBelowThresholdCheck */

PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOHitOrAboveThresholdCheck (
    void)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccRxFIFOHitOrAboveThresholdCheck",
                                    IX_SSP_NOT_INIT);

    /* Read the SSP status register, SSSR */
    ixSspAccStsStored = IX_OSAL_READ_LONG (ixSspSRAddr);
                                                    
    /* Check the Rx FIFO Service Request bit has been set to determine if the
    threshold has been hit or is above */
    if(IX_SSP_RX_FIFO_BELOW_THLD ==
        ((ixSspAccStsStored & IX_SSP_RX_FIFO_SVC_REQ_MASK) >>
        IX_SSP_RX_FIFO_SVC_REQ_LOC))
    {
        return IX_SSP_RX_FIFO_BELOW_THRESHOLD;
    }
    else
    {
        return IX_SSP_RX_FIFO_HIT_ABOVE_THRESHOLD;
    }
} /* end of ixSspAccRxFIFOHitOrAboveThresholdCheck */

/**
 * Configuration functions
 */

PUBLIC IX_SSP_STATUS
ixSspAccSSPPortStatusSet (
    IxSspAccPortStatus portStatusSelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccSSPPortStatusSet", IX_SSP_NOT_INIT);

    /* Check for validity of parameter */
    if(portStatusSelected >= INVALID_SSP_PORT_STATUS)
        return IX_SSP_FAIL;

    /* Write the parameter into the SSP Port Enable of SSCR0 register if selected
        differ from current status */
    if(((ixSspAccCfgStored.sscr0 & IX_SSP_PORT_STATUS_MASK) >>
         IX_SSP_PORT_STATUS_LOC) != portStatusSelected)
    {
        ixSspAccCfgStored.sscr0 =
            (ixSspAccCfgStored.sscr0 & (~IX_SSP_PORT_STATUS_MASK)) |
            (portStatusSelected << IX_SSP_PORT_STATUS_LOC);
        IX_OSAL_WRITE_LONG (ixSspCR0Addr, ixSspAccCfgStored.sscr0);
    } /* end of parameter write when current differs from selected */

    return IX_SSP_SUCCESS;
} /* end of ixSspAccSSPPortStatusSet */

PUBLIC IX_SSP_STATUS
ixSspAccFrameFormatSelect (
    IxSspAccFrameFormat frameFormatSelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccFrameFormatSelect", IX_SSP_NOT_INIT);

    /* Check for validity of parameter */
    if(frameFormatSelected >= INVALID_FORMAT)
        return IX_SSP_INVALID_FRAME_FORMAT_ENUM_VALUE;

    /* Determine if the SSP port is enabled. */
    if (SSP_PORT_ENABLE ==
        ((ixSspAccCfgStored.sscr0 & IX_SSP_PORT_STATUS_MASK) >>
        IX_SSP_PORT_STATUS_LOC))
    {
        /* SSP Port enabled.
            Disable the SSP Port (clears FIFOs), then write the parameter
            into the Frame Format bit of SSCR0 register and re-enable the SSP
            Port. */
        ixSspAccSSPPortStatusSet(SSP_PORT_DISABLE);
        ixSspAccCfgStored.sscr0 =
            (ixSspAccCfgStored.sscr0 & (~IX_SSP_FRAME_FORMAT_MASK)) |
            (frameFormatSelected << IX_SSP_FRAME_FORMAT_LOC);
        ixSspAccSSPPortStatusSet(SSP_PORT_ENABLE); /* Both the format and the
                                                   status will be written into
                                                   the SSCR0 register together.
                                                    */
    } /* end of SSP_PORT_ENABLE */
    else /* start of SSP_PORT_DISABLE */
    {
        /* SSP Port not enabled.
        Write the parameter into the Frame Format bit of SSCR0 register */
        ixSspAccCfgStored.sscr0 =
            (ixSspAccCfgStored.sscr0 & (~IX_SSP_FRAME_FORMAT_MASK)) |
            (frameFormatSelected << IX_SSP_FRAME_FORMAT_LOC);
        IX_OSAL_WRITE_LONG (ixSspCR0Addr, ixSspAccCfgStored.sscr0);
    } /* end of SSP_PORT_DISABLE */

    return IX_SSP_SUCCESS;
} /* end of ixSspAccFrameFormatSelect */

PUBLIC IX_SSP_STATUS
ixSspAccDataSizeSelect (
    IxSspAccDataSize dataSizeSelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccDataSizeSelect", IX_SSP_NOT_INIT);

    /* Check for validity of parameter */
    if( (dataSizeSelected <= DATA_SIZE_TOO_SMALL) ||
        (dataSizeSelected >= DATA_SIZE_TOO_BIG) )
        return IX_SSP_INVALID_DATA_SIZE_ENUM_VALUE;

    /* Only allow change if Tx FIFO is empty */
    if(IX_SSP_FIFO_EMPTY != ixSspAccTxFIFOLevelGet())
        return IX_SSP_TX_FIFO_NOT_EMPTY;

    /* Only allow change if Rx FIFO is empty */
    if(IX_SSP_FIFO_EMPTY != ixSspAccRxFIFOLevelGet())
        return IX_SSP_RX_FIFO_NOT_EMPTY;

    /* Determine if the SSP port is enabled. */
    if (SSP_PORT_ENABLE ==
        ((ixSspAccCfgStored.sscr0 & IX_SSP_PORT_STATUS_MASK) >>
        IX_SSP_PORT_STATUS_LOC))
    {
        /* SSP Port enabled.
            Disable the SSP Port (clears FIFOs), then write the parameter
            into the data size select bit of SSCR0 register and re-enable the
            SSP Port. */
        ixSspAccSSPPortStatusSet(SSP_PORT_DISABLE);
        ixSspAccCfgStored.sscr0 =
            (ixSspAccCfgStored.sscr0 & (~IX_SSP_DATA_SIZE_MASK)) |
            (dataSizeSelected << IX_SSP_DATA_SIZE_LOC);
        ixSspAccSSPPortStatusSet(SSP_PORT_ENABLE); /* Both the data size and the
                                                   status will be written into
                                                   the SSCR0 register together.
                                                    */
    } /* end of SSP_PORT_ENABLE */
    else /* start of SSP_PORT_DISABLE */
    {
        /* SSP Port not enabled.
            Write the parameter into the data size select bit of SSCR0 register */
        ixSspAccCfgStored.sscr0 =
            (ixSspAccCfgStored.sscr0 & (~IX_SSP_DATA_SIZE_MASK)) |
            (dataSizeSelected << IX_SSP_DATA_SIZE_LOC);
        IX_OSAL_WRITE_LONG (ixSspCR0Addr, ixSspAccCfgStored.sscr0);
    } /* end of SSP_PORT_DISABLE */
    
    return IX_SSP_SUCCESS;
} /* end of ixSspAccDataSizeSelect */

PUBLIC IX_SSP_STATUS
ixSspAccClockSourceSelect (
    IxSspAccClkSource clkSourceSelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccClockSourceSelect", IX_SSP_NOT_INIT);

    /* Check for validity of parameter */
    if(clkSourceSelected >= INVALID_CLK_SOURCE)
        return IX_SSP_INVALID_CLOCK_SOURCE_ENUM_VALUE;

    /* Write the parameter into the Clock Source bit of SSCR0 register */
    ixSspAccCfgStored.sscr0 = (ixSspAccCfgStored.sscr0 & (~IX_SSP_CLK_SRC_MASK))
                                | (clkSourceSelected << IX_SSP_CLK_SRC_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR0Addr, ixSspAccCfgStored.sscr0);
    
    return IX_SSP_SUCCESS;
} /* end of ixSspAccClockSourceSelect */

PUBLIC IX_SSP_STATUS
ixSspAccSerialClockRateConfigure (
    UINT8 serialClockRateSelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccSerialClockRateConfigure", IX_SSP_NOT_INIT);

    /* Write the parameter into the Clock Rate bits of SSCR0 register */
    ixSspAccCfgStored.sscr0 =
        (ixSspAccCfgStored.sscr0 & (~IX_SSP_SERIAL_CLK_RATE_MASK)) |
        (serialClockRateSelected << IX_SSP_SERIAL_CLK_RATE_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR0Addr, ixSspAccCfgStored.sscr0);
    return IX_SSP_SUCCESS;
} /* end of ixSspAccSerialClockRateConfigure */

PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOIntEnable (
    RxFIFOThresholdHandler rxFIFOIntrHandler)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccRxFIFOIntEnable", IX_SSP_NOT_INIT);

    /* Only allow to enable the interrupt if interrupt mode is set at init */
    if(FALSE == ixSspAccInterruptMode)
        return IX_SSP_POLL_MODE_BLOCKING;

    /* Check if a handler is provided */
    if(NULL == rxFIFOIntrHandler)
        return IX_SSP_RX_FIFO_HANDLER_MISSING;

    /* Set the Rx FIFO threshold interrupt handler function pointer to point to
    the function pointer parameter and enable the Rx FIFO interrupt by writing
    a one to the Rx FIFO Interrupt enable bit ofthe SSCR1 register */
    ixRxFIFOThsldHdlr = rxFIFOIntrHandler;
    ixSspAccCfgStored.sscr1 =
        (ixSspAccCfgStored.sscr1 & (~IX_SSP_RX_FIFO_INT_ENABLE_MASK)) |
        (IX_SSP_INTERRUPT_ENABLE << IX_SSP_RX_FIFO_INT_ENABLE_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1);
    
    return IX_SSP_SUCCESS;
} /* end of ixSspAccRxFIFOIntEnable */

PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOIntDisable (
    void)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccRxFIFOIntDisable", IX_SSP_NOT_INIT);

    if(TRUE == ixSspAccInterruptMode)
    {
        /* Disable the Rx FIFO interrupt by writing zero to the Rx FIFO
            Interrupt enable bit ofthe SSCR1 register and setting the Tx
            FIFO threshold interrupt handler function pointer to NULL.*/
        ixSspAccCfgStored.sscr1 =
            (ixSspAccCfgStored.sscr1 & (~IX_SSP_RX_FIFO_INT_ENABLE_MASK)) |
            (IX_SSP_INTERRUPT_DISABLE << IX_SSP_RX_FIFO_INT_ENABLE_LOC);
        IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1);
        ixRxFIFOThsldHdlr = NULL;
    } /* end of TRUE == ixSspAccInterruptMode */

    return IX_SSP_SUCCESS;
} /* end of ixSspAccRxFIFOIntDisable */

PUBLIC IX_SSP_STATUS
ixSspAccTxFIFOIntEnable (
    TxFIFOThresholdHandler txFIFOIntrHandler)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccTxFIFOIntEnable", IX_SSP_NOT_INIT);

    /* Only allow to enable the interrupt if interrupt mode is set at init */
    if(FALSE == ixSspAccInterruptMode)
        return IX_SSP_POLL_MODE_BLOCKING;

    /* Check if a handler is provided */
    if(NULL == txFIFOIntrHandler)
        return IX_SSP_TX_FIFO_HANDLER_MISSING;

    /* Set the Tx FIFO threshold interrupt handler function pointer to point to
        the function pointer parameter and enable the Tx FIFO interrupt by
        writing a one to the Rx FIFO Interrupt enable bit ofthe SSCR1
        register */
    ixTxFIFOThsldHdlr = txFIFOIntrHandler;
    ixSspAccCfgStored.sscr1 =
        (ixSspAccCfgStored.sscr1 & (~IX_SSP_TX_FIFO_INT_ENABLE_MASK)) |
        (IX_SSP_INTERRUPT_ENABLE << IX_SSP_TX_FIFO_INT_ENABLE_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1); 

    return IX_SSP_SUCCESS;
} /* end of ixSspAccTxFIFOIntEnable */

PUBLIC IX_SSP_STATUS
ixSspAccTxFIFOIntDisable (
    void)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccTxFIFOIntDisable", IX_SSP_NOT_INIT);

    if(TRUE == ixSspAccInterruptMode)
    {
        /* Disable the Tx FIFO interrupt by writing zero to the Rx FIFO
            Interrupt enable bit ofthe SSCR1 register and setting the Tx
            FIFO threshold interrupt    handler function pointer to NULL. */
        ixSspAccCfgStored.sscr1 =
            (ixSspAccCfgStored.sscr1 & (~IX_SSP_TX_FIFO_INT_ENABLE_MASK)) |
            (IX_SSP_INTERRUPT_DISABLE << IX_SSP_TX_FIFO_INT_ENABLE_LOC);
        IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1);
        ixTxFIFOThsldHdlr = NULL;
    } /* end of TRUE == ixSspAccInterruptMode */

    return IX_SSP_SUCCESS;
} /* end of ixSspAccTxFIFOIntDisable */

PUBLIC IX_SSP_STATUS
ixSspAccLoopbackEnable (
    BOOL loopbackEnable)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccLoopbackEnable", IX_SSP_NOT_INIT);

    /* Write the parameter into the loopback enable bit of SSCR1 register */
    ixSspAccCfgStored.sscr1 =
        (ixSspAccCfgStored.sscr1 & (~IX_SSP_LOOPBACK_ENABLE_MASK)) |
        (loopbackEnable << IX_SSP_LOOPBACK_ENABLE_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1);
    
    return IX_SSP_SUCCESS;
} /* end of ixSspAccLoopbackEnable */

PUBLIC IX_SSP_STATUS
ixSspAccSpiSclkPolaritySet (
    IxSspAccSpiSclkPolarity spiSclkPolaritySelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccSpiSclkPolaritySet", IX_SSP_NOT_INIT);

    /* Check for validity of parameter */
    if(spiSclkPolaritySelected >= INVALID_SPI_POLARITY)
        return IX_SSP_INVALID_SPI_POLARITY_ENUM_VALUE;

    /* Write the parameter into the SPI SCLK Polarity bit of SSCR1 register */
    ixSspAccCfgStored.sscr1 =
        (ixSspAccCfgStored.sscr1 & (~IX_SSP_SPI_SCLK_POLARITY_MASK)) |
        (spiSclkPolaritySelected << IX_SSP_SPI_SCLK_POLARITY_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1);

    return IX_SSP_SUCCESS;
} /* end of ixSspAccSpiSclkPolaritySet */

PUBLIC IX_SSP_STATUS
ixSspAccSpiSclkPhaseSet (
    IxSspAccSpiSclkPhase spiSclkPhaseSelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccSpiSclkPhaseSet", IX_SSP_NOT_INIT);

    /* Check for validity of parameter */
    if(spiSclkPhaseSelected >= INVALID_SPI_PHASE)
        return IX_SSP_INVALID_SPI_PHASE_ENUM_VALUE;

    /* Write the parameter into the SPI SCLK Phase bit of SSCR1 register */
    ixSspAccCfgStored.sscr1 =
        (ixSspAccCfgStored.sscr1 & (~IX_SSP_SPI_SCLK_PHASE_MASK)) |
        (spiSclkPhaseSelected << IX_SSP_SPI_SCLK_PHASE_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1);

    return IX_SSP_SUCCESS;
} /* end of ixSspAccSpiSclkPhaseSet */

PUBLIC IX_SSP_STATUS
ixSspAccMicrowireControlWordSet (
    IxSspAccMicrowireCtlWord microwireCtlWordSelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccMicrowireControlWordSet", IX_SSP_NOT_INIT);

    /* Check for validity of parameter */
    if(microwireCtlWordSelected >= INVALID_MICROWIRE_CTL_WORD)
        return IX_SSP_INVALID_MICROWIRE_CTL_CMD_ENUM_VALUE;

    /* Only allow change if Tx FIFO is empty */
    if(IX_SSP_FIFO_EMPTY != ixSspAccTxFIFOLevelGet())
        return IX_SSP_TX_FIFO_NOT_EMPTY;

    /* Write the parameter into the Microwire Data Size bit of SSCR1 register*/
    ixSspAccCfgStored.sscr1 =
        (ixSspAccCfgStored.sscr1 & (~IX_SSP_MICROWIRE_CTL_WORD_MASK)) |
        (microwireCtlWordSelected << IX_SSP_MICROWIRE_CTL_WORD_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1);

    return IX_SSP_SUCCESS;
} /* end of ixSspAccMicrowireControlWordSet */

PUBLIC IX_SSP_STATUS
ixSspAccTxFIFOThresholdSet (
    IxSspAccFifoThreshold txFIFOThresholdSelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccTxFIFOThresholdSet", IX_SSP_NOT_INIT);

    /* Check for validity of parameter */
    if(txFIFOThresholdSelected >= INVALID_FIFO_TSHLD)
        return IX_SSP_INVALID_TX_FIFO_THRESHOLD_ENUM_VALUE;

    /* Write the parameter into the Tx FIFO threshold bits of SSCR1 register */
    ixSspAccCfgStored.sscr1 =
        (ixSspAccCfgStored.sscr1 & (~IX_SSP_TX_FIFO_THLD_MASK)) |
        (txFIFOThresholdSelected << IX_SSP_TX_FIFO_THLD_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1);

    return IX_SSP_SUCCESS;
} /* end of ixSspAccTxFIFOThresholdSet */

PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOThresholdSet (
    IxSspAccFifoThreshold rxFIFOThresholdSelected)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccRxFIFOThresholdSet", IX_SSP_NOT_INIT);

    /* Check for validity of parameter */
    if(rxFIFOThresholdSelected >= INVALID_FIFO_TSHLD)
        return IX_SSP_INVALID_RX_FIFO_THRESHOLD_ENUM_VALUE;

    /* Write the parameter into the Rx FIFO threshold bits of SSCR1 register */
    ixSspAccCfgStored.sscr1 =
        (ixSspAccCfgStored.sscr1 & (~IX_SSP_RX_FIFO_THLD_MASK)) |
        (rxFIFOThresholdSelected << IX_SSP_RX_FIFO_THLD_LOC);
    IX_OSAL_WRITE_LONG (ixSspCR1Addr, ixSspAccCfgStored.sscr1);

    return IX_SSP_SUCCESS;
} /* end of ixSspAccRxFIFOThresholdSet */

/**
 * Debug functions
 */

PUBLIC IX_SSP_STATUS
ixSspAccStatsGet (
    IxSspAccStatsCounters *sspStats)
{
    if(NULL == sspStats)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixSspAccStatsGet: stats pointer is NULL.\n",
            0,0,0,0,0,0);
        return IX_SSP_FAIL;
    } /* end of NULL == sspStats */
        
    /* Copy the SSP stats counters to the struct pointer passed in */
    sspStats->ixSspRcvCounter = ixSspAccStatsCounters.ixSspRcvCounter;
    sspStats->ixSspXmitCounter = ixSspAccStatsCounters.ixSspXmitCounter;
    sspStats->ixSspOverflowCounter = ixSspAccStatsCounters.ixSspOverflowCounter;

    return IX_SSP_SUCCESS;
} /* end of ixSspAccStatsGet */

PUBLIC void
ixSspAccStatsReset (
    void)
{
    /* Clear the SSP stats counters to zero */
    ixSspAccStatsCounters.ixSspRcvCounter = 0;
    ixSspAccStatsCounters.ixSspXmitCounter = 0;
    ixSspAccStatsCounters.ixSspOverflowCounter = 0;

    return;
} /* end of ixSspAccStatsReset */

PUBLIC IX_SSP_STATUS
ixSspAccShow (
    void)
{
    IxSspAccStatsCounters sspStats;
    UINT8 TxFIFOLevel;
    UINT8 RxFIFOLevel;

    /* Disallow this function from running further if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccShow", IX_SSP_NOT_INIT);

    /* Read and display the SSP status */
    ixSspAccStsStored = IX_OSAL_READ_LONG (ixSspSRAddr);

    RxFIFOLevel = ixSspAccRxFIFOLevelGet();
    TxFIFOLevel = ixSspAccTxFIFOLevelGet();
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Rx FIFO Level  : %d\n", RxFIFOLevel,0,0,0,0,0);
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Tx FIFO Level  : %d\n", TxFIFOLevel,0,0,0,0,0);
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "1 = YES, 0 = NO\n",0,0,0,0,0,0);
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Rx FIFO Overrun: %d\n", ((ixSspAccStsStored &
        IX_SSP_RX_FIFO_OVERRUN_MASK) >> IX_SSP_RX_FIFO_OVERRUN_LOC),0,0,0,0,0);
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "SSP Busy: %d\n", ((ixSspAccStsStored & IX_SSP_BUSY_MASK) >>
        IX_SSP_BUSY_LOC),0,0,0,0,0);
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Rx FIFO Threshold Hit or Above: %d\n",
        ((ixSspAccStsStored & IX_SSP_RX_FIFO_SVC_REQ_MASK) >>
        IX_SSP_RX_FIFO_SVC_REQ_LOC),0,0,0,0,0);
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Tx FIFO Threshold Hit or Below: %d\n",
        ((ixSspAccStsStored & IX_SSP_TX_FIFO_SVC_REQ_MASK) >>
        IX_SSP_TX_FIFO_SVC_REQ_LOC),0,0,0,0,0);

    /* Read and display the SSP stats counters */
    ixSspAccStatsGet(&sspStats);
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "SSP frames received   : %d\n", sspStats.ixSspRcvCounter,0,0,0,0,0);
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "SSP frames transmitted: %d\n", sspStats.ixSspXmitCounter,0,0,0,0,0);
    ixOsalLog(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "SSP overflow occurence: %d\n", sspStats.ixSspOverflowCounter,0,0,0,0,0);

    return IX_SSP_SUCCESS;
} /* end of ixSspAccShow */

PUBLIC IX_SSP_STATUS
ixSspAccSSPBusyCheck (
    void)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccSSPBusyCheck", IX_SSP_NOT_INIT);

    /* Read the SSSR to determine the state of the SSP */
    ixSspAccStsStored = IX_OSAL_READ_LONG (ixSspSRAddr);
    /* Return the status of the SSP Port (busy or idle) */
    if(IX_SSP_IS_BUSY ==
        ((ixSspAccStsStored & IX_SSP_BUSY_MASK) >> IX_SSP_BUSY_LOC))
    {
        return IX_SSP_BUSY;
    }
    else
    {
        return IX_SSP_IDLE;
    }
} /* end of ixSspAccSSPBusyCheck */

PUBLIC UINT8
ixSspAccTxFIFOLevelGet (
    void)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccTxFIFOLevelGet", IX_SSP_FIFO_EMPTY);


    ixSspAccStsStored = IX_OSAL_READ_LONG (ixSspSRAddr);
    /* If the Tx FIFO level is non-zero, the value is the actual level */
    if(IX_SSP_FIFO_FULL_OR_EMPTY !=
        ((ixSspAccStsStored & IX_SSP_TX_FIFO_LVL_MASK) >>
        IX_SSP_TX_FIFO_LVL_LOC))
    {
        return ((ixSspAccStsStored & IX_SSP_TX_FIFO_LVL_MASK) >>
                IX_SSP_TX_FIFO_LVL_LOC);
    }

    /* If the Tx FIFO level is zero, the value can be 0 (empty) or 16 (full)
    depending on the Tx FIFO Not Full bit */
    if(IX_SSP_TX_FIFO_FULL ==
        ((ixSspAccStsStored & IX_SSP_TX_FIFO_NOT_FULL_MASK) >>
        IX_SSP_TX_FIFO_NOT_FULL_LOC))
    {
        return IX_SSP_FIFO_FULL;
    }
    else
    {
        return IX_SSP_FIFO_EMPTY;
    }
} /* end of ixSspAccTxFIFOLevelGet */

PUBLIC UINT8
ixSspAccRxFIFOLevelGet (
    void)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccRxFIFOLevelGet", IX_SSP_FIFO_EMPTY);

    ixSspAccStsStored = IX_OSAL_READ_LONG (ixSspSRAddr);
    /* If the Rx FIFO level is non-zero, the value is the actual level */
    if(IX_SSP_FIFO_FULL_OR_EMPTY !=
        ((((ixSspAccStsStored & IX_SSP_RX_FIFO_LVL_MASK) >>
        IX_SSP_RX_FIFO_LVL_LOC) + 1) & 0xF))
    {
        return (((ixSspAccStsStored & IX_SSP_RX_FIFO_LVL_MASK) >>
            IX_SSP_RX_FIFO_LVL_LOC) + 1);
    }

    /* If the Rx FIFO level is zero, the value can be 0 (empty) or 16 (full)
    depending on the Rx FIFO Not Empty bit */
    if(IX_SSP_RX_FIFO_EMPTY ==
        ((ixSspAccStsStored & IX_SSP_RX_FIFO_NOT_EMPTY_MASK) >>
        IX_SSP_RX_FIFO_NOT_EMPTY_LOC))
    {
        return IX_SSP_FIFO_EMPTY;
    }
    else
    {
        return IX_SSP_FIFO_FULL;
    }
} /* end of ixSspAccRxFIFOLevelGet */

PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOOverrunCheck (
    void)
{
    /* Disallow this function from running if SSP not initialized */
    IX_SSP_INIT_SUCCESS_CHECK("ixSspAccRxFIFOOverrunCheck", IX_SSP_NOT_INIT);

    ixSspAccStsStored = IX_OSAL_READ_LONG (ixSspSRAddr);
    /* Check if an overrun has occurred*/
    if(IX_SSP_OVERRUN_HAS_OCCURRED !=
        ((ixSspAccStsStored & IX_SSP_RX_FIFO_OVERRUN_MASK) >>
        IX_SSP_RX_FIFO_OVERRUN_LOC))
    {
        return IX_SSP_NO_OVERRUN;
    }

    /* Update the overrun stats counter */
    ixSspAccStatsCounters.ixSspOverflowCounter++;
    
    /* Write 1 to clear the overrun bit */
    ixSspAccStsStored = IX_SSP_SET_TO_BE_CLEARED << IX_SSP_RX_FIFO_OVERRUN_LOC; 
    IX_OSAL_WRITE_LONG (ixSspSRAddr, ixSspAccStsStored);

    return IX_SSP_OVERRUN_OCCURRED;
} /* end of ixSspAccRxFIFOOverrunCheck */

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccInterruptDetected (
    void);
 *
 * @brief The top level Interrupt Service Routine that is called when an SSP
 *          interrupt occurs
 *
 * @param - None 
 *
 * This function is the interrupt service routine that is called when a SSP
 * interrupt occurs. It will determine the source of the interrupt by checking
 * the SSSR and call the appropriate handler - ixRxFIFOOverrunHdlr,
 * ixRxFIFOThsldHdlr, or ixTxFIFOThsldHdlr
 *
 * @return 
 *      - void
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : yes
 *
 */
PRIVATE void ixSspAccInterruptDetected (
    void)
{
    ixSspAccStsStored = IX_OSAL_READ_LONG (ixSspSRAddr);
    if(IX_SSP_INTERRUPTED == ((ixSspAccStsStored &
        IX_SSP_RX_FIFO_OVERRUN_MASK) >> IX_SSP_RX_FIFO_OVERRUN_LOC))
    {
        /* Increment the overrun counter */
        ixSspAccStatsCounters.ixSspOverflowCounter++;
        
        /* Call the Rx FIFO Overrun handler registered in the IxSspInitVars */
        (*ixRxFIFOOverrunHdlr)();
        
        /* Clear the overrun interrupt */
        ixSspAccStsStored = IX_SSP_SET_TO_BE_CLEARED <<
                            IX_SSP_RX_FIFO_OVERRUN_LOC;
        IX_OSAL_WRITE_LONG (ixSspSRAddr, ixSspAccStsStored);
        return;
    } /* end of rxFIFOOverrun interrupt detected */

    if((IX_SSP_INTERRUPTED ==
        ((ixSspAccStsStored & IX_SSP_RX_FIFO_SVC_REQ_MASK) >>
        IX_SSP_RX_FIFO_SVC_REQ_LOC)) && (NULL != ixRxFIFOThsldHdlr))
    {
        /* Call the Rx FIFO threshold handler registered through the function
            ixSspAccInit or ixSspAccRxFIFOIntEnable */
        (*ixRxFIFOThsldHdlr)();
        return;
    } /* end of rxFIFO interrupt detected */
    
    if((IX_SSP_INTERRUPTED ==
        ((ixSspAccStsStored & IX_SSP_TX_FIFO_SVC_REQ_MASK) >>
        IX_SSP_TX_FIFO_SVC_REQ_LOC)) && (NULL != ixTxFIFOThsldHdlr))
    {
        /* Call the Tx FIFO threshold handler registered through the function
            ixSspAccInit or ixSspAccTxFIFOIntEnable */
        (*ixTxFIFOThsldHdlr)();
        return;
    } /* end of TxFIFO interrupt detected */

    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
        "ixSspAccInterruptDetected(): INVALID STATE REACHED\n", 0,0,0,0,0,0);
    return;
} /* end of ixSspAccInterruptDetected */

#endif /* __ixp46X || __ixp43X */
