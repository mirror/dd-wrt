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

 * This Software, including technical data, may be subject to U.S. export
 * control laws, including the U.S. Export Administration Act and its associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION
 * OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Module to support operations on bitmap of cores. Coremask can be used to
 * select a specific core, a group of cores, or all available cores, for
 * initialization and differentiation of roles within a single shared binary
 * executable image.
 *
 * <hr>$Revision: 83979 $<hr>
 *
 */

#if defined(__U_BOOT__) 
#include <linux/ctype.h>
#include <errno.h>
#include <asm/arch/cvmx.h>
#include <asm/arch/cvmx-asm.h>
#include <asm/arch/cvmx-spinlock.h>
#include <asm/arch/cvmx-coremask.h>
#elif defined(CVMX_BUILD_FOR_LINUX_KERNEL)
#include <linux/ctype.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-asm.h>
#include <asm/octeon/cvmx-spinlock.h>
#include <asm/octeon/cvmx-coremask.h>
#else
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include "cvmx.h"
#include "cvmx-asm.h"
#include "cvmx-spinlock.h"
#include "cvmx-coremask.h"
#endif

#define  CVMX_COREMASK_MAX_SYNCS  20	/* maximum number of coremasks for barrier sync */

/**
 * This structure defines the private state maintained by coremask module.
 *
 */
CVMX_SHARED static struct {

	cvmx_spinlock_t lock;			/**< mutex spinlock */

	struct {

		cvmx_coremask_t coremask;	/**< coremask specified for barrier */
		cvmx_coremask_t checkin;	/**< bitmask of cores checking in */
		volatile unsigned int exit;	/**< variable to poll for exit condition */

	} s[CVMX_COREMASK_MAX_SYNCS];

} state = {
	{CVMX_SPINLOCK_UNLOCKED_VAL}, { {{{0,},}, {{0,},}, 0}, },
};

/**
 * Wait (stall) until all cores in the given coremask has reached this point
 * in the program execution before proceeding.
 *
 * @param  coremask  the group of cores performing the barrier sync
 *
 */
void cvmx_coremask_barrier_sync(const cvmx_coremask_t *pcm)
{
	int i;
	unsigned int target;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	assert(pcm != NULL && !((long)pcm & 3));
#endif
	cvmx_spinlock_lock(&state.lock);

	for (i = 0; i < CVMX_COREMASK_MAX_SYNCS; i++) {

		if (cvmx_coremask_is_empty(&state.s[i].coremask)) {
			/* end of existing coremask list, create new entry, fall-thru */
			cvmx_coremask_copy(&state.s[i].coremask, pcm);
		}

		if (cvmx_coremask_cmp(&state.s[i].coremask, pcm) == 0) {

			target = state.s[i].exit + 1;	/* wrap-around at 32b */

			cvmx_coremask_set_self(&state.s[i].checkin);

			if (cvmx_coremask_cmp(&state.s[i].checkin, pcm) == 0) {
				cvmx_coremask_clear_all(&state.s[i].checkin);
				state.s[i].exit = target;	/* signal exit condition */
			}
			cvmx_spinlock_unlock(&state.lock);

			while (state.s[i].exit != target) ;

			return;
		}
	}

	/* error condition - coremask array overflowed */
	cvmx_spinlock_unlock(&state.lock);
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	assert(0);
#endif
}

