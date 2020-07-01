// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2019 Namjae Jeon <linkinjeon@kernel.org>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>
#include <limits.h>

#include "exfat_ondisk.h"
#include "libexfat.h"
#include "version.h"

#ifdef WORDS_BIGENDIAN
#define BITOP_LE_SWIZZLE	(~0x7)
#else
#define BITOP_LE_SWIZZLE        0
#endif

#define BIT_MASK(nr)            ((1) << ((nr) % 32))
#define BIT_WORD(nr)            ((nr) / 32)

unsigned int print_level  = EXFAT_INFO;

static inline void set_bit(int nr, unsigned int *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p  |= mask;
}

static inline void clear_bit(int nr, unsigned int *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p &= ~mask;
}

static inline void set_bit_le(int nr, void *addr)
{
	set_bit(nr ^ BITOP_LE_SWIZZLE, addr);
}

static inline void clear_bit_le(int nr, void *addr)
{
	clear_bit(nr ^ BITOP_LE_SWIZZLE, addr);
}

void exfat_set_bit(struct exfat_blk_dev *bd, char *bitmap,
		unsigned int clu)
{
	int b;

	b = clu & ((bd->sector_size << 3) - 1);

	set_bit_le(b, bitmap);
}

void exfat_clear_bit(struct exfat_blk_dev *bd, char *bitmap,
		unsigned int clu)
{
	int b;

	b = clu & ((bd->sector_size << 3) - 1);

	clear_bit_le(b, bitmap);
}

wchar_t exfat_bad_char(wchar_t w)
{
	return (w < 0x0020)
		|| (w == '*') || (w == '?') || (w == '<') || (w == '>')
		|| (w == '|') || (w == '"') || (w == ':') || (w == '/')
		|| (w == '\\');
}

void boot_calc_checksum(unsigned char *sector, unsigned short size,
		bool is_boot_sec, __le32 *checksum)
{
	unsigned int index;

	if (is_boot_sec) {
		for (index = 0; index < size; index++) {
			if ((index == 106) || (index == 107) || (index == 112))
				continue;
			*checksum = ((*checksum & 1) ? 0x80000000 : 0) +
				(*checksum >> 1) + sector[index];
		}
	} else {
		for (index = 0; index < size; index++) {
			*checksum = ((*checksum & 1) ? 0x80000000 : 0) +
				(*checksum >> 1) + sector[index];
		}
	}
}

void show_version(void)
{
	printf("exfatprogs version : %s\n", EXFAT_PROGS_VERSION);
}

static inline unsigned int sector_size_bits(unsigned int size)
{
	unsigned int bits = 8;

	do {
		bits++;
		size >>= 1;
	} while (size > 256);

	return bits;
}

static void exfat_set_default_cluster_size(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui)
{
	if (256 * MB >= bd->size)
		ui->cluster_size = 4 * KB;
	else if (32 * GB >= bd->size)
		ui->cluster_size = 32 * KB;
	else
		ui->cluster_size = 128 * KB;
}

void init_user_input(struct exfat_user_input *ui)
{
	memset(ui, 0, sizeof(struct exfat_user_input));
	ui->writeable = true;
	ui->quick = true;
}

int exfat_get_blk_dev_info(struct exfat_user_input *ui,
		struct exfat_blk_dev *bd)
{
	int fd, ret = -1;
	off_t blk_dev_size;

	fd = open(ui->dev_name, ui->writeable ? O_RDWR|O_EXCL : O_RDONLY);
	if (fd < 0) {
		exfat_err("open failed : %s, %s\n", ui->dev_name,
			strerror(errno));
		return -1;
	}
	blk_dev_size = lseek(fd, 0, SEEK_END);
	if (blk_dev_size <= 0) {
		exfat_err("invalid block device size(%s)\n",
			ui->dev_name);
		ret = blk_dev_size;
		close(fd);
		goto out;
	}

	bd->dev_fd = fd;
	bd->size = blk_dev_size;
	if (!ui->cluster_size)
		exfat_set_default_cluster_size(bd, ui);

	if (ioctl(fd, BLKSSZGET, &bd->sector_size) < 0)
		bd->sector_size = DEFAULT_SECTOR_SIZE;
	bd->sector_size_bits = sector_size_bits(bd->sector_size);
	bd->num_sectors = blk_dev_size / DEFAULT_SECTOR_SIZE;
	bd->num_clusters = blk_dev_size / ui->cluster_size;

	exfat_debug("Block device name : %s\n", ui->dev_name);
	exfat_debug("Block device size : %lld\n", bd->size);
	exfat_debug("Block sector size : %u\n", bd->sector_size);
	exfat_debug("Number of the sectors : %llu\n",
		bd->num_sectors);
	exfat_debug("Number of the clusters : %u\n",
		bd->num_clusters);

