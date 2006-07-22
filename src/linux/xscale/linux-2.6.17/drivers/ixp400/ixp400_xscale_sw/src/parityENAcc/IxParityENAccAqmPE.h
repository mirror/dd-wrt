/**
 * @file IxParityENAccAqmPE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for AQM Parity Detection Enabler sub-component
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

#ifndef IXPARITYENACCAQMPE_H
#define IXPARITYENACCAQMPE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * Typedefs used in this file
 */

/* Source of the parity error notification in AQM */
typedef enum  /* IxParityENAccAqmPEParityErrorSource  */
{
    IXP400_PARITYENACC_PE_AQM = IX_PARITYENACC_AQM
} IxParityENAccAqmPEParityErrorSource;

/* AQM parity detection configuration parameter */
typedef IxParityENAccPEConfigOption IxParityENAccAqmPEConfigOption;

/* AQM Parity Error Context Message */
typedef struct /* IxParityENAccAqmPEParityErrorContext */
{
    /* AQM source info of parity error */
    IxParityENAccAqmPEParityErrorSource  aqmParitySource;

    /* Read Always */
    IxParityENAccPEParityErrorAccess  aqmAccessType;

    /* Address faulty location in AQM SRAM */
    IxParityENAccPEParityErrorAddress  aqmParityAddress;

    /* Data read from the faulty location */
    IxParityENAccPEParityErrorData  aqmParityData;
} IxParityENAccAqmPEParityErrorContext;


/*
 * Local functions declarations
 *
 * NOTE: They are to be made available to the Main sub-component only
 */

/* Function for Initialisation */
IX_STATUS
ixParityENAccAqmPEInit (IxParityENAccInternalCallback ixAqmPECallback);

/* Function to Enable/Disable parity error detection in AQM */
IX_STATUS
ixParityENAccAqmPEDetectionConfigure(IxParityENAccAqmPEConfigOption ixAqmPDCfg);

/* Function to present the parity error context to Main module */
IX_STATUS
ixParityENAccAqmPEParityErrorContextFetch 
    (IxParityENAccAqmPEParityErrorContext *ixAqmPECMsg);

/* Function to clear off the parity error interrupt */
IX_STATUS
ixParityENAccAqmPEParityInterruptClear (void);

#endif /* IXPARITYENACCAQMPE_H */
