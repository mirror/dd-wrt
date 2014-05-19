/**
 * @file IxParityENAccEbcPE.c
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Source file for EBC Parity Detection Enabler sub-component 
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
#include "IxParityENAccEbcPE.h"
#include "IxParityENAccEbcPE_p.h"

/* Virtual base address of EBC */
static UINT32 ixEbcVirtualBaseAddr = 0;

/*
 * EBC sub-module level functions definitions
 */

IX_STATUS
ixParityENAccEbcPEInit (IxParityENAccInternalCallback ixEbcPECallback)
{
    UINT32 ebcVirtualBaseAddr = 0;
    IxParityENAccChipSelectId csId = IXP400_PARITYENACC_PE_EBC_CHIPSEL0;
    register IxParityENAccEbcPERegisters *ebcPERegisters = 
                &ixParityENAccEbcPEConfig.ebcPERegisters;
    
    /* Verify parameters */
    if ((IxParityENAccInternalCallback)NULL == ixEbcPECallback)
    {
        return IX_FAIL;
    } /* end of if */

    /* Memory mapping of the EBC registers */
    if ((UINT32)NULL == (ebcVirtualBaseAddr = (UINT32) IX_OSAL_MEM_MAP (
                                               IXP400_PARITYENACC_EBC_BASEADDR,
                                               IXP400_PARITYENACC_EBC_MEMMAP_SIZE)))
    {
        return IX_FAIL;
    } /* end of if */

    ixEbcVirtualBaseAddr = ebcVirtualBaseAddr;
    
    /* Virtual Addresses assignment for EBC Registers */
    ebcPERegisters->expTimingCs[IXP400_PARITYENACC_PE_EBC_CHIPSEL0] = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_TIMING_CS0_OFFSET;
    ebcPERegisters->expTimingCs[IXP400_PARITYENACC_PE_EBC_CHIPSEL1] = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_TIMING_CS1_OFFSET;
    ebcPERegisters->expTimingCs[IXP400_PARITYENACC_PE_EBC_CHIPSEL2] = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_TIMING_CS2_OFFSET;
    ebcPERegisters->expTimingCs[IXP400_PARITYENACC_PE_EBC_CHIPSEL3] = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_TIMING_CS3_OFFSET;
    ebcPERegisters->expTimingCs[IXP400_PARITYENACC_PE_EBC_CHIPSEL4] = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_TIMING_CS4_OFFSET;
    ebcPERegisters->expTimingCs[IXP400_PARITYENACC_PE_EBC_CHIPSEL5] = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_TIMING_CS5_OFFSET;
    ebcPERegisters->expTimingCs[IXP400_PARITYENACC_PE_EBC_CHIPSEL6] = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_TIMING_CS6_OFFSET;
    ebcPERegisters->expTimingCs[IXP400_PARITYENACC_PE_EBC_CHIPSEL7] = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_TIMING_CS7_OFFSET;
    ebcPERegisters->expMstControl = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_MST_CONTROL_OFFSET;
    ebcPERegisters->expParityStatus = 
        ebcVirtualBaseAddr + IXP400_PARITYENACC_EBC_PARITY_STATUS_OFFSET;
    
    /* Register main module internal callback routine */
    ixParityENAccEbcPEConfig.ebcPECallback = ixEbcPECallback;

    /* Interrupt Service Routine Info for debug purpose */
    ixParityENAccEbcPEConfig.ebcIsrInfo.ebcInterruptId = 
        IRQ_IXP400_INTC_PARITYENACC_EBC;
    ixParityENAccEbcPEConfig.ebcIsrInfo.ebcIsr = ixParityENAccEbcPEIsr;

    /* Disable parity error detection on both Inbound & Outbound interfaces */
    for (; csId < IXP400_PARITYENACC_PE_EBC_CHIPSEL_MAX; csId++)
    {
        IXP400_PARITYENACC_REG_BIT_CLEAR(ebcPERegisters->expTimingCs[csId],
            IXP400_PARITYENACC_EBC_TIMING_CSX_PAR_EN);
    } /* end of for */
    IXP400_PARITYENACC_REG_BIT_CLEAR(ebcPERegisters->expMstControl,
        IXP400_PARITYENACC_EBC_MST_CONTROL_INPAR_EN);

    /* Install EBC Interrupt Service Routine */
    {
        INT32 lockKey = ixOsalIrqLock();
        if ((IX_SUCCESS != ixOsalIrqBind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_EBC,
                                        (IxOsalVoidFnVoidPtr) ixParityENAccEbcPEIsr,
                                        (void *) NULL)) ||
            (IX_SUCCESS != ixParityENAccIcInterruptDisable(
                            IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT)))
        {
            ixOsalIrqUnlock(lockKey);
            IX_OSAL_MEM_UNMAP(ebcVirtualBaseAddr);
            return IX_FAIL;
        } /* end of if */
        ixOsalIrqUnlock(lockKey);
    }
    return IX_SUCCESS;
} /* end of ixParityENAccEbcPEInit() function */

