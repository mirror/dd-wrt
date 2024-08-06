// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2021 LG Electronics.
 *
 *   Author(s): Hyunchul Lee <hyc.lee@gmail.com>
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

#include "exfat_ondisk.h"
#include "libexfat.h"
#include "exfat_fs.h"
#include "exfat_dir.h"

#define EXFAT_MAX_UPCASE_CHARS	0x10000

struct exfat2img_hdr {
	__le32	magic;
	__le32	major_version;
	__le32	minor_version;
	__le32	data_offset;
	__le32	heap_clus_offset;
	__le32	cluster_size;
	__le32	cluster_count;
	__le32	reserved[20];
} __packed;

#define EI_MAGIC		0xB67598DB
#define EI_CC_PAYLOAD_LEN	4

enum {
	EI_CC_INVALID,
	EI_CC_COPY_1,
	EI_CC_COPY_2,	/* followed by cluster count(4-byte) */
	EI_CC_SKIP_1,
	EI_CC_SKIP_2,	/* followed by cluster count(4-byte) */
};

struct exfat2img {
	int			out_fd;
	bool			is_stdout;
	off_t			stdout_offset;
	bool			save_cc;
	struct exfat_blk_dev	bdev;
	struct exfat		*exfat;
	void			*dump_cluster;
	struct buffer_desc	*scan_bdesc;
	struct exfat_de_iter	de_iter;
};

struct exfat_stat {
	long		dir_count;
	long		file_count;
	long		error_count;
	uint64_t	written_bytes;
};

static struct exfat2img_hdr ei_hdr;
static struct exfat2img ei;
static struct exfat_stat exfat_stat;
static struct path_resolve_ctx path_resolve_ctx;

static struct option opts[] = {
	{"output",	required_argument,	NULL,	'o' },
	{"version",	no_argument,		NULL,	'V' },
	{"help",	no_argument,		NULL,	'h' },
	{NULL,		0,			NULL,	 0  }
};

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s <device> [image-file]\n", name);
	fprintf(stderr, "\t-o | --output <image-file> Specify destination file\n");
	fprintf(stderr, "\t-V | --version             Show version\n");
	fprintf(stderr, "\t-h | --help                Show help\n");
	exit(EXIT_FAILURE);
}

