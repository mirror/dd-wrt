/*
 * mtd.c
 *
 * Copyright (C) 2006 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __UCLIBC__
	#include <error.h>
#endif
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>

#include <string.h>
#include <linux/version.h>

#if KERNEL_VERSION(2, 6, 0) <= LINUX_VERSION_CODE
	#define IS_KERNEL26 1
#else
	#define IS_KERNEL26 0
#endif

#if IS_KERNEL26
	#include <linux/mtd/mtd-abi.h>
#else
	#include <linux/mtd/mtd.h>
#endif

#include <trxhdr.h>
#include <crc.h>
#include <ddnvram.h>
#include <shutils.h>

#include <cy_conf.h>
#include <utils.h>

#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
	#define STORE32_LE(X) bswap_32(X)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	#define STORE32_LE(X) (X)
#else
	#error unkown endianness!
#endif

#ifndef OFFSETOF
	#define OFFSETOF(type, member) ((uint)(uintptr) & ((type *)0)->member)
#endif

#ifndef ROUNDUP
	#define ROUNDUP(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#endif

/* Netgear definitions */

#define WGR614_LZMA_LOADER_SIZE 0x000919 //loader+400.lzma = 2329 bytes, please change if size changes!
#define FLASH_SIZE_4M 0x400000
#define FLASH_SIZE_8M 0x800000
#define CFE_SIZE_128K 0x020000
#define CFE_SIZE_256K 0x040000
#define NETGEAR_LEN_CHK_ADDR_4M 0x3AFFF8
#define NETGEAR_LEN_CHK_ADDR_8M 0x7AFFF8
#define NETGEAR_LEN_CHK_ADDR_8M_2 0x73FFF8
/* end */

/* Belkin series */
#define TRX_MAGIC_F7D3301 0x20100322 /* Belkin Share Max; router's birthday ? */
#define TRX_MAGIC_F7D3302 0x20090928 /* Belkin Share; router's birthday ? */
#define TRX_MAGIC_F7D4302 0x20091006 /* Belkin Play; router's birthday ? */
#define TRX_MAGIC_F5D8235V3 0x00017116 /* Belkin F7D8235V3 */
#define TRX_MAGIC_QA 0x12345678 /* Belkin: cfe: It's QA firmware */
/* end */

/* Netgear chk header */
#define NETGEAR_CHK_MAGIC 0x5E24232A

#include <asm/posix_types.h>

#ifndef MEMGETBADBLOCK
	#define MEMGETBADBLOCK _IOW('M', 11, __kernel_loff_t)
#endif
static int mtdtype = 0;

static int mtd_block_is_bad(int fd, int offset)
{
	int r = 0;
	loff_t o = offset;

	if (mtdtype == MTD_NANDFLASH) {
		r = ioctl(fd, MEMGETBADBLOCK, &o);
		if (r < 0) {
			dd_logerror("flash", "Failed to get erase block status");
			exit(1);
		}
	}
	return r;
}

/* 
 * Open an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @param       flags   open() flags
 * @return      return value of open()
 */
static int mtd_open(const char *mtd, int flags)
{
	FILE *fp;
	char dev[PATH_MAX];
	int i;

	if ((fp = fopen("/proc/mtd", "r"))) {
		while (fgets(dev, sizeof(dev), fp)) {
			if (sscanf(dev, "mtd%d:", &i) && strstr(dev, mtd)) {
				if (!strcmp(mtd, "nvram") && strstr(dev, "nvram_cfe"))
					continue;
				snprintf(dev, sizeof(dev), "/dev/mtd/%d", i);
				fclose(fp);
				return open(dev, flags);
			}
		}
		fclose(fp);
	}

	return open(mtd, flags);
}

/* 
 * Erase an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @return      0 on success and errno on failure
 */
