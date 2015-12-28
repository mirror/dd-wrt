/**
 * @file IxParityENAccMcuPE.c
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Source file for MCU Parity Detection Enabler sub-component 
 * of the IXP400 Parity Error Notifier access component.
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

#if defined(__ixp46X) || defined(__ixp43X)

/* 
 * System defined include files
 */
#include "IxOsal.h"

/*
 * User defined include files
 */
#include "IxParityENAccIcE.h"
#include "IxParityENAccMcuPE.h"
#include "IxParityENAccMcuPE_p.h"

/* Virtual base address of MCU */
static UINT32 ixMcuVirtualBaseAddr = 0;

/*
 * MCU Sub-module level functions definitions
 */

IX_STATUS
ixParityENAccMcuPEInit (IxParityENAccInternalCallback ixMcuPECallback)
{
    UINT32 virtualBaseAddr = 0;

    /* Verify parameters */
    if ((IxParityENAccInternalCallback)NULL == ixMcuPECallback)
    {
        return IX_FAIL;
    } /* end of if */

    /* Memory mapping of the MCU registers */
    if ((UINT32)NULL == (virtualBaseAddr = (UINT32) IX_OSAL_MEM_MAP (
                                            IXP400_PARITYENACC_MCU_BASEADDR,
                                            IXP400_PARITYENACC_MCU_MEMMAP_SIZE)))
    {
        return IX_FAIL;
    } /* end of if */

    ixMcuVirtualBaseAddr = virtualBaseAddr;

    /* Virtual Addresses assignment for MCU Registers */
    ixParityENAccMcuPEConfig.mcuPERegisters.mcuEccr  = 
        virtualBaseAddr + IXP400_PARITYENACC_MCU_ECCR_OFFSET;
    ixParityENAccMcuPEConfig.mcuPERegisters.mcuElog0 = 
        virtualBaseAddr + IXP400_PARITYENACC_MCU_ELOG0_OFFSET;
    ixParityENAccMcuPEConfig.mcuPERegisters.mcuElog1 = 
        virtualBaseAddr + IXP400_PARITYENACC_MCU_ELOG1_OFFSET;
    ixParityENAccMcuPEConfig.mcuPERegisters.mcuEcar0 = 
        virtualBaseAddr + IXP400_PARITYENACC_MCU_ECAR0_OFFSET;
    ixParityENAccMcuPEConfig.mcuPERegisters.mcuEcar1 = 
        virtualBaseAddr + IXP400_PARITYENACC_MCU_ECAR1_OFFSET;
    ixParityENAccMcuPEConfig.mcuPERegisters.mcuMcisr = 
        virtualBaseAddr + IXP400_PARITYENACC_MCU_MCISR_OFFSET;

    /* Register main module internal callback routine */
    ixParityENAccMcuPEConfig.mcuPECallback = ixMcuPECallback;

    /* Interrupt Service Routine Info for debug purpose only */
    ixParityENAccMcuPEConfig.mcuIsrInfo.mcuInterruptId = 
        IRQ_IXP400_INTC_PARITYENACC_MCU;
    ixParityENAccMcuPEConfig.mcuIsrInfo.mcuIsr = ixParityENAccMcuPEIsr;

    /*
     * Disable parity error detection for both single and multi-bit ECC
     * and correction of single bit parity using ECC
     */
    IXP400_PARITYENACC_REG_BIT_CLEAR(
        ixParityENAccMcuPEConfig.mcuPERegisters.mcuEccr,
        IXP400_PARITYENACC_MCU_SBIT_CORRECT_MASK |
        IXP400_PARITYENACC_MCU_MBIT_REPORT_MASK  |
        IXP400_PARITYENACC_MCU_SBIT_REPORT_MASK);

    /* Clear off the pending interrupts, if any */
    IXP400_PARITYENACC_REG_BIT_SET(
        ixParityENAccMcuPEConfig.mcuPERegisters.mcuMcisr,
        IXP400_PARITYENACC_MCU_ERROR0_MASK |
        IXP400_PARITYENACC_MCU_ERROR1_MASK |
        IXP400_PARITYENACC_MCU_ERRORN_MASK);

    /* Install MCU Interrupt Service Routine after disabling the interrupt */
    {
        INT32 lockKey = ixOsalIrqLock();
        if ((IX_SUCCESS != ixOsalIrqBind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_MCU,
                                        (IxOsalVoidFnVoidPtr) ixParityENAccMcuPEIsr,
                                        (void *) NULL)) ||
            (IX_SUCCESS != ixParityENAccIcInterruptDisable(
                            IXP400_PARITYENACC_INTC_MCU_PARITY_INTERRUPT)))
        {
            ixOsalIrqUnlock(lockKey);
            IX_OSAL_MEM_UNMAP (virtualBaseAddr);
            return IX_FAIL;
        } /* end of if */
        ixOsalIrqUnlock(lockKey);
    }
    return IX_SUCCESS;
} /* end of ixParityENAccMcuPEInit() function */

