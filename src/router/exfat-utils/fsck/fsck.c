// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2019 Namjae Jeon <linkinjeon@kernel.org>
 *   Copyright (C) 2020 Hyunchul Lee <hyc.lee@gmail.com>
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <locale.h>

#include "exfat_ondisk.h"
#include "libexfat.h"
#include "repair.h"
#include "exfat_fs.h"
#include "exfat_dir.h"
#include "fsck.h"

struct fsck_user_input {
	struct exfat_user_input		ei;
	enum fsck_ui_options		options;
};

#define EXFAT_MAX_UPCASE_CHARS	0x10000

#define FSCK_EXIT_NO_ERRORS		0x00
#define FSCK_EXIT_CORRECTED		0x01
#define FSCK_EXIT_NEED_REBOOT		0x02
#define FSCK_EXIT_ERRORS_LEFT		0x04
#define FSCK_EXIT_OPERATION_ERROR	0x08
#define FSCK_EXIT_SYNTAX_ERROR		0x10
#define FSCK_EXIT_USER_CANCEL		0x20
#define FSCK_EXIT_LIBRARY_ERROR		0x80

struct exfat_stat {
	long		dir_count;
	long		file_count;
	long		error_count;
	long		fixed_count;
};

struct exfat_fsck exfat_fsck;
struct exfat_stat exfat_stat;
struct path_resolve_ctx path_resolve_ctx;

static struct option opts[] = {
	{"repair",	no_argument,	NULL,	'r' },
	{"repair-yes",	no_argument,	NULL,	'y' },
	{"repair-no",	no_argument,	NULL,	'n' },
	{"repair-auto",	no_argument,	NULL,	'p' },
	{"rescue",	no_argument,	NULL,	's' },
	{"version",	no_argument,	NULL,	'V' },
	{"verbose",	no_argument,	NULL,	'v' },
	{"help",	no_argument,	NULL,	'h' },
	{"?",		no_argument,	NULL,	'?' },
	{"ignore-bad-fs",	no_argument,	NULL,	'b' },
	{NULL,		0,		NULL,	 0  }
};

static void usage(char *name)
{
	fprintf(stderr, "Usage: %s\n", name);
	fprintf(stderr, "\t-r | --repair        Repair interactively\n");
	fprintf(stderr, "\t-y | --repair-yes    Repair without ask\n");
	fprintf(stderr, "\t-n | --repair-no     No repair\n");
	fprintf(stderr, "\t-p | --repair-auto   Repair automatically\n");
	fprintf(stderr, "\t-a                   Repair automatically\n");
	fprintf(stderr, "\t-b | --ignore-bad-fs Try to recover even if exfat is not found\n");
	fprintf(stderr, "\t-s | --rescue        Assign orphaned clusters to files\n");
	fprintf(stderr, "\t-V | --version       Show version\n");
	fprintf(stderr, "\t-v | --verbose       Print debug\n");
	fprintf(stderr, "\t-h | --help          Show help\n");

	exit(FSCK_EXIT_SYNTAX_ERROR);
}

