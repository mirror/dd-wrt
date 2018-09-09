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
 * Main Coredump Interface
 *
 * <hr>$Revision$<hr>
 */
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-coremask.h"
#include "cvmx-coredump-bootmem.h"
#include "cvmx-coredump.h"
#include "cvmx-interrupt.h"

#ifdef CVMX_COREDEUMP_DEBUG
	#define CVMX_COREDUMP_DEBUG_PRINT cvmx_safe_printf
#else
	#define CVMX_COREDUMP_DEBUG_PRINT(...) (void)(0)
#endif

static cvmx_coredump_mem_block_t *dump_blk;

static void
cvmx_coredump_init_global_data(void *ptr)
{
	uint64_t phys = cvmx_ptr_to_phys(ptr);
	cvmx_coredump_mem_block_t *p;
	/*
	 * Since at this point, TLBs are not mapped 1 to 1,
	 * we should just use KSEG0/XKPHYS accesses.
	 */
#if defined(CVMX_ABI_N32)
	p = CASTPTR(cvmx_coredump_mem_block_t, CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0, phys));
#else
	p = CASTPTR(cvmx_coredump_mem_block_t, CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, phys));
#endif
	memset(p, 0, sizeof(cvmx_coredump_mem_block_t));

	/* Initialize magic and version */
	p->magic = CVMX_COREDUMP_MAGIC;
	p->version = CVMX_COREDUMP_CURRENT_VERSION;
	p->tlb_entries = cvmx_core_get_tlb_entries();
}

/*
 * Initialize the memory we need for dumping ELF headers
 */
void
cvmx_coredump_init(void)
{
	void *ptr;
	uint64_t phys;

	ptr = cvmx_bootmem_alloc_named_range_once(
			sizeof(cvmx_coredump_mem_block_t),
			 0, 0, 8,
			CVMX_COREDUMP_BLK_NAME, cvmx_coredump_init_global_data);

	phys = cvmx_ptr_to_phys(ptr);

	/* Since TLBs are not always mapped 1 to 1, we should just use access via KSEG0 for n32
	   and XKPHYS for 64bit. */
#if defined(CVMX_ABI_N32)
	dump_blk = CASTPTR(cvmx_coredump_mem_block_t,
			   CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0, phys));
#else
	dump_blk = CASTPTR(cvmx_coredump_mem_block_t,
			   CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, phys));
#endif
	CVMX_COREDUMP_DEBUG_PRINT("coredump: block is at %p\n", dump_blk);
	/* If the named block is the wrong version, just say we did not initilize core dump. */
	if (dump_blk->magic != CVMX_COREDUMP_MAGIC || dump_blk->version != CVMX_COREDUMP_CURRENT_VERSION)
	{
		dump_blk = NULL;
		return;
	}
	/* Set the remote_controlled bit to 0 as we might have done a hot plugin of this core from a crashed
	   core. */
	dump_blk->registers[cvmx_get_core_num()].remote_controlled = 0;
}

#define CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_STATUS            (0xFFFFFFFFFF301000ull)
#define CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_ADDRESS(num)      (0xFFFFFFFFFF301100ull + 0x100 * (num))
#define CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_ADDRESS_MASK(num) (0xFFFFFFFFFF301108ull + 0x100 * (num))
#define CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_ASID(num)         (0xFFFFFFFFFF301110ull + 0x100 * (num))
#define CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_CONTROL(num)      (0xFFFFFFFFFF301118ull + 0x100 * (num))

#define CVMX_DEBUG_HW_DATA_BREAKPOINT_STATUS                   (0xFFFFFFFFFF302000ull)
#define CVMX_DEBUG_HW_DATA_BREAKPOINT_ADDRESS(num)             (0xFFFFFFFFFF302100ull + 0x100 * (num))
#define CVMX_DEBUG_HW_DATA_BREAKPOINT_ADDRESS_MASK(num)        (0xFFFFFFFFFF302108ull + 0x100 * (num))
#define CVMX_DEBUG_HW_DATA_BREAKPOINT_ASID(num)                (0xFFFFFFFFFF302110ull + 0x100 * (num))
#define CVMX_DEBUG_HW_DATA_BREAKPOINT_CONTROL(num)             (0xFFFFFFFFFF302118ull + 0x100 * (num))

