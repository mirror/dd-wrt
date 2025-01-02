/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-debug.h>

#define cvmx_interrupt_in_isr 0

#else
#include "cvmx.h"
#include "cvmx-debug.h"

#ifndef CVMX_BUILD_FOR_TOOLCHAIN
extern int cvmx_interrupt_in_isr;
#else
#define cvmx_interrupt_in_isr 0
#endif

#endif

static int cvmx_debug_remote_mem_wait_for_resume(volatile cvmx_debug_core_context_t * context)
{
	volatile int timeout = __SHRT_MAX__;
	int status;
	CVMX_SYNCW;
	while (context->remote_controlled == COMMAND_NOP && timeout >= 0)
		timeout--;
	CVMX_SYNCW;
	status = context->remote_controlled;
	CVMX_SYNCW;
	return status;
}

static void cvmx_debug_memory_change_core(int oldcore, int newcore)
{
	/* This should cause the host gdb to change the core but there is no way to signal to it, the core has changed. */
}

cvmx_debug_comm_t cvmx_debug_remote_comm = {
	.init = NULL,
	.install_break_handler = NULL,
	.needs_proxy = 0,
	.getpacket = NULL,
	.putpacket = NULL,
	.wait_for_resume = cvmx_debug_remote_mem_wait_for_resume,
	.change_core = cvmx_debug_memory_change_core,
};
