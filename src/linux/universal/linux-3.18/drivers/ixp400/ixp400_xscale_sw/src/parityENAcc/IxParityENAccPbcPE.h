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

IX_STATUS
ixParityENAccPbcPEUnload(void);

#endif /* IXPARITYENACCPBCPE_H */