int mtd_erase(const char *mtd)
{
	int mtd_fd;
	struct mtd_info_user mtd_info;
	struct erase_info_user erase_info;

	/* 
	 * char *et0; char *et1;
	 * 
	 * et0=nvram_safe_get("et0macaddr"); et1=nvram_safe_get("et1macaddr");
	 * et0=strdup(et0); et1=strdup(et1); 
	 */
	/* 
	 * Open MTD device 
	 */
	if ((mtd_fd = mtd_open(mtd, O_RDWR)) < 0) {
		perror(mtd);
		return errno;
	}

	/* 
	 * Get sector size 
	 */
	if (ioctl(mtd_fd, MEMGETINFO, &mtd_info) != 0) {
		perror(mtd);
		close(mtd_fd);
		return errno;
	}

	erase_info.length = mtd_info.erasesize;
	mtdtype = mtd_info.type;
	if (mtdtype == MTD_NANDFLASH)
		dd_loginfoverbose("flash", "Flash is NAND");
	for (erase_info.start = 0; erase_info.start < mtd_info.size; erase_info.start += mtd_info.erasesize) {
		dd_loginfoverbose("flash", "erase[%d]", erase_info.start);
		(void)ioctl(mtd_fd, MEMUNLOCK, &erase_info);
		if (mtd_block_is_bad(mtd_fd, erase_info.start)) {
			dd_logerror("flash", "\nSkipping bad block at 0x%08zx", erase_info.start);
			continue;
		}
		if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0) {
			perror(mtd);
			close(mtd_fd);
			return errno;
		}
	}

	close(mtd_fd);
	dd_loginfoverbose("flash", "erase[%d]", erase_info.start);
	/* 
	 * nvram_set("et0macaddr",et0); nvram_set("et1macaddr",et1);
	 * nvram_commit(); free(et0); free(et1); 
	 */

	return 0;
}

extern int http_get(const char *server, char *buf, size_t count, off_t offset);

/* 
 * Write a file to an MTD device
 * @param       path    file to write or a URL
 * @param       mtd     path to or partition name of MTD device 
 * @return      0 on success and errno on failure
 */

#define WRITE_BLOCKSIZE 65536

#define SQUASHFS_MAGIC 0x74717368

static int write_main(int argc, char *argv[])
{
	struct erase_info_user erase_info;
	struct erase_info_user tmp_erase_info;

	struct sysinfo info;
	struct trx_header trx;
	unsigned int crc;
	int squashfound = 0;
	unsigned int crc_data = 0;
	unsigned int data_len = 0;
	unsigned int cal_chksum = 0;
	FILE *fp;
	FILE *p;
	char *buf = NULL;
	int count, len, off;
	int sum = 0; // for debug
	int ret = -1;
	int i = 0;
	int badblocks = 0;
	unsigned char lzmaloader[4096];
	int brand = getRouterBrand();

	if (argc < 3) {
		fprintf(stderr, "usage: write [path] [device]\n");
		return -EINVAL;
	}
	const char *path = argv[1];
	const char *mtd = argv[2];
	int writeubi = 0;
	int writeubifs = 0;
	int writeubiformat = 0;
	/* 
	 * Netgear WGR614v8_L: Read, store and write back old lzma loader from 1st block 
	 */
	unsigned int trxhd = STORE32_LE(TRX_MAGIC);

	char *part;

#if defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_IPQ6018)
	switch (brand) {
	case ROUTER_WRT_1900AC:
	case ROUTER_WRT_1200AC:
	case ROUTER_WRT_1900ACV2:
	case ROUTER_WRT_1900ACS:
	case ROUTER_WRT_3200ACM:
	case ROUTER_WRT_32X:
	case ROUTER_LINKSYS_EA8500:
	case ROUTER_LINKSYS_EA8300:
	case ROUTER_LINKSYS_MR7350:
	case ROUTER_LINKSYS_MR7500:
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
	case ROUTER_LINKSYS_MX8500:
	case ROUTER_LINKSYS_MX5300:
	case ROUTER_LINKSYS_MR5500:
	case ROUTER_LINKSYS_MX5500:
		part = getUEnv("boot_part");
		if (part) {
			dd_loginfoverbose("flash", "boot partition is %s", part);
			if (!strcmp(part, "2")) {
				mtd = "linux";
			} else {
				mtd = "linux2";
			}
			dd_loginfoverbose("flash", "flash to partition %s", mtd);
		} else {
			dd_logerror("flash", "no boot partition info found", mtd);
		}
		break;
	case ROUTER_DYNALINK_DLWRX36:
		writeubiformat = 1;
		mtd = "rootfs";
		eval("startservice", "finishupgrade", "-f");
		fprintf(stderr, "set fw env to mtdparts=mtdparts=nand0:0x6100000@0x1000000(fs),0x6100000@0x7a00000(fs_1)\n");
		eval("fw_setenv", "mtdparts", "mtdparts=nand0:0x6100000@0x1000000(fs),0x6100000@0x7a00000(fs_1)");
		break;
	case ROUTER_FORTINET_FAP231F:
		writeubiformat = 1;
		mtd = "rootfs";
		break;
	case ROUTER_GLINET_AX1800:
		writeubiformat = 1;
		mtd = "rootfs";
		break;
	case ROUTER_BUFFALO_WXR5950AX12:
		writeubiformat = 1;
		mtd = "rootfs";
		break;
	}
#endif
rewrite:;
	count = off = 0;
	nvram_seti("flash_active", 1);
	sleep(1);

	/* 
	 * Examine TRX/CHK header 
	 */

	if ((fp = fopen(path, "r"))) {
		count = safe_fread(&trx, 1, sizeof(struct trx_header), fp);
	} else {
		return -1;
	}
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);

	// count = http_get (path, (char *) &trx, sizeof (struct trx_header), 0);
	if (count < sizeof(struct trx_header)) {
		dd_logerror("%s: File is too small (%d bytes)", path, count);
		goto fail;
	}
	sysinfo(&info);
