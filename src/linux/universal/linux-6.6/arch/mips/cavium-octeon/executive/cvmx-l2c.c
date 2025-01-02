/***********************license start***************
 * Copyright (c) 2003-2015  Cavium Inc. (support@cavium.com). All rights
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
 * Implementation of the Level 2 Cache (L2C) control,
 * measurement, and debugging facilities.
 *
 * <hr>$Revision: 164990 $<hr>
 *
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-l2c.h>
#include <asm/octeon/cvmx-spinlock.h>
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-l2c.h"
#include "cvmx-spinlock.h"
#include "cvmx-interrupt.h"
#endif

#ifndef CVMX_BUILD_FOR_LINUX_HOST
/*
 * This spinlock is used internally to ensure that only one core is
 * performing certain L2 operations at a time.
 *
 * NOTE: This only protects calls from within a single application -
 * if multiple applications or operating systems are running, then it
 * is up to the user program to coordinate between them.
 */
CVMX_SHARED cvmx_spinlock_t cvmx_l2c_spinlock;
#endif

int cvmx_l2c_get_core_way_partition(uint32_t core)
{
	uint32_t field;
	int node = cvmx_get_node_num();

	/* Validate the core number */
	if (core >= cvmx_octeon_num_cores())
		return -1;

	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3())
		return (cvmx_read_csr_node(node, CVMX_L2C_WPAR_PPX(core)) & 0xffff);

	/*
	 * Use the lower two bits of the coreNumber to determine the
	 * bit offset of the UMSK[] field in the L2C_SPAR register.
	 */
	field = (core & 0x3) * 8;

	/*
	 * Return the UMSK[] field from the appropriate L2C_SPAR
	 * register based on the coreNumber.
	 */

	switch (core & 0xC) {
	case 0x0:
		return (cvmx_read_csr(CVMX_L2C_SPAR0) & (0xFF << field)) >> field;
	case 0x4:
		return (cvmx_read_csr(CVMX_L2C_SPAR1) & (0xFF << field)) >> field;
	case 0x8:
		return (cvmx_read_csr(CVMX_L2C_SPAR2) & (0xFF << field)) >> field;
	case 0xC:
		return (cvmx_read_csr(CVMX_L2C_SPAR3) & (0xFF << field)) >> field;
	}
	return 0;
}

int cvmx_l2c_set_core_way_partition(uint32_t core, uint32_t mask)
{
	uint32_t field;
	uint32_t valid_mask;
	int node = cvmx_get_node_num();

	if (OCTEON_IS_OCTEON1PLUS()) {
		valid_mask = (0x1 << cvmx_l2c_get_num_assoc()) - 1;

		mask &= valid_mask;

		/* A UMSK setting which blocks all L2C Ways is an error on some chips */
		if (mask == valid_mask)
			return -1;
	}

	/* Validate the core number */
	if (core >= cvmx_octeon_num_cores())
		return -1;

	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) {
		cvmx_write_csr_node(node, CVMX_L2C_WPAR_PPX(core), mask);
		return 0;
	}

	/*
	 * Use the lower two bits of core to determine the bit offset of the
	 * UMSK[] field in the L2C_SPAR register.
	 */
	field = (core & 0x3) * 8;

	/*
	 * Assign the new mask setting to the UMSK[] field in the appropriate
	 * L2C_SPAR register based on the core_num.
	 *
	 */
	switch (core & 0xC) {
	case 0x0:
		cvmx_write_csr(CVMX_L2C_SPAR0, (cvmx_read_csr(CVMX_L2C_SPAR0) & ~(0xFF << field)) | mask << field);
		break;
	case 0x4:
		cvmx_write_csr(CVMX_L2C_SPAR1, (cvmx_read_csr(CVMX_L2C_SPAR1) & ~(0xFF << field)) | mask << field);
		break;
	case 0x8:
		cvmx_write_csr(CVMX_L2C_SPAR2, (cvmx_read_csr(CVMX_L2C_SPAR2) & ~(0xFF << field)) | mask << field);
		break;
	case 0xC:
		cvmx_write_csr(CVMX_L2C_SPAR3, (cvmx_read_csr(CVMX_L2C_SPAR3) & ~(0xFF << field)) | mask << field);
		break;
	}
	return 0;
}

int cvmx_l2c_set_hw_way_partition(uint32_t mask)
{
	uint32_t valid_mask;
	int node = cvmx_get_node_num();

	if (OCTEON_IS_OCTEON1PLUS()) {
		valid_mask = (0x1 << cvmx_l2c_get_num_assoc()) - 1;
		mask &= 0xffff;

		/* A UMSK setting which blocks all L2C Ways is an error on some chips */
		if (mask == valid_mask)
			return -1;
	}

	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3())
		cvmx_write_csr_node(node, CVMX_L2C_WPAR_IOBX(0), mask);
	else
		cvmx_write_csr(CVMX_L2C_SPAR4, (cvmx_read_csr(CVMX_L2C_SPAR4) & ~0xFF) | mask);
	return 0;
}

int cvmx_l2c_get_hw_way_partition(void)
{
	int node = cvmx_get_node_num();
	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3())
		return cvmx_read_csr_node(node, CVMX_L2C_WPAR_IOBX(0)) & 0xffff;
	else
		return cvmx_read_csr(CVMX_L2C_SPAR4) & (0xFF);
}

int cvmx_l2c_set_hw_way_partition2(uint32_t mask)
{
	uint32_t valid_mask;
	int node = cvmx_get_node_num();

	if (CVMX_L2C_IOBS < 2)
		return -1;

	valid_mask = (0x1 << cvmx_l2c_get_num_assoc()) - 1;
	mask &= valid_mask;
	cvmx_write_csr_node(node, CVMX_L2C_WPAR_IOBX(1), mask);
	return 0;
}

int cvmx_l2c_get_hw_way_partition2(void)
{
	int node = cvmx_get_node_num();
	if (CVMX_L2C_IOBS < 2)
		return -1;

	return cvmx_read_csr_node(node, CVMX_L2C_WPAR_IOBX(1)) & 0xffff;
}

void cvmx_l2c_config_perf(uint32_t counter, enum cvmx_l2c_event event, uint32_t clear_on_read)
{
	if (OCTEON_IS_OCTEON1PLUS()) {
		union cvmx_l2c_pfctl pfctl;

		pfctl.u64 = cvmx_read_csr(CVMX_L2C_PFCTL);

		switch (counter) {
		case 0:
			pfctl.s.cnt0sel = event;
			pfctl.s.cnt0ena = 1;
			pfctl.s.cnt0rdclr = clear_on_read;
			break;
		case 1:
			pfctl.s.cnt1sel = event;
			pfctl.s.cnt1ena = 1;
			pfctl.s.cnt1rdclr = clear_on_read;
			break;
		case 2:
			pfctl.s.cnt2sel = event;
			pfctl.s.cnt2ena = 1;
			pfctl.s.cnt2rdclr = clear_on_read;
			break;
		case 3:
		default:
			pfctl.s.cnt3sel = event;
			pfctl.s.cnt3ena = 1;
			pfctl.s.cnt3rdclr = clear_on_read;
			break;
		}

		cvmx_write_csr(CVMX_L2C_PFCTL, pfctl.u64);
	} else {
		union cvmx_l2c_tadx_prf l2c_tadx_prf;
		int tad;

		cvmx_warn_if(clear_on_read, "L2C counters don't support clear on read for this chip\n");

		l2c_tadx_prf.u64 = cvmx_read_csr(CVMX_L2C_TADX_PRF(0));

		switch (counter) {
		case 0:
			l2c_tadx_prf.s.cnt0sel = event;
			break;
		case 1:
			l2c_tadx_prf.s.cnt1sel = event;
			break;
		case 2:
			l2c_tadx_prf.s.cnt2sel = event;
			break;
		default:
		case 3:
			l2c_tadx_prf.s.cnt3sel = event;
			break;
		}
		for (tad = 0; tad < CVMX_L2C_TADS; tad++)
			cvmx_write_csr(CVMX_L2C_TADX_PRF(tad), l2c_tadx_prf.u64);
	}
}

uint64_t cvmx_l2c_read_perf(uint32_t counter)
{
	if (OCTEON_IS_OCTEON3()) {
		uint64_t count = 0;
		int tad = 0;

		for (tad = 0; tad < CVMX_L2C_TADS; tad++)
			count += cvmx_read_csr(CVMX_L2C_TADX_PFCX(counter, tad));
		return count;
	}

	switch (counter) {
	case 0:
		if (OCTEON_IS_OCTEON1PLUS())
			return cvmx_read_csr(CVMX_L2C_PFC0);
		else {
			uint64_t counter = 0;
			int tad;
			for (tad = 0; tad < CVMX_L2C_TADS; tad++)
				counter += cvmx_read_csr(CVMX_L2C_TADX_PFC0(tad));
			return counter;
		}
	case 1:
		if (OCTEON_IS_OCTEON1PLUS())
			return cvmx_read_csr(CVMX_L2C_PFC1);
		else {
			uint64_t counter = 0;
			int tad;
			for (tad = 0; tad < CVMX_L2C_TADS; tad++)
				counter += cvmx_read_csr(CVMX_L2C_TADX_PFC1(tad));
			return counter;
		}
	case 2:
		if (OCTEON_IS_OCTEON1PLUS())
			return cvmx_read_csr(CVMX_L2C_PFC2);
		else {
			uint64_t counter = 0;
			int tad;
			for (tad = 0; tad < CVMX_L2C_TADS; tad++)
				counter += cvmx_read_csr(CVMX_L2C_TADX_PFC2(tad));
			return counter;
		}
	case 3:
	default:
		if (OCTEON_IS_OCTEON1PLUS())
			return cvmx_read_csr(CVMX_L2C_PFC3);
		else {
			uint64_t counter = 0;
			int tad;
			for (tad = 0; tad < CVMX_L2C_TADS; tad++)
				counter += cvmx_read_csr(CVMX_L2C_TADX_PFC3(tad));
			return counter;
		}
	}
}