#define ei_err(parent, inode, fmt, ...)			\
({							\
		exfat_resolve_path_parent(&path_resolve_ctx,	\
			parent, inode);			\
		exfat_err("ERROR: %s: " fmt,		\
			path_resolve_ctx.local_path,	\
			##__VA_ARGS__);			\
})

static void free_exfat2img(struct exfat2img *ei)
{
	if (ei->scan_bdesc)
		exfat_free_buffer(ei->exfat, ei->scan_bdesc);
	if (ei->exfat)
		exfat_free_exfat(ei->exfat);
	if (ei->dump_cluster)
		free(ei->dump_cluster);
	if (ei->out_fd)
		close(ei->out_fd);
	if (ei->bdev.dev_fd)
		close(ei->bdev.dev_fd);
}

static int create_exfat2img(struct exfat2img *ei,
			    struct pbr *bs,
			    const char *out_path)
{
	int err;

	ei->exfat = exfat_alloc_exfat(&ei->bdev, bs);
	if (!ei->exfat)
		return -ENOMEM;

	ei->dump_cluster = malloc(ei->exfat->clus_size);
	if (!ei->dump_cluster) {
		err = -ENOMEM;
		goto err;
	}

	ei->scan_bdesc = exfat_alloc_buffer(ei->exfat);
	if (!ei->scan_bdesc) {
		err = -ENOMEM;
		goto err;
	}

	if (strcmp(out_path, "-")) {
		ei->out_fd = open(out_path, O_CREAT | O_TRUNC | O_RDWR, 0664);
	} else {
		ei->is_stdout = true;
		ei->out_fd = fileno(stdout);
		ei->save_cc = true;
	}
	if (ei->out_fd < 0) {
		exfat_err("failed to open %s: %s\n", out_path,
			  strerror(errno));
		err = -errno;
		goto err;
	}

	return 0;
err:
	free_exfat2img(ei);
	return err;
}

/**
 * @end: excluded.
 */
static ssize_t dump_range(struct exfat2img *ei, off_t start, off_t end)
{
	struct exfat *exfat = ei->exfat;
	size_t len, total_len = 0;
	ssize_t ret;

	if (ei->is_stdout) {
		unsigned int sc, sc_offset;
		unsigned int ec, ec_offset;

		if (exfat_o2c(ei->exfat, start, &sc, &sc_offset) < 0)
			return -ERANGE;
		if (exfat_o2c(ei->exfat, end - 1, &ec, &ec_offset) < 0)
			return -ERANGE;
		exfat_bitmap_set_range(ei->exfat, exfat->alloc_bitmap,
				       sc, ec - sc + 1);
		return end - start;
	}

	while (start < end) {
		len = (size_t)MIN(end - start, exfat->clus_size);

		ret = exfat_read(exfat->blk_dev->dev_fd,
				 ei->dump_cluster, len, start);
		if (ret != (ssize_t)len) {
			exfat_err("failed to read %llu bytes at %llu\n",
				  (unsigned long long)len,
				  (unsigned long long)start);
			return -EIO;
		}

		ret = pwrite(ei->out_fd, ei->dump_cluster, len, start);
		if (ret != (ssize_t)len) {
			exfat_err("failed to write %llu bytes at %llu\n",
				  (unsigned long long)len,
				  (unsigned long long)start);
			return -EIO;
		}

		start += len;
		total_len += len;
		exfat_stat.written_bytes += len;
	}
	return total_len;
}

static int dump_sectors(struct exfat2img *ei,
			off_t start_sect,
			off_t end_sect_excl)
{
	struct exfat *exfat = ei->exfat;
	off_t s, e;

	s = exfat_s2o(exfat, start_sect);
	e = exfat_s2o(exfat, end_sect_excl);
	return dump_range(ei, s, e) <= 0 ? -EIO : 0;
}

static int dump_clusters(struct exfat2img *ei,
			 clus_t start_clus,
			 clus_t end_clus_excl)
{
	struct exfat *exfat = ei->exfat;
	off_t s, e;

	s = exfat_c2o(exfat, start_clus);
	e = exfat_c2o(exfat, end_clus_excl);
	return dump_range(ei, s, e) <= 0 ? -EIO : 0;
}

static int dump_directory(struct exfat2img *ei,
			  struct exfat_inode *inode, size_t size,
			  clus_t *out_clus_count)
{
	struct exfat *exfat = ei->exfat;
	clus_t clus, possible_count;
	uint64_t max_count;
	size_t dump_size;
	off_t start_off, end_off;

	if (size == 0)
		return -EINVAL;

	if (!(inode->attr & ATTR_SUBDIR))
		return -EINVAL;

	clus = inode->first_clus;
	*out_clus_count = 0;
	max_count = DIV_ROUND_UP(inode->size, exfat->clus_size);

	possible_count = (256 * MB) >> (exfat->bs->bsx.sect_per_clus_bits +
					exfat->bs->bsx.sect_size_bits);
	possible_count = MIN(possible_count, exfat->clus_count);

	while (exfat_heap_clus(exfat, clus) && *out_clus_count < possible_count) {
		dump_size = MIN(size, exfat->clus_size);
		start_off = exfat_c2o(exfat, clus);
		end_off = start_off + DIV_ROUND_UP(dump_size, 512) * 512;

		if (dump_range(ei, start_off, end_off) < 0)
			return -EIO;

		*out_clus_count += 1;
		size -= dump_size;
		if (size == 0)
			break;

		if (inode->is_contiguous) {
			if (*out_clus_count >= max_count)
				break;
		}
		if (exfat_get_inode_next_clus(exfat, inode, clus, &clus))
			return -EINVAL;
	}
	return 0;
}

static int dump_root(struct exfat2img *ei)
{
	struct exfat *exfat = ei->exfat;
	struct exfat_inode *root;
	clus_t clus_count = 0;

	root = exfat_alloc_inode(ATTR_SUBDIR);
	if (!root)
		return -ENOMEM;

	root->first_clus = le32_to_cpu(exfat->bs->bsx.root_cluster);
	dump_directory(ei, root, (size_t)-1, &clus_count);
	root->size = clus_count * exfat->clus_size;

	ei->exfat->root = root;
	return 0;
}

static int read_file_dentry_set(struct exfat_de_iter *iter,
				struct exfat_inode **new_node, int *skip_dentries)
{
	struct exfat_dentry *file_de, *stream_de, *dentry;
	struct exfat_inode *node = NULL;
	int i, ret;

	ret = exfat_de_iter_get(iter, 0, &file_de);
	if (ret || file_de->type != EXFAT_FILE) {
		exfat_debug("failed to get file dentry\n");
		return -EINVAL;
	}

	ret = exfat_de_iter_get(iter, 1, &stream_de);
	if (ret || stream_de->type != EXFAT_STREAM) {
		exfat_debug("failed to get stream dentry\n");
		*skip_dentries = 2;
		goto skip_dset;
	}

	*new_node = NULL;
	node = exfat_alloc_inode(le16_to_cpu(file_de->file_attr));
	if (!node)
		return -ENOMEM;

	for (i = 2; i <= MIN(file_de->file_num_ext, 1 + MAX_NAME_DENTRIES); i++) {
		ret = exfat_de_iter_get(iter, i, &dentry);
		if (ret || dentry->type != EXFAT_NAME)
			break;
		memcpy(node->name +
		       (i - 2) * ENTRY_NAME_MAX, dentry->name_unicode,
		       sizeof(dentry->name_unicode));
	}

	node->first_clus = le32_to_cpu(stream_de->stream_start_clu);
	node->is_contiguous =
		((stream_de->stream_flags & EXFAT_SF_CONTIGUOUS) != 0);
	node->size = le64_to_cpu(stream_de->stream_size);

	*skip_dentries = i;
	*new_node = node;
	return 0;
skip_dset:
	*new_node = NULL;
	exfat_free_inode(node);
	return -EINVAL;
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

	if (node->attr & ATTR_SUBDIR)
		exfat_stat.dir_count++;
	else
		exfat_stat.file_count++;
	*new_node = node;
	return ret;
}

static int read_bitmap(struct exfat2img *ei, struct exfat_de_iter *iter)
{
	struct exfat *exfat = ei->exfat;
	struct exfat_dentry *dentry;
	int ret;

	ret = exfat_de_iter_get(iter, 0, &dentry);
	if (ret || dentry->type != EXFAT_BITMAP) {
		exfat_debug("failed to get bimtap dentry\n");
		return -EINVAL;
	}

	exfat_debug("start cluster %#x, size %#" PRIx64 "\n",
		    le32_to_cpu(dentry->bitmap_start_clu),
		    le64_to_cpu(dentry->bitmap_size));

	if (!exfat_heap_clus(exfat, le32_to_cpu(dentry->bitmap_start_clu))) {
		exfat_err("invalid start cluster of allocate bitmap. 0x%x\n",
			  le32_to_cpu(dentry->bitmap_start_clu));
		return -EINVAL;
	}

	exfat->disk_bitmap_clus = le32_to_cpu(dentry->bitmap_start_clu);
	exfat->disk_bitmap_size = DIV_ROUND_UP(exfat->clus_count, 8);

	return dump_clusters(ei,
			     exfat->disk_bitmap_clus,
			     exfat->disk_bitmap_clus +
			     DIV_ROUND_UP(exfat->disk_bitmap_size,
					  exfat->clus_size));
}

static int read_upcase_table(struct exfat2img *ei,
			     struct exfat_de_iter *iter)
{
	struct exfat *exfat = ei->exfat;
	struct exfat_dentry *dentry = NULL;
	int retval;
	ssize_t size;

	retval = exfat_de_iter_get(iter, 0, &dentry);
	if (retval || dentry->type != EXFAT_UPCASE) {
		exfat_debug("failed to get upcase dentry\n");
		return -EINVAL;
	}

	if (!exfat_heap_clus(exfat, le32_to_cpu(dentry->upcase_start_clu))) {
		exfat_err("invalid start cluster of upcase table. 0x%x\n",
			  le32_to_cpu(dentry->upcase_start_clu));
		return -EINVAL;
	}

	size = EXFAT_MAX_UPCASE_CHARS * sizeof(__le16);
	return dump_clusters(ei, le32_to_cpu(dentry->upcase_start_clu),
			     le32_to_cpu(dentry->upcase_start_clu) +
			     DIV_ROUND_UP(size, exfat->clus_size));
}

static int read_children(struct exfat2img *ei, struct exfat_inode *dir,
			 off_t *end_file_offset)
{
	struct exfat *exfat = ei->exfat;
	struct exfat_inode *node = NULL;
	struct exfat_dentry *dentry;
	struct exfat_de_iter *de_iter;
	int dentry_count;
	int ret;

	*end_file_offset = 0;
	de_iter = &ei->de_iter;
	ret = exfat_de_iter_init(de_iter, exfat, dir, ei->scan_bdesc);
	if (ret == EOF)
		return 0;
	else if (ret)
		return ret;

	while (1) {
		ret = exfat_de_iter_get(de_iter, 0, &dentry);
		if (ret == EOF) {
			break;
		} else if (ret) {
			ei_err(dir->parent, dir,
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
		case EXFAT_BITMAP:
			if (dir == exfat->root) {
				ret = read_bitmap(ei, de_iter);
				if (ret)
					exfat_debug("failed to read bitmap\n");
			}
			break;
		case EXFAT_UPCASE:
			if (dir == exfat->root) {
				ret = read_upcase_table(ei, de_iter);
				if (ret)
					exfat_debug("failed to upcase table\n");
			}
			break;
		case EXFAT_VOLUME:
		default:
			break;
		}

		ret = exfat_de_iter_advance(de_iter, dentry_count);
	}
out:
	*end_file_offset = exfat_de_iter_file_offset(de_iter);
	exfat_de_iter_flush(de_iter);
	return 0;
err:
	exfat_free_children(dir, false);
	INIT_LIST_HEAD(&dir->children);
	exfat_de_iter_flush(de_iter);
	return ret;
}

static int dump_filesystem(struct exfat2img *ei)
{
	struct exfat *exfat = ei->exfat;
	struct exfat_inode *dir;
	int ret = 0, dir_errors;
	clus_t clus_count;
	off_t end_file_offset;

	if (!exfat->root) {
		exfat_err("root is NULL\n");
		return -ENOENT;
	}

	list_add(&exfat->root->list, &exfat->dir_list);

	while (!list_empty(&exfat->dir_list)) {
		dir = list_entry(exfat->dir_list.next,
				 struct exfat_inode, list);
		clus_count = 0;

		if (!(dir->attr & ATTR_SUBDIR)) {
			ei_err(dir->parent, dir,
			       "failed to travel directories. the node is not directory\n");
			ret = -EINVAL;
			goto out;
		}

		dir_errors = read_children(ei, dir, &end_file_offset);
		if (!dir_errors) {
			dump_directory(ei, dir, (size_t)end_file_offset,
				       &clus_count);
		} else if (dir_errors) {
			dump_directory(ei, dir, (size_t)-1,
				       &clus_count);
			exfat_resolve_path(&path_resolve_ctx, dir);
			exfat_debug("failed to check dentries: %s\n",
				    path_resolve_ctx.local_path);
			ret = dir_errors;
		}

		list_del(&dir->list);
		exfat_free_ancestors(dir);
	}
out:
	exfat_free_dir_list(exfat);
	return ret;
}

static int dump_bytes_to_stdout(struct exfat2img *ei,
				off_t start, off_t end_excl)
{
	struct exfat *exfat = ei->exfat;
	size_t len;
	ssize_t ret;

	if (start != ei->stdout_offset) {
		exfat_err("try to skip for stdout at %llu, expected: %llu\n",
			  (unsigned long long)start,
			  (unsigned long long)ei->stdout_offset);
		return -EINVAL;
	}

	while (start < end_excl) {
		len = (size_t)MIN(end_excl - start, exfat->clus_size);
		ret = exfat_read(exfat->blk_dev->dev_fd, ei->dump_cluster,
				 len, start);
		if (ret != (ssize_t)len) {
			exfat_err("failed to read %llu bytes at %llu\n",
				  (unsigned long long)len,
				  (unsigned long long)start);
			return -EIO;
		}

		ret = write(ei->out_fd, ei->dump_cluster, len);
		if (ret != (ssize_t)len) {
			exfat_err("failed to write %llu bytes at %llu\n",
				  (unsigned long long)len,
				  (unsigned long long)start);
			return -EIO;
		}

		start += len;
		ei->stdout_offset += len;
		exfat_stat.written_bytes += len;
	}
	return 0;
}

static int dump_clusters_to_stdout(struct exfat2img *ei,
				   unsigned int start_clu, unsigned int end_clu,
				   bool fill_zero)
{
	unsigned int clu, clu_count;
	unsigned char cc;
	unsigned int cc_clu_count, cc_len;
	off_t start_off, end_off_excl;
	char buf[1 + EI_CC_PAYLOAD_LEN];

	clu = start_clu;
	clu_count = end_clu - start_clu + 1;

	if (ei->save_cc) {
		/* if the count of clusters is less than 5, use SKIP_1 or COPY_2 */
		cc_clu_count = clu_count < 5 ? 1 : clu_count;
		cc_len = cc_clu_count == 1 ? 1 : 1 + EI_CC_PAYLOAD_LEN;
		if (fill_zero)
			cc = cc_clu_count == 1 ? EI_CC_SKIP_1 : EI_CC_SKIP_2;
		else
			cc = cc_clu_count == 1 ? EI_CC_COPY_1 : EI_CC_COPY_2;
	} else {
		cc = EI_CC_INVALID;
		cc_clu_count = clu_count;
	}

	while (clu <= end_clu) {
		if (cc != EI_CC_INVALID) {
			buf[0] = cc;
			*((__le32 *)&buf[1]) =
				cpu_to_le32(cc_clu_count);
			if (write(ei->out_fd, buf, cc_len) != (ssize_t)cc_len) {
				exfat_err("failed to write cc %d : %u\n for %u ~ %u clusters\n",
					  cc, cc_clu_count,
					  start_clu, start_clu + cc_clu_count - 1);
			}
		}

		if (cc == EI_CC_COPY_1 || cc == EI_CC_COPY_2) {
			start_off = exfat_c2o(ei->exfat, clu);
			end_off_excl = exfat_c2o(ei->exfat, clu + cc_clu_count);

			if (dump_bytes_to_stdout(ei, start_off, end_off_excl) < 0)
				return -EIO;
		} else {
			ei->stdout_offset += (off_t)cc_clu_count * ei->exfat->clus_size;
		}
		clu += cc_clu_count;
	}

	return 0;
}

static int dump_to_stdout(struct exfat2img *ei)
{
	struct exfat *exfat = ei->exfat;
	off_t start_off, end_off;
	unsigned int clu, last_clu, next_clu;
	unsigned int start_clu, end_clu;

	start_off = 0;
	end_off = exfat_s2o(exfat, le32_to_cpu(exfat->bs->bsx.clu_offset));
	if (dump_bytes_to_stdout(ei, start_off, end_off) < 0) {
		exfat_err("failed to dump boot sectors and FAT tables\n");
		return -EIO;
	}

	clu = EXFAT_FIRST_CLUSTER;
	last_clu = clu + exfat->clus_count;
	while (clu < last_clu) {
		/* read and write clusters for allocated ones */
		start_clu = 0;
		while (clu < last_clu &&
		       exfat_bitmap_get(exfat->alloc_bitmap, clu)) {
			if (!start_clu)
				start_clu = clu;
			end_clu = clu;
			clu++;
		}

		if (start_clu) {
			if (dump_clusters_to_stdout(ei, start_clu, end_clu, false) < 0) {
				start_off = exfat_c2o(exfat, start_clu);
				end_off = exfat_c2o(exfat, end_clu);
				exfat_err("failed to dump range from %llx to %llx\n",
					  (unsigned long long)start_off,
					  (unsigned long long)end_off);
				return -EIO;
			}
		}

		/* exit if all of the remaining clusters are free */
		if (clu >= last_clu)
			break;
		if (exfat_bitmap_find_one(exfat, exfat->alloc_bitmap,
					  clu, &next_clu))
			next_clu = EXFAT_FIRST_CLUSTER + exfat->clus_count;

		/* write zeroes for free clusters */
		start_clu = clu;
		end_clu = next_clu - 1;
		if (dump_clusters_to_stdout(ei, start_clu, end_clu, true) < 0) {
			start_off = exfat_c2o(exfat, start_clu);
			end_off = exfat_c2o(exfat, end_clu);
			exfat_err("failed to dump zero range from %llx to %llx\n",
				  (unsigned long long)start_off,
				  (unsigned long long)end_off);
			return -EIO;
		}

		clu = next_clu;
	}

	return 0;
}

static int dump_header(struct exfat2img *ei)
{
	struct exfat *exfat = ei->exfat;

	ei_hdr.magic = cpu_to_le32(EI_MAGIC);
	ei_hdr.major_version = cpu_to_le32(1);
	ei_hdr.minor_version = cpu_to_le32(0);
	ei_hdr.data_offset = cpu_to_le32(sizeof(struct exfat2img_hdr));
	ei_hdr.heap_clus_offset =
		cpu_to_le32(le32_to_cpu(exfat->bs->bsx.clu_offset) *
			    exfat->sect_size);
	ei_hdr.cluster_size = cpu_to_le32(exfat->clus_size);
	ei_hdr.cluster_count = cpu_to_le32(exfat->clus_count);

	if (write(ei->out_fd, &ei_hdr, sizeof(ei_hdr)) != (ssize_t)sizeof(ei_hdr)) {
		exfat_err("failed to write exfat2img header\n");
		return -EIO;
	}
	return 0;
}

static ssize_t read_stream(int fd, void *buf, size_t len)
{
	size_t read_len = 0;
	ssize_t ret;

	while (read_len < len) {
		ret = read(fd, buf, len - read_len);
		if (ret < 0) {
			if (errno != -EAGAIN && errno != -EINTR)
				return -1;
			ret = 0;
		} else if (ret == 0) {
			return 0;
		}
		buf = (char *)buf + (size_t)ret;
		read_len += (size_t)ret;
	}
	return read_len;
}

static int restore_from_stdin(struct exfat2img *ei)
{
	int in_fd, ret = 0;
	unsigned char cc;
	unsigned int clu, end_clu;
	unsigned int cc_clu_count;
	unsigned int clus_size;
	__le32 t_cc_clu_count;
	off_t out_start_off, out_end_off_excl;
	off_t in_start_off;
	size_t len;

	in_fd = fileno(stdin);
	if (in_fd < 0) {
		exfat_err("failed to get fd from stdin\n");
		return in_fd;
	}

	if (read_stream(in_fd, &ei_hdr, sizeof(ei_hdr)) != (ssize_t)sizeof(ei_hdr)) {
		exfat_err("failed to read a header\n");
		return -EIO;
	}

	if (le32_to_cpu(ei_hdr.magic) != EI_MAGIC) {
		exfat_err("header has invalid magic %#x, expected %#x\n",
			  le32_to_cpu(ei_hdr.magic), EI_MAGIC);
		return -EINVAL;
	}

	clus_size = le32_to_cpu(ei_hdr.cluster_size);

	ei->out_fd = ei->bdev.dev_fd;
	ei->dump_cluster = malloc(clus_size);
	if (!ei->dump_cluster)
		return -ENOMEM;

	/* restore boot regions, and FAT tables */
	in_start_off = le32_to_cpu(ei_hdr.data_offset);
	out_start_off = 0;
	out_end_off_excl = le32_to_cpu(ei_hdr.heap_clus_offset);
	while (out_start_off < out_end_off_excl) {
		len = MIN(out_end_off_excl - out_start_off, clus_size);
		if (read_stream(in_fd, ei->dump_cluster, len) != (ssize_t)len) {
			exfat_err("failed to read first meta region. %llu ~ %llu\n",
				  (unsigned long long)in_start_off,
				  (unsigned long long)in_start_off + len);
			ret = -EIO;
			goto out;
		}

		if (pwrite(ei->out_fd, ei->dump_cluster, len, out_start_off)
		    != (ssize_t)len) {
			exfat_err("failed to write first meta region. %llu ~ %llu\n",
				  (unsigned long long)out_start_off,
				  (unsigned long long)out_start_off + len);
			ret = -EIO;
			goto out;
		}

		out_start_off += len;
		in_start_off += len;
	}

	/* restore heap clusters */
	clu = 0;
	while (clu < le32_to_cpu(ei_hdr.cluster_count)) {
		if (read_stream(in_fd, &cc, sizeof(cc)) != (ssize_t)sizeof(cc)) {
			exfat_err("failed to read cc at %llu\n",
				  (unsigned long long)in_start_off);
			ret = -EIO;
			goto out;
		}
		in_start_off += 1;

		if (cc == EI_CC_COPY_2 || cc == EI_CC_SKIP_2) {
			if (read_stream(in_fd, &t_cc_clu_count, EI_CC_PAYLOAD_LEN) !=
			    (ssize_t)EI_CC_PAYLOAD_LEN) {
				exfat_err("failed to read cc cluster count at %llu\n",
					  (unsigned long long)in_start_off);
				ret = -EIO;
				goto out;
			}
			cc_clu_count = le32_to_cpu(t_cc_clu_count);
			in_start_off += EI_CC_PAYLOAD_LEN;
		} else if (cc == EI_CC_COPY_1 || cc == EI_CC_SKIP_1) {
			cc_clu_count = 1;
		} else {
			exfat_err("unexpected cc %d at %llu\n",
				  cc, (unsigned long long)in_start_off);
			ret = -EINVAL;
			goto out;
		}

		if (cc == EI_CC_COPY_1 || cc == EI_CC_COPY_2) {
			end_clu = clu + cc_clu_count;
			while (clu < end_clu) {
				if (read_stream(in_fd, ei->dump_cluster,
						clus_size) != (ssize_t)clus_size) {
					exfat_err("failed to read range %llu ~ %llu\n",
						  (unsigned long long)in_start_off,
						  (unsigned long long)in_start_off + clus_size);
					ret = -EIO;
					goto out;
				}
				if (pwrite(ei->out_fd, ei->dump_cluster,
					   clus_size, out_start_off) != (ssize_t)clus_size) {
					exfat_err("failed to write range %llu ~ %llu\n",
						  (unsigned long long)out_start_off,
						  (unsigned long long)out_start_off + clus_size);
					ret = -EIO;
					goto out;
				}

				out_start_off += clus_size;
				in_start_off += clus_size;
				clu++;
			}
		} else {
			out_start_off += (off_t)cc_clu_count * clus_size;
			in_start_off +=  (off_t)cc_clu_count * clus_size;
			if (lseek(ei->out_fd, out_start_off, SEEK_SET) == (off_t)-1) {
				exfat_err("failed to seek to %llu\n",
					  (unsigned long long)out_start_off);
				ret = -EIO;
				goto out;
			}
			clu += cc_clu_count;
		}
	}
out:
	if (fsync(ei->out_fd)) {
		exfat_err("failed to fsync: %d\n", errno);
		ret = -EIO;
	}
	free(ei->dump_cluster);
	return ret;
}

int main(int argc, char * const argv[])
{
	int err = 0, c;
	const char *in_path, *out_path = NULL, *blkdev_path;
	struct pbr *bs;
	struct exfat_user_input ui;
	off_t last_sect;
	bool restore;

	print_level = EXFAT_ERROR;

	opterr = 0;
	while ((c = getopt_long(argc, argv, "o:Vh", opts, NULL)) != EOF) {
		switch (c) {
		case 'o':
			out_path = optarg;
			break;
		case 'V':
			show_version();
			return 0;
		case 'h':
			/* Fall through */
		default:
			usage(argv[0]);
			break;
		}
	}

	show_version();
	if (!(optind == argc - 1 && out_path) &&
	    !(optind == argc - 2 && !out_path))
		usage(argv[0]);

	in_path = argv[optind++];
	if (!out_path)
		out_path = argv[optind++];

	if (!strcmp(in_path, "-")) {
		restore = true;
		blkdev_path = out_path;
	} else {
		restore = false;
		blkdev_path = in_path;
	}

	memset(&ui, 0, sizeof(ui));
	snprintf(ui.dev_name, sizeof(ui.dev_name), "%s", blkdev_path);
	if (restore)
		ui.writeable = true;
	else
		ui.writeable = false;

	if (exfat_get_blk_dev_info(&ui, &ei.bdev)) {
		exfat_err("failed to open %s\n", ui.dev_name);
		return EXIT_FAILURE;
	}

	if (restore)
		return restore_from_stdin(&ei);

	err = read_boot_sect(&ei.bdev, &bs);
	if (err) {
		close(ei.bdev.dev_fd);
		return EXIT_FAILURE;
	}

	err = create_exfat2img(&ei, bs, out_path);
	if (err)
		return EXIT_FAILURE;

	if (!ei.is_stdout) {
		err = dump_sectors(&ei, 0, le32_to_cpu(ei.exfat->bs->bsx.clu_offset));
		if (err) {
			exfat_err("failed to dump boot sectors, fat\n");
			goto out;
		}

		last_sect = (off_t)le32_to_cpu(ei.exfat->bs->bsx.clu_offset) +
			(le32_to_cpu(ei.exfat->bs->bsx.clu_count) <<
			 ei.exfat->bs->bsx.sect_per_clus_bits) - 1;
		err = dump_sectors(&ei, last_sect, last_sect + 1);
		if (err) {
			exfat_err("failed to dump last sector\n");
			goto out;
		}
	}

	err = dump_root(&ei);
	if (err) {
		exfat_err("failed to dump root\n");
		goto out;
	}

	dump_filesystem(&ei);

	if (ei.is_stdout) {
		err = dump_header(&ei);
		if (err)
			goto out;
		err = dump_to_stdout(&ei);
		if (err)
			goto out;
	} else {
		err = fsync(ei.out_fd);
		if (err) {
			exfat_err("failed to fsync %s. %d\n", out_path, errno);
			goto out;
		}
		close(ei.out_fd);
	}

	printf("%ld files found, %ld directories dumped, %llu kbytes written\n",
	       exfat_stat.file_count,
	       exfat_stat.dir_count,
	       (unsigned long long)DIV_ROUND_UP(exfat_stat.written_bytes, 1024));

out:
	free_exfat2img(&ei);
	return err == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
