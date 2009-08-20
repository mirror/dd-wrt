/*
 * hndrte ARM port specific routines.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte_arm.c,v 1.59.2.2 2008/07/16 00:41:46 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <hndrte.h>
#include <hndcpu.h>
#include <sbhndarm.h>
#include <sbchipc.h>
#include <hndchipc.h>


#ifndef HNDRTE_POLLING
static void hndrte_pmu_isr(void *cbdata, uint32 intst);
#endif

#if defined(BCMRECLAIM) && defined(CONFIG_XIP)
#error "Both XIP and RECLAIM defined"
#endif


#ifdef DONGLEBUILD
bool bcmreclaimed = FALSE;

#ifdef BCMRECLAIM
bool r1_reclaimed = FALSE;
#endif
bool r2_reclaimed = FALSE;

void
hndrte_reclaim(void)
{
	uint reclaim_size = 0;
#ifdef BCMRECLAIM
	if (!bcmreclaimed && !r1_reclaimed) {
		extern char _rstart1[], _rend1[];	/* start/end of reclaimable area */
		reclaim_size = (uint)(_rend1 - _rstart1);
		if (reclaim_size) {
			bzero(_rstart1, reclaim_size);	/* blow away the reclaim region */
			hndrte_arena_add((uint32)_rstart1, reclaim_size);
		}

		/* Nightly dongle test searches output for "Returned (.*) bytes to the heap" */
		printf("reclaim section 2: Returned %d bytes to the heap\n", reclaim_size);
		bcmreclaimed = TRUE;
		r1_reclaimed = TRUE;
		return;
	}
#endif /* BCMRECLAIM */
	if (!r2_reclaimed) {
		extern char _rstart2[], _rend2[];	/* start/end of reclaimable area */
		reclaim_size = (uint)(_rend2 - _rstart2);

		if (reclaim_size) {
			bzero(_rstart2, reclaim_size);	/* blow away the reclaim region */
			hndrte_arena_add((uint32)_rstart2, reclaim_size);
		}

		/* Nightly dongle test searches output for "Returned (.*) bytes to the heap" */
		printf("reclaim section 1: Returned %d bytes to the heap\n", reclaim_size);
		bcmreclaimed = FALSE;
		r2_reclaimed = TRUE;
		return;
	}
}
#endif /* DONGLEBUILD */


/*
 * Timing support:
 *	hndrte_delay(us): Delay us microseconds.
 */

/*
 * c0 ticks per usec - used by hndrte_delay() and is based on 80Mhz clock.
 * We will never expect the busy loop delay to run on any other slow clocks.
 */
static uint32 c0counts_per_us = HT_CLOCK / 1000000;

/* ticks per msec - used by hndrte_update_now() and is based on either 80Mhz
 * clock or 32Khz clock depending on the compile-time decision.
 */
/* ILP clock speed default to 32KHz */
static uint32 pmuticks_per_ms = ILP_CLOCK / 1000;

/* PmuRev0 has a 10-bit PMU RsrcReq timer which can last 31.x msec
 * at 32KHz clock. To work around this limitation we chop larger timer to
 * multiple maximum 31 msec timers. When these 31 msec timers expire the ISR
 * will be running at 32KHz to save power.
 */
static uint max_timer_dur = ((1 << 10) - 1) / 32;
/* PmuRev1 has a 24-bit PMU RsrcReq timer. However it pushes all other bits
 * upward. To make the code to run for all revs we use a variable to tell how
 * many bits we need to shift.
 */

#if BCMCHIPID == BCM4328_CHIP_ID
static uint8 flags_shift = 0;
#else
#define flags_shift	14
#endif

/* We'll keep time in ms */
static uint32 lastcount = 0;

uint32
hndrte_update_now(void)
{
	uint32 count, diff;

	count = R_REG(hndrte_osh, &hndrte_ccr->pmutimer);

	diff = (count >= lastcount) ? (count - lastcount) :
		(~0 - lastcount + 1 + count);
	if (diff >= pmuticks_per_ms) {
		lastcount = count;
		return diff / pmuticks_per_ms;
	}

	return 0;
}