IX_STATUS
ixParityENAccEbcPEDetectionConfigure(IxParityENAccEbcPEConfigOption ixEbcPDCfg)
{
    UINT32 ebcPDCfgFlags  = 0;
    UINT32 ebcPDCfgStatus = 0;
    UINT32 ebcTmpPDCfgStatus = 0;
    register IxParityENAccEbcPERegisters *ebcPERegisters = 
                &ixParityENAccEbcPEConfig.ebcPERegisters;

    int loopIdx = 100;

    if (IXP400_PARITYENACC_PE_EBC_CS == ixEbcPDCfg.ebcCsExtSource)
    {
        if (ixEbcPDCfg.ebcCsId >= IXP400_PARITYENACC_PE_EBC_CHIPSEL_MAX)
        {
            return IX_FAIL;
        } /* end of if */

        /* Get current parity detection configuration of Chip Select */
        IXP400_PARITYENACC_REG_READ(
            ebcPERegisters->expTimingCs[ixEbcPDCfg.ebcCsId], &ebcPDCfgStatus);

        /* Enable parity error detection */
        ebcPDCfgFlags  = IXP400_PARITYENACC_EBC_TIMING_CSX_PAR_EN;

        if (IXP400_PARITYENACC_PE_ENABLE == ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled)
        {
            IXP400_PARITYENACC_VAL_BIT_SET(ebcPDCfgStatus, ebcPDCfgFlags);
        } 
        else  /* Disable parity error detection */
        {
            IXP400_PARITYENACC_VAL_BIT_CLEAR(ebcPDCfgStatus, ebcPDCfgFlags);
        } /* end of if */

        while (loopIdx-- && (ebcTmpPDCfgStatus != ebcPDCfgStatus))
        {
            /* Set the new configuration */
            IXP400_PARITYENACC_REG_WRITE(
                ebcPERegisters->expTimingCs[ixEbcPDCfg.ebcCsId], ebcPDCfgStatus);

            /* Configuration successful? */
            IXP400_PARITYENACC_REG_READ(
                ebcPERegisters->expTimingCs[ixEbcPDCfg.ebcCsId],&ebcTmpPDCfgStatus);
        }

        if (ebcTmpPDCfgStatus != ebcPDCfgStatus)
        {
            return IX_FAIL;
        } /* end of if */

        /* 
         * This step required for Even/Odd Parity Type detection from the 
         * EBC Master Control Register if the chip select configuration is
         * specified along with the parity type which is part of the Master
         * Control Register only
         */
        /* Get current parity detection configuration of External Master */
        IXP400_PARITYENACC_REG_READ(ebcPERegisters->expMstControl,&ebcPDCfgStatus);
    } /* else of if */
    else /* EBC Master Control */
    {
        ebcPDCfgFlags  = IXP400_PARITYENACC_EBC_MST_CONTROL_INPAR_EN;

        /* Get current parity detection configuration */
        IXP400_PARITYENACC_REG_READ(ebcPERegisters->expMstControl, &ebcPDCfgStatus);

        /* Enable parity error detection */
        if (IXP400_PARITYENACC_PE_ENABLE == ixEbcPDCfg.ebcInOrOutbound.ebcExtMstEnabled)
        {
            IXP400_PARITYENACC_VAL_BIT_SET(ebcPDCfgStatus,ebcPDCfgFlags);
        } 
        else  /* Disable parity error detection */
        {
            IXP400_PARITYENACC_VAL_BIT_CLEAR(ebcPDCfgStatus,ebcPDCfgFlags);
        } /* end of if */
    } /* end of if */

    /* Set Even/Odd parity type */
    ebcPDCfgFlags = IXP400_PARITYENACC_EBC_MST_CONTROL_ODDPARITY;

    if (IXP400_PARITYENACC_PE_ODD_PARITY == ixEbcPDCfg.parityOddEven)
    {
        IXP400_PARITYENACC_VAL_BIT_SET(ebcPDCfgStatus,ebcPDCfgFlags);
    } 
    else  /* Set even parity */
    {
        IXP400_PARITYENACC_VAL_BIT_CLEAR(ebcPDCfgStatus,ebcPDCfgFlags);
    } /* end of if */

    loopIdx = 10;
    while (loopIdx-- && (ebcTmpPDCfgStatus != ebcPDCfgStatus))
    {
        /* Set the new configuration */
        IXP400_PARITYENACC_REG_WRITE(ebcPERegisters->expMstControl, ebcPDCfgStatus);

        /* Configuration successful? */
        IXP400_PARITYENACC_REG_READ(ebcPERegisters->expMstControl,&ebcTmpPDCfgStatus);
    }

    if (ebcTmpPDCfgStatus == ebcPDCfgStatus)
    {
        /* Enable/Disable the corresponding interrupt at Interrupt Controller */
        return ((IXP400_PARITYENACC_PE_ENABLE == 
                    ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled) ||
                (IXP400_PARITYENACC_PE_ENABLE == 
                    ixEbcPDCfg.ebcInOrOutbound.ebcExtMstEnabled)) ?
                    ixParityENAccIcInterruptEnable( 
                        IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT) :
                    ixParityENAccIcInterruptDisable(
                        IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT);
    }
    else
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixParityENAccEbcPEDetectionConfigure(): returned IX_FAIL\n",
            0,0,0,0,0,0);
        return IX_FAIL;
    } /* end of if */
} /* end of ixParityENAccEbcPEDetectionConfigure() function */

