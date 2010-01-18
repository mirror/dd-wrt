/**
 * @file IxParityENAccNpePE.c
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief  Source file for NPE Parity Detection Enabler sub-component 
 * of the IXP400 Parity Error Notifier access component.
 *
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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
#define IXPARITYENACCNPEPE_C
/* 
 * System defined include files
 */
#include "IxOsal.h"

/*
 * User defined include files
 *
 * NOTE: The NPE headerfiles must appear in that order
 */
#include "IxParityENAccIcE.h"
#include "IxParityENAccNpePE.h"
#include "IxParityENAccNpePE_p.h"

/*
 * NPE sub-module level functions definitions
 */

IX_STATUS
ixParityENAccNpePEInit (IxParityENAccInternalCallback ixNpePECallback)
{
    UINT32 npeAVirtualBaseAddr = 0;
    UINT32 npeBVirtualBaseAddr = 0;
    UINT32 npeCVirtualBaseAddr = 0;
    UINT32 ebcVirtualBaseAddr  = 0;
    UINT32 ebcConfigReg1Value  = 0;

    /*
     * These Bits are to be combined always with the Parity Error Detection 
     * configuration bits.  Otherwise their effect is Nullified
     */
    UINT32 npePDCfgFlags  = IXP400_PARITYENACC_NPE_CONTROL_IPEWE | 
                            IXP400_PARITYENACC_NPE_CONTROL_DPEWE |
                            IXP400_PARITYENACC_NPE_CONTROL_EEEWE;
    UINT32 npePDCfgStatus = 0;

    register IxParityENAccNpePEConfig *npeAPEConfig = 
        &ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_A];
    register IxParityENAccNpePEConfig *npeBPEConfig = 
        &ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_B];
    register IxParityENAccNpePEConfig *npeCPEConfig = 
        &ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_C];

    /* Verify parameters */
    if ((IxParityENAccInternalCallback)NULL == ixNpePECallback)
    {

        return IX_FAIL;
    } /* end of if */

    /* Memory mapping of the NPE-A registers */
    if ((UINT32)NULL == (npeAVirtualBaseAddr = (UINT32) IX_OSAL_MEM_MAP (
                                                IXP400_PARITYENACC_NPEA_BASEADDR,
                                                IXP400_PARITYENACC_NPE_MEMMAP_SIZE)))
    {
        return IX_FAIL;
    } /* end of if */

    /* Memory mapping of the NPE-B registers */
    if ((UINT32)NULL == (npeBVirtualBaseAddr = (UINT32) IX_OSAL_MEM_MAP (
                                                IXP400_PARITYENACC_NPEB_BASEADDR,
                                                IXP400_PARITYENACC_NPE_MEMMAP_SIZE)))
    {
        IX_OSAL_MEM_UNMAP(npeAVirtualBaseAddr);
        return IX_FAIL;
    } /* end of if */

    /* Memory mapping of the NPE-C registers */
    if ((UINT32)NULL == (npeCVirtualBaseAddr = (UINT32) IX_OSAL_MEM_MAP (
                                                IXP400_PARITYENACC_NPEC_BASEADDR,
                                                IXP400_PARITYENACC_NPE_MEMMAP_SIZE)))
    {
        IX_OSAL_MEM_UNMAP(npeAVirtualBaseAddr);
        IX_OSAL_MEM_UNMAP(npeBVirtualBaseAddr);
        return IX_FAIL;
    } /* end of if */

    /* Memory mapping of the Expansion Bus Controller Config Register #1 */
    if ((UINT32)NULL == (ebcVirtualBaseAddr =  (UINT32) IX_OSAL_MEM_MAP (
                                                IXP400_PARITYENACC_EBC_BASEADDR,
                                                IXP400_PARITYENACC_EBC_MEMMAP_SIZE)))
    {
        IX_OSAL_MEM_UNMAP(npeAVirtualBaseAddr);
        IX_OSAL_MEM_UNMAP(npeBVirtualBaseAddr);
        IX_OSAL_MEM_UNMAP(npeCVirtualBaseAddr);
        return IX_FAIL;
    } /* end of if */


    /* 
     * Fetch the NPE Error Handling Enable Status from Expansion Bus Controller
     * Config Register #1
     */

    /* Enable the NPE Error Handling Bits for Parity Error Interrupt Generation for EXT ERR */
    IXP400_PARITYENACC_REG_BIT_SET(ebcVirtualBaseAddr, IXP400_PARITYENACC_NPE_EXPCNFG1_NPEA_ERREN);
    IXP400_PARITYENACC_REG_BIT_SET(ebcVirtualBaseAddr, IXP400_PARITYENACC_NPE_EXPCNFG1_NPEB_ERREN);
    IXP400_PARITYENACC_REG_BIT_SET(ebcVirtualBaseAddr, IXP400_PARITYENACC_NPE_EXPCNFG1_NPEC_ERREN);

    IXP400_PARITYENACC_REG_READ(ebcVirtualBaseAddr, &ebcConfigReg1Value);

    ixParityENAccNpePEErrorHandlingEnable[IXP400_PARITYENACC_PE_NPE_A] = 
        IXP400_PARITYENACC_VAL_BIT_CHECK(ebcConfigReg1Value,
            IXP400_PARITYENACC_NPE_EXPCNFG1_NPEA_ERREN);

    ixParityENAccNpePEErrorHandlingEnable[IXP400_PARITYENACC_PE_NPE_B] = 
        IXP400_PARITYENACC_VAL_BIT_CHECK(ebcConfigReg1Value,
            IXP400_PARITYENACC_NPE_EXPCNFG1_NPEB_ERREN);

    ixParityENAccNpePEErrorHandlingEnable[IXP400_PARITYENACC_PE_NPE_C] = 
        IXP400_PARITYENACC_VAL_BIT_CHECK(ebcConfigReg1Value,
            IXP400_PARITYENACC_NPE_EXPCNFG1_NPEC_ERREN);

    /* Virtual Addresses assignment for NPE-A/B/C Registers */
    npeAPEConfig->npePERegisters.npeStatusRegister  = 
        npeAVirtualBaseAddr + IXP400_PARITYENACC_NPE_STATUS_OFFSET;
    npeAPEConfig->npePERegisters.npeControlRegister = 
        npeAVirtualBaseAddr + IXP400_PARITYENACC_NPE_CONTROL_OFFSET;

    npeBPEConfig->npePERegisters.npeStatusRegister  = 
        npeBVirtualBaseAddr + IXP400_PARITYENACC_NPE_STATUS_OFFSET;
    npeBPEConfig->npePERegisters.npeControlRegister = 
        npeBVirtualBaseAddr + IXP400_PARITYENACC_NPE_CONTROL_OFFSET;

    npeCPEConfig->npePERegisters.npeStatusRegister  = 
        npeCVirtualBaseAddr + IXP400_PARITYENACC_NPE_STATUS_OFFSET;
    npeCPEConfig->npePERegisters.npeControlRegister = 
        npeCVirtualBaseAddr + IXP400_PARITYENACC_NPE_CONTROL_OFFSET;

    /* Register main module internal callback routines for NPEs */
    npeAPEConfig->npePECallback = ixNpePECallback;
    npeBPEConfig->npePECallback = ixNpePECallback;
    npeCPEConfig->npePECallback = ixNpePECallback;

    /* Interrupt Service Routine(s) Info for NPEs */
    npeAPEConfig->npeIsrInfo.npeInterruptId = IRQ_IXP400_INTC_PARITYENACC_NPEA;
    npeAPEConfig->npeIsrInfo.npeIsr = ixParityENAccNpePENpeAIsr;

    npeBPEConfig->npeIsrInfo.npeInterruptId = IRQ_IXP400_INTC_PARITYENACC_NPEB;
    npeBPEConfig->npeIsrInfo.npeIsr = ixParityENAccNpePENpeBIsr;

    npeCPEConfig->npeIsrInfo.npeInterruptId = IRQ_IXP400_INTC_PARITYENACC_NPEC;
    npeCPEConfig->npeIsrInfo.npeIsr = ixParityENAccNpePENpeCIsr;

    /*
     * Disable parity error detection for the IMEM, DMEM and Ext Error 
     * of all the NPEs
     */

    /* 
     * Enable the Write Enable Bits to clear off IMEM, DMEM & Ext Error bits
     */
    IXP400_PARITYENACC_REG_READ(npeAPEConfig->npePERegisters.npeControlRegister, &npePDCfgStatus);

    IXP400_PARITYENACC_VAL_BIT_SET(npePDCfgStatus, npePDCfgFlags);
    IXP400_PARITYENACC_VAL_BIT_CLEAR(npePDCfgStatus, IXP400_PARITYENACC_NPE_CONTROL_IPE |
        IXP400_PARITYENACC_NPE_CONTROL_DPE | IXP400_PARITYENACC_NPE_CONTROL_EEE);

    IXP400_PARITYENACC_REG_BIT_SET(npeAPEConfig->npePERegisters.npeControlRegister, npePDCfgStatus);


    IXP400_PARITYENACC_REG_READ(npeBPEConfig->npePERegisters.npeControlRegister, &npePDCfgStatus);

    IXP400_PARITYENACC_VAL_BIT_SET(npePDCfgStatus, npePDCfgFlags);
    IXP400_PARITYENACC_VAL_BIT_CLEAR(npePDCfgStatus, IXP400_PARITYENACC_NPE_CONTROL_IPE |
        IXP400_PARITYENACC_NPE_CONTROL_DPE | IXP400_PARITYENACC_NPE_CONTROL_EEE);

    IXP400_PARITYENACC_REG_BIT_SET(npeBPEConfig->npePERegisters.npeControlRegister, npePDCfgStatus);


    IXP400_PARITYENACC_REG_READ(npeCPEConfig->npePERegisters.npeControlRegister, &npePDCfgStatus);

    IXP400_PARITYENACC_VAL_BIT_SET(npePDCfgStatus, npePDCfgFlags);
    IXP400_PARITYENACC_VAL_BIT_CLEAR(npePDCfgStatus, IXP400_PARITYENACC_NPE_CONTROL_IPE |
        IXP400_PARITYENACC_NPE_CONTROL_DPE | IXP400_PARITYENACC_NPE_CONTROL_EEE);

    IXP400_PARITYENACC_REG_BIT_SET(npeCPEConfig->npePERegisters.npeControlRegister, npePDCfgStatus);

    /* Install NPE-A/B/C Interrupt Service Routines */
    {
        INT32 lockKey = ixOsalIrqLock();
        if ((IX_SUCCESS != ixOsalIrqBind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_NPEA,
                            (IxOsalVoidFnVoidPtr) ixParityENAccNpePENpeAIsr, (void *) NULL)) ||
            (IX_FAIL == ixParityENAccIcInterruptDisable(
                            IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT)))
        {
            ixOsalIrqUnlock(lockKey);
            IX_OSAL_MEM_UNMAP(npeAVirtualBaseAddr);
            IX_OSAL_MEM_UNMAP(npeBVirtualBaseAddr);
            IX_OSAL_MEM_UNMAP(npeCVirtualBaseAddr);
            IX_OSAL_MEM_UNMAP(ebcVirtualBaseAddr);
            return IX_FAIL;
        } /* end of if */

        if ((IX_SUCCESS != ixOsalIrqBind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_NPEB,
                            (IxOsalVoidFnVoidPtr) ixParityENAccNpePENpeBIsr, (void *) NULL)) ||
            (IX_FAIL == ixParityENAccIcInterruptDisable(
                            IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT)))
        {
            ixOsalIrqUnbind((UINT32) IRQ_IXP400_INTC_PARITYENACC_NPEA);

            ixOsalIrqUnlock(lockKey);
            IX_OSAL_MEM_UNMAP(npeAVirtualBaseAddr);
            IX_OSAL_MEM_UNMAP(npeBVirtualBaseAddr);
            IX_OSAL_MEM_UNMAP(npeCVirtualBaseAddr);
            IX_OSAL_MEM_UNMAP(ebcVirtualBaseAddr);
            return IX_FAIL;
        } /* end of if */

        if ((IX_SUCCESS != ixOsalIrqBind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_NPEC,
                            (IxOsalVoidFnVoidPtr) ixParityENAccNpePENpeCIsr, (void *) NULL))||
            (IX_FAIL == ixParityENAccIcInterruptDisable(
                            IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT)))
        {
            ixOsalIrqUnbind((UINT32) IRQ_IXP400_INTC_PARITYENACC_NPEA);
            ixOsalIrqUnbind((UINT32) IRQ_IXP400_INTC_PARITYENACC_NPEB);

            ixOsalIrqUnlock(lockKey);
            IX_OSAL_MEM_UNMAP(npeAVirtualBaseAddr);
            IX_OSAL_MEM_UNMAP(npeBVirtualBaseAddr);
            IX_OSAL_MEM_UNMAP(npeCVirtualBaseAddr);
            IX_OSAL_MEM_UNMAP(ebcVirtualBaseAddr);
            return IX_FAIL;
        } /* end of if */
        ixOsalIrqUnlock(lockKey);
    }

    return IX_SUCCESS;
} /* end of ixParityENAccNpePEInit() function */

