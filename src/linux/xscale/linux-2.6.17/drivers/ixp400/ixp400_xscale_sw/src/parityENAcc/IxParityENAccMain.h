/**
 * @file IxParityENAccMain.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief  Header file for the Main sub-component of the IXP400 Parity 
 * Error Notifier access component.
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

#ifndef IXPARITYENACCMAIN_H
#define IXPARITYENACCMAIN_H

/*
 * User defined include files
 */

#include "IxOsal.h"
#include "IxParityENAcc.h"

/*
 * #defines and macros used
 */

/* Inline or Non-Inlined function declaration/definition macro */
#ifdef NO_INLINE_APIS
    #define IXP400_PARITYENACC_INLINE  /* empty define */
#else /* else of ifdef NO_INLINE_APIS */
    #define IXP400_PARITYENACC_INLINE __inline__ extern
#endif /* end of ifdef NO_INLINE_APIS */

/*
 * Macros for memory mapped registers access and bit manipulations
 */
#define IXP400_PARITYENACC_REG_READ(regAddr, varRef)        \
do {                                                        \
    *(varRef) = IX_OSAL_READ_LONG(regAddr);               \
} while (0)

#define IXP400_PARITYENACC_REG_WRITE(regAddr, varValue)     \
do {                                                        \
    IX_OSAL_WRITE_LONG(regAddr, varValue);                \
} while (0)

#define IXP400_PARITYENACC_REG_BIT_SET(regAddr, bitMask)    \
do {                                                        \
    /* *(regAddr) |= (bitMask); */                          \
    IX_OSAL_WRITE_LONG(regAddr,                           \
        (IX_OSAL_READ_LONG(regAddr) | bitMask));          \
} while (0)

#define IXP400_PARITYENACC_REG_BIT_CLEAR(regAddr, bitMask)  \
do {                                                        \
    /* *(regAddr) &= ~(bitMask); */                         \
    IX_OSAL_WRITE_LONG(regAddr,                           \
        IX_OSAL_READ_LONG(regAddr) & ~(bitMask));         \
} while (0)

#define IXP400_PARITYENACC_REG_BIT_CHECK(regAddr, bitMask)  \
    ((IX_OSAL_READ_LONG(regAddr)  & (bitMask)) == (bitMask))

/*
 * Macros for bit manipulation on values instead of registers
 */
#define IXP400_PARITYENACC_VAL_READ(value, bitMask)         \
    ((value) & (bitMask))

#define IXP400_PARITYENACC_VAL_BIT_SET(value, bitMask)      \
do {                                                        \
    ((value) |= (bitMask));                                 \
} while (0)

#define IXP400_PARITYENACC_VAL_BIT_CLEAR(value, bitMask)    \
do {                                                        \
    ((value) &= ~(bitMask));                                \
} while (0)

#define IXP400_PARITYENACC_VAL_BIT_CHECK(value, bitMask)    \
    (((value) & (bitMask)) == (bitMask))

/* Fused-Out Module Config Bit Masks for main module */
#define IXP400_PARITYENACC_FUSED_MODULE_NPEA    (1 << 0)
#define IXP400_PARITYENACC_FUSED_MODULE_NPEB    (1 << 1)
#define IXP400_PARITYENACC_FUSED_MODULE_NPEC    (1 << 2)
#define IXP400_PARITYENACC_FUSED_MODULE_MCU_ECC (1 << 3)
#define IXP400_PARITYENACC_FUSED_MODULE_PBC     (1 << 4)

/* Macro for Message Logging */
#define IXP400_PARITYENACC_MSGLOG(logLevel, logDevice, fmtStr,  \
            arg1, arg2, arg3, arg4, arg5, arg6)                 \
do {                                                            \
    ixOsalLog(logLevel, logDevice, fmtStr, arg1, arg2, arg3,    \
                arg4, arg5, arg6);                              \
} while (0)

/*
 * Typedefs used in this file
 */

