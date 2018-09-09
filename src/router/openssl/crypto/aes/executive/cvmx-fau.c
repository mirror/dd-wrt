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
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-fau.h>
#include <asm/octeon/cvmx-global-resources.h>
#include <asm/octeon/cvmx-bootmem.h>
#else
#include "cvmx.h"
#include "cvmx-fau.h"
#include "cvmx-global-resources.h"
#include "cvmx-bootmem.h"
#endif

CVMX_SHARED uint64_t cvmx_fau_register_type[CVMX_FAU_MAX_REGISTERS_8];

/** Allocates FAU register of specified bytes.
 *  @param reg_size register size in bytes.
 *  @return base address of allocated FAU register
 */
int cvmx_fau_alloc(int reg_size)
{
	int base;

	if(cvmx_create_global_resource_range(CVMX_GR_TAG_FAU, CVMX_FAU_MAX_REGISTERS_8)) {
		cvmx_dprintf("Failed to create FAU global resource\n");
		return -1;
	}
	base = cvmx_allocate_global_resource_range(CVMX_GR_TAG_FAU, 0, reg_size, reg_size);
	if(base == -1) {
		cvmx_dprintf("ERROR: Failed to alloc FAU register of %d bytes\n", (int)reg_size);
		return -1;
	}
	if(base >= CVMX_FAU_MAX_REGISTERS_8) {
		cvmx_dprintf("ERROR: Invalid FAU register %d got allocated\n",base);
		return -1;
	}
	cvmx_fau_register_type[base] = reg_size;
	return base;
}

/** Allocates 8bit FAU register.
 *  @return value is the base address of allocated FAU register
 */
int cvmx_fau8_alloc()
{
	return (cvmx_fau_alloc(1));
}

/** Allocates 16bit FAU register.
 *  @return value is the base address of allocated FAU register
 */
int cvmx_fau16_alloc()
{
	return (cvmx_fau_alloc(2));
}

/** Allocates 32bit FAU register.
 *  @return value is the base address of allocated FAU register
 */
int cvmx_fau32_alloc()
{
	return (cvmx_fau_alloc(4));
}

/** Allocates 64bit FAU register.
 *  @return value is the base address of allocated FAU register
 */
int cvmx_fau64_alloc()
{
	return (cvmx_fau_alloc(8));
}

/** Frees the specified FAU register.
 *  @param address Base address of register to release.
 *  @return 0 on success; -1 on failure
 */
int cvmx_fau_free(int address)
{
	if (address >= CVMX_FAU_MAX_REGISTERS_8) {
		cvmx_dprintf("ERROR: Invalid address %d in cvmx_fau_free\n", (int)address);
		return -1;
	}
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_FAU, address,
	    cvmx_fau_register_type[address] ) == -1) {
		cvmx_dprintf("ERROR Failed to release FPU register %d\n", (int)address);
		return -1;
	}
	return 0;
}

/** Display the fau registers array
 */
void cvmx_fau_show ()
{
	cvmx_show_global_resource_range(CVMX_GR_TAG_FAU);
}