IX_STATUS
ixParityENAccNpePEDetectionConfigure (
    IxParityENAccPENpeId ixNpeId,
    IxParityENAccNpePEConfigOption ixNpePDCfg)
{
    UINT32 npePDCfgFlags  = IXP400_PARITYENACC_NPE_CONTROL_IPE |
                            IXP400_PARITYENACC_NPE_CONTROL_DPE |
                            IXP400_PARITYENACC_NPE_CONTROL_EEE ;

    /* These WE bits are essential to update other flags */
    UINT32 npePDCtlFlags  = IXP400_PARITYENACC_NPE_CONTROL_IPEWE | 
                            IXP400_PARITYENACC_NPE_CONTROL_DPEWE |
                            IXP400_PARITYENACC_NPE_CONTROL_EEEWE |
                            IXP400_PARITYENACC_NPE_CONTROL_PPWE; 
    UINT32 npePDCfgStatus = 0;
    UINT32 npeTmpPDCfgStatus = 0;

    /* Validate parameters */
    if (ixNpeId >= IXP400_PARITYENACC_PE_NPE_MAX)
    {
        return IX_FAIL;
    } /* end of if */

    /* Get current parity detection configuration */
#ifdef __vxworks
    /* 
     * Fix to handle the NPE configuration hang issue with ethAcc END driver
     * enabled in the bootrom and/or vxWorks.st which generates NPE related
     * parity errors prior to the NPE parity detection configuration.
     */
    if (IX_FAIL == ixParityENAccIcInterruptDisable(
                       (IXP400_PARITYENACC_PE_NPE_A == ixNpeId) ? 
                           IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT : 
                           (IXP400_PARITYENACC_PE_NPE_B == ixNpeId) ?
                               IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT : 
                               IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT))
    {
        return IX_FAIL;
    }
#endif

    /* Enable parity error detection */
    if (IXP400_PARITYENACC_PE_ENABLE == ixNpePDCfg.ideEnabled)
    {
        IXP400_PARITYENACC_VAL_BIT_SET(npePDCfgStatus, npePDCfgFlags);
    } 
    /* Disable parity error detection */
    else
    {
        IXP400_PARITYENACC_VAL_BIT_CLEAR(npePDCfgStatus,npePDCfgFlags);
    } /* end of if */

    /* Odd parity polarity */
    if (IX_PARITYENACC_ODD_PARITY == ixNpePDCfg.parityOddEven)
    {
        IXP400_PARITYENACC_VAL_BIT_SET(npePDCfgStatus,IXP400_PARITYENACC_NPE_CONTROL_PP);
    }
    else
    {
        IXP400_PARITYENACC_VAL_BIT_CLEAR(npePDCfgStatus,IXP400_PARITYENACC_NPE_CONTROL_PP);
    } /* end of if */

    /* Always include these bits for the parity error config changes to take place */
    IXP400_PARITYENACC_VAL_BIT_SET(npePDCfgStatus, npePDCtlFlags);

    /* Set the new configuration */
    IXP400_PARITYENACC_REG_WRITE (
        ixParityENAccNpePEConfig[ixNpeId].npePERegisters.npeControlRegister,
        npePDCfgStatus);

    IXP400_PARITYENACC_REG_READ(
        ixParityENAccNpePEConfig[ixNpeId].npePERegisters.npeControlRegister,
        &npeTmpPDCfgStatus);

    /*
     * These WE bits are read as zeros only so we need to clear them off from the value
     * we have just written so that compare gives correct result.
     */
    IXP400_PARITYENACC_VAL_BIT_CLEAR(npePDCfgStatus, npePDCtlFlags);

    /*
     * The extra bits other than the parity detection control bits/flags, if any, needs
     * to be cleared from the value just read so as to check for the correct parity 
     * detection control bits/flags for correct result.
     *
     * Example: Step 1. Written Value: 3c1c0000
     *          Step 2. ReadE 001c0000 (Expected)
     *          Step 3. ReadA 001d0000 (Actual)
     *          Step 4. ReadE != ReadA
     *          Fix: Step 1. Mask off with 3c00 0000 (as they are read zeros, refer to the
     *                       immediate above step in the code.)
     *               Step 2. Mask off bits other than parity detection control bits/flags
     *                       from the read value.
     *                       001c0000 (= 001d0000 & ~10000) here 10000 extracted using
     *                       (= 001d0000 & ~001c0000).
     *               Step 3. Now we can compare only the parity detection control bits/flags 
     *                       i.e., between written and read control bits.
     */
    {
        UINT32 npeTmpNonPDCfgStatus  = npeTmpPDCfgStatus;
        IXP400_PARITYENACC_VAL_BIT_CLEAR(npeTmpNonPDCfgStatus, npePDCfgStatus);
        IXP400_PARITYENACC_VAL_BIT_CLEAR(npeTmpPDCfgStatus, npeTmpNonPDCfgStatus);
    }

    if (npeTmpPDCfgStatus == npePDCfgStatus)
    {
        /* Enable/Disable the corresponding interrupt at Interrupt Controller */
        if (IXP400_PARITYENACC_PE_ENABLE == ixNpePDCfg.ideEnabled)
        {
            return ixParityENAccIcInterruptEnable( 
                       (IXP400_PARITYENACC_PE_NPE_A == ixNpeId) ? 
                            IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT : 
                            (IXP400_PARITYENACC_PE_NPE_B == ixNpeId) ?
                                IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT : 
                                IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT);
        }
        else
        {
            return ixParityENAccIcInterruptDisable(
                       (IXP400_PARITYENACC_PE_NPE_A == ixNpeId) ? 
                            IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT : 
                            (IXP400_PARITYENACC_PE_NPE_B == ixNpeId) ?
                                IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT : 
                                IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT);
        } /* end of if */
    }
    else
    {
        return IX_FAIL;
    } /* end of if */
} /* end of ixParityENAccNpePEDetectionConfigure() function */


