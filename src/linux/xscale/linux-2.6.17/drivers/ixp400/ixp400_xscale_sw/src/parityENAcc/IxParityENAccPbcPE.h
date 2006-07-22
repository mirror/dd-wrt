/**
 * @file IxParityENAccPbcPE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for PBC Parity Detection Enabler sub-component
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

#ifndef IXPARITYENACCPBCPE_H
#define IXPARITYENACCPBCPE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * Typedefs used in this file
 */

/*
 * PCI bus controller parity detection configuration parameter 
 *
 * NOTE: PCI bus controller can be an Initiator or Target of PCI bus
 * transactions  but there is one single parity bit that controls the
 * parity detection on both the interfaces.  Since, it is possible to 
 * differentiate between the Initiator and Target interfaces by reading
 * some other bits in the registers, the software choose to implement 
 * the enabling or disabling of parity error detection. The main module
 * will implement these configuration and filtering capabilities but 
 * for the parity enabler sub-module it is simply enable or disable of 
 * parity detection.
 */
typedef IxParityENAccPEConfigOption IxParityENAccPbcPEConfigOption;

/* Source Interface of the parity error notification on PCI bus controller */
typedef enum  /* IxParityENAccPbcPEParityErrorSource  */
{
    /* PCI Bus Controller as Initiator */
    IXP400_PARITYENACC_PE_PBC_INITIATOR = IX_PARITYENACC_PBC_INITIATOR,

    /* PCI Bus Controller as Target */
    IXP400_PARITYENACC_PE_PBC_TARGET    = IX_PARITYENACC_PBC_TARGET
} IxParityENAccPbcPEParityErrorSource;

/* PCI Bus Controller Parity Error Context Message */
typedef struct /* IxParityENAccPbcPEParityErrorContext */
{
    /* PBC source info of parity error */
    IxParityENAccPbcPEParityErrorSource  pbcParitySource;

    /* Read or Write Access */
    IxParityENAccPEParityErrorAccess  pbcAccessType;
} IxParityENAccPbcPEParityErrorContext;

/*
 * Local functions declarations
 *
 * NOTE: They are to be made available to the Main sub-component only
 */

IX_STATUS
ixParityENAccPbcPEInit(IxParityENAccInternalCallback ixPbcPECallback);

IX_STATUS
ixParityENAccPbcPEDetectionConfigure(IxParityENAccPbcPEConfigOption ixPbcPDCfg);

IX_STATUS
ixParityENAccPbcPEParityErrorContextFetch(IxParityENAccPbcPEParityErrorContext *ixPbcPECMsg);

IX_STATUS
ixParityENAccPbcPEParityInterruptClear(IxParityENAccPbcPEParityErrorContext ixPbcPECMsg);

#endif /* IXPARITYENACCPBCPE_H */
