/*
 * jffs.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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
	eval("umount", "/jffs");
	rmmod("jffs2");
}

void start_jffs2(void)
{
	char *rwpart = "ddwrt";
	int itworked = 0;
	char dev[64];
#if defined(HAVE_R9000)
	int mtd = getMTD("plex");
#else
	int mtd = getMTD("ddwrt");
#endif
	int ubidev = 1;

#ifdef HAVE_IPQ806X
	int brand = getRouterBrand();
	switch (brand) {
	case ROUTER_TRENDNET_TEW827:
	case ROUTER_ASROCK_G10:
	case ROUTER_NETGEAR_R9000:
		ubidev = 1;
		break;
	case ROUTER_LINKSYS_EA8500:
	default:
		ubidev = 0;
		break;
	}
#endif
	char udev[32];
	sprintf(udev, "/dev/ubi%d", ubidev);
	char upath[32];
	sprintf(upath, "ubi%d:ddwrt", ubidev);
	if (nvram_matchi("sys_enable_jffs2", 1)) {
		insmod("crc32 lzma_compress lzma_decompress lzo_compress lzo_decompress jffs2");
		if (nvram_matchi("sys_clean_jffs2", 1)) {
			nvram_seti("sys_clean_jffs2", 0);
			nvram_commit();
			sprintf(dev, "/dev/mtd%d", mtd);
#if defined(HAVE_WNDR3700V4)
			itworked = eval("erase", rwpart);
			itworked = eval("flash_erase", dev, "0", "0");
			itworked = eval("mkfs.jffs2", "-o", "/dev/mtdblock3", "-n", "-b", "-e", "131072", "-p");
#elif defined(HAVE_MVEBU) || defined(HAVE_R9000) || defined(HAVE_IPQ806X)
			itworked = eval("ubidetach", "-p", dev);
			itworked = eval("mtd", "erase", dev);
			itworked = eval("flash_erase", dev, "0", "0");
			itworked = eval("ubiattach", "-p", dev);
			itworked = eval("ubimkvol", udev, "-N", "ddwrt", "-m");
#else
			itworked = eval("mtd", "erase", rwpart);
			itworked = eval("flash_erase", dev, "0", "0");
#endif

#if defined(HAVE_R9000) || defined(HAVE_MVEBU) || defined(HAVE_IPQ806X)
			itworked += mount(upath, "/jffs", "ubifs", MS_MGC_VAL, NULL);
#else
			sprintf(dev, "/dev/mtdblock/%d", getMTD("ddwrt"));
			itworked += mount(dev, "/jffs", "jffs2", MS_MGC_VAL, NULL);
#endif
			if (itworked) {
				nvram_seti("jffs_mounted", 0);
			} else {
				nvram_seti("jffs_mounted", 1);
			}

		} else {
#if defined(HAVE_R9000) || defined(HAVE_MVEBU) || defined(HAVE_IPQ806X)
			sprintf(dev, "/dev/mtd%d", mtd);
			itworked = eval("ubiattach", "-p", dev);
			itworked += mount(upath, "/jffs", "ubifs", MS_MGC_VAL, NULL);
#else
			itworked = eval("mtd", "unlock", rwpart);
			sprintf(dev, "/dev/mtdblock/%d", getMTD("ddwrt"));
			itworked += mount(dev, "/jffs", "jffs2", MS_MGC_VAL, NULL);
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
