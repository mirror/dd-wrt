/*
 *  linux/include/asm-arm/proc-armv/system.h
 *
 *  Copyright (C) 1996 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_PROC_SYSTEM_H
#define __ASM_PROC_SYSTEM_H

#include <linux/config.h>

#define set_cr(x)					\
	__asm__ __volatile__(				\
	"mcr	p15, 0, %0, c1, c0	@ set CR"	\
	: : "r" (x))

#define CR_M	(1 << 0)	/* MMU enable				*/
#define CR_A	(1 << 1)	/* Alignment abort enable		*/
#define CR_C	(1 << 2)	/* Dcache enable			*/
#define CR_W	(1 << 3)	/* Write buffer enable			*/
#define CR_P	(1 << 4)	/* 32-bit exception handler		*/
#define CR_D	(1 << 5)	/* 32-bit data address range		*/
#define CR_L	(1 << 6)	/* Implementation defined		*/
#define CD_B	(1 << 7)	/* Big endian				*/
#define CR_S	(1 << 8)	/* System MMU protection		*/
#define CD_R	(1 << 9)	/* ROM MMU protection			*/
#define CR_F	(1 << 10)	/* Implementation defined		*/
#define CR_Z	(1 << 11)	/* Implementation defined		*/
#define CR_I	(1 << 12)	/* Icache enable			*/
#define CR_V	(1 << 13)	/* Vectors relocated to 0xffff0000	*/
#define CR_RR	(1 << 14)	/* Round Robin cache replacement	*/

extern unsigned long cr_no_alignment;	/* defined in entry-armv.S */
extern unsigned long cr_alignment;	/* defined in entry-armv.S */

#ifdef __ARM_ARCH_4__
#define vectors_base()	((cr_alignment & CR_V) ? 0xffff0000 : 0)
#else
#define vectors_base()	(0)
#endif

/*
 * A couple of speedups for the ARM
 */

/*
 * Save the current interrupt enable state & disable IRQs
 */
#define __save_flags_cli(x)					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ save_flags_cli\n"	\
"	orr	%1, %0, #128\n"					\
"	msr	cpsr_c, %1"					\
	: "=r" (x), "=r" (temp)					\
	:							\
	: "memory");						\
	})

/*
 * Save the current interrupt enable state & enable IRQs
 */
#define __save_flags_sti(x)					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ save_flags_sti\n"	\
"	bic	%1, %0, #128\n"					\
"	msr	cpsr_c, %1"					\
	: "=r" (x), "=r" (temp)					\
	:							\
	: "memory");						\
	})
	
/*
 * Enable IRQs
 */
#define __sti()							\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ sti\n"		\
"	bic	%0, %0, #128\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory");						\
	})

/*
 * Disable IRQs
 */
#define __cli()							\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ cli\n"		\
"	orr	%0, %0, #128\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory");						\
	})

/*
 * Enable FIQs
 */
#define __stf()							\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ stf\n"		\
"	bic	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory");						\
	})

/*
 * Disable FIQs
 */
#define __clf()							\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ clf\n"		\
"	orr	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory");						\
	})

/*
 * save current IRQ & FIQ state
 */
#define __save_flags(x)						\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ save_flags\n"		\
	  : "=r" (x)						\
	  :							\
	  : "memory")

/*
 * restore saved IRQ & FIQ state
 */
#define __restore_flags(x)					\
	__asm__ __volatile__(					\
	"msr	cpsr_c, %0		@ restore_flags\n"	\
	:							\
	: "r" (x)						\
	: "memory")

#if defined(CONFIG_CPU_SA1100) || defined(CONFIG_CPU_SA110)
/*
 * On the StrongARM, "swp" is terminally broken since it bypasses the
 * cache totally.  This means that the cache becomes inconsistent, and,
 * since we use normal loads/stores as well, this is really bad.
 * Typically, this causes oopsen in filp_close, but could have other,
 * more disasterous effects.  There are two work-arounds:
 *  1. Disable interrupts and emulate the atomic swap
 *  2. Clean the cache, perform atomic swap, flush the cache
 *
 * We choose (1) since its the "easiest" to achieve here and is not
 * dependent on the processor type.
 */
#define swp_is_buggy
#endif

extern __inline__ unsigned long __xchg(unsigned long x, volatile void *ptr, int size)
{
	extern void __bad_xchg(volatile void *, int);
	unsigned long ret;
#ifdef swp_is_buggy
	unsigned long flags;
#endif

	switch (size) {
#ifdef swp_is_buggy
		case 1:
			__save_flags_cli(flags);
			ret = *(volatile unsigned char *)ptr;
			*(volatile unsigned char *)ptr = x;
			__restore_flags(flags);
			break;

		case 4:
			__save_flags_cli(flags);
			ret = *(volatile unsigned long *)ptr;
			*(volatile unsigned long *)ptr = x;
			__restore_flags(flags);
			break;
#else
		case 1:	__asm__ __volatile__ ("swpb %0, %1, [%2]"
					: "=r" (ret)
					: "r" (x), "r" (ptr)
					: "memory");
			break;
		case 4:	__asm__ __volatile__ ("swp %0, %1, [%2]"
					: "=r" (ret)
					: "r" (x), "r" (ptr)
					: "memory");
			break;
#endif
		default: __bad_xchg(ptr, size);
	}

	return ret;
}

#endif