void
hndrte_delay(uint32 us)
{
	uint32 curr, lim;

	curr = get_arm_cyclecount();
	lim = curr + (us * c0counts_per_us);

	if (lim < curr)
		while (get_arm_cyclecount() > curr)
			;

	while (get_arm_cyclecount() < lim)
		;
}


/*
 * hndrte.
 *
 * Initialize and background:
 *	hndrte_cpu_init: Initialize the world.
 */

void
BCMATTACHFN(hndrte_cpu_init)(si_t *sih)
{
	si_arm_init(sih);

#ifdef EXT_CBALL
	{
	uint32 *v = (uint32 *)0;
	extern char __traps[], _mainCRTStartup[];

	/*
	 * Write new exception vectors.
	 * EXT_CBALL currently does not link with 'startup' at address 0.
	 */

	v[ 0] = 0xea000000 | ((uint32)_mainCRTStartup / 4 - 2);	/* 0000: b <reset> */
	v[ 1] = 0xe59ff014;				/* 0004: ldr pc, [pc, #20] */
	v[ 2] = 0xe59ff014;				/* 0008: ldr pc, [pc, #20] */
	v[ 3] = 0xe59ff014;				/* 000c: ldr pc, [pc, #20] */
	v[ 4] = 0xe59ff014;				/* 0010: ldr pc, [pc, #20] */
	v[ 5] = 0xe59ff014;				/* 0014: ldr pc, [pc, #20] */
	v[ 6] = 0xe59ff014;				/* 0018: ldr pc, [pc, #20] */
	v[ 7] = 0xe59ff014;				/* 001c: ldr pc, [pc, #20] */
	v[ 8] = (uint32)__traps + 0x00;			/* 0020: =tr_und */
	v[ 9] = (uint32)__traps + 0x10;			/* 0024: =tr_swi */
	v[10] = (uint32)__traps + 0x20;			/* 0028: =tr_iab */
	v[11] = (uint32)__traps + 0x30;			/* 002c: =tr_dab */
	v[12] = (uint32)__traps + 0x40;			/* 0030: =tr_bad */
	v[13] = (uint32)__traps + 0x50;			/* 0034: =tr_irq */
	v[14] = (uint32)__traps + 0x60;			/* 0038: =tr_fiq */
	}
#endif /* EXT_CBALL */

	/* Set trap handler */
	hndrte_set_trap((uint32)hndrte_trap_handler);

	/* Initialize timers */
	c0counts_per_us = (si_cpu_clock(sih) + 999999) / 1000000;
	pmuticks_per_ms = (si_ilp_clock(sih) + 999) / 1000;
	if (sih->pmurev >= 1) {
		max_timer_dur = ((1 << 24) - 1);
#if BCMCHIPID == BCM4328_CHIP_ID
		flags_shift = 14;
#endif
	} else {
		max_timer_dur = ((1 << 10) - 1);
	}
	max_timer_dur /= pmuticks_per_ms;

#ifndef HNDRTE_POLLING
	/* Register the timer interrupt handler */
	si_cc_register_isr(sih, hndrte_pmu_isr, CI_PMU, NULL);
#endif
}

void *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
hndrte_dma_alloc_consistent(uint size, void *pap, char *file, int line)
#else
hndrte_dma_alloc_consistent(uint size, void *pap)
#endif
{
	void *buf;

	/* align on a 4k boundary (2^12) */
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!(buf = hndrte_malloc_align(size, 12, file, line)))
#else
	if (!(buf = hndrte_malloc_align(size, 12)))
#endif
		return NULL;
	ASSERT(ISALIGNED((uintptr)buf, DMA_CONSISTENT_ALIGN));

