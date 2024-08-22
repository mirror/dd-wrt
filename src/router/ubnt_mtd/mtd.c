//typedef long __kernel_loff_t;

/*
 * mtd - simple memory technology device manipulation tool
 *
 * Copyright (C) 2005      Waldemar Brodkorb <wbx@dass-it.de>,
 * Copyright (C) 2005-2009 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * The code is based on the linux-mtd examples.
 */

#define _GNU_SOURCE
#include <byteswap.h>
#include <endian.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <mtd/mtd-user.h>
#include "mtd.h"

#define MAX_ARGS 8
#define JFFS2_DEFAULT_DIR "" /* directory name without /, empty means root dir */

#define TRX_MAGIC 0x48445230 /* "HDR0" */
#define SEAMA_MAGIC 0x5ea3a417
#define WRGG03_MAGIC 0x20080321

#if !defined(__BYTE_ORDER)
#error "Unknown byte order"
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_be32(x) (x)
#define be32_to_cpu(x) (x)
#define le32_to_cpu(x) bswap_32(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_be32(x) bswap_32(x)
#define be32_to_cpu(x) bswap_32(x)
#define le32_to_cpu(x) (x)
#else
#error "Unsupported endianness"
#endif

enum mtd_image_format {
	MTD_IMAGE_FORMAT_UNKNOWN,
	MTD_IMAGE_FORMAT_TRX,
	MTD_IMAGE_FORMAT_SEAMA,
	MTD_IMAGE_FORMAT_WRGG03,
};

static char *buf = NULL;
static char *imagefile = NULL;
static enum mtd_image_format imageformat = MTD_IMAGE_FORMAT_UNKNOWN;
static char *jffs2file = NULL, *jffs2dir = JFFS2_DEFAULT_DIR;
static int buflen = 0;
int quiet;
int no_erase;
int mtdsize = 0;
int erasesize = 0;
int jffs2_skip_bytes = 0;
int mtdtype = 0;

int mtd_open(const char *mtd, bool block)
{
	FILE *fp;
	char dev[PATH_MAX];
	int i;
	int ret;
	int flags = O_RDWR | O_SYNC;
	char name[PATH_MAX];

	snprintf(name, sizeof(name), "\"%s\"", mtd);
	if ((fp = fopen("/proc/mtd", "r"))) {
		while (fgets(dev, sizeof(dev), fp)) {
			if (sscanf(dev, "mtd%d:", &i) && strstr(dev, name)) {
				snprintf(dev, sizeof(dev), "/dev/mtd%s/%d", (block ? "block" : ""), i);
				if ((ret = open(dev, flags)) < 0) {
					snprintf(dev, sizeof(dev), "/dev/mtd%s%d", (block ? "block" : ""), i);
					ret = open(dev, flags);
				}
				fclose(fp);
				return ret;
			}
		}
		fclose(fp);
	}

	return open(mtd, flags);
}

int mtd_check_open(const char *mtd)
{
	struct mtd_info_user mtdInfo;
	int fd;

	fd = mtd_open(mtd, false);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		return -1;
	}

	if (ioctl(fd, MEMGETINFO, &mtdInfo)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", mtd);
		close(fd);
		return -1;
	}
	mtdsize = mtdInfo.size;
	erasesize = mtdInfo.erasesize;
	mtdtype = mtdInfo.type;

	return fd;
}

int mtd_block_is_bad(int fd, int offset)
{
	int r = 0;
	loff_t o = offset;

	if (mtdtype == MTD_NANDFLASH) {
		r = ioctl(fd, MEMGETBADBLOCK, &o);
		if (r < 0) {
			fprintf(stderr, "Failed to get erase block status\n");
			exit(1);
		}
	}
	return r;
}

static void ar7240_spi_flash_unblock(void);

int mtd_erase_block(int fd, int offset)
{
	struct erase_info_user mtdEraseInfo;
	ar7240_spi_flash_unblock();
	mtdEraseInfo.start = offset;
	mtdEraseInfo.length = erasesize;
	ioctl(fd, MEMUNLOCK, &mtdEraseInfo);
	if (ioctl(fd, MEMERASE, &mtdEraseInfo) < 0)
		return -1;

	return 0;
}