#ifndef CVMX_BUILD_FOR_LINUX_HOST
/**
 * @INTERNAL
 * Helper function use to fault in cache lines for L2 cache locking
 *
 * @param addr   Address of base of memory region to read into L2 cache
 * @param len    Length (in bytes) of region to fault in
 */
static void fault_in(uint64_t addr, int len)
{
	volatile char *ptr;
	volatile char dummy;
	/*
	 * Adjust addr and length so we get all cache lines even for
	 * small ranges spanning two cache lines.
	 */
	len += addr & CVMX_CACHE_LINE_MASK;
	addr &= ~CVMX_CACHE_LINE_MASK;
	ptr = (volatile char *)cvmx_phys_to_ptr(addr);
	/*
	 * Invalidate L1 cache to make sure all loads result in data
	 * being in L2.
	 */
	CVMX_DCACHE_INVALIDATE;
	while (len > 0) {
		dummy += *ptr;
		len -= CVMX_CACHE_LINE_SIZE;
		ptr += CVMX_CACHE_LINE_SIZE;
	}
}

int cvmx_l2c_lock_line(uint64_t addr)
{
	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) {
		int shift = CVMX_L2C_TAG_ADDR_ALIAS_SHIFT;
		uint64_t assoc = cvmx_l2c_get_num_assoc();
		uint32_t tag = cvmx_l2c_v2_address_to_tag(addr);
		uint64_t indext = cvmx_l2c_address_to_index(addr) << CVMX_L2C_IDX_ADDR_SHIFT;
		uint64_t index = CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, indext);
		uint64_t way;
		uint32_t tad;
		union cvmx_l2c_tadx_tag l2c_tadx_tag;

		if (tag == 0xFFFFFFFF) {
			cvmx_dprintf("ERROR: cvmx_l2c_lock_line: addr 0x%llx in LMC hole." "\n", (unsigned long long)addr);
			return -1;
		}

		tad = cvmx_l2c_address_to_tad(addr);

		/* cvmx_dprintf("shift=%d index=%lx tag=%x\n",shift, index, tag); */
		CVMX_CACHE_LCKL2(CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, addr), 0);
		CVMX_SYNCW;
		/* Make sure we were able to lock the line */
		for (way = 0; way < assoc; way++) {
			uint64_t caddr = index | (way << shift);
			CVMX_CACHE_LTGL2I(caddr, 0);
			/* make sure CVMX_L2C_TADX_TAG is updated */
			CVMX_SYNC;
			l2c_tadx_tag.u64 = cvmx_read_csr(CVMX_L2C_TADX_TAG(tad));
			if (OCTEON_IS_OCTEON2() && l2c_tadx_tag.cn61xx.valid &&
			    l2c_tadx_tag.cn61xx.tag == tag)
				break;
			else if (OCTEON_IS_MODEL(OCTEON_CN70XX)
				 && l2c_tadx_tag.cn70xx.valid
				 && l2c_tadx_tag.cn70xx.tag == tag)
				break;
			else if ((OCTEON_IS_MODEL(OCTEON_CN73XX)
				  || OCTEON_IS_MODEL(OCTEON_CNF75XX))
				 && l2c_tadx_tag.cn73xx.tag == tag)
				break;
			else if (l2c_tadx_tag.cn78xx.ts != 0
				 && l2c_tadx_tag.cn78xx.tag == tag)
			        break;

			 /* cvmx_dprintf("caddr=%lx tad=%d tagu64=%lx valid=%x tag=%x \n", caddr,
			   tad, l2c_tadx_tag.u64, l2c_tadx_tag.cn70xx.valid, l2c_tadx_tag.cn70xx.tag); */
		}

		/* Check if a valid line is found */
		if (way >= assoc) {
			/* cvmx_dprintf("ERROR: cvmx_l2c_lock_line: line not found for locking at"
			   " 0x%llx address\n", (unsigned long long)addr); */
			return -1;
		}

		/* Check if lock bit is not set */
		if (!l2c_tadx_tag.s.lock) {
			/* cvmx_dprintf("ERROR: cvmx_l2c_lock_line: Not able to lock at "
			   "0x%llx address\n", (unsigned long long)addr); */
			return -1;
		}
		return 0;
	} else {
		int retval = 0;
		union cvmx_l2c_dbg l2cdbg;
		union cvmx_l2c_lckbase lckbase;
		union cvmx_l2c_lckoff lckoff;
		union cvmx_l2t_err l2t_err;

		cvmx_spinlock_lock(&cvmx_l2c_spinlock);

		l2cdbg.u64 = 0;
		lckbase.u64 = 0;
		lckoff.u64 = 0;

		/* Clear l2t error bits if set */
		l2t_err.u64 = cvmx_read_csr(CVMX_L2T_ERR);
		l2t_err.s.lckerr = 1;
		l2t_err.s.lckerr2 = 1;
		cvmx_write_csr(CVMX_L2T_ERR, l2t_err.u64);

		addr &= ~CVMX_CACHE_LINE_MASK;

		/* Set this core as debug core */
		l2cdbg.s.ppnum = cvmx_get_core_num();
		CVMX_SYNC;
		cvmx_write_csr(CVMX_L2C_DBG, l2cdbg.u64);
		cvmx_read_csr(CVMX_L2C_DBG);

		lckoff.s.lck_offset = 0;	/* Only lock 1 line at a time */
		cvmx_write_csr(CVMX_L2C_LCKOFF, lckoff.u64);
		cvmx_read_csr(CVMX_L2C_LCKOFF);

		if (((union cvmx_l2c_cfg)(cvmx_read_csr(CVMX_L2C_CFG))).s.idxalias) {
			int alias_shift = CVMX_L2C_IDX_ADDR_SHIFT + 2 * cvmx_l2c_get_set_bits() - 1;
			uint64_t addr_tmp = addr ^ (addr & ((1 << alias_shift) - 1)) >> cvmx_l2c_get_set_bits();
			lckbase.s.lck_base = addr_tmp >> 7;
		} else {
			lckbase.s.lck_base = addr >> 7;
		}

		lckbase.s.lck_ena = 1;
		cvmx_write_csr(CVMX_L2C_LCKBASE, lckbase.u64);
		/* Make sure it gets there */
		cvmx_read_csr(CVMX_L2C_LCKBASE);

		fault_in(addr, CVMX_CACHE_LINE_SIZE);

		lckbase.s.lck_ena = 0;
		cvmx_write_csr(CVMX_L2C_LCKBASE, lckbase.u64);
		/* Make sure it gets there */
		cvmx_read_csr(CVMX_L2C_LCKBASE);

		/* Stop being debug core */
		cvmx_write_csr(CVMX_L2C_DBG, 0);
		cvmx_read_csr(CVMX_L2C_DBG);

		l2t_err.u64 = cvmx_read_csr(CVMX_L2T_ERR);
		if (l2t_err.s.lckerr || l2t_err.s.lckerr2)
			retval = 1;	/* We were unable to lock the line */

		cvmx_spinlock_unlock(&cvmx_l2c_spinlock);
		return retval;
	}
}

int cvmx_l2c_lock_mem_region(uint64_t start, uint64_t len)
{
	int retval = 0;

	/* Round start/end to cache line boundaries */
	len += start & CVMX_CACHE_LINE_MASK;
	start &= ~CVMX_CACHE_LINE_MASK;
	len = (len + CVMX_CACHE_LINE_MASK) & ~CVMX_CACHE_LINE_MASK;

	while (len) {
		if (cvmx_l2c_lock_line(start) != 0)
			retval--;
		start += CVMX_CACHE_LINE_SIZE;
		len -= CVMX_CACHE_LINE_SIZE;
	}
	return retval;
}

void cvmx_l2c_flush(void)
{
	uint64_t assoc, set;
	uint64_t n_assoc, n_set;

	n_set = cvmx_l2c_get_num_sets();
	n_assoc = cvmx_l2c_get_num_assoc();

	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) {
		uint64_t address;
		/* These may look like constants, but they aren't... */
		int assoc_shift = CVMX_L2C_TAG_ADDR_ALIAS_SHIFT;
		int set_shift = CVMX_L2C_IDX_ADDR_SHIFT;
		for (set = 0; set < n_set; set++) {
			for (assoc = 0; assoc < n_assoc; assoc++) {
				address = CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, (assoc << assoc_shift) | (set << set_shift));
				CVMX_CACHE_WBIL2I(address, 0);
			}
		}
	} else {
		for (set = 0; set < n_set; set++)
			for (assoc = 0; assoc < n_assoc; assoc++)
				cvmx_l2c_flush_line(assoc, set);
	}
}

