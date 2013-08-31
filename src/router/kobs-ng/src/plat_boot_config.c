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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mtd.h"
#include "plat_boot_config.h"

/* System version */
platform_config *plat_config_data;

static platform_config mx23_boot_config = {
	.m_u32RomVer = ROM_Version_0,
	.m_u32EnDISBBM = 0,
	.m_u32EnSoftEcc = 0,
	.m_u32EnBootStreamVerify = 1,
	.m_u32UseNfcGeo = 1,
	.m_u32UseMultiBootArea = 1,
	.m_u32UseSinglePageStride = 0,
	.m_u32DBBT_FingerPrint = DBBT_FINGERPRINT2,
	.rom_mtd_init = v0_rom_mtd_init,
	.rom_mtd_commit_structures = v0_rom_mtd_commit_structures,
};

static platform_config mx28_boot_config = {
	.m_u32RomVer = ROM_Version_1,
	.m_u32EnDISBBM = 0,
	.m_u32EnSoftEcc = 1,
	.m_u32EnBootStreamVerify = 1,
	.m_u32UseNfcGeo = 1,
	.m_u32UseMultiBootArea = 0,
	.m_u32UseSinglePageStride = 1,
	.m_u32DBBT_FingerPrint = DBBT_FINGERPRINT2,
	.rom_mtd_init = v1_rom_mtd_init,
	.rom_mtd_commit_structures = v1_rom_mtd_commit_structures,
};

static platform_config mx53to1_boot_config = {
	.m_u32RomVer = ROM_Version_2,
	.m_u32EnDISBBM = 0,
	.m_u32EnSoftEcc = 0,
	.m_u32EnBootStreamVerify = 0,
	.m_u32UseNfcGeo = 0,
	.m_u32UseMultiBootArea = 0,
	.m_u32UseSinglePageStride = 0,
	.m_u32DBBT_FingerPrint = DBBT_FINGERPRINT2_V2,
	.rom_mtd_init = v2_rom_mtd_init,
	.rom_mtd_commit_structures = v2_rom_mtd_commit_structures,
};

static platform_config mx53to2_boot_config = {
	.m_u32RomVer = ROM_Version_2,
	.m_u32EnDISBBM = 1,
	.m_u32EnSoftEcc = 0,
	.m_u32EnBootStreamVerify = 0,
	.m_u32UseNfcGeo = 0,
	.m_u32UseMultiBootArea = 0,
	.m_u32UseSinglePageStride = 0,
	.m_u32DBBT_FingerPrint = DBBT_FINGERPRINT2_V2,
	.rom_mtd_init = v2_rom_mtd_init,
	.rom_mtd_commit_structures = v2_rom_mtd_commit_structures,
};

static platform_config mx50_boot_config = {
	.m_u32RomVer = ROM_Version_3,
	.m_u32EnDISBBM = 0,
	.m_u32EnSoftEcc = 1,
	.m_u32EnBootStreamVerify = 0,
	.m_u32UseNfcGeo = 1,
	.m_u32UseMultiBootArea = 0,
	.m_u32UseSinglePageStride = 0,
	.m_u32DBBT_FingerPrint = DBBT_FINGERPRINT2,
	.rom_mtd_init = v4_rom_mtd_init,
	.rom_mtd_commit_structures = v4_rom_mtd_commit_structures,
};

static platform_config mx6q_boot_config = {
	.m_u32RomVer = ROM_Version_3,
	.m_u32EnDISBBM = 0,
	.m_u32EnBootStreamVerify = 0,
	.m_u32UseNfcGeo = 0,
	.m_u32UseMultiBootArea = 0,
	.m_u32UseSinglePageStride = 0,
	.m_u32DBBT_FingerPrint = DBBT_FINGERPRINT2,
	.rom_mtd_init = v4_rom_mtd_init,
	.rom_mtd_commit_structures = v4_rom_mtd_commit_structures,
};

int discover_boot_rom_version(void)
{
	FILE         *cpuinfo;
	char         line_buffer[100];
	static char  *banner = "Revision";
	static char  *banner_hw = "Hardware";
	char *rev;
	int system_rev, hw_system_rev = 0;

	cpuinfo = fopen("/proc/cpuinfo", "r");
	if (!cpuinfo) {
		fprintf(stderr, "Can't open /proc/cpuinfo to"
			       "discover Boot ROM version.\n");
		exit(1);
	}

	for (;;) {
		if (!fgets(line_buffer, sizeof(line_buffer), cpuinfo))
			break;

		/* Check if it's revision line */
		if (strncmp(line_buffer, banner, strlen(banner))) {
			/*
			 * Why use the `Hardware` to parse the system type ?
			 * [1] If boot linux kernel directly from SD card not by uboot,
			 *     the `Revision` will be zero.
			 * [2] The code does not change the old logic.
			 */
			if (!strncmp(line_buffer, banner_hw, strlen(banner))) {
				rev = strstr(line_buffer, "MX");
				if (rev) {
					char tmp[3] = {};

					rev += 2;
					memcpy(tmp, rev, 2);
					hw_system_rev = strtoul(tmp, NULL, 16);
				}
			}
			continue;
		}

		rev = index(line_buffer, ':');
		if (rev != NULL) {
			rev++;
			system_rev = strtoul(rev, NULL, 16);
			system_rev = mxc_cpu(system_rev);
			if (!system_rev)
				system_rev = hw_system_rev;

			switch (system_rev) {
			case  MX23:
				plat_config_data = &mx23_boot_config;
				break;

			case  MX28:
				plat_config_data = &mx28_boot_config;
				break;

			case  MX53:
				if (mxc_cpu_is_rev(system_rev, CHIP_REV_2_0) < 0)
					plat_config_data = &mx53to1_boot_config;
				else
					plat_config_data = &mx53to2_boot_config;
				break;

			case  MX50:
				plat_config_data = &mx50_boot_config;
				break;

			case MX6:
			case MX6Q:
			case MX6DL:
				plat_config_data = &mx6q_boot_config;
				break;

			default:
				fprintf(stderr, "Couldn't find Boot ROM version\n");
				break;
			}

			fclose(cpuinfo);
			if (plat_config_data) {
				plat_config_data->m_u32Arm_type = system_rev;
				return 0;
			}
			return -1;
		}
	}
	fclose(cpuinfo);
	return -1;
}