IX_STATUS
ixParityENAccNpePEParityErrorContextFetch (
    IxParityENAccPENpeId ixNpeId,
    IxParityENAccNpePEParityErrorContext *ixNpePECMsg)
{
    /* Validate parameters */
    if ((ixNpeId >= IXP400_PARITYENACC_PE_NPE_MAX) ||
        ((IxParityENAccNpePEParityErrorContext *)NULL == ixNpePECMsg))
    {
        return IX_FAIL;
    } /* end of if */

    /* For NPEs it is always the read access to the IMEM/DMEM  */
    ixNpePECMsg->npeAccessType = IXP400_PARITYENACC_PE_READ;

    /* 
     * Identify parity errors detected from Status Register
     */

    /* Retrieve the raw parity error status */
    ixParityENAccNpePEParityErrorStatusGet();

    /*
     * NPE will be locked-up when any of the parity error is detected
     * Check the parity errors in the following order
     *
     * 1) Instruction Memory 2) Data Memory 3) External Error
     */
    if (IXP400_PARITYENACC_VAL_BIT_CHECK(
            ixParityENAccNpePEConfig[ixNpeId].npeParityErrorStatus.\
                npeStatusRegisterValue, (UINT32) IXP400_PARITYENACC_NPE_STATUS_IMEM_PARITY))
    {
        ixNpePECMsg->npeParitySource = IXP400_PARITYENACC_PE_NPE_IMEM;
        return IX_SUCCESS;
    } /* end of if */

    if (IXP400_PARITYENACC_VAL_BIT_CHECK(
            ixParityENAccNpePEConfig[ixNpeId].npeParityErrorStatus.\
                npeStatusRegisterValue, (UINT32) IXP400_PARITYENACC_NPE_STATUS_DMEM_PARITY))
    {
        ixNpePECMsg->npeParitySource = IXP400_PARITYENACC_PE_NPE_DMEM;
        return IX_SUCCESS;
    } /* end of if */

    if (IXP400_PARITYENACC_VAL_BIT_CHECK(
            ixParityENAccNpePEConfig[ixNpeId].npeParityErrorStatus.\
                npeStatusRegisterValue, (UINT32) IXP400_PARITYENACC_NPE_STATUS_EXT_ERROR))
    {
        ixNpePECMsg->npeParitySource = IXP400_PARITYENACC_PE_NPE_EXT;
        return IX_SUCCESS;
    } /* end of if */

#ifndef NDEBUG
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
        "ixParityENAccNpePEParityErrorContextFetch(): "\
        "Can't fetch parity context of NPE #%u!!!\n", ixNpeId, 0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
    return IX_FAIL;
} /* end of ixParityENAccNpePEParityErrorContextFetch() function */


