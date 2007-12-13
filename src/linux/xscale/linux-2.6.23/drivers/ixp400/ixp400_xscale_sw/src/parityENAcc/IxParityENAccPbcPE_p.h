/**
 * @file IxParityENAccPbcPE_p.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Private header file for PBC Parity Detection Enabler sub-
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

#ifndef IXPARITYENACCPBCPE_P_H
#define IXPARITYENACCPBCPE_P_H

#include "IxOsal.h"
/*
 * #defines and macros used in this file.
 */

/* IRQ Level for the PBC */
#define IRQ_IXP400_INTC_PARITYENACC_PBC  (IX_OSAL_IXP400_PCI_INT_IRQ_LVL)

/*
 * Base Addresses for PBC Control and Status Registers (CSR Port Registers)
 * 
 * The offset of 0x10 is for the AD_CBE register needed for PBC sub-component
 * access 
 */
#define IXP400_PARITYENACC_PBC_PCICSR_BASEADDR   (IX_OSAL_IXP400_PCI_CFG_PHYS_BASE + 0x10)

/*
 * Address Range for PBC registers
 * (Memory size = Register size x Number of registers)
 */
#define IXP400_PARITYENACC_PBC_PCICSR_MEMMAP_SIZE   (0x04 * 0x06)

/* PBC registers Relative Offset values */
#define IXP400_PARITYENACC_PBC_CRP_AD_CBE_OFFSET    (0x00)
#define IXP400_PARITYENACC_PBC_CRP_WDATA_OFFSET     (0x04)
#define IXP400_PARITYENACC_PBC_CRP_RDATA_OFFSET     (0x08)
#define IXP400_PARITYENACC_PBC_CSR_OFFSET           (0x0C)
#define IXP400_PARITYENACC_PBC_ISR_OFFSET           (0x10)
#define IXP400_PARITYENACC_PBC_INTEN_OFFSET         (0x14)

/*
 * Mask values to access PBC Control and Status Registers
 */

/* Masks for bits in PBC Control and Status Register */
#define IXP400_PARITYENACC_PBC_CSR_IC           (1 << 15)
#define IXP400_PARITYENACC_PBC_CSR_HOST         (1)

/* Masks for bits in PBC Interrupt Status Register */
#define IXP400_PARITYENACC_PBC_ISR_PPE          (1 << 2)

/* Masks for bits in PBC Interrupt Enable Register */
#define IXP400_PARITYENACC_PBC_INTEN_PPE        (1 << 2)

/*
 * Macros to access the Parity Error detection configuration
 * on the PCI Controller PCI Config Register - SRCR
 */

/* PCI Controller PCI Config Register - SRCR Address */
#define IXP400_PARITYENACC_PBC_CRP_AD_CBE_SRCR   (0x04)

/* PCI Controller Command Byte Enables (0 Enabled) */
#define IXP400_PARITYENACC_PBC_CRP_AD_CBE_BE0    (0 << 20)
#define IXP400_PARITYENACC_PBC_CRP_AD_CBE_BE1    (0 << 21)
#define IXP400_PARITYENACC_PBC_CRP_AD_CBE_BE2    (0 << 22)
#define IXP400_PARITYENACC_PBC_CRP_AD_CBE_BE3    (0 << 23)

/* PCI Controller Commands */
#define IXP400_PARITYENACC_PBC_CRP_AD_CBE_CMD_READ   (0 << 16)
#define IXP400_PARITYENACC_PBC_CRP_AD_CBE_CMD_WRITE  (1 << 16)

/*  Mask for PCI Controller PCI function bits [10:8] */
#define IXP400_PARITYENACC_PBC_CRP_AD_CBE_FN     (0xFFFFF8FF)

/* Masks for bits in the PCI Config Register - SRCR */
#define IXP400_PARITYENACC_PBC_PCICFG_SRCR_DPE   (1 << 31)
#define IXP400_PARITYENACC_PBC_PCICFG_SRCR_MDPE  (1 << 24)
#define IXP400_PARITYENACC_PBC_PCICFG_SRCR_PER   (1 << 6)

/* Helper macro for both Read & Write Ops of PCI Config Register - SRCR */
#define IXP400_PARITYENACC_PBC_PCICSR_SRCR_RDWR        \
            (IXP400_PARITYENACC_PBC_CRP_AD_CBE_SRCR  | \
              IXP400_PARITYENACC_PBC_CRP_AD_CBE_BE0  | \
              IXP400_PARITYENACC_PBC_CRP_AD_CBE_BE1  | \
              IXP400_PARITYENACC_PBC_CRP_AD_CBE_BE2  | \
              IXP400_PARITYENACC_PBC_CRP_AD_CBE_BE3) 

