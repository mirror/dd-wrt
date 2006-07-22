/**
 * @file IxParityENAccEbcPE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for EBC Parity Detection Enabler sub-component
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

#ifndef IXPARITYENACCEBCPE_H
#define IXPARITYENACCEBCPE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * Typedefs used in this file
 */

/* Configure parity error notification on Expansion bus controller */
typedef enum  /* IxParityENAccEbcPEParityConfigSource  */
{
    IXP400_PARITYENACC_PE_EBC_CS = IX_PARITYENACC_EBC_CS,        /* Chip Select */
    IXP400_PARITYENACC_PE_EBC_EXTMST = IX_PARITYENACC_EBC_EXTMST /* EBC Master */
} IxParityENAccEbcPEParityConfigSource;

/* Source of the parity error notification on Expansion bus controller */
typedef IxParityENAccEbcPEParityConfigSource IxParityENAccEbcPEParityErrorSource;

/* Chip Selects identifiers */
typedef enum  /* IxParityENAccChipSelectId */
{
    IXP400_PARITYENACC_PE_EBC_CHIPSEL0,    /* Chip Select - 0 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL1,    /* Chip Select - 1 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL2,    /* Chip Select - 2 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL3,    /* Chip Select - 3 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL4,    /* Chip Select - 4 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL5,    /* Chip Select - 5 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL6,    /* Chip Select - 6 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL7,    /* Chip Select - 7 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL_MAX  /* MAX # Chip Select */
} IxParityENAccChipSelectId;

/*
 * Expansion Bus Controller parity detection configuration parameter
 *
 * NOTE: Chip Select and EXT Master are mutually exclusive
 */
typedef struct /* IxParityENAccEbcPEConfigOption */
{
    /* Chip or EXT Master */
    IxParityENAccEbcPEParityConfigSource  ebcCsExtSource;

    union /* ebcInOrOutbound */
    {
        IxParityENAccPEConfigOption  ebcCsEnabled;     /* Chip Select */
        IxParityENAccPEConfigOption  ebcExtMstEnabled; /* EXT Master */
    } ebcInOrOutbound;

    /* Chip Select: Applicable only when chip select configuration */
    IxParityENAccChipSelectId  ebcCsId;

    /* Parity - Odd or Even applies to both CS and EXT Master */
    IxParityENAccParityType  parityOddEven;            
} IxParityENAccEbcPEConfigOption;

/* Expansion Bus Controller Parity Error Context Message */
typedef struct /* IxParityENAccEbcPEParityErrorContext */
{
    /* EBC source info of parity error */
    IxParityENAccEbcPEParityErrorSource  ebcParitySource;

    /* Outbound Read or Inbound Write Access */
    IxParityENAccPEParityErrorAccess  ebcAccessType;

    /* AHB bus Address [31:2] */
    IxParityENAccPEParityErrorAddress  ebcParityAddress;
} IxParityENAccEbcPEParityErrorContext;

/*
 * Local functions declarations
 *
 * NOTE: They are to be made available to the Main sub-component only
 */

/* Function for Initialisation */
IX_STATUS
ixParityENAccEbcPEInit (IxParityENAccInternalCallback ixEbcPECallback);

/* Function to Enable/Disable parity error detection in EBC */
IX_STATUS
ixParityENAccEbcPEDetectionConfigure(IxParityENAccEbcPEConfigOption ixEbcPDCfg);

/* Function to present the parity error context to Main module */
IX_STATUS 
ixParityENAccEbcPEParityErrorContextFetch(
    IxParityENAccEbcPEParityErrorContext *ixEbcPECMsg);

/* Function to clear off the parity error interrupt */
IX_STATUS
ixParityENAccEbcPEParityInterruptClear (
    IxParityENAccEbcPEParityConfigSource ixEbcParityErrSrc);

#endif /* IXPARITYENACCEBCPE_H */
