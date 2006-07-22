/**
 * @file IxParityENAccNpePE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for NPE Parity Detection Enabler sub-component
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

#ifndef IXPARITYENACCNPEPE_H
#define IXPARITYENACCNPEPE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * typedefs used in this file
 */

/* NPE parity detection configuration parameters */
typedef struct /* IxParityENAccNpePEConfigOption */
{
    IxParityENAccPEConfigOption ideEnabled; /* IMem, DMem, External Error */
    IxParityENAccParityType parityOddEven;  /* Parity - Odd or Even */
} IxParityENAccNpePEConfigOption;

/* NPE Identifiers */
typedef enum  /* IxParityENAccPENpeId */
{
    IXP400_PARITYENACC_PE_NPE_A,     /* NPE - A */
    IXP400_PARITYENACC_PE_NPE_B,     /* NPE - B */
    IXP400_PARITYENACC_PE_NPE_C,     /* NPE - C */
    IXP400_PARITYENACC_PE_NPE_MAX    /* MAX # of NPEs */
} IxParityENAccPENpeId;

/* Source of the parity error */
typedef enum  /* IxParityENAccNpePEParityErrorSource  */
{
    IXP400_PARITYENACC_PE_NPE_IMEM,  /* Instruction memory */
    IXP400_PARITYENACC_PE_NPE_DMEM,  /* Data memory */
    IXP400_PARITYENACC_PE_NPE_EXT    /* External Error */
} IxParityENAccNpePEParityErrorSource;

/* NPE Parity Error Context Message */
typedef struct /* IxParityENAccNpePEParityErrorContext */
{
    /* NPE source info of parity error */
    IxParityENAccNpePEParityErrorSource   npeParitySource;

    /* Read Always */
    IxParityENAccPEParityErrorAccess      npeAccessType;
} IxParityENAccNpePEParityErrorContext;

/*
 * Local functions declarations accessible to Main sub-module
 */

/* Function for Initialisation */
IX_STATUS
ixParityENAccNpePEInit (IxParityENAccInternalCallback ixNpePECallback);

/* Function to Enable/Disable parity error detection in NPE */
IX_STATUS
ixParityENAccNpePEDetectionConfigure (
    IxParityENAccPENpeId ixNpeId,
    IxParityENAccNpePEConfigOption ixNpePDCfg);

/* Function to present the parity error context to Main module */
IX_STATUS
ixParityENAccNpePEParityErrorContextFetch (
    IxParityENAccPENpeId ixNpeId,
    IxParityENAccNpePEParityErrorContext *ixNpePECMsg);

/* Function to clear off the parity error interrupt */
IX_STATUS
ixParityENAccNpePEParityInterruptClear (IxParityENAccPENpeId ixNpeId);

#endif /* IXPARITYENACCNPEPE_H */
