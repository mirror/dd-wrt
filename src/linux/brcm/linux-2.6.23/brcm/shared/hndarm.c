/*
 * BCM43XX Sonics SiliconBackplane ARM core routines
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: hndarm.c,v 1.52.2.4 2008/08/27 02:01:19 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndcpu.h>
#include <arminc.h>
#include <sbhndarm.h>
#include <hndpmu.h>
#include <bcmdevs.h>
#include <sbsocram.h>

/* Global arm regs pointer */
void *hndarm_armr = NULL;
/* Global arm core rev */
uint32 hndarm_rev = 0;

#if defined(__ARM_ARCH_4T__)
/* point to either arm7sr0_wait() or arm_wfi() */
static void (*arm7s_wait)(si_t *sih) = NULL;

/* Stub. arm7tdmi-s has only one IRQ */
uint
BCMINITFN(si_irq)(si_t *sih)
{
	return 0;
}

#if BCMCHIPID == BCM4328_CHIP_ID
#ifdef HNDRTE_TEST
static void
arm7sr0_sleep(si_t *sih)
{
	W_REG(sih->osh, ARMREG(hndarm_armr, sleepcontrol), 1);
	__asm__ __volatile__("nop; nop; nop; nop; nop");
}
#endif	/* HNDRTE_TEST */
#endif	/* BCMCHIPID == BCM4328_CHIP_ID */

void
hnd_cpu_wait(si_t *sih)
{
	if (arm7s_wait != NULL) {
		arm7s_wait(sih);
		return;
	}
	while (1);
}

#elif defined(__ARM_ARCH_7M__)
/*
 * Map SB cores sharing the ARM IRQ0 to virtual dedicated OS IRQs.
 * Per-port BSP code is required to provide necessary translations between
 * the shared ARM IRQ0 and the virtual OS IRQs based on SB core flag.
 *
 * See sb_irq() for the mapping.
 */
static uint shirq_map_base = 0;

/*
 * Returns the ARM IRQ assignment of the current core. On Cortex-M3
 * it's mapped 1-to-1 with the backplane flag:
 *
 *	IRQ 0 - shared by multiple cores based on isrmask register
 *	IRQ 1 to 14 <==> flag + 1
 *	IRQ 15 - serr
 */
static uint
BCMINITFN(si_getirq)(si_t *sih)
{
	osl_t *osh;
	void *regs;
	uint flag;
	uint idx;
	uint irq;

	osh = si_osh(sih);
	flag = si_flag(sih);

	idx = si_coreidx(sih);
	regs = si_setcore(sih, ARM_CORE_ID, 0);
	ASSERT(regs);

	if (R_REG(osh, ARMREG(regs, isrmask)) & (1 << flag))
		irq = ARMCM3_SHARED_INT;
	else
		irq = flag + 1;

	si_setcoreidx(sih, idx);
	return irq;
}

/*
 * Assigns the specified ARM IRQ to the specified core. Shared ARM
 * IRQ 0 may be assigned more than once. ARM IRQ is enabled after
 * the assignment.
 */
static void
BCMINITFN(si_setirq)(si_t *sih, uint irq, uint coreid, uint coreunit)
{
	osl_t *osh;
	void *regs;
	uint32 flag;
	uint idx;

	osh = si_osh(sih);

	idx = si_coreidx(sih);
	regs = si_setcore(sih, coreid, coreunit);
	ASSERT(regs);

	flag = si_flag(sih);
	ASSERT(irq == flag + 1 || irq == ARMCM3_SHARED_INT);

	regs = si_setcore(sih, ARM_CORE_ID, 0);
	ASSERT(regs);

	if (irq != ARMCM3_SHARED_INT) {
		uint32 isrmask = R_REG(osh, ARMREG(regs, isrmask)) & ~(1 << flag);
		if (isrmask == 0)
			disable_arm_ints(ARMCM3_INT(ARMCM3_SHARED_INT));
		enable_arm_ints(ARMCM3_INT(flag + 1));
		W_REG(osh, ARMREG(regs, isrmask), isrmask);
	}
	else {
		disable_arm_ints(ARMCM3_INT(flag + 1));
		enable_arm_ints(ARMCM3_INT(ARMCM3_SHARED_INT));
		OR_REG(osh, ARMREG(regs, isrmask), (1 << flag));
	}

	si_setcoreidx(sih, idx);
}

/*
 * Return the ARM IRQ assignment of the current core. If necessary
 * map cores sharing the ARM IRQ0 to virtual dedicated OS IRQs.
 */
uint
BCMINITFN(si_irq)(si_t *sih)
{
	uint irq = si_getirq(sih);
	if (irq == ARMCM3_SHARED_INT && 0 != shirq_map_base)
		irq = si_flag(sih) + shirq_map_base;
	return irq;
}

void
hnd_cpu_wait(si_t *sih)
{
	__asm__ __volatile__("wfi");
}
#endif	/* __ARM_ARCH_7M__ */

/*
 * Initializes clocks and interrupts. SB and NVRAM access must be
 * initialized prior to calling.
 */
