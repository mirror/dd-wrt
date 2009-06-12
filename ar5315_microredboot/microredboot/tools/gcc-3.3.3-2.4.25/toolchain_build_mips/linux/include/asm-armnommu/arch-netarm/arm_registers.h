/*
 * linux/include/asm-arm/proc-armv/ptrace.h
 *
 * Copyright (C) 1996 Russell King
 */

#ifndef __ARM_REGISTERS_H
#define __ARM_REGISTERS_H

#define CPSR_USR26_MODE		0x00
#define CPSR_FIQ26_MODE		0x01
#define CPSR_IRQ26_MODE		0x02
#define CPSR_SVC26_MODE		0x03
#define CPSR_USR_MODE		0x10
#define CPSR_FIQ_MODE		0x11
#define CPSR_IRQ_MODE		0x12
#define CPSR_SVC_MODE		0x13
#define CPSR_ABT_MODE		0x17
#define CPSR_UND_MODE		0x1b
#define CPSR_SYSTEM_MODE	0x1f
#define CPSR_MODE_MASK		0x1f
#define CPSR_F_BIT		0x40
#define CPSR_I_BIT		0x80
#define CPSR_CC_V_BIT		(1 << 28)
#define CPSR_CC_C_BIT		(1 << 29)
#define CPSR_CC_Z_BIT		(1 << 30)
#define CPSR_CC_N_BIT		(1 << 31)

#endif

