/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009 - 2012 Cavium, Inc.
 */
#include <linux/clocksource.h>
#include <linux/init.h>
#include <linux/time.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-mio-defs.h>

/*
 * By default use only the cvmcount source and leave this one
 * disabled/unregistered.
 */
#define REGISTER_OCTEON_PTP_CSRC 0

#if REGISTER_OCTEON_PTP_CSRC
static cycle_t octeon_ptp_clock_read(struct clocksource *cs)
{
	return octeon_read_ptp_csr(CVMX_MIO_PTP_CLOCK_HI);
}

static struct clocksource clocksource_ptp_clock = {
	.name		= "OCTEON_PTP_CLOCK",
	.rating		= 400,
	.read		= octeon_ptp_clock_read,
	.mask		= CLOCKSOURCE_MASK(64),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};
#endif

int __init ptp_clock_init(void)
{
	u64 clock_comp;
	union cvmx_mio_ptp_clock_cfg ptp_clock_cfg;

	/* Chips prior to CN6XXX don't support the PTP clock source */
	if (!OCTEON_IS_MODEL(OCTEON_CN6XXX) && !OCTEON_IS_MODEL(OCTEON_CNF7XXX))
		return 0;

	/* FIXME: Remove this when PTP is implemented in the simulator */
	if (octeon_is_simulation())
		return 0;

	/* Get the current state of the PTP clock */
	ptp_clock_cfg.u64 = octeon_read_ptp_csr(CVMX_MIO_PTP_CLOCK_CFG);
	if (!ptp_clock_cfg.s.ext_clk_en) {
		/*
		 * The clock has not been configured to use an
		 * external source.  Program it to use the main clock
		 * reference.
		 */
		clock_comp = (NSEC_PER_SEC << 32) / octeon_get_io_clock_rate();
		cvmx_write_csr(CVMX_MIO_PTP_CLOCK_COMP, clock_comp);
		pr_info("PTP Clock: Using sclk reference at %lld Hz\n",
			(NSEC_PER_SEC << 32) / clock_comp);
	} else {
		/* The clock is already programmed to use an external GPIO */
		clock_comp = octeon_read_ptp_csr(CVMX_MIO_PTP_CLOCK_COMP);
		pr_info("PTP Clock: Using GPIO %d at %lld Hz\n",
			ptp_clock_cfg.s.ext_clk_in,
			(NSEC_PER_SEC << 32) / clock_comp);
	}

	/* Enable the clock if it wasn't done already */
	if (!ptp_clock_cfg.s.ptp_en) {
		ptp_clock_cfg.s.ptp_en = 1;
		cvmx_write_csr(CVMX_MIO_PTP_CLOCK_CFG, ptp_clock_cfg.u64);
	}
#if REGISTER_OCTEON_PTP_CSRC
	/* Add PTP as a high quality clocksource with nano second granularity */
	clocksource_register_hz(&clocksource_ptp_clock, NSEC_PER_SEC);
#endif
	return 0;
}
arch_initcall(ptp_clock_init);
