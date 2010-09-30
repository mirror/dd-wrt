/*
 * Broadcom SiliconBackplane ARM definitions
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: sbhndarm.h,v 13.10 2008/12/17 13:12:05 Exp $
 */

#ifndef	_sbhndarm_h_
#define	_sbhndarm_h_

#include <arminc.h>
#include <sbconfig.h>

/* register offsets */
#define	ARM7_CORECTL		0

/* bits in corecontrol */
#define	ACC_FORCED_RST		0x1
#define	ACC_SERRINT		0x2
#define ACC_NOTSLEEPINGCLKREQ_SHIFT	24

/* arm resetlog */
#define SBRESETLOG		0x1
#define SERRORLOG		0x2

/* arm core-specific control flags */
#define	SICF_REMAP_MSK		0x001c
#define	SICF_REMAP_NONE		0
#define	SICF_REMAP_ROM		0x0004
#define	SIFC_REMAP_FLASH	0x0008

/* misc core-specific defines */
#if defined(__ARM_ARCH_4T__)
/* arm7tdmi-s */
/* backplane related stuff */
#define ARM_CORE_ID		ARM7S_CORE_ID	/* arm coreid */
#define SI_ARM_ROM		SI_ARM7S_ROM	/* ROM backplane/system address */
#define SI_ARM_SRAM2		SI_ARM7S_SRAM2	/* RAM backplane address when remap is 1 or 2 */
#elif defined(__ARM_ARCH_7M__)
/* cortex-m3 */
/* backplane related stuff */
#define ARM_CORE_ID		ARMCM3_CORE_ID	/* arm coreid */
#define SI_ARM_ROM		SI_ARMCM3_ROM	/* ROM backplane/system address */
#define SI_ARM_SRAM2		SI_ARMCM3_SRAM2	/* RAM backplane address when remap is 1 or 2 */
/* core registers offsets */
#define ARMCM3_CYCLECNT		0x90		/* Cortex-M3 core registers offsets */
#define ARMCM3_INTTIMER		0x94
#define ARMCM3_INTMASK		0x98
#define ARMCM3_INTSTATUS	0x9c
/* interrupt/exception */
#define ARMCM3_NUMINTS		16		/* # of external interrupts */
#define ARMCM3_INTALL		((1 << ARMCM3_NUMINTS) - 1)	/* Interrupt mask */
#define	ARMCM3_FAULTMASK	0x40000000	/* Master fault enable/disable */
#define	ARMCM3_PRIMASK		0x80000000	/* Master interrupt enable/disable */
#define ARMCM3_SHARED_INT	0		/* Interrupt shared by multiple cores */
#define ARMCM3_INT(i)		(1 << (i))	/* Individual interrupt enable/disable */
/* compatible with arm7tdmi-s */
#define PS_I	ARMCM3_PRIMASK
#define PS_F	ARMCM3_FAULTMASK
#endif	/* __ARM_ARCH_7M__ */

#ifndef _LANGUAGE_ASSEMBLY

/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif	/* PAD */

#if defined(__ARM_ARCH_4T__)
/* arm7tdmi-s */
typedef volatile struct {
	uint32	corecontrol;	/* 0 */
	uint32	sleepcontrol;	/* 4 */
	uint32	PAD;
	uint32	biststatus;	/* 0xc */
	uint32	firqstatus;	/* 0x10 */
	uint32	fiqmask;	/* 0x14 */
	uint32	irqmask;	/* 0x18 */
	uint32	PAD;
	uint32	resetlog;	/* 0x20 */
	uint32	gpioselect;	/* 0x24 */
	uint32	gpioenable;	/* 0x28 */
	uint32	PAD;
	uint32	bpaddrlo;	/* 0x30 */
	uint32	bpaddrhi;	/* 0x34 */
	uint32	bpdata;		/* 0x38 */
	uint32	bpindaccess;	/* 0x3c */
	uint32	PAD[104];
	uint32	clk_ctl_st;	/* 0x1e0 */
	uint32	hw_war;		/* 0x1e4 */
} armregs_t;
#define ARMREG(regs, reg)	(&((armregs_t *)regs)->reg)
#endif	/* __ARM_ARCH_4T__ */

#if defined(__ARM_ARCH_7M__)
/* cortex-m3 */
typedef volatile struct {
	uint32	corecontrol;	/* 0x0 */
	uint32	corestatus;	/* 0x4 */
	uint32	PAD[1];
	uint32	biststatus;	/* 0xc */
	uint32	nmiisrst;	/* 0x10 */
	uint32	nmimask;	/* 0x14 */
	uint32	isrmask;	/* 0x18 */
	uint32	PAD[1];
	uint32	resetlog;	/* 0x20 */
	uint32	gpioselect;	/* 0x24 */
	uint32	gpioenable;	/* 0x28 */
	uint32	PAD[1];
	uint32	bpaddrlo;	/* 0x30 */
	uint32	bpaddrhi;	/* 0x34 */
	uint32	bpdata;		/* 0x38 */
	uint32	bpindaccess;	/* 0x3c */
	uint32	PAD[16];
	uint32	bwalloc;	/* 0x80 */
	uint32	PAD[3];
	uint32	cyclecnt;	/* 0x90 */
	uint32	inttimer;	/* 0x94 */
	uint32	intmask;	/* 0x98 */
	uint32	intstatus;	/* 0x9c */
	uint32	PAD[80];
	uint32	clk_ctl_st;	/* 0x1e0 */
} cm3regs_t;
#define ARMREG(regs, reg)	(&((cm3regs_t *)regs)->reg)
#endif	/* __ARM_ARCH_7M__ */

#endif	/* _LANGUAGE_ASSEMBLY */

#endif	/* _sbhndarm_h_ */