IX_STATUS 
ixParityENAccMcuPEDetectionConfigure (IxParityENAccMcuPEConfigOption ixMcuPDCfg)
{
    UINT32 mcuPDCfgFlags  = IXP400_PARITYENACC_MCU_ECC_EN_MASK; /* Always to be included */
    UINT32 mcuPDCfgStatus = 0;
    UINT32 mcuTmpPDCfgStatus = 0;

    /*
     * Enable parity error detection for given options
     *
     * - ECC enable
     * - Single-bit ECC error report
     * - Multi-bit ECC error report
     * - Single-bit parity correction using ECC
     */

    /* Enable Single-bit parity error detection */
    if (IXP400_PARITYENACC_PE_ENABLE == ixMcuPDCfg.singlebitDetectEnabled)
    {
        mcuPDCfgFlags |= IXP400_PARITYENACC_MCU_SBIT_REPORT_MASK;
    } /* end of if */

    /* Enable Single-bit parity error correction */
    if (IXP400_PARITYENACC_PE_ENABLE == ixMcuPDCfg.singlebitCorrectionEnabled)
    {
        mcuPDCfgFlags |= IXP400_PARITYENACC_MCU_SBIT_CORRECT_MASK;
    } /* end of if */

    /* Enable Multi-bit parity error detection */
    if (IXP400_PARITYENACC_PE_ENABLE == ixMcuPDCfg.multibitDetectionEnabled)
    {
        mcuPDCfgFlags |= IXP400_PARITYENACC_MCU_MBIT_REPORT_MASK;
    } /* end of if */

    /* Check the current ECC feature configuration */
    IXP400_PARITYENACC_REG_READ(ixParityENAccMcuPEConfig.mcuPERegisters.mcuEccr,
        &mcuPDCfgStatus);

    /* Existing configuration is same as requested one */
    if (mcuPDCfgStatus == mcuPDCfgFlags)
    {
        return IX_SUCCESS;
    }
    mcuPDCfgStatus = mcuPDCfgFlags;

    IXP400_PARITYENACC_REG_WRITE((ixParityENAccMcuPEConfig.mcuPERegisters.mcuEccr),
        mcuPDCfgStatus);

    /* Verify that configuration has been successful or not */
    IXP400_PARITYENACC_REG_READ((ixParityENAccMcuPEConfig.mcuPERegisters.mcuEccr),
        &mcuTmpPDCfgStatus);

    if (mcuTmpPDCfgStatus == mcuPDCfgStatus)
    {
        /* Enable/Disable the corresponding interrupt at Interrupt Controller */
        IXP400_PARITYENACC_VAL_BIT_CLEAR(mcuPDCfgFlags,IXP400_PARITYENACC_MCU_ECC_EN_MASK);
        return (0 != mcuPDCfgFlags) ?
                        ixParityENAccIcInterruptEnable( 
                            IXP400_PARITYENACC_INTC_MCU_PARITY_INTERRUPT) :
                        ixParityENAccIcInterruptDisable(
                            IXP400_PARITYENACC_INTC_MCU_PARITY_INTERRUPT);
    }
    else
    {
        return IX_FAIL;
    } /* end of if */
} /* end of ixParityENAccMcuPEDetectionConfigure() function */