IX_STATUS 
ixParityENAccEbcPEParityErrorContextFetch(
    IxParityENAccEbcPEParityErrorContext *ixEbcPECMsg)
{
    if ((IxParityENAccEbcPEParityErrorContext *)NULL == ixEbcPECMsg)
    {
        return IX_FAIL;
    } /* end of if */

    /* Fetch EBC Parity Error Status */
    ixParityENAccEbcPEParityErrorStatusGet();

    ixEbcPECMsg->ebcParityAddress = IXP400_PARITYENACC_VAL_READ(
        ixParityENAccEbcPEConfig.ebcParityErrorStatus.expParityStatusValue,
        IXP400_PARITYENACC_EBC_PARITY_STATUS_ERRADDR);

    /* Detected Parity Error on Inbound Write by an External Master? */
    if (IXP400_PARITYENACC_VAL_BIT_CHECK(
            ixParityENAccEbcPEConfig.ebcParityErrorStatus.expParityStatusValue,
            IXP400_PARITYENACC_EBC_PARITY_STATUS_INERRSTS))
    {
        ixEbcPECMsg->ebcParitySource = IXP400_PARITYENACC_PE_EBC_EXTMST;
        ixEbcPECMsg->ebcAccessType   = IXP400_PARITYENACC_PE_WRITE;
        return IX_SUCCESS;
    } /* end of if */

    /* Detected Parity Error on Outbound Read by the Expansion Bus Controller? */
    if (IXP400_PARITYENACC_VAL_BIT_CHECK(
            ixParityENAccEbcPEConfig.ebcParityErrorStatus.expParityStatusValue,
            IXP400_PARITYENACC_EBC_PARITY_STATUS_OUTERRSTS))
    {
        ixEbcPECMsg->ebcParitySource = IXP400_PARITYENACC_PE_EBC_CS;
        ixEbcPECMsg->ebcAccessType   = IXP400_PARITYENACC_PE_READ;
        return IX_SUCCESS;
    } /* end of if */

#ifndef NDEBUG 
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
        "ixParityENAccEbcPEParityErrorContextFetch(): "
        "Can't fetch parity context of EBC #%u!!!\n", 0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
    return IX_FAIL;
} /* end of ixParityENAccEbcPEParityErrorContextFetch() function */

IX_STATUS
ixParityENAccEbcPEParityInterruptClear (
    IxParityENAccEbcPEParityConfigSource ixEbcParityErrSrc)
{
    /* Clear off Parity Error due to Inbound Write by an External Master? */
    if (IXP400_PARITYENACC_PE_EBC_EXTMST == ixEbcParityErrSrc)
    {
        /* Write '1' to clear off the InBound Parity Error Status */
        IXP400_PARITYENACC_REG_BIT_SET(
            ixParityENAccEbcPEConfig.ebcPERegisters.expParityStatus,
            IXP400_PARITYENACC_EBC_PARITY_STATUS_INERRSTS);

        return IX_SUCCESS;
    } /* end of if */

    /* Clear off Parity Error due to Outbound Read by EBC? */
    if (IXP400_PARITYENACC_PE_EBC_CS == ixEbcParityErrSrc)
    {
        /* Write '1' to clear off the Outbound Parity Error Status */
        IXP400_PARITYENACC_REG_BIT_SET(
            ixParityENAccEbcPEConfig.ebcPERegisters.expParityStatus,
            IXP400_PARITYENACC_EBC_PARITY_STATUS_OUTERRSTS);

        return IX_SUCCESS;
    } /* end of if */

    return IX_FAIL;
} /* end of ixParityENAccEbcPEParityInterruptClear() function */

