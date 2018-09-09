/***********************license start************************************
 * Copyright (c) 2004-2010 Cavium Inc. (support@cavium.com). All rights
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
 *
 *
 * For any questions regarding licensing please contact
 * marketing@cavium.com
 *
 **********************license end**************************************/
#include "octeon-boot-info.h"

/* Debugger related defines */
#define DEBUG_STACK_SIZE        1024	/* Size of the debugger stack */

/* Total number of (64 bit)registers stored. Must match debug_handler.S */
#define DEBUG_NUMREGS           512

#ifndef EXCEPTION_BASE_BASE
#define EXCEPTION_BASE_BASE     0	/* must be 4k aligned */
#endif
/* Increment size for exception base addresses (4k minimum) */
#ifndef EXCEPTION_BASE_INCR
#define EXCEPTION_BASE_INCR     (4*1024)
#endif
/* 16 4k blocks */

/* Each SE app has its own execption vectors and they start at this base
address.
  - The size for each application is 4K
  - Assuming a max of 31 SE apps the exception vectors could range from
    the base to base + 124K
*/
#define SE_APP_EXCEPTION_BASE 0x2000
#define SE_APP_EXCEPTION_SIZE 0x4000

#define BOOTLOADER_DEBUG_REG_SAVE_SIZE  (CVMX_MAX_CORES * DEBUG_NUMREGS * 8)
#define BOOTLOADER_DEBUG_STACK_SIZE  (CVMX_MAX_CORES * DEBUG_STACK_SIZE)
#define BOOTLOADER_PCI_READ_BUFFER_SIZE     (256)
#define BOOTLOADER_PCI_WRITE_BUFFER_SIZE     (256)

/* Bootloader debugger stub register save area follows exception vector space */
#define BOOTLOADER_DEBUG_REG_SAVE_BASE	\
	(EXCEPTION_BASE_BASE + OCTEON_EXCEPTION_VECTOR_BLOCK_SIZE)
/* debugger stack follows reg save area */
#define BOOTLOADER_DEBUG_STACK_BASE	\
	(BOOTLOADER_DEBUG_REG_SAVE_BASE + BOOTLOADER_DEBUG_REG_SAVE_SIZE)
/* pci communication blocks.  Read/write are from bootloader perspective*/
/* Original bootloader command block */
#define BOOTLOADER_PCI_READ_BUFFER_BASE	\
	(BOOTLOADER_DEBUG_STACK_BASE + BOOTLOADER_DEBUG_STACK_SIZE)

#define BOOTLOADER_BOOTMEM_DESC_ADDR	\
	(BOOTLOADER_PCI_READ_BUFFER_BASE + BOOTLOADER_PCI_READ_BUFFER_SIZE)

/* BOOTLOADER_BOOTMEM_DESC_ADDR size */
#define BOOTLOADER_BOOTMEM_DESC_SPACE	\
	(BOOTLOADER_BOOTMEM_DESC_ADDR + 0x8)

#define BOOTLOADER_END_RESERVED_SPACE	\
	(BOOTLOADER_BOOTMEM_DESC_SPACE + 128 /* space for actual descriptor */)
/* NOTE: The bootmem descriptor (allocated at BOOTLOADER_BOOTMEM_DESC_SPACE can
 * be moved without affecting users, as the address is looked upt from
 * BOOTLOADER_BOOTMEM_DESC_ADDR.  It is the BOOTLOADER_BOOTMEM_DESC_ADDR field
 * that can't be easily changed.
 */

/* Use the range EXCEPTION_BASE_BASE + 0x800 - 0x1000 (2k-4k) for bootloader
 * private data structures that need fixed addresses
 */
/* NOTE changes to these macros have to be changed in
 * executive/octeon-boot-info.h also.
 */

#define BOOTLOADER_PRIV_DATA_BASE        (EXCEPTION_BASE_BASE + 0x800)
#define BOOTLOADER_BOOT_VECTOR           (BOOTLOADER_PRIV_DATA_BASE)

/* WORD */
#define BOOTLOADER_DEBUG_TRAMPOLINE		\
	(BOOTLOADER_BOOT_VECTOR + BOOT_VECTOR_SIZE)

