/*
 * jffs.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

void stop_jffs2(void)
{
#if defined(HAVE_R9000)
	int mtd = getMTD("plex");
#else
	int mtd = getMTD("ddwrt");
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
	int itworked = 0;
	char dev[64];
	int classic = 0;
#if defined(HAVE_R9000)
	int mtd = getMTD("plex");
#else
	int mtd = getMTD("ddwrt");
#endif
	if (mtd == -1)
		return;
	int ubidev = 1;

#ifdef HAVE_IPQ806X
	int brand = getRouterBrand();
	switch (brand) {
	case ROUTER_ASUS_AC58U:
		classic = 1;
		break;
	case ROUTER_TRENDNET_TEW827:
	case ROUTER_ASROCK_G10:
	case ROUTER_NETGEAR_R9000:
	case ROUTER_LINKSYS_EA8300:
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
	sprintf(upath, "ubi%d:ddwrt", ubidev);
	if (nvram_matchi("enable_jffs2", 1)) {
		insmod("crc32 lzma_compress lzma_decompress lzo_compress lzo_decompress jffs2");
		if (nvram_matchi("clean_jffs2", 1)) {
			nvram_seti("clean_jffs2", 0);
			nvram_commit();
			sprintf(dev, "/dev/mtd%d", mtd);
#if defined(HAVE_DW02_412H)
			itworked = eval("erase", rwpart);
			itworked = eval("flash_erase", dev, "0", "0");
			itworked = eval("mkfs.jffs2", "-o", "/dev/mtdblock11", "-n", "-b", "-e", "131072", "-p");
#elif defined(HAVE_WNDR3700V4)
			itworked = eval("erase", rwpart);
			itworked = eval("flash_erase", dev, "0", "0");
			itworked = eval("mkfs.jffs2", "-o", "/dev/mtdblock3", "-n", "-b", "-e", "131072", "-p");
#elif defined(HAVE_MVEBU) || defined(HAVE_R9000) || defined(HAVE_IPQ806X) || defined(HAVE_R6800)
			if (classic) {
				itworked = eval("mtd", "erase", rwpart);
				itworked = eval("flash_erase", dev, "0", "0");
			} else {
				itworked = eval("ubidetach", "-p", dev);
				itworked = eval("mtd", "erase", dev);
				itworked = eval("flash_erase", dev, "0", "0");
				itworked = eval("ubiattach", "-p", dev);
				itworked = eval("ubimkvol", udev, "-N", "ddwrt", "-m");
			}
#else
			itworked = eval("mtd", "erase", rwpart);
			itworked = eval("flash_erase", dev, "0", "0");
#endif

#if defined(HAVE_R9000) || defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_R6800)
			if (classic) {
				sprintf(dev, "/dev/mtdblock/%d", getMTD("ddwrt"));
				itworked += mount(dev, "/jffs", "jffs2", MS_MGC_VAL | MS_NOATIME, NULL);
			} else {
				itworked += mount(upath, "/jffs", "ubifs", MS_MGC_VAL | MS_NOATIME, NULL);
			}
#else
			sprintf(dev, "/dev/mtdblock/%d", getMTD("ddwrt"));
			itworked += mount(dev, "/jffs", "jffs2", MS_MGC_VAL | MS_NOATIME, NULL);
#endif
			if (itworked) {
				nvram_seti("jffs_mounted", 0);
			} else {
				nvram_seti("jffs_mounted", 1);
			}

		} else {
#if defined(HAVE_R9000) || defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_R6800)
			if (classic) {
				itworked = eval("mtd", "unlock", rwpart);
				sprintf(dev, "/dev/mtdblock/%d", getMTD("ddwrt"));
				itworked += mount(dev, "/jffs", "jffs2", MS_MGC_VAL | MS_NOATIME, NULL);
			} else {
				sprintf(dev, "/dev/mtd%d", mtd);
				itworked = eval("ubiattach", "-p", dev);
				itworked += mount(upath, "/jffs", "ubifs", MS_MGC_VAL | MS_NOATIME, NULL);
			}
#else
			itworked = eval("mtd", "unlock", rwpart);
			sprintf(dev, "/dev/mtdblock/%d", getMTD("ddwrt"));
			itworked += mount(dev, "/jffs", "jffs2", MS_MGC_VAL | MS_NOATIME, NULL);
#endif
			if (itworked) {
				nvram_seti("jffs_mounted", 0);
			} else {
				nvram_seti("jffs_mounted", 1);
			}
		}
	}
}
#endif
