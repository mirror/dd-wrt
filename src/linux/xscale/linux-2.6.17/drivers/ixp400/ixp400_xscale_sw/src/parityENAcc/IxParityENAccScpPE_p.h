/**
 * @file IxParityENAccSpcPE_p.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Private header file for SWCP Parity Detection Enabler sub-
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

#ifndef IXPARITYENACCSPCPE_P_H
#define IXPARITYENACCSPCPE_P_H

#include "IxOsal.h"

/*
 * #defines and macros used in this file.
 */

/* IRQ Level for the SWCP */
#define IRQ_IXP400_INTC_PARITYENACC_SWCP  IX_OSAL_IXP400_SWCP_IRQ_LVL


/*
 * Typedefs used in the SWCP sub-module
 */

/* SWCP ISR Type */  
typedef IxParityENAccPEIsr IxParityENAccSwcpPEIsr;

/* SWCP Interrupt Service Routine */
typedef struct  /* IxParityENAccSwcpPEIsrInfo */
{
    UINT32  swcpInterruptId;        /* SWCP Interrupt Identifier */
    IxParityENAccSwcpPEIsr swcpIsr; /* ISR for handling interrupts */
} IxParityENAccSwcpPEIsrInfo;

/* SWCP Configuration Information */
typedef struct  /* IxParityENAccSwcpPEConfig */
{
    /* Internal Callback Routine */
    IxParityENAccInternalCallback  swcpPECallback;

    /* Interrupt Service Routine Info */
    IxParityENAccSwcpPEIsrInfo  swcpIsrInfo;
} IxParityENAccSwcpPEConfig;

/*
 * Variable declarations
 */

static IxParityENAccSwcpPEConfig ixParityENAccSwcpPEConfig =
{
    /* SWCP internal callback */
    (IxParityENAccInternalCallback) NULL,
    /* SWCP ISR Info */
    { 0, (IxParityENAccSwcpPEIsr) NULL }
};
 
/*
 * Local functions declarations
 */

/* SWCP interrupt service routine */
void
ixParityENAccSwcpPEIsr(void);

#endif /* IXPARITYENACCSPCPE_P_H */
