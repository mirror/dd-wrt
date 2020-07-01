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
#include "fsck.h"
#include "repair.h"

struct fsck_user_input {
	struct exfat_user_input		ei;
	enum fsck_ui_options		options;
};

#define EXFAT_MAX_UPCASE_CHARS	0x10000

#ifdef WORDS_BIGENDIAN
typedef __u8	bitmap_t;
#else
typedef __u32	bitmap_t;
#endif

#define BITS_PER	(sizeof(bitmap_t) * 8)
#define BIT_MASK(__c)	(1 << ((__c) % BITS_PER))
#define BIT_ENTRY(__c)	((__c) / BITS_PER)

#define EXFAT_BITMAP_SIZE(__c_count)	\
	(DIV_ROUND_UP(__c_count, BITS_PER) * sizeof(bitmap_t))
#define EXFAT_BITMAP_GET(__bmap, __c)	\
			(((bitmap_t *)(__bmap))[BIT_ENTRY(__c)] & BIT_MASK(__c))
#define EXFAT_BITMAP_SET(__bmap, __c)	\
			(((bitmap_t *)(__bmap))[BIT_ENTRY(__c)] |= \
			 BIT_MASK(__c))

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
	long		dir_free_count;
	long		file_free_count;
};

struct path_resolve_ctx {
	struct exfat_inode	*ancestors[255];
	__le16			utf16_path[PATH_MAX + 2];
	char			local_path[PATH_MAX * MB_LEN_MAX + 1];
};

struct exfat_stat exfat_stat;
struct path_resolve_ctx path_resolve_ctx;

static struct option opts[] = {
	{"repair",	no_argument,	NULL,	'r' },
	{"repair-yes",	no_argument,	NULL,	'y' },
	{"repair-no",	no_argument,	NULL,	'n' },
	{"repair-auto",	no_argument,	NULL,	'p' },
	{"version",	no_argument,	NULL,	'V' },
	{"verbose",	no_argument,	NULL,	'v' },
	{"help",	no_argument,	NULL,	'h' },
	{"?",		no_argument,	NULL,	'?' },
	{NULL,		0,		NULL,	 0  }
};

static void usage(char *name)
{
	fprintf(stderr, "Usage: %s\n", name);
	fprintf(stderr, "\t-r | --repair        Repair interactively\n");
	fprintf(stderr, "\t-y | --repair-yes    Repair without ask\n");
	fprintf(stderr, "\t-n | --repair-no     No repair\n");
	fprintf(stderr, "\t-p | --repair-auto   Repair automatically\n");
	fprintf(stderr, "\t-V | --version       Show version\n");
	fprintf(stderr, "\t-v | --verbose       Print debug\n");
	fprintf(stderr, "\t-h | --help          Show help\n");

	exit(FSCK_EXIT_SYNTAX_ERROR);
}

static struct exfat_inode *alloc_exfat_inode(__u16 attr)
{
	struct exfat_inode *node;
	int size;

	size = offsetof(struct exfat_inode, name) + NAME_BUFFER_SIZE;
	node = (struct exfat_inode *)calloc(1, size);
	if (!node) {
		exfat_err("failed to allocate exfat_node\n");
		return NULL;
	}

	node->parent = NULL;
	INIT_LIST_HEAD(&node->children);
	INIT_LIST_HEAD(&node->sibling);
	INIT_LIST_HEAD(&node->list);

	node->last_pclus = EXFAT_EOF_CLUSTER;
	node->attr = attr;
	if (attr & ATTR_SUBDIR)
		exfat_stat.dir_count++;
	else
		exfat_stat.file_count++;
	return node;
}

static void free_exfat_inode(struct exfat_inode *node)
{
	if (node->attr & ATTR_SUBDIR)
		exfat_stat.dir_free_count++;
	else
		exfat_stat.file_free_count++;
	free(node);
}

static void inode_free_children(struct exfat_inode *dir, bool file_only)
{
	struct exfat_inode *node, *i;

	list_for_each_entry_safe(node, i, &dir->children, sibling) {
		if (file_only) {
			if (!(node->attr & ATTR_SUBDIR)) {
				list_del(&node->sibling);
				free_exfat_inode(node);
			}
		} else {
			list_del(&node->sibling);
			list_del(&node->list);
			free_exfat_inode(node);
		}
	}
}

static void inode_free_file_children(struct exfat_inode *dir)
{
	inode_free_children(dir, true);
}

/* delete @child and all ancestors that does not have
 * children
 */
static void inode_free_ancestors(struct exfat_inode *child)
{
	struct exfat_inode *parent;

	if (!list_empty(&child->children))
		return;

	do {
		if (!(child->attr & ATTR_SUBDIR)) {
			exfat_err("not directory.\n");
			return;
		}

		parent = child->parent;
		list_del(&child->sibling);
		free_exfat_inode(child);

		child = parent;
	} while (child && list_empty(&child->children));

	return;
}

static void free_exfat(struct exfat *exfat)
{
	int i;

	if (exfat) {
		if (exfat->bs)
			free(exfat->bs);
		if (exfat->alloc_bitmap)
			free(exfat->alloc_bitmap);
		if (exfat->disk_bitmap)
			free(exfat->disk_bitmap);
		for (i = 0; i < 2; i++) {
			if (exfat->buffer_desc[i].buffer)
				free(exfat->buffer_desc[i].buffer);
			if (exfat->buffer_desc[i].dirty)
				free(exfat->buffer_desc[i].dirty);
		}
		free(exfat);
	}
}

static struct exfat *alloc_exfat(struct exfat_blk_dev *bd, struct pbr *bs)
{
	struct exfat *exfat;
	int i;

