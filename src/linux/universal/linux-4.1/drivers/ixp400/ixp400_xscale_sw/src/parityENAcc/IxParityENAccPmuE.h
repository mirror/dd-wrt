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

    /* USB Host Controller 0 */
    IXP400_PARITYENACC_PMUE_AHBS_MST_USBH0 = IX_PARITYENACC_AHBS_MST_USBH0,

    /* USB Host Controller 1 */
    IXP400_PARITYENACC_PMUE_AHBS_MST_USBH1 = IX_PARITYENACC_AHBS_MST_USBH1,

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

    /* USB Host Controller 0 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_USBH0 = IX_PARITYENACC_AHBS_SLV_USBH0,

    /* USB Host Controller 1 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_USBH1 = IX_PARITYENACC_AHBS_SLV_USBH1,

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

/* Function to unload the component */
IX_STATUS
ixParityENAccPmuEUnload(void);

#endif /* IXPARITYENACCPMUE_H */
