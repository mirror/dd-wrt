/***********************license start***************
 * Copyright (c) 2003-2011  Cavium Inc. (support@cavium.com). All rights
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
 * Interface to Octeon boot structure
 *
 * <hr>$Revision:  $<hr>
 */

#ifndef __OCTEON_BOOT_INFO_H__
#define __OCTEON_BOOT_INFO_H__

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/types.h>
#include <asm/octeon/cvmx-asm.h>
#elif defined(__U_BOOT__)
# include <asm/arch/cvmx-asm.h>
#else
# include "cvmx-asm.h"
#endif

/* Currently only used by u-boot.  Applications use the value in
 * cvmx-boot-vector.h */
#define OCTEON_BOOT_MOVEABLE_MAGIC 0xdb00110ad358eace

/** Offset of magic number in the boot bus moveable region */
#define OCTEON_BOOT_MOVEABLE_MAGIC_OFFSET	0x70

/** Offset within the boot bus moveable region for the boot vector table */
#define OCTEON_BOOT_VECTOR_MOVEABLE_OFFSET	0x78

/** Each per-node boot vector table also has a name */
#define	OCTEON_BOOT_VECTOR_LOCATOR_NAME	"__cvmx_boot_vector_"

#ifndef __ASSEMBLY__

/*
 * This structure is access by bootloader, Linux kernel and the Linux
 * user space utility "bootoct".

 * In the bootloader, this structure is accessed by assembly code in start.S,
 * so any changes to content or size must be reflected there as well.

 * This is placed at a fixed address in DRAM, so that cores can access it
 * when they come out of reset.  It is used to setup the minimal bootloader
 * runtime environment (stack, but no heap, global data ptr) that is needed
 * by the non-boot cores to setup the environment for the applications.
 * The boot_info_addr is the address of a boot_info_block_t structure
 * which contains more core-specific information.
 *
 * The Linux kernel and the Linux bootoct utility access this structure for
 * implementing CPU hotplug functionality and booting of idle cores with SE
 * apps respectively.
 *
 */
struct  boot_init_vector {
	/* First stage address - in ram instead of flash */
	uint64_t code_addr;
#ifdef __BIG_ENDIAN_BITFIELD
	/* Setup code for application, NOT application entry point */
	uint32_t app_start_func_addr;
	/* k0 is used for global data - needs to be passed to other cores */
	uint32_t k0_val;
#else
	uint32_t k0_val;
	uint32_t app_start_func_addr;
#endif	/* LITTLE_ENDIAN */
	/* Address of boot info block structure */
	uint64_t boot_info_addr;
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t flags;		/* flags */
	uint32_t pad;
#else
	uint32_t pad;
	uint32_t flags;		/* flags */
#endif	/* LITTLE_ENDIAN */
};
typedef struct  boot_init_vector boot_init_vector_t;

#if defined(__ASM_GBL_DATA_H)	/* defined above */
/*
 * Definition of a data structure to mimic the old u-boot gd_t data structure.
 */
#undef GD_TMP_STR_SIZE
#define GD_TMP_STR_SIZE 32

/* This structure is deprecated, use sysinfo instead */
struct linux_app_global_data {
#if	_MIPS_SZPTR == 32
	uint32_t pad0;
#endif
	bd_t *bd;
	uint32_t flags;
	uint32_t baudrate;
	uint32_t have_console;	/* serial_init() was called */
	uint64_t ram_size;	/* RAM size */
	uint64_t reloc_off;	/* Relocation Offset */
	uint32_t env_addr;	/* Address  of Environment struct */
	uint32_t env_valid;	/* Checksum of Environment valid? */
	uint32_t cpu_clock_mhz;	/* CPU clock speed in MHz */
	uint32_t ddr_clock_mhz;	/* DDR clock (not data rate!) in MHz */
	uint32_t ddr_ref_hertz;	/* DDR Ref clock Hertz */
	int mcu_rev_maj;
	int mcu_rev_min;
	int console_uart;

	/* EEPROM data structures as read from EEPROM or populated by other
	 * means on boards without an EEPROM
	 */
	octeon_eeprom_board_desc_t board_desc;
	octeon_eeprom_clock_desc_t clock_desc;
	octeon_eeprom_mac_addr_t mac_desc;

#if	_MIPS_SZPTR == 32
	uint32_t pad1;
#endif
	void **jt;		/* jump table, not used */
#if	_MIPS_SZPTR == 32
	uint32_t pad2;
#endif
	char *err_msg;		/* pointer to error message to save
				 * until console is up.  Not used.
				 */
	char tmp_str[GD_TMP_STR_SIZE];

	uint32_t uboot_flash_address;	/* Address of normal bootloader
					 * in flash
					 */
	uint32_t uboot_flash_size;	/* Size of normal bootloader */
	uint64_t dfm_ram_size;	/* DFM RAM size */
};
typedef struct linux_app_global_data linux_app_global_data_t;

