/**
 * @file IxParityENAccPbcPE.c
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Source file for PBC Parity Detection Enabler sub-component 
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
#include "IxParityENAccPbcPE.h"
#include "IxParityENAccPbcPE_p.h"

/* Virtual base address of PBC */
static UINT32 ixPbcVirtualBaseAddr = 0;

/*
 * PBC sub-module level functions definitions
 */

IX_STATUS
ixParityENAccPbcPEInit(IxParityENAccInternalCallback ixPbcPECallback)
{
    UINT32 pbcVirtualBaseAddr = 0;
    register IxParityENAccPbcPERegisters *pbcPERegisters =
        &ixParityENAccPbcPEConfig.pbcPERegisters;
    
    /* Verify parameters */
    if ((IxParityENAccInternalCallback)NULL == ixPbcPECallback)
    {
        return IX_FAIL;
    } /* end of if */

    /* Memory mapping of the PBC registers */
    if ((UINT32)NULL == (pbcVirtualBaseAddr = (UINT32) IX_OSAL_MEM_MAP (
                                              IXP400_PARITYENACC_PBC_PCICSR_BASEADDR,
                                              IXP400_PARITYENACC_PBC_PCICSR_MEMMAP_SIZE)))
    {
        return IX_FAIL;
    } /* end of if */

    ixPbcVirtualBaseAddr = pbcVirtualBaseAddr;

    /* Virtual Addresses assignment for PBC Control and Status Registers */
    pbcPERegisters->pciCrpAdCbe = 
        pbcVirtualBaseAddr + IXP400_PARITYENACC_PBC_CRP_AD_CBE_OFFSET;
    pbcPERegisters->pciCrpWdata = 
        pbcVirtualBaseAddr + IXP400_PARITYENACC_PBC_CRP_WDATA_OFFSET;
    pbcPERegisters->pciCrpRdata = 
        pbcVirtualBaseAddr + IXP400_PARITYENACC_PBC_CRP_RDATA_OFFSET;
    pbcPERegisters->pciCsr = 
        pbcVirtualBaseAddr + IXP400_PARITYENACC_PBC_CSR_OFFSET;
    pbcPERegisters->pciIsr = 
        pbcVirtualBaseAddr + IXP400_PARITYENACC_PBC_ISR_OFFSET;
    pbcPERegisters->pciInten = 
        pbcVirtualBaseAddr + IXP400_PARITYENACC_PBC_INTEN_OFFSET;

    /* Register main module internal callback routine */
    ixParityENAccPbcPEConfig.pbcPECallback = ixPbcPECallback;

    /* Interrupt Service Routine Info for debug purpose */
    ixParityENAccPbcPEConfig.pbcIsrInfo.pbcInterruptId = 
        IRQ_IXP400_INTC_PARITYENACC_PBC;
    ixParityENAccPbcPEConfig.pbcIsrInfo.pbcIsr = ixParityENAccPbcPEIsr;

    /* Disable parity error detection */

    /* Write '1' to clear-off the PPE bit */
    IXP400_PARITYENACC_REG_BIT_SET(
        pbcPERegisters->pciIsr, IXP400_PARITYENACC_PBC_ISR_PPE);

    IXP400_PARITYENACC_REG_BIT_CLEAR(
        pbcPERegisters->pciInten, IXP400_PARITYENACC_PBC_INTEN_PPE);

    /* Install PBC Interrupt Service Routine */
    {
        INT32 lockKey = ixOsalIrqLock();
        if ((IX_SUCCESS != ixOsalIrqBind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_PBC,
                                        (IxOsalVoidFnVoidPtr) ixParityENAccPbcPEIsr,
                                        (void *) NULL)) ||
            (IX_FAIL == ixParityENAccIcInterruptDisable(
                        IXP400_PARITYENACC_INTC_PBC_PARITY_INTERRUPT)))
        {
            ixOsalIrqUnlock(lockKey);
            IX_OSAL_MEM_UNMAP(pbcVirtualBaseAddr);
            return IX_FAIL;
        } /* end of if */
        ixOsalIrqUnlock(lockKey);
    }

    return IX_SUCCESS;
} /* end of ixParityENAccPbcPEInit() function */