int cvmx_coremask_str2bmp(cvmx_coremask_t *pcm, char *hexstr)
{
	int i, j;
	int l;		/* length of the hexstr in characters */
	int lb;		/* number of bits taken by hexstr */
	int hldr_offset;/* holder's offset within the coremask */
	int hldr_xsz;	/* holder's size in the number of hex digits */
	cvmx_coremask_holder_t h;
	char c;

#define MINUS_ONE (hexstr[0] == '-' && hexstr[1] == '1' && hexstr[2] == 0)
	if (MINUS_ONE) {
		cvmx_coremask_set_all(pcm);
		return 0;
	}

	/* Skip '0x' from hexstr */
	if (hexstr[0] == '0' && (hexstr[1] == 'x' || hexstr[1] == 'X'))
		hexstr += 2;

	if (hexstr[0] == '0') {
		cvmx_dprintf("%s: hexstr starts with zero\n", __func__);
		return -3;
	}

	l = strlen(hexstr);
	for (i = 0; i < l; i++) {
		if (isxdigit((int)hexstr[i]) == 0) {
			cvmx_dprintf("%s: Non-hex digit within hexstr\n",
			    __func__);
			return -2;
		}
	}

	lb = (l - 1) * 4;
	if (hexstr[0] > '7')
		lb += 4;
	else if (hexstr[0] > '3')
		lb += 3;
	else if (hexstr[0] > '1')
		lb += 2;
	else
		lb += 1;
	if (lb > CVMX_MIPS_MAX_CORES) {
		cvmx_dprintf("%s: hexstr (%s) is too long\n", __func__,
		    hexstr);
		return -1;
	}
	cvmx_coremask_clear_all(pcm);
	hldr_offset = 0;
	hldr_xsz = 2 * sizeof(cvmx_coremask_holder_t);
	for (i = l; i > 0; i -= hldr_xsz) {
		c = hexstr[i];
		hexstr[i] = 0;
		j = i - hldr_xsz;
		if (j < 0)
			j = 0;
		h = strtoull(&hexstr[j], NULL, 16);
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
		if (errno == EINVAL) {
			cvmx_dprintf("%s: strtou returns w/ EINVAL\n",
			    __func__);
			return -2;
		}
#endif
		pcm->coremask_bitmap[hldr_offset] = h;
		hexstr[i] = c;
		hldr_offset++;
	}

	return 0;
}

int cvmx_coremask_bmp2str(const cvmx_coremask_t *pcm, char *hexstr)
{
	int i, n;
	char *p;
	char *fmt1, *fmt2;

	if (sizeof(cvmx_coremask_holder_t) <= sizeof(uint32_t)) {
		fmt1 = "%x";
		fmt2 = "%08x";
	} else {
		fmt1 = "%llx";
		fmt2 = "%016llx";
	}

	n = 0;
	p = hexstr;
	for (i = CVMX_COREMASK_BMPSZ - 1; i >= 0; i--)
		if (pcm->coremask_bitmap[i])
			break;
	if (i < 0) {
		sprintf(&p[n], "0");
		return 0;
	}

	if (pcm->coremask_bitmap[i])
		n = sprintf(&p[n], fmt1, pcm->coremask_bitmap[i]);

	for (i--; i >= 0; i--)
		n += sprintf(&p[n], fmt2, pcm->coremask_bitmap[i]);

	return 0;
}

void cvmx_coremask_print(const cvmx_coremask_t *pcm)
{
	int i, j;
	int start;
	int found = 0;
	/* Print one node per line.  Since the bitmap is stored LSB to MSB
	 * we reverse the order when printing.
	 */
	if (!octeon_has_feature(OCTEON_FEATURE_MULTINODE)) {
		start = 0;
		for (j = CVMX_COREMASK_MAX_CORES_PER_NODE - CVMX_COREMASK_HLDRSZ;
		     j >= 0; j -= CVMX_COREMASK_HLDRSZ) {
			if (pcm->coremask_bitmap[j / CVMX_COREMASK_HLDRSZ] != 0)
				start = 1;
			if (start)
				cvmx_dprintf(" 0x%llx",
				       (unsigned long long)pcm->coremask_bitmap[j / CVMX_COREMASK_HLDRSZ]);
		}
		if (start)
			found = 1;
		/* If the coremask is empty print <EMPTY> so it is not confusing.  */
		if (!found)
			cvmx_dprintf("<EMPTY>");
		cvmx_dprintf("\n");
		return;
	}

	for (i = 0; i < CVMX_MAX_USED_CORES_BMP;
	     i += CVMX_COREMASK_MAX_CORES_PER_NODE) {
		cvmx_dprintf("%s  node %d:", i > 0 ? "\n" : "",
		       cvmx_coremask_core_to_node(i));
		start = 0;
		for (j = i + CVMX_COREMASK_MAX_CORES_PER_NODE
			 - CVMX_COREMASK_HLDRSZ;
		     j >= i; j -= CVMX_COREMASK_HLDRSZ) {
			/* Don't start printing until we get a non-zero word. */
			if (pcm->coremask_bitmap[j / CVMX_COREMASK_HLDRSZ] != 0)
				start = 1;

			if (start)
				cvmx_dprintf(" 0x%llx",
				       (unsigned long long)pcm->coremask_bitmap[j / CVMX_COREMASK_HLDRSZ]);
		}
		if (start)
			found = 1;
	}
	/* If the coremask is empty print <EMPTY> so it is not confusing.  */
	if (!found)
		cvmx_dprintf("<EMPTY>");
	cvmx_dprintf("\n");
}