int cvmx_l2c_unlock_line(uint64_t address)
{
	uint32_t tad = cvmx_l2c_address_to_tad(address);

	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) {
		int assoc;
		union cvmx_l2c_tag tag;
		uint32_t tag_addr;
		uint32_t index = cvmx_l2c_address_to_index(address);

		tag_addr = ((address >> CVMX_L2C_TAG_ADDR_ALIAS_SHIFT) & ((1 << CVMX_L2C_TAG_ADDR_ALIAS_SHIFT) - 1));

		/*
		 * For OcteonII, we can flush a line by using the physical
		 * address directly, so finding the cache line used by
		 * the address is only required to provide the proper
		 * return value for the function.
		 */
		for (assoc = 0; assoc < cvmx_l2c_get_num_assoc(); assoc++) {
			tag = cvmx_l2c_get_tag_v2(assoc, index, tad);

			if (tag.s.V && (tag.s.addr == tag_addr)) {
				CVMX_CACHE_WBIL2(CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, address), 0);
				return tag.s.L;
			}
		}
	} else {
		int assoc;
		union cvmx_l2c_tag tag;
		uint32_t tag_addr;

		uint32_t index = cvmx_l2c_address_to_index(address);

		/* Compute portion of address that is stored in tag */
		tag_addr = ((address >> CVMX_L2C_TAG_ADDR_ALIAS_SHIFT) & ((1 << CVMX_L2C_TAG_ADDR_ALIAS_SHIFT) - 1));
		for (assoc = 0; assoc < cvmx_l2c_get_num_assoc(); assoc++) {
			tag = cvmx_l2c_get_tag_v2(assoc, index, tad);

			if (tag.s.V && (tag.s.addr == tag_addr)) {
				cvmx_l2c_flush_line(assoc, index);
				return tag.s.L;
			}
		}
	}
	return 0;
}

int cvmx_l2c_unlock_mem_region(uint64_t start, uint64_t len)
{
	int num_unlocked = 0;
	/* Round start/end to cache line boundaries */
	len += start & CVMX_CACHE_LINE_MASK;
	start &= ~CVMX_CACHE_LINE_MASK;
	len = (len + CVMX_CACHE_LINE_MASK) & ~CVMX_CACHE_LINE_MASK;
	while (len > 0) {
		num_unlocked += cvmx_l2c_unlock_line(start);
		start += CVMX_CACHE_LINE_SIZE;
		len -= CVMX_CACHE_LINE_SIZE;
	}

	return num_unlocked;
}

/*
 * Internal l2c tag types.  These are converted to a generic structure
 * that can be used on all chips.
 */
union __cvmx_l2c_tag {
	uint64_t u64;
#ifdef __BIG_ENDIAN_BITFIELD
	struct cvmx_l2c_tag_cn50xx {
		uint64_t reserved:40;
		uint64_t V:1;	/* Line valid */
		uint64_t D:1;	/* Line dirty */
		uint64_t L:1;	/* Line locked */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t addr:20;	/* Phys mem addr (33..14) */
	} cn50xx;
	struct cvmx_l2c_tag_cn30xx {
		uint64_t reserved:41;
		uint64_t V:1;	/* Line valid */
		uint64_t D:1;	/* Line dirty */
		uint64_t L:1;	/* Line locked */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t addr:19;	/* Phys mem addr (33..15) */
	} cn30xx;
	struct cvmx_l2c_tag_cn31xx {
		uint64_t reserved:42;
		uint64_t V:1;	/* Line valid */
		uint64_t D:1;	/* Line dirty */
		uint64_t L:1;	/* Line locked */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t addr:18;	/* Phys mem addr (33..16) */
	} cn31xx;
	struct cvmx_l2c_tag_cn38xx {
		uint64_t reserved:43;
		uint64_t V:1;	/* Line valid */
		uint64_t D:1;	/* Line dirty */
		uint64_t L:1;	/* Line locked */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t addr:17;	/* Phys mem addr (33..17) */
	} cn38xx;
	struct cvmx_l2c_tag_cn58xx {
		uint64_t reserved:44;
		uint64_t V:1;	/* Line valid */
		uint64_t D:1;	/* Line dirty */
		uint64_t L:1;	/* Line locked */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t addr:16;	/* Phys mem addr (33..18) */
	} cn58xx;
#else
	struct cvmx_l2c_tag_cn50xx {
		uint64_t addr:20;	/* Phys mem addr (33..14) */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t L:1;	/* Line locked */
		uint64_t D:1;	/* Line dirty */
		uint64_t V:1;	/* Line valid */
		uint64_t reserved:40;
	} cn50xx;
	struct cvmx_l2c_tag_cn30xx {
		uint64_t addr:19;	/* Phys mem addr (33..15) */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t L:1;	/* Line locked */
		uint64_t D:1;	/* Line dirty */
		uint64_t V:1;	/* Line valid */
		uint64_t reserved:41;
	} cn30xx;
	struct cvmx_l2c_tag_cn31xx {
		uint64_t addr:18;	/* Phys mem addr (33..16) */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t L:1;	/* Line locked */
		uint64_t D:1;	/* Line dirty */
		uint64_t V:1;	/* Line valid */
		uint64_t reserved:42;
	} cn31xx;
	struct cvmx_l2c_tag_cn38xx {
		uint64_t addr:17;	/* Phys mem addr (33..17) */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t L:1;	/* Line locked */
		uint64_t D:1;	/* Line dirty */
		uint64_t V:1;	/* Line valid */
		uint64_t reserved:43;
	} cn38xx;
	struct cvmx_l2c_tag_cn58xx {
		uint64_t addr:16;	/* Phys mem addr (33..18) */
		uint64_t U:1;	/* Use, LRU eviction */
		uint64_t L:1;	/* Line locked */
		uint64_t D:1;	/* Line dirty */
		uint64_t V:1;	/* Line valid */
		uint64_t reserved:44;
	} cn58xx;
#endif
	struct cvmx_l2c_tag_cn58xx cn56xx;	/* 2048 sets */
	struct cvmx_l2c_tag_cn31xx cn52xx;	/* 512 sets */
};

/**
 * @INTERNAL
 * Function to read a L2C tag.  This code make the current core
 * the 'debug core' for the L2.  This code must only be executed by
 * 1 core at a time.
 *
 * @param assoc  Association (way) of the tag to dump
 * @param index  Index of the cacheline
 *
 * @return The Octeon model specific tag structure.  This is
 *         translated by a wrapper function to a generic form that is
 *         easier for applications to use.
 */
static union __cvmx_l2c_tag __read_l2_tag(uint64_t assoc, uint64_t index)
{

	uint64_t debug_tag_addr = CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, (index << 7) + 96);
	uint64_t core = cvmx_get_core_num();
	union __cvmx_l2c_tag tag_val;
	uint64_t dbg_addr = CVMX_L2C_DBG;
	unsigned long flags;

	union cvmx_l2c_dbg debug_val;
	debug_val.u64 = 0;
	/*
	 * For low core count parts, the core number is always small
	 * enough to stay in the correct field and not set any
	 * reserved bits.
	 */
	debug_val.s.ppnum = core;
	debug_val.s.l2t = 1;
	debug_val.s.set = assoc;

	cvmx_local_irq_save(flags);
	/*
	 * Make sure core is quiet (no prefetches, etc.) before
	 * entering debug mode.
	 */
	CVMX_SYNC;
	/* Flush L1 to make sure debug load misses L1 */
	CVMX_DCACHE_INVALIDATE;

	/*
	 * The following must be done in assembly as when in debug
	 * mode all data loads from L2 return special debug data, not
	 * normal memory contents.  Also, interrupts must be disabled,
	 * since if an interrupt occurs while in debug mode the ISR
	 * will get debug data from all its memory * reads instead of
	 * the contents of memory.
	 */

	asm volatile (".set push\n\t" ".set mips64\n\t" ".set noreorder\n\t" "sd    %[dbg_val], 0(%[dbg_addr])\n\t"	/* Enter debug mode, wait for store */
		      "ld    $0, 0(%[dbg_addr])\n\t" "ld    %[tag_val], 0(%[tag_addr])\n\t"	/* Read L2C tag data */
		      "sd    $0, 0(%[dbg_addr])\n\t"	/* Exit debug mode, wait for store */
		      "ld    $0, 0(%[dbg_addr])\n\t" "cache 9, 0($0)\n\t"	/* Invalidate dcache to discard debug data */
		      ".set pop":[tag_val] "=r"(tag_val)
		      :[dbg_addr] "r"(dbg_addr),[dbg_val] "r"(debug_val),[tag_addr] "r"(debug_tag_addr)
		      :"memory");

	cvmx_local_irq_restore(flags);

	return tag_val;
}

union cvmx_l2c_tag cvmx_l2c_get_tag_v2(uint32_t association, uint32_t index, uint32_t tad)
{
	union cvmx_l2c_tag tag;
	tag.u64 = 0;

	if ((int)association >= cvmx_l2c_get_num_assoc()) {
		cvmx_dprintf("ERROR: cvmx_l2c_get_tag association out of range\n");
		return tag;
	}
	if ((int)index >= cvmx_l2c_get_num_sets()) {
		cvmx_dprintf("ERROR: cvmx_l2c_get_tag index out of range (arg: %d, max: %d)\n", (int)index, cvmx_l2c_get_num_sets());
		return tag;
	}
	if (OCTEON_IS_OCTEON2()  || OCTEON_IS_OCTEON3()) {
		union cvmx_l2c_tadx_tag l2c_tadx_tag;
		uint64_t address = CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS,
						(association << CVMX_L2C_TAG_ADDR_ALIAS_SHIFT) | (index << CVMX_L2C_IDX_ADDR_SHIFT));
		/*
		 * Use L2 cache Index load tag cache instruction, as
		 * hardware loads the virtual tag for the L2 cache
		 * block with the contents of L2C_TAD0_TAG
		 * register.
		 */
		if (tad > CVMX_L2C_TADS) {
			cvmx_dprintf("ERROR: cvmx_l2c_get_tag_v2: TAD#%d out of range\n", (unsigned int)tad);
			return tag;
		}
		CVMX_CACHE_LTGL2I(address, 0);
		CVMX_SYNC;	/* make sure CVMX_L2C_TADX_TAG is updated */
		l2c_tadx_tag.u64 = cvmx_read_csr(CVMX_L2C_TADX_TAG(tad));

