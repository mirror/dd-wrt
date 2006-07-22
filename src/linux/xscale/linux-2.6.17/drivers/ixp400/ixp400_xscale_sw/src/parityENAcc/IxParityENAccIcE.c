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
#include "IxParityENAccIcE_p.h"

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
    if (IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT < ixIcParityIntrId)
    {
        return IX_FAIL;
    } /* end of if */

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
    if (IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT < ixIcParityIntrId)
    {
        return IX_FAIL;
    } /* end of if */

    switch (ixIcParityIntrId)
    {
        case IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_NPEA);

            ixOsalIrqEnable(IX_OSAL_IXP400_NPEA_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.npeAIntrSt = FALSE;
            break;
        } /* end of IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_NPEB);

            ixOsalIrqEnable(IX_OSAL_IXP400_NPEB_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.npeBIntrSt = FALSE;
            break;
        } /* end of IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn, IXP400_PARITYENACC_INTC_NPEC);

            ixOsalIrqEnable(IX_OSAL_IXP400_NPEC_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.npeCIntrSt = FALSE;
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

            ixOsalIrqEnable(IX_OSAL_IXP400_SWCP_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.swcpIntrSt = FALSE;
            break;
        } /* end of IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT */

        case IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT:
        {
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.intrEn2, IXP400_PARITYENACC_INTC_AQM);
            IXP400_PARITYENACC_REG_BIT_SET(
                ixParityENAccIcERegisters.errorEn2, IXP400_PARITYENACC_INTC_AQM);

            ixOsalIrqEnable(IX_OSAL_IXP400_AQM_IRQ_LVL);
            ixParityENAccIcEMaskedOffParityIntr.aqmIntrSt = FALSE;
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

    ixIcParityInterruptStatus->npeBParityInterrupt = 
        (IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrStValue,
            IXP400_PARITYENACC_INTC_NPEB) && 
        (FALSE == ixParityENAccIcEMaskedOffParityIntr.npeBIntrSt));

    ixIcParityInterruptStatus->npeCParityInterrupt = 
        (IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrStValue,
            IXP400_PARITYENACC_INTC_NPEC) && 
        (FALSE == ixParityENAccIcEMaskedOffParityIntr.npeCIntrSt));

    ixIcParityInterruptStatus->pbcParityInterrupt  = 
        IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrStValue,
            IXP400_PARITYENACC_INTC_PBC);

    ixIcParityInterruptStatus->swcpParityInterrupt = 
        (IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrSt2Value,
            IXP400_PARITYENACC_INTC_SWCP) && 
        (FALSE == ixParityENAccIcEMaskedOffParityIntr.swcpIntrSt));

    ixIcParityInterruptStatus->aqmParityInterrupt  = 
        (IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrSt2Value,
            IXP400_PARITYENACC_INTC_AQM)&& 
        (FALSE == ixParityENAccIcEMaskedOffParityIntr.aqmIntrSt));

    ixIcParityInterruptStatus->mcuParityInterrupt  = 
        IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrSt2Value,
            IXP400_PARITYENACC_INTC_MCU);

    ixIcParityInterruptStatus->ebcParityInterrupt  = 
        IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccIcEParityIntrStatus.intrSt2Value,
            IXP400_PARITYENACC_INTC_EBC);

    return IX_SUCCESS;
} /* end of ixParityENAccIcInterruptStatusGet() function */

#endif /* __ixp46X */
