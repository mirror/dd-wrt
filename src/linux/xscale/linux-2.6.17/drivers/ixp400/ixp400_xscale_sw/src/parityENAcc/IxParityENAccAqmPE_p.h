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
