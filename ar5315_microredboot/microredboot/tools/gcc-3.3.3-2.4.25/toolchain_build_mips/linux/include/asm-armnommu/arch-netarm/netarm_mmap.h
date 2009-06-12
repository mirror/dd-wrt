/*
 * include/asm-armnommu/arch-netarm/netarm_mmap.h
 *
 * Copyright (C) 2003 Videon Central, Inc.
 * Copyright (C) 2000, 2001 NETsilicon, Inc.
 * Copyright (C) 2000, 2001 Red Hat, Inc.
 *
 * This software is copyrighted by Red Hat. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied 
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall Red Hat
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) : Joe deBlaquiere
 *
 * Modified to support NS7520 by Art Shipkowski <art@videon-central.com>
 */

#ifndef __NETARM_MEMORY_MAP_H
#define __NETARM_MEMORY_MAP_H

#include <linux/autoconf.h>

#ifndef (CONFIG_NETARM_NS7520)
/* structure definitions for per-unit configuration data */
#define	NETARM_MMAP_CS0_BASE	(0x10000000)	/* Flash AM29DL323DB */
#define	NETARM_MMAP_CS0_MASK	(0xFF800000)	/* 8M bytes */

#define	NETARM_MMAP_CS1_BASE	(0x00000000)	/* EDO DRAM MT4LC4M16R6 */
#define	NETARM_MMAP_CS1_MASK	(0xFF000000)	/* 16M bytes */

#define	NETARM_MMAP_CS2_BASE	(0x01000000)	/* EDO DRAM MT4LC4M16R6 */
#define	NETARM_MMAP_CS2_MASK	(0xFF000000)	/* 16M bytes */

#define	NETARM_MMAP_CS3_BASE	(0x20000000)	/* EEPROM - AT28LV64B */
#define	NETARM_MMAP_CS3_MASK	(0xFFFFE000)	/* 8K bytes */

#define	NETARM_MMAP_CS4_BASE	(0x30000000)	/* EPSON SED1355 */
#define	NETARM_MMAP_CS4_MASK	(0xFF000000)	/* Memory Mapped I/O */
#else
/*
 * The NS7520 has a nasty bug relating to SDRAM refresh if you place any
 * chip selects above the first 256MB of addressable space.
 *
 * There is also a limit on memory-to-memory DMA that requires the source
 * address be in the first 512MB addressable.
 *
 * These defaults are a little silly IMHO, anyhow. Perhaps they should be
 * set up in the kernel configuration for those not using a bootloader?
 */
#define	NETARM_MMAP_CS0_BASE	(FLASH_BASE) 	/* Flash AM29DL323DB */
#define	NETARM_MMAP_CS0_MASK	(0xFF800000)	/* 8M bytes */

#define	NETARM_MMAP_CS1_BASE	(DRAM_BASE) 	/* EDO DRAM MT4LC4M16R6 */
#define	NETARM_MMAP_CS1_MASK	(0xFF000000)	/* 16M bytes */

#define	NETARM_MMAP_CS2_BASE	(0x01000000)	/* EDO DRAM MT4LC4M16R6 */
#define	NETARM_MMAP_CS2_MASK	(0xFF000000)	/* 16M bytes */

#define	NETARM_MMAP_CS3_BASE	(0x02000000)	/* EEPROM - AT28LV64B */
#define	NETARM_MMAP_CS3_MASK	(0xFFFFE000)	/* 8K bytes */

#define	NETARM_MMAP_CS4_BASE	(0x03000000)	/* EPSON SED1355 */
#define	NETARM_MMAP_CS4_MASK	(0xFF000000)	/* Memory Mapped I/O */
#endif

#define NETARM_MMAP_RAM_BASE	(NETARM_MMAP_CS1_BASE)
#define NETARM_MMAP_FLASH_BASE	(NETARM_MMAP_CS0_BASE)
#define NETARM_MMAP_EEPROM_BASE	(NETARM_MMAP_CS3_BASE)
#define NETARM_MMAP_FBDEV_BASE	(NETARM_MMAP_CS4_BASE)

#ifdef	CONFIG_NETARM_NET40_REV2
#define NETARM_MMAP_FLASH_COPY_SIZE	(0x00100000)	/* 1MB copy */
#else
#define NETARM_MMAP_FLASH_COPY_SIZE	(0x00400000)	/* 4MB copy */
#endif

#ifdef	CONFIG_ETHER_NETARM
/* the OUI is defined via make xconfig (arch/armnommu/drivers/net/Config.in) */
/* #define	NETARM_OUI		(0x0040AF) */

#define	NETARM_OUI_BYTE1	((NETARM_OUI >> 16) & 0xFF)
#define NETARM_OUI_BYTE2	((NETARM_OUI >> 8) & 0xFF)
#define NETARM_OUI_BYTE3	(NETARM_OUI & 0xFF)
#endif

#endif