IX_STATUS 
ixParityENAccMcuPEParityErrorContextFetch (
    IxParityENAccMcuPEParityErrorContext *ixMcuPECMsg)
{
    BOOL mcuParityError0 = FALSE;
    BOOL mcuParityError1 = FALSE;
    BOOL mcuParityErrorN = FALSE;

    UINT32 mcuParitySource0 = IXP400_PARITYENACC_MCU_ERR_SMBIT_SGL;
    UINT32 mcuParitySource1 = IXP400_PARITYENACC_MCU_ERR_SMBIT_SGL;

    if ((IxParityENAccMcuPEParityErrorContext *)NULL == ixMcuPECMsg)
    {
        return IX_FAIL;
    } /* end of if */

    /* Fetch MCU registers' contents into internal datastructure */
    ixParityENAccMcuPEParityErrorStatusGet();

    /*
     * Prepare the Parity Error Context as needed by the Main sub-module
     */

    /* Identify the Multi & Single bit parity errors */
    ixParityENAccMcuPEParityErrorStatusInterpret ( &mcuParityError0, 
        &mcuParityError1, &mcuParityErrorN, &mcuParitySource0, &mcuParitySource1);

    /*
     * Select multi or single-bit parity errors as show in decision table
     *
     *        Parity #1 -->
     *
     *             -               S           M
     *  P     +------------+------------+------------+
     *  a  -  | -(10)/O(9) |    1 (8)   |    1 (5)   |
     *  r     +------------+------------+------------+
     *  i  S  |    0 (7)   |    0 (6)   |    1 (4)   |
     *  t     +------------+------------+------------+
     *  y  M  |    0 (3)   |    0 (2)   |    0 (1)   |
     *        +------------+------------+------------+
     *  #0
     *  |
     *  v
     *
     * While: -   No parity error
     *        O   Overflow of parity errors
     *        S   Single bit parity error
     *        M   Multibit parity error
     *        0   Parity error #0
     *        1   Parity error #1
     *        (n) Parity error reporting/notification order
     */

    /* Parity Error Not Set (Row #1 & Column #1) */
    if ((FALSE == mcuParityError0) && (FALSE == mcuParityError1))
    {
        /* No Parity Errors? */
        if (FALSE == mcuParityErrorN)
        {
            ixMcuPECMsg->mcuParitySource = IXP400_PARITYENACC_PE_MCU_NOPARITY;
            return IX_SUCCESS;
        } /* else of if */
        /* Overflow of Parity Errors
         *
         * This scenario occurs after clearing the other multi/single-bit
         * parity errors and the Overflow of parity errors have been observed
         *
         * NOTE: Other Parity Error Context attributes will have undetermined
         *       values
         */
        else
        {
            ixMcuPECMsg->mcuParitySource = IXP400_PARITYENACC_PE_MCU_OVERFLOW;
            return IX_SUCCESS;
        } /* end of if */
    } /* end of if */

    /*
     * Retrieve Parity Error #1
     *
     * Case-A: Parity Error #1 is multi-bit  (AND) Parity Error #0 is single-bit
     * i.e., (Notification #4) (OR)
     * Case-B: Parity Error #1 is multi-bit  (AND) Parity Error #0 not Set
     * i.e., (Notification #5) (OR)  
     * Case-C: Parity Error #1 is single-bit (AND) Parity Error #0 not Set
     * i.e., (Notification #8)
     */
    if ((TRUE == mcuParityError1) && 
        /* Case - A */
        (((IXP400_PARITYENACC_MCU_ERR_SMBIT_MLT == mcuParitySource1) &&
         (((TRUE == mcuParityError0) && 
           (IXP400_PARITYENACC_MCU_ERR_SMBIT_SGL == mcuParitySource0)) ||
        /* Case - B */
          (FALSE == mcuParityError0))) ||
        /* Case - C */
        ((IXP400_PARITYENACC_MCU_ERR_SMBIT_SGL == mcuParitySource1) &&
         (FALSE == mcuParityError0))))
    {
        ixParityENAccMcuPEParityErrorStatusTransform (ixMcuPECMsg,
        ixParityENAccMcuPEConfig.mcuParityErrorStatus.mcuElog1Value,
            ixParityENAccMcuPEConfig.mcuParityErrorStatus.mcuEcar1Value);

        return IX_SUCCESS;
    } /* end of if */

    /* 
     * All other cases where Parity Error #0 will be retrieved
     */
    ixParityENAccMcuPEParityErrorStatusTransform (ixMcuPECMsg,
        ixParityENAccMcuPEConfig.mcuParityErrorStatus.mcuElog0Value,
        ixParityENAccMcuPEConfig.mcuParityErrorStatus.mcuEcar0Value);

    return IX_SUCCESS;
} /* end of ixParityENAccMcuPEParityErrorContextFetch() function */

