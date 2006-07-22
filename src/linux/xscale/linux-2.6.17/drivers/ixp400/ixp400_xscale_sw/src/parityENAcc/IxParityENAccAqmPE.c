/**
 * @file IxParityENAccAqmPE.c
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Source file for AQM Parity Detection Enabler sub-component 
 * of the IXP400 Parity Error Notifier access component.
 *
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

#ifdef __ixp46X

/* 
 * System defined include files
 */
#include "IxOsal.h"

/*
 * User defined include files
 */
#include "IxParityENAccIcE.h"
#include "IxParityENAccAqmPE.h"
#include "IxParityENAccAqmPE_p.h"

void
ixParityENAccAqmPERegDump(void);

/*
 * AQM sub-module level functions definitions
 */

IX_STATUS
ixParityENAccAqmPEInit (IxParityENAccInternalCallback ixAqmPECallback)
{
    UINT32 aqmVirtualBaseAddr = 0;

    /* Verify parameters */
    if ((IxParityENAccInternalCallback)NULL == ixAqmPECallback)
    {
        return IX_FAIL;
    } /* end of if */

    /* Memory mapping of the AQM registers */
    aqmVirtualBaseAddr = (UINT32) IX_OSAL_MEM_MAP (
                                      IXP400_PARITYENACC_AQM_BASEADDR,
                                      IXP400_PARITYENACC_AQM_MEMMAP_SIZE);
    if ((UINT32)NULL == aqmVirtualBaseAddr)
    {
        return IX_FAIL;
    } /* end of if */

    /* Virtual Addresses assignment for AQM Registers */
    ixParityENAccAqmPEConfig.aqmPERegisters.aqmQueAddErr  = 
        aqmVirtualBaseAddr + IXP400_PARITYENACC_AQM_QUEADDRERR_OFFSET;
    ixParityENAccAqmPEConfig.aqmPERegisters.aqmQueDataErr  = 
        aqmVirtualBaseAddr + IXP400_PARITYENACC_AQM_QUEDATAERR_OFFSET;

    /* Register main module internal callback routine for AQM */
    ixParityENAccAqmPEConfig.aqmPECallback = ixAqmPECallback;

    /* Interrupt Service Routine Info for AQM for debug purpose */
    ixParityENAccAqmPEConfig.aqmIsrInfo.aqmInterruptId = 
        IRQ_IXP400_INTC_PARITYENACC_AQM;
    ixParityENAccAqmPEConfig.aqmIsrInfo.aqmIsr = ixParityENAccAqmPEIsr;

    /* Disable parity error detection */
    IXP400_PARITYENACC_REG_BIT_CLEAR(
        ixParityENAccAqmPEConfig.aqmPERegisters.aqmQueAddErr,
            IXP400_PARITYENACC_AQM_QUEADDRERR_PERR_ENABLE |
            IXP400_PARITYENACC_AQM_QUEADDRERR_PERR_FLAG);

    /* Install AQM Interrupt Service Routine */
    {
        INT32 lockKey = ixOsalIrqLock();
        if ((IX_SUCCESS != ixOsalIrqBind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_AQM,
                                        (IxOsalVoidFnVoidPtr) ixParityENAccAqmPEIsr,
                                        (void *) NULL)) ||
            (IX_SUCCESS != ixParityENAccIcInterruptDisable(
                            IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT)))
        {
            ixOsalIrqUnlock(lockKey);
            IX_OSAL_MEM_UNMAP(aqmVirtualBaseAddr);
            return IX_FAIL;
        } /* end of if */
        ixOsalIrqUnlock(lockKey);
    }

    return IX_SUCCESS;
} /* end of ixParityENAccAqmPEInit() function */