	exfat = (struct exfat *)calloc(1, sizeof(*exfat));
	if (!exfat) {
		free(bs);
		exfat_err("failed to allocate exfat\n");
		return NULL;
	}

	INIT_LIST_HEAD(&exfat->dir_list);
	exfat->blk_dev = bd;
	exfat->bs = bs;
	exfat->clus_count = le32_to_cpu(bs->bsx.clu_count);
	exfat->clus_size = EXFAT_CLUSTER_SIZE(bs);
	exfat->sect_size = EXFAT_SECTOR_SIZE(bs);

	/* TODO: bitmap could be very large. */
	exfat->alloc_bitmap = (char *)calloc(1,
			EXFAT_BITMAP_SIZE(exfat->clus_count));
	if (!exfat->alloc_bitmap) {
		exfat_err("failed to allocate bitmap\n");
		goto err;
	}

	exfat->disk_bitmap = (char *)malloc(
				EXFAT_BITMAP_SIZE(exfat->clus_count));
	if (!exfat->disk_bitmap) {
		exfat_err("failed to allocate bitmap\n");
		goto err;
	}

	/* allocate cluster buffers */
	for (i = 0; i < 2; i++) {
		exfat->buffer_desc[i].buffer =
			(char *)malloc(exfat->clus_size);
		if (!exfat->buffer_desc[i].buffer)
			goto err;
		exfat->buffer_desc[i].dirty =
			(char *)calloc(
			(exfat->clus_size / exfat->sect_size), 1);
		if (!exfat->buffer_desc[i].dirty)
			goto err;
	}
	return exfat;
err:
	free_exfat(exfat);
	return NULL;
}

static void exfat_free_dir_list(struct exfat *exfat)
{
	struct exfat_inode *dir, *i;

	list_for_each_entry_safe(dir, i, &exfat->dir_list, list) {
		inode_free_file_children(dir);
		list_del(&dir->list);
		free_exfat_inode(dir);
	}
}

/*
 * get references of ancestors that include @child until the count of
 * ancesters is not larger than @count and the count of characters of
 * their names is not larger than @max_char_len.
 * return true if root is reached.
 */
bool get_ancestors(struct exfat_inode *child,
		struct exfat_inode **ancestors, int count,
		int max_char_len,
		int *ancestor_count)
{
	struct exfat_inode *dir;
	int name_len, char_len;
	int root_depth, depth, i;

	root_depth = 0;
	char_len = 0;
	max_char_len += 1;

	dir = child;
	while (dir) {
		name_len = exfat_utf16_len(dir->name, NAME_BUFFER_SIZE);
		if (char_len + name_len > max_char_len)
			break;

		/* include '/' */
		char_len += name_len + 1;
		root_depth++;

		dir = dir->parent;
	}

	depth = MIN(root_depth, count);

	for (dir = child, i = depth - 1; i >= 0; dir = dir->parent, i--)
		ancestors[i] = dir;

	*ancestor_count = depth;
	return dir == NULL;
}

static int resolve_path(struct path_resolve_ctx *ctx, struct exfat_inode *child)
{
	int depth, i;
	int name_len;
	__le16 *utf16_path;
	static const __le16 utf16_slash = cpu_to_le16(0x002F);
	static const __le16 utf16_null = cpu_to_le16(0x0000);
	size_t in_size;

	ctx->local_path[0] = '\0';

	get_ancestors(child,
			ctx->ancestors,
			sizeof(ctx->ancestors) / sizeof(ctx->ancestors[0]),
			PATH_MAX,
			&depth);

	utf16_path = ctx->utf16_path;
	for (i = 0; i < depth; i++) {
		name_len = exfat_utf16_len(ctx->ancestors[i]->name,
				NAME_BUFFER_SIZE);
		memcpy((char *)utf16_path, (char *)ctx->ancestors[i]->name,
				name_len * 2);
		utf16_path += name_len;
		memcpy((char *)utf16_path, &utf16_slash, sizeof(utf16_slash));
		utf16_path++;
	}

	if (depth > 0)
		utf16_path--;
	memcpy((char *)utf16_path, &utf16_null, sizeof(utf16_null));
	utf16_path++;

	in_size = (utf16_path - ctx->utf16_path) * sizeof(__le16);
	return exfat_utf16_dec(ctx->utf16_path, in_size,
				ctx->local_path, sizeof(ctx->local_path));
}

static int resolve_path_parent(struct path_resolve_ctx *ctx,
			struct exfat_inode *parent, struct exfat_inode *child)
{
	int ret;
	struct exfat_inode *old;

	old = child->parent;
	child->parent = parent;

	ret = resolve_path(ctx, child);
	child->parent = old;
	return ret;
}

