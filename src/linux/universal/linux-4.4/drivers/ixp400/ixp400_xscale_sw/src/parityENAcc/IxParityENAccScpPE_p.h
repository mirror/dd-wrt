/**
 * @file IxParityENAccSpcPE_p.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Private header file for SWCP Parity Detection Enabler sub-
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

#ifndef IXPARITYENACCSPCPE_P_H
#define IXPARITYENACCSPCPE_P_H

#include "IxOsal.h"

/*
 * #defines and macros used in this file.
 */

/* IRQ Level for the SWCP */
#if !defined(__ixp43X)
#define IRQ_IXP400_INTC_PARITYENACC_SWCP  IX_OSAL_IXP400_SWCP_IRQ_LVL
#else /* define dummy value for IXP 43X */
#define IRQ_IXP400_INTC_PARITYENACC_SWCP  0 
#endif


/*
 * Typedefs used in the SWCP sub-module
 */

/* SWCP ISR Type */  
typedef IxParityENAccPEIsr IxParityENAccSwcpPEIsr;

/* SWCP Interrupt Service Routine */
typedef struct  /* IxParityENAccSwcpPEIsrInfo */
{
    UINT32  swcpInterruptId;        /* SWCP Interrupt Identifier */
    IxParityENAccSwcpPEIsr swcpIsr; /* ISR for handling interrupts */
} IxParityENAccSwcpPEIsrInfo;

/* SWCP Configuration Information */
typedef struct  /* IxParityENAccSwcpPEConfig */
{
    /* Internal Callback Routine */
    IxParityENAccInternalCallback  swcpPECallback;

    /* Interrupt Service Routine Info */
    IxParityENAccSwcpPEIsrInfo  swcpIsrInfo;
} IxParityENAccSwcpPEConfig;

/*
 * Variable declarations
 */

static IxParityENAccSwcpPEConfig ixParityENAccSwcpPEConfig =
{
    /* SWCP internal callback */
    (IxParityENAccInternalCallback) NULL,
    /* SWCP ISR Info */
    { 0, (IxParityENAccSwcpPEIsr) NULL }
};
 
/*
 * Local functions declarations
 */

/* SWCP interrupt service routine */
void
ixParityENAccSwcpPEIsr(void);

#endif /* IXPARITYENACCSPCPE_P_H */
