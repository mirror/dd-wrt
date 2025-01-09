/*
 * jffs.c
 *
 * Copyright (C) 2007 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#ifdef HAVE_JFFS2
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <sys/mount.h>

#define DEFAULT_UBIFS_COMPR "zstd"

void stop_jffs2(void)
{
#if defined(HAVE_R9000)
	int mtd = getMTD("plex");
#else
	int mtd = getMTD("ddwrt");
	if (mtd == -1)
		mtd = getMTD("jffs2");
#endif
	if (mtd == -1)
		return;
	umount("/jffs2");
	rmmod("jffs2");
	nvram_seti("jffs_mounted", 0);
}

void start_jffs2(void)
{
	char *rwpart = "ddwrt";
	int didntwork = 0;
	char dev[64];
	int classic = 0;
	int ax89 = 0;
#if defined(HAVE_R9000)
	int mtd = getMTD("plex");
#else
	int mtd = getMTD("ddwrt");
	if (mtd == -1) {
		mtd = getMTD("jffs2");
	}
#endif
	int ubidev = 1;
	int brand = getRouterBrand();
#if defined(HAVE_IPQ806X) || defined(HAVE_IPQ6018)
	switch (brand) {
	case ROUTER_ASUS_AC58U:
		classic = 1;
		break;
	case ROUTER_ASUS_AX89X:
		rwpart = "jffs2";
		ax89 = 1;
		break;
	case ROUTER_DYNALINK_DLWRX36:
		ubidev = 0;
		rwpart = "rootfs_data";
		break;
	case ROUTER_FORTINET_FAP231F:
		ubidev = 1;
		mtd = getMTD("fap_data");
		break;
	case ROUTER_TRENDNET_TEW827:
	case ROUTER_ASROCK_G10:
	case ROUTER_NETGEAR_R9000:
	case ROUTER_LINKSYS_EA8300:
	case ROUTER_LINKSYS_MR7350:
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
	case ROUTER_LINKSYS_MR5500:
	case ROUTER_LINKSYS_MX5500:
		ubidev = 1;
		break;
	case ROUTER_LINKSYS_EA8500:
	default:
		ubidev = 0;
		break;
	}
#endif
	nvram_seti("jffs_mounted", 0);
	if (!nvram_matchi("enable_jffs2", 1) || nvram_matchi("clean_jffs2", 1)) {
		umount2("/jffs2", MNT_DETACH);
	}
	char udev[32];
	sprintf(udev, "/dev/ubi%d", ubidev);
	char upath[32];
	sprintf(upath, "ubi%d:%s", ubidev, rwpart);
	if (nvram_matchi("enable_jffs2", 1)) {
		insmod("crc32 lzma_compress lzma_decompress lzo_compress lzo_decompress jffs2");
		if (nvram_matchi("clean_jffs2", 1)) {
			nvram_seti("clean_jffs2", 0);
			nvram_commit();
			sprintf(dev, "/dev/mtd%d", mtd);
#if defined(HAVE_DW02_412H)
			eval("erase", rwpart);
			eval("flash_erase", dev, "0", "0");
			eval("mkfs.jffs2", "-o", "/dev/mtdblock11", "-n", "-b", "-e", "131072", "-p");
#elif defined(HAVE_WNDR3700V4)
			eval("erase", rwpart);
			eval("flash_erase", dev, "0", "0");
			eval("mkfs.jffs2", "-o", "/dev/mtdblock3", "-n", "-b", "-e", "131072", "-p");
#elif defined(HAVE_MVEBU) || defined(HAVE_R9000) || defined(HAVE_IPQ806X) || defined(HAVE_R6800) || defined(HAVE_IPQ6018)
			if (ax89) {
				eval("mtd", "erase", rwpart);
			} else if (classic) {
				eval("mtd", "erase", rwpart);
				eval("flash_erase", dev, "0", "0");
			} else {
				if (brand == ROUTER_DYNALINK_DLWRX36) {
					eval("mkfs.ubifs", "-x", "zstd", "-y", "/dev/ubi0_2");
				} else {
					eval("ubidetach", "-p", dev);
					eval("mtd", "erase", dev);
					eval("flash_erase", dev, "0", "0");
					eval("ubiattach", "-p", dev);
					eval("ubimkvol", udev, "-N", "ddwrt", "-m");
				}
			}
#else
			eval("mtd", "erase", rwpart);
			eval("flash_erase", dev, "0", "0");
#endif

#if defined(HAVE_R9000) || defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_R6800) || defined(HAVE_IPQ6018)
			if (ax89) {
				didntwork = mount("/dev/ubi0_5", "/jffs", "ubifs", MS_MGC_VAL | MS_NOATIME,
						  "compr=" DEFAULT_UBIFS_COMPR);
			} else if (classic) {
				sprintf(dev, "/dev/mtdblock/%d", getMTD(rwpart));
				didntwork = mount(dev, "/jffs", "jffs2", MS_MGC_VAL | MS_NOATIME, NULL);
			} else {
				didntwork = mount(upath, "/jffs", "ubifs", MS_MGC_VAL | MS_NOATIME, "compr=" DEFAULT_UBIFS_COMPR);
			}
#else
			sprintf(dev, "/dev/mtdblock/%d", getMTD(rwpart));
			didntwork = mount(dev, "/jffs", "jffs2", MS_MGC_VAL | MS_NOATIME, NULL);
#endif
			if (didntwork) {
				nvram_seti("jffs_mounted", 0);
			} else {
				nvram_seti("jffs_mounted", 1);
			}

		} else {
#if defined(HAVE_R9000) || defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_R6800) || defined(HAVE_IPQ6018)
			didntwork = 0;
			if (ax89) {
				didntwork += mount("/dev/ubi0_5", "/jffs", "ubifs", MS_MGC_VAL | MS_NOATIME,
						   "compr=" DEFAULT_UBIFS_COMPR);
			} else if (classic) {
				eval("mtd", "unlock", rwpart);
				sprintf(dev, "/dev/mtdblock/%d", getMTD(rwpart));
				didntwork = mount(dev, "/jffs", "jffs2", MS_MGC_VAL | MS_NOATIME, NULL);
			} else {
				sprintf(dev, "/dev/mtd%d", mtd);
				if (mtd >= 0)
					eval("ubiattach", "-p", dev);
				didntwork = mount(upath, "/jffs", "ubifs", MS_MGC_VAL | MS_NOATIME, "compr=" DEFAULT_UBIFS_COMPR);
			}
#else
			eval("mtd", "unlock", rwpart);
			sprintf(dev, "/dev/mtdblock/%d", getMTD(rwpart));
			didntwork = mount(dev, "/jffs", "jffs2", MS_MGC_VAL | MS_NOATIME, NULL);
#endif
			if (didntwork) {
				nvram_seti("jffs_mounted", 0);
				nvram_seti("clean_jffs2", 1);
				start_jffs2();
			} else {
				nvram_seti("jffs_mounted", 1);
			}
		}
	}
}
#endif