#define repair_file_ask(iter, inode, code, fmt, ...)	\
({							\
		resolve_path_parent(&path_resolve_ctx,	\
				(iter)->parent, inode);	\
		exfat_repair_ask((iter)->exfat, code,	\
			"%s: " fmt,			\
			path_resolve_ctx.local_path,	\
			##__VA_ARGS__);			\
})

static inline bool heap_clus(struct exfat *exfat, clus_t clus)
{
	return clus >= EXFAT_FIRST_CLUSTER &&
		(clus - EXFAT_FIRST_CLUSTER) < exfat->clus_count;
}

int get_next_clus(struct exfat *exfat, struct exfat_inode *node,
				clus_t clus, clus_t *next)
{
	off_t offset;

	*next = EXFAT_EOF_CLUSTER;

	if (!heap_clus(exfat, clus))
		return -EINVAL;

	if (node->is_contiguous) {
		*next = clus + 1;
		return 0;
	}

	offset = le32_to_cpu(exfat->bs->bsx.fat_offset) <<
				exfat->bs->bsx.sect_size_bits;
	offset += sizeof(clus_t) * clus;

	if (exfat_read(exfat->blk_dev->dev_fd, next, sizeof(*next), offset)
			!= sizeof(*next))
		return -EIO;
	*next = le32_to_cpu(*next);
	return 0;
}

static int set_fat(struct exfat *exfat, clus_t clus, clus_t next_clus)
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

static int check_clus_chain(struct exfat *exfat, struct exfat_inode *node)
{
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
		(node->size > 0 && !heap_clus(exfat, node->first_clus))) {
		if (repair_file_ask(&exfat->de_iter, node,
			ER_FILE_FIRST_CLUS, "first cluster is wrong"))
			goto truncate_file;
		else
			return -EINVAL;
	}

	while (clus != EXFAT_EOF_CLUSTER) {
		if (count >= max_count) {
			if (node->is_contiguous)
				break;
			if (repair_file_ask(&exfat->de_iter, node,
					ER_FILE_SMALLER_SIZE,
					"more clusters are allocated. "
					"truncate to %" PRIu64 " bytes",
					count * exfat->clus_size))
				goto truncate_file;
			else
				return -EINVAL;
		}

		/*
		 * This cluster is already allocated. it may be shared with
		 * the other file, or there is a loop in cluster chain.
		 */
		if (EXFAT_BITMAP_GET(exfat->alloc_bitmap,
				clus - EXFAT_FIRST_CLUSTER)) {
			if (repair_file_ask(&exfat->de_iter, node,
					ER_FILE_DUPLICATED_CLUS,
					"cluster is already allocated for "
					"the other file. truncated to %"
					PRIu64 " bytes",
					count * exfat->clus_size))
				goto truncate_file;
			else
				return -EINVAL;
		}

		/* This cluster is allocated or not */
		if (get_next_clus(exfat, node, clus, &next))
			goto truncate_file;
		if (!node->is_contiguous) {
			if (!heap_clus(exfat, next) &&
					next != EXFAT_EOF_CLUSTER) {
				if (repair_file_ask(&exfat->de_iter, node,
						ER_FILE_INVALID_CLUS,
						"broken cluster chain. "
						"truncate to %"
						PRIu64 " bytes",
						count * exfat->clus_size))
					goto truncate_file;

				else
					return -EINVAL;
			}
		} else {
			if (!EXFAT_BITMAP_GET(exfat->disk_bitmap,
					clus - EXFAT_FIRST_CLUSTER)) {
				if (repair_file_ask(&exfat->de_iter, node,
						ER_FILE_INVALID_CLUS,
						"cluster is marked as free. "
						"truncate to %"
						PRIu64 " bytes",
						count * exfat->clus_size))
					goto truncate_file;

				else
					return -EINVAL;
			}
		}

		count++;
		EXFAT_BITMAP_SET(exfat->alloc_bitmap,
				clus - EXFAT_FIRST_CLUSTER);
		prev = clus;
		clus = next;
	}

	if (count < max_count) {
		if (repair_file_ask(&exfat->de_iter, node,
			ER_FILE_LARGER_SIZE, "less clusters are allocated. "
			"truncates to %" PRIu64 " bytes",
			count * exfat->clus_size))
			goto truncate_file;
		else
			return -EINVAL;
	}

	return 0;
truncate_file:
	node->size = count * exfat->clus_size;
	if (!heap_clus(exfat, prev))
		node->first_clus = EXFAT_FREE_CLUSTER;

	exfat_de_iter_get_dirty(&exfat->de_iter, 1, &stream_de);
	if (count * exfat->clus_size <
			le64_to_cpu(stream_de->stream_valid_size))
		stream_de->stream_valid_size = cpu_to_le64(
				count * exfat->clus_size);
	if (!heap_clus(exfat, prev))
		stream_de->stream_start_clu = EXFAT_FREE_CLUSTER;
	stream_de->stream_size = cpu_to_le64(
			count * exfat->clus_size);

	/* remaining clusters will be freed while FAT is compared with
	 * alloc_bitmap.
	 */
	if (!node->is_contiguous && heap_clus(exfat, prev))
		return set_fat(exfat, prev, EXFAT_EOF_CLUSTER);
	return 0;
}

static bool root_get_clus_count(struct exfat *exfat, struct exfat_inode *node,
							clus_t *clus_count)
{
	clus_t clus;

	clus = node->first_clus;
	*clus_count = 0;

	do {
		if (!heap_clus(exfat, clus)) {
			exfat_err("/: bad cluster. 0x%x\n", clus);
			return false;
		}

		if (EXFAT_BITMAP_GET(exfat->alloc_bitmap,
				clus - EXFAT_FIRST_CLUSTER)) {
			resolve_path(&path_resolve_ctx, node);
			exfat_err("/: cluster is already allocated, or "
				"there is a loop in cluster chain\n");
			return false;
		}

		EXFAT_BITMAP_SET(exfat->alloc_bitmap,
				clus - EXFAT_FIRST_CLUSTER);

		if (get_next_clus(exfat, node, clus, &clus) != 0) {
			exfat_err("/: broken cluster chain\n");
			return false;
		}

		(*clus_count)++;
	} while (clus != EXFAT_EOF_CLUSTER);
	return true;
}

static off_t exfat_s2o(struct exfat *exfat, off_t sect)
{
	return sect << exfat->bs->bsx.sect_size_bits;
}

off_t exfat_c2o(struct exfat *exfat, unsigned int clus)
{
	if (clus < EXFAT_FIRST_CLUSTER)
		return ~0L;

	return exfat_s2o(exfat, le32_to_cpu(exfat->bs->bsx.clu_offset) +
				((clus - EXFAT_FIRST_CLUSTER) <<
				 exfat->bs->bsx.sect_per_clus_bits));
}