IX_STATUS
ixParityENAccMcuPEParityInterruptClear (
    IxParityENAccMcuPEParityErrorSource ixMcuParityErrSrc,
    IxParityENAccPEParityErrorAddress ixMcuParityErrAddress)
{
    BOOL mcuParityError0 = FALSE;
    BOOL mcuParityError1 = FALSE;
    BOOL mcuParityErrorN = FALSE;

    UINT32 mcuParitySource0 = IXP400_PARITYENACC_MCU_ERR_SMBIT_SGL;
    UINT32 mcuParitySource1 = IXP400_PARITYENACC_MCU_ERR_SMBIT_SGL;

    register IxParityENAccMcuPERegisters *mcuPERegisters = 
                &ixParityENAccMcuPEConfig.mcuPERegisters;

    IxParityENAccPEParityErrorAddress mcuParityErrorAddr = 
        IXP400_PARITYENACC_VAL_READ(ixParityENAccMcuPEConfig.\
            mcuParityErrorStatus.mcuEcar0Value, IXP400_PARITYENACC_MCU_ERR_ADDRESS_MASK);

    /* Identify the Multi & Single bit parity errors */
    ixParityENAccMcuPEParityErrorStatusInterpret (
        &mcuParityError0, &mcuParityError1, &mcuParityErrorN,
        &mcuParitySource0, &mcuParitySource1);

    switch (ixMcuParityErrSrc)
    {
        case IXP400_PARITYENACC_PE_MCU_SBIT:
        {
            /* Parity error number #0 is of single bit type */
            if ((TRUE == mcuParityError0) && 
                (IXP400_PARITYENACC_MCU_ERR_SMBIT_SGL == mcuParitySource0) &&
                (mcuParityErrorAddr == ixMcuParityErrAddress))
            {
                /* Write '1' to clear the single bit parity interrupt */
                IXP400_PARITYENACC_REG_WRITE(mcuPERegisters->mcuMcisr,
                    IXP400_PARITYENACC_MCU_ERROR0_MASK);
                break;
            } /* end of if */
            
            /* Parity error number #1 is of single bit type */
    
            /* Write '1' to clear the single bit parity interrupt */
            IXP400_PARITYENACC_REG_WRITE(mcuPERegisters->mcuMcisr,
                IXP400_PARITYENACC_MCU_ERROR1_MASK);
            break;
        } /* end of case IXP400_PARITYENACC_PE_MCU_SBIT */

        case IXP400_PARITYENACC_PE_MCU_MBIT:
        {
            /* Parity error number #0 is of multi bit type */
            if ((TRUE == mcuParityError0) && 
                (IXP400_PARITYENACC_MCU_ERR_SMBIT_MLT == mcuParitySource0) &&
                (mcuParityErrorAddr == ixMcuParityErrAddress))
            {
                /* Write '1' to clear the multi bit parity interrupt */
                IXP400_PARITYENACC_REG_WRITE(mcuPERegisters->mcuMcisr,
                    IXP400_PARITYENACC_MCU_ERROR0_MASK);
                break;
            } /* end of if */
            
            /* Parity error number #1 is of multi bit type */
    
            /* Write '1' to clear the multi bit parity interrupt */
            IXP400_PARITYENACC_REG_WRITE(mcuPERegisters->mcuMcisr,
                IXP400_PARITYENACC_MCU_ERROR1_MASK);
            break;
        } /* end of case IXP400_PARITYENACC_PE_MCU_MBIT */

        case IXP400_PARITYENACC_PE_MCU_OVERFLOW:
        {
            /* Write '1' to clear the Parity Overflow Interrupt */
            IXP400_PARITYENACC_REG_WRITE(mcuPERegisters->mcuMcisr,
                IXP400_PARITYENACC_MCU_ERRORN_MASK);
            break;
        } /* end of case IXP400_PARITYENACC_PE_MCU_OVERFLOW */
        default:
        {
            /* This part of the code should never be reached */
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccMcuPEParityInterruptClear(): "
                "Invalid MCU interrupt source to clear\n", 0,0,0,0,0,0);
            return IX_FAIL;
        } /* end of case default */
    } /* end of switch */

    return IX_SUCCESS;
} /* end of ixParityENAccMcuPEParityInterruptClear() function */

