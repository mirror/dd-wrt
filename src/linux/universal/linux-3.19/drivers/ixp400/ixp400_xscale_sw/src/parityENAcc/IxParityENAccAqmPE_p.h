/**
 * @file IxParityENAccAqmPE_p.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Private header file for AQM Parity Detection Enabler sub-
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

#ifndef IXPARITYENACCAQMPE_P_H
#define IXPARITYENACCAQMPE_P_H

#include "IxOsal.h"

/*
 * #defines and macros used in this file.
 */

/* IRQ Level for the AQM */
#define IRQ_IXP400_INTC_PARITYENACC_AQM  (IX_OSAL_IXP400_AQM_IRQ_LVL)

/*
 * Base Addresses for AQM Address and Control Registers
 *
 * The offset 0x0460 is for the Address and Control Register from 
 * the base address of the AQM error processing 
 */

#define IXP400_PARITYENACC_AQM_BASEADDR  (IX_OSAL_IXP400_QMGR_PHYS_BASE + 0x0460)
/*
 * Address Range for AQM registers
 * (Memory size = Register size x Number of registers)
 */
#define IXP400_PARITYENACC_AQM_MEMMAP_SIZE        (0x04 * 0x02)

/* AQM registers Relative Offset values */
#define IXP400_PARITYENACC_AQM_QUEADDRERR_OFFSET  (0x00)
#define IXP400_PARITYENACC_AQM_QUEDATAERR_OFFSET  (0x04)


/*
 * Mask values to access AQM registers
 */

/* Masks for bits in AQM Address/Control Register */
#define IXP400_PARITYENACC_AQM_QUEADDRERR_PERR_ENABLE   (1 << 15)
#define IXP400_PARITYENACC_AQM_QUEADDRERR_PERR_FLAG     (1 << 16)
#define IXP400_PARITYENACC_AQM_QUEADDRERR_PERR_ADDRESS  (0x03FF)


/*
 * Typedefs used in this file
 */

/* AQM Control and Error Address Registers virutal addresses */
typedef struct  /* IxParityENAccAqmPERegisters */
{
    UINT32  aqmQueAddErr;  /* AQM SRAM Parity Error Address/Control Register */
    UINT32  aqmQueDataErr; /* AQM SRAM Error Data Register */
} IxParityENAccAqmPERegisters;

/* AQM ISR type */
typedef IxParityENAccPEIsr IxParityENAccAqmPEIsr;

/* AQM Interrupt Service Routine  */
typedef struct  /* IxParityENAccAqmPEISRInfo */
{
    UINT32  aqmInterruptId;          /* AQM Interrupt Identifier */
    IxParityENAccAqmPEIsr  aqmIsr;  /* ISR for handling interrupts */
} IxParityENAccAqmPEIsrInfo;

/* AQM Parity Error Status */
typedef struct  /* IxParityENAccAqmPEParityErrorStatus */
{
    UINT32  aqmQueAddErrValue;  /* AQM SRAM Parity Error
                                 * Address and Control Register Contents 
                                 */
    UINT32  aqmQueDataErrValue; /* AQM SRAM Error Data Register Contents */
} IxParityENAccAqmPEParityErrorStatus;

/* AQM Configuration Information */
typedef struct  /* IxParityENAccAqmPEConfig  */
{
    /* AQM Control and Status Register(s) virtual addresses */
    IxParityENAccAqmPERegisters  aqmPERegisters;

    /* Contents of Status and Control Register */
    IxParityENAccAqmPEParityErrorStatus  aqmParityErrorStatus;

    /* Internal callback routine */
    IxParityENAccInternalCallback  aqmPECallback;

    /* Interrupt service routine Info */
    IxParityENAccAqmPEIsrInfo  aqmIsrInfo;
} IxParityENAccAqmPEConfig;

/*
 * Variable declarations
 */
static IxParityENAccAqmPEConfig ixParityENAccAqmPEConfig =
{
    { 0,0 },  /* AQM registers */
    { 0,0 },  /* AQM parity error info */
    (IxParityENAccInternalCallback) NULL,  /* AQM internal callback */
    { 0, (IxParityENAccAqmPEIsr) NULL }    /* AQM ISR Info */
};
 
/*
 * AQM sub-module local functions declarations
 */

/* AQM interrupt service routine */
void
ixParityENAccAqmPEIsr(void);

/* Get parity error status into internal datastructures */
IX_STATUS
ixParityENAccAqmPEParityErrorStatusGet (void);

#endif /* IXPARITYENACCAQMPE_P_H */
