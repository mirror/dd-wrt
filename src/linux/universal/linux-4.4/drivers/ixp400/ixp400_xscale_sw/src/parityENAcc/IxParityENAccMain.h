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

IX_STATUS
ixParityENAccCheckNpeIdValidity(UINT32 npeID);

#endif /* IXPARITYENACCMAIN_H */
