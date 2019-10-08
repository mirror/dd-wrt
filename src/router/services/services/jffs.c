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
	int mtd = getMTD("ddwrt");
#if defined(HAVE_R9000)
	int mtd = getMTD("plex");
#else
	int mtd = getMTD("ddwrt");
#endif
	char blockdev[32];
	sprintf(blockdev, "/dev/mtdblock%d", mtd);

	if (nvram_matchi("sys_enable_jffs2", 1)) {
		insmod("crc32 lzma_compress lzma_decompress lzo_compress lzo_decompress jffs2");
		if (nvram_matchi("sys_clean_jffs2", 1)) {
			nvram_seti("sys_clean_jffs2", 0);
			nvram_commit();
#if defined(HAVE_WNDR3700V4) || defined(HAVE_IPQ806X)
			itworked = eval("erase", rwpart);
			itworked = eval("mkfs.jffs2", "-x", "zlib", "-x", "rtime", "-o", blockdev, "-n", "-b", "-e", "131072", "-p");
#elif defined(HAVE_MVEBU) || defined(HAVE_R9000)
			sprintf(dev, "/dev/mtd%d", mtd);
			itworked = eval("ubidetach", "-p", dev);
			itworked = eval("mtd", "erase", dev);
			itworked = eval("ubiattach", "-p", dev);
			itworked = eval("ubimkvol", "/dev/ubi1", "-N", "ddwrt", "-m");
#else
			itworked = eval("mtd", "erase", rwpart);
#endif

#if defined(HAVE_R9000) || defined(HAVE_MVEBU)
			itworked += mount("ubi1:ddwrt", "/jffs", "ubifs", MS_MGC_VAL, NULL);
#else
			sprintf(dev, "/dev/mtdblock/%d", mtd);
			itworked += mount(dev, "/jffs", "jffs2", MS_MGC_VAL, NULL);
#endif
			if (itworked) {
				nvram_seti("jffs_mounted", 0);
			} else {
				nvram_seti("jffs_mounted", 1);
			}

		} else {
#if defined(HAVE_R9000) || defined(HAVE_MVEBU)
			sprintf(dev, "/dev/mtd%d", mtd);
			itworked = eval("ubiattach", "-p", dev);
			itworked += mount("ubi1:ddwrt", "/jffs", "ubifs", MS_MGC_VAL, NULL);
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
