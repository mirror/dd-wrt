/**
 * @file IxParityENAccIcE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for Interrupt Controller Enabler sub-component of the 
 * IXP400 Parity Error Notifier access component.
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

#ifndef IXPARITYENACCICE_H
#define IXPARITYENACCICE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"


/*
 * Typedefs used in this file
 */

/* Parity Interrupt Identifiers */
typedef enum /* IxParityENAccIcParityInterruptId */
{
    /* Priority - 0: MCU Parity Interrupt */
    IXP400_PARITYENACC_INTC_MCU_PARITY_INTERRUPT,

    /* Priority - 1: NPE-A Parity Interrupt */
    IXP400_PARITYENACC_INTC_NPEA_PARITY_INTERRUPT,

    /* Priority - 2: NPE-B Parity Interrupt */
    IXP400_PARITYENACC_INTC_NPEB_PARITY_INTERRUPT,

    /* Priority - 3: NPE-C Parity Interrupt */
    IXP400_PARITYENACC_INTC_NPEC_PARITY_INTERRUPT,

    /* Priority - 4: Switching Coprocessor Parity Interrupt */
    IXP400_PARITYENACC_INTC_SWCP_PARITY_INTERRUPT,

    /* Priority - 5: Queue Manager Parity Interrupt */
    IXP400_PARITYENACC_INTC_AQM_PARITY_INTERRUPT,

    /* Priority - 6: PCI Bus Controller Parity Interrupt */
    IXP400_PARITYENACC_INTC_PBC_PARITY_INTERRUPT,

    /* Priority - 7: Expansion Bus Controller Parity Interrupt */
    IXP400_PARITYENACC_INTC_EBC_PARITY_INTERRUPT
} IxParityENAccIcParityInterruptId;

/* Interrupt Controller Parity Interrupts Status */
typedef struct /* IxParityENAccIcParityInterruptStatus */
{
    /* Priority - 0: MCU Parity Interrupt */
    BOOL  mcuParityInterrupt;

    /* Priority - 1: NPE-A Parity Interrupt */
    BOOL  npeAParityInterrupt;

    /* Priority - 2: NPE-B Parity Interrupt */
    BOOL  npeBParityInterrupt;

    /* Priority - 3: NPE-C Parity Interrupt */
    BOOL  npeCParityInterrupt;

    /* Priority - 4: Switching Coprocessor Parity Interrupt */
    BOOL  swcpParityInterrupt;

    /* Priority - 5: Queue Manager Parity Interrupt */
    BOOL  aqmParityInterrupt;

    /* Priority - 6: PCI Bus Controller Parity Interrupt */
    BOOL  pbcParityInterrupt;

    /* Priority - 7: Expansion Bus Controller Parity Interrupt */
    BOOL  ebcParityInterrupt;
} IxParityENAccIcParityInterruptStatus;

 
/*
 * Local functions declarations
 *
 * NOTE: They are to be made available to the Main sub-component only
 */

/* Function for Initialisation */
IX_STATUS
ixParityENAccIcEInit (void);

/* Function to get the pending interrupts status into local datastructures */
IX_STATUS
ixParityENAccIcInterruptStatusGet(
    IxParityENAccIcParityInterruptStatus *ixIcParityInterruptStatus);

/* Function to disable a particular parity error interrupt */
IX_STATUS
ixParityENAccIcInterruptDisable (IxParityENAccIcParityInterruptId ixIcParityIntrId);

/* Function to enable a particular parity error interrupt */
IX_STATUS
ixParityENAccIcInterruptEnable (IxParityENAccIcParityInterruptId ixIcParityIntrId);

#endif /* IXPARITYENACCICE_H */