#ifdef CONFIG_XIP
	/* 
	 * arm bootloader memory is remapped but backplane addressing is
	 * 0-based
	 *
	 * Background: since the mask rom bootloader code executes in a
	 * read-only memory space apart from SoC RAM, data addresses
	 * specified by bootloader code must be decoded differently from
	 * text addresses. Consequently, the processor must run in a
	 * "memory remap" mode whereby data addresses containing the
	 * MEMORY_REMAP bit are decoded as residing in SoC RAM. However,
	 * backplane agents, e.g., the dma engines, always use 0-based
	 * addresses for SoC RAM, regardless of processor mode.
	 * Consequently it is necessary to strip the MEMORY_REMAP bit
	 * from addresses programmed into backplane agents.
	 */
	*(ulong *)pap = (ulong)buf & ~MEMORY_REMAP;
#else
	*(ulong *)pap = (ulong)buf;
#endif /* CONFIG_XIP */

	return (buf);
}

void
hndrte_dma_free_consistent(void *va)
{
	hndrte_free(va);
}

/* Two consecutive res_req_timer writes must be 2 ILP + 2 OCP clocks apart */
#if BCMCHIPID == BCM4328_CHIP_ID
static void
pr41608_war(void)
{
	static volatile uint32 hndrte_pmutmr_time = 0;
	uint32 expect;

	if (hndrte_sih->pmurev > 0)
		return;

	expect = hndrte_pmutmr_time + 3;
	if (expect < hndrte_pmutmr_time)
		while (R_REG(hndrte_osh, &hndrte_ccr->pmutimer) >= hndrte_pmutmr_time);
	while (R_REG(hndrte_osh, &hndrte_ccr->pmutimer) <= expect)
		;
	hndrte_pmutmr_time = R_REG(hndrte_osh, &hndrte_ccr->pmutimer);
}
#else
#define pr41608_war()
#endif /* BCMCHIPID == BCM4328_CHIP_ID */

static uint32 timer_extra_ms = 0;
static uint32 timer_extra_ms_start;

void
hndrte_set_irq_timer(uint ms)
{
	uint32 req;
	uint ticks;

	if (ms > max_timer_dur) {
		/* don't req HT if we are breaking a large timer to multiple max h/w duration */
		timer_extra_ms = ms - max_timer_dur;
		timer_extra_ms_start = hndrte_time();
		ms = max_timer_dur;
		req = (PRRT_ALP_REQ | PRRT_INTEN) << flags_shift;
	} else {
		req = (PRRT_HT_REQ | PRRT_INTEN) << flags_shift;
		timer_extra_ms = 0;
	}

	ticks = ms ? ms * pmuticks_per_ms : 2;

	pr41608_war();

	W_REG(hndrte_osh, &hndrte_ccr->res_req_timer, req | ticks);
	(void)R_REG(hndrte_osh, &hndrte_ccr->res_req_timer);
}

void
hndrte_ack_irq_timer(void)
{
	pr41608_war();

	W_REG(hndrte_osh, &hndrte_ccr->res_req_timer, 0);
	(void)R_REG(hndrte_osh, &hndrte_ccr->res_req_timer);
}

void
hndrte_wait_irq(si_t *sih)
{
	hnd_cpu_wait(sih);
}

void
hndrte_enable_interrupts(void)
{
	enable_arm_ints(PS_I);
}

void
hndrte_disable_interrupts(void)
{
	disable_arm_ints(PS_I);
}

/* Enable/Disable h/w HT request */
#if defined(__ARM_ARCH_4T__)
#if BCMCHIPID == BCM4328_CHIP_ID || BCMCHIPID == BCM4325_CHIP_ID
#define HW_HTREQ_ON() \
	do { \
		if (hndarm_rev < 2) \
			AND_REG(hndrte_osh, ARMREG(hndarm_armr, clk_ctl_st), ~CCS_FORCEHWREQOFF); \
	} while (0)
#define HW_HTREQ_OFF() \
	do { \
		if (hndarm_rev < 2) \
			OR_REG(hndrte_osh, ARMREG(hndarm_armr, clk_ctl_st), CCS_FORCEHWREQOFF); \
	} while (0)