/* Helper macro to Read from the PCI Config Register - SRCR */
#define IXP400_PARITYENACC_PBC_PCICSR_SRCR_READ         \
			(IXP400_PARITYENACC_PBC_PCICSR_SRCR_RDWR  | \
             IXP400_PARITYENACC_PBC_CRP_AD_CBE_CMD_READ)

/* Helper macro to Write onto the PCI Config Register - SRCR */
#define IXP400_PARITYENACC_PBC_PCICSR_SRCR_WRITE        \
			(IXP400_PARITYENACC_PBC_PCICSR_SRCR_RDWR  | \
             IXP400_PARITYENACC_PBC_CRP_AD_CBE_CMD_WRITE)

/*
 * Typedefs used in the PBC sub-module
 */

/* PBC Registers virutal addresses */
typedef struct  /* IxParityENAccPbcPERegisters */
{
    UINT32  pciInten;    /* PCI Controller Interrupt Enable Register 
                          * virtual address
                          */
    UINT32  pciIsr;      /* PCI Controller Interrupt Status Register 
                          * virtual address
                          */
    UINT32  pciCsr;      /* PCI Controller Control and Status Register 
                          * virtual address
                          */
    UINT32  pciCrpAdCbe; /* PCI Controller Configuration Port 
                          * Address/Command/Byte Enables Register 
                          * virtual address
                          */
    UINT32  pciCrpWdata; /* PCI Controller Configuration Port 
                          * Write Data Register virtual address
                          */
    UINT32  pciCrpRdata; /* PCI Controller Configuration Port
                          * Read Data Register virtual address
                          */
} IxParityENAccPbcPERegisters;

/* PBC ISR type */  
typedef IxParityENAccPEIsr IxParityENAccPbcPEIsr;

/* PBC Interrupt Service Routine Info for debug purpose */
typedef struct  /* IxParityENAccPbcPEISRInfo */
{
    UINT32  pbcInterruptId;       /* PCI Bus Controller Interrupt Identifier */
    IxParityENAccPbcPEIsr pbcIsr; /* ISR for handling interrupts */
} IxParityENAccPbcPEIsrInfo;

/* PBC Parity Error Status */
typedef struct  /* IxParityENAccPbcPEParityErrorStatus */
{
    UINT32  pciIntenValue;  /* PCI Controller Interrupt Enable Register contents */
    UINT32  pciIsrValue;    /* PCI Controller Interrupt Status Register contents */
    UINT32  pciCsrValue;    /* PCI Controller Control and Status Register contents */
    UINT32  pciSrcrValue;   /* PCI Status Register/Control Register contents */
} IxParityENAccPbcPEParityErrorStatus;

/* PBC Configuration Information */
typedef struct  /* IxParityENAccPbcPEConfig */
{
     /* PCI Controller Control and Status Registers */
    IxParityENAccPbcPERegisters  pbcPERegisters;

     /* Contents of PCI Controller Status and Control Register */
    IxParityENAccPbcPEParityErrorStatus  pbcParityErrorStatus;

    /* Internal Callback Routine */
    IxParityENAccInternalCallback  pbcPECallback;

    /* Interrupt Service Routine Info */
    IxParityENAccPbcPEIsrInfo  pbcIsrInfo;
} IxParityENAccPbcPEConfig;


/*
 * Variable declarations
 */

static IxParityENAccPbcPEConfig ixParityENAccPbcPEConfig =
{
    /* PBC registers virtual addresses */
    { 0,0,0,0,0,0 },
    /* PBC parity error info/status */
    { 0,0,0,0 },
    /* PBC internal callback */
    (IxParityENAccInternalCallback) NULL,
    /* PBC ISR Info */
    { 0, (IxParityENAccPbcPEIsr) NULL }
};

/*
 * Local functions declarations
 */

void 
ixParityENAccPbcPEIsr(void);

/* Get parity error status into internal datastructures */
void
ixParityENAccPbcPEParityErrorStatusGet (void);

/* Set parity error interrupt status to clear */
void
ixParityENAccPbcPEParityErrorStatusClear (void);

#endif /* IXPARITYENACCPBCPE_P_H */