		if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
			if (l2c_tadx_tag.cn78xx.ts != 0)
				tag.s.V = 1;
			tag.s.D = l2c_tadx_tag.cn78xx.sblkdty;
			tag.s.L = l2c_tadx_tag.cn78xx.lock;
			tag.s.U = l2c_tadx_tag.cn78xx.used;
			tag.s.addr = l2c_tadx_tag.cn78xx.tag;
		} else if (OCTEON_IS_MODEL(OCTEON_CN73XX)
			   || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
			if (l2c_tadx_tag.cn73xx.ts != 0)
				tag.s.V = 1;
			tag.s.D = l2c_tadx_tag.cn73xx.sblkdty;
			tag.s.L = l2c_tadx_tag.cn73xx.lock;
			tag.s.U = l2c_tadx_tag.cn73xx.used;
			tag.s.addr = l2c_tadx_tag.cn73xx.tag;
		} else {
			tag.s.V = l2c_tadx_tag.cn61xx.valid;
			tag.s.D = l2c_tadx_tag.cn61xx.dirty;
			tag.s.L = l2c_tadx_tag.cn61xx.lock;
			tag.s.U = l2c_tadx_tag.cn61xx.use;
			tag.s.addr = (OCTEON_IS_MODEL(OCTEON_CN70XX)
				? l2c_tadx_tag.cn70xx.tag
				   : l2c_tadx_tag.cn61xx.tag);
		}
	} else {
		union __cvmx_l2c_tag tmp_tag;
		/* __read_l2_tag is intended for internal use only */
		tmp_tag = __read_l2_tag(association, index);

		/*
		 * Convert all tag structure types to generic version,
		 * as it can represent all models.
		 */
		if (OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)) {
			tag.s.V = tmp_tag.cn58xx.V;
			tag.s.D = tmp_tag.cn58xx.D;
			tag.s.L = tmp_tag.cn58xx.L;
			tag.s.U = tmp_tag.cn58xx.U;
			tag.s.addr = tmp_tag.cn58xx.addr;
		} else if (OCTEON_IS_MODEL(OCTEON_CN38XX)) {
			tag.s.V = tmp_tag.cn38xx.V;
			tag.s.D = tmp_tag.cn38xx.D;
			tag.s.L = tmp_tag.cn38xx.L;
			tag.s.U = tmp_tag.cn38xx.U;
			tag.s.addr = tmp_tag.cn38xx.addr;
		} else if (OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN52XX)) {
			tag.s.V = tmp_tag.cn31xx.V;
			tag.s.D = tmp_tag.cn31xx.D;
			tag.s.L = tmp_tag.cn31xx.L;
			tag.s.U = tmp_tag.cn31xx.U;
			tag.s.addr = tmp_tag.cn31xx.addr;
		} else if (OCTEON_IS_MODEL(OCTEON_CN30XX)) {
			tag.s.V = tmp_tag.cn30xx.V;
			tag.s.D = tmp_tag.cn30xx.D;
			tag.s.L = tmp_tag.cn30xx.L;
			tag.s.U = tmp_tag.cn30xx.U;
			tag.s.addr = tmp_tag.cn30xx.addr;
		} else if (OCTEON_IS_MODEL(OCTEON_CN50XX)) {
			tag.s.V = tmp_tag.cn50xx.V;
			tag.s.D = tmp_tag.cn50xx.D;
			tag.s.L = tmp_tag.cn50xx.L;
			tag.s.U = tmp_tag.cn50xx.U;
			tag.s.addr = tmp_tag.cn50xx.addr;
		} else {
			cvmx_dprintf("Unsupported OCTEON Model in %s\n", __func__);
		}
	}
	return tag;
}

union cvmx_l2c_tag cvmx_l2c_get_tag(uint32_t association, uint32_t index)
{
	union cvmx_l2c_tag tag;
	tag.u64 = 0;

	if ((int)association >= cvmx_l2c_get_num_assoc()) {
		cvmx_dprintf("ERROR: cvmx_l2c_get_tag association out of range\n");
		return tag;
	}
	if ((int)index >= cvmx_l2c_get_num_sets()) {
		cvmx_dprintf("ERROR: cvmx_l2c_get_tag index out of range (arg: %d, max: %d)\n", (int)index, cvmx_l2c_get_num_sets());
		return tag;
	}
	if (OCTEON_IS_OCTEON2()  || OCTEON_IS_OCTEON3()) {
		union cvmx_l2c_tadx_tag l2c_tadx_tag;
		uint64_t address = CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS,
						(association << CVMX_L2C_TAG_ADDR_ALIAS_SHIFT) | (index << CVMX_L2C_IDX_ADDR_SHIFT));
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			cvmx_dprintf("ERROR: Cannot use %s on OCTEON CN68XX, use cvmx_l2c_get_tag_v2 instead!\n", __func__);
			return tag;
		}
		/*
		 * Use L2 cache Index load tag cache instruction, as
		 * hardware loads the virtual tag for the L2 cache
		 * block with the contents of L2C_TAD0_TAG
		 * register.
		 */
		CVMX_CACHE_LTGL2I(address, 0);
		CVMX_SYNC;	/* make sure CVMX_L2C_TADX_TAG is updated */
		l2c_tadx_tag.u64 = cvmx_read_csr(CVMX_L2C_TADX_TAG(0));

		if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
			if (l2c_tadx_tag.cn78xx.ts == 0)
				tag.s.V = 1;
			tag.s.D = l2c_tadx_tag.cn78xx.sblkdty;
			tag.s.L = l2c_tadx_tag.cn78xx.lock;
			tag.s.U = l2c_tadx_tag.cn78xx.used;
			tag.s.addr = l2c_tadx_tag.cn78xx.tag;
		} else if (OCTEON_IS_MODEL(OCTEON_CN73XX)
			   || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
			if (l2c_tadx_tag.cn73xx.ts == 0)
				tag.s.V = 1;
			tag.s.D = l2c_tadx_tag.cn73xx.sblkdty;
			tag.s.L = l2c_tadx_tag.cn73xx.lock;
			tag.s.U = l2c_tadx_tag.cn73xx.used;
			tag.s.addr = l2c_tadx_tag.cn73xx.tag;
		} else {
			tag.s.V = l2c_tadx_tag.cn61xx.valid;
			tag.s.D = l2c_tadx_tag.cn61xx.dirty;
			tag.s.L = l2c_tadx_tag.cn61xx.lock;
			tag.s.U = l2c_tadx_tag.cn61xx.use;
			tag.s.addr = (OCTEON_IS_MODEL(OCTEON_CN70XX)
				? l2c_tadx_tag.cn70xx.tag
				   : l2c_tadx_tag.cn61xx.tag);
		}
	} else {
		union __cvmx_l2c_tag tmp_tag;
		/* __read_l2_tag is intended for internal use only */
		tmp_tag = __read_l2_tag(association, index);

		/*
		 * Convert all tag structure types to generic version,
		 * as it can represent all models.
		 */
		if (OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)) {
			tag.s.V = tmp_tag.cn58xx.V;
			tag.s.D = tmp_tag.cn58xx.D;
			tag.s.L = tmp_tag.cn58xx.L;
			tag.s.U = tmp_tag.cn58xx.U;
			tag.s.addr = tmp_tag.cn58xx.addr;
		} else if (OCTEON_IS_MODEL(OCTEON_CN38XX)) {
			tag.s.V = tmp_tag.cn38xx.V;
			tag.s.D = tmp_tag.cn38xx.D;
			tag.s.L = tmp_tag.cn38xx.L;
			tag.s.U = tmp_tag.cn38xx.U;
			tag.s.addr = tmp_tag.cn38xx.addr;
		} else if (OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN52XX)) {
			tag.s.V = tmp_tag.cn31xx.V;
			tag.s.D = tmp_tag.cn31xx.D;
			tag.s.L = tmp_tag.cn31xx.L;
			tag.s.U = tmp_tag.cn31xx.U;
			tag.s.addr = tmp_tag.cn31xx.addr;
		} else if (OCTEON_IS_MODEL(OCTEON_CN30XX)) {
			tag.s.V = tmp_tag.cn30xx.V;
			tag.s.D = tmp_tag.cn30xx.D;
			tag.s.L = tmp_tag.cn30xx.L;
			tag.s.U = tmp_tag.cn30xx.U;
			tag.s.addr = tmp_tag.cn30xx.addr;
		} else if (OCTEON_IS_MODEL(OCTEON_CN50XX)) {
			tag.s.V = tmp_tag.cn50xx.V;
			tag.s.D = tmp_tag.cn50xx.D;
			tag.s.L = tmp_tag.cn50xx.L;
			tag.s.U = tmp_tag.cn50xx.U;
			tag.s.addr = tmp_tag.cn50xx.addr;
		} else {
			cvmx_dprintf("Unsupported OCTEON Model in %s\n", __func__);
		}
	}
	return tag;
}
#endif