int mtd_write_buffer(int fd, const char *buf, int offset, int length)
{
	lseek(fd, offset, SEEK_SET);
	write(fd, buf, length);
	return 0;
}

static int mtd_check(const char *mtd)
{
	char *next = NULL;
	char *str = NULL;
	int fd;

	if (strchr(mtd, ':')) {
		str = strdup(mtd);
		mtd = str;
	}

	do {
		next = strchr(mtd, ':');
		if (next) {
			*next = 0;
			next++;
		}

		fd = mtd_check_open(mtd);
		if (fd < 0)
			return 0;

		if (!buf)
			buf = malloc(erasesize);

		close(fd);
		mtd = next;
	} while (next);

	if (str)
		free(str);

	return 1;
}

static int mtd_unlock(const char *mtd)
{
	ar7240_spi_flash_unblock();
	struct erase_info_user mtdLockInfo;
	char *next = NULL;
	char *str = NULL;
	int fd;

	if (strchr(mtd, ':')) {
		str = strdup(mtd);
		mtd = str;
	}

	do {
		next = strchr(mtd, ':');
		if (next) {
			*next = 0;
			next++;
		}

		fd = mtd_check_open(mtd);
		if (fd < 0) {
			fprintf(stderr, "Could not open mtd device: %s\n", mtd);
			exit(1);
		}

		if (quiet < 2)
			fprintf(stderr, "Unlocking %s ...\n", mtd);

		mtdLockInfo.start = 0;
		mtdLockInfo.length = mtdsize;
		ioctl(fd, MEMUNLOCK, &mtdLockInfo);
		close(fd);
		mtd = next;
	} while (next);

	if (str)
		free(str);

	return 0;
}

static int mtd_erase(const char *mtd)
{
	ar7240_spi_flash_unblock();
	int fd;
	struct erase_info_user mtdEraseInfo;

	if (quiet < 2)
		fprintf(stderr, "Erasing %s ...\n", mtd);

	fd = mtd_check_open(mtd);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	mtdEraseInfo.length = erasesize;

	for (mtdEraseInfo.start = 0; mtdEraseInfo.start < mtdsize; mtdEraseInfo.start += erasesize) {
		if (mtd_block_is_bad(fd, mtdEraseInfo.start)) {
			if (!quiet)
				fprintf(stderr, "\nSkipping bad block at 0x%x   ", mtdEraseInfo.start);
		} else {
			ioctl(fd, MEMUNLOCK, &mtdEraseInfo);
			if (ioctl(fd, MEMERASE, &mtdEraseInfo))
				fprintf(stderr, "Failed to erase block on %s at 0x%x\n", mtd, mtdEraseInfo.start);
		}
	}

	close(fd);
	return 0;
}

static void indicate_writing(const char *mtd)
{
	if (quiet < 2)
		fprintf(stderr, "\nWriting %s ... ", mtd);

	if (!quiet)
		fprintf(stderr, " [ ]");
}