#ifdef HAVE_SNMP
	stop_service("snmp");
#endif
#ifdef HAVE_PPPOESERVER
	stop_service("pppoeserver");
#endif
#ifdef HAVE_OLSRD
	stop_service("olsrd");
#endif
#ifdef HAVE_UPNP
	stop_service("upnpd");
#endif
#ifdef HAVE_FREERADIUS
	stop_service("freeradius");
#endif
#ifdef HAVE_TRANSMISSION
	stop_service("transmission");
#endif
#ifdef HAVE_PLEX
	stop_service("plex");
#endif
#ifdef HAVE_IPV6
	stop_service("radvd");
#endif
	stop_service("resetbutton");
	stop_service("cron");
	stop_service("process_monitor");
	stop_service("wland");
	killall("wdswatchdog.sh", SIGTERM);
	killall("schedulerb.sh", SIGTERM);
	killall("proxywatchdog.sh", SIGTERM);
	eval("service", "syslog", "stop");
	eval("mount", "-f", "-o", "remount,ro", "/jffs");
	eval("umount", "-r", "-f", "/jffs");
	eval("umount", "-r", "-f", "/usr/local");
	eval("mount", "-f", "-o", "remount,ro", "/");
	eval("service", "syslog", "start");

#if defined(HAVE_MVEBU) || defined(HAVE_R9000) || defined(HAVE_IPQ806X) || defined(HAVE_R6800) || defined(HAVE_IPQ6018)
	#if defined(HAVE_R9000)
	int mtddev = getMTD("plex");
	#else
	int mtddev = getMTD("ddwrt");
	if (mtddev == -1) {
		mtddev = getMTD("jffs2");
	}
	#endif
	if (mtddev > 0) {
		char devdev[32];
		sprintf(devdev, "/dev/mtd%d", mtddev);
		eval("ubidetach", "-p", devdev, "-f");
	}
#endif
	if (trx.magic != TRX_MAGIC || trx.len < sizeof(struct trx_header)) {
		dd_logerror("flash", "%s: Bad trx header", path);
		goto fail;
	}
	/* open partitions from mmc device */
	char *kernelname = getdisc();
	if (!kernelname) {
		dd_logerror("flash", "partition for system not found");
		goto fail;
	}

	eval("hdparm", "-W", "0", kernelname);
	eval("sdparm", "-s", "WCE", "-S", kernelname);
	eval("sdparm", "-c", "WCE", "-S", kernelname);

	FILE *f_kernel = fopen(kernelname, "r+b");
	if (!f_kernel) {
		dd_logerror("flash", "Error opening: %s", kernelname);
		goto fail;
	}

	fseek(f_kernel, 0, SEEK_END);
	size_t kernellen = ftello(f_kernel);
	rewind(f_kernel);

	if (STORE32_LE(trx.flag_version) & TRX_NO_HEADER)
		trx.len -= sizeof(struct trx_header);

	// #ifndef HAVE_WRK54G
	if (kernellen < trx.len) {
		dd_logerror("flash", "Image too big for partition: %s", mtd);
		perror(mtd);
		ret = -1;
		goto fail;
	}

	/* 
	 * See if we have enough memory to store the whole file 
	 */
	dd_loginfoverbose("flash", "freeram=[%ld] bufferram=[%ld]", info.freeram, info.bufferram);
	int mul = 1; // temporarily use 1 instead of 4 until we
