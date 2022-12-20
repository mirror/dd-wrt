/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Copyright (C) 2019 Namjae Jeon <linkinjeon@kernel.org>
 */

#ifndef _MKFS_H

#define MIN_NUM_SECTOR			(2048)
#define EXFAT_MAX_CLUSTER_SIZE		(32*1024*1024)

struct exfat_mkfs_info {
	unsigned int total_clu_cnt;
	unsigned int used_clu_cnt;
	unsigned int fat_byte_off;
	unsigned int fat_byte_len;
	unsigned int clu_byte_off;
	unsigned int bitmap_byte_off;
	unsigned int bitmap_byte_len;
	unsigned int ut_byte_off;
	unsigned int ut_start_clu;
	unsigned int ut_clus_off;
	unsigned int ut_byte_len;
	unsigned int root_byte_off;
	unsigned int root_byte_len;
	unsigned int root_start_clu;
	unsigned int volume_serial;
};

extern struct exfat_mkfs_info finfo;

int exfat_create_upcase_table(struct exfat_blk_dev *bd);

#endif /* !_MKFS_H */
