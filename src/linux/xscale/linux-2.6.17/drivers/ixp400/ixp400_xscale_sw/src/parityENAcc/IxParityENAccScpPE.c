/**
 * @file IxParityENAccSpcPE.c
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Source file for SWCP Parity Detection Enabler sub-component 
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
#include "IxParityENAccScpPE.h"
#include "IxParityENAccScpPE_p.h"

/*
 * NOTE: The Switching Coprocessor hardware block, does not have the 
 * capability to enable/disable the parity error detection. It has the 
 * parity error detection always enabled and thus the enable/disable
 * functionality is implemented in software by the main module inorder
 * to control the notification of the parity error interrupt to XScale.
 */

/*
 * SWCP sub-module level functions definitions
 */

IX_STATUS
ixParityENAccSwcpPEInit (IxParityENAccInternalCallback ixSwcpPECallback)
{
    /* Verify parameters */
    if ((IxParityENAccInternalCallback)NULL == ixSwcpPECallback)
    {
        return IX_FAIL;
    } /* end of if */

    /* Register main module internal callback routine */
    ixParityENAccSwcpPEConfig.swcpPECallback = ixSwcpPECallback;

    /* Interrupt Service Routine Info */
    ixParityENAccSwcpPEConfig.swcpIsrInfo.swcpInterruptId = 
        IRQ_IXP400_INTC_PARITYENACC_SWCP;
    ixParityENAccSwcpPEConfig.swcpIsrInfo.swcpIsr = ixParityENAccSwcpPEIsr;

    /* Install SWCP Interrupt Service Routine */
    {
        INT32 lockKey = ixOsalIrqLock();
        if ((IX_SUCCESS != ixOsalIrqBind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_SWCP,
                                        (IxOsalVoidFnVoidPtr) ixParityENAccSwcpPEIsr,
                                        (void *) NULL)) ||
            (IX_SUCCESS != ixParityENAccIcInterruptDisable(
                            IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT)))
        {
            ixOsalIrqUnlock(lockKey);
            return IX_FAIL;
        } /* end of if */
        ixOsalIrqUnlock(lockKey);
    }

    return IX_SUCCESS;
} /* end of ixParityENAccSwcpPEInit() function */

IX_STATUS
ixParityENAccSwcpPEDetectionConfigure (
    IxParityENAccSwcpPEConfigOption ixSwcpPDCfg)
{
    if (IXP400_PARITYENACC_PE_ENABLE == ixSwcpPDCfg)
    {
        return ixParityENAccIcInterruptEnable(
                   IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT);
    }
    else
    {
        return ixParityENAccIcInterruptDisable(
                   IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT);
    } /* end of if */
} /* end of ixParityENAccSwcpPEDetectionConfigure() function */

IX_STATUS
ixParityENAccSwcpPEParityInterruptClear (void)
{
    /* Disable the interrupt from triggering further */
    return ixParityENAccIcInterruptDisable(
               IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT);
} /* end of ixParityENAccSwcpPEParityInterruptClear() function */

IX_STATUS
ixParityENAccSwcpPEParityErrorContextFetch (
    IxParityENAccSwcpPEParityErrorContext *ixSwcpPECMsg)
{
    if ((IxParityENAccSwcpPEParityErrorContext *)NULL == ixSwcpPECMsg)
    {
        return IX_FAIL;
    } /* end of if */

    /* The read can only detect the parity error in SWCP SRAM */
    ixSwcpPECMsg->swcpParitySource = IXP400_PARITYENACC_PE_SWCP;
    ixSwcpPECMsg->swcpAccessType   = IXP400_PARITYENACC_PE_READ;
    return IX_SUCCESS;
} /* end of ixParityENAccSwcpPEParityErrorContextFetch() function */

void
ixParityENAccSwcpPEIsr (void)
{
    /*
     * XScale can't access the TailPtr register.
     *
     * Invoke the internal SWCP callback routine to notify the pairty error 
     * detected on the SWCP
     * 
     * NOTE: The SWCP parity error context information will be obtained only
     * when the public API client application requests for such information.
     */
    (*ixParityENAccSwcpPEConfig.swcpPECallback)(
        ixParityENAccSwcpPEConfig.swcpIsrInfo.swcpInterruptId,
        ixParityENAccSwcpPEConfig.swcpIsrInfo.swcpIsr);
} /* end of ixParityENAccSwcpPEIsr() function */

#endif /* __ixp46X */