#ifdef HAVE_IPQ6018
	#define MINEXTRA 64
#else
	#define MINEXTRA 8
#endif

	// found a a solution
	if (info.freeram >= (trx.len + MINEXTRA * 1024 * 1024) && brand != ROUTER_ASUS_AC58U) {
		dd_loginfoverbose("flash", "The free memory is enough, writing image once.");
		erase_info.length = trx.len;
	} else {
		erase_info.length = WRITE_BLOCKSIZE;
		dd_loginfoverbose("flash", "The free memory is not enough, writing image per %d bytes.", erase_info.length);
	}

	/* 
	 * Allocate temporary buffer 
	 */
	if (!(buf = malloc(erase_info.length))) {
		mul = 1;
		erase_info.length = WRITE_BLOCKSIZE;
		dd_loginfoverbose("flash", "The free memory is not enough, writing image per %d bytes.", erase_info.length);
		if (!(buf = malloc(erase_info.length))) {
			dd_logerror("flash", "memory allocation of %d bytes failed", erase_info.length);
			perror("malloc");
			goto fail;
		}
	}

	/* 
	 * Calculate CRC over header 
	 */
	crc = crc32((uint8 *)&trx.flag_version, sizeof(struct trx_header) - OFFSETOF(struct trx_header, flag_version),
		    CRC32_INIT_VALUE);
	crc_data = 0;
	int first = 0;
	/* 
	 * Write file or URL to MTD device 
	 */
	badblocks = 0;
	size_t pos = 0;
	FILE *f_write = f_kernel;
	for (erase_info.start = 0; erase_info.start < trx.len; erase_info.start += count) {
		len = MIN(erase_info.length, trx.len - erase_info.start);
		if ((STORE32_LE(trx.flag_version) & TRX_NO_HEADER) || erase_info.start)
			count = off = 0;
		else {
			count = off = sizeof(struct trx_header);
			memcpy(buf, &trx, sizeof(struct trx_header));
		}
		count += safe_fread(&buf[off], 1, len - off, fp);

		/* 
		 * for debug 
		 */
		sum = sum + count;

		if (((count < len) && (len - off) > (WRITE_BLOCKSIZE)) || (count == 0 && feof(fp))) {
			dd_logerror("flash", "%s: Truncated file (actual %d expect %d)", path, count - off, len - off);
			goto fail;
		}
		/* 
		 * Update CRC 
		 */
		crc = crc32(&buf[off], count - off, crc);

		if (!squashfound) {
			for (i = 0; i < (count - off); i++) {
				unsigned int *sq = (unsigned int *)&buf[off + i];

				if (*sq == SQUASHFS_MAGIC) {
					squashfound = 1;
					break;
				}
				crc_data += (unsigned char)buf[off + i];
			}
			data_len += i;
		}
		/* 
		 * Check CRC before writing if possible 
		 */
		if (sum == trx.len) {
			if (crc != trx.crc32) {
				dd_logerror("flash", "%s: Bad CRC (0x%08X expected, but 0x%08X calculated)", path, trx.crc32, crc);
				goto fail;
			} else {
				dd_loginfoverbose("flash", "%s: CRC OK (0x%08X)", mtd, crc);
				dd_loginfoverbose("flash", "Writing image to flash, waiting a moment...");
			}
			printf("\n");
		}

		erase_info.length = WRITE_BLOCKSIZE;

		int length = ROUNDUP(count, WRITE_BLOCKSIZE);
		int base = erase_info.start;
		for (i = 0; i < (length / WRITE_BLOCKSIZE); i++) {
			int redo = 0;
again:;
			dd_loginfoverbose("flash", "write block [%d] at [0x%08X]", (base + (i * WRITE_BLOCKSIZE)),
					  base + (i * WRITE_BLOCKSIZE) + badblocks);
			erase_info.start = base + (i * WRITE_BLOCKSIZE);
			memcpy(&tmp_erase_info, &erase_info, sizeof(erase_info));
			tmp_erase_info.start += badblocks;

			int l;
			for (l = 0; l < WRITE_BLOCKSIZE; l++) {
				unsigned char *p_buf = buf + (i * WRITE_BLOCKSIZE) + l;
				putc(*p_buf, f_write);
			}
		}
	}
	fsync(fileno(f_kernel));
	sync();

	fseek(f_kernel, 0, SEEK_SET);
	dd_loginfoverbose("flash", "reread kernel partition");
	for (i = 0; i < kernellen; i++)
		getc(f_kernel);
	fclose(f_kernel);

	dd_loginfoverbose("flash", "done [%d]", i * WRITE_BLOCKSIZE);
	/* 
	 * Netgear: Write len and checksum at the end of mtd1 
	 */
	int sector_start;
	char *tmp;