IX_STATUS
ixParityENAccAqmPEDetectionConfigure
    (IxParityENAccAqmPEConfigOption ixAqmPDCfg)
{
    UINT32 aqmPDCfgFlags  = IXP400_PARITYENACC_AQM_QUEADDRERR_PERR_ENABLE;
    UINT32 aqmPDCfgStatus = 0;
    UINT32 aqmTmpPDCfgStatus = 0;

    /* Enable parity error detection */
    if (IXP400_PARITYENACC_PE_ENABLE == ixAqmPDCfg)
    {
        IXP400_PARITYENACC_VAL_BIT_SET(aqmPDCfgStatus, aqmPDCfgFlags);
    } 
    else  /* Disable parity error detection */
    {
        IXP400_PARITYENACC_VAL_BIT_CLEAR(aqmPDCfgStatus, aqmPDCfgFlags);
    } /* end of if */

    /*
     * The following sequence of steps works without the following while loop on Emulator
     * but doesn't work on BMP
     */

    while (TRUE != IXP400_PARITYENACC_VAL_BIT_CHECK(aqmTmpPDCfgStatus, aqmPDCfgStatus))
    {
        /* Set the new configuration */
        IXP400_PARITYENACC_REG_WRITE (
            ixParityENAccAqmPEConfig.aqmPERegisters.aqmQueAddErr, aqmPDCfgStatus);

        /* Verify that the configuration is successful or not */
        IXP400_PARITYENACC_REG_READ(
            ixParityENAccAqmPEConfig.aqmPERegisters.aqmQueAddErr,&aqmTmpPDCfgStatus);
    }

    if (TRUE == IXP400_PARITYENACC_VAL_BIT_CHECK(aqmTmpPDCfgStatus, aqmPDCfgStatus))
    {
        return (IXP400_PARITYENACC_PE_ENABLE == ixAqmPDCfg) ? 
                    ixParityENAccIcInterruptEnable(
                        IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT) :
                    ixParityENAccIcInterruptDisable(
                        IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT);
    }
    else
    {
        return IX_FAIL;
    } /* end of if */
} /* end of ixParityENAccAqmPEDetectionConfigure() function */

IX_STATUS
ixParityENAccAqmPEParityErrorContextFetch
    (IxParityENAccAqmPEParityErrorContext *ixAqmPECMsg)
{
    /* Validate parameters */
    if ((IxParityENAccAqmPEParityErrorContext *)NULL == ixAqmPECMsg)
    {
        return IX_FAIL;
    } /* end of if */

    /* For AQM it is always the read access to the SRAM */
    ixAqmPECMsg->aqmAccessType = IXP400_PARITYENACC_PE_READ;

    /* AQM as parity error source */
    ixAqmPECMsg->aqmParitySource = IXP400_PARITYENACC_PE_AQM;

    /*
     * Read the raw parity error status into local data structure
     * from the Address/Control and Data registers of the AQM
     */
    IXP400_PARITYENACC_REG_READ(
        ixParityENAccAqmPEConfig.aqmPERegisters.aqmQueAddErr,
        &ixParityENAccAqmPEConfig.aqmParityErrorStatus.aqmQueAddErrValue);
    IXP400_PARITYENACC_REG_READ(
        ixParityENAccAqmPEConfig.aqmPERegisters.aqmQueDataErr,
        &ixParityENAccAqmPEConfig.aqmParityErrorStatus.aqmQueDataErrValue);

    /* Parity error address and data */
    ixAqmPECMsg->aqmParityAddress = 
        IXP400_PARITYENACC_VAL_READ(
            ixParityENAccAqmPEConfig.aqmParityErrorStatus.aqmQueAddErrValue,
            IXP400_PARITYENACC_AQM_QUEADDRERR_PERR_ADDRESS);
    ixAqmPECMsg->aqmParityData = 
            ixParityENAccAqmPEConfig.aqmParityErrorStatus.aqmQueDataErrValue;

    return IX_SUCCESS;
} /* end of ixParityENAccAqmPEParityErrorContextFetch() function */

IX_STATUS
ixParityENAccAqmPEParityInterruptClear (void)
{
    /* Clear off parity error details */
    IXP400_PARITYENACC_REG_BIT_CLEAR(
        ixParityENAccAqmPEConfig.aqmPERegisters.aqmQueAddErr,
            IXP400_PARITYENACC_AQM_QUEADDRERR_PERR_FLAG);

    /* Disable the interrupt from triggering further */
    return ixParityENAccIcInterruptDisable(
               IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT);

} /* end of ixParityENAccAqmPEParityInterruptClear() function */

void
ixParityENAccAqmPEIsr(void)
{
    /*
     * No need to read parity error status in the Address and Control Register
     *
     * Invoke the internal AQM callback routine to notify the pairty error 
     * detected in SRAM.
     * 
     * NOTE: The AQM parity error context information will be obtained only 
     * when the public API client application requests for such information.
     */
    (*ixParityENAccAqmPEConfig.aqmPECallback)(
        ixParityENAccAqmPEConfig.aqmIsrInfo.aqmInterruptId,
        ixParityENAccAqmPEConfig.aqmIsrInfo.aqmIsr);
} /* end of ixParityENAccAqmPEIsr() function */

#endif /* __ixp46X */
