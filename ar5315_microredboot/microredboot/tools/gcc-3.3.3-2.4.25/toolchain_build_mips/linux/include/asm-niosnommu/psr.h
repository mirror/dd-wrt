/*
 * psr.h: This file holds the macros for masking off various parts of
 *        the processor status register on the NIOS.
 *
 * Copyright (C) 2001 Vic Phillips
 */

#ifndef __LINUX_NIOS_PSR_H
#define __LINUX_NIOS_PSR_H

/* The NIOS PSR (STATUS) fields are laid out as the following:
 *
 *  ----------------------------
 *  | IE | IPRI |  CWP  | ICC  |
 *  | 15 | 9-14 |  4-8  | 0-3  |
 *  ----------------------------
 */

#define PSR_C       0x0001         /* carry bit                  */
#define PSR_Z       0x0002         /* zero bit                   */
#define PSR_V       0x0004         /* overflow bit               */
#define PSR_N       0x0008         /* negative bit               */
#define PSR_ICC     0x000f         /* integer condition codes    */
#define PSR_CWP     0x01f0         /* current window pointer     */
#define PSR_IPRI    0x7e00         /* interrupt priority level   */
#define PSR_IE      0x8000         /* interrupt enable field     */

/* Interrupt priority level 3 is the highest usable when calling c code.
 * This allows under/over flow traps while blocking all other interrupts.
 */
#define PSR_IPRI_3  0x0600         /* interrupt priority level 3  */

/* This bit is not actually in the NIOS STATUS register, but is emulated
 * using a global variable: nios_status
 */

#define PSR_SUPERVISOR		0x80000000  /* supervisor privilege level */
#define PSR_SUPERVISOR_BIT	31				/* (as a bit number)          */

//#ifdef __KERNEL__
//
//#ifndef __ASSEMBLY__
//
///* Get the %psr register. */
//extern inline unsigned int get_psr(void)
//{
//	unsigned int psr;
//	__asm__ __volatile__("rdctl	%0\n\t" : "=r" (psr));
//	return psr;
//}
//
//extern inline void put_psr(unsigned int new_psr)
//{
//	__asm__ __volatile__("wrctl	%0\n\t"
//			     "nop\n\t" : : "r" (new_psr));
//}
//
//#endif /* !(__ASSEMBLY__) */
//
//#endif /* (__KERNEL__) */

#endif /* !(__LINUX_NIOS_PSR_H) */