IX_STATUS
ixParityENAccPbcPEDetectionConfigure(IxParityENAccPbcPEConfigOption ixPbcPDCfg)
{
    UINT32 pbcPDCfgStatus = 0;
    UINT32 pbcTmpPDCfgStatus = 0;
    int    loopIdx = 0;

    /* Read the PCI Controller PCI Config SRCR register */
    IXP400_PARITYENACC_REG_WRITE(
        ixParityENAccPbcPEConfig.pbcPERegisters.pciCrpAdCbe,
        IXP400_PARITYENACC_PBC_PCICSR_SRCR_READ);
    IXP400_PARITYENACC_REG_READ(
        ixParityENAccPbcPEConfig.pbcPERegisters.pciCrpRdata,
        &pbcPDCfgStatus);

    /* 
     * Set/Clear the PER bit of SRCR register & 
     * Enable/Disable Parity Error Notification
     */
    if (IXP400_PARITYENACC_PE_ENABLE == ixPbcPDCfg)
    {
        /* Set the PER bit of SRCR register */
        IXP400_PARITYENACC_VAL_BIT_SET(pbcPDCfgStatus, 
            IXP400_PARITYENACC_PBC_PCICFG_SRCR_PER);

        /* Enable the PCI Parity Error Interrupt Notification */
        IXP400_PARITYENACC_REG_BIT_SET(
            ixParityENAccPbcPEConfig.pbcPERegisters.pciInten,
            IXP400_PARITYENACC_PBC_INTEN_PPE);
    } 
    /* else of if */
    else
    {
        /* Clear the PER bit of SRCR register */
        IXP400_PARITYENACC_VAL_BIT_CLEAR(pbcPDCfgStatus,
            IXP400_PARITYENACC_PBC_PCICFG_SRCR_PER);

        /* Disable the PCI Parity Error Interrupt Notification */
        IXP400_PARITYENACC_REG_BIT_CLEAR(
            ixParityENAccPbcPEConfig.pbcPERegisters.pciInten,
            IXP400_PARITYENACC_PBC_INTEN_PPE);
    } /* end of if */

    /* Write back the PCI Controller PCI Config SRCR register */
    IXP400_PARITYENACC_REG_WRITE(
        ixParityENAccPbcPEConfig.pbcPERegisters.pciCrpAdCbe,
        IXP400_PARITYENACC_PBC_PCICSR_SRCR_WRITE);
    IXP400_PARITYENACC_REG_WRITE(
        ixParityENAccPbcPEConfig.pbcPERegisters.pciCrpWdata,
        pbcPDCfgStatus);

    loopIdx = 10;
    while (loopIdx--)
    {
        /* Verify that the configuration is successful or not */
        IXP400_PARITYENACC_REG_WRITE(
            ixParityENAccPbcPEConfig.pbcPERegisters.pciCrpAdCbe,
            IXP400_PARITYENACC_PBC_PCICSR_SRCR_READ);
        IXP400_PARITYENACC_REG_READ(
            ixParityENAccPbcPEConfig.pbcPERegisters.pciCrpRdata,
            &pbcTmpPDCfgStatus);
    }

    if (TRUE == IXP400_PARITYENACC_VAL_BIT_CHECK(pbcPDCfgStatus, pbcTmpPDCfgStatus))
    {
        /* Enable/Disable the corresponding interrupt at Interrupt Controller */
        return (IXP400_PARITYENACC_PE_ENABLE == ixPbcPDCfg) ?
                    ixParityENAccIcInterruptEnable(
                        IXP400_PARITYENACC_INTC_PBC_PARITY_INTERRUPT) :
                    ixParityENAccIcInterruptDisable(
                        IXP400_PARITYENACC_INTC_PBC_PARITY_INTERRUPT);
    }
    else
    {
        return IX_FAIL;
    } /* end of if */
} /* end of ixParityENAccPbcPEDetectionConfigure() function */