int cvmx_l2c_address_to_tad(uint64_t addr)
{
	uint32_t tad;
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)
	    || OCTEON_IS_MODEL(OCTEON_CN73XX)
	    || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_l2c_ctl_t l2c_ctl;
		l2c_ctl.u64 = cvmx_read_csr(CVMX_L2C_CTL);
		if (!l2c_ctl.s.disidxalias) {
			tad = ((addr >> 7) ^ (addr >> 12) ^ (addr >> 18)) & 3;
		} else {
			tad = (addr >> 7) & 3;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_l2c_ctl_t l2c_ctl;
		l2c_ctl.u64 = cvmx_read_csr(CVMX_L2C_CTL);
		if (!l2c_ctl.s.disidxalias) {
			tad = ((addr >> 7) ^ (addr >> 12) ^ (addr >> 20)) & 7;
		} else {
			tad = (addr >> 7) & 7;
		}
	} else {
		tad = 0;
	}
	return tad;
}

uint32_t cvmx_l2c_v2_address_to_tag(uint64_t addr)
{
	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) {
		if ((addr > (OCTEON_DDR0_SIZE - 1))
		    && (addr < OCTEON_DDR1_BASE))
			return (uint32_t) (-1);

		if (addr > OCTEON_DDR1_BASE)
			addr = addr - OCTEON_DDR0_SIZE;

		addr = addr & 0x7FFFFFFFFULL;
		return (uint32_t) (addr >> CVMX_L2C_TAG_ADDR_ALIAS_SHIFT);
	}
	return -1;
}

uint32_t cvmx_l2c_address_to_index(uint64_t addr)
{
	uint64_t idx = addr >> CVMX_L2C_IDX_ADDR_SHIFT;
	int indxalias = 0;

	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) {
		union cvmx_l2c_ctl l2c_ctl;
		l2c_ctl.u64 = cvmx_read_csr(CVMX_L2C_CTL);
		indxalias = !l2c_ctl.s.disidxalias;
	} else {
		union cvmx_l2c_cfg l2c_cfg;
		l2c_cfg.u64 = cvmx_read_csr(CVMX_L2C_CFG);
		indxalias = l2c_cfg.s.idxalias;
	}

	if (indxalias) {
		/* For 4 TADs */
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)
		    || OCTEON_IS_MODEL(OCTEON_CN73XX)
		    || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
			uint32_t a_14_12 = (idx / (CVMX_L2C_MEMBANK_SELECT_SIZE / (1 << CVMX_L2C_IDX_ADDR_SHIFT))) & 0x7;
			idx ^= (idx / cvmx_l2c_get_num_sets()) & 0x3ff;
			idx ^= a_14_12 & 0x3;
			idx ^= a_14_12 << 2;
		/* For 8 TADs */
		} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
			uint32_t a_14_12 = (idx / (CVMX_L2C_MEMBANK_SELECT_SIZE / (1 << CVMX_L2C_IDX_ADDR_SHIFT))) & 0x7;
			uint64_t above_normal_index = (idx / cvmx_l2c_get_num_sets()) & 0xff;	// A<27:20>
			idx ^= above_normal_index;		  // XOR in A<27:20>
			idx ^= (above_normal_index & 0x1F) << 8;  // XOR in A<24:20>
			idx ^= a_14_12;				  // XOR in A<14:12>
			idx ^= (a_14_12 & 0x3) << 3;		  // XOR in A<13:12>
		} else if (OCTEON_IS_OCTEON2()
			   || OCTEON_IS_MODEL(OCTEON_CN70XX)) {
			uint32_t a_14_12 = (idx / (CVMX_L2C_MEMBANK_SELECT_SIZE / (1 << CVMX_L2C_IDX_ADDR_SHIFT))) & 0x7;
			idx ^= idx / cvmx_l2c_get_num_sets();
			idx ^= a_14_12;
		} else {
			idx ^= ((addr & CVMX_L2C_ALIAS_MASK) >> CVMX_L2C_TAG_ADDR_ALIAS_SHIFT);
		}
	}
	idx &= CVMX_L2C_IDX_MASK;
	return idx;
}

/**
 * Decodes TQD L2D single and double-bit errors to the appropriate cache index
 * for the CVMX_CACHE_LTGL2I operation.
 *
 * @param	node	CPU node number
 * @param	tad	TAD interface
 *
 * @return	index address to pass to the LTGL2I cache operation (3)
 */
uint64_t cvmx_l2c_tqdl2d_to_index_7xxx(int node, int tad)
{
	union cvmx_l2c_tqdx_err l2c_tqdx_err;
	uint64_t cindex;

	l2c_tqdx_err.u64 = cvmx_read_csr_node(node, CVMX_L2C_TQDX_ERR(tad));

	/* Figure out the index to pass to the cache 3 instruction
	 * Note that this is poorly documented and in many cases the HRM is
	 * wrong.
	 *
	 * Also note that L2 index aliasing has no impact on this.
	 *
	 * Please see the comments for each chip model for the correct
	 * mapping.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		/* This applies to t88 as well.
		 *
		 * payload<24> = remote node
		 * payload<23:20> = L2DIDX<10:7>   = way[3:0]
		 * payload<19:13> = L2DIDX<6:0>    = index<12:6>
		 * payload<12:11> = L2DIDX<12:11>  = index<5:4>
		 * payload<10>    = QDNUM<2>       = index<3>
		 * payload<9:7>   = TAD[2:0]       = index<0:2>
		 */
		cindex = (tad & 7) << 7;	/* 7:9 */
		cindex |= ((l2c_tqdx_err.s.qdnum >> 2) & 1) << 10;
		cindex |= ((l2c_tqdx_err.s.l2didx >> 11) & 3) << 11;
		cindex |= (l2c_tqdx_err.s.l2didx & 0x7f) << 13;
		cindex |= ((l2c_tqdx_err.s.l2didx >> 7) & 0xF) << 20;
		if (cvmx_get_node_num() != (unsigned)node)
			cindex |= 1 << 24;
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		/* payload<24:22> = 0
		 * payload<18:17> = L2DIDX<10:9> = way[1:0]
		 * payload<16:8>  = L2DIDX<8:0>  = index<9:1>
		 * payload<7>     = L2DIDX<12>   = index<0>
		 */
		cindex = ((l2c_tqdx_err.s.l2didx >> 12) & 1) << 7;
		cindex |= (l2c_tqdx_err.s.l2didx & 0xfff) << 8;
	} else {	/* CN73XX/CNF75XX */
		/* payload<24:22> = 0
		 * payload<21:18> = L2DIDX<10:7>  = way[3:0]
		 * payload<17:11> = L2DIDX<6:0>   = index<10:4>
		 * payload<10:9>  = L2DIDX<12:11> = index<3:2>
		 * payload<8:7>   = TAD[1:0]      = index<0:1>
		 */
		cindex = (tad & 3) << 7;	/* 7:8 */
		cindex |= (l2c_tqdx_err.s.l2didx & 0x1800) >> 2; /* 9:10 */
		cindex |= (l2c_tqdx_err.s.l2didx & 0x7f) << 11;  /* 11:17 */
		cindex |= (l2c_tqdx_err.s.l2didx & 0x780) << 11; /* 18:21 */
	}
	/* t81:
	 * payload<24:21> = 0
	 * payload<20:17> = L2DIDX<10:7>  = way[3:0]
	 * payload<16:10> = L2DIDX<6:0>   = index<9:3>
	 * payload<9:8>   = L2DIDX<12:11> = index<2:1>
	 * payload<7>     = QDNUM<2>      = index<0>
	 *
	 * t83:
	 * payload<24:23> = 0
	 * payload<22:19> = L2DIDX<10:7>  = way[3:0]
	 * payload<18:12> = L2DIDX<6:0>   = index<11:5>
	 * payload<11:10> = L2DIDX<12:11> = index<4:3>
	 * payload<9>     = QDNUM<2>      = index<2>
	 * payload<8:7>   = TAD[1:0]      = index<1:0>
	 */
	return cindex;
}

/**
 * Decodes TTG tag single and double-bit errors to the appropriate cache index
 * for the CVMX_CACHE_LTGL2I operation.
 *
 * @param	node	CPU node number
 * @param	tad	TAD interface
 *
 * @return	index address to pass to the LTGL2I cache operation (3)
 */