/* WORD * NUM_CORES */
#define BOOTLOADER_DEBUG_TRAMPOLINE_CORE	\
	(BOOTLOADER_DEBUG_TRAMPOLINE + 4)

/* WORD * NUM_CORES */
#define BOOTLOADER_DEBUG_FLAGS_BASE		\
	(BOOTLOADER_DEBUG_TRAMPOLINE_CORE + 4*CVMX_MAX_CORES)

#define BOOTLOADER_NEXT_AVAIL_ADDR		\
	(BOOTLOADER_DEBUG_FLAGS_BASE + 4*CVMX_MAX_CORES)

/* Debug flag bit definitions in cvmx-bootloader.h */

/* Address used for boot vectors for non-zero cores */
#ifndef BOOT_VECTOR_BASE
#define BOOT_VECTOR_BASE    (0x80000000 | BOOTLOADER_BOOT_VECTOR)
#endif

/* Defines for creating named block for idle cores to loop. */
#define IDLE_CORE_BLOCK_NAME	"idle-core-loop"
#define IDLE_CORE_BLOCK_SIZE	0x10

/* Reserved DRAM for exception vectors, bootinfo structures, bootloader, etc */
//#define OCTEON_RESERVED_LOW_MEM_SIZE    (1*1024*1024 - IDLE_CORE_BLOCK_SIZE)

/* Reserved DRAM for flat device tree */
#define OCTEON_FDT_BASE		(512 * 1024)
#define OCTEON_FDT_MAX_SIZE	(128 * 1024)
#define OCTEON_FDT_LE_BASE	(OCTEON_FDT_BASE + OCTEON_FDT_MAX_SIZE)
#define OCTEON_FDT_LE_MAX_SIZE  (OCTEON_FDT_MAX_SIZE)

/* Reserved DRAM for exception vectors, bootinfo structures, bootloader, etc */
/* Only 512MB of RAM is reserved initially so that other structures can use
 * named blocks within the first 1MB.  The bootloader is responsible for
 * reserving all memory in the first 1MB
 */
#define OCTEON_RESERVED_LOW_MEM_SIZE    (512*1024)

/* NOTE: U-Boot will always reserve the first 1MB of memory */
#define OCTEON_RESERVED_LOW_BOOT_MEM_SIZE       (1024*1024)

/* Reserved DRAM for idle code */
#define OCTEON_IDLE_CORE_MEM	(1*1024*1024 - IDLE_CORE_BLOCK_SIZE)
#define OCTEON_LINUX_RESERVED_MEM  (OCTEON_IDLE_CORE_MEM + IDLE_CORE_BLOCK_SIZE)

#define OCTEON_BOOTLOADER_NAMED_BLOCK_TMP_PREFIX "__tmp"

/* The environment variables used for controlling the reserved block allocations
 * are:
 *
 * octeon_reserved_mem_bl_base
 * octeon_reserved_mem_bl_size
 * octeon_reserved_mem_linux_base
 * octeon_reserved_mem_linux_size
 *
 * If the size is defined to be 0, then the reserved block is not created.  See
 * bootloader HTML documentation for more information.
 */

/* DRAM reserved for loading images.  This is freed just
 * before the application starts, but is used by the bootloader for
 * loading ELF images, etc */
#define OCTEON_BOOTLOADER_LOAD_MEM_NAME    "__tmp_load"

/* DRAM section reserved for the Linux kernel (we want this to be contiguous
 * with the bootloader reserved area to that they can be merged on free.
 */
#define OCTEON_LINUX_RESERVED_MEM_NAME    "__tmp_reserved_linux"

/* Named block name for block containing u-boot environment that is created
 * for use by application
 */
#define OCTEON_ENV_MEM_NAME    "__bootloader_env"

/* Named block name for block containing the flat device tree that is created
 * for use by applications and the Linux kernel.
 */
#define OCTEON_FDT_BLOCK_NAME	"__fdt"

/* Named blockname  for block containing the little-endian version of the
 * flat device tree that is created for use by applications and the Linux kernel
 */
#define OCTEON_FDT_LE_BLOCK_NAME	"__fdt_le"

