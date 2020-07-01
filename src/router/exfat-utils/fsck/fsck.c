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
			((__bmap)[BIT_ENTRY(__c)] & BIT_MASK(__c))
#define EXFAT_BITMAP_SET(__bmap, __c)	\
			((__bmap)[BIT_ENTRY(__c)] |= BIT_MASK(__c))

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

static struct exfat *alloc_exfat(struct exfat_blk_dev *bd)
{
	struct exfat *exfat;

	exfat = (struct exfat *)calloc(1, sizeof(*exfat));
	if (!exfat) {
		exfat_err("failed to allocate exfat\n");
		return NULL;
	}

	exfat->blk_dev = bd;
	INIT_LIST_HEAD(&exfat->dir_list);
	return exfat;
}

static void free_exfat(struct exfat *exfat)
{
	if (exfat) {
		if (exfat->bs)
			free(exfat->bs);
		if (exfat->de_iter.dentries)
			free(exfat->de_iter.dentries);
		if (exfat->alloc_bitmap)
			free(exfat->alloc_bitmap);
		free(exfat);
	}
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

static inline bool exfat_invalid_clus(struct exfat *exfat, clus_t clus)
{
	return clus < EXFAT_FIRST_CLUSTER ||
	(clus - EXFAT_FIRST_CLUSTER) > le32_to_cpu(exfat->bs->bsx.clu_count);
}

static int inode_get_clus_next(struct exfat *exfat, struct exfat_inode *node,
				clus_t clus, clus_t *next)
{
	off_t offset;

	if (exfat_invalid_clus(exfat, clus))
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

static bool inode_check_clus_chain(struct exfat *exfat, struct exfat_inode *node)
{
	clus_t clus;
	clus_t clus_count;

	clus = node->first_clus;
	clus_count = DIV_ROUND_UP(node->size, EXFAT_CLUSTER_SIZE(exfat->bs));

	while (clus_count--) {
		if (exfat_invalid_clus(exfat, clus)) {
			exfat_err("bad cluster. 0x%x\n", clus);
			return false;
		}

		if (!EXFAT_BITMAP_GET(exfat->alloc_bitmap,
					clus - EXFAT_FIRST_CLUSTER)) {
			exfat_err(
				"cluster allocated, but not in bitmap. 0x%x\n",
				clus);
			return false;
		}

		if (inode_get_clus_next(exfat, node, clus, &clus) != 0) {
			exfat_err(
				"broken cluster chain. (previous cluster 0x%x)\n",
				clus);
			return false;
		}
	}
	return true;
}

static bool inode_get_clus_count(struct exfat *exfat, struct exfat_inode *node,
							clus_t *clus_count)
{
	clus_t clus;

	clus = node->first_clus;
	*clus_count = 0;

	do {
		if (exfat_invalid_clus(exfat, clus)) {
			exfat_err("bad cluster. 0x%x\n", clus);
			return false;
		}

		if (inode_get_clus_next(exfat, node, clus, &clus) != 0) {
			exfat_err(
				"broken cluster chain. (previous cluster 0x%x)\n",
				clus);
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

static off_t exfat_c2o(struct exfat *exfat, unsigned int clus)
{
	if (clus < EXFAT_FIRST_CLUSTER)
		return ~0L;

	return exfat_s2o(exfat, le32_to_cpu(exfat->bs->bsx.clu_offset) +
				((clus - EXFAT_FIRST_CLUSTER) <<
				 exfat->bs->bsx.sect_per_clus_bits));
}

static ssize_t exfat_file_read(struct exfat *exfat, struct exfat_inode *node,
			void *buf, size_t total_size, off_t file_offset)
{
	size_t clus_size;
	clus_t start_l_clus, l_clus, p_clus;
	unsigned int clus_offset;
	int ret;
	off_t device_offset;
	ssize_t read_size;
	size_t remain_size;

	if (file_offset >= (off_t)node->size)
		return EOF;

	clus_size = EXFAT_CLUSTER_SIZE(exfat->bs);
	total_size = MIN(total_size, node->size - file_offset);
	remain_size = total_size;

	if (remain_size == 0)
		return 0;

	start_l_clus = file_offset / clus_size;
	clus_offset = file_offset % clus_size;
	if (start_l_clus >= node->last_lclus &&
			node->last_pclus != EXFAT_EOF_CLUSTER) {
		l_clus = node->last_lclus;
		p_clus = node->last_pclus;
	} else {
		l_clus = 0;
		p_clus = node->first_clus;
	}

	while (p_clus != EXFAT_EOF_CLUSTER) {
		if (exfat_invalid_clus(exfat, p_clus))
			return -EINVAL;
		if (l_clus < start_l_clus)
			goto next_clus;

		read_size = MIN(remain_size, clus_size - clus_offset);
		device_offset = exfat_c2o(exfat, p_clus) + clus_offset;
		if (exfat_read(exfat->blk_dev->dev_fd, buf, read_size,
					device_offset) != read_size)
			return -EIO;

		clus_offset = 0;
		buf = (char *)buf + read_size;
		remain_size -= read_size;
		if (remain_size == 0)
			goto out;

next_clus:
		l_clus++;
		ret = inode_get_clus_next(exfat, node, p_clus, &p_clus);
		if (ret)
			return ret;
	}
out:
	node->last_lclus = l_clus;
	node->last_pclus = p_clus;
	return total_size - remain_size;
}

static int boot_region_checksum(struct exfat *exfat)
{
	__le32 checksum;
	unsigned short size;
	void *sect;
	unsigned int i;

	size = EXFAT_SECTOR_SIZE(exfat->bs);
	sect = malloc(size);
	if (!sect)
		return -ENOMEM;

	checksum = 0;

	boot_calc_checksum((unsigned char *)exfat->bs, size, true, &checksum);
	for (i = 1; i < 11; i++) {
		if (exfat_read(exfat->blk_dev->dev_fd, sect, size, i * size) !=
				(ssize_t)size) {
			free(sect);
			return -EIO;
		}
		boot_calc_checksum(sect, size, false, &checksum);
	}

	if (exfat_read(exfat->blk_dev->dev_fd, sect, size, i * size) !=
			(ssize_t)size) {
		free(sect);
		return -EIO;
	}
	for (i = 0; i < size/sizeof(checksum); i++) {
		if (le32_to_cpu(((__le32 *)sect)[i]) != checksum) {
			union exfat_repair_context rctx = {
				.bs_checksum.checksum		= checksum,
				.bs_checksum.checksum_sect	= sect,
			};
			if (!exfat_repair(exfat, ER_BS_CHECKSUM, &rctx)) {
				exfat_err("invalid checksum. 0x%x\n",
					le32_to_cpu(((__le32 *)sect)[i]));
				free(sect);
				return -EIO;
			}
		}
	}

	free(sect);
	return 0;
}

static bool exfat_boot_region_check(struct exfat *exfat)
{
	struct pbr *bs;
	ssize_t ret;

	bs = (struct pbr *)malloc(sizeof(struct pbr));
	if (!bs) {
		exfat_err("failed to allocate memory\n");
		return false;
	}

	exfat->bs = bs;

	ret = exfat_read(exfat->blk_dev->dev_fd, bs, sizeof(*bs), 0);
	if (ret != sizeof(*bs)) {
		exfat_err("failed to read a boot sector. %zd\n", ret);
		goto err;
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

	ret = boot_region_checksum(exfat);
	if (ret) {
		exfat_err("failed to verify the checksum of a boot region. %zd\n",
			ret);
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
			exfat->blk_dev->size) {
		exfat_err("too large sector count: %" PRIu64 "\n, expected: %llu\n",
				le64_to_cpu(bs->bsx.vol_length),
				exfat->blk_dev->num_sectors);
		goto err;
	}

	if (le32_to_cpu(bs->bsx.clu_count) * EXFAT_CLUSTER_SIZE(bs) >
			exfat->blk_dev->size) {
		exfat_err("too large cluster count: %u, expected: %u\n",
				le32_to_cpu(bs->bsx.clu_count),
				exfat->blk_dev->num_clusters);
		goto err;
	}

	return true;
err:
	free(bs);
	exfat->bs = NULL;
	return false;
}

static size_t utf16_len(const __le16 *str, size_t max_size)
{
	size_t i = 0;

	while (le16_to_cpu(str[i]) && i < max_size)
		i++;
	return i;
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
		name_len = utf16_len(dir->name, NAME_BUFFER_SIZE);
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
	const __le16 utf16_slash = cpu_to_le16(0x002F);
	size_t in_size;

	ctx->local_path[0] = '\0';

	get_ancestors(child,
			ctx->ancestors,
			sizeof(ctx->ancestors) / sizeof(ctx->ancestors[0]),
			PATH_MAX,
			&depth);

	utf16_path = ctx->utf16_path;
	for (i = 0; i < depth; i++) {
		name_len = utf16_len(ctx->ancestors[i]->name, NAME_BUFFER_SIZE);
		memcpy((char *)utf16_path, (char *)ctx->ancestors[i]->name,
				name_len * 2);
		utf16_path += name_len;
		memcpy((char *)utf16_path, &utf16_slash, sizeof(utf16_slash));
		utf16_path++;
	}

	if (depth > 0)
		utf16_path--;
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

static int exfat_de_iter_init(struct exfat_de_iter *iter, struct exfat *exfat,
						struct exfat_inode *dir)
{
	ssize_t ret;

	if (!iter->dentries) {
		iter->read_size = EXFAT_CLUSTER_SIZE(exfat->bs);
		iter->dentries = malloc(iter->read_size * 2);
		if (!iter->dentries) {
			exfat_err("failed to allocate memory\n");
			return -ENOMEM;
		}
	}

	ret = exfat_file_read(exfat, dir, iter->dentries, iter->read_size, 0);
	if (ret != iter->read_size) {
		exfat_err("failed to read directory entries.\n");
		return -EIO;
	}

	iter->exfat = exfat;
	iter->parent = dir;
	iter->de_file_offset = 0;
	iter->next_read_offset = iter->read_size;
	iter->max_skip_dentries = 0;
	return 0;
}

static int exfat_de_iter_get(struct exfat_de_iter *iter,
				int ith, struct exfat_dentry **dentry)
{
	off_t de_next_file_offset;
	unsigned int de_next_offset;
	bool need_read_1_clus = false;
	int ret;

	de_next_file_offset = iter->de_file_offset +
			ith * sizeof(struct exfat_dentry);

	if (de_next_file_offset + sizeof(struct exfat_dentry) >
		round_down(iter->parent->size, sizeof(struct exfat_dentry)))
		return EOF;

	/*
	 * dentry must be in current cluster, or next cluster which
	 * will be read
	 */
	if (de_next_file_offset -
		(iter->de_file_offset / iter->read_size) * iter->read_size >=
		iter->read_size * 2)
		return -ERANGE;

	de_next_offset = de_next_file_offset % (iter->read_size * 2);

	/* read a cluster if needed */
	if (de_next_file_offset >= iter->next_read_offset) {
		void *buf;

		need_read_1_clus = de_next_offset < iter->read_size;
		buf = need_read_1_clus ?
			iter->dentries : iter->dentries + iter->read_size;

		ret = exfat_file_read(iter->exfat, iter->parent, buf,
			iter->read_size, iter->next_read_offset);
		if (ret == EOF) {
			return EOF;
		} else if (ret <= 0) {
			exfat_err("failed to read a cluster. %d\n", ret);
			return ret;
		}
		iter->next_read_offset += iter->read_size;
	}

	if (ith + 1 > iter->max_skip_dentries)
		iter->max_skip_dentries = ith + 1;

	*dentry = (struct exfat_dentry *) (iter->dentries + de_next_offset);
	return 0;
}

/*
 * @skip_dentries must be the largest @ith + 1 of exfat_de_iter_get
 * since the last call of exfat_de_iter_advance
 */
static int exfat_de_iter_advance(struct exfat_de_iter *iter, int skip_dentries)
{
	if (skip_dentries != iter->max_skip_dentries)
		return -EINVAL;

	iter->max_skip_dentries = 0;
	iter->de_file_offset = iter->de_file_offset +
				skip_dentries * sizeof(struct exfat_dentry);
	return 0;
}

static off_t exfat_de_iter_file_offset(struct exfat_de_iter *iter)
{
	return iter->de_file_offset;
}

static bool check_inode(struct exfat *exfat, struct exfat_inode *parent,
						struct exfat_inode *node)
{
	bool ret = false;

	if (node->size == 0 && node->first_clus != EXFAT_FREE_CLUSTER) {
		resolve_path_parent(&path_resolve_ctx, parent, node);
		exfat_err("file is empty, but first cluster is %#x: %s\n",
				node->first_clus, path_resolve_ctx.local_path);
		ret = false;
	}

	if (node->size > 0 && exfat_invalid_clus(exfat, node->first_clus)) {
		resolve_path_parent(&path_resolve_ctx, parent, node);
		exfat_err("first cluster %#x is invalid: %s\n",
				node->first_clus, path_resolve_ctx.local_path);
		ret = false;
	}

	if (node->size > le32_to_cpu(exfat->bs->bsx.clu_count) *
				EXFAT_CLUSTER_SIZE(exfat->bs)) {
		resolve_path_parent(&path_resolve_ctx, parent, node);
		exfat_err("size %" PRIu64 " is greater than cluster heap: %s\n",
				node->size, path_resolve_ctx.local_path);
		ret = false;
	}

	if (node->size == 0 && node->is_contiguous) {
		resolve_path_parent(&path_resolve_ctx, parent, node);
		exfat_err("empty, but marked as contiguous: %s\n",
					path_resolve_ctx.local_path);
		ret = false;
	}

	if ((node->attr & ATTR_SUBDIR) &&
			node->size % EXFAT_CLUSTER_SIZE(exfat->bs) != 0) {
		resolve_path_parent(&path_resolve_ctx, parent, node);
		exfat_err("directory size %" PRIu64 " is not divisible by %d: %s\n",
				node->size, EXFAT_CLUSTER_SIZE(exfat->bs),
				path_resolve_ctx.local_path);
		ret = false;
	}

	if (!node->is_contiguous && !inode_check_clus_chain(exfat, node)) {
		resolve_path_parent(&path_resolve_ctx, parent, node);
		exfat_err("corrupted cluster chain: %s\n",
				path_resolve_ctx.local_path);
		ret = false;
	}

	return ret;
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

static int read_file_dentries(struct exfat_de_iter *iter,
			struct exfat_inode **new_node, int *skip_dentries)
{
	struct exfat_dentry *file_de, *stream_de, *name_de;
	struct exfat_inode *node;
	int i, ret;
	__le16 checksum;

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

	checksum = file_calc_checksum(iter);
	if (le16_to_cpu(file_de->file_checksum) != checksum) {
		exfat_err("invalid checksum. 0x%x != 0x%x\n",
			le16_to_cpu(file_de->file_checksum),
			le16_to_cpu(checksum));
		ret = -EINVAL;
		goto err;
	}

	node->size = le64_to_cpu(stream_de->stream_size);
	node->first_clus = le32_to_cpu(stream_de->stream_start_clu);
	node->is_contiguous =
		((stream_de->stream_flags & EXFAT_SF_CONTIGUOUS) != 0);

	if (le64_to_cpu(stream_de->stream_valid_size) > node->size) {
		resolve_path_parent(&path_resolve_ctx, iter->parent, node);
		exfat_err("valid size %" PRIu64 " greater than size %" PRIu64 ": %s\n",
			le64_to_cpu(stream_de->stream_valid_size), node->size,
			path_resolve_ctx.local_path);
		ret = -EINVAL;
		goto err;
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

	ret = check_inode(de_iter->exfat, de_iter->parent, node);
	if (ret) {
		exfat_err("corrupted file directory entries.\n");
		free_exfat_inode(node);
		return ret;
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

static bool read_alloc_bitmap(struct exfat_de_iter *iter)
{
	struct exfat_dentry *dentry;
	struct exfat *exfat;
	ssize_t alloc_bitmap_size;

	exfat = iter->exfat;
	if (exfat_de_iter_get(iter, 0, &dentry))
		return false;

	exfat_debug("start cluster %#x, size %#" PRIx64 "\n",
			le32_to_cpu(dentry->bitmap_start_clu),
			le64_to_cpu(dentry->bitmap_size));

	exfat->bit_count = le32_to_cpu(exfat->bs->bsx.clu_count);

	if (le64_to_cpu(dentry->bitmap_size) <
			DIV_ROUND_UP(exfat->bit_count, 8)) {
		exfat_err("invalid size of allocation bitmap. 0x%" PRIx64 "\n",
				le64_to_cpu(dentry->bitmap_size));
		return false;
	}
	if (exfat_invalid_clus(exfat, le32_to_cpu(dentry->bitmap_start_clu))) {
		exfat_err("invalid start cluster of allocate bitmap. 0x%x\n",
				le32_to_cpu(dentry->bitmap_start_clu));
		return false;
	}

	/* TODO: bitmap could be very large. */
	alloc_bitmap_size = EXFAT_BITMAP_SIZE(exfat->bit_count);
	exfat->alloc_bitmap = (__u32 *)malloc(alloc_bitmap_size);
	if (!exfat->alloc_bitmap) {
		exfat_err("failed to allocate bitmap\n");
		return false;
	}

	if (exfat_read(exfat->blk_dev->dev_fd,
			exfat->alloc_bitmap, alloc_bitmap_size,
			exfat_c2o(exfat,
			le32_to_cpu(dentry->bitmap_start_clu))) !=
			alloc_bitmap_size) {
		exfat_err("failed to read bitmap\n");
		free(exfat->alloc_bitmap);
		exfat->alloc_bitmap = NULL;
		return false;
	}

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

	if (exfat_invalid_clus(exfat, le32_to_cpu(dentry->upcase_start_clu))) {
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
			if (!read_alloc_bitmap(de_iter)) {
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
	return 0;
err:
	inode_free_children(dir, false);
	INIT_LIST_HEAD(&dir->children);
	return ret;
}

/*
 * for each directory in @dir_list.
 * 1. read all dentries and allocate exfat_nodes for files and directories.
 *    and append directory exfat_nodes to the head of @dir_list
 * 2. free all of file exfat_nodes.
 * 3. if the directory does not have children, free its exfat_node.
 */
static bool exfat_filesystem_check(struct exfat *exfat)
{
	struct exfat_inode *dir;
	bool ret = true;

	if (!exfat->root) {
		exfat_err("root is NULL\n");
		return false;
	}

	list_add(&exfat->root->list, &exfat->dir_list);

	while (!list_empty(&exfat->dir_list)) {
		dir = list_entry(exfat->dir_list.next, struct exfat_inode, list);

		if (!(dir->attr & ATTR_SUBDIR)) {
			resolve_path(&path_resolve_ctx, dir);
			ret = false;
			exfat_err("failed to travel directories. "
					"the node is not directory: %s\n",
					path_resolve_ctx.local_path);
			goto out;
		}

		if (read_children(exfat, dir)) {
			resolve_path(&path_resolve_ctx, dir);
			ret = false;
			exfat_err("failed to check dentries: %s\n",
					path_resolve_ctx.local_path);
			goto out;
		}

		list_del(&dir->list);
		inode_free_file_children(dir);
		inode_free_ancestors(dir);
	}
out:
	exfat_free_dir_list(exfat);
	exfat->root = NULL;
	return ret;
}

static bool exfat_root_dir_check(struct exfat *exfat)
{
	struct exfat_inode *root;
	clus_t clus_count;

	root = alloc_exfat_inode(ATTR_SUBDIR);
	if (!root) {
		exfat_err("failed to allocate memory\n");
		return false;
	}

	root->first_clus = le32_to_cpu(exfat->bs->bsx.root_cluster);
	if (!inode_get_clus_count(exfat, root, &clus_count)) {
		exfat_err("failed to follow the cluster chain of root\n");
		goto err;
	}
	root->size = clus_count * EXFAT_CLUSTER_SIZE(exfat->bs);

	exfat->root = root;
	exfat_debug("root directory: start cluster[0x%x] size[0x%" PRIx64 "]\n",
		root->first_clus, root->size);
	return true;
err:
	free_exfat_inode(root);
	exfat->root = NULL;
	return false;
}

void exfat_show_info(struct exfat *exfat)
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

void exfat_show_stat(void)
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
	int c, ret;
	struct fsck_user_input ui;
	struct exfat_blk_dev bd;
	struct exfat *exfat = NULL;
	bool version_only = false;

	memset(&ui, 0, sizeof(ui));
	memset(&bd, 0, sizeof(bd));

	print_level = EXFAT_ERROR;

	if (!setlocale(LC_CTYPE, ""))
		exfat_err("failed to init locale/codeset\n");

	opterr = 0;
	while ((c = getopt_long(argc, argv, "rynVvh", opts, NULL)) != EOF) {
		switch (c) {
		case 'n':
			if (ui.options & FSCK_OPTS_REPAIR)
				usage(argv[0]);
			ui.options |= FSCK_OPTS_REPAIR_NO;
			ui.ei.writeable = false;
			break;
		case 'r':
			if (ui.options & FSCK_OPTS_REPAIR)
				usage(argv[0]);
			ui.options |= FSCK_OPTS_REPAIR_ASK;
			ui.ei.writeable = true;
			break;
		case 'y':
			if (ui.options & FSCK_OPTS_REPAIR)
				usage(argv[0]);
			ui.options |= FSCK_OPTS_REPAIR_YES;
			ui.ei.writeable = true;
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
	if (version_only)
		exit(FSCK_EXIT_SYNTAX_ERROR);

	if (optind != argc - 1)
		usage(argv[0]);

	snprintf(ui.ei.dev_name, sizeof(ui.ei.dev_name), "%s", argv[optind]);
	ret = exfat_get_blk_dev_info(&ui.ei, &bd);
	if (ret < 0) {
		exfat_err("failed to open %s. %d\n", ui.ei.dev_name, ret);
		return FSCK_EXIT_OPERATION_ERROR;
	}

	exfat = alloc_exfat(&bd);
	if (!exfat) {
		ret = FSCK_EXIT_OPERATION_ERROR;
		goto err;
	}
	exfat->options = ui.options;

	exfat_debug("verifying boot regions...\n");
	if (!exfat_boot_region_check(exfat)) {
		exfat_err("failed to verify boot regions.\n");
		ret = FSCK_EXIT_ERRORS_LEFT;
		goto err;
	}

	exfat_show_info(exfat);

	exfat_debug("verifying root directory...\n");
	if (!exfat_root_dir_check(exfat)) {
		exfat_err("failed to verify root directory.\n");
		ret = FSCK_EXIT_ERRORS_LEFT;
		goto out;
	}

	exfat_debug("verifying directory entries...\n");
	if (!exfat_filesystem_check(exfat)) {
		exfat_err("failed to verify directory entries.\n");
		ret = FSCK_EXIT_ERRORS_LEFT;
		goto out;
	}

	if (ui.ei.writeable)
		fsync(bd.dev_fd);
	printf("%s: clean\n", ui.ei.dev_name);
	ret = FSCK_EXIT_NO_ERRORS;
out:
	exfat_show_stat();
err:
	free_exfat(exfat);
	close(bd.dev_fd);
	return ret;
}