uint64_t cvmx_l2c_ttgx_to_index_7xxx(int node, int tad, bool remote)
{
	union cvmx_l2c_ttgx_err l2c_ttgx_err;
	union cvmx_l2c_rtgx_err l2c_rtgx_err;
	uint64_t cindex;

	if (remote)
		l2c_rtgx_err.u64 = cvmx_read_csr_node(node, CVMX_L2C_RTGX_ERR(tad));
	else
		l2c_ttgx_err.u64 = cvmx_read_csr_node(node, CVMX_L2C_TTGX_ERR(tad));

	/* Figure out the index to pass to the cache 3 instruction
	 * Note that this is poorly documented and in many cases the HRM is
	 * wrong.
	 *
	 * Also note that L2 index aliasing has no impact on this.
	 *
	 * Please see the comments for each chip model for the correct
	 * mapping.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		/* This applies to t88 as well.
		 *
		 * payload<24> = remote node
		 * payload<23:20> = L2DIDX<10:7>   = way[3:0]
		 * payload<19:13> = L2DIDX<6:0>    = index<12:6>
		 * payload<12:11> = L2DIDX<12:11>  = index<5:4>
		 * payload<10>    = QDNUM<2>       = index<3>
		 * payload<9:7>   = TAD[2:0]       = index<0:2> = addr<7:9?
		 */

		/* cindex = (tad & 7) << 7; */	/* 7:9 are the tad */
		if (remote) {
			cindex = l2c_rtgx_err.s.l2idx << 7;	/* 7:19 index */
			cindex |= l2c_rtgx_err.s.way << 20;	/* 20:23 way */
			cindex |= (1 << 24);			/* Remote tag */
		} else {
			cindex = l2c_ttgx_err.cn78xx.l2idx << 7;/* 7:19 index */
			cindex |= l2c_ttgx_err.cn78xx.way << 20;/* 20:23 way */
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		/* payload<24:22> = 0
		 * payload<18:17> = L2DIDX<10:9> = way[1:0]
		 * payload<16:8>  = L2DIDX<8:0>  = index<9:1>
		 * payload<7>     = L2DIDX<12>   = index<0>
		 */
		cindex = l2c_ttgx_err.cn70xx.l2idx << 7;	/* 7:16 index */
		cindex |= l2c_ttgx_err.cn70xx.way << 17;	/* 17:18 way */
	} else {	/* CN73XX/CNF75XX */
		/* payload<24:22> = 0
		 * payload<18:17> = L2DIDX<10:9> = way[1:0]
		 * payload<16:8>  = L2DIDX<8:0>  = index<9:1>
		 * payload<7>     = L2DIDX<12>   = index<0>
		 */
		cindex = l2c_ttgx_err.cn73xx.l2idx << 7;	/* 7:19 index */
		cindex |= l2c_ttgx_err.cn73xx.way << 20;	/* 20:23 way */
	}
	/* t81:
	 * payload<24:21> = 0
	 * payload<20:17> = L2DIDX<10:7>  = way[3:0]
	 * payload<16:10> = L2DIDX<6:0>   = index<9:3>
	 * payload<9:8>   = L2DIDX<12:11> = index<2:1>
	 * payload<7>     = QDNUM<2>      = index<0>
	 *
	 * t83:
	 * payload<24:23> = 0
	 * payload<22:19> = L2DIDX<10:7>  = way[3:0]
	 * payload<18:12> = L2DIDX<6:0>   = index<11:5>
	 * payload<11:10> = L2DIDX<12:11> = index<4:3>
	 * payload<9>     = QDNUM<2>      = index<2>
	 * payload<8:7>   = TAD[1:0]      = index<1:0>
	 */
	return cindex;
}

int cvmx_l2c_get_cache_size_bytes(void)
{
	return cvmx_l2c_get_num_sets() * cvmx_l2c_get_num_assoc() * CVMX_CACHE_LINE_SIZE;
}

/**
 * Return log base 2 of the number of sets in the L2 cache
 * @return
 */
int cvmx_l2c_get_set_bits(void)
{
	int l2_set_bits;
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		l2_set_bits = 13;	/* 8192 sets */
	else if (OCTEON_IS_MODEL(OCTEON_CN56XX)
		 || OCTEON_IS_MODEL(OCTEON_CN58XX)
		 || OCTEON_IS_MODEL(OCTEON_CN68XX)
		 || OCTEON_IS_MODEL(OCTEON_CN73XX)
		 || OCTEON_IS_MODEL(OCTEON_CNF75XX))
		l2_set_bits = 11;	/* 2048 sets */
	else if (OCTEON_IS_MODEL(OCTEON_CN38XX)
		 || OCTEON_IS_MODEL(OCTEON_CN63XX)
		 || OCTEON_IS_MODEL(OCTEON_CN66XX)
		 || OCTEON_IS_MODEL(OCTEON_CN70XX))
		l2_set_bits = 10;	/* 1024 sets */
	else if (OCTEON_IS_MODEL(OCTEON_CN31XX)
		 || OCTEON_IS_MODEL(OCTEON_CN52XX)
		 || OCTEON_IS_MODEL(OCTEON_CN61XX)
		 || OCTEON_IS_MODEL(OCTEON_CNF71XX))
		l2_set_bits = 9;	/* 512 sets */
	else if (OCTEON_IS_MODEL(OCTEON_CN30XX))
		l2_set_bits = 8;	/* 256 sets */
	else if (OCTEON_IS_MODEL(OCTEON_CN50XX))
		l2_set_bits = 7;	/* 128 sets */
	else {
		cvmx_dprintf("Unsupported OCTEON Model in %s\n", __func__);
		l2_set_bits = 11;	/* 2048 sets */
	}
	return l2_set_bits;
}

/* Return the number of sets in the L2 Cache */
int cvmx_l2c_get_num_sets(void)
{
	return 1 << cvmx_l2c_get_set_bits();
}

/* Return the number of associations in the L2 Cache */
int cvmx_l2c_get_num_assoc(void)
{
	int l2_assoc;

	if (OCTEON_IS_MODEL(OCTEON_CN56XX)
		|| OCTEON_IS_MODEL(OCTEON_CN52XX)
		|| OCTEON_IS_MODEL(OCTEON_CN58XX)
		|| OCTEON_IS_MODEL(OCTEON_CN50XX)
		|| OCTEON_IS_MODEL(OCTEON_CN38XX))
		l2_assoc = 8;
	else if (OCTEON_IS_OCTEON2()
		 || OCTEON_IS_MODEL(OCTEON_CN78XX)
		 || OCTEON_IS_MODEL(OCTEON_CN73XX)
		 || OCTEON_IS_MODEL(OCTEON_CNF75XX))
		l2_assoc = 16;
	else if (OCTEON_IS_MODEL(OCTEON_CN31XX)
		|| OCTEON_IS_MODEL(OCTEON_CN30XX))
		l2_assoc = 4;
	else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		union cvmx_mio_fus_dat3 mio_fus_dat3;

		mio_fus_dat3.u64 = cvmx_read_csr(CVMX_MIO_FUS_DAT3);

		switch (mio_fus_dat3.s.l2c_crip) {
		case 3:  /* 1/4 size */
			l2_assoc = 1;
			break;
		case 2:  /* 1/2 size */
			l2_assoc = 2;
			break;
		case 1:  /* 3/4 size */
			l2_assoc = 3;
			break;
		default: /* Full size */
			l2_assoc = 4;
			break;
		}
		return l2_assoc;
	} else {
		cvmx_dprintf("Unsupported OCTEON Model in %s\n", __func__);
		l2_assoc = 8;
	}

	/* Check to see if part of the cache is disabled */
	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) {
		union cvmx_mio_fus_dat3 mio_fus_dat3;
		union cvmx_l2c_wpar_iobx l2c_wpar_iob;

		mio_fus_dat3.u64 = cvmx_read_csr(CVMX_MIO_FUS_DAT3);
		l2c_wpar_iob.u64 = cvmx_read_csr(CVMX_L2C_WPAR_IOBX(0));
		/*
		 * cvmx_mio_fus_dat3.s.l2c_crip fuses map as follows
		 * <2> will be not used for 63xx
		 * <1> disables 1/2 ways
		 * <0> disables 1/4 ways
		 * They are cumulative, so for 63xx:
		 * <1> <0>
		 * 0 0 16-way 2MB cache
		 * 0 1 12-way 1.5MB cache
		 * 1 0 8-way 1MB cache
		 * 1 1 4-way 512KB cache
		 */

		if (mio_fus_dat3.cn63xx.l2c_crip == 3)
			l2_assoc = 4;
		else if (mio_fus_dat3.cn63xx.l2c_crip == 2)
			l2_assoc = 8;
		else if (mio_fus_dat3.cn63xx.l2c_crip == 1)
			l2_assoc = 12;
		else if (l2c_wpar_iob.s.mask)
			l2_assoc = cvmx_dpop(~(l2c_wpar_iob.s.mask) & 0xffff);
	} else {
		union cvmx_l2d_fus3 val;
		val.u64 = cvmx_read_csr(CVMX_L2D_FUS3);
		/*
		 * Using shifts here, as bit position names are
		 * different for each model but they all mean the
		 * same.
		 */
		if ((val.u64 >> 35) & 0x1)
			l2_assoc = l2_assoc >> 2;
		else if ((val.u64 >> 34) & 0x1)
			l2_assoc = l2_assoc >> 1;
	}
	return l2_assoc;
}

#ifndef CVMX_BUILD_FOR_LINUX_HOST
/**
 * Flush a line from the L2 cache
 * This should only be called from one core at a time, as this routine
 * sets the core to the 'debug' core in order to flush the line.
 *
 * @param assoc  Association (or way) to flush
 * @param index  Index to flush
 */
void cvmx_l2c_flush_line(uint32_t assoc, uint32_t index)
{
	/* Check the range of the index. */
	if (index > (uint32_t) cvmx_l2c_get_num_sets()) {
		cvmx_dprintf("ERROR: cvmx_l2c_flush_line index out of range.\n");
		return;
	}

	/* Check the range of association. */
	if (assoc > (uint32_t) cvmx_l2c_get_num_assoc()) {
		cvmx_dprintf("ERROR: cvmx_l2c_flush_line association out of range.\n");
		return;
	}

	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) {
		uint64_t address;
		/* Create the address based on index and association.
		 * Bits<20:17> select the way of the cache block involved in
		 *             the operation
		 * Bits<16:7> of the effect address select the index
		 */
		address = CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, (assoc << CVMX_L2C_TAG_ADDR_ALIAS_SHIFT) | (index << CVMX_L2C_IDX_ADDR_SHIFT));
		CVMX_CACHE_WBIL2I(address, 0);
	} else {
		union cvmx_l2c_dbg l2cdbg;

		l2cdbg.u64 = 0;
		if (!OCTEON_IS_MODEL(OCTEON_CN30XX))
			l2cdbg.s.ppnum = cvmx_get_core_num();
		l2cdbg.s.finv = 1;

		l2cdbg.s.set = assoc;
		cvmx_spinlock_lock(&cvmx_l2c_spinlock);
		/*
		 * Enter debug mode, and make sure all other writes
		 * complete before we enter debug mode
		 */
		CVMX_SYNC;
		cvmx_write_csr(CVMX_L2C_DBG, l2cdbg.u64);
		cvmx_read_csr(CVMX_L2C_DBG);

		CVMX_PREPARE_FOR_STORE(CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, index * CVMX_CACHE_LINE_SIZE), 0);
		/* Exit debug mode */
		CVMX_SYNC;
		cvmx_write_csr(CVMX_L2C_DBG, 0);
		cvmx_read_csr(CVMX_L2C_DBG);
		cvmx_spinlock_unlock(&cvmx_l2c_spinlock);
	}
}
#endif

