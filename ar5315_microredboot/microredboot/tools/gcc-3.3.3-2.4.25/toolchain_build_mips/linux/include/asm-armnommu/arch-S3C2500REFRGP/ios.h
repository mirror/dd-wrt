/*
 * ios.h 
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc.
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 * This file includes the I/O ports definitions 
 * of the S3C2500X RISC microcontroller
 * based on the Samsung's "S3C2500X 32-bit RISC
 * microcontroller pre. User's Manual"
 */

#ifndef __ASM_ARCH_IOS_H
#define __ASM_ARCH_IOS_H

#define MAX_IOS_PORTS	64

#define IO_BASE		0xf0030000

#define	IOPMOD1		0xF0030000	/* I/O port mode select lower R		*/
#define	IOPMOD2		0xF0030004	/* I/O port mode select upper R		*/
#define	IO_Ftn_cont1	0xF0030008	/* I/O port select lower R		*/
#define	IO_Ftn_cont2	0xF003000C	/* I/O port select upper R		*/
#define	IOP4DMA		0xF0030010	/* I/O port special function R for DMA	*/
#define	IOP4xINT	0xF0030014	/* I/O port spec. function R for Ext.IRQ*/
#define	IOP4xINT_PEND	0xF0030018	/* External IRQ clear R			*/
#define	IOPDATA1	0xF003001c	/* I/O port data R			*/
#define	IOPDATA2	0xF0030020	/* I/O port data R			*/
#define	IOPDRV1		0xF0030024	/* I/O port drive control R		*/
#define	IOPDRV2		0xF0030028	/* I/O port drive control R		*/

#endif /* _ASM_ARCH_IOS_H */