#define fsck_err(parent, inode, fmt, ...)		\
({							\
		exfat_resolve_path_parent(&path_resolve_ctx,	\
			parent, inode);			\
		exfat_err("ERROR: %s: " fmt,		\
			path_resolve_ctx.local_path,	\
			##__VA_ARGS__);			\
})

#define repair_file_ask(iter, inode, code, fmt, ...)	\
({							\
		if (inode)						\
			exfat_resolve_path_parent(&path_resolve_ctx,	\
					    (iter)->parent, inode);	\
		else							\
			exfat_resolve_path(&path_resolve_ctx,		\
				     (iter)->parent);			\
		exfat_repair_ask(&exfat_fsck, code,			\
				 "ERROR: %s: " fmt " at %#" PRIx64,	\
				 path_resolve_ctx.local_path,		\
				 ##__VA_ARGS__,				\
				 exfat_de_iter_device_offset(iter));	\
})

static int check_clus_chain(struct exfat_de_iter *de_iter, int stream_idx,
			    struct exfat_inode *node)
{
	struct exfat *exfat = de_iter->exfat;
	struct exfat_dentry *stream_de;
	clus_t clus, prev, next;
	uint64_t count, max_count;

	clus = node->first_clus;
	prev = EXFAT_EOF_CLUSTER;
	count = 0;
	max_count = DIV_ROUND_UP(node->size, exfat->clus_size);

	if (node->size == 0 && node->first_clus == EXFAT_FREE_CLUSTER)
		return 0;

	/* the first cluster is wrong */
	if ((node->size == 0 && node->first_clus != EXFAT_FREE_CLUSTER) ||
	    (node->size > 0 && !exfat_heap_clus(exfat, node->first_clus))) {
		if (repair_file_ask(de_iter, node,
				    ER_FILE_FIRST_CLUS,
				    "size %#" PRIx64 ", but the first cluster %#x",
				    node->size, node->first_clus))
			goto truncate_file;
		else
			return -EINVAL;
	}

	while (clus != EXFAT_EOF_CLUSTER) {
		if (count >= max_count) {
			if (node->is_contiguous)
				break;
			if (repair_file_ask(de_iter, node,
					    ER_FILE_SMALLER_SIZE,
					    "more clusters are allocated. truncate to %"
					    PRIu64 " bytes",
					    count * exfat->clus_size))
				goto truncate_file;
			else
				return -EINVAL;
		}

		/*
		 * This cluster is already allocated. it may be shared with
		 * the other file, or there is a loop in cluster chain.
		 */
		if (exfat_bitmap_get(exfat->alloc_bitmap, clus)) {
			if (repair_file_ask(de_iter, node,
					    ER_FILE_DUPLICATED_CLUS,
					    "cluster is already allocated for the other file. truncated to %"
					    PRIu64 " bytes",
					    count * exfat->clus_size))
				goto truncate_file;
			else
				return -EINVAL;
		}

		if (!exfat_bitmap_get(exfat->disk_bitmap, clus)) {
			if (!repair_file_ask(de_iter, node,
					     ER_FILE_INVALID_CLUS,
					     "cluster %#x is marked as free",
					     clus))
				return -EINVAL;
		}

		/* This cluster is allocated or not */
		if (exfat_get_inode_next_clus(exfat, node, clus, &next))
			goto truncate_file;
		if (next == EXFAT_BAD_CLUSTER) {
			if (repair_file_ask(de_iter, node,
					    ER_FILE_INVALID_CLUS,
					    "BAD cluster. truncate to %"
					    PRIu64 " bytes",
					    count * exfat->clus_size))
				goto truncate_file;
			else
				return -EINVAL;
		} else if (!node->is_contiguous) {
			if (next != EXFAT_EOF_CLUSTER &&
			    !exfat_heap_clus(exfat, next)) {
				if (repair_file_ask(de_iter, node,
						    ER_FILE_INVALID_CLUS,
						    "broken cluster chain. truncate to %"
						    PRIu64 " bytes",
						    (count + 1) * exfat->clus_size)) {
					count++;
					prev = clus;
					exfat_bitmap_set(exfat->alloc_bitmap,
							 clus);
					goto truncate_file;
				} else {
					return -EINVAL;
				}
			}
		}

		count++;
		exfat_bitmap_set(exfat->alloc_bitmap, clus);
		prev = clus;
		clus = next;
	}

	if (count < max_count) {
		if (repair_file_ask(de_iter, node, ER_FILE_LARGER_SIZE,
				    "less clusters are allocated. truncates to %"
				    PRIu64 " bytes",
				    count * exfat->clus_size))
			goto truncate_file;
		else
			return -EINVAL;
	}

	return 0;
truncate_file:
	node->size = count * exfat->clus_size;
	if (!exfat_heap_clus(exfat, prev))
		node->first_clus = EXFAT_FREE_CLUSTER;

	exfat_de_iter_get_dirty(de_iter, stream_idx, &stream_de);
	if (stream_idx == 1 && count * exfat->clus_size <
	    le64_to_cpu(stream_de->stream_valid_size))
		stream_de->stream_valid_size = cpu_to_le64(
							   count * exfat->clus_size);
	if (!exfat_heap_clus(exfat, prev))
		stream_de->stream_start_clu = EXFAT_FREE_CLUSTER;
	stream_de->stream_size = cpu_to_le64(
					     count * exfat->clus_size);

	/* remaining clusters will be freed while FAT is compared with
	 * alloc_bitmap.
	 */
	if (!node->is_contiguous && exfat_heap_clus(exfat, prev)) {
		if (exfat_set_fat(exfat, prev, EXFAT_EOF_CLUSTER))
			return -EIO;
	}
	return 1;
}

static int root_check_clus_chain(struct exfat *exfat,
				 struct exfat_inode *node,
				 clus_t *clus_count)
{
	clus_t clus, next, prev = EXFAT_EOF_CLUSTER;

	if (!exfat_heap_clus(exfat, node->first_clus))
		goto out_trunc;

	clus = node->first_clus;
	*clus_count = 0;

	do {
		if (exfat_bitmap_get(exfat->alloc_bitmap, clus)) {
			if (exfat_repair_ask(&exfat_fsck,
					     ER_FILE_DUPLICATED_CLUS,
					     "ERROR: the cluster chain of root is cyclic"))
				goto out_trunc;
			return -EINVAL;
		}

		exfat_bitmap_set(exfat->alloc_bitmap, clus);

		if (exfat_get_inode_next_clus(exfat, node, clus, &next)) {
			exfat_err("ERROR: failed to read the fat entry of root");
			goto out_trunc;
		}

		if (next != EXFAT_EOF_CLUSTER && !exfat_heap_clus(exfat, next)) {
			if (exfat_repair_ask(&exfat_fsck,
					     ER_FILE_INVALID_CLUS,
					     "ERROR: the cluster chain of root is broken")) {
				if (next != EXFAT_BAD_CLUSTER) {
					prev = clus;
					(*clus_count)++;
				}
				goto out_trunc;
			}
			return -EINVAL;
		}

		prev = clus;
		clus = next;
		(*clus_count)++;
	} while (clus != EXFAT_EOF_CLUSTER);

	return 0;
out_trunc:
	if (!exfat_heap_clus(exfat, prev)) {
		exfat_err("ERROR: the start cluster of root is wrong\n");
		return -EINVAL;
	}
	node->size = *clus_count * exfat->clus_size;
	return exfat_set_fat(exfat, prev, EXFAT_EOF_CLUSTER);
}

static int boot_region_checksum(int dev_fd,
				int bs_offset, unsigned int sect_size)
{
	void *sect;
	unsigned int i;
	uint32_t checksum;
	int ret = 0;

	sect = malloc(sect_size);
	if (!sect)
		return -ENOMEM;

	checksum = 0;
	for (i = 0; i < 11; i++) {
		if (exfat_read(dev_fd, sect, sect_size,
				bs_offset * sect_size + i * sect_size) !=
				(ssize_t)sect_size) {
			exfat_err("failed to read boot region\n");
			ret = -EIO;
			goto out;
		}
		boot_calc_checksum(sect, sect_size, i == 0, &checksum);
	}

	if (exfat_read(dev_fd, sect, sect_size,
			bs_offset * sect_size + 11 * sect_size) !=
			(ssize_t)sect_size) {
		exfat_err("failed to read a boot checksum sector\n");
		ret = -EIO;
		goto out;
	}

	for (i = 0; i < sect_size/sizeof(checksum); i++) {
		if (le32_to_cpu(((__le32 *)sect)[i]) != checksum) {
			exfat_err("checksum of boot region is not correct. %#x, but expected %#x\n",
				le32_to_cpu(((__le32 *)sect)[i]), checksum);
			ret = -EINVAL;
			goto out;
		}
	}
out:
	free(sect);
	return ret;
}

static int exfat_mark_volume_dirty(struct exfat *exfat, bool dirty)
{
	uint16_t flags;

	flags = le16_to_cpu(exfat->bs->bsx.vol_flags);
	if (dirty)
		flags |= 0x02;
	else
		flags &= ~0x02;

	exfat->bs->bsx.vol_flags = cpu_to_le16(flags);
	if (exfat_write(exfat->blk_dev->dev_fd, exfat->bs,
			sizeof(struct pbr), 0) != (ssize_t)sizeof(struct pbr)) {
		exfat_err("failed to set VolumeDirty\n");
		return -EIO;
	}

	if (fsync(exfat->blk_dev->dev_fd) != 0) {
		exfat_err("failed to set VolumeDirty\n");
		return -EIO;
	}
	return 0;
}

static int read_boot_region(struct exfat_blk_dev *bd, struct pbr **pbr,
			    int bs_offset, unsigned int sect_size,
			    bool verbose)
{
	struct pbr *bs;
	int ret = -EINVAL;
	unsigned long long clu_max_count;

	*pbr = NULL;
	bs = malloc(sizeof(struct pbr));
	if (!bs) {
		exfat_err("failed to allocate memory\n");
		return -ENOMEM;
	}

	if (exfat_read(bd->dev_fd, bs, sizeof(*bs),
			bs_offset * sect_size) != (ssize_t)sizeof(*bs)) {
		exfat_err("failed to read a boot sector\n");
		ret = -EIO;
		goto err;
	}

	if (memcmp(bs->bpb.oem_name, "EXFAT   ", 8) != 0) {
		if (verbose)
			exfat_err("failed to find exfat file system\n");
		goto err;
	}

	ret = boot_region_checksum(bd->dev_fd, bs_offset, sect_size);
	if (ret < 0)
		goto err;

	ret = -EINVAL;
	if (EXFAT_SECTOR_SIZE(bs) < 512 || EXFAT_SECTOR_SIZE(bs) > 4 * KB) {
		if (verbose)
			exfat_err("too small or big sector size: %d\n",
				  EXFAT_SECTOR_SIZE(bs));
		goto err;
	}

	if (EXFAT_CLUSTER_SIZE(bs) > 32 * MB) {
		if (verbose)
			exfat_err("too big cluster size: %d\n",
				  EXFAT_CLUSTER_SIZE(bs));
		goto err;
	}

	if (bs->bsx.fs_version[1] != 1 || bs->bsx.fs_version[0] != 0) {
		if (verbose)
			exfat_err("unsupported exfat version: %d.%d\n",
				  bs->bsx.fs_version[1], bs->bsx.fs_version[0]);
		goto err;
	}

	if (bs->bsx.num_fats != 1) {
		if (verbose)
			exfat_err("unsupported FAT count: %d\n",
				  bs->bsx.num_fats);
		goto err;
	}

	if (le64_to_cpu(bs->bsx.vol_length) * EXFAT_SECTOR_SIZE(bs) >
			bd->size) {
		if (verbose)
			exfat_err("too large sector count: %" PRIu64 ", expected: %llu\n",
				  le64_to_cpu(bs->bsx.vol_length),
				  bd->num_sectors);
		goto err;
	}

	clu_max_count = (le64_to_cpu(bs->bsx.vol_length) - le32_to_cpu(bs->bsx.clu_offset)) >>
				bs->bsx.sect_per_clus_bits;
	if (le32_to_cpu(bs->bsx.clu_count) > clu_max_count) {
		if (verbose)
			exfat_err("too large cluster count: %u, expected: %llu\n",
				  le32_to_cpu(bs->bsx.clu_count),
				  MIN(clu_max_count, EXFAT_MAX_NUM_CLUSTER));
		goto err;
	}

	*pbr = bs;
	return 0;
err:
	free(bs);
	return ret;
}

static int restore_boot_region(struct exfat_blk_dev *bd, unsigned int sect_size)
{
	int i;
	char *sector;
	int ret;

	sector = malloc(sect_size);
	if (!sector)
		return -ENOMEM;

	for (i = 0; i < 12; i++) {
		if (exfat_read(bd->dev_fd, sector, sect_size,
				BACKUP_BOOT_SEC_IDX * sect_size +
				i * sect_size) !=
				(ssize_t)sect_size) {
			ret = -EIO;
			goto free_sector;
		}
		if (i == 0)
			((struct pbr *)sector)->bsx.perc_in_use = 0xff;

		if (exfat_write(bd->dev_fd, sector, sect_size,
				BOOT_SEC_IDX * sect_size +
				i * sect_size) !=
				(ssize_t)sect_size) {
			ret = -EIO;
			goto free_sector;
		}
	}

	if (fsync(bd->dev_fd)) {
		ret = -EIO;
		goto free_sector;
	}
	ret = 0;

free_sector:
	free(sector);
	return ret;
}

static int exfat_boot_region_check(struct exfat_blk_dev *blkdev,
				   struct pbr **bs,
				   bool ignore_bad_fs_name)
{
	struct pbr *boot_sect;
	unsigned int sect_size;
	int ret;

	/* First, find out the exfat sector size */
	boot_sect = malloc(sizeof(*boot_sect));
	if (boot_sect == NULL)
		return -ENOMEM;

	if (exfat_read(blkdev->dev_fd, boot_sect,
		       sizeof(*boot_sect), 0) != (ssize_t)sizeof(*boot_sect)) {
		exfat_err("failed to read Main boot sector\n");
		free(boot_sect);
		return -EIO;
	}

	if (memcmp(boot_sect->bpb.oem_name, "EXFAT   ", 8) != 0 &&
	    !ignore_bad_fs_name) {
		exfat_err("Bad fs_name in boot sector, which does not describe a valid exfat filesystem\n");
		free(boot_sect);
		return -ENOTSUP;
	}

	sect_size = 1 << boot_sect->bsx.sect_size_bits;
	free(boot_sect);

	/* check boot regions */
	ret = read_boot_region(blkdev, bs,
			       BOOT_SEC_IDX, sect_size, true);
	if (ret == -EINVAL &&
	    exfat_repair_ask(&exfat_fsck, ER_BS_BOOT_REGION,
			     "boot region is corrupted. try to restore the region from backup"
				)) {
		const unsigned int sector_sizes[] = {512, 4096, 1024, 2048};
		unsigned int i;

		if (sect_size >= 512 && sect_size <= EXFAT_MAX_SECTOR_SIZE) {
			ret = read_boot_region(blkdev, bs,
					       BACKUP_BOOT_SEC_IDX, sect_size,
					       false);
			if (!ret)
				goto restore;
		}

		for (i = 0; i < sizeof(sector_sizes)/sizeof(sector_sizes[0]); i++) {
			if (sector_sizes[i] == sect_size)
				continue;

			ret = read_boot_region(blkdev, bs,
					       BACKUP_BOOT_SEC_IDX,
					       sector_sizes[i], false);
			if (!ret) {
				sect_size = sector_sizes[i];
				goto restore;
			}
		}
		exfat_err("backup boot region is also corrupted\n");
	}

	return ret;
restore:
	ret = restore_boot_region(blkdev, sect_size);
	if (ret) {
		exfat_err("failed to restore boot region from backup\n");
		free(*bs);
		*bs = NULL;
	}
	return ret;
}

static int file_calc_checksum(struct exfat_de_iter *iter, uint16_t *checksum)
{
	struct exfat_dentry *file_de, *de;
	int i, ret;

	*checksum = 0;
	ret = exfat_de_iter_get(iter, 0, &file_de);
	if (ret)
		return ret;

	exfat_calc_dentry_checksum(file_de, checksum, true);
	for (i = 1; i <= file_de->file_num_ext; i++) {
		ret = exfat_de_iter_get(iter, i, &de);
		if (ret)
			return ret;
		exfat_calc_dentry_checksum(de, checksum, false);
	}
	return 0;
}

/*
 * return 0 if there are no errors, or 1 if errors are fixed, or
 * an error code
 */
static int check_inode(struct exfat_de_iter *iter, struct exfat_inode *node)
{
	struct exfat *exfat = iter->exfat;
	struct exfat_dentry *dentry;
	int ret = 0;
	uint16_t checksum;
	bool valid = true;

	ret = check_clus_chain(iter, 1, node);
	if (ret < 0)
		return ret;

	if (node->size > le32_to_cpu(exfat->bs->bsx.clu_count) *
				(uint64_t)exfat->clus_size) {
		fsck_err(iter->parent, node,
			"size %" PRIu64 " is greater than cluster heap\n",
			node->size);
		valid = false;
	}

	if (node->size == 0 && node->is_contiguous) {
		if (repair_file_ask(iter, node, ER_FILE_ZERO_NOFAT,
				"empty, but has no Fat chain")) {
			exfat_de_iter_get_dirty(iter, 1, &dentry);
			dentry->stream_flags &= ~EXFAT_SF_CONTIGUOUS;
			ret = 1;
		} else
			valid = false;
	}

	if ((node->attr & ATTR_SUBDIR) &&
			node->size % exfat->clus_size != 0) {
		fsck_err(iter->parent, node,
			"directory size %" PRIu64 " is not divisible by %d\n",
			node->size, exfat->clus_size);
		valid = false;
	}

	ret = file_calc_checksum(iter, &checksum);
	if (ret)
		return ret;
	exfat_de_iter_get(iter, 0, &dentry);
	if (checksum != le16_to_cpu(dentry->file_checksum)) {
		exfat_de_iter_get_dirty(iter, 0, &dentry);
		dentry->file_checksum = cpu_to_le16(checksum);
		ret = 1;
	}

	return valid ? ret : -EINVAL;
}

static int handle_duplicated_filename(struct exfat_de_iter *iter,
		struct exfat_inode *inode)
{
	int ret;
	struct exfat_lookup_filter filter;
	char filename[PATH_MAX + 1] = {0};

	ret = exfat_lookup_file_by_utf16name(iter->exfat, iter->parent,
			inode->name, &filter);
	if (ret)
		return ret;

	free(filter.out.dentry_set);

	/* Hash is same, but filename is not same */
	if (exfat_de_iter_device_offset(iter) == filter.out.dev_offset)
		return 0;

	ret = exfat_utf16_dec(inode->name, NAME_BUFFER_SIZE, filename,
			PATH_MAX);
	if (ret < 0) {
		exfat_err("failed to decode filename\n");
		return ret;
	}

	return exfat_repair_rename_ask(&exfat_fsck, iter, filename,
			ER_DE_DUPLICATED_NAME, "filename is duplicated");
}

static int check_name_dentry_set(struct exfat_de_iter *iter,
				 struct exfat_inode *inode)
{
	struct exfat_dentry *stream_de;
	size_t name_len;
	__u16 hash;
	int ret = 0;

	exfat_de_iter_get(iter, 1, &stream_de);

	name_len = exfat_utf16_len(inode->name, NAME_BUFFER_SIZE);
	if (stream_de->stream_name_len != name_len) {
		if (repair_file_ask(iter, NULL, ER_DE_NAME_LEN,
				    "the name length of a file is wrong")) {
			exfat_de_iter_get_dirty(iter, 1, &stream_de);
			stream_de->stream_name_len = (__u8)name_len;
			ret = 1;
		} else {
			return -EINVAL;
		}
	}

	hash = exfat_calc_name_hash(iter->exfat, inode->name, (int)name_len);
	if (cpu_to_le16(hash) != stream_de->stream_name_hash) {
		if (repair_file_ask(iter, NULL, ER_DE_NAME_HASH,
				    "the name hash of a file is wrong")) {
			exfat_de_iter_get_dirty(iter, 1, &stream_de);
			stream_de->stream_name_hash = cpu_to_le16(hash);
			ret = 1;
		} else {
			return -EINVAL;
		}
	}

	if (BITMAP_GET(iter->name_hash_bitmap, hash)) {
		ret = handle_duplicated_filename(iter, inode);
	} else
		BITMAP_SET(iter->name_hash_bitmap, hash);

	return ret;
}

const __le16 MSDOS_DOT[ENTRY_NAME_MAX] = {cpu_to_le16(46), 0, };
const __le16 MSDOS_DOTDOT[ENTRY_NAME_MAX] = {cpu_to_le16(46), cpu_to_le16(46), 0, };

static int handle_dot_dotdot_filename(struct exfat_de_iter *iter,
				      struct exfat_dentry *dentry,
				      int strm_name_len)
{
	char *filename;

	if (!memcmp(dentry->name_unicode, MSDOS_DOT, strm_name_len * 2))
		filename = ".";
	else if (!memcmp(dentry->name_unicode, MSDOS_DOTDOT,
			 strm_name_len * 2))
		filename = "..";
	else
		return 0;

	return exfat_repair_rename_ask(&exfat_fsck, iter, filename,
			ER_DE_DOT_NAME, "filename is not allowed");
}

static int read_file_dentry_set(struct exfat_de_iter *iter,
				struct exfat_inode **new_node, int *skip_dentries)
{
	struct exfat_dentry *file_de, *stream_de, *dentry;
	struct exfat_inode *node = NULL;
	int i, j, ret, name_de_count;
	bool need_delete = false, need_copy_up = false;
	uint16_t checksum;

	ret = exfat_de_iter_get(iter, 0, &file_de);
	if (ret || file_de->type != EXFAT_FILE) {
		exfat_err("failed to get file dentry\n");
		return -EINVAL;
	}

	ret = file_calc_checksum(iter, &checksum);
	if (ret || checksum != le16_to_cpu(file_de->file_checksum)) {
		if (repair_file_ask(iter, NULL, ER_DE_CHECKSUM,
				    "the checksum %#x of a file is wrong, expected: %#x",
				    le16_to_cpu(file_de->file_checksum), checksum))
			need_delete = true;
		*skip_dentries = 1;
		goto skip_dset;
	}

	if (file_de->file_num_ext < 2) {
		if (repair_file_ask(iter, NULL, ER_DE_SECONDARY_COUNT,
				    "a file has too few secondary count. %d",
				    file_de->file_num_ext))
			need_delete = true;
		*skip_dentries = 1;
		goto skip_dset;
	}

	ret = exfat_de_iter_get(iter, 1, &stream_de);
	if (ret || stream_de->type != EXFAT_STREAM) {
		if (repair_file_ask(iter, NULL, ER_DE_STREAM,
				    "failed to get stream dentry"))
			need_delete = true;
		*skip_dentries = 2;
		goto skip_dset;
	}

	*new_node = NULL;
	node = exfat_alloc_inode(le16_to_cpu(file_de->file_attr));
	if (!node)
		return -ENOMEM;

	name_de_count = DIV_ROUND_UP(stream_de->stream_name_len, ENTRY_NAME_MAX);
	for (i = 2; i <= MIN(name_de_count + 1, file_de->file_num_ext); i++) {
		ret = exfat_de_iter_get(iter, i, &dentry);
		if (ret || dentry->type != EXFAT_NAME) {
			if (repair_file_ask(iter, NULL, ER_DE_NAME,
					    "failed to get name dentry")) {
				if (i == 2) {
					need_delete = 1;
					*skip_dentries = i + 1;
					goto skip_dset;
				}
				break;
			} else {
				*skip_dentries = i + 1;
				goto skip_dset;
			}
		}

		memcpy(node->name +
		       (i - 2) * ENTRY_NAME_MAX, dentry->name_unicode,
		       sizeof(dentry->name_unicode));
	}

	ret = check_name_dentry_set(iter, node);
	if (ret < 0) {
		*skip_dentries = file_de->file_num_ext + 1;
		goto skip_dset;
	} else if (ret) {
		exfat_de_iter_get(iter, 1, &stream_de);
		if (DIV_ROUND_UP(stream_de->stream_name_len, ENTRY_NAME_MAX) !=
		    name_de_count)
			i = DIV_ROUND_UP(stream_de->stream_name_len, ENTRY_NAME_MAX) + 2;
	}

	if (file_de->file_num_ext == 2 && stream_de->stream_name_len <= 2) {
		ret = handle_dot_dotdot_filename(iter, dentry,
				stream_de->stream_name_len);
		if (ret < 0) {
			*skip_dentries = file_de->file_num_ext + 1;
			goto skip_dset;
		}
	}

	for (j = i; i <= file_de->file_num_ext; i++) {
		exfat_de_iter_get(iter, i, &dentry);
		if (dentry->type == EXFAT_VENDOR_EXT ||
		    dentry->type == EXFAT_VENDOR_ALLOC) {
			char zeroes[EXFAT_GUID_LEN] = {0};
			/*
			 * Vendor GUID should not be zero, But Windows fsck
			 * also does not check and fix it.
			 */
			if (!memcmp(dentry->dentry.vendor_ext.guid,
				    zeroes, EXFAT_GUID_LEN))
				repair_file_ask(iter, NULL, ER_VENDOR_GUID,
						"Vendor Extension has zero filled GUID");
			if (dentry->type == EXFAT_VENDOR_ALLOC) {
				struct exfat_inode *vendor_node;

				/* verify cluster chain */
				vendor_node = exfat_alloc_inode(0);
				if (!vendor_node) {
					*skip_dentries = i + i;
					goto skip_dset;
				}
				vendor_node->first_clus =
					le32_to_cpu(dentry->dentry.vendor_alloc.start_clu);
				vendor_node->is_contiguous = ((dentry->dentry.vendor_alloc.flags
							       & EXFAT_SF_CONTIGUOUS) != 0);
				vendor_node->size =
					le64_to_cpu(dentry->dentry.vendor_alloc.size);
				if (check_clus_chain(iter, i, vendor_node) < 0) {
					exfat_free_inode(vendor_node);
					*skip_dentries = i + 1;
					goto skip_dset;
				}
				if (vendor_node->size == 0 &&
				    vendor_node->is_contiguous) {
					exfat_de_iter_get_dirty(iter, i, &dentry);
					dentry->stream_flags &= ~EXFAT_SF_CONTIGUOUS;

				}
				exfat_free_inode(vendor_node);
			}

			if (need_copy_up) {
				struct exfat_dentry *src_de;

				exfat_de_iter_get_dirty(iter, j, &src_de);
				memcpy(src_de, dentry, sizeof(struct exfat_dentry));
			}
			j++;
		} else {
			if (need_copy_up) {
				continue;
			} else if (repair_file_ask(iter, NULL, ER_DE_UNKNOWN,
						  "unknown entry type %#x", dentry->type)) {
				j = i;
				need_copy_up = true;
			} else {
				*skip_dentries = i + 1;
				goto skip_dset;
			}
		}
	}

	node->first_clus = le32_to_cpu(stream_de->stream_start_clu);
	node->is_contiguous =
		((stream_de->stream_flags & EXFAT_SF_CONTIGUOUS) != 0);
	node->size = le64_to_cpu(stream_de->stream_size);

	if (node->size < le64_to_cpu(stream_de->stream_valid_size)) {
		*skip_dentries = file_de->file_num_ext + 1;
		if (repair_file_ask(iter, node, ER_FILE_VALID_SIZE,
				    "valid size %" PRIu64 " greater than size %" PRIu64,
				    le64_to_cpu(stream_de->stream_valid_size),
				    node->size)) {
			exfat_de_iter_get_dirty(iter, 1, &stream_de);
			stream_de->stream_valid_size =
					stream_de->stream_size;
		} else {
			*skip_dentries = file_de->file_num_ext + 1;
			goto skip_dset;
		}
	}

	if (file_de->file_num_ext != j - 1) {
		if (repair_file_ask(iter, node, ER_DE_SECONDARY_COUNT,
				    "SecondaryCount %d is different with %d",
				    file_de->file_num_ext, j - 1)) {
			exfat_de_iter_get_dirty(iter, 0, &file_de);
			file_de->file_num_ext = j - 1;
		} else {
			*skip_dentries = file_de->file_num_ext + 1;
			goto skip_dset;
		}
	}

	*skip_dentries = (file_de->file_num_ext + 1);
	*new_node = node;
	return 0;
skip_dset:
	if (need_delete) {
		exfat_de_iter_get_dirty(iter, 0, &dentry);
		dentry->type &= EXFAT_DELETE;
	}
	for (i = 1; i < *skip_dentries; i++) {
		exfat_de_iter_get(iter, i, &dentry);
		if (dentry->type == EXFAT_FILE)
			break;
		if (need_delete) {
			exfat_de_iter_get_dirty(iter, i, &dentry);
			dentry->type &= EXFAT_DELETE;
		}
	}
	*skip_dentries = i;
	*new_node = NULL;
	exfat_free_inode(node);
	return need_delete ? 1 : -EINVAL;
}

static int read_file(struct exfat_de_iter *de_iter,
		struct exfat_inode **new_node, int *dentry_count)
{
	struct exfat_inode *node;
	int ret;

	*new_node = NULL;

	ret = read_file_dentry_set(de_iter, &node, dentry_count);
	if (ret)
		return ret;

	ret = check_inode(de_iter, node);
	if (ret < 0) {
		exfat_free_inode(node);
		return -EINVAL;
	}

	if (node->attr & ATTR_SUBDIR)
		exfat_stat.dir_count++;
	else
		exfat_stat.file_count++;
	*new_node = node;
	return ret;
}

static int read_bitmap(struct exfat *exfat)
{
	struct exfat_lookup_filter filter = {
		.in.type	= EXFAT_BITMAP,
		.in.dentry_count = 0,
		.in.filter	= NULL,
		.in.param	= NULL,
	};
	struct exfat_dentry *dentry;
	int retval;

	retval = exfat_lookup_dentry_set(exfat, exfat->root, &filter);
	if (retval)
		return retval;

	dentry = filter.out.dentry_set;
	exfat_debug("start cluster %#x, size %#" PRIx64 "\n",
			le32_to_cpu(dentry->bitmap_start_clu),
			le64_to_cpu(dentry->bitmap_size));

	if (le64_to_cpu(dentry->bitmap_size) <
			DIV_ROUND_UP(exfat->clus_count, 8)) {
		exfat_err("invalid size of allocation bitmap. 0x%" PRIx64 "\n",
				le64_to_cpu(dentry->bitmap_size));
		return -EINVAL;
	}
	if (!exfat_heap_clus(exfat, le32_to_cpu(dentry->bitmap_start_clu))) {
		exfat_err("invalid start cluster of allocate bitmap. 0x%x\n",
				le32_to_cpu(dentry->bitmap_start_clu));
		return -EINVAL;
	}

	exfat->disk_bitmap_clus = le32_to_cpu(dentry->bitmap_start_clu);
	exfat->disk_bitmap_size = DIV_ROUND_UP(exfat->clus_count, 8);

	exfat_bitmap_set_range(exfat, exfat->alloc_bitmap,
			       le64_to_cpu(dentry->bitmap_start_clu),
			       DIV_ROUND_UP(exfat->disk_bitmap_size,
					    exfat->clus_size));
	free(filter.out.dentry_set);

	if (exfat_read(exfat->blk_dev->dev_fd, exfat->disk_bitmap,
			exfat->disk_bitmap_size,
			exfat_c2o(exfat, exfat->disk_bitmap_clus)) !=
			(ssize_t)exfat->disk_bitmap_size)
		return -EIO;
	return 0;
}

static int decompress_upcase_table(const __le16 *in_table, size_t in_len,
				   __u16 *out_table, size_t out_len)
{
	size_t i, k;
	uint16_t ch;

	if (in_len > out_len)
		return -E2BIG;

	for (k = 0; k < out_len; k++)
		out_table[k] = k;

	for (i = 0, k = 0; i < in_len && k < out_len; i++) {
		ch = le16_to_cpu(in_table[i]);

		if (ch == 0xFFFF && i + 1 < in_len) {
			uint16_t len = le16_to_cpu(in_table[++i]);

			k += len;
		} else {
			out_table[k++] = ch;
		}
	}
	return 0;
}

static int read_upcase_table(struct exfat *exfat)
{
	struct exfat_lookup_filter filter = {
		.in.type	= EXFAT_UPCASE,
		.in.dentry_count = 0,
		.in.filter	= NULL,
		.in.param	= NULL,
	};
	struct exfat_dentry *dentry = NULL;
	__le16 *upcase = NULL;
	int retval;
	ssize_t size;
	__le32 checksum;

	retval = exfat_lookup_dentry_set(exfat, exfat->root, &filter);
	if (retval)
		return retval;

	dentry = filter.out.dentry_set;

	if (!exfat_heap_clus(exfat, le32_to_cpu(dentry->upcase_start_clu))) {
		exfat_err("invalid start cluster of upcase table. 0x%x\n",
			le32_to_cpu(dentry->upcase_start_clu));
		retval = -EINVAL;
		goto out;
	}

	size = (ssize_t)le64_to_cpu(dentry->upcase_size);
	if (size > (ssize_t)(EXFAT_MAX_UPCASE_CHARS * sizeof(__le16)) ||
			size == 0 || size % sizeof(__le16)) {
		exfat_err("invalid size of upcase table. 0x%" PRIx64 "\n",
			le64_to_cpu(dentry->upcase_size));
		retval = -EINVAL;
		goto out;
	}

	upcase = malloc(size);
	if (!upcase) {
		exfat_err("failed to allocate upcase table\n");
		retval = -ENOMEM;
		goto out;
	}

	if (exfat_read(exfat->blk_dev->dev_fd, upcase, size,
			exfat_c2o(exfat,
			le32_to_cpu(dentry->upcase_start_clu))) != size) {
		exfat_err("failed to read upcase table\n");
		retval = -EIO;
		goto out;
	}

	checksum = 0;
	boot_calc_checksum((unsigned char *)upcase, size, false, &checksum);
	if (le32_to_cpu(dentry->upcase_checksum) != checksum) {
		exfat_err("corrupted upcase table %#x (expected: %#x)\n",
			checksum, le32_to_cpu(dentry->upcase_checksum));
		retval = -EINVAL;
		goto out;
	}

	exfat_bitmap_set_range(exfat, exfat->alloc_bitmap,
			       le32_to_cpu(dentry->upcase_start_clu),
			       DIV_ROUND_UP(le64_to_cpu(dentry->upcase_size),
					    exfat->clus_size));

	exfat->upcase_table = calloc(EXFAT_UPCASE_TABLE_CHARS, sizeof(uint16_t));
	if (!exfat->upcase_table) {
		retval = -EIO;
		goto out;
	}

	decompress_upcase_table(upcase, size / 2,
				exfat->upcase_table, EXFAT_UPCASE_TABLE_CHARS);
out:
	if (dentry)
		free(dentry);
	if (upcase)
		free(upcase);
	return retval;
}

static int read_children(struct exfat_fsck *fsck, struct exfat_inode *dir)
{
	struct exfat *exfat = fsck->exfat;
	struct exfat_inode *node = NULL;
	struct exfat_dentry *dentry;
	struct exfat_de_iter *de_iter;
	int dentry_count;
	int ret;

	de_iter = &fsck->de_iter;
	ret = exfat_de_iter_init(de_iter, exfat, dir, fsck->buffer_desc);
	if (ret == EOF)
		return 0;
	else if (ret)
		return ret;

	de_iter->name_hash_bitmap = fsck->name_hash_bitmap;
	memset(fsck->name_hash_bitmap, 0,
			EXFAT_BITMAP_SIZE(EXFAT_MAX_HASH_COUNT));

	while (1) {
		ret = exfat_de_iter_get(de_iter, 0, &dentry);
		if (ret == EOF) {
			break;
		} else if (ret) {
			fsck_err(dir->parent, dir,
				"failed to get a dentry. %d\n", ret);
			goto err;
		}

		dentry_count = 1;

		switch (dentry->type) {
		case EXFAT_FILE:
			ret = read_file(de_iter, &node, &dentry_count);
			if (ret < 0) {
				exfat_stat.error_count++;
				break;
			} else if (ret) {
				exfat_stat.error_count++;
				exfat_stat.fixed_count++;
			}

			if (node) {
				if ((node->attr & ATTR_SUBDIR) && node->size) {
					node->parent = dir;
					list_add_tail(&node->sibling,
						      &dir->children);
					list_add_tail(&node->list,
						      &exfat->dir_list);
				} else {
					exfat_free_inode(node);
				}
			}
			break;
		case EXFAT_LAST:
			goto out;
		case EXFAT_VOLUME:
		case EXFAT_BITMAP:
		case EXFAT_UPCASE:
		case EXFAT_GUID:
			if (dir == exfat->root)
				break;
			/* fallthrough */
		default:
			if (IS_EXFAT_DELETED(dentry->type))
				break;
			if (repair_file_ask(de_iter, NULL, ER_DE_UNKNOWN,
					    "unknown entry type %#x", dentry->type)) {
				struct exfat_dentry *dentry;

				exfat_de_iter_get_dirty(de_iter, 0, &dentry);
				dentry->type &= EXFAT_DELETE;
			}
			break;
		}

		exfat_de_iter_advance(de_iter, dentry_count);
	}
out:
	exfat_de_iter_flush(de_iter);
	return 0;
err:
	exfat_free_children(dir, false);
	INIT_LIST_HEAD(&dir->children);
	exfat_de_iter_flush(de_iter);
	return ret;
}

/* write bitmap segments for clusters which are marked
 * as free, but allocated to files.
 */
static int write_bitmap(struct exfat_fsck *fsck)
{
	struct exfat *exfat = fsck->exfat;
	bitmap_t *disk_b, *alloc_b, *ohead_b;
	off_t dev_offset;
	unsigned int i, bitmap_bytes, byte_offset, write_bytes;

	dev_offset = exfat_c2o(exfat, exfat->disk_bitmap_clus);
	bitmap_bytes = EXFAT_BITMAP_SIZE(le32_to_cpu(exfat->bs->bsx.clu_count));

	disk_b = (bitmap_t *)exfat->disk_bitmap;
	alloc_b = (bitmap_t *)exfat->alloc_bitmap;
	ohead_b = (bitmap_t *)exfat->ohead_bitmap;

	for (i = 0; i < bitmap_bytes / sizeof(bitmap_t); i++)
		ohead_b[i] = alloc_b[i] | disk_b[i];

	i = 0;
	while (i < bitmap_bytes / sizeof(bitmap_t)) {
		if (ohead_b[i] == disk_b[i]) {
			i++;
			continue;
		}

		byte_offset = ((i * sizeof(bitmap_t)) / 512) * 512;
		write_bytes = MIN(512, bitmap_bytes - byte_offset);

		if (exfat_write(exfat->blk_dev->dev_fd,
				(char *)ohead_b + byte_offset, write_bytes,
				dev_offset + byte_offset) != (ssize_t)write_bytes)
			return -EIO;

		i = (byte_offset + write_bytes) / sizeof(bitmap_t);
	}
	return 0;

}

/*
 * for each directory in @dir_list.
 * 1. read all dentries and allocate exfat_nodes for files and directories.
 *    and append directory exfat_nodes to the head of @dir_list
 * 2. free all of file exfat_nodes.
 * 3. if the directory does not have children, free its exfat_node.
 */
static int exfat_filesystem_check(struct exfat_fsck *fsck)
{
	struct exfat *exfat = fsck->exfat;
	struct exfat_inode *dir;
	int ret = 0, dir_errors;

	if (!exfat->root) {
		exfat_err("root is NULL\n");
		return -ENOENT;
	}

	fsck->name_hash_bitmap = malloc(EXFAT_BITMAP_SIZE(EXFAT_MAX_HASH_COUNT));
	if (!fsck->name_hash_bitmap) {
		exfat_err("failed to allocate name hash bitmap\n");
		return -ENOMEM;
	}

	list_add(&exfat->root->list, &exfat->dir_list);

	while (!list_empty(&exfat->dir_list)) {
		dir = list_entry(exfat->dir_list.next,
				 struct exfat_inode, list);

		if (!(dir->attr & ATTR_SUBDIR)) {
			fsck_err(dir->parent, dir,
				"failed to travel directories. "
				"the node is not directory\n");
			ret = -EINVAL;
			goto out;
		}

		dir_errors = read_children(fsck, dir);
		if (dir_errors) {
			exfat_resolve_path(&path_resolve_ctx, dir);
			exfat_debug("failed to check dentries: %s\n",
					path_resolve_ctx.local_path);
			ret = dir_errors;
		}

		list_del(&dir->list);
		exfat_free_file_children(dir);
		exfat_free_ancestors(dir);
	}
out:
	exfat_free_dir_list(exfat);
	free(fsck->name_hash_bitmap);
	return ret;
}

static int exfat_root_dir_check(struct exfat *exfat)
{
	struct exfat_inode *root;
	clus_t clus_count = 0;
	int err;

	root = exfat_alloc_inode(ATTR_SUBDIR);
	if (!root)
		return -ENOMEM;

	exfat->root = root;
	root->first_clus = le32_to_cpu(exfat->bs->bsx.root_cluster);
	if (root_check_clus_chain(exfat, root, &clus_count)) {
		exfat_err("failed to follow the cluster chain of root\n");
		exfat_free_inode(root);
		exfat->root = NULL;
		return -EINVAL;
	}
	root->size = clus_count * exfat->clus_size;

	exfat_stat.dir_count++;
	exfat_debug("root directory: start cluster[0x%x] size[0x%" PRIx64 "]\n",
		root->first_clus, root->size);

	err = exfat_read_volume_label(exfat);
	if (err && err != EOF)
		exfat_err("failed to read volume label\n");

	err = read_bitmap(exfat);
	if (err) {
		exfat_err("failed to read bitmap\n");
		return -EINVAL;
	}

	err = read_upcase_table(exfat);
	if (err) {
		exfat_err("failed to read upcase table\n");
		return -EINVAL;
	}

	root->dev_offset = 0;
	err = exfat_build_file_dentry_set(exfat, " ", ATTR_SUBDIR,
					  &root->dentry_set, &root->dentry_count);
	if (err) {
		exfat_free_inode(root);
		return -ENOMEM;
	}
	return 0;
}

static int read_lostfound(struct exfat *exfat, struct exfat_inode **lostfound)
{
	struct exfat_lookup_filter filter;
	struct exfat_inode *inode;
	int err;

	err = exfat_lookup_file(exfat, exfat->root, "LOST+FOUND", &filter);
	if (err)
		return err;

	inode = exfat_alloc_inode(ATTR_SUBDIR);
	if (!inode) {
		free(filter.out.dentry_set);
		return -ENOMEM;
	}

	inode->dentry_set = filter.out.dentry_set;
	inode->dentry_count = filter.out.dentry_count;
	inode->dev_offset = filter.out.dev_offset;

	inode->first_clus =
		le32_to_cpu(filter.out.dentry_set[1].dentry.stream.start_clu);
	inode->size =
		le64_to_cpu(filter.out.dentry_set[1].dentry.stream.size);

	*lostfound = inode;
	return 0;
}

/* Create temporary files under LOST+FOUND and assign orphan
 * chains of clusters to these files.
 */
static int rescue_orphan_clusters(struct exfat_fsck *fsck)
{
	struct exfat *exfat = fsck->exfat;
	struct exfat_inode *lostfound;
	bitmap_t *disk_b, *alloc_b, *ohead_b;
	struct exfat_dentry *dset;
	clus_t clu_count, clu, s_clu, e_clu;
	int err, dcount;
	unsigned int i;
	char name[] = "FILE0000000.CHK";
	struct exfat_dentry_loc loc;
	struct exfat_lookup_filter lf = {
		.in.type = EXFAT_INVAL,
		.in.dentry_count = 0,
		.in.filter = NULL,
	};

	clu_count = le32_to_cpu(exfat->bs->bsx.clu_count);

	/* find clusters which are not marked as free, but not allocated to
	 * any files.
	 */
	disk_b = (bitmap_t *)exfat->disk_bitmap;
	alloc_b = (bitmap_t *)exfat->alloc_bitmap;
	ohead_b = (bitmap_t *)exfat->ohead_bitmap;
	for (i = 0; i < EXFAT_BITMAP_SIZE(clu_count) / sizeof(bitmap_t); i++)
		ohead_b[i] = disk_b[i] & ~alloc_b[i];

	/* no orphan clusters */
	if (exfat_bitmap_find_one(exfat, exfat->ohead_bitmap,
				EXFAT_FIRST_CLUSTER, &s_clu))
		return 0;

	err = exfat_create_file(exfat_fsck.exfat,
				exfat_fsck.exfat->root,
				"LOST+FOUND",
				ATTR_SUBDIR);
	if (err) {
		exfat_err("failed to create LOST+FOUND directory\n");
		return err;
	}

	if (fsync(exfat_fsck.exfat->blk_dev->dev_fd) != 0) {
		exfat_err("failed to sync()\n");
		return -EIO;
	}

	err = read_lostfound(exfat, &lostfound);
	if (err) {
		exfat_err("failed to find LOST+FOUND\n");
		return err;
	}

	/* get the last empty region of LOST+FOUND */
	err = exfat_lookup_dentry_set(exfat, lostfound, &lf);
	if (err && err != EOF) {
		exfat_err("failed to find the last empty slot in LOST+FOUND\n");
		goto out;
	}

	loc.parent = lostfound;
	loc.file_offset = lf.out.file_offset;
	loc.dev_offset = lf.out.dev_offset;

	/* build a template dentry set */
	err = exfat_build_file_dentry_set(exfat, name, 0, &dset, &dcount);
	if (err) {
		exfat_err("failed to create a temporary file in LOST+FOUNDn");
		goto out;
	}
	dset[1].dentry.stream.flags |= EXFAT_SF_CONTIGUOUS;

	/* create temporary files and allocate contiguous orphan clusters
	 * to each file.
	 */
	for (clu = EXFAT_FIRST_CLUSTER; clu < clu_count + EXFAT_FIRST_CLUSTER &&
	     exfat_bitmap_find_one(exfat, exfat->ohead_bitmap, clu, &s_clu) == 0;) {
		if (exfat_bitmap_find_zero(exfat, exfat->ohead_bitmap, s_clu, &e_clu))
			e_clu = clu_count + EXFAT_FIRST_CLUSTER;
		clu = e_clu;

		snprintf(name, sizeof(name), "FILE%07d.CHK",
			 (unsigned int)(loc.file_offset >> 5));
		err = exfat_update_file_dentry_set(exfat, dset, dcount,
						   name, s_clu, e_clu - s_clu);
		if (err)
			continue;
		err = exfat_add_dentry_set(exfat, &loc, dset, dcount, true);
		if (err)
			continue;
	}

	free(dset);
	err = 0;
out:
	exfat_free_inode(lostfound);
	return err;
}

static char *bytes_to_human_readable(size_t bytes)
{
	static const char * const units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
	static char buf[15*4];
	unsigned int i, shift, quoti, remain;
	i = sizeof(units) / sizeof(units[0]) - 1;

	while (i && (bytes >> i * 10) == 0)
		i--;

	shift = i * 10;
	quoti = (unsigned int)(bytes / (1ULL << shift));
	remain = 0;
	if (shift > 0) {
		remain = (unsigned int)
			((bytes & ((1ULL << shift) - 1)) >> (shift - 10));
		remain = (remain * 100) / 1024;
	}

	snprintf(buf, sizeof(buf), "%u.%02u %s", quoti, remain, units[i]);
	return buf;
}

static void exfat_show_info(struct exfat_fsck *fsck, const char *dev_name)
{
	struct exfat *exfat = fsck->exfat;
	bool clean;

	exfat_info("sector size:  %s\n",
		bytes_to_human_readable(1 << exfat->bs->bsx.sect_size_bits));
	exfat_info("cluster size: %s\n",
		bytes_to_human_readable(exfat->clus_size));
	exfat_info("volume size:  %s\n",
		bytes_to_human_readable(exfat->blk_dev->size));

	clean = exfat_stat.error_count == 0 ||
		exfat_stat.error_count == exfat_stat.fixed_count;
	printf("%s: %s. directories %ld, files %ld\n", dev_name,
			clean ? "clean" : "corrupted",
			exfat_stat.dir_count, exfat_stat.file_count);
	if (exfat_stat.error_count)
		printf("%s: files corrupted %ld, files fixed %ld\n", dev_name,
			exfat_stat.error_count - exfat_stat.fixed_count,
			exfat_stat.fixed_count);
}

int main(int argc, char * const argv[])
{
	struct fsck_user_input ui;
	struct exfat_blk_dev bd;
	struct pbr *bs = NULL;
	int c, ret, exit_code;
	bool version_only = false;

	memset(&ui, 0, sizeof(ui));
	memset(&bd, 0, sizeof(bd));

	print_level = EXFAT_ERROR;

	if (!setlocale(LC_CTYPE, ""))
		exfat_err("failed to init locale/codeset\n");

	opterr = 0;
	while ((c = getopt_long(argc, argv, "arynpbsVvh", opts, NULL)) != EOF) {
		switch (c) {
		case 'n':
			if (ui.options & FSCK_OPTS_REPAIR_ALL)
				usage(argv[0]);
			ui.options |= FSCK_OPTS_REPAIR_NO;
			break;
		case 'r':
			if (ui.options & FSCK_OPTS_REPAIR_ALL)
				usage(argv[0]);
			ui.options |= FSCK_OPTS_REPAIR_ASK;
			break;
		case 'y':
			if (ui.options & FSCK_OPTS_REPAIR_ALL)
				usage(argv[0]);
			ui.options |= FSCK_OPTS_REPAIR_YES;
			break;
		case 'a':
		case 'p':
			if (ui.options & FSCK_OPTS_REPAIR_ALL)
				usage(argv[0]);
			ui.options |= FSCK_OPTS_REPAIR_AUTO;
			break;
		case 'b':
			ui.options |= FSCK_OPTS_IGNORE_BAD_FS_NAME;
			break;
		case 's':
			ui.options |= FSCK_OPTS_RESCUE_CLUS;
			break;
		case 'V':
			version_only = true;
			break;
		case 'v':
			if (print_level < EXFAT_DEBUG)
				print_level++;
			break;
		case '?':
		case 'h':
		default:
			usage(argv[0]);
		}
	}

	show_version();
	if (optind != argc - 1)
		usage(argv[0]);

	if (version_only)
		exit(FSCK_EXIT_SYNTAX_ERROR);
	if (ui.options & FSCK_OPTS_REPAIR_WRITE)
		ui.ei.writeable = true;
	else {
		if (ui.options & (FSCK_OPTS_IGNORE_BAD_FS_NAME |
				  FSCK_OPTS_RESCUE_CLUS))
			usage(argv[0]);
		ui.options |= FSCK_OPTS_REPAIR_NO;
		ui.ei.writeable = false;
	}

	exfat_fsck.options = ui.options;

	snprintf(ui.ei.dev_name, sizeof(ui.ei.dev_name), "%s", argv[optind]);
	ret = exfat_get_blk_dev_info(&ui.ei, &bd);
	if (ret < 0) {
		exfat_err("failed to open %s. %d\n", ui.ei.dev_name, ret);
		return FSCK_EXIT_OPERATION_ERROR;
	}

	ret = exfat_boot_region_check(&bd, &bs,
				      ui.options & FSCK_OPTS_IGNORE_BAD_FS_NAME ?
				      true : false);
	if (ret)
		goto err;

	exfat_fsck.exfat = exfat_alloc_exfat(&bd, bs);
	if (!exfat_fsck.exfat) {
		ret = -ENOMEM;
		goto err;
	}

	exfat_fsck.buffer_desc = exfat_alloc_buffer(exfat_fsck.exfat);
	if (!exfat_fsck.buffer_desc) {
		ret = -ENOMEM;
		goto err;
	}

	if ((exfat_fsck.options & FSCK_OPTS_REPAIR_WRITE) &&
	    exfat_mark_volume_dirty(exfat_fsck.exfat, true)) {
		ret = -EIO;
		goto err;
	}

	exfat_debug("verifying root directory...\n");
	ret = exfat_root_dir_check(exfat_fsck.exfat);
	if (ret) {
		exfat_err("failed to verify root directory.\n");
		goto out;
	}

	exfat_debug("verifying directory entries...\n");
	ret = exfat_filesystem_check(&exfat_fsck);
	if (ret)
		goto out;

	if (exfat_fsck.options & FSCK_OPTS_RESCUE_CLUS) {
		rescue_orphan_clusters(&exfat_fsck);
		exfat_fsck.dirty = true;
		exfat_fsck.dirty_fat = true;
	}

	if (exfat_fsck.options & FSCK_OPTS_REPAIR_WRITE) {
		ret = write_bitmap(&exfat_fsck);
		if (ret) {
			exfat_err("failed to write bitmap\n");
			goto out;
		}
	}

	if (ui.ei.writeable && fsync(bd.dev_fd)) {
		exfat_err("failed to sync\n");
		ret = -EIO;
		goto out;
	}
	if (exfat_fsck.options & FSCK_OPTS_REPAIR_WRITE)
		exfat_mark_volume_dirty(exfat_fsck.exfat, false);

out:
	exfat_show_info(&exfat_fsck, ui.ei.dev_name);
err:
	if (ret && ret != -EINVAL)
		exit_code = FSCK_EXIT_OPERATION_ERROR;
	else if (ret == -EINVAL ||
		 exfat_stat.error_count != exfat_stat.fixed_count)
		exit_code = FSCK_EXIT_ERRORS_LEFT;
	else if (exfat_fsck.dirty)
		exit_code = FSCK_EXIT_CORRECTED;
	else
		exit_code = FSCK_EXIT_NO_ERRORS;

	if (exfat_fsck.buffer_desc)
		exfat_free_buffer(exfat_fsck.exfat, exfat_fsck.buffer_desc);
	if (exfat_fsck.exfat)
		exfat_free_exfat(exfat_fsck.exfat);
	close(bd.dev_fd);
	return exit_code;
}
