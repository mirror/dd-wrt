/*
 * HNDRTE arm trap handling.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte_armtrap.h,v 13.3 2007/03/06 02:30:31 Exp $
 */

#ifndef	_hndrte_armtrap_h
#define	_hndrte_armtrap_h


/* ARM trap handling */

/* Trap types defined by ARM (see arminc.h) */

/* Trap locations in lo memory */
#define	TRAP_STRIDE	4
#define FIRST_TRAP	TR_RST
#define LAST_TRAP	(TR_FIQ * TRAP_STRIDE)

#if defined(__ARM_ARCH_4T__)
#define	MAX_TRAP_TYPE	(TR_FIQ + 1)
#elif defined(__ARM_ARCH_7M__)
#define	MAX_TRAP_TYPE	(TR_ISR + ARMCM3_NUMINTS)
#endif	/* __ARM_ARCH_7M__ */

/* The trap structure is defined here as offsets for assembly */
#define	TR_TYPE		0x00
#define	TR_EPC		0x04
#define	TR_CPSR		0x08
#define	TR_SPSR		0x0c
#define	TR_REGS		0x10
#define	TR_SP		(TR_REGS + (13 * 4))
#define	TR_LR		(TR_REGS + (14 * 4))
#define	TR_PC		(TR_REGS + (15 * 4))

#define	TRAP_T_SIZE	80

#ifndef	_LANGUAGE_ASSEMBLY

#include <typedefs.h>

typedef struct _trap_struct {
	uint32		type;
	uint32		epc;
	uint32		cpsr;
	uint32		spsr;
	uint32		r0;
	uint32		r1;
	uint32		r2;
	uint32		r3;
	uint32		r4;
	uint32		r5;
	uint32		r6;
	uint32		r7;
	uint32		r8;
	uint32		r9;
	uint32		r10;
	uint32		r11;
	uint32		r12;
	uint32		r13;
	uint32		r14;
	uint32		pc;
} trap_t;

#endif	/* !_LANGUAGE_ASSEMBLY */

#endif	/* _hndrte_armtrap_h */
