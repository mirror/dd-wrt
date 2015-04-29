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

/* Function to unload the component */
IX_STATUS
ixParityENAccIcEUnload(void);

/* Function to check for valid interrupt Id & to check boundary conditions */
IX_STATUS
ixParityENAccCheckIntIdValidity (IxParityENAccIcParityInterruptId ixIcParityIntrId);

#endif /* IXPARITYENACCICE_H */