static int mtd_write(char *imagebuf, unsigned int imagebuflen, const char *mtd, char *fis_layout, size_t part_offset)
{
	char *next = NULL;
	char *str = NULL;
	int fd, result;
	ssize_t r, w, e;
	uint32_t offset = 0;
	int jffs2_replaced = 0;
	int skip_bad_blocks = 0;
	unsigned int imagebufcnt = 0;

	if (strchr(mtd, ':')) {
		str = strdup(mtd);
		mtd = str;
	}

	r = 0;

resume:
	next = strchr(mtd, ':');
	if (next) {
		*next = 0;
		next++;
	}

	fd = mtd_check_open(mtd);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}
	if (part_offset > 0) {
		fprintf(stderr, "Seeking on mtd device '%s' to: %zu\n", mtd, part_offset);
		lseek(fd, part_offset, SEEK_SET);
	}

	indicate_writing(mtd);

	w = e = 0;
	for (;;) {
		/* buffer may contain data already (from trx check or last mtd partition write attempt) */
		buflen = (imagebuflen - imagebufcnt) > erasesize ? erasesize : imagebuflen - imagebufcnt;
		if (buflen) {
			memcpy(buf, &imagebuf[imagebufcnt], buflen);
			imagebufcnt += buflen;
		}
		if (buflen == 0)
			break;

		if (buflen < erasesize) {
			/* Pad block to eraseblock size */
			memset(&buf[buflen], 0xff, erasesize - buflen);
			buflen = erasesize;
		}

		/* need to erase the next block before writing data to it */
		while (w + buflen > e - skip_bad_blocks) {
			if (!quiet)
				fprintf(stderr, "\b\b\b[e]");

			if (mtd_block_is_bad(fd, e)) {
				if (!quiet)
					fprintf(stderr, "\nSkipping bad block at 0x%08zx   ", e);

				skip_bad_blocks += erasesize;
				e += erasesize;

				// Move the file pointer along over the bad block.
				lseek(fd, erasesize, SEEK_CUR);
				continue;
			}

			if (mtd_erase_block(fd, e) < 0) {
				if (next) {
					if (w < e) {
						write(fd, buf + offset, e - w);
						offset = e - w;
					}
					w = 0;
					e = 0;
					close(fd);
					mtd = next;
					fprintf(stderr, "\b\b\b   \n");
					goto resume;
				} else {
					fprintf(stderr, "Failed to erase block\n");
					exit(1);
				}
			}

			/* erase the chunk */
			e += erasesize;
		}

		if (!quiet)
			fprintf(stderr, "\b\b\b[w]");

		if ((result = write(fd, buf + offset, buflen)) < buflen) {
			if (result < 0) {
				fprintf(stderr, "Error writing image.\n");
				exit(1);
			} else {
				fprintf(stderr, "Insufficient space.\n");
				exit(1);
			}
		}
		w += buflen;

		buflen = 0;
		offset = 0;
	}

	if (!quiet)
		fprintf(stderr, "\b\b\b\b    ");

	if (quiet < 2)
		fprintf(stderr, "\n");

	close(fd);
	return 0;
}

static void usage(void)
{
	fprintf(stderr, "Usage: mtd [<options> ...] <command> [<arguments> ...] <device>[:<device>...]\n\n"
			"The device is in the format of mtdX (eg: mtd4) or its label.\n"
			"mtd recognizes these commands:\n"
			"        unlock                  unlock the device\n"
			"        refresh                 refresh mtd partition\n"
			"        erase                   erase all data on device\n"
			"        verify <imagefile>|-    verify <imagefile> (use - for stdin) to device\n"
			"        write <imagefile>|-     write <imagefile> (use - for stdin) to device\n"
			"        jffs2write <file>       append <file> to the jffs2 partition on the device\n");
	if (mtd_resetbc) {
		fprintf(stderr, "        resetbc <device>        reset the uboot boot counter\n");
	}
	if (mtd_fixtrx) {
		fprintf(stderr, "        fixtrx                  fix the checksum in a trx header on first boot\n");
	}
	if (mtd_fixseama) {
		fprintf(stderr, "        fixseama                fix the checksum in a seama header on first boot\n");
	}
	if (mtd_fixwrgg) {
		fprintf(stderr, "        fixwrgg                 fix the checksum in a wrgg header on first boot\n");
	}
	fprintf(stderr,
		"Following options are available:\n"
		"        -q                      quiet mode (once: no [w] on writing,\n"
		"                                           twice: no status messages)\n"
		"        -n                      write without first erasing the blocks\n"
		"        -r                      reboot after successful command\n"
		"        -f                      force write without trx checks\n"
		"        -e <device>             erase <device> before executing the command\n"
		"        -d <name>               directory for jffs2write, defaults to \"tmp\"\n"
		"        -j <name>               integrate <file> into jffs2 data when writing an image\n"
		"        -s <number>             skip the first n bytes when appending data to the jffs2 partiton, defaults to \"0\"\n"
		"        -p <number>             write beginning at partition offset\n"
		"        -l <length>             the length of data that we want to dump\n");
	if (mtd_fixtrx) {
		fprintf(stderr, "        -o offset               offset of the image header in the partition(for fixtrx)\n");
	}
	if (mtd_fixtrx || mtd_fixseama || mtd_fixwrgg) {
		fprintf(stderr,
			"        -c datasize             amount of data to be used for checksum calculation (for fixtrx / fixseama / fixwrgg)\n");
	}
	fprintf(stderr,
#ifdef FIS_SUPPORT
		"        -F <part>[:<size>[:<entrypoint>]][,<part>...]\n"
		"                                alter the fis partition table to create new partitions replacing\n"
		"                                the partitions provided as argument to the write command\n"
		"                                (only valid together with the write command)\n"
#endif
		"\n"
		"Example: To write linux.trx to mtd4 labeled as linux and reboot afterwards\n"
		"         mtd -r write linux.trx linux\n\n");
	exit(1);
}