/*
 * Local functions definitions
 */

void
ixParityENAccEbcPEIsr(void)
{
    /*
     * No need to read parity error status from the Parity Status Register
     *
     * Invoke the internal EBC callback routine to notify the pairty error 
     * detected on the Inbound or Outbound Interface
     * 
     * NOTE: The EBC parity error context information will be obtained only 
     * when the public API client application requests for such information.
     */
    (*ixParityENAccEbcPEConfig.ebcPECallback)(
        ixParityENAccEbcPEConfig.ebcIsrInfo.ebcInterruptId,
        ixParityENAccEbcPEConfig.ebcIsrInfo.ebcIsr);
} /* end of ixParityENAccEbcPEIsr() function */

void
ixParityENAccEbcPEParityErrorStatusGet (void)
{
    IxParityENAccChipSelectId csId = IXP400_PARITYENACC_PE_EBC_CHIPSEL0;
    register IxParityENAccEbcPERegisters *ebcPERegisters = 
                &ixParityENAccEbcPEConfig.ebcPERegisters;
    register IxParityENAccEbcPEParityErrorStatus *ebcPEParityStatus = 
                &ixParityENAccEbcPEConfig.ebcParityErrorStatus;

    /*
     * Read the raw parity error status into local data structure from 
     * the Master, Timing & Control and Status registers of the EBC
     */
    /* EBC Master Control Register contents */
    IXP400_PARITYENACC_REG_READ(ebcPERegisters->expMstControl,
        &ebcPEParityStatus->expMstControlValue);

    /* EBC Parity Status Register contents */
    IXP400_PARITYENACC_REG_READ(ebcPERegisters->expParityStatus,
        &ebcPEParityStatus->expParityStatusValue);

    for (; csId < IXP400_PARITYENACC_PE_EBC_CHIPSEL_MAX; csId++)
    {
        /* EBC Timing and Control Register of Chip Select #n contents */
        IXP400_PARITYENACC_REG_READ(ebcPERegisters->expTimingCs[csId],
            &ebcPEParityStatus->expTimingCsValue[csId]);
    } /* end of for */
} /* end of ixParityENAccEbcPEParityErrorStatusGet() function */

IX_STATUS 
ixParityENAccEbcPEUnload(void)
{
    UINT32 lockKey;
    UINT32 status = IX_SUCCESS;
    IxParityENAccChipSelectId  ebcCsId;
    IxParityENAccEbcPEConfigOption ixEbcPDCfg;

    /* Disable expansion Bus Controller erros with chip-select option */
    for( ebcCsId = IXP400_PARITYENACC_PE_EBC_CHIPSEL0; 
         ebcCsId < IXP400_PARITYENACC_PE_EBC_CHIPSEL_MAX;
	 ebcCsId++)
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_CS;
	ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled = IXP400_PARITYENACC_PE_DISABLE;
	ixEbcPDCfg.ebcCsId = ebcCsId;
	ixEbcPDCfg.parityOddEven = IX_PARITYENACC_EVEN_PARITY;
	ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg);
    }
    
    /* Disable expansion bus controller parity errors with EXT Master option */
    ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_EXTMST;
    ixEbcPDCfg.ebcInOrOutbound.ebcExtMstEnabled = IXP400_PARITYENACC_PE_DISABLE;
    ixEbcPDCfg.ebcCsId = IX_PARITYENACC_EVEN_PARITY;
    ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg);

    /* Unbind the IRQ */    
    lockKey = ixOsalIrqLock();
    if (IX_SUCCESS != ixOsalIrqUnbind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_EBC))
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
            "ixParityENAccEbcPEUnload(): "\
            "Can't unbind the EBC ISR to IRQ_IXP400_INTC_PARITYENACC_EBC!!!\n",0,0,0,0,0,0);
	status = IX_FAIL;
    }
    ixOsalIrqUnlock(lockKey);

    /* Unmap the memory */
    IX_OSAL_MEM_UNMAP(ixEbcVirtualBaseAddr);
    
    return status;
} /* end of ixParityENAccEbcPEUnload() function */   

#endif /* __ixp46X || __ixp43X */
