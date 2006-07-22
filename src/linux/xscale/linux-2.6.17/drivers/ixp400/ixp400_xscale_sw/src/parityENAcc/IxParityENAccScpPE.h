/**
 * @file IxParityENAccSpcPE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for SWCP Parity Detection Enabler sub-component
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

#ifndef IXPARITYENACCSPCPE_H
#define IXPARITYENACCSPCPE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * Typedefs used in this file
 */
/* SWCP parity detection configuration parameter */
typedef IxParityENAccPEConfigOption IxParityENAccSwcpPEConfigOption;

/* Source of the parity error notification SWCP */
typedef enum  /* IxParityENAccSwcpPEParityErrorSource  */
{
    IXP400_PARITYENACC_PE_SWCP = IX_PARITYENACC_SWCP
} IxParityENAccSwcpPEParityErrorSource;

/* SWCP Parity Error Context Message */
typedef struct /* IxParityENAccSwcpPEParityErrorContext */
{
    /* SWCP source info of parity error */
    IxParityENAccSwcpPEParityErrorSource swcpParitySource;

    /* Read Always */
    IxParityENAccPEParityErrorAccess     swcpAccessType;
} IxParityENAccSwcpPEParityErrorContext;

/*
 * Local functions declarations accessible to Main sub-component
 *
 * NOTE: They are to be made available to the Main sub-component only
 */

/* Function for Initialisation */
IX_STATUS
ixParityENAccSwcpPEInit (IxParityENAccInternalCallback ixSwcpPECallback);

/* Function to Configure the SWCP parity error interrupt */
IX_STATUS
ixParityENAccSwcpPEDetectionConfigure (
    IxParityENAccSwcpPEConfigOption ixSwcpPDCfg);

/* Function to clear off the parity error interrupt */
IX_STATUS
ixParityENAccSwcpPEParityInterruptClear (void);

/* Function to present the parity error context to Main module */
IX_STATUS
ixParityENAccSwcpPEParityErrorContextFetch(
    IxParityENAccSwcpPEParityErrorContext *ixSwcpPECMsg);

#endif /* IXPARITYENACCSPCPE_H */
