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

struct exfat_de_iter {
	struct exfat		*exfat;
	struct exfat_inode	*parent;
	unsigned char		*dentries;	/* cluster * 2 allocated */
	unsigned int		read_size;	/* cluster size */
	off_t			de_file_offset;	/* offset in dentries buffer */
	off_t			next_read_offset;
	int			max_skip_dentries;
};

enum fsck_ui_options {
	FSCK_OPTS_REPAIR_ASK	= 0x01,
	FSCK_OPTS_REPAIR_YES	= 0x02,
	FSCK_OPTS_REPAIR_NO	= 0x04,
	FSCK_OPTS_REPAIR	= 0x07,
};

struct exfat {
	enum fsck_ui_options	options;
	struct exfat_blk_dev	*blk_dev;
	struct pbr		*bs;
	char			volume_label[VOLUME_LABEL_BUFFER_SIZE];
	struct exfat_inode	*root;
	struct list_head	dir_list;
	struct exfat_de_iter	de_iter;
	__u32			*alloc_bitmap;
	__u64			bit_count;
};

#define EXFAT_CLUSTER_SIZE(pbr) (1 << ((pbr)->bsx.sect_size_bits +	\
					(pbr)->bsx.sect_per_clus_bits))
#define EXFAT_SECTOR_SIZE(pbr) (1 << (pbr)->bsx.sect_size_bits)

#endif