void
BCMATTACHFN(si_arm_init)(si_t *sih)
{
	sbsocramregs_t *sr;
	osl_t *osh;

#ifdef EXT_CBALL
	return;
#endif	/* !EXT_CBALL */

	osh = si_osh(sih);

	/* Enable/Disable SOCRAM memory standby */
	sr = si_setcore(sih, SOCRAM_CORE_ID, 0);
	if (sr != NULL) {
		uint32 bank;
		uint32 rev;

		bank = (R_REG(osh, &sr->coreinfo) & SRCI_SRNB_MASK) >> SRCI_SRNB_SHIFT;
		ASSERT(bank);

		/* SOCRAM standby is disabled by default in corerev >= 4 so
		 * enable it with a fixed standby timer value equivelant to
		 * 1.2ms of backplane clocks at 80Mhz. Use nvram variable
		 * "ramstbydis" with non-zero value to use hardware default.
		 */
		rev = si_corerev(sih);
		if (rev >= 5) {
			if (getintvar(NULL, "ramstbydis") == 0) {
				uint32 ctlval = ISSIM_ENAB(sih) ? 8 : 0x17fff;
				while (bank--) {
					W_REG(osh, &sr->bankidx, bank);
					W_REG(osh, &sr->standbyctrl,
					      (1 << SRSC_SBYEN_SHIFT) |
					      (1 << SRSC_SBYOVRVAL_SHIFT) | ctlval);
				}
				AND_REG(osh, &sr->pwrctl, ~(1 << SRPC_PMU_STBYDIS_SHIFT));
			}
		}
#if BCMCHIPID == BCM4328_CHIP_ID || BCMCHIPID == BCM4325_CHIP_ID
		else if (rev == 1 || rev == 2) {
			while (bank--) {
				W_REG(osh, &sr->bankidx, bank);
				W_REG(osh, &sr->standbyctrl, (1 << SRSC_SBYOVR_SHIFT));
			}
		}
#endif	/* BCMCHIPID == BCM4328_CHIP_ID || BCMCHIPID == BCM4325_CHIP_ID */
		/* else {} */
	}

	/* Cache ARM core register base and core rev */
	hndarm_armr = si_setcore(sih, ARM_CORE_ID, 0);
	ASSERT(hndarm_armr);
	hndarm_rev = si_corerev(sih);

	/* Now that it's safe, allow ARM to request HT */
	W_REG(osh, ARMREG(hndarm_armr, clk_ctl_st), 0);
	SPINWAIT(((R_REG(osh, ARMREG(hndarm_armr, clk_ctl_st)) & CCS_HTAVAIL) == 0),
	         PMU_MAX_TRANSITION_DLY);
	/* ASSERT(R_REG(osh, ARMREG(hndarm_armr, clk_ctl_st)) & CCS_HTAVAIL); */

	/* Initialize CPU sleep/wait mechanism */
#if defined(__ARM_ARCH_4T__)
	switch (hndarm_rev) {

#if BCMCHIPID == BCM4328_CHIP_ID
	case 0:
		OR_REG(osh, ARMREG(hndarm_armr, clk_ctl_st), CCS_FORCEILP);
#ifdef HNDRTE_TEST
		arm7s_wait = arm7sr0_sleep;
#else
		/* arm7s_wait = NULL; */
#endif
		break;
#endif	/* 4328 */
#if BCMCHIPID == BCM4325_CHIP_ID
	case 1:
#ifdef HNDRTE_TEST
		arm7s_wait = arm_wfi;
#else
		/* arm7s_wait = NULL; */
#endif
		break;
#endif	/* 4325 */
	default:
		arm7s_wait = arm_wfi;
		break;
	}
#elif defined(__ARM_ARCH_7M__)

	/* No need to setup wait for this architecture */

#endif	/* ARM */

	/* Initliaze interrupts/IRQs */
#if defined(__ARM_ARCH_4T__)
	W_REG(osh, ARMREG(hndarm_armr, irqmask), 0xffffffff);
	W_REG(osh, ARMREG(hndarm_armr, fiqmask), 0);

#elif defined(__ARM_ARCH_7M__)
	switch (CHIPID(sih->chip)) {
	case BCM4322_CHIP_ID:
	case BCM43221_CHIP_ID:
	case BCM43231_CHIP_ID:
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, USB20D_CORE_ID, 0);
		break;
	default:
		break;
	}
#endif	/* ARM */

	/* Enable reset & error loggings */
	OR_REG(osh, ARMREG(hndarm_armr, resetlog), (SBRESETLOG | SERRORLOG));
}

uint32
BCMINITFN(si_cpu_clock)(si_t *sih)
{
	if (PMUCTL_ENAB(sih))
		return si_pmu_cpu_clock(sih, si_osh(sih));

	return si_clock(sih);
}

uint32
BCMINITFN(si_mem_clock)(si_t *sih)
{
	if (PMUCTL_ENAB(sih))
		return si_pmu_mem_clock(sih, si_osh(sih));

	return si_clock(sih);
}

/* Start chipc watchdog timer and wait till reset */
void
hnd_cpu_reset(si_t *sih)
{
	si_watchdog(sih, 1);
	while (1);
}

void
hnd_cpu_jumpto(void *addr)
{
	arm_jumpto(addr);
}