	ret = 0;
	bd->dev_fd = fd;
out:
	return ret;
}

ssize_t exfat_read(int fd, void *buf, size_t size, off_t offset)
{
	return pread(fd, buf, size, offset);
}

ssize_t exfat_write(int fd, void *buf, size_t size, off_t offset)
{
	return pwrite(fd, buf, size, offset);
}

ssize_t exfat_utf16_enc(const char *in_str, __u16 *out_str, size_t out_size)
{
	size_t mbs_len, out_len, i;
	wchar_t *wcs;

	mbs_len = mbstowcs(NULL, in_str, 0);
	if (mbs_len == (size_t)-1) {
		if (errno == EINVAL || errno == EILSEQ)
			exfat_err("invalid character sequence in current locale\n");
		return -errno;
	}

	wcs = calloc(mbs_len+1, sizeof(wchar_t));
	if (!wcs)
		return -ENOMEM;

	/* First convert multibyte char* string to wchar_t* string */
	if (mbstowcs(wcs, in_str, mbs_len+1) == (size_t)-1) {
		if (errno == EINVAL || errno == EILSEQ)
			exfat_err("invalid character sequence in current locale\n");
		return -errno;
	}

	/* Convert wchar_t* string (sequence of code points) to UTF-16 string */
	for (i = 0, out_len = 0; i < mbs_len; i++) {
		if (2*(out_len+1) > out_size ||
		    (wcs[i] >= 0x10000 && 2*(out_len+2) > out_size)) {
			exfat_err("input string is too long\n");
			free(wcs);
			return -E2BIG;
		}

		/* Encode code point above Plane0 as UTF-16 surrogate pair */
		if (wcs[i] >= 0x10000) {
			out_str[out_len++] =
			  cpu_to_le16(((wcs[i] - 0x10000) >> 10) + 0xD800);
			wcs[i] = ((wcs[i] - 0x10000) & 0x3FF) + 0xDC00;
		}

		out_str[out_len++] = cpu_to_le16(wcs[i]);
	}

	free(wcs);
	return 2*out_len;
}

ssize_t exfat_utf16_dec(const __u16 *in_str, size_t in_len,
			char *out_str, size_t out_size)
{
	size_t wcs_len, out_len, c_len, i;
	char c_str[MB_LEN_MAX];
	wchar_t *wcs;
	mbstate_t ps;
	wchar_t w;

	wcs = calloc(in_len/2+1, sizeof(wchar_t));
	if (!wcs)
		return -ENOMEM;

	/* First convert UTF-16 string to wchar_t* string */
	for (i = 0, wcs_len = 0; i < in_len/2; i++, wcs_len++) {
		wcs[wcs_len] = le16_to_cpu(in_str[i]);
		/*
		 * If wchar_t can store code point above Plane0
		 * then unpack UTF-16 surrogate pair to code point
		 */
#if WCHAR_MAX >= 0x10FFFF
		if (wcs[wcs_len] >= 0xD800 && wcs[wcs_len] <= 0xDBFF &&
		    i+1 < in_len/2) {
			w = le16_to_cpu(in_str[i+1]);
			if (w >= 0xDC00 && w <= 0xDFFF) {
				wcs[wcs_len] = 0x10000 +
					       ((wcs[wcs_len] - 0xD800) << 10) +
					       (w - 0xDC00);
				i++;
			}
		}
#endif
	}

	memset(&ps, 0, sizeof(ps));

	/* And then convert wchar_t* string to multibyte char* string */
	for (i = 0, out_len = 0, c_len = 0; i < wcs_len; i++) {
		c_len = wcrtomb(c_str, wcs[i], &ps);
		/*
		 * If character is non-representable in current locale then
		 * try to store it as Unicode replacement code point U+FFFD
		 */
		if (c_len == (size_t)-1 && errno == EILSEQ)
			c_len = wcrtomb(c_str, 0xFFFD, &ps);
		/* If U+FFFD is also non-representable, try question mark */
		if (c_len == (size_t)-1 && errno == EILSEQ)
			c_len = wcrtomb(c_str, L'?', &ps);
		/* If also (7bit) question mark fails then we cannot do more */
		if (c_len == (size_t)-1) {
			exfat_err("invalid UTF-16 sequence\n");
			free(wcs);
			return -errno;
		}
		if (out_len+c_len > out_size) {
			exfat_err("input string is too long\n");
			free(wcs);
			return -E2BIG;
		}
		memcpy(out_str+out_len, c_str, c_len);
		out_len += c_len;
	}

	free(wcs);

	/* Last iteration of above loop should have produced null byte */
	if (c_len == 0 || out_str[out_len] != 0) {
		exfat_err("invalid UTF-16 sequence\n");
		return -errno;
	}

	return out_len-1;
}