/**
 * Initialize the BIG address in L2C+DRAM to generate proper error
 * on reading/writing to an non-existant memory location.
 *
 * @param node      OCX CPU node number
 * @param mem_size  Amount of DRAM configured in MB.
 * @param mode      Allow/Disallow reporting errors L2C_INT_SUM[BIGRD,BIGWR].
 */
void cvmx_l2c_set_big_size_node(int node, uint64_t mem_size, int mode)
{
	if ((OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3())
	    && !OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_X)) {
		cvmx_l2c_big_ctl_t big_ctl;
		int bits = 0, zero_bits = 0;
		uint64_t mem;

		if (mem_size > (CVMX_L2C_MAX_MEMSZ_ALLOWED * 1024ull)) {
			cvmx_dprintf("WARNING: Invalid memory size(%lld) requested, should be <= %lld\n",
				     (unsigned long long)mem_size, (unsigned long long)CVMX_L2C_MAX_MEMSZ_ALLOWED * 1024);
			mem_size = CVMX_L2C_MAX_MEMSZ_ALLOWED * 1024;
		}

		mem = mem_size;
		while (mem) {
			if ((mem & 1) == 0)
				zero_bits++;
			bits++;
			mem >>= 1;
		}

		if ((bits - zero_bits) != 1 || (bits - 9) <= 0) {
			cvmx_dprintf("ERROR: Invalid DRAM size (%lld) requested, refer to L2C_BIG_CTL[maxdram] for valid options.\n", (unsigned long long)mem_size);
			return;
		}

		/*
  		 * The BIG/HOLE is logic is not supported in pass1 as per
		 * Errata L2C-17736
		 */
		if (mode == 0 && OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			mode = 1;

		big_ctl.u64 = 0;
		big_ctl.s.maxdram = bits - 9;
		big_ctl.cn61xx.disable = mode;
		cvmx_write_csr_node(node, CVMX_L2C_BIG_CTL, big_ctl.u64);
	}
}

/**
 * Initialize the BIG address in L2C+DRAM to generate proper error
 * on reading/writing to an non-existant memory location.
 *
 * @param mem_size  Amount of DRAM configured in MB.
 * @param mode      Allow/Disallow reporting errors L2C_INT_SUM[BIGRD,BIGWR].
 */
void cvmx_l2c_set_big_size(uint64_t mem_size, int mode)
{
	cvmx_l2c_set_big_size_node(0, mem_size, mode);
}


#if !defined(CVMX_BUILD_FOR_LINUX_HOST) && !defined(CVMX_BUILD_FOR_LINUX_KERNEL)
/* L2C Virtualization APIs. These APIs are based on Octeon II documentation. */

/*
 * These could be used by the Linux kernel, but currently are not, so
 * disable them to save space.
 */

/**
 * @INTERNAL
 * Helper function to decode VALUE to number of allowed virtualization IDS.
 * Returns L2C_VRT_CTL[NUMID].
 *
 * @param nvid     Number of virtual Ids.
 * @return         On success decode to NUMID, or to -1 on failure.
 */
static inline int __cvmx_l2c_vrt_decode_numid(int nvid)
{
	int bits = -1;
	int zero_bits = -1;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return -1;

	if (nvid < 1 || nvid > CVMX_L2C_VRT_MAX_VIRTID_ALLOWED) {
		cvmx_dprintf("WARNING: Invalid number of virtual ids(%d) requested, should be <= 64\n", nvid);
		return bits;
	}

	while (nvid) {
		if ((nvid & 1) == 0)
			zero_bits++;

		bits++;
		nvid >>= 1;
	}

	if (bits == 1 || (zero_bits && ((bits - zero_bits) == 1)))
		return zero_bits;
	return -1;
}

/**
 * Set maxium number of Virtual IDs allowed in a machine.
 *
 * @param nvid   Number of virtial ids allowed in a machine.
 * @return       Return 0 on success or -1 on failure.
 */
int cvmx_l2c_vrt_set_max_virtids(int nvid)
{
	cvmx_l2c_vrt_ctl_t l2c_vrt_ctl;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return -1;

	l2c_vrt_ctl.u64 = cvmx_read_csr(CVMX_L2C_VRT_CTL);

	if (l2c_vrt_ctl.s.enable) {
		cvmx_dprintf("WARNING: Changing number of Virtual Machine IDs is not allowed after Virtualization is enabled\n");
		return -1;
	}

	if (nvid < 1 || nvid > CVMX_L2C_VRT_MAX_VIRTID_ALLOWED) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_set_max_virtids: Invalid number of Virtual Machine IDs(%d) requested, max allowed %d\n", nvid, CVMX_L2C_VRT_MAX_VIRTID_ALLOWED);
		return -1;
	}

	/* Calculate the numid based on nvid */
	l2c_vrt_ctl.s.numid = __cvmx_l2c_vrt_decode_numid(nvid);
	cvmx_write_csr(CVMX_L2C_VRT_CTL, l2c_vrt_ctl.u64);
	return 0;
}

/**
 * Get maxium number of virtual IDs allowed in a machine.
 *
 * @return  Return number of virtual machine IDs or -1 on failure.
 */
int cvmx_l2c_vrt_get_max_virtids(void)
{
	int virtids;
	cvmx_l2c_vrt_ctl_t l2c_vrt_ctl;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return -1;

	l2c_vrt_ctl.u64 = cvmx_read_csr(CVMX_L2C_VRT_CTL);
	virtids = 1 << (l2c_vrt_ctl.s.numid + 1);
	if (virtids > CVMX_L2C_VRT_MAX_VIRTID_ALLOWED) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_get_max_virtids: Invalid number of Virtual IDs initialized (%d)\n", virtids);
		return -1;
	}
	return virtids;
}

/**
 * @INTERNAL
 * Helper function to decode VALUE to memory space coverage of L2C_VRT_MEM.
 * Returns L2C_VRT_CTL[MEMSZ].
 *
 * @param memsz    Memory in GB.
 * @return         On success, decode to MEMSZ, or on failure return -1.
 */
static inline int __cvmx_l2c_vrt_decode_memsize(int memsz)
{
	int bits = 0;
	int zero_bits = 0;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return -1;

	if (memsz == 0 || memsz > CVMX_L2C_MAX_MEMSZ_ALLOWED) {
		cvmx_dprintf("WARNING: Invalid virtual memory size(%d) requested, should be <= %d\n", memsz, CVMX_L2C_MAX_MEMSZ_ALLOWED);
		return -1;
	}

	while (memsz) {
		if ((memsz & 1) == 0)
			zero_bits++;

		bits++;
		memsz >>= 1;
	}

	if (bits == 1 || (bits - zero_bits) == 1)
		return zero_bits;
	return -1;
}

/**
 * Set the maxium size of memory space to be allocated for virtualization.
 *
 * @param memsz  Size of the virtual memory in GB
 * @return       Return 0 on success or -1 on failure.
 */
int cvmx_l2c_vrt_set_max_memsz(int memsz)
{
	cvmx_l2c_vrt_ctl_t l2c_vrt_ctl;
	int decode = 0;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return -1;

	l2c_vrt_ctl.u64 = cvmx_read_csr(CVMX_L2C_VRT_CTL);

	if (l2c_vrt_ctl.s.enable) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_set_memsz: Changing the size of the memory after Virtualization is enabled is not allowed.\n");
		return -1;
	}

	if (memsz >= (int)(cvmx_sysinfo_get()->system_dram_size / 1000000)) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_set_memsz: Invalid memory size (%d GB), greater than available on the chip\n", memsz);
		return -1;
	}

	decode = __cvmx_l2c_vrt_decode_memsize(memsz);
	if (decode == -1) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_set_memsz: Invalid memory size (%d GB), refer to L2C_VRT_CTL[MEMSZ] for more information\n", memsz);
		return -1;
	}

	l2c_vrt_ctl.s.memsz = decode;
	cvmx_write_csr(CVMX_L2C_VRT_CTL, l2c_vrt_ctl.u64);
	return 0;
}

/**
 * Set a Virtual ID to a set of cores.
 *
 * @param virtid    Assign virtid to a set of cores.
 * @param coremask  The group of cores to assign a unique virtual id.
 * @return          Return 0 on success, otherwise -1.
 */
