/**
 * @file IxParityENAccIcE_p.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Private header file for Interrupt Controller Enabler sub-component
 * of the IXP400 Parity Error Notifier access component.
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

#ifndef IXPARITYENACCICE_P_H
#define IXPARITYENACCICE_P_H

#include "IxOsal.h"

/*
 * #defines and macros used in this file.
 */

/* Base Addresses for Interrupt Controller Registers */
#define IXP400_PARITYENACC_INTC_BASEADDR     (IX_OSAL_IXP400_INTC_PHYS_BASE)

/* 
 * Address Range for Interrupt Controller registers
 * (Memory size = Register size x Number of registers)
 */
#define IXP400_PARITYENACC_INTC_MEMMAP_SIZE  (0x04 * 0x0E)

/* INTC registers Relative Offset values */
#define IXP400_PARITYENACC_INTC_ST_OFFSET         (0x00)
#define IXP400_PARITYENACC_INTC_EN_OFFSET         (0x04)
#define IXP400_PARITYENACC_INTC_IRQ_ST_OFFSET     (0x0C)
#define IXP400_PARITYENACC_INTC_ST2_OFFSET        (0x20)
#define IXP400_PARITYENACC_INTC_EN2_OFFSET        (0x24)
#define IXP400_PARITYENACC_INTC_IRQ_ST2_OFFSET    (0x2C)
#define IXP400_PARITYENACC_INTC_ERROR_EN2_OFFSET  (0x34)

/*
 * Mask values to access INTC Registers
 */

/* Masks for bits in INTC Select/Enable/Status Register */
#define IXP400_PARITYENACC_INTC_NPEA   (1 << IX_OSAL_IXP400_NPEA_IRQ_LVL)
#define IXP400_PARITYENACC_INTC_NPEB   (1 << IX_OSAL_IXP400_NPEB_IRQ_LVL)
#define IXP400_PARITYENACC_INTC_NPEC   (1 << IX_OSAL_IXP400_NPEC_IRQ_LVL)
#define IXP400_PARITYENACC_INTC_PBC    (1 << IX_OSAL_IXP400_PCI_INT_IRQ_LVL)

/* Masks for bits in INTC Select/Enable/Status/Error Register #2 */
#define IXP400_PARITYENACC_INTC_SWCP   (1 << (IX_OSAL_IXP400_SWCP_IRQ_LVL - 32))
#define IXP400_PARITYENACC_INTC_AQM    (1 << (IX_OSAL_IXP400_AQM_IRQ_LVL - 32))
#define IXP400_PARITYENACC_INTC_MCU    (1 << (IX_OSAL_IXP400_MCU_IRQ_LVL - 32))
#define IXP400_PARITYENACC_INTC_EBC    (1 << (IX_OSAL_IXP400_EBC_IRQ_LVL - 32))

/*
 * Typedefs used in this file
 */

/* Interrupt Controller Registers virtual addresses */
typedef struct  /* IxParityENAccIcERegisters */
{
    UINT32 intrSt;     /* Interrupt Status Register Address */
    UINT32 intrSt2;    /* Interrupt Status Register_2 Address */
    UINT32 intrEn;     /* Interrupt Enable Register Address */
    UINT32 intrEn2;    /* Interrupt Enable Register_2 Address */
    UINT32 intrIrqSt;  /* IRQ Status Register Address */
    UINT32 intrIrqSt2; /* IRQ Status Register_2 Address */
    UINT32 errorEn2;   /* Error Priority Enable Register Address */
} IxParityENAccIcERegisters;

/* Interrupt Controller Status */
typedef struct  /* IxParityENAccIcEParityInterruptStatus */
{
    UINT32 intrStValue;     /* Interrupt Status Register Contents */
    UINT32 intrSt2Value;    /* Interrupt Status Register_2 Contents */
    UINT32 intrEnValue;     /* Interrupt Enable Register Contents */
    UINT32 intrEn2Value;    /* Interrupt Enable Register_2 Contents */
    UINT32 intrIrqStValue;  /* IRQ Status register Contents */
    UINT32 intrIrqSt2Value; /* IRQ Status register_2 Contents */
    UINT32 errorEn2Value;   /* Error Priority Enable Register Contents */
} IxParityENAccIcEParityInterruptStatus;

/*
 *  The following data structure has been added to solve the problem with LSP
 *  turning off the enable-bits in the INTCEN and INTCEN2 registers.  Due to
 *  which the interrupt status get function has been modified to get the status
 *  based on the INTCST and INTCST2 registers only. Now this solution has the
 *  problem of returning the NPEs status even if their interrupts are actually
 *  have been disabled.  This data structue will maintain the status of the
 *  masked-off interrupts so that they are not reported until re-enabled.
 */
typedef struct  /* IxParityENAccIcEMaskedOffParityInterrupts */
{
	volatile BOOL npeAIntrSt;  /* Masked off staus of NPE-A interrupt */
	volatile BOOL npeBIntrSt;  /* Masked off staus of NPE-B interrupt */
	volatile BOOL npeCIntrSt;  /* Masked off staus of NPE-C interrupt */
	volatile BOOL aqmIntrSt;   /* Masked off staus of AQM interrupt */
	volatile BOOL swcpIntrSt;  /* Masked off staus of SWCP interrupt */
} IxParityENAccIcEMaskedOffParityInterrupts;

/*
 * Variable declarations
 */
static IxParityENAccIcERegisters ixParityENAccIcERegisters =
{
    0,0,0,0,0,0,0
};

static IxParityENAccIcEParityInterruptStatus ixParityENAccIcEParityIntrStatus =
{
    0,0,0,0,0,0,0
};

static IxParityENAccIcEMaskedOffParityInterrupts ixParityENAccIcEMaskedOffParityIntr =
{
    0,0,0,0,0
};

#endif /* IXPARITYENACCICE_P_H */