static int boot_region_checksum(struct exfat *exfat)
{
	void *sect;
	unsigned int i;
	uint32_t checksum;
	int ret = 0;
	unsigned short size;

	size = EXFAT_SECTOR_SIZE(exfat->bs);
	sect = malloc(size);
	if (!sect)
		return -ENOMEM;

	checksum = 0;

	boot_calc_checksum((unsigned char *)exfat->bs, size, true, &checksum);
	for (i = 1; i < 11; i++) {
		if (exfat_read(exfat->blk_dev->dev_fd, sect, size, i * size) !=
				(ssize_t)size) {
			ret = -EIO;
			goto out;
		}
		boot_calc_checksum(sect, size, false, &checksum);
	}

	if (exfat_read(exfat->blk_dev->dev_fd, sect, size, 11 * size) !=
			(ssize_t)size) {
		ret = -EIO;
		goto out;
	}

	for (i = 0; i < size/sizeof(checksum); i++) {
		if (le32_to_cpu(((__le32 *)sect)[i]) != checksum) {
			if (exfat_repair_ask(exfat, ER_BS_CHECKSUM,
				"checksums of boot sector are not correct. "
				"%#x, but expected %#x",
				le32_to_cpu(((__le32 *)sect)[i]), checksum)) {
				goto out_write;
			} else {
				ret = -EINVAL;
				goto out;
			}
		}
	}
out:
	free(sect);
	return ret;

out_write:
	for (i = 0; i < size/sizeof(checksum); i++)
		((__le32 *)sect)[i] = cpu_to_le32(checksum);

	if (exfat_write(exfat->blk_dev->dev_fd,
			sect, size, size * 11) != size) {
		exfat_err("failed to write checksum sector\n");
		free(sect);
		return -EIO;
	}
	free(sect);
	return 0;
}