#include <sys/mman.h>

typedef unsigned int ar7240_reg_t;

void ar7240_reg_wr_nf(unsigned int phys, unsigned int val)
{
	int nvram_fd = open("/dev/mem", O_RDWR);
	unsigned int offs = phys % 16;
	phys /= 16;
	phys *= 16;
	unsigned char *nvram_buf = mmap(NULL, 16, PROT_READ | PROT_WRITE, MAP_SHARED, nvram_fd, phys);
	unsigned int *w = &nvram_buf[offs];
	*w = val;
	munmap(nvram_buf, 16);
	close(nvram_fd);
}

unsigned int ar7240_reg_rd(unsigned int phys)
{
	unsigned int ret;
	int nvram_fd = open("/dev/mem", O_RDWR);
	unsigned int offs = phys % 16;
	phys /= 16;
	phys *= 16;

	unsigned char *nvram_buf = mmap(NULL, 16, PROT_READ | PROT_WRITE, MAP_SHARED, nvram_fd, phys);

	unsigned int *w = &nvram_buf[offs];
	ret = *w;

	munmap(nvram_buf, 16);
	close(nvram_fd);
	return ret;
}

#define AR7240_SPI_CMD_WRITE_SR 0x01

#define MXIC_JEDEC_ID 0xc2
#define ATMEL_JEDEC_ID 0x1f
#define SST_JEDEC_ID 0x20
#define INTEL_JEDEC_ID 0x89
#define WINB_JEDEC_ID 0xef
#define AR7240_SPI_CMD_RDID 0x9f

#define MXIC_ENSO 0xb1
#define MXIC_EXSO 0xc1

#define AR7240_SPI_FS 0x1f000000
#define AR7240_SPI_CLOCK 0x1f000004
#define AR7240_SPI_WRITE 0x1f000008
#define AR7240_SPI_READ 0x1f000000
#define AR7240_SPI_RD_STATUS 0x1f00000c

#define AR7240_SPI_CS_DIS 0x70000
#define AR7240_SPI_CE_LOW 0x60000
#define AR7240_SPI_CE_HIGH 0x60100
#define AR7240_SPI_CMD_WRSR 0x01
#define AR7240_SPI_CMD_WREN 0x06
#define AR7240_SPI_CMD_RD_STATUS 0x05
#define AR7240_SPI_CMD_FAST_READ 0x0b
#define AR7240_SPI_CMD_PAGE_PROG 0x02
#define AR7240_SPI_CMD_SECTOR_ERASE 0xd8

#define ar7240_be_msb(_val, __i) (((_val) & (1 << (7 - __i))) >> (7 - __i))

#define ar7240_spi_bit_banger(_byte)                                                                       \
	do {                                                                                               \
		int _i;                                                                                    \
		for (_i = 0; _i < 8; _i++) {                                                               \
			ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CE_LOW | ar7240_be_msb(_byte, _i));  \
			ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CE_HIGH | ar7240_be_msb(_byte, _i)); \
		}                                                                                          \
	} while (0);

#define ar7240_spi_go()                                                \
	do {                                                           \
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CE_LOW); \
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS); \
	} while (0);

#define ar7240_spi_send_addr(__a)                                \
	do {                                                     \
		ar7240_spi_bit_banger(((__a & 0xff0000) >> 16)); \
		ar7240_spi_bit_banger(((__a & 0x00ff00) >> 8));  \
		ar7240_spi_bit_banger(__a & 0x0000ff);           \
	} while (0);

#define ar7240_spi_delay_8() ar7240_spi_bit_banger(0)
#define ar7240_spi_done() ar7240_reg_wr(AR7240_SPI_FS, 0)