IX_STATUS
ixParityENAccNpePEParityInterruptClear (IxParityENAccPENpeId ixNpeId)
{
    /* Validate parameters */
    if (ixNpeId >= IXP400_PARITYENACC_PE_NPE_MAX)
    {
        return IX_FAIL;
    } /* end of if */

    /* Disable the interrupt from triggering further */
    return ixParityENAccIcInterruptDisable(
               (IXP400_PARITYENACC_PE_NPE_A == ixNpeId) ? 
                IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT : 
               (IXP400_PARITYENACC_PE_NPE_B == ixNpeId) ?
                IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT : 
                IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT);
} /* end of ixParityENAccNpePEParityInterruptClear() function */

void
ixParityENAccNpePEParityErrorStatusGet (void)
{
    /* Local Variables */
    IxParityENAccPENpeId ixNpeId = IXP400_PARITYENACC_PE_NPE_A;

    /*
     * Read the raw parity error status into local data structure
     * from the control and status registers of all the NPEs
     */
    for (; ixNpeId < IXP400_PARITYENACC_PE_NPE_MAX; ixNpeId++)
    {
        IXP400_PARITYENACC_REG_READ(
            ixParityENAccNpePEConfig[ixNpeId].npePERegisters.npeControlRegister,
            &ixParityENAccNpePEConfig[ixNpeId].npeParityErrorStatus.\
                npeControlRegisterValue);
        IXP400_PARITYENACC_REG_READ(
            ixParityENAccNpePEConfig[ixNpeId].npePERegisters.npeStatusRegister,
            &ixParityENAccNpePEConfig[ixNpeId].npeParityErrorStatus.\
                npeStatusRegisterValue);
    } /* end of for */
} /* end of ixParityENAccNpePEParityErrorStatusGet() function */

