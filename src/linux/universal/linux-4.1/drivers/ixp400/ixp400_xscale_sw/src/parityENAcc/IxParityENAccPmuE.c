/**
 * @file IxParityENAccPmuE.c
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Source file for Performance Monitoring Unit Enabler sub-
 * component of the IXP400 Parity Error Notifier access component.
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
#include "IxParityENAccPmuE.h"
#include "IxParityENAccPmuE_p.h"

/* Virtual base address of PMU */
static UINT32 ixPmuVirtualBaseAddr = 0;

/*
 * PMU sub-module level functions definitions
 */
IX_STATUS
ixParityENAccPmuEInit (void)
{
    /* Virtual Addresses assignment for PMU Previous Master/Slave Register */
    if ((UINT32)NULL == (ixParityENAccPmuEPmsr = (UINT32) IX_OSAL_MEM_MAP (
                                                 IXP400_PARITYENACC_PMU_BASEADDR,
                                                 IXP400_PARITYENACC_PMU_MEMMAP_SIZE)))
    {
        return IX_FAIL;
    } /* end of if */

    ixPmuVirtualBaseAddr = ixParityENAccPmuEPmsr;
    
    return IX_SUCCESS;
} /* end of ixParityENAccPmuEInit() function */

IX_STATUS 
ixParityENAccPmuEAHBTransactionStatus (
    IxParityENAccPmuEAHBErrorTransaction *ixPmuAhbTransactionStatus)
{
    UINT32 pmuPmsrStatus = 0;

    if ((IxParityENAccPmuEAHBErrorTransaction *)NULL == ixPmuAhbTransactionStatus)
    {
        return IX_FAIL;
    } /* end of if */

    /* Read PMU Previous Master/Slave Register contents */
    IXP400_PARITYENACC_REG_READ(ixParityENAccPmuEPmsr, &ixParityENAccPmuEPmsrStatus);

    if (TRUE == IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccPmuEPmsrStatus,
                    (UINT32) IXP400_PARITYENACC_PMU_PMSR_SOUTH_AHB_ERR))
    {
        /* Check the South AHB Master */
        pmuPmsrStatus = IXP400_PARITYENACC_VAL_READ_PMU_MASTER_SOUTH(
                            ixParityENAccPmuEPmsrStatus);
        switch (pmuPmsrStatus)
        {
            case IXP400_PARITYENACC_PMU_PMSS_XSCALE_BIU:
            case IXP400_PARITYENACC_PMU_PMSS_PBC:
            case IXP400_PARITYENACC_PMU_PMSS_AHB_BRIDGE:
            case IXP400_PARITYENACC_PMU_PMSS_EBC:
            case IXP400_PARITYENACC_PMU_PMSS_USBH0:
            case IXP400_PARITYENACC_PMU_PMSS_USBH1:
            {
                ixPmuAhbTransactionStatus->ahbErrorMaster = 
                    ixParityENAccPmuESouthAhbMaster[pmuPmsrStatus];
                break;
            } /* end of cases */

            default:
            {
#ifndef NDEBUG
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                    "ixParityENAccPmuEAHBTransactionStatus(): "
                    "Unknown South AHB Master from PMU !!!\n", 0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
                return IX_FAIL;
                break;
            } /* end of default case */
        } /* end of switch */

        /* Check the South AHB Slave */
        pmuPmsrStatus = IXP400_PARITYENACC_VAL_READ_PMU_SLAVE_SOUTH(
                            ixParityENAccPmuEPmsrStatus);
        switch (pmuPmsrStatus)
        {
            case IXP400_PARITYENACC_PMU_PMSS_PBC:
            case IXP400_PARITYENACC_PMU_PMSS_EBC:
            case IXP400_PARITYENACC_PMU_PMSS_MCU:
            case IXP400_PARITYENACC_PMU_PMSS_APB_BRIDGE:
            case IXP400_PARITYENACC_PMU_PMSS_AQM:
            case IXP400_PARITYENACC_PMU_PMSS_USBH0:
#if defined(__ixp43X)
            case IXP400_PARITYENACC_PMU_PMSS_RSA:
#else
            case IXP400_PARITYENACC_PMU_PMSS_USBH1:
#endif
            {
                ixPmuAhbTransactionStatus->ahbErrorSlave = 
                    ixParityENAccPmuESouthAhbSlave[pmuPmsrStatus];
                break;
            } /* end of cases */

            default:
            {
#ifndef NDEBUG
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccPmuEAHBTransactionStatus(): "
                    "Unknown South AHB Slave from PMU !!!\n", 0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
                return IX_FAIL;
                break;
            } /* end of default case */
        } /* end of switch */

        /* 
         * Clear off the South AHB error transaction indicator 
         * by writing '1' to corresponding bit
         */
        IXP400_PARITYENACC_REG_BIT_SET(ixParityENAccPmuEPmsr, 
            IXP400_PARITYENACC_PMU_PMSR_SOUTH_AHB_ERR);

        return IX_SUCCESS;
    } /* end of if (TRUE == IXP400_PARITYENACC_VAL_BIT_CHECK(
                    ixParityENAccPmuEPmsrStatus,
                    IXP400_PARITYENACC_PMU_PMSR_SOUTH_AHB_ERR)) */

    if (TRUE == IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccPmuEPmsrStatus,
                    IXP400_PARITYENACC_PMU_PMSR_NORTH_AHB_ERR))
    {
        /* Check the North AHB Master */
        pmuPmsrStatus = IXP400_PARITYENACC_VAL_READ_PMU_MASTER_NORTH(
                            ixParityENAccPmuEPmsrStatus);
        switch (pmuPmsrStatus)
        {
            case IXP400_PARITYENACC_PMU_PMSN_NPE_A:
#if !defined(__ixp43X)
            case IXP400_PARITYENACC_PMU_PMSN_NPE_B:
#endif
            case IXP400_PARITYENACC_PMU_PMSN_NPE_C:
            {
                ixPmuAhbTransactionStatus->ahbErrorMaster = 
                    ixParityENAccPmuENorthAhbMaster[pmuPmsrStatus];
                break;
            } /* end of case IXP400_PARITYENACC_PMU_PMSN_NPE_C */

            default:
            {
#ifndef NDEBUG
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                    "ixParityENAccPmuEAHBTransactionStatus(): "
                    "Unknown North AHB Master from PMU !!!\n", 0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
                return IX_FAIL;
                break;
            } /* end of default case */
        } /* end of switch */

        /* Check the North AHB Slave */
        pmuPmsrStatus = IXP400_PARITYENACC_VAL_READ_PMU_SLAVE_NORTH(
                            ixParityENAccPmuEPmsrStatus);
        switch (pmuPmsrStatus)
        {
            case IXP400_PARITYENACC_PMU_PMSN_MCU:
            case IXP400_PARITYENACC_PMU_PMSN_AHB_BRIDGE:
            {
                ixPmuAhbTransactionStatus->ahbErrorSlave = 
                    ixParityENAccPmuENorthAhbSlave[pmuPmsrStatus];
                break;
            } /* end of case IXP400_PARITYENACC_PMU_PMSN_AHB_BRIDGE */

            default:
            {
#ifndef NDEBUG
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccPmuEAHBTransactionStatus(): "
                    "Unknown North AHB Slave from PMU !!!\n", 0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
                return IX_FAIL;
                break;
            } /* end of default case */
        } /* end of switch */

        /* 
         * Clear off the North AHB error transaction indicator 
         * by writing '1' to corresponding bit
         */
        IXP400_PARITYENACC_REG_BIT_SET(ixParityENAccPmuEPmsr, 
            IXP400_PARITYENACC_PMU_PMSR_NORTH_AHB_ERR);

        return IX_SUCCESS;
    } /* end of if (TRUE == IXP400_PARITYENACC_VAL_BIT_CHECK(
                                ixParityENAccPmuEPmsrStatus,
                                IXP400_PARITYENACC_PMU_PMSR_NORTH_AHB_ERR)) */

    return IX_SUCCESS;
} /* end of ixParityENAccPmuEAHBTransactionStatus() function */

IX_STATUS 
ixParityENAccPmuEUnload(void)
{
    /* Unmap the memory */
    IX_OSAL_MEM_UNMAP(ixPmuVirtualBaseAddr);

    return IX_SUCCESS;
}/* end of ixParityENAccPmuEUnload() function */

#endif /* __ixp46X || __ixp43X */