IX_STATUS
ixParityENAccPbcPEParityErrorContextFetch(
    IxParityENAccPbcPEParityErrorContext *ixPbcPECMsg)
{
    BOOL isrPPE   = FALSE;
    BOOL srcrDPE  = FALSE;
    BOOL srcrMDPE = FALSE;

    if ((IxParityENAccPbcPEParityErrorContext *)NULL == ixPbcPECMsg)
    {
        return IX_FAIL;
    } /* end of if */

    /* Fetch PBC Parity Error Status into local data structures*/
    ixParityENAccPbcPEParityErrorStatusGet();

    /*
     * PCI Controller Initiator/Target Interface and Parity Errors detection
     * during Read / Write Transactions show in the following decision table 
     *
     *             +--------------------+--------------------+
     *             |    PCI Initiator   |     PCI Target     |
     *             +----------+---------+----------+---------+
     *             |    Read  |  Write  |   Read   |  Write  |
     *             +----------+---------+----------+---------+
     *   isr.PPE   |     1    |    1    |     -    |    1    |
     *             +----------+---------+----------+---------+
     *  srcr.DPE   |     1    |    0    |     -    |    1    |
     *             +----------+---------+----------+---------+
     *  srcr.MDPE  |     1    |    1    |     -    |    0    |
     *             +----------+---------+----------+---------+
     *
     * While:
     *        -   Error handling is left to initiator of transaction
     *        1   Bit in the register (row heading-reg.BIT) is SET
     *        0   Bit in the register is CLEAR
     *        x   Don't Care
     */

    isrPPE   = IXP400_PARITYENACC_VAL_BIT_CHECK(
                   ixParityENAccPbcPEConfig.pbcParityErrorStatus.pciIsrValue,
                   (UINT32) IXP400_PARITYENACC_PBC_ISR_PPE);

    srcrDPE  = IXP400_PARITYENACC_VAL_BIT_CHECK(
                   ixParityENAccPbcPEConfig.pbcParityErrorStatus.pciSrcrValue,
                   (UINT32) IXP400_PARITYENACC_PBC_PCICFG_SRCR_DPE);

    srcrMDPE = IXP400_PARITYENACC_VAL_BIT_CHECK(
                   ixParityENAccPbcPEConfig.pbcParityErrorStatus.pciSrcrValue,
                   (UINT32) IXP400_PARITYENACC_PBC_PCICFG_SRCR_MDPE);

    /* Is Parity Error Detected ? */
    if (TRUE == isrPPE)
    {
        /* Is due to PCI Initiator Transaction ? */
        if (TRUE == srcrMDPE)
        {
            /* Is due to PCI Initiator Read Transaction ? */
            if (TRUE == srcrDPE)
            {
                ixPbcPECMsg->pbcParitySource = IXP400_PARITYENACC_PE_PBC_INITIATOR;
                ixPbcPECMsg->pbcAccessType   = IXP400_PARITYENACC_PE_READ;
                return IX_SUCCESS;
            } /* else of if */
            /* Is due to PCI Initiator Write Transaction ? */
            else
            {
                ixPbcPECMsg->pbcParitySource = IXP400_PARITYENACC_PE_PBC_INITIATOR;
                ixPbcPECMsg->pbcAccessType   = IXP400_PARITYENACC_PE_WRITE;
                return IX_SUCCESS;
            } /* end of if */            
        } /* end of if */

        /* Is due to PCI Target Write Transaction ? */
        if (TRUE == srcrDPE)
        {
            ixPbcPECMsg->pbcParitySource = IXP400_PARITYENACC_PE_PBC_TARGET;
            ixPbcPECMsg->pbcAccessType   = IXP400_PARITYENACC_PE_WRITE;
            return IX_SUCCESS;
        } /* end of if */

        /* Is due to PCI Target Read Transaction? Possibly */
        ixPbcPECMsg->pbcParitySource = IXP400_PARITYENACC_PE_PBC_TARGET;
        ixPbcPECMsg->pbcAccessType   = IXP400_PARITYENACC_PE_READ;
        return IX_SUCCESS;
    } /* end of if */

#ifndef NDEBUG
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
        "ixParityENAccPbcPEParityErrorContextFetch(): "
        "Can't fetch parity context of PBC !!!\n", 0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */

    return IX_FAIL;
} /* end of function */

IX_STATUS
ixParityENAccPbcPEParityInterruptClear (
    IxParityENAccPbcPEParityErrorContext ixPbcPECMsg)
{
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "\nixParityENAccPbcPEParityInterruptClear():"
        "\n\tpbcParitySource:%x\tpbcAccessType:%x\n", 
        ixPbcPECMsg.pbcParitySource,ixPbcPECMsg.pbcAccessType,0,0,0,0);
        	
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_DEBUG2, IX_OSAL_LOG_DEV_STDOUT,
        "ixParityENAccPbcPEParityInterruptClear(): "
        "\nIXP400_PARITYENACC_PE_PBC_INITIATOR:%x"
        "\tIXP400_PARITYENACC_PE_READ:%x\n", 
        (IXP400_PARITYENACC_PE_PBC_INITIATOR),(IXP400_PARITYENACC_PE_READ),0,0,0,0);
        	
    /* Clear off Parity Error Status due to PCI Initiator Read? */
    if (IXP400_PARITYENACC_PE_PBC_INITIATOR == ixPbcPECMsg.pbcParitySource)
    {
        /* Write '1' to clear off the srcr.MDPE and srcr.DPE */
        IXP400_PARITYENACC_VAL_BIT_SET(
            ixParityENAccPbcPEConfig.pbcParityErrorStatus.pciSrcrValue,
            (IXP400_PARITYENACC_PBC_PCICFG_SRCR_MDPE | 
            IXP400_PARITYENACC_PBC_PCICFG_SRCR_DPE) );
    } /* end of if */

    /* Clear off Parity Error Status due to PCI Target Write? */
    if (IXP400_PARITYENACC_PE_PBC_TARGET == ixPbcPECMsg.pbcParitySource)
    {
        /* Write '1' to clear off the srcr.DPE */
        IXP400_PARITYENACC_VAL_BIT_SET(
            ixParityENAccPbcPEConfig.pbcParityErrorStatus.pciSrcrValue,
            IXP400_PARITYENACC_PBC_PCICFG_SRCR_DPE);
    } /* end of if */

    /* Clear parity error interrupt status */
    ixParityENAccPbcPEParityErrorStatusClear();

    return IX_SUCCESS;
} /* end of ixParityENAccPbcPEParityInterruptClear() function */


