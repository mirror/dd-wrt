// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2020 Hyunchul Lee <hyc.lee@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "exfat_ondisk.h"
#include "libexfat.h"
#include "fsck.h"

static ssize_t write_clus(struct exfat_de_iter *iter, int bufidx)
{
	off_t device_offset;
	struct exfat *exfat = iter->exfat;
	struct buffer_desc *desc;
	unsigned int i;

	desc = &iter->buffer_desc[bufidx];
	device_offset = exfat_c2o(exfat, desc->p_clus);

	for (i = 0; i < iter->read_size / iter->write_size; i++) {
		if (desc->dirty[i]) {
			if (exfat_write(exfat->blk_dev->dev_fd,
					desc->buffer + i * iter->write_size,
					iter->write_size,
					device_offset + i * iter->write_size)
					!= (ssize_t)iter->write_size)
				return -EIO;
			desc->dirty[i] = 0;
		}
	}
	return 0;
}

static ssize_t read_next_clus(struct exfat_de_iter *iter, clus_t l_clus)
{
	struct exfat *exfat = iter->exfat;
	struct buffer_desc *desc;
	off_t device_offset;
	int ret;

	desc = &iter->buffer_desc[l_clus & 0x01];
	if (l_clus == 0)
		desc->p_clus = iter->parent->first_clus;

	if (write_clus(iter, l_clus & 0x01))
		return -EIO;

	if (l_clus > 0) {
		ret = get_next_clus(exfat, iter->parent,
				iter->buffer_desc[(l_clus - 1) & 0x01].p_clus,
				&desc->p_clus);
		if (ret)
			return ret;
	}
	device_offset = exfat_c2o(exfat, desc->p_clus);
	return exfat_read(exfat->blk_dev->dev_fd, desc->buffer,
			iter->read_size, device_offset);
}

int exfat_de_iter_init(struct exfat_de_iter *iter, struct exfat *exfat,
				struct exfat_inode *dir)
{
	iter->exfat = exfat;
	iter->parent = dir;
	iter->read_size = exfat->clus_size;
	iter->write_size = exfat->sect_size;

	if (!iter->buffer_desc)
		iter->buffer_desc = exfat->buffer_desc;

	if (read_next_clus(iter, 0) != (ssize_t)iter->read_size) {
		exfat_err("failed to read directory entries.\n");
		return -EIO;
	}

	iter->de_file_offset = 0;
	iter->next_read_offset = iter->read_size;
	iter->max_skip_dentries = 0;
	return 0;
}

int exfat_de_iter_get(struct exfat_de_iter *iter,
			int ith, struct exfat_dentry **dentry)
{
	off_t next_de_file_offset;
	ssize_t ret;
	clus_t next_l_clus;

	next_de_file_offset = iter->de_file_offset +
			ith * sizeof(struct exfat_dentry);
	next_l_clus = (clus_t) (next_de_file_offset / iter->read_size);

	if (next_de_file_offset + sizeof(struct exfat_dentry) >
		iter->parent->size)
		return EOF;
	/*
	 * desired dentry must be in current, or next cluster which
	 * will be read
	 */
	if (next_l_clus > iter->de_file_offset / iter->read_size + 1)
		return -ERANGE;

	/* read next cluster if needed */
	if (next_de_file_offset >= iter->next_read_offset) {
		ret = read_next_clus(iter, next_l_clus);
		if (ret == EOF) {
			return EOF;
		} else if (ret != (ssize_t)iter->read_size) {
			exfat_err("failed to read a cluster. %zd\n", ret);
			return ret;
		}
		iter->next_read_offset += iter->read_size;
	}

	if (ith + 1 > iter->max_skip_dentries)
		iter->max_skip_dentries = ith + 1;

	*dentry = (struct exfat_dentry *)
			(iter->buffer_desc[next_l_clus & 0x01].buffer +
			next_de_file_offset % iter->read_size);
	return 0;
}

int exfat_de_iter_get_dirty(struct exfat_de_iter *iter,
			int ith, struct exfat_dentry **dentry)
{
	off_t next_file_offset;
	clus_t l_clus;
	int ret, sect_idx;

	ret = exfat_de_iter_get(iter, ith, dentry);
	if (!ret) {
		next_file_offset = iter->de_file_offset +
				ith * sizeof(struct exfat_dentry);
		l_clus = (clus_t)(next_file_offset / iter->read_size);
		sect_idx = (int)(next_file_offset / iter->write_size);
		iter->buffer_desc[l_clus & 0x01].dirty[sect_idx] = 1;
	}

	return ret;
}

int exfat_de_iter_flush(struct exfat_de_iter *iter)
{
	if (write_clus(iter, 0) ||
		write_clus(iter, 1))
		return -EIO;
	return 0;
}

/*
 * @skip_dentries must be the largest @ith + 1 of exfat_de_iter_get
 * since the last call of exfat_de_iter_advance
 */
int exfat_de_iter_advance(struct exfat_de_iter *iter, int skip_dentries)
{
	if (skip_dentries != iter->max_skip_dentries)
		return -EINVAL;

	iter->max_skip_dentries = 0;
	iter->de_file_offset = iter->de_file_offset +
				skip_dentries * sizeof(struct exfat_dentry);
	return 0;
}

off_t exfat_de_iter_file_offset(struct exfat_de_iter *iter)
{
	return iter->de_file_offset;
}
