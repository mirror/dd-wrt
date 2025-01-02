/************************license start***************
 * Copyright (c) 2014 Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *     * Neither the name of Cavium Inc. nor the names of
 *       its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
 * REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
 * DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
 * PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
 * POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING OUT
 * OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

#ifndef __CVMX_BOOT_VECTOR_H__
#define __CVMX_BOOT_VECTOR_H__

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/octeon.h>
#else
#include "cvmx.h"
#endif

/*
 * The boot vector table is made up of an array of 1024 elements of
 * struct cvmx_boot_vector_element.  There is one entry for each
 * possible MIPS CPUNum, indexed by the CPUNum.
 *
 * Once cvmx_boot_vector_get() returns a non-NULL value (indicating
 * success), NMI to a core will cause execution to transfer to the
 * target_ptr location for that core's entry in the vector table.
 *
 * The struct cvmx_boot_vector_element fields app0, app1, and app2 can
 * be used by the application that has set the target_ptr in any
 * application specific manner, they are not touched by the vectoring
 * code.
 *
 * The boot vector code clobbers the CP0_DESAVE register, and on
 * OCTEON II and later CPUs also clobbers CP0_KScratch2.  All GP
 * registers are preserved, except on pre-OCTEON II CPUs, where k1 is
 * clobbered.
 *
 */


/*
 * Applications install the boot bus code in cvmx-boot-vector.c, which
 * uses this magic:
 */
#define OCTEON_BOOT_MOVEABLE_MAGIC1 0xdb00110ad358eacdull

struct cvmx_boot_vector_element {
	/* kseg0 or xkphys address of target code. */
	uint64_t target_ptr;
	/* Three application specific arguments. */
	uint64_t app0;
	uint64_t app1;
	uint64_t app2;
};

struct cvmx_boot_vector_element *cvmx_boot_vector_get(void);

#endif /* __CVMX_BOOT_VECTOR_H__ */
