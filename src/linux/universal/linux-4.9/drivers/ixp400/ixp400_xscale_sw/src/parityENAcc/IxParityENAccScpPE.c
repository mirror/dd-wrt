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

IX_STATUS 
ixParityENAccSwcpPEUnload(void)
{
    UINT32 lockKey;
    UINT32 status = IX_SUCCESS;
    IxParityENAccSwcpPEConfigOption ixSwcpPDCfg;
    
    ixSwcpPDCfg = IXP400_PARITYENACC_PE_DISABLE;
    ixParityENAccSwcpPEDetectionConfigure(ixSwcpPDCfg);

    /* Unbind the IRQ */    
    lockKey = ixOsalIrqLock();
    if (IX_SUCCESS != ixOsalIrqUnbind ((UINT32) IRQ_IXP400_INTC_PARITYENACC_SWCP))
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
            "ixParityENAccSwcpPEUnload(): "\
            "Can't unbind the SWCP ISR to IRQ_IXP400_INTC_PARITYENACC_SWCP!!!\n",0,0,0,0,0,0);
        status = IX_FAIL;
    }    
    ixOsalIrqUnlock(lockKey);

    return status;
} /* end of ixParityENAccSwcpPEUnload() function */

#endif /* __ixp46X || __ixp43X */