static int exfat_mark_volume_dirty(struct exfat *exfat, bool dirty)
{
	uint16_t flags;

	if (!(exfat->options & FSCK_OPTS_REPAIR_WRITE))
		return 0;

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

static int read_boot_region(struct exfat_blk_dev *bd, struct pbr **pbr)
{
	struct pbr *bs;

	bs = (struct pbr *)malloc(sizeof(struct pbr));
	if (!bs) {
		exfat_err("failed to allocate memory\n");
		return -ENOMEM;
	}

	if (exfat_read(bd->dev_fd, bs, sizeof(*bs), 0) !=
			(ssize_t)sizeof(*bs)) {
		exfat_err("failed to read a boot sector\n");
		free(bs);
		return -EIO;
	}

	if (memcmp(bs->bpb.oem_name, "EXFAT   ", 8) != 0) {
		exfat_err("failed to find exfat file system.\n");
		goto err;
	}

	if (EXFAT_SECTOR_SIZE(bs) < 512 || EXFAT_SECTOR_SIZE(bs) > 4 * KB) {
		exfat_err("too small or big sector size: %d\n",
				EXFAT_SECTOR_SIZE(bs));
		goto err;
	}

	if (EXFAT_CLUSTER_SIZE(bs) > 32 * MB) {
		exfat_err("too big cluster size: %d\n", EXFAT_CLUSTER_SIZE(bs));
		goto err;
	}

	if (bs->bsx.fs_version[1] != 1 || bs->bsx.fs_version[0] != 0) {
		exfat_err("unsupported exfat version: %d.%d\n",
				bs->bsx.fs_version[1], bs->bsx.fs_version[0]);
		goto err;
	}

	if (bs->bsx.num_fats != 1) {
		exfat_err("unsupported FAT count: %d\n", bs->bsx.num_fats);
		goto err;
	}

	if (le64_to_cpu(bs->bsx.vol_length) * EXFAT_SECTOR_SIZE(bs) >
			bd->size) {
		exfat_err("too large sector count: %" PRIu64 "\n, expected: %llu\n",
				le64_to_cpu(bs->bsx.vol_length),
				bd->num_sectors);
		goto err;
	}

	if (le32_to_cpu(bs->bsx.clu_count) * EXFAT_CLUSTER_SIZE(bs) >
			bd->size) {
		exfat_err("too large cluster count: %u, expected: %u\n",
				le32_to_cpu(bs->bsx.clu_count),
				bd->num_clusters);
		goto err;
	}

	*pbr = bs;
	return 0;
err:
	free(bs);
	return -EINVAL;
}

static void dentry_calc_checksum(struct exfat_dentry *dentry,
				__le16 *checksum, bool primary)
{
	unsigned int i;
	uint8_t *bytes;

	bytes = (uint8_t *)dentry;

	*checksum = ((*checksum << 15) | (*checksum >> 1)) + bytes[0];
	*checksum = ((*checksum << 15) | (*checksum >> 1)) + bytes[1];

	i = primary ? 4 : 2;
	for (; i < sizeof(*dentry); i++) {
		*checksum = ((*checksum << 15) | (*checksum >> 1)) + bytes[i];
	}
}

static __le16 file_calc_checksum(struct exfat_de_iter *iter)
{
	__le16 checksum;
	struct exfat_dentry *file_de, *de;
	int i;

	checksum = 0;
	exfat_de_iter_get(iter, 0, &file_de);

	dentry_calc_checksum(file_de, &checksum, true);
	for (i = 1; i <= file_de->file_num_ext; i++) {
		exfat_de_iter_get(iter, i, &de);
		dentry_calc_checksum(de, &checksum, false);
	}

	return checksum;
}

static bool check_inode(struct exfat_de_iter *iter, struct exfat_inode *node)
{
	struct exfat *exfat = iter->exfat;
	struct exfat_dentry *dentry;
	bool ret = true;
	uint16_t checksum;

	if (check_clus_chain(exfat, node))
		return false;

	if (node->size > le32_to_cpu(exfat->bs->bsx.clu_count) *
				(uint64_t)exfat->clus_size) {
		resolve_path_parent(&path_resolve_ctx, iter->parent, node);
		exfat_err("size %" PRIu64 " is greater than cluster heap: %s\n",
				node->size, path_resolve_ctx.local_path);
		ret = false;
	}

	if (node->size == 0 && node->is_contiguous) {
		resolve_path_parent(&path_resolve_ctx, iter->parent, node);
		exfat_err("empty, but marked as contiguous: %s\n",
					path_resolve_ctx.local_path);
		ret = false;
	}

	if ((node->attr & ATTR_SUBDIR) &&
			node->size % exfat->clus_size != 0) {
		resolve_path_parent(&path_resolve_ctx, iter->parent, node);
		exfat_err("directory size %" PRIu64 " is not divisible by %d: %s\n",
				node->size, exfat->clus_size,
				path_resolve_ctx.local_path);
		ret = false;
	}

	checksum = file_calc_checksum(iter);
	exfat_de_iter_get(iter, 0, &dentry);
	if (checksum != le16_to_cpu(dentry->file_checksum)) {
		if (repair_file_ask(iter, node, ER_DE_CHECKSUM,
				"the checksum of a file is wrong")) {
			exfat_de_iter_get_dirty(iter, 0, &dentry);
			dentry->file_checksum = cpu_to_le16(checksum);
		} else
			ret = false;
	}

	return ret;
}

static int read_file_dentries(struct exfat_de_iter *iter,
			struct exfat_inode **new_node, int *skip_dentries)
{
	struct exfat_dentry *file_de, *stream_de, *name_de;
	struct exfat_inode *node;
	int i, ret;

	/* TODO: mtime, atime, ... */

	ret = exfat_de_iter_get(iter, 0, &file_de);
	if (ret || file_de->type != EXFAT_FILE) {
		exfat_err("failed to get file dentry. %d\n", ret);
		return -EINVAL;
	}
	ret = exfat_de_iter_get(iter, 1, &stream_de);
	if (ret || stream_de->type != EXFAT_STREAM) {
		exfat_err("failed to get stream dentry. %d\n", ret);
		return -EINVAL;
	}

	*new_node = NULL;
	node = alloc_exfat_inode(le16_to_cpu(file_de->file_attr));
	if (!node)
		return -ENOMEM;

	if (file_de->file_num_ext < 2) {
		exfat_err("too few secondary count. %d\n",
				file_de->file_num_ext);
		free_exfat_inode(node);
		return -EINVAL;
	}

	for (i = 2; i <= file_de->file_num_ext; i++) {
		ret = exfat_de_iter_get(iter, i, &name_de);
		if (ret || name_de->type != EXFAT_NAME) {
			exfat_err("failed to get name dentry. %d\n", ret);
			ret = -EINVAL;
			goto err;
		}

		memcpy(node->name +
			(i-2) * ENTRY_NAME_MAX, name_de->name_unicode,
			sizeof(name_de->name_unicode));
	}

	node->first_clus = le32_to_cpu(stream_de->stream_start_clu);
	node->is_contiguous =
		((stream_de->stream_flags & EXFAT_SF_CONTIGUOUS) != 0);
	node->size = le64_to_cpu(stream_de->stream_size);

	if (node->size < le64_to_cpu(stream_de->stream_valid_size)) {
		if (repair_file_ask(iter, node, ER_FILE_VALID_SIZE,
			"valid size %" PRIu64 " greater than size %" PRIu64,
			le64_to_cpu(stream_de->stream_valid_size),
			node->size)) {
			exfat_de_iter_get_dirty(iter, 1, &stream_de);
			stream_de->stream_valid_size =
					stream_de->stream_size;
		} else {
			ret = -EINVAL;
			goto err;
		}
	}

	*skip_dentries = (file_de->file_num_ext + 1);
	*new_node = node;
	return 0;
err:
	*skip_dentries = 0;
	*new_node = NULL;
	free_exfat_inode(node);
	return ret;
}

static int read_file(struct exfat_de_iter *de_iter,
		struct exfat_inode **new_node, int *dentry_count)
{
	struct exfat_inode *node;
	int ret;

	*new_node = NULL;

	ret = read_file_dentries(de_iter, &node, dentry_count);
	if (ret) {
		exfat_err("corrupted file directory entries.\n");
		return ret;
	}

	ret = check_inode(de_iter, node);
	if (!ret) {
		exfat_err("corrupted file directory entries.\n");
		free_exfat_inode(node);
		return -EINVAL;
	}

	node->dentry_file_offset = exfat_de_iter_file_offset(de_iter);
	*new_node = node;
	return 0;
}

static bool read_volume_label(struct exfat_de_iter *iter)
{
	struct exfat *exfat;
	struct exfat_dentry *dentry;
	__le16 disk_label[VOLUME_LABEL_MAX_LEN];

	exfat = iter->exfat;
	if (exfat_de_iter_get(iter, 0, &dentry))
		return false;

	if (dentry->vol_char_cnt == 0)
		return true;

	if (dentry->vol_char_cnt > VOLUME_LABEL_MAX_LEN) {
		exfat_err("too long label. %d\n", dentry->vol_char_cnt);
		return false;
	}

	memcpy(disk_label, dentry->vol_label, sizeof(disk_label));
	if (exfat_utf16_dec(disk_label, dentry->vol_char_cnt*2,
		exfat->volume_label, sizeof(exfat->volume_label)) < 0) {
		exfat_err("failed to decode volume label\n");
		return false;
	}

	exfat_info("volume label [%s]\n", exfat->volume_label);
	return true;
}

static void exfat_bitmap_set_range(struct exfat *exfat,
			clus_t start_clus, clus_t count)
{
	clus_t clus;

	if (!heap_clus(exfat, start_clus) ||
		!heap_clus(exfat, start_clus + count))
		return;

	clus = start_clus;
	while (clus < start_clus + count) {
		EXFAT_BITMAP_SET(exfat->alloc_bitmap,
				clus - EXFAT_FIRST_CLUSTER);
		clus++;
	}
}

static bool read_bitmap(struct exfat_de_iter *iter)
{
	struct exfat_dentry *dentry;
	struct exfat *exfat;

	exfat = iter->exfat;
	if (exfat_de_iter_get(iter, 0, &dentry))
		return false;

	exfat_debug("start cluster %#x, size %#" PRIx64 "\n",
			le32_to_cpu(dentry->bitmap_start_clu),
			le64_to_cpu(dentry->bitmap_size));

	if (le64_to_cpu(dentry->bitmap_size) <
			DIV_ROUND_UP(exfat->clus_count, 8)) {
		exfat_err("invalid size of allocation bitmap. 0x%" PRIx64 "\n",
				le64_to_cpu(dentry->bitmap_size));
		return false;
	}
	if (!heap_clus(exfat, le32_to_cpu(dentry->bitmap_start_clu))) {
		exfat_err("invalid start cluster of allocate bitmap. 0x%x\n",
				le32_to_cpu(dentry->bitmap_start_clu));
		return false;
	}

	exfat->disk_bitmap_clus = le32_to_cpu(dentry->bitmap_start_clu);
	exfat->disk_bitmap_size = DIV_ROUND_UP(exfat->clus_count, 8);

	exfat_bitmap_set_range(exfat, le64_to_cpu(dentry->bitmap_start_clu),
			DIV_ROUND_UP(exfat->disk_bitmap_size,
			exfat->clus_size));

	if (exfat_read(exfat->blk_dev->dev_fd, exfat->disk_bitmap,
			exfat->disk_bitmap_size,
			exfat_c2o(exfat, exfat->disk_bitmap_clus)) !=
			(ssize_t)exfat->disk_bitmap_size)
		return false;

	return true;
}

static bool read_upcase_table(struct exfat_de_iter *iter)
{
	struct exfat_dentry *dentry;
	struct exfat *exfat;
	ssize_t size;
	__le16 *upcase;
	__le32 checksum;

	exfat = iter->exfat;

	if (exfat_de_iter_get(iter, 0, &dentry))
		return false;

	if (!heap_clus(exfat, le32_to_cpu(dentry->upcase_start_clu))) {
		exfat_err("invalid start cluster of upcase table. 0x%x\n",
			le32_to_cpu(dentry->upcase_start_clu));
		return false;
	}

	size = (ssize_t)le64_to_cpu(dentry->upcase_size);
	if (size > (ssize_t)(EXFAT_MAX_UPCASE_CHARS * sizeof(__le16)) ||
			size == 0 || size % sizeof(__le16)) {
		exfat_err("invalid size of upcase table. 0x%" PRIx64 "\n",
			le64_to_cpu(dentry->upcase_size));
		return false;
	}

	upcase = (__le16 *)malloc(size);
	if (!upcase) {
		exfat_err("failed to allocate upcase table\n");
		return false;
	}

	if (exfat_read(exfat->blk_dev->dev_fd, upcase, size,
			exfat_c2o(exfat,
			le32_to_cpu(dentry->upcase_start_clu))) != size) {
		exfat_err("failed to read upcase table\n");
		free(upcase);
		return false;
	}

	checksum = 0;
	boot_calc_checksum((unsigned char *)upcase, size, false, &checksum);
	if (le32_to_cpu(dentry->upcase_checksum) != checksum) {
		exfat_err("corrupted upcase table %#x (expected: %#x)\n",
			checksum, le32_to_cpu(dentry->upcase_checksum));
		free(upcase);
		return false;
	}

	exfat_bitmap_set_range(exfat, le32_to_cpu(dentry->upcase_start_clu),
			DIV_ROUND_UP(le64_to_cpu(dentry->upcase_size),
			exfat->clus_size));

	free(upcase);
	return true;
}

static int read_children(struct exfat *exfat, struct exfat_inode *dir)
{
	int ret;
	struct exfat_inode *node = NULL;
	struct exfat_dentry *dentry;
	int dentry_count;
	struct list_head sub_dir_list;
	struct exfat_de_iter *de_iter;

	INIT_LIST_HEAD(&sub_dir_list);

	de_iter = &exfat->de_iter;
	ret = exfat_de_iter_init(de_iter, exfat, dir);
	if (ret == EOF)
		return 0;
	else if (ret)
		return ret;

	while (1) {
		ret = exfat_de_iter_get(de_iter, 0, &dentry);
		if (ret == EOF) {
			break;
		} else if (ret) {
			exfat_err("failed to get a dentry. %d\n", ret);
			goto err;
		}

		dentry_count = 1;

		switch (dentry->type) {
		case EXFAT_FILE:
			ret = read_file(de_iter, &node, &dentry_count);
			if (ret) {
				exfat_err("failed to verify file. %d\n", ret);
				goto err;
			}

			if ((node->attr & ATTR_SUBDIR) && node->size) {
				node->parent = dir;
				list_add_tail(&node->sibling, &dir->children);
				list_add_tail(&node->list, &sub_dir_list);
			} else
				free_exfat_inode(node);
			break;
		case EXFAT_VOLUME:
			if (!read_volume_label(de_iter)) {
				exfat_err("failed to verify volume label\n");
				ret = -EINVAL;
				goto err;
			}
			break;
		case EXFAT_BITMAP:
			if (!read_bitmap(de_iter)) {
				exfat_err(
					"failed to verify allocation bitmap\n");
				ret = -EINVAL;
				goto err;
			}
			break;
		case EXFAT_UPCASE:
			if (!read_upcase_table(de_iter)) {
				exfat_err(
					"failed to verify upcase table\n");
				ret = -EINVAL;
				goto err;
			}
			break;
		default:
			if (IS_EXFAT_DELETED(dentry->type) ||
					(dentry->type == EXFAT_UNUSED))
				break;
			exfat_err("unknown entry type. 0x%x\n", dentry->type);
			ret = -EINVAL;
			goto err;
		}

		exfat_de_iter_advance(de_iter, dentry_count);
	}
	list_splice(&sub_dir_list, &exfat->dir_list);
	exfat_de_iter_flush(de_iter);
	return 0;
err:
	inode_free_children(dir, false);
	INIT_LIST_HEAD(&dir->children);
	exfat_de_iter_flush(de_iter);
	return ret;
}

static int write_dirty_fat(struct exfat *exfat)
{
	struct buffer_desc *bd;
	off_t offset;
	ssize_t len;
	size_t read_size, write_size;
	clus_t clus, last_clus, clus_count, i;
	unsigned int idx;

	clus = 0;
	last_clus = le32_to_cpu(exfat->bs->bsx.clu_count) + 2;
	bd = exfat->buffer_desc;
	idx = 0;
	offset = le32_to_cpu(exfat->bs->bsx.fat_offset) *
		exfat->sect_size;
	read_size = exfat->clus_size;
	write_size = exfat->sect_size;

	while (clus < last_clus) {
		clus_count = MIN(read_size / sizeof(clus_t), last_clus - clus);
		len = exfat_read(exfat->blk_dev->dev_fd, bd[idx].buffer,
				clus_count * sizeof(clus_t), offset);
		if (len != (ssize_t)(sizeof(clus_t) * clus_count)) {
			exfat_err("failed to read fat entries, %zd\n", len);
			return -EIO;
		}

		/* TODO: read ahead */

		for (i = clus ? clus : EXFAT_FIRST_CLUSTER;
				i < clus + clus_count; i++) {
			if (!EXFAT_BITMAP_GET(exfat->alloc_bitmap,
					i - EXFAT_FIRST_CLUSTER) &&
					((clus_t *)bd[idx].buffer)[i - clus] !=
					EXFAT_FREE_CLUSTER) {
				((clus_t *)bd[idx].buffer)[i - clus] =
					EXFAT_FREE_CLUSTER;
				bd[idx].dirty[(i - clus) /
					(write_size / sizeof(clus_t))] = true;
			}
		}

		for (i = 0; i < read_size; i += write_size) {
			if (bd[idx].dirty[i / write_size]) {
				if (exfat_write(exfat->blk_dev->dev_fd,
						&bd[idx].buffer[i], write_size,
						offset + i) !=
						(ssize_t)write_size) {
					exfat_err("failed to write "
						"fat entries\n");
					return -EIO;

				}
				bd[idx].dirty[i / write_size] = false;
			}
		}

		idx ^= 0x01;
		clus = clus + clus_count;
		offset += len;
	}
	return 0;
}

static int write_dirty_bitmap(struct exfat *exfat)
{
	struct buffer_desc *bd;
	off_t offset, last_offset, bitmap_offset;
	ssize_t len;
	ssize_t read_size, write_size, i, size;
	int idx;

	offset = exfat_c2o(exfat, exfat->disk_bitmap_clus);
	last_offset = offset + exfat->disk_bitmap_size;
	bitmap_offset = 0;
	read_size = exfat->clus_size;
	write_size = exfat->sect_size;

	bd = exfat->buffer_desc;
	idx = 0;

	while (offset < last_offset) {
		len = MIN(read_size, last_offset - offset);
		if (exfat_read(exfat->blk_dev->dev_fd, bd[idx].buffer,
				len, offset) != (ssize_t)len)
			return -EIO;

		/* TODO: read-ahead */

		for (i = 0; i < len; i += write_size) {
			size = MIN(write_size, len - i);
			if (memcmp(&bd[idx].buffer[i],
					exfat->alloc_bitmap + bitmap_offset + i,
					size)) {
				if (exfat_write(exfat->blk_dev->dev_fd,
					exfat->alloc_bitmap + bitmap_offset + i,
					size, offset + i) != size)
					return -EIO;
			}
		}

		idx ^= 0x01;
		offset += len;
		bitmap_offset += len;
	}
	return 0;
}

static int reclaim_free_clusters(struct exfat *exfat)
{
	if (write_dirty_fat(exfat)) {
		exfat_err("failed to write fat entries\n");
		return -EIO;
	}
	if (write_dirty_bitmap(exfat)) {
		exfat_err("failed to write bitmap\n");
		return -EIO;
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
static int exfat_filesystem_check(struct exfat *exfat)
{
	struct exfat_inode *dir;
	int ret = 0, dir_errors;

	if (!exfat->root) {
		exfat_err("root is NULL\n");
		return -ENOENT;
	}

	list_add(&exfat->root->list, &exfat->dir_list);

	while (!list_empty(&exfat->dir_list)) {
		dir = list_entry(exfat->dir_list.next, struct exfat_inode, list);

		if (!(dir->attr & ATTR_SUBDIR)) {
			resolve_path(&path_resolve_ctx, dir);
			exfat_err("failed to travel directories. "
					"the node is not directory: %s\n",
					path_resolve_ctx.local_path);
			ret = -EINVAL;
			goto out;
		}

		dir_errors = read_children(exfat, dir);
		if (dir_errors) {
			resolve_path(&path_resolve_ctx, dir);
			exfat_err("failed to check dentries: %s\n",
					path_resolve_ctx.local_path);
			ret = dir_errors;
		}

		list_del(&dir->list);
		inode_free_file_children(dir);
		inode_free_ancestors(dir);
	}
out:
	exfat_free_dir_list(exfat);
	exfat->root = NULL;
	if (exfat->dirty_fat && reclaim_free_clusters(exfat))
		return -EIO;
	return ret;
}

static int exfat_root_dir_check(struct exfat *exfat)
{
	struct exfat_inode *root;
	clus_t clus_count;

	root = alloc_exfat_inode(ATTR_SUBDIR);
	if (!root) {
		exfat_err("failed to allocate memory\n");
		return -ENOMEM;
	}

	root->first_clus = le32_to_cpu(exfat->bs->bsx.root_cluster);
	if (!root_get_clus_count(exfat, root, &clus_count)) {
		exfat_err("failed to follow the cluster chain of root\n");
		free_exfat_inode(root);
		return -EINVAL;
	}
	root->size = clus_count * exfat->clus_size;

	exfat->root = root;
	exfat_debug("root directory: start cluster[0x%x] size[0x%" PRIx64 "]\n",
		root->first_clus, root->size);
	return 0;
}

static void exfat_show_info(struct exfat *exfat)
{
	exfat_info("Bytes per sector: %d\n",
			1 << exfat->bs->bsx.sect_size_bits);
	exfat_info("Sectors per cluster: %d\n",
			1 << exfat->bs->bsx.sect_per_clus_bits);
	exfat_info("Cluster heap count: %d(0x%x)\n",
			le32_to_cpu(exfat->bs->bsx.clu_count),
			le32_to_cpu(exfat->bs->bsx.clu_count));
	exfat_info("Cluster heap offset: %#x\n",
			le32_to_cpu(exfat->bs->bsx.clu_offset));
}

static void exfat_show_stat(void)
{
	exfat_debug("Found directories: %ld\n", exfat_stat.dir_count);
	exfat_debug("Found files: %ld\n", exfat_stat.file_count);
	exfat_debug("Found leak directories: %ld\n",
			exfat_stat.dir_count - exfat_stat.dir_free_count);
	exfat_debug("Found leak files: %ld\n",
			exfat_stat.file_count - exfat_stat.file_free_count);
}

int main(int argc, char * const argv[])
{
	struct fsck_user_input ui;
	struct exfat_blk_dev bd;
	struct exfat *exfat = NULL;
	struct pbr *bs = NULL;
	int c, ret, exit_code;
	bool version_only = false;

	memset(&ui, 0, sizeof(ui));
	memset(&bd, 0, sizeof(bd));

	print_level = EXFAT_ERROR;

	if (!setlocale(LC_CTYPE, ""))
		exfat_err("failed to init locale/codeset\n");

	opterr = 0;
	while ((c = getopt_long(argc, argv, "rynpVvh", opts, NULL)) != EOF) {
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
		case 'p':
			if (ui.options & FSCK_OPTS_REPAIR_ALL)
				usage(argv[0]);
			ui.options |= FSCK_OPTS_REPAIR_AUTO;
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
		ui.options |= FSCK_OPTS_REPAIR_NO;
		ui.ei.writeable = false;
	}

	snprintf(ui.ei.dev_name, sizeof(ui.ei.dev_name), "%s", argv[optind]);
	ret = exfat_get_blk_dev_info(&ui.ei, &bd);
	if (ret < 0) {
		exfat_err("failed to open %s. %d\n", ui.ei.dev_name, ret);
		return FSCK_EXIT_OPERATION_ERROR;
	}

	ret = read_boot_region(&bd, &bs);
	if (ret)
		goto err;

	exfat = alloc_exfat(&bd, bs);
	if (!exfat) {
		ret = -ENOMEM;
		goto err;
	}
	exfat->options = ui.options;

	if (exfat_mark_volume_dirty(exfat, true)) {
		ret = -EIO;
		goto err;
	}

	ret = boot_region_checksum(exfat);
	if (ret)
		goto err;

	exfat_show_info(exfat);

	exfat_debug("verifying root directory...\n");
	ret = exfat_root_dir_check(exfat);
	if (ret) {
		exfat_err("failed to verify root directory.\n");
		goto out;
	}

	exfat_debug("verifying directory entries...\n");
	ret = exfat_filesystem_check(exfat);
	if (ret) {
		exfat_err("failed to verify directory entries.\n");
		goto out;
	}

	if (ui.ei.writeable && fsync(bd.dev_fd)) {
		ret = -EIO;
		goto out;
	}
	exfat_mark_volume_dirty(exfat, false);

	printf("%s: clean\n", ui.ei.dev_name);
out:
	exfat_show_stat();
err:
	if (ret == -EINVAL)
		exit_code = FSCK_EXIT_ERRORS_LEFT;
	else if (ret)
		exit_code = FSCK_EXIT_OPERATION_ERROR;
	else if (exfat->dirty)
		exit_code = FSCK_EXIT_CORRECTED;
	else
		exit_code = FSCK_EXIT_NO_ERRORS;
	free_exfat(exfat);
	close(bd.dev_fd);
	return exit_code;
}
