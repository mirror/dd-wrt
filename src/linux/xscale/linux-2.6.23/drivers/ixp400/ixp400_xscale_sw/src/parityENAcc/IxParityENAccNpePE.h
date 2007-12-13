/**
 * @file IxParityENAccNpePE.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Header file for NPE Parity Detection Enabler sub-component
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

#ifndef IXPARITYENACCNPEPE_H
#define IXPARITYENACCNPEPE_H

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"

/*
 * typedefs used in this file
 */

/* NPE parity detection configuration parameters */
typedef struct /* IxParityENAccNpePEConfigOption */
{
    IxParityENAccPEConfigOption ideEnabled; /* IMem, DMem, External Error */
    IxParityENAccParityType parityOddEven;  /* Parity - Odd or Even */
} IxParityENAccNpePEConfigOption;

/* NPE Identifiers */
typedef enum  /* IxParityENAccPENpeId */
{
    IXP400_PARITYENACC_PE_NPE_A,     /* NPE - A */
    IXP400_PARITYENACC_PE_NPE_B,     /* NPE - B */
    IXP400_PARITYENACC_PE_NPE_C,     /* NPE - C */
    IXP400_PARITYENACC_PE_NPE_MAX    /* MAX # of NPEs */
} IxParityENAccPENpeId;

/* Source of the parity error */
typedef enum  /* IxParityENAccNpePEParityErrorSource  */
{
    IXP400_PARITYENACC_PE_NPE_IMEM,  /* Instruction memory */
    IXP400_PARITYENACC_PE_NPE_DMEM,  /* Data memory */
    IXP400_PARITYENACC_PE_NPE_EXT    /* External Error */
} IxParityENAccNpePEParityErrorSource;

/* NPE Parity Error Context Message */
typedef struct /* IxParityENAccNpePEParityErrorContext */
{
    /* NPE source info of parity error */
    IxParityENAccNpePEParityErrorSource   npeParitySource;

    /* Read Always */
    IxParityENAccPEParityErrorAccess      npeAccessType;
} IxParityENAccNpePEParityErrorContext;

/*
 * Local functions declarations accessible to Main sub-module
 */

/* Function for Initialisation */
IX_STATUS
ixParityENAccNpePEInit (IxParityENAccInternalCallback ixNpePECallback);

/* Function to Enable/Disable parity error detection in NPE */
IX_STATUS
ixParityENAccNpePEDetectionConfigure (
    IxParityENAccPENpeId ixNpeId,
    IxParityENAccNpePEConfigOption ixNpePDCfg);

/* Function to present the parity error context to Main module */
IX_STATUS
ixParityENAccNpePEParityErrorContextFetch (
    IxParityENAccPENpeId ixNpeId,
    IxParityENAccNpePEParityErrorContext *ixNpePECMsg);

/* Function to clear off the parity error interrupt */
IX_STATUS
ixParityENAccNpePEParityInterruptClear (IxParityENAccPENpeId ixNpeId);

/* Function to unload the component */
IX_STATUS
ixParityENAccNpePEUnload(void);

#endif /* IXPARITYENACCNPEPE_H */
