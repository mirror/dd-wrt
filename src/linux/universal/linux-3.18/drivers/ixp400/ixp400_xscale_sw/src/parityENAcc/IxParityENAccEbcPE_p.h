/**
 * @file IxParityENAccEbcPE_p.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Private header file for EBC Parity Detection Enabler sub-
 * component of the IXP400 Parity Error Notifier access component.
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

#ifndef IXPARITYENACCEBCPE_P_H
#define IXPARITYENACCEBCPE_P_H

#include "IxOsal.h"

/*
 * #defines and macros used in this file.
 */
/* IRQ Level for the EBC */
#if !defined(__ixp43X)
#define IRQ_IXP400_INTC_PARITYENACC_EBC   (IX_OSAL_IXP400_EBC_IRQ_LVL)
#else /* define dummy values for IXP 43X */
#define IRQ_IXP400_INTC_PARITYENACC_EBC   0
#endif

/* Base Addresses for EBC Control and Status Registers */
#if !defined(__ixp43X)
#define IXP400_PARITYENACC_EBC_BASEADDR   (IX_OSAL_IXP400_EXP_BUS_REGS_PHYS_BASE)
#else /* define dummy value for IXP 43X */
#define IXP400_PARITYENACC_EBC_BASEADDR   0
#endif

/*
 * Address Range for EBC registers 
 *
 * (Memory size = Register size x Number of registers)
 *
 * The address range contain the registers addresses which are not
 * required for parity error detection configuration and reporting
 * the status but included here for easier access of other registers
 */
#define IXP400_PARITYENACC_EBC_MEMMAP_SIZE          (0x04 * 0x10)

/* EBC registers Relative Offset values */
#define IXP400_PARITYENACC_EBC_TIMING_CS0_OFFSET    (0x00)
#define IXP400_PARITYENACC_EBC_TIMING_CS1_OFFSET    (0x04)
#define IXP400_PARITYENACC_EBC_TIMING_CS2_OFFSET    (0x08)
#define IXP400_PARITYENACC_EBC_TIMING_CS3_OFFSET    (0x0C)
#define IXP400_PARITYENACC_EBC_TIMING_CS4_OFFSET    (0x10)
#define IXP400_PARITYENACC_EBC_TIMING_CS5_OFFSET    (0x14)
#define IXP400_PARITYENACC_EBC_TIMING_CS6_OFFSET    (0x18)
#define IXP400_PARITYENACC_EBC_TIMING_CS7_OFFSET    (0x1C)
#define IXP400_PARITYENACC_EBC_MST_CONTROL_OFFSET   (0x0100)
#define IXP400_PARITYENACC_EBC_PARITY_STATUS_OFFSET (0x0120)

/*
 * Mask values to access EBC registers
 */

/* Masks for bits in EBC Master Control Register */
#define IXP400_PARITYENACC_EBC_MST_CONTROL_ODDPARITY    (1 << 3)
#define IXP400_PARITYENACC_EBC_MST_CONTROL_INPAR_EN     (1 << 2)

/* Masks for bits in EBC ChipSelect Timing and Control Register */
#define IXP400_PARITYENACC_EBC_TIMING_CSX_EN            (1 << 31)
#define IXP400_PARITYENACC_EBC_TIMING_CSX_PAR_EN        (1 << 30)

/* Masks for bits in EBC Parity Status Register */
#define IXP400_PARITYENACC_EBC_PARITY_STATUS_ERRADDR    (0xFFFFFFFC)
#define IXP400_PARITYENACC_EBC_PARITY_STATUS_INERRSTS   (1 << 1)
#define IXP400_PARITYENACC_EBC_PARITY_STATUS_OUTERRSTS  (1 << 0)

/*
 * Typedefs used in EBC sub-module
 */

/* EBC Registers virtual addresses */
typedef struct  /* IxParityENAccEbcPERegisters */
{
    /* Expansion Bus Controller Timing and Control Register 
     * & Chip Selects virtual addresses
     */
    UINT32  expTimingCs[IXP400_PARITYENACC_PE_EBC_CHIPSEL_MAX];

    UINT32  expMstControl;   /* Expansion Bus Controller 
                              * External Master Control Register 
                              * virtual address 
                              */
    UINT32  expParityStatus; /* Expansion Bus Controller Parity
                              * Status Register virtual address
                              */
} IxParityENAccEbcPERegisters;

/* EBC ISR type */  
typedef IxParityENAccPEIsr IxParityENAccEbcPEIsr;

/* EBC Interrupt Service Routine Info */
typedef struct  /* IxParityENAccEbcPEISRInfo */
{
    UINT32  ebcInterruptId;  /* Expansion Bus Controller 
                              * Interrupt Identifier
                              */
    IxParityENAccEbcPEIsr  ebcIsr;  /* ISR for handling interrupts */
} IxParityENAccEbcPEIsrInfo;

/* EBC Parity Error Status */
typedef struct  /* IxParityENAccEbcPEParityErrorStatus */
{
    /* Expansion Bus Controller Timing and Control Register
     * for Chip Selects contents
     */
    UINT32  expTimingCsValue[IXP400_PARITYENACC_PE_EBC_CHIPSEL_MAX];
    UINT32  expMstControlValue;   /* Expansion Bus Controller
                                   * External Master Control Register contents
                                   */
    UINT32  expParityStatusValue; /* Expansion Bus Controller Parity Status 
                                   * Register contents
                                   */
} IxParityENAccEbcPEParityErrorStatus;

/* EBC Configuration Information */
typedef struct  /* IxParityENAccEbcPEConfig */
{
    /* EBC Control and Status Registers */
    IxParityENAccEbcPERegisters  ebcPERegisters;

    /* Contents of EBC Controller Status and Control Register */
    IxParityENAccEbcPEParityErrorStatus  ebcParityErrorStatus;

    /* Interrupt Service Routine Info */
    IxParityENAccEbcPEIsrInfo  ebcIsrInfo;

    /* Internal Callback Routine */
    IxParityENAccInternalCallback  ebcPECallback;
} IxParityENAccEbcPEConfig;

/*
 * Variable declarations
 */
static IxParityENAccEbcPEConfig ixParityENAccEbcPEConfig =
{
    /* EBC registers virtual addresses */
    { { 0,0,0,0,0,0,0,0 }, 0, 0 },
    /* EBC parity error info */
    { { 0,0,0,0,0,0,0,0 }, 0, 0 },
    /* EBC ISR Info */
    { 0, (IxParityENAccEbcPEIsr) NULL },
    /* EBC internal callback */
    (IxParityENAccInternalCallback) NULL
};

 
/*
 * Local functions declarations
 */
void 
ixParityENAccEbcPEIsr(void);

/* Get parity error status into internal datastructures */
void
ixParityENAccEbcPEParityErrorStatusGet (void);

#endif /* IXPARITYENACCEBCPE_P_H */
