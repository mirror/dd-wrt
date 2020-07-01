/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Copyright (C) 2020 Hyunchul Lee <hyc.lee@gmail.com>
 */
#ifndef _FSCK_H
#define _FSCK_H

#include "list.h"

typedef __u32 clus_t;

struct exfat_inode {
	struct exfat_inode	*parent;
	struct list_head	children;
	struct list_head	sibling;
	struct list_head	list;
	clus_t			first_clus;
	clus_t			last_lclus;
	clus_t			last_pclus;
	__u16			attr;
	uint64_t		size;
	bool			is_contiguous;
	off_t			dentry_file_offset;
	__le16			name[0];	/* only for directory */
};

#define EXFAT_NAME_MAX			255
#define NAME_BUFFER_SIZE		((EXFAT_NAME_MAX+1)*2)

struct buffer_desc {
	clus_t		p_clus;
	char		*buffer;
	char		*dirty;
};

struct exfat_de_iter {
	struct exfat		*exfat;
	struct exfat_inode	*parent;
	struct buffer_desc	*buffer_desc;		/* cluster * 2 */
	unsigned int		read_size;		/* cluster size */
	unsigned int		write_size;		/* sector size */
	off_t			de_file_offset;
	off_t			next_read_offset;
	int			max_skip_dentries;
};

enum fsck_ui_options {
	FSCK_OPTS_REPAIR_ASK	= 0x01,
	FSCK_OPTS_REPAIR_YES	= 0x02,
	FSCK_OPTS_REPAIR_NO	= 0x04,
	FSCK_OPTS_REPAIR_AUTO	= 0x08,
	FSCK_OPTS_REPAIR_WRITE	= 0x0b,
	FSCK_OPTS_REPAIR_ALL	= 0x0f,
};

struct exfat {
	enum fsck_ui_options	options;
	bool			dirty:1;
	bool			dirty_fat:1;
	struct exfat_blk_dev	*blk_dev;
	struct pbr		*bs;
	char			volume_label[VOLUME_LABEL_BUFFER_SIZE];
	struct exfat_inode	*root;
	struct list_head	dir_list;
	clus_t			clus_count;
	unsigned int		clus_size;
	unsigned int		sect_size;
	struct exfat_de_iter	de_iter;
	struct buffer_desc	buffer_desc[2];	/* cluster * 2 */
	char			*alloc_bitmap;
	char			*disk_bitmap;
	clus_t			disk_bitmap_clus;
	unsigned int		disk_bitmap_size;
};

#define EXFAT_CLUSTER_SIZE(pbr) (1 << ((pbr)->bsx.sect_size_bits +	\
					(pbr)->bsx.sect_per_clus_bits))
#define EXFAT_SECTOR_SIZE(pbr) (1 << (pbr)->bsx.sect_size_bits)

/* fsck.c */
off_t exfat_c2o(struct exfat *exfat, unsigned int clus);
int get_next_clus(struct exfat *exfat, struct exfat_inode *node,
				clus_t clus, clus_t *next);

/* de_iter.c */
int exfat_de_iter_init(struct exfat_de_iter *iter, struct exfat *exfat,
				struct exfat_inode *dir);
int exfat_de_iter_get(struct exfat_de_iter *iter,
			int ith, struct exfat_dentry **dentry);
int exfat_de_iter_get_dirty(struct exfat_de_iter *iter,
			int ith, struct exfat_dentry **dentry);
int exfat_de_iter_flush(struct exfat_de_iter *iter);
int exfat_de_iter_advance(struct exfat_de_iter *iter, int skip_dentries);
off_t exfat_de_iter_file_offset(struct exfat_de_iter *iter);

#endif