/*
 * Local functions definitions
 */

void 
ixParityENAccPbcPEIsr(void)
{
    /*
     * No need to read parity error status from the Parity Status Register
     *
     * Invoke the internal PBC callback routine to notify the parity error 
     * detected on the PCI Initiator or PCI Target Interface
     * 
     * NOTE: The PBC parity error context information will be obtained only 
     * when the public API client application requests for such information.
     */
    (*ixParityENAccPbcPEConfig.pbcPECallback)(
        ixParityENAccPbcPEConfig.pbcIsrInfo.pbcInterruptId,
        ixParityENAccPbcPEConfig.pbcIsrInfo.pbcIsr);
} /* end of ixParityENAccPbcPEIsr() function */

/* Get parity error status into internal datastructures */
void
ixParityENAccPbcPEParityErrorStatusGet (void)
{
    register IxParityENAccPbcPERegisters *pbcPERegisters =
        &ixParityENAccPbcPEConfig.pbcPERegisters;
    register IxParityENAccPbcPEParityErrorStatus *pbcPEErrSts =
        &ixParityENAccPbcPEConfig.pbcParityErrorStatus;

    /* Read PCI Controller Interrupt Enable Register contents */
    IXP400_PARITYENACC_REG_READ(pbcPERegisters->pciInten, &pbcPEErrSts->pciIntenValue);

    /* Read PCI Controller Interrupt Status Register contents */
    IXP400_PARITYENACC_REG_READ(pbcPERegisters->pciIsr, &pbcPEErrSts->pciIsrValue);

    /* Read PCI Controller Control and Status Register contents */
    IXP400_PARITYENACC_REG_READ(pbcPERegisters->pciCsr, &pbcPEErrSts->pciCsrValue);

    /* Read the PCI Controller PCI Config SRCR register */
    IXP400_PARITYENACC_REG_WRITE(pbcPERegisters->pciCrpAdCbe,
        IXP400_PARITYENACC_PBC_PCICSR_SRCR_READ);
    IXP400_PARITYENACC_REG_READ(pbcPERegisters->pciCrpRdata, &pbcPEErrSts->pciSrcrValue);
} /* end of ixParityENAccPbcPEParityErrorStatusGet() function */

/* Set parity error interrupt status to clear */
void 
ixParityENAccPbcPEParityErrorStatusClear (void)
{
    /* Update the PCI Controller PCI Config SRCR register */
   IXP400_PARITYENACC_REG_WRITE(ixParityENAccPbcPEConfig.pbcPERegisters.pciCrpAdCbe,
        IXP400_PARITYENACC_PBC_PCICSR_SRCR_WRITE);    
    IXP400_PARITYENACC_REG_WRITE(ixParityENAccPbcPEConfig.pbcPERegisters.pciCrpWdata,
        ixParityENAccPbcPEConfig.pbcParityErrorStatus.pciSrcrValue);
 
    /* Clear off Parity Error Interrupt Status by writing '1' onto PPE bit */
    IXP400_PARITYENACC_REG_BIT_SET(ixParityENAccPbcPEConfig.pbcPERegisters.pciIsr,
        IXP400_PARITYENACC_PBC_ISR_PPE);
} /* end of ixParityENAccPbcPEParityErrorStatusClear() function */

IX_STATUS 
ixParityENAccPbcPEUnload(void)
{
    UINT32 lockKey;
    UINT32 status = IX_SUCCESS;
    IxParityENAccPEConfigOption ixPbcPDCfg;

    ixPbcPDCfg = IXP400_PARITYENACC_PE_DISABLE;
    ixParityENAccPbcPEDetectionConfigure(ixPbcPDCfg);

    /* Unbind the IRQ */    
    lockKey = ixOsalIrqLock();
    if (IX_SUCCESS != ixOsalIrqUnbind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_PBC))
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
            "ixParityENAccPbcPEUnload(): "\
            "Can't unbind the PBC ISR to IRQ_IXP400_INTC_PARITYENACC_PBC!!!\n",0,0,0,0,0,0);
        status = IX_FAIL;
    }    
    ixOsalIrqUnlock(lockKey);

    /* Unmap the memory */
    IX_OSAL_MEM_UNMAP(ixPbcVirtualBaseAddr);

    return status;
} /* end of ixParityENAccPbcPEUnload() function */

#endif /* __ixp46X || __ixp43X */
