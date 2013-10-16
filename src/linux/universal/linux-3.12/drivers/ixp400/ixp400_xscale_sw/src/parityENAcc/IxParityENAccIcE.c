/**
 * @file IxParityENAccIcE.c
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Source file for Interrupt Controller Enabler sub-component of the 
 * IXP400 Parity Error Notifier access component.
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
#include "IxParityENAccIcE_p.h"
#include "IxFeatureCtrl.h"
/* Virtual base address of INT */
static UINT32 ixIntcVirtualBaseAddr = 0;

/*
 * Interrupt Controller sub-module level functions definitions
 */
IX_STATUS
ixParityENAccIcEInit (void)
{
    UINT32 intcVirtualBaseAddr = 0;
    
    /* Memory mapping of the INTC Registers */
    if ((UINT32)NULL == (intcVirtualBaseAddr = (UINT32) IX_OSAL_MEM_MAP (
                                                IXP400_PARITYENACC_INTC_BASEADDR,
                                                IXP400_PARITYENACC_INTC_MEMMAP_SIZE)))
    {
        return IX_FAIL;
    } /* end of if */

    ixIntcVirtualBaseAddr = intcVirtualBaseAddr;

    /*
     * Virtual Addresses assignment for INTC Registers
     *
     * Interrupt Controller Driver would have selected the Parity Error
     * Interrupts as IRQs.
     */
    ixParityENAccIcERegisters.intrSt = 
        intcVirtualBaseAddr + IXP400_PARITYENACC_INTC_ST_OFFSET;
    ixParityENAccIcERegisters.intrSt2 =
        intcVirtualBaseAddr + IXP400_PARITYENACC_INTC_ST2_OFFSET;
    ixParityENAccIcERegisters.intrEn =
        intcVirtualBaseAddr + IXP400_PARITYENACC_INTC_EN_OFFSET;
    ixParityENAccIcERegisters.intrEn2 =
        intcVirtualBaseAddr + IXP400_PARITYENACC_INTC_EN2_OFFSET;
    ixParityENAccIcERegisters.intrIrqSt =
        intcVirtualBaseAddr + IXP400_PARITYENACC_INTC_IRQ_ST_OFFSET;
    ixParityENAccIcERegisters.intrIrqSt2 =
        intcVirtualBaseAddr + IXP400_PARITYENACC_INTC_IRQ_ST2_OFFSET;
    ixParityENAccIcERegisters.errorEn2 = 
        intcVirtualBaseAddr + IXP400_PARITYENACC_INTC_ERROR_EN2_OFFSET;

    return IX_SUCCESS;
} /* end of ixParityENAccIcEInit() function */

IX_STATUS
ixParityENAccIcInterruptDisable (
    IxParityENAccIcParityInterruptId ixIcParityIntrId)
{

    IX_STATUS status = IX_SUCCESS;

    status = ixParityENAccCheckIntIdValidity (ixIcParityIntrId);         
  
    if (status == IX_FAIL)
    {

        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccIcInterruptDisable(): "\
                "Unknown Interrupt 0x%x to Disable !!!\n", ixIcParityIntrId,0,0,0,0,0);
        return IX_FAIL;
    }
 
    switch (ixIcParityIntrId)
    {
        case IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_NPEA);

            ixOsalIrqDisable(IX_OSAL_IXP400_NPEA_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.npeAIntrSt = TRUE;

            break;
        } /* end of IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_NPEB);

            ixOsalIrqDisable(IX_OSAL_IXP400_NPEB_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.npeBIntrSt = TRUE;

            break;
        } /* end of IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_NPEC);

            ixOsalIrqDisable(IX_OSAL_IXP400_NPEC_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.npeCIntrSt = TRUE;

            break;
        } /* end of IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_PBC_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_PBC);

            ixOsalIrqDisable(IX_OSAL_IXP400_PCI_INT_IRQ_LVL);
            break;
        } /* end of IXP400_PARITYENACC_INTC_PBC_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.intrEn2, IXP400_PARITYENACC_INTC_SWCP);
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.errorEn2, IXP400_PARITYENACC_INTC_SWCP);

            ixOsalIrqDisable(IX_OSAL_IXP400_SWCP_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.swcpIntrSt = TRUE;

            break;
        } /* end of IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.intrEn2, IXP400_PARITYENACC_INTC_AQM);
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.errorEn2, IXP400_PARITYENACC_INTC_AQM);

            ixOsalIrqDisable(IX_OSAL_IXP400_AQM_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.aqmIntrSt = TRUE;

            break;
        } /* end of IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_MCU_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.intrEn2, IXP400_PARITYENACC_INTC_MCU);
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.errorEn2, IXP400_PARITYENACC_INTC_MCU);

            break;
        } /* end of IXP400_PARITYENACC_INTC_MCU_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.intrEn2, IXP400_PARITYENACC_INTC_EBC);
            IXP400_PARITYENACC_REG_BIT_CLEAR(
                ixParityENAccIcERegisters.errorEn2, IXP400_PARITYENACC_INTC_EBC);

            break;
        } /* end of IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT */

        default:
        {
#ifndef NDEBUG
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccIcInterruptDisable(): "\
                "Unknown Interrupt 0x%x to Disable !!!\n", ixIcParityIntrId,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
            return IX_FAIL;
        } /* end of default */
    } /* end of switch */

    return IX_SUCCESS;
} /* end of ixParityENAccIcInterruptDisable() function */