static void cvmx_coredump_save_core_context(cvmx_coredump_registers_t *context, uint64_t *registers)
{
	unsigned i;
	memcpy(context->regs, registers, 32*sizeof(uint64_t));

	context->lo = registers[32];
	context->hi = registers[33];

	CVMX_MF_COP0(context->cop0.index, COP0_INDEX);
	CVMX_MF_COP0(context->cop0.entrylo[0], COP0_ENTRYLO0);
	CVMX_MF_COP0(context->cop0.entrylo[1], COP0_ENTRYLO1);
	CVMX_MF_COP0(context->cop0.entryhi, COP0_ENTRYHI);
	CVMX_MF_COP0(context->cop0.pagemask, COP0_PAGEMASK);
	CVMX_MF_COP0(context->cop0.status, COP0_STATUS);
	CVMX_MF_COP0(context->cop0.cause, COP0_CAUSE);
	CVMX_MF_COP0(context->cop0.debug, COP0_DEBUG);
	CVMX_MF_COP0(context->cop0.multicoredebug, COP0_MULTICOREDEBUG);
	CVMX_MF_COP0(context->cop0.perfval[0], COP0_PERFVALUE0);
	CVMX_MF_COP0(context->cop0.perfval[1], COP0_PERFVALUE1);
	CVMX_MF_COP0(context->cop0.perfctrl[0], COP0_PERFCONTROL0);
	CVMX_MF_COP0(context->cop0.perfctrl[1], COP0_PERFCONTROL1);
	context->cop0.depc = registers[35];
	CVMX_MF_COP0(context->cop0.desave, COP0_DESAVE);

#if 0
	/* FIXME: Cannot read the hardware breakpoints in non EJAT debug mode. */
	context->hw_ibp.status = cvmx_read_csr(CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_STATUS);
	for (i = 0; i < 4; i++) {
		context->hw_ibp.address[i] = cvmx_read_csr(CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_ADDRESS(i));
		context->hw_ibp.address_mask[i] = cvmx_read_csr(CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_ADDRESS_MASK(i));
		context->hw_ibp.asid[i] = cvmx_read_csr(CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_ASID(i));
		context->hw_ibp.control[i] = cvmx_read_csr(CVMX_DEBUG_HW_INSTRUCTION_BREAKPOINT_CONTROL(i));
	}

	context->hw_dbp.status = cvmx_read_csr(CVMX_DEBUG_HW_DATA_BREAKPOINT_STATUS);
	for (i = 0; i < 4; i++) {
		context->hw_dbp.address[i] = cvmx_read_csr(CVMX_DEBUG_HW_DATA_BREAKPOINT_ADDRESS(i));
		context->hw_dbp.address_mask[i] = cvmx_read_csr(CVMX_DEBUG_HW_DATA_BREAKPOINT_ADDRESS_MASK(i));
		context->hw_dbp.asid[i] = cvmx_read_csr(CVMX_DEBUG_HW_DATA_BREAKPOINT_ASID(i));
		context->hw_dbp.control[i] = cvmx_read_csr(CVMX_DEBUG_HW_DATA_BREAKPOINT_CONTROL(i));
	}
#endif

	for (i = 0; i < dump_blk->tlb_entries; i++) {
		CVMX_MT_COP0(i, COP0_INDEX);
		asm volatile ("tlbr");
		CVMX_MF_COP0(context->tlbs[i].entrylo[0], COP0_ENTRYLO0);
		CVMX_MF_COP0(context->tlbs[i].entrylo[1], COP0_ENTRYLO1);
		CVMX_MF_COP0(context->tlbs[i].entryhi, COP0_ENTRYHI);
		CVMX_MF_COP0(context->tlbs[i].pagemask, COP0_PAGEMASK);
	}

	/* Mark the core as having a coredump. */
	context->remote_controlled = 1;
	CVMX_SYNCW;
}

/* 
 * Fill in the register and TLB information in the coredump structure.
 */
int
cvmx_coredump_start_dump(uint64_t *registers)
{
	unsigned int corenum = cvmx_get_core_num();

	/* If the coredump is not intialized then report a failure. */
	if (dump_blk == 0)
		return 1;

	cvmx_coredump_save_core_context(&dump_blk->registers[corenum], registers);
	cvmx_safe_printf("Core file is ready to dump\n");

	return 0;
}

/* The functions which are (will) be called from cvmx-bootmem.c */

void
cvmx_coredump_add_mem_range(uint64_t addr, uint64_t size)
{
	int i;
	cvmx_coredump_mem_info_t *coredump_info;

	/* Early exit if coredump is not initalized. */
	if (dump_blk == NULL)
		return;

	coredump_info = &dump_blk->mem_dump;

	for (i = 0; i < CVMX_COREDUMP_MAX_MEM_DESC; i++) {
		if (!coredump_info->meminfo[i].valid) {
			break;
		}
	}
	if (i == CVMX_COREDUMP_MAX_MEM_DESC) {
		printf("%s:Max number of memory range exceeded\n", __FUNCTION__);
		return;
	}
	coredump_info->meminfo[i].valid = 1;
	coredump_info->meminfo[i].base = addr;
	coredump_info->meminfo[i].size = size;

	//DEBUG_PRINT("Added entry:%lx size:%lx\n", addr, size);
	coredump_info->num_mem_entry++;
}

void
cvmx_coredump_delete_mem(uint64_t addr)
{
	int i;
	cvmx_coredump_mem_info_t *coredump_info;

	/* Early exit if coredump is not initalized. */
	if (dump_blk == NULL)
		return;

	coredump_info = &dump_blk->mem_dump;

	for (i = 0; i < CVMX_COREDUMP_MAX_MEM_DESC; i++) {
		if (coredump_info->meminfo[i].base == addr) {
			break;
		}
	}
	if (i == CVMX_COREDUMP_MAX_MEM_DESC) {
		printf("%s: Memory range not found\n", __FUNCTION__);
		return;
	}
	
	memset(&coredump_info->meminfo[i], 0, sizeof(coredump_info->meminfo[i]));
	coredump_info->num_mem_entry-- ;
}
