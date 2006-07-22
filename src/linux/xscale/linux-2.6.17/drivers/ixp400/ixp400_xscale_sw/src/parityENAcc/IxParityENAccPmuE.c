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
#include "IxParityENAccPmuE.h"
#include "IxParityENAccPmuE_p.h"

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
            case IXP400_PARITYENACC_PMU_PMSS_USBH:
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
            case IXP400_PARITYENACC_PMU_PMSS_USBH:
            case IXP400_PARITYENACC_PMU_PMSS_MCU:
            case IXP400_PARITYENACC_PMU_PMSS_APB_BRIDGE:
            case IXP400_PARITYENACC_PMU_PMSS_AQM:
            case IXP400_PARITYENACC_PMU_PMSS_RSA:
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
            case IXP400_PARITYENACC_PMU_PMSN_NPE_B:
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

#endif /* __ixp46X */