IX_STATUS
ixParityENAccIcInterruptEnable (
    IxParityENAccIcParityInterruptId ixIcParityIntrId)
{
    
    IX_STATUS status = IX_SUCCESS;

    status = ixParityENAccCheckIntIdValidity (ixIcParityIntrId);         
  
    if (status == IX_FAIL)
    {

        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccIcInterruptEnable(): "\
                "Unknown Interrupt 0x%x to Enable !!!\n", ixIcParityIntrId,0,0,0,0,0);
        return IX_FAIL;
    }

    switch (ixIcParityIntrId)
    {
        case IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_NPEA);
            
            ixParityENAccIcEMaskedOffParityIntr.npeAIntrSt = FALSE;  
            ixOsalIrqEnable(IX_OSAL_IXP400_NPEA_IRQ_LVL);
            
            break;
        } /* end of IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_NPEB);

            ixParityENAccIcEMaskedOffParityIntr.npeBIntrSt = FALSE;
            ixOsalIrqEnable(IX_OSAL_IXP400_NPEB_IRQ_LVL);
            
            break;
        } /* end of IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_NPEC);

            ixParityENAccIcEMaskedOffParityIntr.npeCIntrSt = FALSE;
            ixOsalIrqEnable(IX_OSAL_IXP400_NPEC_IRQ_LVL);
            
            break;
        } /* end of IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_PBC_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_PBC);

            ixOsalIrqEnable(IX_OSAL_IXP400_PCI_INT_IRQ_LVL);
            break;
        } /* end of IXP400_PARITYENACC_INTC_PBC_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn2, IXP400_PARITYENACC_INTC_SWCP);
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.errorEn2, IXP400_PARITYENACC_INTC_SWCP);
            
            ixParityENAccIcEMaskedOffParityIntr.swcpIntrSt = FALSE;
            ixOsalIrqEnable(IX_OSAL_IXP400_SWCP_IRQ_LVL);
            
            break;
        } /* end of IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn2, IXP400_PARITYENACC_INTC_AQM);
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.errorEn2, IXP400_PARITYENACC_INTC_AQM);

            ixParityENAccIcEMaskedOffParityIntr.aqmIntrSt = FALSE;
            ixOsalIrqEnable(IX_OSAL_IXP400_AQM_IRQ_LVL);
            
            break;
        } /* end of IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_MCU_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn2, IXP400_PARITYENACC_INTC_MCU);
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.errorEn2, IXP400_PARITYENACC_INTC_MCU);

            break;
        } /* end of IXP400_PARITYENACC_INTC_MCU_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn2, IXP400_PARITYENACC_INTC_EBC);
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.errorEn2, IXP400_PARITYENACC_INTC_EBC);

            break;
        } /* end of IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT */

        default:
        {
#ifndef NDEBUG
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccIcInterruptEnable(): "\
                "Unknown Interrupt 0x%x to Enable !!!\n", ixIcParityIntrId,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
            return IX_FAIL;
        } /* end of default */
    } /* end of switch */

    return IX_SUCCESS;
} /* end of ixParityENAccIcInterruptEnable() function */