#else	/* BCMCHIPID != BCM4328_CHIP_ID && BCMCHIPID != BCM4325_CHIP_ID */
#define HW_HTREQ_ON()
#define HW_HTREQ_OFF()
#endif	/* BCMCHIPID != BCM4328_CHIP_ID && BCMCHIPID != BCM4325_CHIP_ID */
#elif defined(__ARM_ARCH_7M__)
#define HW_HTREQ_ON()
#define HW_HTREQ_OFF()
#endif	/* __ARM_ARCH_7M__ */

void
hndrte_idle_init(si_t *sih)
{
#ifndef HNDRTE_POLLING
	HW_HTREQ_OFF();
#endif
}

#ifndef HNDRTE_POLLING
static void
hndrte_pmu_isr(void *cbdata, uint32 ccintst)
{
	/* Handle pmu timer interrupt here */
	if (
#if BCMCHIPID == BCM4328_CHIP_ID
	    ccintst == 0 ||
#endif
	    (R_REG(hndrte_osh, &hndrte_ccr->res_req_timer) & (PRRT_REQ_ACTIVE << flags_shift))) {
		/* Clear the pmustatus.intpend bit */
		W_REG(hndrte_osh, &hndrte_ccr->pmustatus, PST_INTPEND);
		(void)R_REG(hndrte_osh, &hndrte_ccr->pmustatus);
		hndrte_ack_irq_timer();
		if (timer_extra_ms) {
			uint32 current = hndrte_time();
			uint32 delta =  current > timer_extra_ms_start ?
			        current - timer_extra_ms_start :
			        (uint32)~0 - timer_extra_ms_start + 1 + current;
			if (timer_extra_ms > delta) {
				hndrte_set_irq_timer(timer_extra_ms - delta);
				return;
			}
		}
		/* Run timeouts */
		hndrte_timer_isr();
	}
}
#endif	/* !HNDRTE_POLLING */

void
hndrte_trap_handler(trap_t *tr)
{
#ifndef HNDRTE_POLLING
#if defined(__ARM_ARCH_4T__)
	if (tr->type == TR_IRQ) {
#if BCMCHIPID == BCM4328_CHIP_ID
		/* Check PMU res_req_timer interrupt apart from others because
		 * this may be an "intermediate" timer due to the h/w max timer
		 * length limitation. We request ALP with these intermediate
		 * timers to save power therefore don't request HT.
		 */
		if (hndrte_sih->ccrev < 21 &&
		    (R_REG(hndrte_osh, &hndrte_ccr->res_req_timer) & PRRT_REQ_ACTIVE)) {
			hndrte_pmu_isr(NULL, 0);
			return;
		}
#endif /* BCMCHIPID == BCM4328_CHIP_ID */
		HW_HTREQ_ON();
		hndrte_isr();
		HW_HTREQ_OFF();
		return;
	}
#elif defined(__ARM_ARCH_7M__)
	if (tr->type >= TR_ISR && tr->type < TR_ISR + ARMCM3_NUMINTS) {
		HW_HTREQ_ON();
		hndrte_isr();
		HW_HTREQ_OFF();
		return;
	}
#endif	/* __ARM_ARCH_7M__ */
#endif	/* !HNDRTE_POLLING */

#ifdef TR_ARMRST
	if (tr->type == TR_ARMRST) {
		printf("ARM reset: SErr=%d, Rstlg=0x%x\n",
		       hndrte_ccsbr->sbtmstatehigh & SBTMH_SERR,
		       R_REG(hndrte_osh, ARMREG(hndarm_armr, resetlog)));
	}
#endif

	printf("\nTrap type 0x%x @ epc 0x%x, cpsr 0x%x, spsr 0x%x, sp 0x%x, lp 0x%x, rpc 0x%x\n",
	       tr->type, tr->epc, tr->cpsr, tr->spsr, tr->r13, tr->r14, tr->pc);

	hndrte_die(__LINE__);
}
