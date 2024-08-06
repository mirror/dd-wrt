// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2019 Namjae Jeon <linkinjeon@kernel.org>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>
#include <limits.h>
#include <assert.h>

#include "exfat_ondisk.h"
#include "libexfat.h"
#include "version.h"
#include "exfat_fs.h"
#include "exfat_dir.h"

unsigned int print_level  = EXFAT_INFO;

void exfat_bitmap_set_range(struct exfat *exfat, char *bitmap,
			    clus_t start_clus, clus_t count)
{
	clus_t clus;

	if (!exfat_heap_clus(exfat, start_clus) ||
	    !exfat_heap_clus(exfat, start_clus + count - 1))
		return;

	clus = start_clus;
	while (clus < start_clus + count) {
		exfat_bitmap_set(bitmap, clus);
		clus++;
	}
}

static int exfat_bitmap_find_bit(struct exfat *exfat, char *bmap,
				 clus_t start_clu, clus_t *next,
				 int bit)
{
	clus_t last_clu;

	last_clu = le32_to_cpu(exfat->bs->bsx.clu_count) +
		EXFAT_FIRST_CLUSTER;
	while (start_clu < last_clu) {
		if (!!exfat_bitmap_get(bmap, start_clu) == bit) {
			*next = start_clu;
			return 0;
		}
		start_clu++;
	}
	return 1;
}

int exfat_bitmap_find_zero(struct exfat *exfat, char *bmap,
			   clus_t start_clu, clus_t *next)
{
	return exfat_bitmap_find_bit(exfat, bmap,
				     start_clu, next, 0);
}

int exfat_bitmap_find_one(struct exfat *exfat, char *bmap,
			  clus_t start_clu, clus_t *next)
{
	return exfat_bitmap_find_bit(exfat, bmap,
				     start_clu, next, 1);
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
	struct stat st;
	unsigned long long blk_dev_offset = 0;

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

	if (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode)) {
		char pathname[sizeof("/sys/dev/block/4294967295:4294967295/start")];
		FILE *fp;

		snprintf(pathname, sizeof(pathname), "/sys/dev/block/%u:%u/start",
			major(st.st_rdev), minor(st.st_rdev));
		fp = fopen(pathname, "r");
		if (fp != NULL) {
			if (fscanf(fp, "%llu", &blk_dev_offset) == 1) {
				/*
				 * Linux kernel always reports partition offset
				 * in 512-byte units, regardless of sector size
				 */
				blk_dev_offset <<= 9;
			}
			fclose(fp);
		}
	}

	bd->dev_fd = fd;
	bd->offset = blk_dev_offset;
	bd->size = blk_dev_size;
	if (!ui->cluster_size)
		exfat_set_default_cluster_size(bd, ui);

	if (!ui->boundary_align)
		ui->boundary_align = DEFAULT_BOUNDARY_ALIGNMENT;

	if (ui->sector_size)
		bd->sector_size = ui->sector_size;
	else if (ioctl(fd, BLKSSZGET, &bd->sector_size) < 0)
		bd->sector_size = DEFAULT_SECTOR_SIZE;
	bd->sector_size_bits = sector_size_bits(bd->sector_size);
	bd->num_sectors = blk_dev_size / bd->sector_size;
	bd->num_clusters = blk_dev_size / ui->cluster_size;

	exfat_debug("Block device name : %s\n", ui->dev_name);
	exfat_debug("Block device offset : %llu\n", bd->offset);
	exfat_debug("Block device size : %llu\n", bd->size);
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

ssize_t exfat_write_zero(int fd, size_t size, off_t offset)
{
	const char zero_buf[4 * KB] = {0};

	lseek(fd, offset, SEEK_SET);

	while (size > 0) {
		int iter_size = MIN(size, sizeof(zero_buf));

		if (iter_size != write(fd, zero_buf, iter_size))
			return -EIO;

		size -= iter_size;
	}

	return 0;
}