IX_STATUS
ixParityENAccIcInterruptStatusGet(
    IxParityENAccIcParityInterruptStatus *ixIcParityInterruptStatus)
{
    if ((IxParityENAccIcParityInterruptStatus *)NULL == ixIcParityInterruptStatus)
    {
        return IX_FAIL;
    } /* end of if */

    /* Fetch the Config and Status of the interrupts */
    IXP400_PARITYENACC_REG_READ(ixParityENAccIcERegisters.intrSt,
        &ixParityENAccIcEParityIntrStatus.intrStValue);

    IXP400_PARITYENACC_REG_READ(ixParityENAccIcERegisters.intrSt2,
        &ixParityENAccIcEParityIntrStatus.intrSt2Value);

    IXP400_PARITYENACC_REG_READ(ixParityENAccIcERegisters.intrEn,
        &ixParityENAccIcEParityIntrStatus.intrEnValue);

    IXP400_PARITYENACC_REG_READ(ixParityENAccIcERegisters.intrEn2,
        &ixParityENAccIcEParityIntrStatus.intrEn2Value);

    IXP400_PARITYENACC_REG_READ(ixParityENAccIcERegisters.intrIrqSt,
        &ixParityENAccIcEParityIntrStatus.intrIrqStValue);

    IXP400_PARITYENACC_REG_READ(ixParityENAccIcERegisters.intrIrqSt2,
        &ixParityENAccIcEParityIntrStatus.intrIrqSt2Value);

    IXP400_PARITYENACC_REG_READ(ixParityENAccIcERegisters.errorEn2,
        &ixParityENAccIcEParityIntrStatus.errorEn2Value);

    /*
     * Return the status of only the Enabled && Flagged interrupts
     */
    ixIcParityInterruptStatus->npeAParityInterrupt = 
        (IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrStValue, 
            IXP400_PARITYENACC_INTC_NPEA) && 
        (FALSE == ixParityENAccIcEMaskedOffParityIntr.npeAIntrSt));

    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X != ixFeatureCtrlDeviceRead())
    {
    
        ixIcParityInterruptStatus->npeBParityInterrupt = 
            (IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.
            intrStValue,
            IXP400_PARITYENACC_INTC_NPEB) && 
        (FALSE == ixParityENAccIcEMaskedOffParityIntr.npeBIntrSt));

    }
    else /* for IXP 43X NPE B intr is disabled Hence initialize to FALSE */ 
    {
        ixIcParityInterruptStatus->npeBParityInterrupt = FALSE;
    }

    ixIcParityInterruptStatus->npeCParityInterrupt = 
        (IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrStValue,
            IXP400_PARITYENACC_INTC_NPEC) && 
        (FALSE == ixParityENAccIcEMaskedOffParityIntr.npeCIntrSt));

    ixIcParityInterruptStatus->pbcParityInterrupt  = 
        IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrStValue,
            IXP400_PARITYENACC_INTC_PBC);

    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X != ixFeatureCtrlDeviceRead())
    {

        ixIcParityInterruptStatus->swcpParityInterrupt = 
            (IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.
            intrSt2Value,
            IXP400_PARITYENACC_INTC_SWCP) && 
            (FALSE == ixParityENAccIcEMaskedOffParityIntr.swcpIntrSt));

    }
    else /* for IXP 43X NPE B intr is disabled Hence initialize to FALSE */ 
    {
        ixIcParityInterruptStatus->swcpParityInterrupt = FALSE;
    }

    ixIcParityInterruptStatus->aqmParityInterrupt  = 
        (IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrSt2Value,
            IXP400_PARITYENACC_INTC_AQM)&& 
        (FALSE == ixParityENAccIcEMaskedOffParityIntr.aqmIntrSt));

    ixIcParityInterruptStatus->mcuParityInterrupt  = 
        IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrSt2Value,
            IXP400_PARITYENACC_INTC_MCU);

    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X != ixFeatureCtrlDeviceRead())
    {
    
        ixIcParityInterruptStatus->ebcParityInterrupt  = 
            IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.
            intrSt2Value,
            IXP400_PARITYENACC_INTC_EBC);

    }
    else /* for IXP 43X NPE B intr is disabled Hence initialize to FALSE */ 
    {
        ixIcParityInterruptStatus->ebcParityInterrupt  = FALSE;
    }

    return IX_SUCCESS;
} /* end of ixParityENAccIcInterruptStatusGet() function */

IX_STATUS   
ixParityENAccIcEUnload(void)
{
    /* Unmap the memory */
    IX_OSAL_MEM_UNMAP(ixIntcVirtualBaseAddr);

    return IX_SUCCESS;
} /* end of ixParityENAccIcEUnload() function */

IX_STATUS
ixParityENAccCheckIntIdValidity (IxParityENAccIcParityInterruptId ixIcParityIntrId)
{
    if (ixIcParityIntrId > IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT ) 
    { 
        return IX_FAIL; 
    } /* end of if */         

    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead ())
    {

        if ((ixIcParityIntrId == IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT)
        || (ixIcParityIntrId == IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT) 
        || (ixIcParityIntrId == IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT)) 
        { 
            return IX_FAIL; 
        }

    } 

    return IX_SUCCESS;

}
#endif /* __ixp46X || __ixp43X */