void
ixParityENAccMcuPEIsr (void)
{
    /*
     * No need to read the ECC/parity error status in the ISR
     * Invoke the internal MCU callback routine to notify the
     * ECC/pairty error detected.
     * 
     * NOTE: The ECC/parity error context information will be 
     * obtained only when the public API client application
     * requests for such information.
     */
    (*ixParityENAccMcuPEConfig.mcuPECallback)(
        ixParityENAccMcuPEConfig.mcuIsrInfo.mcuInterruptId,
        ixParityENAccMcuPEConfig.mcuIsrInfo.mcuIsr);
} /* end of ixParityENAccMcuPEIsr() function */

void
ixParityENAccMcuPEParityErrorStatusGet (void)
{
    register IxParityENAccMcuPERegisters *mcuPERegisters = 
                &ixParityENAccMcuPEConfig.mcuPERegisters;
    register IxParityENAccMcuPEParityErrorStatus *mcuPEStatus = 
                &ixParityENAccMcuPEConfig.mcuParityErrorStatus;
    /*
     * Read the raw parity error status into local datastructure
     * from the various MCU registers
     */

    /* ECC Control Register contents */
    IXP400_PARITYENACC_REG_READ(mcuPERegisters->mcuEccr, &mcuPEStatus->mcuEccrValue);

    /* ECC Log Register_0 contents */
    IXP400_PARITYENACC_REG_READ(mcuPERegisters->mcuElog0, &mcuPEStatus->mcuElog0Value);
    
    /* ECC Log Register_1 contents */
    IXP400_PARITYENACC_REG_READ(mcuPERegisters->mcuElog1, &mcuPEStatus->mcuElog1Value);

    /* ECC Address Register_0 contents */
    IXP400_PARITYENACC_REG_READ(mcuPERegisters->mcuEcar0, &mcuPEStatus->mcuEcar0Value);

    /* ECC Address Register_1 contents */
    IXP400_PARITYENACC_REG_READ(mcuPERegisters->mcuEcar1, &mcuPEStatus->mcuEcar1Value);

    /* Memory Controller Interrupt Status Register contents */
    IXP400_PARITYENACC_REG_READ(mcuPERegisters->mcuMcisr, &mcuPEStatus->mcuMcisrValue);
} /* end of ixParityENAccMcuPEParityErrorStatusGet() function */

