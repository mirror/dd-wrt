/**
 * @file IxParityENAccMcuPE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for MCU Parity Detection Enabler sub-component
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

#ifndef IXPARITYENACCMCUPE_H
#define IXPARITYENACCMCUPE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * Typedefs used in this file
 */

/* MCU pairty detection configuration parameter */
typedef struct /* IxParityENAccMcuPEConfigOption */
{
    /* Single-bit parity error detection */
    IxParityENAccPEConfigOption singlebitDetectEnabled;

    /* Single-bit parity error correction */
    IxParityENAccPEConfigOption singlebitCorrectionEnabled;

    /* Multi-bit parity error detection */
    IxParityENAccPEConfigOption multibitDetectionEnabled;  
} IxParityENAccMcuPEConfigOption;

/* Source of the parity error notification in DDR MCU */
typedef enum  /* IxParityENAccMcuPEParityErrorSource  */
{
    /* Single bit parity */
    IXP400_PARITYENACC_PE_MCU_SBIT = IX_PARITYENACC_MCU_SBIT,

    /* Multi bit parity */
    IXP400_PARITYENACC_PE_MCU_MBIT = IX_PARITYENACC_MCU_MBIT,

    /* Parity errors in excess of two */
    IXP400_PARITYENACC_PE_MCU_OVERFLOW = IX_PARITYENACC_MCU_OVERFLOW,

    /* No Parity */
    IXP400_PARITYENACC_PE_MCU_NOPARITY = IX_PARITYENACC_NO_PARITY
} IxParityENAccMcuPEParityErrorSource;

/*
 * The requester interface through which the SDRAM memory access 
 * resulted in the parity error.
 */
typedef enum  /* IxParityENAccPEParityErrorRequester */
{
    /* Intel XScale Core BIU */
    IXP400_PARITYENACC_PE_MCU_MPI = IX_PARITYENACC_MPI,

    /* IB BUS (South or North AHB Bus) */
    IXP400_PARITYENACC_PE_MCU_AHB_BUS = IX_PARITYENACC_AHB_BUS
} IxParityENAccPEParityErrorRequester;

/* MCU Parity Error Context Message */
typedef struct /* IxParityENAccMcuPEParityErrorContext */
{
    /* MCU source info of parity error */
    IxParityENAccMcuPEParityErrorSource  mcuParitySource;

    /* Read or Write Access */
    IxParityENAccPEParityErrorAccess  mcuAccessType;

    /* Address faulty location in MCU SDRAM */
    IxParityENAccPEParityErrorAddress  mcuParityAddress;

    /* The faulty bit of the Single-bit parity */
    IxParityENAccPEParityErrorData  mcuParityData;

    /*  MCU port used for the SDRAM access */
    IxParityENAccPEParityErrorRequester  mcuRequester;
} IxParityENAccMcuPEParityErrorContext;

/*
 * Local functions declarations
 * NOTE: They are to be made available to the Main sub-component only
 */

/* Initialise the MCU sub-module */
IX_STATUS
ixParityENAccMcuPEInit (IxParityENAccInternalCallback ixMcuPECallback);

/* Configure/Re-configure MCU parity error detection enable/disable */
IX_STATUS 
ixParityENAccMcuPEDetectionConfigure (IxParityENAccMcuPEConfigOption ixMcuPDCfg);

/* Retrieve the MCU parity error context */
IX_STATUS 
ixParityENAccMcuPEParityErrorContextFetch (
    IxParityENAccMcuPEParityErrorContext *ixMcuPECMsg);

/* Clear single/multi-bit and overflow parity interrupt conditions */
IX_STATUS
ixParityENAccMcuPEParityInterruptClear (
    IxParityENAccMcuPEParityErrorSource ixMcuParityErrSrc,
    IxParityENAccPEParityErrorAddress ixMcuParityErrAddress);

/* Function to unload the component */
IX_STATUS
ixParityENAccMcuPEUnload(void);

#endif /* IXPARITYENACCMCUPE_H */
