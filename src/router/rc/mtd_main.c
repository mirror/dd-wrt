/* 
 * mtd - simple memory technology device manipulation tool
 *
 * Copyright (C) 2005 Waldemar Brodkorb <wbx@dass-it.de>,
 *                        Felix Fietkau <nbd@openwrt.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: mtd.c 3991 2006-06-18 18:03:31Z nico $
 *
 * The code is based on the linux-mtd examples.
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
#include <shutils.h>

#define TRX_MAGIC 0x30524448 /* "HDR0" */
#define BUFSIZE (16 * 1024)
#define MAX_ARGS 8

#define SYSTYPE_UNKNOWN 0
#define SYSTYPE_BROADCOM 1
/* 
 * to be continued 
 */
static int mtdtype;

struct trx_header2 {
	uint32_t magic; /* "HDR0" */
	uint32_t len; /* Length of file including header */
	uint32_t crc32; /* 32-bit CRC from flag_version to end of
				 * file */
	uint32_t flag_version; /* 0:15 flags, 16:31 version */
	uint32_t offsets[3]; /* Offsets of partitions from start of header 
				 */
};

char buf[BUFSIZE];
int buflen = 0;

static int image_check_bcom(FILE *imagefp, const char *mtd)
{
	struct trx_header2 *trx = (struct trx_header2 *)buf;
	struct mtd_info_user mtdInfo;
	int fd;

	buflen = safe_fread(buf, 1, 32, imagefp);
	if (buflen < 32) {
		fprintf(stdout,
			"Could not get image header, file too small (%d bytes)\n",
			buflen);
		return 0;
	}

	switch (trx->magic) {
	case 0x47343557: /* W54G */
	case 0x53343557: /* W54S */
	case 0x73343557: /* W54s */
	case 0x46343557: /* W54F */
	case 0x55343557: /* W54U */
		/* 
		 * ignore the first 32 bytes 
		 */
		buflen =
			safe_fread(buf, 1, sizeof(struct trx_header2), imagefp);
		break;
	}

	if (trx->magic != TRX_MAGIC || trx->len < sizeof(struct trx_header2)) {
		fprintf(stderr, "Bad trx header\n");
		fprintf(stderr,
			"If this is a firmware in bin format, like some of the\n"
			"original firmware files are, use following command to convert to trx:\n"
			"dd if=firmware.bin of=firmware.trx bs=32 skip=1\n");
		return 0;
	}

	/* 
	 * check if image fits to mtd device 
	 */
	fd = mtd_open(mtd, O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	if (ioctl(fd, MEMGETINFO, &mtdInfo)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", mtd);
		exit(1);
	}

	if (mtdInfo.size < trx->len) {
		fprintf(stderr, "Image too big for partition: %s\n", mtd);
		close(fd);
		return 0;
	}

	close(fd);
	return 1;
}

static int image_check(FILE *imagefp, const char *mtd)
{
	int fd, systype;
	size_t count;
	char *c;
	FILE *f;

	systype = SYSTYPE_UNKNOWN;
	f = fopen("/proc/cpuinfo", "r");
	while (f && !feof(f) && (fgets(buf, BUFSIZE - 1, f) != NULL)) {
		if ((strncmp(buf, "system type", 11) == 0) &&
		    (c = strchr(buf, ':'))) {
			c += 2;
			if (strncmp(c, "Broadcom BCM947XX", 17) == 0)
				systype = SYSTYPE_BROADCOM;
		}
	}
	fclose(f);

	switch (systype) {
	case SYSTYPE_BROADCOM:
		return image_check_bcom(imagefp, mtd);
	default:
		return 1;
	}
}

static int mtd_check(char *mtd)
{
	struct mtd_info_user mtdInfo;
	int fd;

	fd = mtd_open(mtd, O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		return 0;
	}

	if (ioctl(fd, MEMGETINFO, &mtdInfo)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", mtd);
		close(fd);
		return 0;
	}
	mtdtype = mtdInfo.type;

	close(fd);
	return 1;
}

static int s_mtd_write(FILE *imagefp, const char *mtd, int quiet)
{
	int fd, i, result;
	size_t r, w, e, skip_bad_blocks = 0;
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdEraseInfo;
	int ret = 0;

	fd = mtd_open(mtd, O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	if (ioctl(fd, MEMGETINFO, &mtdInfo)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", mtd);
		close(fd);
		exit(1);
	}
	char *writebuf = malloc(mtdInfo.erasesize);
	r = w = e = 0;
	if (!quiet)
		fprintf(stderr, " [ ]");

	for (;;) {
		/* 
		 * buffer may contain data already (from trx check) 
		 */
		r = buflen;
		r += safe_fread(writebuf + buflen, 1,
				mtdInfo.erasesize - buflen, imagefp);
		w += r;

		/* 
		 * EOF 
		 */
		if (r <= 0)
			break;

		/* 
		 * need to erase the next block before writing data to it 
		 */
		while (w > e - skip_bad_blocks) {
			mtdEraseInfo.start = e;
			mtdEraseInfo.length = mtdInfo.erasesize;

			if (!quiet)
				fprintf(stderr, "\b\b\b[e]");
			/* 
			 * erase the chunk 
			 */
			if (mtd_block_is_bad(fd, mtdEraseInfo.start)) {
				if (!quiet)
					fprintf(stderr,
						"\nSkipping bad block at 0x%08zx   ",
						e);
				skip_bad_blocks += mtdInfo.erasesize;
				e += mtdInfo.erasesize;
				// Move the file pointer along over the bad block.
				lseek(fd, mtdInfo.erasesize, SEEK_CUR);
				continue;
			}

			if (ioctl(fd, MEMERASE, &mtdEraseInfo) < 0) {
				fprintf(stderr, "Erasing mtd failed: %s\n",
					mtd);
				exit(1);
			}
			e += mtdInfo.erasesize;
		}

		if (!quiet)
			fprintf(stderr, "\b\b\b[w]");
		if (r < mtdInfo.erasesize) {
			fprintf(stderr,
				"\nWarning unaligned data, we use manual padding to avoid errors. size was %d!!!\n",
				r);
		}
		if ((result = write(fd, writebuf, mtdInfo.erasesize)) < r) {
			if (result < 0) {
				fprintf(stderr, "Error writing image.\n");
				exit(1);
			} else {
				fprintf(stderr, "Insufficient space.\n");
				exit(1);
			}
		}
		buflen = 0;
	}
	if (!quiet)
		fprintf(stderr, "\b\b\b\b");

	close(fd);
	free(writebuf);
	return 0;
}

#define BOOTCOUNT_MAGIC 0x20110811

struct bootcounter {
	uint32_t magic;
	uint32_t count;
	uint32_t checksum;
};

static char page[2048];

static int mtd_resetbc(char *mtd)
{
	struct mtd_info_user mtd_info;
	struct bootcounter *curr = (struct bootcounter *)page;
	unsigned int i;
	int last_count = 0;
	int num_bc;
	int fd;
	int ret;

	if (!mtd_check(mtd))
		return -1;

	fd = mtd_open(mtd, O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "error while opening mtd\n");
		return -1;
	}

	if (ioctl(fd, MEMGETINFO, &mtd_info) < 0) {
		fprintf(stderr, "failed to get mtd info!\n");
		return -1;
	}
	//      num_bc = mtd_info.size / mtd_info.writesize;
	num_bc = mtd_info.size / mtd_info.oobblock;

	for (i = 0; i < num_bc; i++) {
		pread(fd, curr, sizeof(*curr), i * mtd_info.oobblock);

		if (curr->magic != BOOTCOUNT_MAGIC &&
		    curr->magic != 0xffffffff) {
			fprintf(stderr, "unexpected magic %08x, bailing out\n",
				curr->magic);
			goto out;
		}

		if (curr->magic == 0xffffffff)
			break;

		last_count = curr->count;
	}

	/* no need to do writes when last boot count is already 0 */
	if (last_count == 0) {
		fprintf(stderr,
			"count is already zero, no need to call again\n");
		goto out;
	}
	fprintf(stderr, "reset boot counter\n");
	if (i == num_bc) {
		struct erase_info_user erase_info;
		erase_info.start = 0;
		erase_info.length = mtd_info.size;

		/* erase block */
		ret = ioctl(fd, MEMERASE, &erase_info);
		if (ret < 0) {
			fprintf(stderr, "failed to erase block: %i\n", ret);
			return -1;
		}

		i = 0;
	}

	memset(curr, 0xff, mtd_info.oobblock);

	curr->magic = BOOTCOUNT_MAGIC;
	curr->count = 0;
	curr->checksum = BOOTCOUNT_MAGIC;

	ret = pwrite(fd, curr, mtd_info.oobblock, i * mtd_info.oobblock);
	if (ret < 0)
		fprintf(stderr, "failed to write: %i\n", ret);
	sync();
out:
	close(fd);

	return 0;
}

static void usage(void)
{
	fprintf(stderr,
		"Usage: mtd [<options> ...] <command> [<arguments> ...] <device>\n\n"
		"The device is in the format of mtdX (eg: mtd4) or its label.\n"
		"mtd recognizes these commands:\n"
		"        unlock                  unlock the device\n"
		"        resetbc                 reset bootcounter for WRT1900AC/WRT1200AC/WRT1900ACv2\n"
		"        erase                   erase all data on device\n"
		"        write <imagefile>|-     write <imagefile> (use - for stdin) to device\n"
		"Following options are available:\n"
		"        -q                      quiet mode (once: no [w] on writing,\n"
		"                                           twice: no status messages)\n"
		"        -r                      reboot after successful command\n"
		"        -f                      force write without trx checks\n"
		"        -e <device>             erase <device> before executing the command\n\n"
		"Example: To write linux.trx to mtd4 labeled as linux and reboot afterwards\n"
		"         mtd -r write linux.trx linux\n\n");
	exit(1);
}

static int mtd_main(int argc, char **argv)
{
	int ch, i, boot, unlock, force, quiet, unlocked;
	char *erase[MAX_ARGS], *device, *imagefile = NULL;
	FILE *imagefp;
	enum { CMD_ERASE, CMD_WRITE, CMD_UNLOCK } cmd;

	erase[0] = NULL;
	boot = 0;
	force = 0;
	buflen = 0;
	quiet = 0;

	while ((ch = getopt(argc, argv, "frqe:")) != -1)
		switch (ch) {
		case 'f':
			force = 1;
			break;
		case 'r':
			boot = 1;
			break;
		case 'q':
			quiet++;
			break;
		case 'e':
			i = 0;
			while ((erase[i] != NULL) && ((i + 1) < MAX_ARGS))
				i++;

			erase[i++] = optarg;
			erase[i] = NULL;
			break;

		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	if ((strcmp(argv[0], "unlock") == 0) && (argc == 2)) {
		cmd = CMD_UNLOCK;
		device = argv[1];
	} else if ((strcmp(argv[0], "erase") == 0) && (argc == 2)) {
		cmd = CMD_ERASE;
		device = argv[1];
	} else if ((strcmp(argv[0], "resetbc") == 0) && (argc == 2)) {
		mtd_resetbc(argv[1]);
		exit(0);
	} else if ((strcmp(argv[0], "write") == 0) && (argc == 3)) {
		cmd = CMD_WRITE;
		device = argv[2];

		if (strcmp(argv[1], "-") == 0) {
			imagefile = "<stdin>";
			imagefp = stdin;
		} else {
			imagefile = argv[1];
			if ((imagefp = fopen(argv[1], "rb")) < 0) {
				fprintf(stderr,
					"Couldn't open image file: %s!\n",
					imagefile);
				exit(1);
			}
		}

		/* 
		 * check trx file before erasing or writing anything 
		 */
		if (!image_check(imagefp, device)) {
			if ((quiet < 2) || !force)
				fprintf(stderr, "TRX check failed!\n");
			if (!force)
				exit(1);
		} else {
			if (!mtd_check(device)) {
				fprintf(stderr,
					"Can't open device for writing!\n");
				exit(1);
			}
		}
	} else {
		usage();
	}

	sync();

	i = 0;
	unlocked = 0;
	while (erase[i] != NULL) {
		if (quiet < 2)
			fprintf(stderr, "Unlocking %s ...\n", erase[i]);
		mtd_unlock(erase[i]);
		if (quiet < 2)
			fprintf(stderr, "Erasing %s ...\n", erase[i]);
		mtd_erase(erase[i]);
		if (strcmp(erase[i], device) == 0)
			unlocked = 1;
		i++;
	}

	if (!unlocked) {
		if (quiet < 2)
			fprintf(stderr, "Unlocking %s ...\n", device);
		mtd_unlock(device);
	}

	switch (cmd) {
	case CMD_UNLOCK:
		break;
	case CMD_ERASE:
		if (quiet < 2)
			fprintf(stderr, "Erasing %s ...\n", device);
		mtd_erase(device);
		break;
	case CMD_WRITE:
		if (quiet < 2)
			fprintf(stderr, "Writing from %s to %s ... ", imagefile,
				device);
		s_mtd_write(imagefp, device, quiet);
		fclose(imagefp);
		if (quiet < 2)
			fprintf(stderr, "\n");
		break;
	}

	sync();

	if (boot)
		kill(1, 15); // send SIGTERM to init for reboot

	return 0;
}
