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

/* Function to unload the component */
IX_STATUS
ixParityENAccAqmPEUnload(void);

#endif /* IXPARITYENACCAQMPE_H */