/* IxParityENAcc module initialisation status */
typedef BOOL IxParityENAccInitStatus;

/* Template ISRs for individual each sub-module ISRs */
typedef void (*IxParityENAccPEIsr)(void);

/* Main module internal callback prototype */
typedef void (*IxParityENAccInternalCallback)(UINT32, IxParityENAccPEIsr);

/*
 * Fused-Out Modules Bit Array
 * Bits[2:0]-NPE-A,NPE-B,NPE-C; Bit[3]-MCU ECC; Bit[4]-PCI; Bits[31:5] reserved
 */
typedef UINT32 IxParityENAccFusedModules;

/* Local instance of hardware blocks configuration status */
typedef IxParityENAccHWParityConfig IxParityENAccParityConfigStatus;

/* The parity error enable/disable configuration option */
typedef enum /* IxParityENAccPEConfigOption */
{
    /* Disable parity error detection */
    IXP400_PARITYENACC_PE_DISABLE = IX_PARITYENACC_DISABLE,

    /* Enable parity error detection */
    IXP400_PARITYENACC_PE_ENABLE  = IX_PARITYENACC_ENABLE 
} IxParityENAccPEConfigOption;

/* Odd or Even Parity Type */
typedef enum  /* IxParityENAccPEParityType */
{
    IXP400_PARITYENACC_PE_ODD_PARITY  = IX_PARITYENACC_ODD_PARITY,  /* Odd Parity */
    IXP400_PARITYENACC_PE_EVEN_PARITY = IX_PARITYENACC_EVEN_PARITY  /* Even Parity */
} IxParityENAccPEParityType;

/* The type of access resulting in parity error */
typedef enum  /* IxParityENAccPEParityErrorAccess  */
{
    IXP400_PARITYENACC_PE_READ  = IX_PARITYENACC_READ,  /* Read Access */
    IXP400_PARITYENACC_PE_WRITE = IX_PARITYENACC_WRITE  /* Write Access */
} IxParityENAccPEParityErrorAccess;

/* The memory location which has parity error */
typedef UINT32 IxParityENAccPEParityErrorAddress;

/* The data read from the memory location which has parity error */
typedef UINT32 IxParityENAccPEParityErrorData;

/* Parity Error Staistics for NPEs */
typedef IxParityENAccNpeParityErrorStats IxParityENAccNpePEParityErrorStats;

/* Parity Error Staistics for SWCP */
typedef UINT32 IxParityENAccSwcpPEParityErrorStats;

/* Parity Error Staistics for AQM */
typedef UINT32 IxParityENAccAqmPEParityErrorStats;

/* Parity Error Staistics for DDR MCU */
typedef IxParityENAccMcuParityErrorStats IxParityENAccMcuPEParityErrorStats;

/* Parity Error Staistics for PCI Bus Controller */
typedef IxParityENAccPbcParityErrorStats IxParityENAccPbcPEParityErrorStats;

/* Parity Error Staistics for Expansion Bus Controller */
typedef IxParityENAccEbcParityErrorStats  IxParityENAccEbcPEParityErrorStats;

/* Parity Error Staistics for all hardware blocks */
typedef struct  /* IxParityENAccPEParityErrorStats */
{
    IxParityENAccNpePEParityErrorStats  npeStats;
    IxParityENAccMcuPEParityErrorStats  mcuStats;
    IxParityENAccPbcPEParityErrorStats  pbcStats;
    IxParityENAccEbcPEParityErrorStats  ebcStats;
    IxParityENAccSwcpPEParityErrorStats swcpStats;
    IxParityENAccAqmPEParityErrorStats  aqmStats;
} IxParityENAccPEParityErrorStats;


/*
 * Local functions declarations
 */

void
ixMcuPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr);

void
ixNpePEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr);

void
ixAqmPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr);

void 
ixEbcPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr);

void 
ixPbcPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr);

void 
ixSwcpPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr);

#endif /* IXPARITYENACCMAIN_H */