size_t exfat_utf16_len(const __le16 *str, size_t max_size)
{
	size_t i = 0;

	while (le16_to_cpu(str[i]) && i < max_size)
		i++;
	return i;
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
		free(wcs);
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
	for (i = 0, out_len = 0, c_len = 0; i <= wcs_len; i++) {
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
	if (c_len == 0 || out_str[out_len-1] != 0) {
		exfat_err("invalid UTF-16 sequence\n");
		return -errno;
	}

	return out_len-1;
}

off_t exfat_get_root_entry_offset(struct exfat_blk_dev *bd)
{
	struct pbr *bs;
	int nbytes;
	unsigned int cluster_size, sector_size;
	off_t root_clu_off;

	bs = malloc(EXFAT_MAX_SECTOR_SIZE);
	if (!bs) {
		exfat_err("failed to allocate memory\n");
		return -ENOMEM;
	}

	nbytes = exfat_read(bd->dev_fd, bs, EXFAT_MAX_SECTOR_SIZE, 0);
	if (nbytes != EXFAT_MAX_SECTOR_SIZE) {
		exfat_err("boot sector read failed: %d\n", errno);
		free(bs);
		return -1;
	}

	if (memcmp(bs->bpb.oem_name, "EXFAT   ", 8) != 0) {
		exfat_err("Bad fs_name in boot sector, which does not describe a valid exfat filesystem\n");
		free(bs);
		return -1;
	}

	sector_size = 1 << bs->bsx.sect_size_bits;
	cluster_size = (1 << bs->bsx.sect_per_clus_bits) * sector_size;
	root_clu_off = le32_to_cpu(bs->bsx.clu_offset) * sector_size +
		(le32_to_cpu(bs->bsx.root_cluster) - EXFAT_RESERVED_CLUSTERS) *
		cluster_size;
	free(bs);

	return root_clu_off;
}

char *exfat_conv_volume_label(struct exfat_dentry *vol_entry)
{
	char *volume_label;
	__le16 disk_label[VOLUME_LABEL_MAX_LEN];

	volume_label = malloc(VOLUME_LABEL_BUFFER_SIZE);
	if (!volume_label)
		return NULL;

	memcpy(disk_label, vol_entry->vol_label, sizeof(disk_label));
	memset(volume_label, 0, VOLUME_LABEL_BUFFER_SIZE);
	if (exfat_utf16_dec(disk_label, vol_entry->vol_char_cnt*2,
		volume_label, VOLUME_LABEL_BUFFER_SIZE) < 0) {
		exfat_err("failed to decode volume label\n");
		free(volume_label);
		return NULL;
	}

	return volume_label;
}

int exfat_read_volume_label(struct exfat *exfat)
{
	struct exfat_dentry *dentry;
	int err;
	__le16 disk_label[VOLUME_LABEL_MAX_LEN];
	struct exfat_lookup_filter filter = {
		.in.type = EXFAT_VOLUME,
		.in.dentry_count = 0,
		.in.filter = NULL,
	};

	err = exfat_lookup_dentry_set(exfat, exfat->root, &filter);
	if (err)
		return err;

	dentry = filter.out.dentry_set;

	if (dentry->vol_char_cnt == 0)
		goto out;

	if (dentry->vol_char_cnt > VOLUME_LABEL_MAX_LEN) {
		exfat_err("too long label. %d\n", dentry->vol_char_cnt);
		err = -EINVAL;
		goto out;
	}

	memcpy(disk_label, dentry->vol_label, sizeof(disk_label));
	if (exfat_utf16_dec(disk_label, dentry->vol_char_cnt*2,
		exfat->volume_label, sizeof(exfat->volume_label)) < 0) {
		exfat_err("failed to decode volume label\n");
		err = -EINVAL;
		goto out;
	}

	exfat_info("label: %s\n", exfat->volume_label);
out:
	free(filter.out.dentry_set);
	return err;
}

int exfat_set_volume_label(struct exfat *exfat, char *label_input)
{
	struct exfat_dentry *pvol;
	struct exfat_dentry_loc loc;
	__u16 volume_label[VOLUME_LABEL_MAX_LEN];
	int volume_label_len, dcount, err;

	struct exfat_lookup_filter filter = {
		.in.type = EXFAT_VOLUME,
		.in.dentry_count = 1,
		.in.filter = NULL,
	};

	err = exfat_lookup_dentry_set(exfat, exfat->root, &filter);
	if (!err) {
		pvol = filter.out.dentry_set;
		dcount = filter.out.dentry_count;
		memset(pvol->vol_label, 0, sizeof(pvol->vol_label));
	} else {
		pvol = calloc(1, sizeof(struct exfat_dentry));
		if (!pvol)
			return -ENOMEM;

		dcount = 1;
		pvol->type = EXFAT_VOLUME;
	}

	volume_label_len = exfat_utf16_enc(label_input,
			volume_label, sizeof(volume_label));
	if (volume_label_len < 0) {
		exfat_err("failed to encode volume label\n");
		free(pvol);
		return -1;
	}

	memcpy(pvol->vol_label, volume_label, volume_label_len);
	pvol->vol_char_cnt = volume_label_len/2;

	loc.parent = exfat->root;
	loc.file_offset = filter.out.file_offset;
	loc.dev_offset = filter.out.dev_offset;
	err = exfat_add_dentry_set(exfat, &loc, pvol, dcount, false);
	exfat_info("new label: %s\n", label_input);

	free(pvol);

	return err;
}

static inline void print_guid(const char *msg, const __u8 *guid)
{
	exfat_info("%s: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
			msg,
			guid[0], guid[1], guid[2], guid[3],
			guid[4], guid[5], guid[5], guid[7],
			guid[8], guid[9], guid[10], guid[11],
			guid[12], guid[13], guid[14], guid[15]);
}

static int set_guid(__u8 *guid, const char *input)
{
	int i, j, zero_len = 0;
	int len = strlen(input);

	if (len != EXFAT_GUID_LEN * 2 && len != EXFAT_GUID_LEN * 2 + 4) {
		exfat_err("invalid format for volume guid\n");
		return -EINVAL;
	}

	for (i = 0, j = 0; i < len; i++) {
		unsigned char ch = input[i];

		if (ch >= '0' && ch <= '9')
			ch -= '0';
		else if (ch >= 'a' && ch <= 'f')
			ch -= 'a' - 0xA;
		else if (ch >= 'A' && ch <= 'F')
			ch -= 'A' - 0xA;
		else if (ch == '-' && len == EXFAT_GUID_LEN * 2 + 4 &&
			 (i == 8 || i == 13 || i == 18 || i == 23))
			continue;
		else {
			exfat_err("invalid character '%c' for volume GUID\n", ch);
			return -EINVAL;
		}

		if (j & 1)
			guid[j >> 1] |= ch;
		else
			guid[j >> 1] = ch << 4;

		j++;

		if (ch == 0)
			zero_len++;
	}

	if (zero_len == EXFAT_GUID_LEN * 2) {
		exfat_err("%s is invalid for volume GUID\n", input);
		return -EINVAL;
	}

	return 0;
}

int exfat_read_volume_guid(struct exfat *exfat)
{
	int err;
	uint16_t checksum = 0;
	struct exfat_dentry *dentry;
	struct exfat_lookup_filter filter = {
		.in.type = EXFAT_GUID,
		.in.dentry_count = 1,
		.in.filter = NULL,
	};

	err = exfat_lookup_dentry_set(exfat, exfat->root, &filter);
	if (err)
		return err;

	dentry = filter.out.dentry_set;
	exfat_calc_dentry_checksum(dentry, &checksum, true);

	if (cpu_to_le16(checksum) == dentry->dentry.guid.checksum)
		print_guid("GUID", dentry->dentry.guid.guid);
	else
		exfat_info("GUID is corrupted, please delete it or set a new one\n");

	free(dentry);

	return err;
}

int __exfat_set_volume_guid(struct exfat_dentry *dentry, const char *guid)
{
	int err;
	uint16_t checksum = 0;

	memset(dentry, 0, sizeof(*dentry));
	dentry->type = EXFAT_GUID;

	err = set_guid(dentry->dentry.guid.guid, guid);
	if (err)
		return err;

	exfat_calc_dentry_checksum(dentry, &checksum, true);
	dentry->dentry.guid.checksum = cpu_to_le16(checksum);

	return 0;
}

/*
 * Create/Update/Delete GUID dentry in root directory
 *
 * create/update GUID if @guid is not NULL.
 * delete GUID if @guid is NULL.
 */
int exfat_set_volume_guid(struct exfat *exfat, const char *guid)
{
	struct exfat_dentry *dentry;
	struct exfat_dentry_loc loc;
	int err;

	struct exfat_lookup_filter filter = {
		.in.type = EXFAT_GUID,
		.in.dentry_count = 1,
		.in.filter = NULL,
	};

	err = exfat_lookup_dentry_set(exfat, exfat->root, &filter);
	if (!err) {
		/* GUID entry is found */
		dentry = filter.out.dentry_set;
	} else {
		/* no GUID to delete */
		if (guid == NULL)
			return 0;

		dentry = calloc(1, sizeof(*dentry));
		if (!dentry)
			return -ENOMEM;
	}

	if (guid) {
		/* Set GUID */
		err = __exfat_set_volume_guid(dentry, guid);
		if (err)
			goto out;
	} else {
		/* Delete GUID */
		dentry->type &= ~EXFAT_INVAL;
	}

	loc.parent = exfat->root;
	loc.file_offset = filter.out.file_offset;
	loc.dev_offset = filter.out.dev_offset;
	err = exfat_add_dentry_set(exfat, &loc, dentry, 1, false);
	if (!err) {
		if (guid)
			print_guid("new GUID", dentry->dentry.guid.guid);
		else
			exfat_info("GUID is deleted\n");
	}

out:
	free(dentry);

	return err;
}

int exfat_read_sector(struct exfat_blk_dev *bd, void *buf, unsigned int sec_off)
{
	int ret;
	unsigned long long offset =
		(unsigned long long)sec_off * bd->sector_size;

	ret = pread(bd->dev_fd, buf, bd->sector_size, offset);
	if (ret < 0) {
		exfat_err("read failed, sec_off : %u\n", sec_off);
		return -1;
	}
	return 0;
}

int exfat_write_sector(struct exfat_blk_dev *bd, void *buf,
		unsigned int sec_off)
{
	int bytes;
	unsigned long long offset =
		(unsigned long long)sec_off * bd->sector_size;

	bytes = pwrite(bd->dev_fd, buf, bd->sector_size, offset);
	if (bytes != (int)bd->sector_size) {
		exfat_err("write failed, sec_off : %u, bytes : %d\n", sec_off,
			bytes);
		return -1;
	}
	return 0;
}

int exfat_write_checksum_sector(struct exfat_blk_dev *bd,
		unsigned int checksum, bool is_backup)
{
	__le32 *checksum_buf;
	int ret = 0;
	unsigned int i;
	unsigned int sec_idx = CHECKSUM_SEC_IDX;

	checksum_buf = malloc(bd->sector_size);
	if (!checksum_buf)
		return -1;

	if (is_backup)
		sec_idx += BACKUP_BOOT_SEC_IDX;

	for (i = 0; i < bd->sector_size / sizeof(int); i++)
		checksum_buf[i] = cpu_to_le32(checksum);

	ret = exfat_write_sector(bd, checksum_buf, sec_idx);
	if (ret) {
		exfat_err("checksum sector write failed\n");
		goto free;
	}

free:
	free(checksum_buf);
	return ret;
}

int exfat_show_volume_serial(int fd)
{
	struct pbr *ppbr;
	int ret;

	ppbr = malloc(EXFAT_MAX_SECTOR_SIZE);
	if (!ppbr) {
		exfat_err("Cannot allocate pbr: out of memory\n");
		return -1;
	}

	/* read main boot sector */
	ret = exfat_read(fd, (char *)ppbr, EXFAT_MAX_SECTOR_SIZE, 0);
	if (ret < 0) {
		exfat_err("main boot sector read failed\n");
		ret = -1;
		goto free_ppbr;
	}

	if (memcmp(ppbr->bpb.oem_name, "EXFAT   ", 8) != 0) {
		exfat_err("Bad fs_name in boot sector, which does not describe a valid exfat filesystem\n");
		ret = -1;
		goto free_ppbr;
	}

	exfat_info("volume serial : 0x%x\n", ppbr->bsx.vol_serial);

free_ppbr:
	free(ppbr);
	return ret;
}

static int exfat_update_boot_checksum(struct exfat_blk_dev *bd, bool is_backup)
{
	unsigned int checksum = 0;
	int ret, sec_idx, backup_sec_idx = 0;
	unsigned char *buf;

	buf = malloc(bd->sector_size);
	if (!buf) {
		exfat_err("Cannot allocate pbr: out of memory\n");
		return -1;
	}

	if (is_backup)
		backup_sec_idx = BACKUP_BOOT_SEC_IDX;

	for (sec_idx = BOOT_SEC_IDX; sec_idx < CHECKSUM_SEC_IDX; sec_idx++) {
		bool is_boot_sec = false;

		ret = exfat_read_sector(bd, buf, sec_idx + backup_sec_idx);
		if (ret < 0) {
			exfat_err("sector(%d) read failed\n", sec_idx);
			ret = -1;
			goto free_buf;
		}

		if (sec_idx == BOOT_SEC_IDX)
			is_boot_sec = true;

		boot_calc_checksum(buf, bd->sector_size, is_boot_sec,
			&checksum);
	}

	ret = exfat_write_checksum_sector(bd, checksum, is_backup);

free_buf:
	free(buf);

	return ret;
}

int exfat_set_volume_serial(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui)
{
	int ret;
	struct pbr *ppbr;

	ppbr = malloc(EXFAT_MAX_SECTOR_SIZE);
	if (!ppbr) {
		exfat_err("Cannot allocate pbr: out of memory\n");
		return -1;
	}

	/* read main boot sector */
	ret = exfat_read(bd->dev_fd, (char *)ppbr, EXFAT_MAX_SECTOR_SIZE,
			BOOT_SEC_IDX);
	if (ret < 0) {
		exfat_err("main boot sector read failed\n");
		ret = -1;
		goto free_ppbr;
	}

	if (memcmp(ppbr->bpb.oem_name, "EXFAT   ", 8) != 0) {
		exfat_err("Bad fs_name in boot sector, which does not describe a valid exfat filesystem\n");
		ret = -1;
		goto free_ppbr;
	}

	bd->sector_size = 1 << ppbr->bsx.sect_size_bits;
	ppbr->bsx.vol_serial = cpu_to_le32(ui->volume_serial);

	/* update main boot sector */
	ret = exfat_write_sector(bd, (char *)ppbr, BOOT_SEC_IDX);
	if (ret < 0) {
		exfat_err("main boot sector write failed\n");
		ret = -1;
		goto free_ppbr;
	}

	/* update backup boot sector */
	ret = exfat_write_sector(bd, (char *)ppbr, BACKUP_BOOT_SEC_IDX);
	if (ret < 0) {
		exfat_err("backup boot sector write failed\n");
		ret = -1;
		goto free_ppbr;
	}

	ret = exfat_update_boot_checksum(bd, 0);
	if (ret < 0) {
		exfat_err("main checksum update failed\n");
		goto free_ppbr;
	}

	ret = exfat_update_boot_checksum(bd, 1);
	if (ret < 0)
		exfat_err("backup checksum update failed\n");
free_ppbr:
	free(ppbr);

	exfat_info("New volume serial : 0x%x\n", ui->volume_serial);

	return ret;
}

unsigned int exfat_clus_to_blk_dev_off(struct exfat_blk_dev *bd,
		unsigned int clu_off_sectnr, unsigned int clu)
{
	return clu_off_sectnr * bd->sector_size +
		(clu - EXFAT_RESERVED_CLUSTERS) * bd->cluster_size;
}

int exfat_get_next_clus(struct exfat *exfat, clus_t clus, clus_t *next)
{
	off_t offset;

	*next = EXFAT_EOF_CLUSTER;

	if (!exfat_heap_clus(exfat, clus))
		return -EINVAL;

	offset = (off_t)le32_to_cpu(exfat->bs->bsx.fat_offset) <<
				exfat->bs->bsx.sect_size_bits;
	offset += sizeof(clus_t) * clus;

	if (exfat_read(exfat->blk_dev->dev_fd, next, sizeof(*next), offset)
			!= sizeof(*next))
		return -EIO;
	*next = le32_to_cpu(*next);
	return 0;
}

int exfat_get_inode_next_clus(struct exfat *exfat, struct exfat_inode *node,
			      clus_t clus, clus_t *next)
{
	*next = EXFAT_EOF_CLUSTER;

	if (node->is_contiguous) {
		if (!exfat_heap_clus(exfat, clus))
			return -EINVAL;
		*next = clus + 1;
		return 0;
	}

	return exfat_get_next_clus(exfat, clus, next);
}

int exfat_set_fat(struct exfat *exfat, clus_t clus, clus_t next_clus)
{
	off_t offset;

	offset = le32_to_cpu(exfat->bs->bsx.fat_offset) <<
		exfat->bs->bsx.sect_size_bits;
	offset += sizeof(clus_t) * clus;

	if (exfat_write(exfat->blk_dev->dev_fd, &next_clus, sizeof(next_clus),
			offset) != sizeof(next_clus))
		return -EIO;
	return 0;
}

off_t exfat_s2o(struct exfat *exfat, off_t sect)
{
	return sect << exfat->bs->bsx.sect_size_bits;
}

off_t exfat_c2o(struct exfat *exfat, unsigned int clus)
{
	assert(clus >= EXFAT_FIRST_CLUSTER);

	return exfat_s2o(exfat, le32_to_cpu(exfat->bs->bsx.clu_offset) +
				((off_t)(clus - EXFAT_FIRST_CLUSTER) <<
				 exfat->bs->bsx.sect_per_clus_bits));
}

int exfat_o2c(struct exfat *exfat, off_t device_offset,
	      unsigned int *clu, unsigned int *offset)
{
	off_t heap_offset;

	heap_offset = exfat_s2o(exfat, le32_to_cpu(exfat->bs->bsx.clu_offset));
	if (device_offset < heap_offset)
		return -ERANGE;

	*clu = (unsigned int)((device_offset - heap_offset) /
			      exfat->clus_size) + EXFAT_FIRST_CLUSTER;
	if (!exfat_heap_clus(exfat, *clu))
		return -ERANGE;
	*offset = (device_offset - heap_offset) % exfat->clus_size;
	return 0;
}

bool exfat_heap_clus(struct exfat *exfat, clus_t clus)
{
	return clus >= EXFAT_FIRST_CLUSTER &&
		(clus - EXFAT_FIRST_CLUSTER) < exfat->clus_count;
}

int exfat_root_clus_count(struct exfat *exfat)
{
	struct exfat_inode *node = exfat->root;
	clus_t clus, next;
	int clus_count = 0;

	if (!exfat_heap_clus(exfat, node->first_clus))
		return -EIO;

	clus = node->first_clus;
	do {
		if (exfat_bitmap_get(exfat->alloc_bitmap, clus))
			return -EINVAL;

		exfat_bitmap_set(exfat->alloc_bitmap, clus);

		if (exfat_get_inode_next_clus(exfat, node, clus, &next)) {
			exfat_err("ERROR: failed to read the fat entry of root");
			return -EIO;
		}

		if (next != EXFAT_EOF_CLUSTER && !exfat_heap_clus(exfat, next))
			return -EINVAL;

		clus = next;
		clus_count++;
	} while (clus != EXFAT_EOF_CLUSTER);

	node->size = clus_count * exfat->clus_size;
	return 0;
}

int read_boot_sect(struct exfat_blk_dev *bdev, struct pbr **bs)
{
	struct pbr *pbr;
	int err = 0;
	unsigned int sect_size, clu_size;

	pbr = malloc(sizeof(struct pbr));
	if (!pbr) {
		exfat_err("failed to allocate memory\n");
		return -ENOMEM;
	}

	if (exfat_read(bdev->dev_fd, pbr, sizeof(*pbr), 0) !=
	    (ssize_t)sizeof(*pbr)) {
		exfat_err("failed to read a boot sector\n");
		err = -EIO;
		goto err;
	}

	err = -EINVAL;
	if (memcmp(pbr->bpb.oem_name, "EXFAT   ", 8) != 0) {
		exfat_err("failed to find exfat file system\n");
		goto err;
	}

	sect_size = 1 << pbr->bsx.sect_size_bits;
	clu_size = 1 << (pbr->bsx.sect_size_bits +
			 pbr->bsx.sect_per_clus_bits);

	if (sect_size < 512 || sect_size > 4 * KB) {
		exfat_err("too small or big sector size: %d\n",
			  sect_size);
		goto err;
	}

	if (clu_size < sect_size || clu_size > 32 * MB) {
		exfat_err("too small or big cluster size: %d\n",
			  clu_size);
		goto err;
	}

	*bs = pbr;
	return 0;
err:
	free(pbr);
	return err;
}

int exfat_parse_ulong(const char *s, unsigned long *out)
{
	char *endptr;

	errno = 0;

	*out = strtoul(s, &endptr, 0);

	if (errno)
		return -errno;

	if (s == endptr || *endptr != '\0')
		return -EINVAL;

	return 0;
}
