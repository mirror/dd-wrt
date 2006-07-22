/**
 * @file IxParityENAccMcuPE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for MCU Parity Detection Enabler sub-component
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

#ifndef IXPARITYENACCMCUPE_H
#define IXPARITYENACCMCUPE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * Typedefs used in this file
 */

/* MCU pairty detection configuration parameter */
typedef struct /* IxParityENAccMcuPEConfigOption */
{
    /* Single-bit parity error detection */
    IxParityENAccPEConfigOption singlebitDetectEnabled;

    /* Single-bit parity error correction */
    IxParityENAccPEConfigOption singlebitCorrectionEnabled;

    /* Multi-bit parity error detection */
    IxParityENAccPEConfigOption multibitDetectionEnabled;  
} IxParityENAccMcuPEConfigOption;

/* Source of the parity error notification in DDR MCU */
typedef enum  /* IxParityENAccMcuPEParityErrorSource  */
{
    /* Single bit parity */
    IXP400_PARITYENACC_PE_MCU_SBIT = IX_PARITYENACC_MCU_SBIT,

    /* Multi bit parity */
    IXP400_PARITYENACC_PE_MCU_MBIT = IX_PARITYENACC_MCU_MBIT,

    /* Parity errors in excess of two */
    IXP400_PARITYENACC_PE_MCU_OVERFLOW = IX_PARITYENACC_MCU_OVERFLOW,

    /* No Parity */
    IXP400_PARITYENACC_PE_MCU_NOPARITY = IX_PARITYENACC_NO_PARITY
} IxParityENAccMcuPEParityErrorSource;

/*
 * The requester interface through which the SDRAM memory access 
 * resulted in the parity error.
 */
typedef enum  /* IxParityENAccPEParityErrorRequester */
{
    /* Intel XScale Core BIU */
    IXP400_PARITYENACC_PE_MCU_MPI = IX_PARITYENACC_MPI,

    /* IB BUS (South or North AHB Bus) */
    IXP400_PARITYENACC_PE_MCU_AHB_BUS = IX_PARITYENACC_AHB_BUS
} IxParityENAccPEParityErrorRequester;

/* MCU Parity Error Context Message */
typedef struct /* IxParityENAccMcuPEParityErrorContext */
{
    /* MCU source info of parity error */
    IxParityENAccMcuPEParityErrorSource  mcuParitySource;

    /* Read or Write Access */
    IxParityENAccPEParityErrorAccess  mcuAccessType;

    /* Address faulty location in MCU SDRAM */
    IxParityENAccPEParityErrorAddress  mcuParityAddress;

    /* The faulty bit of the Single-bit parity */
    IxParityENAccPEParityErrorData  mcuParityData;

    /*  MCU port used for the SDRAM access */
    IxParityENAccPEParityErrorRequester  mcuRequester;
} IxParityENAccMcuPEParityErrorContext;

/*
 * Local functions declarations
 * NOTE: They are to be made available to the Main sub-component only
 */

/* Initialise the MCU sub-module */
IX_STATUS
ixParityENAccMcuPEInit (IxParityENAccInternalCallback ixMcuPECallback);

/* Configure/Re-configure MCU parity error detection enable/disable */
IX_STATUS 
ixParityENAccMcuPEDetectionConfigure (IxParityENAccMcuPEConfigOption ixMcuPDCfg);

/* Retrieve the MCU parity error context */
IX_STATUS 
ixParityENAccMcuPEParityErrorContextFetch (
    IxParityENAccMcuPEParityErrorContext *ixMcuPECMsg);

/* Clear single/multi-bit and overflow parity interrupt conditions */
IX_STATUS
ixParityENAccMcuPEParityInterruptClear (
    IxParityENAccMcuPEParityErrorSource ixMcuParityErrSrc,
    IxParityENAccPEParityErrorAddress ixMcuParityErrAddress);

#endif /* IXPARITYENACCMCUPE_H */
