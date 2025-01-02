/***********************license start***************
 * Copyright (c) 2003-2012  Cavium Inc. (support@cavium.com). All rights
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

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx-range.h>
#else
#include "cvmx-range.h"
#endif

#define CVMX_RANGE_AVAILABLE ((uint64_t) -88)
#define addr_of_element(base, index) (1ull << 63 | (base + sizeof(uint64_t) + (index) * sizeof(uint64_t)))
#define addr_of_size(base) (1ull << 63 | base)

static const int debug = 0;

int cvmx_range_memory_size(int nelements)
{
	return sizeof(uint64_t) * (nelements + 1);
}

int cvmx_range_init(uint64_t range_addr, int size)
{
	uint64_t i;
	uint64_t lsize = size;

	cvmx_write64_uint64(addr_of_size(range_addr),lsize);
	for (i = 0; i < lsize; i++) {
		cvmx_write64_uint64(addr_of_element(range_addr,i), CVMX_RANGE_AVAILABLE);
	}
	return 0;
}


static int64_t cvmx_range_find_next_available(uint64_t range_addr, uint64_t index, int align)
{
	uint64_t i;
	uint64_t size = cvmx_read64_uint64(addr_of_size(range_addr));

	while ((index % align) != 0)
		index++;

	for (i = index; i < size; i += align) {
		uint64_t r_owner = cvmx_read64_uint64(addr_of_element(range_addr,i));
		if (debug)
			cvmx_dprintf("%s: index=%d owner=%llx\n",
				__func__, (int) i, 
				(unsigned long long) r_owner);
		if (r_owner == CVMX_RANGE_AVAILABLE)
			return i;
	}
	return -1;
}

static int64_t cvmx_range_find_last_available(uint64_t range_addr, uint64_t index, uint64_t align)
{
	uint64_t i;
	uint64_t size = cvmx_read64_uint64(addr_of_size(range_addr));

	if (index == 0)
		index = size - 1;

	while ((index % align) != 0)
		index++;

	for (i = index; i > align; i -= align) {
		uint64_t r_owner = cvmx_read64_uint64(
			addr_of_element(range_addr, i));
		if (debug)
			cvmx_dprintf("%s: index=%d owner=%llx\n",
				__func__, (int) i,
				(unsigned long long) r_owner);
		if (r_owner == CVMX_RANGE_AVAILABLE)
			return i;
	}
	return -1;
}

int cvmx_range_alloc_ordered(uint64_t range_addr, uint64_t owner, uint64_t cnt, int align, int reverse)
{
	uint64_t i=0, size;
	int64_t first_available;

	if (debug)
		cvmx_dprintf("%s: range_addr=%llx  owner=%llx cnt=%d \n",
			__func__,
			(unsigned long long) range_addr,
			(unsigned long long) owner,
			(int)cnt);

	size = cvmx_read64_uint64(addr_of_size(range_addr));
	while (i < size) {
		uint64_t available_cnt=0;
		if (reverse) first_available = cvmx_range_find_last_available(range_addr, i, align);
		else first_available = cvmx_range_find_next_available(range_addr, i, align);
		if (first_available == -1)
			return -1;
		i = first_available;

		if (debug)
			cvmx_dprintf("%s: first_available=%d \n", __func__,
				     (int) first_available);
		while ((available_cnt != cnt) && (i < size)) {
			uint64_t r_owner = cvmx_read64_uint64(addr_of_element(range_addr,i));
			if (r_owner == CVMX_RANGE_AVAILABLE)
				available_cnt++;
			i++;
		}
		if (available_cnt == cnt) {
			uint64_t j;

			if (debug)
				cvmx_dprintf("%s: first_available=%d available=%d\n",
					     __func__, (int) first_available,
					     (int) available_cnt);

			for (j = first_available; j < first_available + cnt; j++) {
				uint64_t a = addr_of_element(range_addr,j);
				cvmx_write64_uint64(a, owner);
			}
			return first_available;
		}
	}

	if (debug) {
		cvmx_dprintf("ERROR: %s: failed to allocate range cnt=%d\n",
			__func__, (int)cnt);
		cvmx_range_show(range_addr);
	}

	return -1;
}

int cvmx_range_alloc(uint64_t range_addr, uint64_t owner, uint64_t cnt, int align)
{
	return cvmx_range_alloc_ordered(range_addr, owner, cnt, align, 0);
}

int  cvmx_range_alloc_non_contiguos(uint64_t range_addr, uint64_t owner, uint64_t cnt,
				    int elements[])
{
	uint64_t i=0, size;
	uint64_t element_index = 0;

	size = cvmx_read64_uint64(addr_of_size(range_addr));
	for (i = 0; i < size; i++) {
		uint64_t r_owner = cvmx_read64_uint64(addr_of_element(range_addr,i));
		if (debug)
			cvmx_dprintf("%s: index=%d owner=%llx\n",
				__func__, (int) i,
				(unsigned long long) r_owner);
		if (r_owner == CVMX_RANGE_AVAILABLE)
			elements[element_index++] = (int) i;

		if (element_index == cnt)
			break;
	}
	if (element_index != cnt) {
		if (debug)
			cvmx_dprintf("%s: failed to allocate non contiguous cnt=%d"
				     " available=%d\n",
				     __func__, (int)cnt, (int) element_index);
		return -1;
	}
	for (i = 0; i < cnt; i++) {
		uint64_t a = addr_of_element(range_addr,elements[i]);
		cvmx_write64_uint64(a, owner);
	}
	return 0;

}

int cvmx_range_reserve(uint64_t range_addr, uint64_t owner, uint64_t base, uint64_t cnt )
{
	uint64_t i, size, r_owner;
	uint64_t up = base + cnt;

	size = cvmx_read64_uint64(addr_of_size(range_addr));
	if (up > size) {
		cvmx_dprintf("ERROR: %s: invalid base or cnt. "
			    "range_addr=0x%llx, owner=0x%llx, size=%d base+cnt=%d\n",
			     __func__,
			     (unsigned long long)range_addr,
			     (unsigned long long)owner,
			     (int)size, (int)up);
		return -1;
	}
	for (i = base; i < up; i++) {
		r_owner = cvmx_read64_uint64(addr_of_element(range_addr,i));
		if (debug)
			cvmx_dprintf("%s: %d: %llx\n",
				__func__, (int) i,
				(unsigned long long) r_owner);
		if (r_owner != CVMX_RANGE_AVAILABLE) {
		    if (debug){
			cvmx_dprintf("%s: resource already reserved base+cnt=%d %llu %llu %llx %llx %llx\n",
				     __func__,
				     (int)i, (unsigned long long)cnt, (unsigned long long)base,
				     (unsigned long long)r_owner, (unsigned long long)range_addr,
				     (unsigned long long)owner);
		    }
			return -1;
		}
	}
	for (i = base; i < up; i++) {
		cvmx_write64_uint64(addr_of_element(range_addr,i), owner);
	}
	return base;
}

int cvmx_range_free_with_owner(uint64_t range_addr, uint64_t owner)
{
	uint64_t i, size;
	int found = -1;

	size = cvmx_read64_uint64(addr_of_size(range_addr));
	for (i = 0; i < size; i++) {
		uint64_t r_owner = cvmx_read64_uint64(addr_of_element(range_addr,i));
		if (r_owner == owner) {
			cvmx_write64_uint64(addr_of_element(range_addr,i), CVMX_RANGE_AVAILABLE);
			found = 0;
		}
	}
	return found;
}

int __cvmx_range_is_allocated(uint64_t range_addr, int bases[], int count)
{
	uint64_t i, cnt, size;
	uint64_t r_owner;

	cnt = count;
	size = cvmx_read64_uint64(addr_of_size(range_addr));
	for (i = 0; i < cnt; i++) {
		uint64_t base = bases[i];
		if (base >= size) {
			cvmx_dprintf("ERROR: %s: invalid base or cnt size=%d "
				     "base=%d \n",
				     __func__, (int) size, (int)base);
			return 0;
		}
		r_owner = cvmx_read64_uint64(addr_of_element(range_addr,base));
		if (r_owner == CVMX_RANGE_AVAILABLE) {
		    if (debug) {
			cvmx_dprintf("%s: i=%d:base=%d is available\n",
				     __func__, (int) i, (int) base);
		    }
			return 0;
		}
	}
	return 1;
}

int cvmx_range_free_mutiple(uint64_t range_addr, int bases[], int count)
{
	uint64_t i, cnt;

	cnt = count;
	if (__cvmx_range_is_allocated(range_addr, bases, count) != 1) {
		return -1;
	}
	for (i = 0; i < cnt; i++) {
		uint64_t base = bases[i];
		cvmx_write64_uint64(addr_of_element(range_addr, base),
				    CVMX_RANGE_AVAILABLE);
	}
	return 0;
}

int cvmx_range_free_with_base(uint64_t range_addr, int base, int cnt)
{
	uint64_t i, size;
	uint64_t up = base + cnt;

	size = cvmx_read64_uint64(addr_of_size(range_addr));
	if (up > size) {
		cvmx_dprintf("ERROR: %s: invalid base or cnt size=%d"
			" base+cnt=%d\n",
			__func__, (int) size, (int)up);
		return -1;
	}
	for (i = base; i < up; i++) {
		cvmx_write64_uint64(addr_of_element(range_addr,i), CVMX_RANGE_AVAILABLE);
	}
	return 0;
}

uint64_t cvmx_range_get_owner(uint64_t range_addr, uint64_t base)
{
	uint64_t size = cvmx_read64_uint64(addr_of_size(range_addr));

	if (base >= size) {
		cvmx_dprintf("ERROR: %s: invalid base or cnt size=%d base=%d\n",
			__func__, (int) size, (int)base);
		return 0;
	}
	return cvmx_read64_uint64(addr_of_element(range_addr, base));
}

void cvmx_range_show(uint64_t range_addr)
{
	uint64_t pval, val, size, pindex, i;

	size = cvmx_read64_uint64(addr_of_size(range_addr));
	pval = cvmx_read64_uint64(addr_of_element(range_addr, 0));
	pindex = 0;

	cvmx_dprintf ("index=%d: owner %llx\n",
				(int) pindex, CAST_ULL(pval));

	for (i = 1; i < size; i++) {
		val = cvmx_read64_uint64(addr_of_element(range_addr,i));
		if (val != pval) {
			cvmx_dprintf ("index=%d: owner %llx\n",
				(int) pindex, CAST_ULL(pval));
			pindex = i;
			pval = val;
		}
	}
	cvmx_dprintf("index=%d: owner %llx\n", (int) pindex, CAST_ULL(pval));
}
