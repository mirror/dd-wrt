/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Interface to power-throttle control, measurement, and debugging
 * facilities.
 *
 * <hr>$Revision: 91072 $<hr>
 *
 */

#include "cvmx.h"
#include "cvmx-asm.h"
#include "cvmx-coremask.h"
#include "cvmx-power-throttle.h"
#include "cvmx-spinlock.h"

CVMX_SHARED cvmx_spinlock_t lock;

static inline void __cvmx_power_throttle_read(void *info)
{
	unsigned long long res;

	asm volatile ("dmfc0 %0, $11, 6" : "=r" (res));

	*(uint64_t*)info = res;
}

static inline void __cvmx_power_throttle_write(void *info)
{
	asm volatile ("dmtc0 %z0, $11, 6" : : "Jr" (*(uint64_t *)info));
}

static inline void __cvmx_power_throttle_read_from_core(int core, void *r)
{
	if (OCTEON_IS_OCTEON2()) {
		uint64_t offset;

		offset = ((core << 8) + (11 << 3) + 6) << 3;
		*(uint64_t *)r = cvmx_read_csr(CVMX_L2C_COP0_MAPX(0) + offset);
	} else {
		cvmx_l2c_cop0_adr_t adr;

		adr.u64 = 0;
		adr.s.ppid = core;
		adr.s.root = 1;
		adr.s.rd = 11;
		adr.s.sel = 6;
		cvmx_write_csr(CVMX_L2C_COP0_ADR, adr.u64);
		*(uint64_t *)r = cvmx_read_csr(CVMX_L2C_COP0_DAT);
	}
}

static inline void __cvmx_power_throttle_write_to_core(int core, void *r)
{
	if (OCTEON_IS_OCTEON2()) {
		uint64_t offset;

		offset = ((core << 8) + (11 << 3) + 6) << 3;
		cvmx_write_csr(CVMX_L2C_COP0_MAPX(0) + offset, (*(uint64_t *)r));
	} else {
		cvmx_l2c_cop0_adr_t adr;

		adr.u64 = 0;
		adr.s.ppid = core;
		adr.s.root = 1;
		adr.s.rd = 11;
		adr.s.sel = 6;
		cvmx_write_csr(CVMX_L2C_COP0_ADR, adr.u64);
		cvmx_read_csr(CVMX_L2C_COP0_DAT);
		cvmx_write_csr(CVMX_L2C_COP0_DAT, (*(uint64_t *)r));
	}
}

/**
 * @INTERNAL
 * Given ppid, read/write COP0 Power Throttle register.
 *
 * @param  ppid : Core id.
 * @param  r    : COP0 power throttling fields
 * @param  write: If set, update COP0 PowThr register else read.
 */
static void cvmx_power_throttle_csr_op(int ppid,
			cvmx_power_throttle_rfield_t *r, bool write)
{
	if (cvmx_get_core_num() == (uint64_t)ppid) {
		if (write)
			__cvmx_power_throttle_write(r);
		else
			__cvmx_power_throttle_read(r);
	} else {
		cvmx_spinlock_lock(&lock);
		if (write)
			__cvmx_power_throttle_write_to_core(ppid, r);
		else
			__cvmx_power_throttle_read_from_core(ppid, r);
		cvmx_spinlock_unlock(&lock);
	}
}

/**
 * Get the POWLIM field as percentage% of the MAXPOW field in r.
 *
 * @param ppid : Core to find the percentage
 * @return  On success return percentage or -1 on failure
 */
int cvmx_power_throttle_get_powlim(int ppid)
{
	cvmx_power_throttle_rfield_t r;
	int t, rv;

	cvmx_power_throttle_csr_op(ppid, &r, false);
	t = r.s.maxpow;
	if (!OCTEON_IS_MODEL(OCTEON_CN63XX)) {
		if (t < r.s.hrmpowadj)
			return -1;
		else
			t -= r.s.hrmpowadj;
	}
	if (t > 0)
		rv = (r.s.powlim * 100) / t;
	else
		rv = 100;

	return rv > 100 ? 100 : rv;
}

/**
 * @INTERNAL
 * Set the POWLIM field as percentage% of the MAXPOW field in r.
 */
static int cvmx_power_throttle_set_powlim(int ppid, uint8_t percentage)
{
	cvmx_power_throttle_rfield_t r;
	uint64_t t;
	int ret = 0;

	assert(percentage < 101);
	if (cvmx_get_core_num() == (uint64_t)ppid) {
		cvmx_power_throttle_csr_op(ppid, &r, false);
		t = r.s.maxpow;
		if (!OCTEON_IS_MODEL(OCTEON_CN63XX)) {
			if (t < r.s.hrmpowadj)
				ret = -1;
			else
				t -= r.s.hrmpowadj;
		}
		r.s.powlim = percentage > 0 ? percentage * t / 100 : 0;
		r.s.ovrrd = 0;		/* MBZ */
		r.s.distag = 0;		/* MBZ */
		if (ret == 0)
			cvmx_power_throttle_csr_op(ppid, &r, true);
	}
	return 0;
}

/**
 * @INTERNAL
 * Throttle given CPU's power.
 *
 * @param  cpu  : Core id
 */
static void cvmx_power_throttle_init(int cpu)
{
	cvmx_power_throttle_rfield_t r;
	cvmx_power_throttle_csr_op(cpu, &r, false);

	r.s.ovrrd = 0;		/* MBZ */
	r.s.distag = 0;		/* MBZ */
	r.s.period = 2;		/* 256 cycles */
	r.s.minthr = 0;
	/* Start at max allowed speed */
	r.s.maxthr = 0xff;
	r.s.powlim = 0xff;

	cvmx_power_throttle_csr_op(cpu, &r, true);
}

/**
 * Return Cop0 Power Throttle register value for a given Core
 *
 * @param cpuid  : Core id
 * @return  Return COP0 PowThr value
 */
cvmx_power_throttle_rfield_t cvmx_power_throttle_show(int cpuid)
{
	cvmx_power_throttle_rfield_t r;

	r.u64 = 0;
	cvmx_power_throttle_csr_op(cpuid, &r, false);

	return r;
}

/**
 * Throttle power to percentage% of configured maximum (MAXPOW).
 *
 * @param percentage	0 to 100
 * @return 0 for success and -1 for error.
 */
int cvmx_power_throttle_self(uint8_t percentage)
{
	int core = cvmx_get_core_num();

	if (!OCTEON_IS_OCTEON2() && !OCTEON_IS_OCTEON3())
		return -1;

	cvmx_power_throttle_init(core);
	if (cvmx_power_throttle_set_powlim(core, percentage) == 0)
		return -1;

	return 0;
}

/**
 * Throttle power to percentage% of configured maximum (MAXPOW)
 * for the cores identified in coremask.
 *
 * @param percentage 	0 to 100
 * @param pcm		bit mask where each bit identifies a core.
 * @return 0 for success and -1 for error.
 */
int cvmx_power_throttle(uint8_t percentage, cvmx_coremask_t *pcm)
{
	int ppid;
	int ret;

	if (!OCTEON_IS_OCTEON2() && !OCTEON_IS_OCTEON3())
		return -1;

	ret = 0;
	cvmx_coremask_for_each_core(ppid, pcm) {
		cvmx_power_throttle_init(ppid);
		if (cvmx_power_throttle_set_powlim(ppid, percentage) == 0)
			ret = -1;
	}

	return ret;
}