int cvmx_l2c_vrt_assign_virtid(int virtid, uint32_t coremask)
{
	uint32_t core = 0;
	int i;
	int found = 0;
	int max_virtid;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return -1;

	max_virtid = cvmx_l2c_vrt_get_max_virtids();

	if (virtid > max_virtid) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_assign_virt_id: Max %d "
			"number of virtids are allowed, passed %d.\n",
			max_virtid, virtid);
		return -1;
	}

	while (core < cvmx_octeon_num_cores()) {
		if ((coremask >> core) & 1) {
			cvmx_l2c_virtid_ppx_t l2c_virtid_ppx;
			l2c_virtid_ppx.u64 = cvmx_read_csr(CVMX_L2C_VIRTID_PPX(core));

			/* Check if the core already has a virtid assigned. */
			if (l2c_virtid_ppx.s.id) {
				cvmx_dprintf("WARNING: cvmx_l2c_vrt_assign_virt_id:"
					" Changing virtid of core #%d to %d from %d.\n",
					(unsigned int)core, virtid, l2c_virtid_ppx.s.id);

				/* Flush L2 cache to avoid write errors */
				cvmx_l2c_flush();
			}
			cvmx_write_csr(CVMX_L2C_VIRTID_PPX(core), virtid & 0x3f);
			found = 1;
		}
		core++;
	}

	/* Invalid coremask passed. */
	if (!found) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_assign_virt_id: Invalid "
			"coremask(0x%x) passed\n", (unsigned int)coremask);
		return -1;
	}

	/* Set the IOBs to normal mode. */
	for (i = 0; i < CVMX_L2C_IOBS; i++) {
		cvmx_l2c_virtid_iobx_t l2c_virtid_iobx;
		l2c_virtid_iobx.u64 = cvmx_read_csr(CVMX_L2C_VIRTID_IOBX(i));
		l2c_virtid_iobx.s.id = 1;
		l2c_virtid_iobx.s.dwbid = 0;
		cvmx_write_csr(CVMX_L2C_VIRTID_IOBX(i), l2c_virtid_iobx.u64);
	}
	return 0;
}

/**
 * Remove a virt id assigned to a set of cores. Update the virtid mask and
 * virtid stored for each core.
 *
 * @param virtid  Remove the specified Virtualization machine ID.
 */
void cvmx_l2c_vrt_remove_virtid(int virtid)
{
	uint32_t core;
	cvmx_l2c_virtid_ppx_t l2c_virtid_ppx;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return;

	for (core = 0; core < cvmx_octeon_num_cores(); core++) {
		l2c_virtid_ppx.u64 = cvmx_read_csr(CVMX_L2C_VIRTID_PPX(core));
		if (virtid == l2c_virtid_ppx.s.id)
			cvmx_write_csr(CVMX_L2C_VIRTID_PPX(core), 0);
	}
}

/**
 * Helper function to protect the memory region based on the granularity.
 */
uint64_t __cvmx_l2c_vrt_get_granularity(void)
{
	uint64_t gran = 0;

	if (OCTEON_IS_OCTEON2()) {
		int nvid;
		uint64_t szd;
		cvmx_l2c_vrt_ctl_t l2c_vrt_ctl;

		l2c_vrt_ctl.u64 = cvmx_read_csr(CVMX_L2C_VRT_CTL);
		nvid = cvmx_l2c_vrt_get_max_virtids();
		szd = (1ull << l2c_vrt_ctl.s.memsz) * 1024 * 1024 * 1024;
		gran = (unsigned long long)(szd * nvid) / (32ull * 1024);
	}
	return gran;
}

CVMX_SHARED cvmx_spinlock_t cvmx_l2c_vrt_spinlock;

/**
 * Block a memory region to be updated for a given virtual id.
 *
 * @param start_addr   Starting address of memory region
 * @param size         Size of the memory to protect
 * @param virtid       Virtual ID to use
 * @param mode         Allow/Disallow write access
 *                        = 0,  Allow write access by virtid
 *                        = 1,  Disallow write access by virtid
 */
int cvmx_l2c_vrt_memprotect(uint64_t start_addr, int size, int virtid, int mode)
{
	uint64_t gran;
	uint64_t end_addr;
	int byte_offset, virtid_offset;
	cvmx_l2c_vrt_ctl_t l2c_vrt_ctl;
	cvmx_l2c_vrt_memx_t l2c_vrt_mem;
	cvmx_l2c_virtid_ppx_t l2c_virtid_ppx;
	int found;
	uint32_t core;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return -1;
	/*
	 * Check the alignment of start address, should be aligned to the
	 * granularity.
	 */
	l2c_vrt_ctl.u64 = cvmx_read_csr(CVMX_L2C_VRT_CTL);

	/* No need to protect if virtualization is not enabled */
	if (!l2c_vrt_ctl.s.enable) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_memprotect: Virtualization is not enabled.\n");
		return -1;
	}

	if (virtid > cvmx_l2c_vrt_get_max_virtids()) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_memprotect: Virtualization id is greater than max allowed\n");
		return -1;
	}

	/* No need to protect if virtid is not assigned to a core */
	found = 0;
	for (core = 0; core < cvmx_octeon_num_cores(); core++) {
		l2c_virtid_ppx.u64 = cvmx_read_csr(CVMX_L2C_VIRTID_PPX(core));
		if (l2c_virtid_ppx.s.id == virtid) {
			found = 1;
			break;
		}
	}
	if (found == 0) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_memprotect: Virtualization id (%d) is not assigned to any core.\n", virtid);
		return -1;
	}

	/*
	 * Make sure previous stores are through before protecting the
	 * memory.
	 */
	CVMX_SYNCW;

	/*
	 * If the L2/DRAM physical address is >= 512 MB, subtract 256
	 * MB to get the address to use. This is because L2C removes
	 * the 256MB "hole" between DR0 and DR1.
	 */
	if ((start_addr < OCTEON_DDR0_SIZE) && ((start_addr + size) > OCTEON_DDR0_SIZE))
		goto fail;
	if ((start_addr >= OCTEON_DDR0_SIZE) && (start_addr < OCTEON_DDR1_BASE))
		goto fail;
	if ((start_addr + size) > OCTEON_MAX_PHY_MEM_SIZE)
		goto fail;

	if (start_addr > OCTEON_DDR1_BASE)
		start_addr = start_addr - OCTEON_DDR0_SIZE;

	end_addr = start_addr + size;
	gran = __cvmx_l2c_vrt_get_granularity();

	if (start_addr != ((start_addr + (gran - 1)) & ~(gran - 1))) {
		cvmx_dprintf("WARNING: cvmx_l2c_vrt_memprotect: Start address is not aligned\n");
		return -1;
	}

	/*
	 * Check the size of the memory to protect, should be aligned
	 * to the granularity.
	 */
	if (end_addr != ((end_addr + (gran - 1)) & ~(gran - 1))) {
		end_addr = (start_addr + (gran - 1)) & ~(gran - 1);
	}

	byte_offset = l2c_vrt_ctl.s.memsz + l2c_vrt_ctl.s.numid + 16;
	virtid_offset = 14 - l2c_vrt_ctl.s.numid;

	cvmx_spinlock_lock(&cvmx_l2c_vrt_spinlock);

	/* Enable memory protection for each virtid for the specified range. */
	while (start_addr < end_addr) {
		/*
		 * When L2C virtualization is enabled and a bit is set
		 * in L2C_VRT_MEM(0..1023), then L2C prevents the
		 * selected virtual machine from storing to the
		 * selected L2C/DRAM region.
		 */
		int offset, position, i;
		int l2c_vrt_mem_bit_index = start_addr >> byte_offset;
		l2c_vrt_mem_bit_index |= (virtid << virtid_offset);

		offset = l2c_vrt_mem_bit_index >> 5;
		position = l2c_vrt_mem_bit_index & 0x1f;

		l2c_vrt_mem.u64 = cvmx_read_csr(CVMX_L2C_VRT_MEMX(offset));
		/* Allow/Disallow write access to memory. */
		if (mode == 0)
			l2c_vrt_mem.s.data &= ~(1 << position);
		else
			l2c_vrt_mem.s.data |= 1 << position;
		l2c_vrt_mem.s.parity = 0;
		/* PARITY<i> is the even parity of DATA<i*8+7:i*8>, which means
		 * that each bit<i> in PARITY[0..3], is the XOR of all the bits
		 * in the corresponding byte in DATA.
		 */
		for (i = 0; i <= 4; i++) {
			uint64_t mask = 0xffull << (i * 8);
			if ((cvmx_pop(l2c_vrt_mem.s.data & mask) & 0x1))
				l2c_vrt_mem.s.parity |= (1ull << i);
		}
		cvmx_write_csr(CVMX_L2C_VRT_MEMX(offset), l2c_vrt_mem.u64);
		start_addr += gran;
	}

	cvmx_spinlock_unlock(&cvmx_l2c_vrt_spinlock);

	return 0;

fail:
	cvmx_dprintf("WARNING: cvmx_l2c_vrt_memprotect: Invalid start address(0x%llx), size(%d), cannot protect memory in hole.\n", (unsigned long long)start_addr, size);
	return -1;
}

/**
 * Enable virtualization.
 *
 * @param mode   Whether out of bound writes are an error.
 */
void cvmx_l2c_vrt_enable(int mode)
{
	cvmx_l2c_vrt_ctl_t l2c_vrt_ctl;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return;

	/* Enable global virtualization */
	l2c_vrt_ctl.u64 = cvmx_read_csr(CVMX_L2C_VRT_CTL);
	l2c_vrt_ctl.s.ooberr = mode;
	l2c_vrt_ctl.s.enable = 1;
	cvmx_write_csr(CVMX_L2C_VRT_CTL, l2c_vrt_ctl.u64);
}

/**
 * Disable virtualization.
 */
void cvmx_l2c_vrt_disable(void)
{
	cvmx_l2c_vrt_ctl_t l2c_vrt_ctl;

	if (OCTEON_IS_OCTEON1PLUS() || OCTEON_IS_OCTEON3())
		return;

	/* Disable global virtualization */
	l2c_vrt_ctl.u64 = cvmx_read_csr(CVMX_L2C_VRT_CTL);
	l2c_vrt_ctl.s.enable = 0;
	cvmx_write_csr(CVMX_L2C_VRT_CTL, l2c_vrt_ctl.u64);
}
#endif
