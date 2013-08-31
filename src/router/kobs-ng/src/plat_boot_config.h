/*
* Copyright (C) 2010-2012 Freescale Semiconductor, Inc. All Rights Reserved.
*/

/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __PLAT_BOOT_CONFIG_H__
#define __PLAT_BOOT_CONFIG_H__

#include <stdio.h>
#include <stdlib.h>

#include "mtd.h"

#define CHIP_REV_1_0	0x10
#define CHIP_REV_2_0	0x20
#define CHIP_REV_2_1	0x21

#define mxc_cpu(c)               ((c) >> 12)
#define mxc_cpu_rev(c)           ((c) & 0xFF)
#define mxc_cpu_is_rev(c, rev)     \
        ((mxc_cpu_rev(c) == rev) ? 1 : ((mxc_cpu_rev(c) < rev) ? -1 : 2))

#define MX23	0x23
#define MX28	0x28
#define MX53	0x53
#define MX50	0x50
/* The 3.5.7 kernel uses the 0x6 for MX6Q. */
#define MX6	0x6
#define MX6Q	0x63
#define MX6DL	0x61

typedef struct _platform_config_t {
	uint32_t m_u32RomVer;
	uint32_t m_u32EnDISBBM;
	uint32_t m_u32EnSoftEcc;
	uint32_t m_u32EnBootStreamVerify;
	uint32_t m_u32UseNfcGeo;
	uint32_t m_u32UseMultiBootArea;
	uint32_t m_u32UseSinglePageStride;
	uint32_t m_u32Arm_type;
	uint32_t m_u32DBBT_FingerPrint;
	int (* rom_mtd_init)(struct mtd_data *md, FILE *fp);
	int (* rom_mtd_commit_structures)(struct mtd_data *md, FILE *fp, int flags);
} platform_config;

extern platform_config *plat_config_data;

extern int discover_boot_rom_version(void);

#endif