void
ixParityENAccMcuPEParityErrorStatusTransform (
    IxParityENAccMcuPEParityErrorContext *ixMcuPECMsg,
    UINT32 mcuElogNValue,
    UINT32 mcuEcarNValue)
{
    /*
     * Input parameter validation is not done in the local functions 
     * to avoid extra code.
     */

        /* ECC/parity error is of Single or Multi bit type */
        ixMcuPECMsg->mcuParitySource = 
            (IXP400_PARITYENACC_MCU_ERR_SMBIT_SGL == 
             IXP400_PARITYENACC_VAL_READ(mcuElogNValue,
                 IXP400_PARITYENACC_MCU_ERR_SMBIT_MASK)) ? 
             IXP400_PARITYENACC_PE_MCU_SBIT : IXP400_PARITYENACC_PE_MCU_MBIT;

        /* Identify Read or Write access that has caused parity error */
        ixMcuPECMsg->mcuAccessType =
            (IXP400_PARITYENACC_MCU_ERR_RW_READ == 
             IXP400_PARITYENACC_VAL_READ(mcuElogNValue,
                 IXP400_PARITYENACC_MCU_ERR_RW_MASK)) ?
                     IXP400_PARITYENACC_PE_READ : IXP400_PARITYENACC_PE_WRITE;

        /* Fetch the parity error syndrome */
        ixMcuPECMsg->mcuParityData = (IxParityENAccPEParityErrorData)
             IXP400_PARITYENACC_VAL_READ(mcuElogNValue,
                 IXP400_PARITYENACC_MCU_ERR_SYNDROME_MASK);

        /* Identify the requesting interface to the MCU */
        ixMcuPECMsg->mcuRequester = 
            (IXP400_PARITYENACC_MCU_ERR_MASTER_CORE_BIU == 
             IXP400_PARITYENACC_VAL_READ(mcuElogNValue,
                 IXP400_PARITYENACC_MCU_ERR_MASTER_MASK)) ?
                     IXP400_PARITYENACC_PE_MCU_MPI : IXP400_PARITYENACC_PE_MCU_AHB_BUS;

        /* Feth the parity error address */
        ixMcuPECMsg->mcuParityAddress = (IxParityENAccPEParityErrorAddress)
            IXP400_PARITYENACC_VAL_READ(mcuEcarNValue,
                IXP400_PARITYENACC_MCU_ERR_ADDRESS_MASK);
} /* end of ixParityENAccMcuPEParityErrorStatusTransform() function */

IX_STATUS 
ixParityENAccMcuPEUnload(void)
{
    UINT32 lockKey;
    UINT32 status = IX_SUCCESS;
    IxParityENAccMcuPEConfigOption ixMcuPDCfg;
    
    ixMcuPDCfg.singlebitDetectEnabled = IXP400_PARITYENACC_PE_DISABLE;
    ixMcuPDCfg.singlebitCorrectionEnabled = IXP400_PARITYENACC_PE_DISABLE;
    ixMcuPDCfg.multibitDetectionEnabled =  IXP400_PARITYENACC_PE_DISABLE;
    ixParityENAccMcuPEDetectionConfigure(ixMcuPDCfg);

    /* Unbind the IRQ */    
    lockKey = ixOsalIrqLock();
    if (IX_SUCCESS != ixOsalIrqUnbind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_MCU))
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
            "ixParityENAccMcuPEUnload(): "\
            "Can't unbind the MCU ISR to IRQ_IXP400_INTC_PARITYENACC_MCU!!!\n",0,0,0,0,0,0);
        status = IX_FAIL;
    }
    ixOsalIrqUnlock(lockKey);

    /* Unmap the memory */
    IX_OSAL_MEM_UNMAP(ixMcuVirtualBaseAddr);

    return status;
} /* end of ixParityENAccMcuPEUnload() function */

#endif /* __ixp46X || __ixp43X */