/* Flags for linux_app_global_data */
#define LA_GD_FLG_RELOC			0x0001	/* Code was relocated to RAM     */
#define LA_GD_FLG_DEVINIT		0x0002	/* Devices have been initialized */
#define LA_GD_FLG_SILENT		0x0004	/* Silent mode                   */
#define LA_GD_FLG_CLOCK_DESC_MISSING	0x0008
#define LA_GD_FLG_BOARD_DESC_MISSING	0x0010
#define LA_GD_FLG_DDR_VERBOSE		0x0020
#define LA_GD_FLG_DDR0_CLK_INITIALIZED	0x0040
#define LA_GD_FLG_DDR1_CLK_INITIALIZED	0x0080
#define LA_GD_FLG_DDR2_CLK_INITIALIZED	0x0100
#define LA_GD_FLG_DDR3_CLK_INITIALIZED  0x0200
#define LA_GD_FLG_FAILSAFE_MODE		0x0400	/* Use failsafe mode */
#define LA_GD_FLG_DDR_TRACE_INIT	0x0800
#define LA_GD_FLG_DFM_CLK_INITIALIZED	0x1000
#define LA_GD_FLG_DFM_VERBOSE		0x2000
#define LA_GD_FLG_DFM_TRACE_INIT	0x4000
#define LA_GD_FLG_MEMORY_PRESERVED	0x8000
#define LA_GD_FLG_RAM_RESIDENT		0x10000	/* RAM boot detected */
#endif /* __ASM_GBL_DATA_H */

/*
 * Definition of a data structure setup by the bootloader to enable Linux to
 * launch SE apps on idle cores.
 */
struct linux_app_boot_info {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t labi_signature;
	uint32_t start_core0_addr;
	uint32_t avail_coremask;
	uint32_t pci_console_active;
	uint32_t icache_prefetch_disable;
	uint32_t padding;
	uint64_t InitTLBStart_addr;
	uint32_t start_app_addr;
	uint32_t cur_exception_base;
	uint32_t no_mark_private_data;
	uint32_t compact_flash_common_base_addr;
	uint32_t compact_flash_attribute_base_addr;
	uint32_t led_display_base_addr;
#else
	uint32_t start_core0_addr;
	uint32_t labi_signature;

	uint32_t pci_console_active;
	uint32_t avail_coremask;

	uint32_t padding;
	uint32_t icache_prefetch_disable;

	uint64_t InitTLBStart_addr;

	uint32_t cur_exception_base;
	uint32_t start_app_addr;

	uint32_t compact_flash_common_base_addr;
	uint32_t no_mark_private_data;

	uint32_t led_display_base_addr;
	uint32_t compact_flash_attribute_base_addr;
#endif
#if defined(__ASM_GBL_DATA_H)	/* defined above */
	linux_app_global_data_t gd;
#endif
};
typedef struct linux_app_boot_info linux_app_boot_info_t;

#endif

/* If not to copy a lot of bootloader's structures
   here is only offset of requested member */
#define AVAIL_COREMASK_OFFSET_IN_LINUX_APP_BOOT_BLOCK    0x765c

/* hardcoded in bootloader */
#define LABI_ADDR_IN_BOOTLOADER                         0x700

#define LINUX_APP_BOOT_BLOCK_NAME "linux-app-boot"

#define LABI_SIGNATURE 0xAABBCC01

/*  from uboot-headers/octeon_mem_map.h */
#if defined(CVMX_BUILD_FOR_LINUX_KERNEL) || defined(CVMX_BUILD_FOR_TOOLCHAIN)
# define EXCEPTION_BASE_INCR     (4 * 1024)
#endif

/* Increment size for exception base addresses (4k minimum) */
#define EXCEPTION_BASE_BASE     0

#define BOOTLOADER_PRIV_DATA_BASE	(EXCEPTION_BASE_BASE + 0x800)
/* 0x800 */
#define BOOTLOADER_BOOT_VECTOR		(BOOTLOADER_PRIV_DATA_BASE)
#define BOOT_VECTOR_NUM_WORDS		(8)
#define BOOT_VECTOR_SIZE		((CVMX_MAX_CORES * 4) * BOOT_VECTOR_NUM_WORDS)
/* 0xe00 */
#define BOOTLOADER_DEBUG_TRAMPOLINE      (BOOTLOADER_BOOT_VECTOR + BOOT_VECTOR_SIZE)	/* WORD */
/* 0xe04 */
#define BOOTLOADER_DEBUG_TRAMPOLINE_CORE (BOOTLOADER_DEBUG_TRAMPOLINE + 4)	/* WORD */

/* ADDRESS 0xec4 */
#define BOOTLOADER_DEBUG_FLAGS_BASE		\
	(BOOTLOADER_DEBUG_TRAMPOLINE_CORE + 4*CVMX_MAX_CORES)

/* 0xed0 */
#define BOOTLOADER_DEBUG_CORE_ALIGN_PAD		\
	(BOOTLOADER_DEBUG_FLAGS_BASE + 12)

/* WORD address ED0 */
#define BOOTLOADER_DEBUG_CORE_RESET_COUNT	\
	(BOOTLOADER_DEBUG_CORE_ALIGN_PAD + 0)

/* WORD address ED4 */
#define BOOTLOADER_DEBUG_CORE_REVID_COUNT	\
	(BOOTLOADER_DEBUG_CORE_RESET_COUNT + 4)

/* WORD address ED8 */

#define BOOTLOADER_NEXT_AVAIL_ADDR		\
	(BOOTLOADER_DEBUG_CORE_REVID_COUNT + 4)

#define OCTEON_EXCEPTION_VECTOR_BLOCK_SIZE  (CVMX_MAX_CORES*EXCEPTION_BASE_INCR)	/* 32 4k blocks */
#define BOOTLOADER_DEBUG_REG_SAVE_BASE  (EXCEPTION_BASE_BASE + OCTEON_EXCEPTION_VECTOR_BLOCK_SIZE)

#endif /* __OCTEON_BOOT_INFO_H__ */
