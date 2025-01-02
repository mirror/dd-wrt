/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004-2008 Cavium Networks
 */
#ifndef __OCTEON_IRQ_H__
#define __OCTEON_IRQ_H__

#ifdef CONFIG_NUMA
	/* We need 256 per node for MSI */
#define NR_IRQS 767
#else
#define NR_IRQS 511
#endif

#define MIPS_CPU_IRQ_BASE OCTEON_IRQ_SW0

/*
 * 0    - unused.
 * 1..8 - MIPS
 *
 * For a total of 9
 */
#define NR_IRQS_LEGACY 9

enum octeon_irq {
/* 1 - 8 represent the 8 MIPS standard interrupt sources */
	OCTEON_IRQ_SW0 = 1,
	OCTEON_IRQ_SW1,
/* CIU0, CUI2, CIU4 are 3, 4, 5 */
	OCTEON_IRQ_5 = 6,
	OCTEON_IRQ_PERF,
	OCTEON_IRQ_TIMER,
/* sources in CIU_INTX_EN0 */
	OCTEON_IRQ_WORKQ0,
	OCTEON_IRQ_WDOG0 = OCTEON_IRQ_WORKQ0 + 64,
	OCTEON_IRQ_MBOX0 = OCTEON_IRQ_WDOG0 + 32,
	OCTEON_IRQ_MBOX1,
	OCTEON_IRQ_MBOX2,
	OCTEON_IRQ_MBOX3,
	OCTEON_IRQ_PCI_INT0,
	OCTEON_IRQ_PCI_INT1,
	OCTEON_IRQ_PCI_INT2,
	OCTEON_IRQ_PCI_INT3,
	OCTEON_IRQ_PCI_MSI0,
	OCTEON_IRQ_PCI_MSI1,
	OCTEON_IRQ_PCI_MSI2,
	OCTEON_IRQ_PCI_MSI3,

	OCTEON_IRQ_TWSI,
	OCTEON_IRQ_TWSI2,
	OCTEON_IRQ_RML,
	OCTEON_IRQ_TIMER0,
	OCTEON_IRQ_TIMER1,
	OCTEON_IRQ_TIMER2,
	OCTEON_IRQ_TIMER3,
	OCTEON_IRQ_SRIO0,
	OCTEON_IRQ_SRIO1,
	OCTEON_IRQ_SRIO2,
	OCTEON_IRQ_SRIO3,
	OCTEON_IRQ_TDM,
};

#endif