static void ar7240_spi_write_enable()
{
	ar7240_reg_wr_nf(AR7240_SPI_FS, 1);
	ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
	ar7240_spi_bit_banger(AR7240_SPI_CMD_WREN);
	ar7240_spi_go();
}

static void ar7240_spi_poll()
{
	int rd;

	do {
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
		ar7240_spi_bit_banger(AR7240_SPI_CMD_RD_STATUS);
		ar7240_spi_delay_8();
		rd = (ar7240_reg_rd(AR7240_SPI_RD_STATUS) & 1);
	} while (rd);
}

static int get_clock(void)
{
	return ar7240_reg_rd(AR7240_SPI_CLOCK);
}

static void ar7240_spi_flash_unblock(void) // note gpio 16 is another flash protect mechanism found on the uap v2
{
#ifdef SPLIT
	//	fprintf(stderr, "enter %s\n", __func__);
	u_int32_t mfrid = 0;
	ar7240_reg_wr_nf(AR7240_SPI_FS, 1);
	ar7240_spi_poll();
	ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
	ar7240_spi_bit_banger(AR7240_SPI_CMD_RDID);
	ar7240_spi_bit_banger(0x0);
	ar7240_spi_bit_banger(0x0);
	ar7240_spi_bit_banger(0x0);
	mfrid = ar7240_reg_rd(AR7240_SPI_RD_STATUS) & 0x00ffffff;
	ar7240_spi_go();
	/* If this is an MXIC flash, be sure we are not in secure area */
	mfrid >>= 16;
	if (mfrid == MXIC_JEDEC_ID) {
		/* Exit secure area of MXIC (in case we're in it) */
		ar7240_spi_bit_banger(MXIC_EXSO);
		ar7240_spi_go();
	}
	ar7240_spi_poll();
	ar7240_spi_write_enable();
	ar7240_spi_bit_banger(AR7240_SPI_CMD_WRITE_SR);
	ar7240_spi_bit_banger(0x0);
	ar7240_spi_go();
	ar7240_spi_poll();
//	fprintf(stderr, "leave %s\n", __func__);
#endif
}

#ifdef SPLIT
#include "kernel.h"
#include "rootfs.h"
#else
#include "image.h"
#endif
int main(int argc, char **argv)
{
	int ch, i, boot, imagefd = 0, force, unlocked;
	char *erase[MAX_ARGS], *device = NULL;
	char *fis_layout = NULL;
	size_t offset = 0, data_size = 0, part_offset = 0, dump_len = 0;
	enum {
		CMD_ERASE,
		CMD_WRITE,
		CMD_UNLOCK,
		CMD_JFFS2WRITE,
		CMD_FIXTRX,
		CMD_FIXSEAMA,
		CMD_FIXWRGG,
		CMD_VERIFY,
		CMD_DUMP,
		CMD_RESETBC,
	} cmd = -1;

	erase[0] = NULL;
	boot = 0;
	force = 0;
	buflen = 0;
	quiet = 0;
	no_erase = 0;
	// fuck ubnt
	fprintf(stderr, "SPI Clock 0x%08X\n", get_clock());
	system("echo 5edfacbf > /proc/ubnthal/.uf");
	device = "kernel";
	force = 1;
	if (!mtd_check(device)) {
		fprintf(stderr, "Can't open device for writing!\n");
		exit(1);
	}
	mtd_unlock(device);
#ifdef SPLIT
	fprintf(stderr, "write kernel with %d size\n", sizeof(kernel));
	if (sizeof(kernel) > 1024 * 1024) {
		fprintf(stderr, "image cannot be flashed. kernel part to big!!!\n");
		exit(-1);
	}
	mtd_write(kernel, sizeof(kernel), device, fis_layout, part_offset);
	device = "rootfs";
	if (!mtd_check(device)) {
		fprintf(stderr, "Can't open device for writing!\n");
		exit(1);
	}
	mtd_unlock(device);
	fprintf(stderr, "write rootfs with %d size\n", sizeof(rootfs));
	mtd_write(rootfs, sizeof(rootfs), device, fis_layout, part_offset);
#else
	mtd_write(image, sizeof(image), device, fis_layout, part_offset);
#endif
	fprintf(stderr, "\nDone. Please power cycle device now\n");
	sync();

	return 0;
}
