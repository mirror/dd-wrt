/**
 * @file IxParityENAccEbcPE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for EBC Parity Detection Enabler sub-component
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

#ifndef IXPARITYENACCEBCPE_H
#define IXPARITYENACCEBCPE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * Typedefs used in this file
 */

/* Configure parity error notification on Expansion bus controller */
typedef enum  /* IxParityENAccEbcPEParityConfigSource  */
{
    IXP400_PARITYENACC_PE_EBC_CS = IX_PARITYENACC_EBC_CS,        /* Chip Select */
    IXP400_PARITYENACC_PE_EBC_EXTMST = IX_PARITYENACC_EBC_EXTMST /* EBC Master */
} IxParityENAccEbcPEParityConfigSource;

/* Source of the parity error notification on Expansion bus controller */
typedef IxParityENAccEbcPEParityConfigSource IxParityENAccEbcPEParityErrorSource;

/* Chip Selects identifiers */
typedef enum  /* IxParityENAccChipSelectId */
{
    IXP400_PARITYENACC_PE_EBC_CHIPSEL0,    /* Chip Select - 0 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL1,    /* Chip Select - 1 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL2,    /* Chip Select - 2 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL3,    /* Chip Select - 3 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL4,    /* Chip Select - 4 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL5,    /* Chip Select - 5 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL6,    /* Chip Select - 6 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL7,    /* Chip Select - 7 */
    IXP400_PARITYENACC_PE_EBC_CHIPSEL_MAX  /* MAX # Chip Select */
} IxParityENAccChipSelectId;

/*
 * Expansion Bus Controller parity detection configuration parameter
 *
 * NOTE: Chip Select and EXT Master are mutually exclusive
 */
typedef struct /* IxParityENAccEbcPEConfigOption */
{
    /* Chip or EXT Master */
    IxParityENAccEbcPEParityConfigSource  ebcCsExtSource;

    union /* ebcInOrOutbound */
    {
        IxParityENAccPEConfigOption  ebcCsEnabled;     /* Chip Select */
        IxParityENAccPEConfigOption  ebcExtMstEnabled; /* EXT Master */
    } ebcInOrOutbound;

    /* Chip Select: Applicable only when chip select configuration */
    IxParityENAccChipSelectId  ebcCsId;

    /* Parity - Odd or Even applies to both CS and EXT Master */
    IxParityENAccParityType  parityOddEven;            
} IxParityENAccEbcPEConfigOption;

/* Expansion Bus Controller Parity Error Context Message */
typedef struct /* IxParityENAccEbcPEParityErrorContext */
{
    /* EBC source info of parity error */
    IxParityENAccEbcPEParityErrorSource  ebcParitySource;

    /* Outbound Read or Inbound Write Access */
    IxParityENAccPEParityErrorAccess  ebcAccessType;

    /* AHB bus Address [31:2] */
    IxParityENAccPEParityErrorAddress  ebcParityAddress;
} IxParityENAccEbcPEParityErrorContext;

/*
 * Local functions declarations
 *
 * NOTE: They are to be made available to the Main sub-component only
 */

/* Function for Initialisation */
IX_STATUS
ixParityENAccEbcPEInit (IxParityENAccInternalCallback ixEbcPECallback);

/* Function to Enable/Disable parity error detection in EBC */
IX_STATUS
ixParityENAccEbcPEDetectionConfigure(IxParityENAccEbcPEConfigOption ixEbcPDCfg);

/* Function to present the parity error context to Main module */
IX_STATUS 
ixParityENAccEbcPEParityErrorContextFetch(
    IxParityENAccEbcPEParityErrorContext *ixEbcPECMsg);

/* Function to clear off the parity error interrupt */
IX_STATUS
ixParityENAccEbcPEParityInterruptClear (
    IxParityENAccEbcPEParityConfigSource ixEbcParityErrSrc);

/* Function to unload the component */
IX_STATUS
ixParityENAccEbcPEUnload(void);

#endif /* IXPARITYENACCEBCPE_H */
