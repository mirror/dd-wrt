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

/**
 * @file
 *
 * Coredump interface and definitions
 *
 * <hr>$Revision$<hr>
 */
#ifndef __CVMX_COREDUMP_H__
#define __CVMX_COREDUMP_H__

#include "cvmx-debug.h"

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#define CVMX_COREDUMP_BLK_NAME	"cvmx-coredump"

/* Use the same interface for the coredump as the debugger stub uses. */
typedef cvmx_debug_core_context_t cvmx_coredump_registers_t;
typedef cvmx_debug_tlb_t cvmx_coredump_tlb_t;

#define CVMX_COREDUMP_MAX_MEM_DESC    256

#define CVMX_CORE_DUMP_MAGIC_PART(A,B,C,D,E,F,G,H) \
	((((uint64_t)A & 0xff) << 56) | (((uint64_t)B & 0xff) << 48) | (((uint64_t)C & 0xff) << 40)  | (((uint64_t)D & 0xff) << 32) | \
	 (((uint64_t)E & 0xff) << 24) | (((uint64_t)F & 0xff) << 16) | (((uint64_t)G & 0xff) << 8)   | (((uint64_t)H & 0xff)))
#define CVMX_COREDUMP_MAGIC CVMX_CORE_DUMP_MAGIC_PART('C','O','R','E','D','U','M','P')
#define CVMX_COREDUMP_CURRENT_VERSION 2

typedef struct {
	uint64_t valid;
	uint64_t base;
	uint64_t size;
} cvmx_coredump_memdesc_t;

typedef struct {
	uint64_t num_mem_entry;
	cvmx_coredump_memdesc_t meminfo[CVMX_COREDUMP_MAX_MEM_DESC];
} cvmx_coredump_mem_info_t;

typedef struct
{
	uint64_t magic;
	uint64_t version;
	uint64_t tlb_entries;
	cvmx_coredump_mem_info_t mem_dump;
	cvmx_coredump_registers_t registers[CVMX_MAX_CORES];
} cvmx_coredump_mem_block_t;

void cvmx_coredump_init(void);
int cvmx_coredump_start_dump(uint64_t *u);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif	/* __CVMX_COREDUMP_H__ */