/*
 * Local functions definitions.
 */

void
ixParityENAccNpePENpeAIsr(void)
{
    /*
     * No need to read the parity error status in the ISR.
     *
     * Invoke the internal NPE - A callback routine to notify the
     * pairty error detected in IMEM/DMEM/ExtErr.
     * 
     * NOTE: The NPE parity error context information will be 
     * obtained only when the public API client application
     * requests for such information.
     */
    (*ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_A].npePECallback)(
        ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_A].npeIsrInfo.npeInterruptId,
        ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_A].npeIsrInfo.npeIsr);
} /* end of ixParityENAccNpePENpeAIsr() function */

void
ixParityENAccNpePENpeBIsr(void)
{
    /*
     * No need to read the parity error status in the ISR.
     *
     * Invoke the internal NPE - B callback routine to notify the
     * pairty error detected in IMEM/DMEM/ExtErr.
     * 
     * NOTE: The NPE parity error context information will be 
     * obtained only when the public API client application
     * requests for such information.
     */
    (*ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_B].npePECallback)(
        ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_B].npeIsrInfo.npeInterruptId,
        ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_B].npeIsrInfo.npeIsr);

    return;
} /* end of ixParityENAccNpePENpeBIsr() function */

void
ixParityENAccNpePENpeCIsr(void)
{
    /*
     * No need to read the parity error status in the ISR.
     *
     * Invoke the internal NPE - C callback routine to notify the
     * pairty error detected in IMEM/DMEM/ExtErr.
     * 
     * NOTE: The NPE parity error context information will be 
     * obtained only when the public API client application
     * requests for such information.
     */
    (*ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_C].npePECallback)(
        ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_C].npeIsrInfo.npeInterruptId,
        ixParityENAccNpePEConfig[IXP400_PARITYENACC_PE_NPE_C].npeIsrInfo.npeIsr);

    return;
} /* end of ixParityENAccNpePENpeCIsr() function */

#endif /* __ixp46X */
