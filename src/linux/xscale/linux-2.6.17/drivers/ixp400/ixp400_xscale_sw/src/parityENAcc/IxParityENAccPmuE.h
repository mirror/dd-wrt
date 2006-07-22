/**
 * @file IxParityENAccPmuE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for Performance Monitoring Unit Enabler sub-
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

#ifndef IXPARITYENACCPMUE_H
#define IXPARITYENACCPMUE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * Typedefs used in this file
 */

/*
 * The Master on the AHB bus interface whose transaction might 
 * have resulted in the parity error
 */
typedef enum  /* IxParityENAccPmuEAHBErrorMaster */
{
    /* NPE - A */
    IXP400_PARITYENACC_PMUE_AHBN_MST_NPE_A = IX_PARITYENACC_AHBN_MST_NPE_A,

    /* NPE - B */
    IXP400_PARITYENACC_PMUE_AHBN_MST_NPE_B = IX_PARITYENACC_AHBN_MST_NPE_B,

    /* NPE - C */
    IXP400_PARITYENACC_PMUE_AHBN_MST_NPE_C = IX_PARITYENACC_AHBN_MST_NPE_C,

    /* XScale Bus Interface Unit */
    IXP400_PARITYENACC_PMUE_AHBS_MST_XSCALE = IX_PARITYENACC_AHBS_MST_XSCALE,

    /* PCI Bus Controller */
    IXP400_PARITYENACC_PMUE_AHBS_MST_PBC = IX_PARITYENACC_AHBS_MST_PBC,

    /* Expansion Bus Controller */
    IXP400_PARITYENACC_PMUE_AHBS_MST_EBC = IX_PARITYENACC_AHBS_MST_EBC,

    /* AHB Bridge */
    IXP400_PARITYENACC_PMUE_AHBS_MST_AHB_BRIDGE = IX_PARITYENACC_AHBS_MST_AHB_BRIDGE,

    /* USB Host Controller */
    IXP400_PARITYENACC_PMUE_AHBS_MST_USBH = IX_PARITYENACC_AHBS_MST_USBH,

    /* Invalid Master */
    IXP400_PARITYENACC_PMUE_AHBS_MST_INVALID
} IxParityENAccPmuEAHBErrorMaster;

/*
 * The Slave on the AHB bus interface whose transaction might 
 * have resulted in the parity error
 */
typedef enum  /* IxParityENAccPmuEAHBErrorSlave */
{
    /* Memory Control Unit */
    IXP400_PARITYENACC_PMUE_AHBN_SLV_MCU = IX_PARITYENACC_AHBN_SLV_MCU,

    /* AHB Bridge */
    IXP400_PARITYENACC_PMUE_AHBN_SLV_AHB_BRIDGE = IX_PARITYENACC_AHBN_SLV_AHB_BRIDGE, 

    /* XScale Bus Interface Unit */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_MCU = IX_PARITYENACC_AHBS_SLV_MCU,

    /* PCI Bus Controller */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_PBC = IX_PARITYENACC_AHBS_SLV_PBC,

    /* Expansion Bus Controller */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_EBC = IX_PARITYENACC_AHBS_SLV_EBC,

    /* APB Bridge */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_APB_BRIDGE = IX_PARITYENACC_AHBS_SLV_APB_BRIDGE,

    /* AQM */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_AQM = IX_PARITYENACC_AHBS_SLV_AQM,

    /* RSA */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_RSA = IX_PARITYENACC_AHBS_SLV_RSA,

    /* USB Host Controller */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_USBH = IX_PARITYENACC_AHBS_SLV_USBH,

    /* Invalid Slave */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_INVALID
} IxParityENAccPmuEAHBErrorSlave;

/*
 * The Master and Slave on the AHB bus interface whose transaction
 * might have resulted in the parity error
 */
typedef struct  /* IxParityENAccPmuEAHBErrorTransaction  */
{
    IxParityENAccPmuEAHBErrorMaster  ahbErrorMaster; /* Master on AHB bus */
    IxParityENAccPmuEAHBErrorSlave   ahbErrorSlave;  /* Slave on AHB bus */
} IxParityENAccPmuEAHBErrorTransaction;


/*
 * Local functions declarations
 */

/* Function for Initialisation */
IX_STATUS
ixParityENAccPmuEInit (void);

/* Function for get the last erroneous AHB transaction status */
IX_STATUS 
ixParityENAccPmuEAHBTransactionStatus (
    IxParityENAccPmuEAHBErrorTransaction *ixPmuAhbTransactionStatus);

#endif /* IXPARITYENACCPMUE_H */