#if defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_IPQ6018)
	switch (brand) {
	case ROUTER_LINKSYS_EA8500:
	case ROUTER_LINKSYS_MR7350:
	case ROUTER_LINKSYS_MR7500:
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
	case ROUTER_LINKSYS_MX8500:
	case ROUTER_LINKSYS_MX5300:
	case ROUTER_LINKSYS_MR5500:
	case ROUTER_LINKSYS_MX5500:
		part = getUEnv("boot_part");
		if (part) {
			dd_loginfoverbose("flash", "boot partition is %s", part);
			if (!strcmp(part, "2")) {
				eval("fw_setenv", "boot_part", "1");
			} else {
				eval("fw_setenv", "boot_part", "2");
			}
		}

		break;
	case ROUTER_WRT_1900AC:
	case ROUTER_WRT_1200AC:
	case ROUTER_WRT_1900ACV2:
	case ROUTER_WRT_1900ACS:
	case ROUTER_WRT_3200ACM:
	case ROUTER_WRT_32X:
	case ROUTER_LINKSYS_EA8300:
		part = getUEnv("boot_part");
		if (part) {
			dd_loginfoverbose("flash", "boot partition is %s", part);
			if (!strcmp(part, "2")) {
				eval("ubootenv", "set", "boot_part", "1");
			} else {
				eval("ubootenv", "set", "boot_part", "2");
			}
		}
		break;
	}
#endif

	ret = 0;
	nvram_seti("flash_active", 0);
	if (buf) {
		free(buf);
	}

	if (fp)
		fclose(fp);
	return ret;

fail:
	nvram_seti("flash_active", 0);
	if (buf) {
		/* 
		 * Dummy read to ensure chip(s) are out of lock/suspend state 
		 */
		free(buf);
	}

	if (fp)
		fclose(fp);
	return -1;
}

/* 
 * Irving -  We need an unlock function in order to mount a r/w jffs2 partition
 * Unlock an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @return      0 on success and errno on failure
 */
static int mtd_unlock(const char *mtd)
{
	int mtd_fd;
	struct mtd_info_user mtd_info;
	struct erase_info_user lock_info;
	/* 
	 * Open MTD device 
	 */
	if ((mtd_fd = mtd_open(mtd, O_RDWR)) < 0) {
		perror(mtd);
		return errno;
	}

	/* 
	 * Get sector size 
	 */
	if (ioctl(mtd_fd, MEMGETINFO, &mtd_info) != 0) {
		perror(mtd);
		close(mtd_fd);
		return errno;
	}

	lock_info.start = 0;
	lock_info.length = mtd_info.size;
	if (ioctl(mtd_fd, MEMUNLOCK, &lock_info)) {
		dd_logerror("flash", "Could not unlock MTD device: %s", mtd);
		perror(mtd);
		close(mtd_fd);
		return errno;
	}

	close(mtd_fd);
	return 0;
}
