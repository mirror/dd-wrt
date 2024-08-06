// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2021 LG Electronics.
 *
 *   Author(s): Hyunchul Lee <hyc.lee@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "exfat_ondisk.h"
#include "libexfat.h"
#include "exfat_fs.h"
#include "exfat_dir.h"

static struct path_resolve_ctx path_resolve_ctx;

#define fsck_err(parent, inode, fmt, ...)		\
({							\
		exfat_resolve_path_parent(&path_resolve_ctx,	\
			parent, inode);			\
		exfat_err("ERROR: %s: " fmt,		\
			path_resolve_ctx.local_path,	\
			##__VA_ARGS__);			\
})

static inline struct buffer_desc *exfat_de_iter_get_buffer(
		struct exfat_de_iter *iter, unsigned int block)
{
	return &iter->buffer_desc[block % iter->exfat->buffer_count];
}

static ssize_t write_block(struct exfat_de_iter *iter, unsigned int block)
{
	off_t device_offset;
	struct exfat *exfat = iter->exfat;
	struct buffer_desc *desc;
	unsigned int i;

	desc = exfat_de_iter_get_buffer(iter, block);

	for (i = 0; i < iter->read_size / iter->write_size; i++) {
		if (BITMAP_GET(desc->dirty, i)) {
			device_offset = exfat_c2o(exfat, desc->p_clus) +
				desc->offset;
			if (exfat_write(exfat->blk_dev->dev_fd,
					desc->buffer + i * iter->write_size,
					iter->write_size,
					device_offset + i * iter->write_size)
					!= (ssize_t)iter->write_size)
				return -EIO;
			BITMAP_CLEAR(desc->dirty, i);
		}
	}
	return 0;
}

static int read_ahead_first_blocks(struct exfat_de_iter *iter)
{
#ifdef POSIX_FADV_WILLNEED
	struct exfat *exfat = iter->exfat;
	clus_t clus_count;
	unsigned int size;

	clus_count = iter->parent->size / exfat->clus_size;

	if (clus_count > 1) {
		iter->ra_begin_offset = 0;
		iter->ra_next_clus = 1;
		size = exfat->clus_size;
	} else {
		iter->ra_begin_offset = 0;
		iter->ra_next_clus = 0;
		size = iter->ra_partial_size;
	}
	return posix_fadvise(exfat->blk_dev->dev_fd,
			exfat_c2o(exfat, iter->parent->first_clus), size,
			POSIX_FADV_WILLNEED);
#else
	return -ENOTSUP;
#endif
}

/**
 * read the next fragment in advance, and assume the fragment
 * which covers @clus is already read.
 */
static int read_ahead_next_blocks(struct exfat_de_iter *iter,
		clus_t clus, unsigned int offset, clus_t p_clus)
{
#ifdef POSIX_FADV_WILLNEED
	struct exfat *exfat = iter->exfat;
	off_t device_offset;
	clus_t clus_count, ra_clus, ra_p_clus;
	unsigned int size;
	int ret = 0;

	clus_count = iter->parent->size / exfat->clus_size;
	if (clus + 1 < clus_count) {
		ra_clus = clus + 1;
		if (ra_clus == iter->ra_next_clus &&
				offset >= iter->ra_begin_offset) {
			ret = exfat_get_inode_next_clus(exfat, iter->parent,
							p_clus, &ra_p_clus);
			if (ret)
				return ret;

			if (ra_p_clus == EXFAT_EOF_CLUSTER)
				return -EIO;

			device_offset = exfat_c2o(exfat, ra_p_clus);
			size = ra_clus + 1 < clus_count ?
				exfat->clus_size : iter->ra_partial_size;
			ret = posix_fadvise(exfat->blk_dev->dev_fd,
					device_offset, size,
					POSIX_FADV_WILLNEED);
			iter->ra_next_clus = ra_clus + 1;
			iter->ra_begin_offset = 0;
		}
	} else {
		if (offset >= iter->ra_begin_offset &&
				offset + iter->ra_partial_size <=
				exfat->clus_size) {
			device_offset = exfat_c2o(exfat, p_clus) +
				offset + iter->ra_partial_size;
			ret = posix_fadvise(exfat->blk_dev->dev_fd,
					device_offset, iter->ra_partial_size,
					POSIX_FADV_WILLNEED);
			iter->ra_begin_offset =
				offset + iter->ra_partial_size;
		}
	}

	return ret;
#else
	return -ENOTSUP;
#endif
}

static int read_ahead_next_dir_blocks(struct exfat_de_iter *iter)
{
#ifdef POSIX_FADV_WILLNEED
	struct exfat *exfat = iter->exfat;
	struct list_head *current;
	struct exfat_inode *next_inode;
	off_t offset;

	if (list_empty(&exfat->dir_list))
		return -EINVAL;

	current = exfat->dir_list.next;
	if (iter->parent == list_entry(current, struct exfat_inode, list) &&
			current->next != &exfat->dir_list) {
		next_inode = list_entry(current->next, struct exfat_inode,
				list);
		offset = exfat_c2o(exfat, next_inode->first_clus);
		return posix_fadvise(exfat->blk_dev->dev_fd, offset,
				iter->ra_partial_size,
				POSIX_FADV_WILLNEED);
	}

	return 0;
#else
	return -ENOTSUP;
#endif
}

static ssize_t read_block(struct exfat_de_iter *iter, unsigned int block)
{
	struct exfat *exfat = iter->exfat;
	struct buffer_desc *desc, *prev_desc;
	off_t device_offset;
	ssize_t ret;

	desc = exfat_de_iter_get_buffer(iter, block);
	if (block == 0) {
		desc->p_clus = iter->parent->first_clus;
		desc->offset = 0;
	}

	/* if the buffer already contains dirty dentries, write it */
	if (write_block(iter, block))
		return -EIO;

	if (block > 0) {
		if (block > iter->parent->size / iter->read_size)
			return EOF;

		prev_desc = exfat_de_iter_get_buffer(iter, block - 1);
		if (prev_desc->offset + 2 * iter->read_size <=
				exfat->clus_size) {
			desc->p_clus = prev_desc->p_clus;
			desc->offset = prev_desc->offset + iter->read_size;
		} else {
			ret = exfat_get_inode_next_clus(exfat, iter->parent,
							prev_desc->p_clus, &desc->p_clus);
			desc->offset = 0;
			if (ret)
				return ret;
			else if (desc->p_clus == EXFAT_EOF_CLUSTER)
				return EOF;
		}
	}

	device_offset = exfat_c2o(exfat, desc->p_clus) + desc->offset;
	ret = exfat_read(exfat->blk_dev->dev_fd, desc->buffer,
			iter->read_size, device_offset);
	if (ret <= 0)
		return ret;

	/*
	 * if a buffer is filled with dentries, read blocks ahead of time,
	 * otherwise read blocks of the next directory in advance.
	 */
	if (desc->buffer[iter->read_size - 32] != EXFAT_LAST)
		read_ahead_next_blocks(iter,
				(block * iter->read_size) / exfat->clus_size,
				(block * iter->read_size) % exfat->clus_size,
				desc->p_clus);
	else
		read_ahead_next_dir_blocks(iter);
	return ret;
}

int exfat_de_iter_init(struct exfat_de_iter *iter, struct exfat *exfat,
		       struct exfat_inode *dir, struct buffer_desc *bd)
{
	iter->exfat = exfat;
	iter->parent = dir;
	iter->write_size = exfat->sect_size;
	iter->read_size = exfat_get_read_size(exfat);
	if (exfat->clus_size <= 32 * KB)
		iter->ra_partial_size = MAX(4 * KB, exfat->clus_size / 2);
	else
		iter->ra_partial_size = exfat->clus_size / 4;
	iter->ra_partial_size = MIN(iter->ra_partial_size, 8 * KB);

	iter->buffer_desc = bd;

	iter->de_file_offset = 0;
	iter->next_read_offset = iter->read_size;
	iter->max_skip_dentries = 0;
	iter->invalid_name_num = 0;

	if (iter->parent->size == 0)
		return EOF;

	read_ahead_first_blocks(iter);
	if (read_block(iter, 0) != (ssize_t)iter->read_size) {
		exfat_err("failed to read directory entries.\n");
		return -EIO;
	}

	return 0;
}

int exfat_de_iter_get(struct exfat_de_iter *iter,
			int ith, struct exfat_dentry **dentry)
{
	off_t next_de_file_offset;
	ssize_t ret;
	unsigned int block;
	struct buffer_desc *bd;

	next_de_file_offset = iter->de_file_offset +
			ith * sizeof(struct exfat_dentry);
	block = (unsigned int)(next_de_file_offset / iter->read_size);

	if (next_de_file_offset + sizeof(struct exfat_dentry) >
		iter->parent->size)
		return EOF;

	/* read next cluster if needed */
	if (next_de_file_offset >= iter->next_read_offset) {
		ret = read_block(iter, block);
		if (ret != (ssize_t)iter->read_size)
			return ret;
		iter->next_read_offset += iter->read_size;
	}

	if (ith + 1 > iter->max_skip_dentries)
		iter->max_skip_dentries = ith + 1;

	bd = exfat_de_iter_get_buffer(iter, block);
	*dentry = (struct exfat_dentry *)(bd->buffer +
			next_de_file_offset % iter->read_size);
	return 0;
}

int exfat_de_iter_get_dirty(struct exfat_de_iter *iter,
			int ith, struct exfat_dentry **dentry)
{
	off_t next_file_offset;
	unsigned int block;
	int ret, sect_idx;
	struct buffer_desc *bd;

	ret = exfat_de_iter_get(iter, ith, dentry);
	if (!ret) {
		next_file_offset = iter->de_file_offset +
				ith * sizeof(struct exfat_dentry);
		block = (unsigned int)(next_file_offset / iter->read_size);
		sect_idx = (int)((next_file_offset % iter->read_size) /
				iter->write_size);
		bd = exfat_de_iter_get_buffer(iter, block);
		BITMAP_SET(bd->dirty, sect_idx);
	}

	return ret;
}

int exfat_de_iter_flush(struct exfat_de_iter *iter)
{
	unsigned int i;

	for (i = 0; i < iter->exfat->buffer_count; i++)
		if (write_block(iter, i))
			return -EIO;
	return 0;
}

int exfat_de_iter_advance(struct exfat_de_iter *iter, int skip_dentries)
{
	if (skip_dentries > iter->max_skip_dentries)
		return -EINVAL;

	iter->max_skip_dentries = 0;
	iter->de_file_offset = iter->de_file_offset +
				skip_dentries * sizeof(struct exfat_dentry);
	return 0;
}

off_t exfat_de_iter_device_offset(struct exfat_de_iter *iter)
{
	struct buffer_desc *bd;
	unsigned int block;

	if ((uint64_t)iter->de_file_offset >= iter->parent->size)
		return EOF;

	block = iter->de_file_offset / iter->read_size;
	bd = exfat_de_iter_get_buffer(iter, block);
	return exfat_c2o(iter->exfat, bd->p_clus) + bd->offset +
		iter->de_file_offset % iter->read_size;
}

off_t exfat_de_iter_file_offset(struct exfat_de_iter *iter)
{
	return iter->de_file_offset;
}

/*
 * try to find the dentry set matched with @filter. this function
 * doesn't verify the dentry set.
 *
 * if found, return 0. if not found, return EOF. otherwise return errno.
 */
int exfat_lookup_dentry_set(struct exfat *exfat, struct exfat_inode *parent,
			    struct exfat_lookup_filter *filter)
{
	struct buffer_desc *bd = NULL;
	struct exfat_dentry *dentry = NULL;
	off_t free_file_offset = 0, free_dev_offset = 0;
	struct exfat_de_iter de_iter;
	int dentry_count, empty_dentry_count = 0;
	int retval;

	if (!exfat->lookup_buffer) {
		exfat->lookup_buffer = exfat_alloc_buffer(exfat);
		if (!exfat->lookup_buffer)
			return -ENOMEM;
	}
	bd = exfat->lookup_buffer;

	retval = exfat_de_iter_init(&de_iter, exfat, parent, bd);
	if (retval == EOF || retval)
		goto out;

	filter->out.dentry_set = NULL;
	while (1) {
		retval = exfat_de_iter_get(&de_iter, 0, &dentry);
		if (retval == EOF) {
			break;
		} else if (retval) {
			fsck_err(parent->parent, parent,
				 "failed to get a dentry. %d\n", retval);
			goto out;
		}

		if (!IS_EXFAT_DELETED(dentry->type)) {
			if (filter->in.dentry_count == 0 ||
			    empty_dentry_count < filter->in.dentry_count)
				empty_dentry_count = 0;
		}

		dentry_count = 1;
		if (dentry->type == filter->in.type) {
			retval = 0;
			if (filter->in.filter)
				retval = filter->in.filter(&de_iter,
							filter->in.param,
							&dentry_count);

			if (retval == 0) {
				struct exfat_dentry *d;
				int i;

				filter->out.dentry_set = calloc(dentry_count,
								sizeof(struct exfat_dentry));
				if (!filter->out.dentry_set) {
					retval = -ENOMEM;
					goto out;
				}
				for (i = 0; i < dentry_count; i++) {
					exfat_de_iter_get(&de_iter, i, &d);
					memcpy(filter->out.dentry_set + i, d,
					       sizeof(struct exfat_dentry));
				}
				filter->out.dentry_count = dentry_count;
				goto out;
			} else if (retval < 0) {
				goto out;
			}
		} else if (IS_EXFAT_DELETED(dentry->type)) {
			if (empty_dentry_count == 0) {
				free_file_offset =
					exfat_de_iter_file_offset(&de_iter);
				free_dev_offset =
					exfat_de_iter_device_offset(&de_iter);
			}

			if (filter->in.dentry_count == 0 ||
			    empty_dentry_count < filter->in.dentry_count)
				empty_dentry_count++;
		}

		exfat_de_iter_advance(&de_iter, dentry_count);
	}

out:
	if (retval == 0) {
		filter->out.file_offset =
			exfat_de_iter_file_offset(&de_iter);
		filter->out.dev_offset =
			exfat_de_iter_device_offset(&de_iter);
	} else if (retval == EOF && empty_dentry_count) {
		filter->out.file_offset = free_file_offset;
		filter->out.dev_offset = free_dev_offset;
	} else {
		filter->out.file_offset = exfat_de_iter_file_offset(&de_iter);
		filter->out.dev_offset = EOF;
	}
	return retval;
}

static int filter_lookup_file(struct exfat_de_iter *de_iter,
			      void *param, int *dentry_count)
{
	struct exfat_dentry *file_de, *stream_de, *name_de;
	__le16 *name;
	int retval, name_len;
	int i;

	retval = exfat_de_iter_get(de_iter, 0, &file_de);
	if (retval || file_de->type != EXFAT_FILE)
		return 1;

	retval = exfat_de_iter_get(de_iter, 1, &stream_de);
	if (retval || stream_de->type != EXFAT_STREAM)
		return 1;

	name = (__le16 *)param;
	name_len = (int)exfat_utf16_len(name, PATH_MAX);

	if (file_de->dentry.file.num_ext <
		1 + (name_len + ENTRY_NAME_MAX - 1) / ENTRY_NAME_MAX)
		return 1;

	for (i = 2; i <= file_de->dentry.file.num_ext && name_len > 0; i++) {
		int len;

		retval = exfat_de_iter_get(de_iter, i, &name_de);
		if (retval || name_de->type != EXFAT_NAME)
			return 1;

		len = MIN(name_len + 1, ENTRY_NAME_MAX);
		if (memcmp(name_de->dentry.name.unicode_0_14,
			   name, len * 2) != 0)
			return 1;

		name += len;
		name_len -= len;
	}

	*dentry_count = i;
	return 0;
}

int exfat_lookup_file_by_utf16name(struct exfat *exfat,
				 struct exfat_inode *parent,
				 __le16 *utf16_name,
				 struct exfat_lookup_filter *filter_out)
{
	int retval;

	filter_out->in.type = EXFAT_FILE;
	filter_out->in.filter = filter_lookup_file;
	filter_out->in.param = utf16_name;
	filter_out->in.dentry_count = 0;

	retval = exfat_lookup_dentry_set(exfat, parent, filter_out);
	if (retval < 0)
		return retval;

	return 0;
}

int exfat_lookup_file(struct exfat *exfat, struct exfat_inode *parent,
		      const char *name, struct exfat_lookup_filter *filter_out)
{
	int retval;
	__le16 utf16_name[PATH_MAX + 2] = {0, };

	retval = (int)exfat_utf16_enc(name, utf16_name, sizeof(utf16_name));
	if (retval < 0)
		return retval;

	return exfat_lookup_file_by_utf16name(exfat, parent, utf16_name,
			filter_out);
}

void exfat_calc_dentry_checksum(struct exfat_dentry *dentry,
				uint16_t *checksum, bool primary)
{
	unsigned int i;
	uint8_t *bytes;

	bytes = (uint8_t *)dentry;

	/* use += to avoid promotion to int; UBSan complaints about signed overflow */
	*checksum = (*checksum << 15) | (*checksum >> 1);
	*checksum += bytes[0];
	*checksum = (*checksum << 15) | (*checksum >> 1);
	*checksum += bytes[1];

	i = primary ? 4 : 2;
	for (; i < sizeof(*dentry); i++) {
		*checksum = (*checksum << 15) | (*checksum >> 1);
		*checksum += bytes[i];
	}
}

static uint16_t calc_dentry_set_checksum(struct exfat_dentry *dset, int dcount)
{
	uint16_t checksum;
	int i;

	if (dcount < MIN_FILE_DENTRIES)
		return 0;

	checksum = 0;
	exfat_calc_dentry_checksum(&dset[0], &checksum, true);
	for (i = 1; i < dcount; i++)
		exfat_calc_dentry_checksum(&dset[i], &checksum, false);
	return checksum;
}

uint16_t exfat_calc_name_hash(struct exfat *exfat,
			      __le16 *name, int len)
{
	int i;
	__le16 ch;
	uint16_t chksum = 0;

	for (i = 0; i < len; i++) {
		ch = exfat->upcase_table[le16_to_cpu(name[i])];
		ch = cpu_to_le16(ch);

		/* use += to avoid promotion to int; UBSan complaints about signed overflow */
		chksum = (chksum << 15) | (chksum >> 1);
		chksum += ch & 0xFF;
		chksum = (chksum << 15) | (chksum >> 1);
		chksum += ch >> 8;
	}
	return chksum;
}

static void unix_time_to_exfat_time(time_t unix_time, __u8 *tz, __le16 *date,
				    __le16 *time, __u8 *time_ms)
{
	struct tm tm;
	__u16 t, d;

	gmtime_r(&unix_time, &tm);
	d = ((tm.tm_year - 80) << 9) | ((tm.tm_mon + 1) << 5) | tm.tm_mday;
	t = (tm.tm_hour << 11) | (tm.tm_min << 5) | (tm.tm_sec >> 1);

	*tz = 0x80;
	*date = cpu_to_le16(d);
	*time = cpu_to_le16(t);
	if (time_ms)
		*time_ms = (tm.tm_sec & 1) * 100;
}

int exfat_build_file_dentry_set(struct exfat *exfat, const char *name,
				unsigned short attr, struct exfat_dentry **dentry_set,
				int *dentry_count)
{
	struct exfat_dentry *dset;
	__le16 utf16_name[PATH_MAX + 2];
	int retval;
	int dcount, name_len, i;
	__le16 e_date, e_time;
	__u8 tz, e_time_ms;

	memset(utf16_name, 0, sizeof(utf16_name));
	retval = exfat_utf16_enc(name, utf16_name, sizeof(utf16_name));
	if (retval < 0)
		return retval;

	name_len = retval / 2;
	dcount = 2 + DIV_ROUND_UP(name_len, ENTRY_NAME_MAX);
	dset = calloc(dcount, DENTRY_SIZE);
	if (!dset)
		return -ENOMEM;

	dset[0].type = EXFAT_FILE;
	dset[0].dentry.file.num_ext = dcount - 1;
	dset[0].dentry.file.attr = cpu_to_le16(attr);

	unix_time_to_exfat_time(time(NULL), &tz,
				&e_date, &e_time, &e_time_ms);

	dset[0].dentry.file.create_date = e_date;
	dset[0].dentry.file.create_time = e_time;
	dset[0].dentry.file.create_time_ms = e_time_ms;
	dset[0].dentry.file.create_tz = tz;

	dset[0].dentry.file.modify_date = e_date;
	dset[0].dentry.file.modify_time = e_time;
	dset[0].dentry.file.modify_time_ms = e_time_ms;
	dset[0].dentry.file.modify_tz = tz;

	dset[0].dentry.file.access_date = e_date;
	dset[0].dentry.file.access_time = e_time;
	dset[0].dentry.file.access_tz = tz;

	dset[1].type = EXFAT_STREAM;
	dset[1].dentry.stream.flags = 0x01;
	dset[1].dentry.stream.name_len = (__u8)name_len;
	dset[1].dentry.stream.name_hash =
		cpu_to_le16(exfat_calc_name_hash(exfat, utf16_name, name_len));

	for (i = 2; i < dcount; i++) {
		dset[i].type = EXFAT_NAME;
		memcpy(dset[i].dentry.name.unicode_0_14,
		       utf16_name + (i - 2) * ENTRY_NAME_MAX,
		       ENTRY_NAME_MAX * 2);
	}

	dset[0].dentry.file.checksum =
		cpu_to_le16(calc_dentry_set_checksum(dset, dcount));

	*dentry_set = dset;
	*dentry_count = dcount;
	return 0;
}

int exfat_update_file_dentry_set(struct exfat *exfat,
				 struct exfat_dentry *dset, int dcount,
				 const char *name,
				 clus_t start_clu, clus_t ccount)
{
	int i, name_len;
	__le16 utf16_name[PATH_MAX + 2];

	if (dset[0].type != EXFAT_FILE || dcount < MIN_FILE_DENTRIES)
		return -EINVAL;

	if (name) {
		name_len = (int)exfat_utf16_enc(name,
						utf16_name, sizeof(utf16_name));
		if (name_len < 0)
			return name_len;

		name_len /= 2;
		if (dcount != 2 + DIV_ROUND_UP(name_len, ENTRY_NAME_MAX))
			return -EINVAL;

		dset[1].dentry.stream.name_len = (__u8)name_len;
		dset[1].dentry.stream.name_hash =
			exfat_calc_name_hash(exfat, utf16_name, name_len);

		for (i = 2; i < dcount; i++) {
			dset[i].type = EXFAT_NAME;
			memcpy(dset[i].dentry.name.unicode_0_14,
			       utf16_name + (i - 2) * ENTRY_NAME_MAX,
			       ENTRY_NAME_MAX * 2);
		}
	}

	dset[1].dentry.stream.valid_size = cpu_to_le64(ccount * exfat->clus_size);
	dset[1].dentry.stream.size = cpu_to_le64(ccount * exfat->clus_size);
	if (start_clu)
		dset[1].dentry.stream.start_clu = cpu_to_le32(start_clu);

	dset[0].dentry.file.checksum =
		cpu_to_le16(calc_dentry_set_checksum(dset, dcount));
	return 0;
}

static int find_free_cluster(struct exfat *exfat,
			     clus_t start, clus_t *new_clu)
{
	clus_t end = le32_to_cpu(exfat->bs->bsx.clu_count) +
		EXFAT_FIRST_CLUSTER;

	if (!exfat_heap_clus(exfat, start))
		return -EINVAL;

	while (start < end) {
		if (exfat_bitmap_find_zero(exfat, exfat->alloc_bitmap,
					   start, new_clu))
			break;
		if (!exfat_bitmap_get(exfat->disk_bitmap, *new_clu))
			return 0;
		start = *new_clu + 1;
	}

	end = start;
	start = EXFAT_FIRST_CLUSTER;
	while (start < end) {
		if (exfat_bitmap_find_zero(exfat, exfat->alloc_bitmap,
					   start, new_clu))
			goto out_nospc;
		if (!exfat_bitmap_get(exfat->disk_bitmap, *new_clu))
			return 0;
		start = *new_clu + 1;
	}

out_nospc:
	*new_clu = EXFAT_EOF_CLUSTER;
	return -ENOSPC;
}

static int exfat_map_cluster(struct exfat *exfat, struct exfat_inode *inode,
			     off_t file_off, clus_t *mapped_clu)
{
	clus_t clu, next, count, last_count;

	if (!exfat_heap_clus(exfat, inode->first_clus))
		return -EINVAL;

	clu = inode->first_clus;
	next = EXFAT_EOF_CLUSTER;
	count = 1;
	if (file_off == EOF)
		last_count = DIV_ROUND_UP(inode->size, exfat->clus_size);
	else
		last_count = file_off / exfat->clus_size + 1;

	while (true) {
		if (count * exfat->clus_size > inode->size)
			return -EINVAL;

		if (count == last_count) {
			*mapped_clu = clu;
			return 0;
		}

		if (exfat_get_inode_next_clus(exfat, inode, clu, &next))
			return -EINVAL;

		if (!exfat_heap_clus(exfat, clu))
			return -EINVAL;

		clu = next;
		count++;
	}
	return -EINVAL;
}

static int exfat_write_dentry_set(struct exfat *exfat,
				  struct exfat_dentry *dset, int dcount,
				  off_t dev_off, off_t *next_dev_off)
{
	clus_t clus;
	unsigned int clus_off, dent_len, first_half_len, sec_half_len;
	off_t first_half_off, sec_half_off = 0;

	if (exfat_o2c(exfat, dev_off, &clus, &clus_off))
		return -ERANGE;

	dent_len = dcount * DENTRY_SIZE;
	first_half_len = MIN(dent_len, exfat->clus_size - clus_off);
	sec_half_len = dent_len - first_half_len;

	first_half_off = dev_off;
	if (sec_half_len) {
		clus_t next_clus;

		if (exfat_get_next_clus(exfat, clus, &next_clus))
			return -EIO;
		if (!exfat_heap_clus(exfat, next_clus))
			return -EINVAL;
		sec_half_off = exfat_c2o(exfat, next_clus);
	}

	if (exfat_write(exfat->blk_dev->dev_fd, dset, first_half_len,
			first_half_off) != (ssize_t)first_half_len)
		return -EIO;

	if (sec_half_len) {
		dset = (struct exfat_dentry *)((char *)dset + first_half_len);
		if (exfat_write(exfat->blk_dev->dev_fd, dset, sec_half_len,
				sec_half_off) != (ssize_t)sec_half_len)
			return -EIO;
	}

	if (next_dev_off) {
		if (sec_half_len)
			*next_dev_off = sec_half_off + sec_half_len;
		else
			*next_dev_off = first_half_off + first_half_len;
	}
	return 0;
}

static int exfat_alloc_cluster(struct exfat *exfat, struct exfat_inode *inode,
			       clus_t *new_clu)
{
	clus_t last_clu;
	int err;
	bool need_dset = inode != exfat->root;

	if ((need_dset && !inode->dentry_set) || inode->is_contiguous)
		return -EINVAL;

	err = find_free_cluster(exfat, exfat->start_clu, new_clu);
	if (err) {
		exfat->start_clu = EXFAT_FIRST_CLUSTER;
		exfat_err("failed to find an free cluster\n");
		return -ENOSPC;
	}
	exfat->start_clu = *new_clu;

	if (exfat_set_fat(exfat, *new_clu, EXFAT_EOF_CLUSTER))
		return -EIO;

	/* zero out the new cluster */
	if (exfat_write_zero(exfat->blk_dev->dev_fd, exfat->clus_size,
				exfat_c2o(exfat, *new_clu))) {
		exfat_err("failed to fill new cluster with zeroes\n");
		return -EIO;
	}

	if (inode->size) {
		err = exfat_map_cluster(exfat, inode, EOF, &last_clu);
		if (err) {
			exfat_err("failed to get the last cluster\n");
			return err;
		}

		if (exfat_set_fat(exfat, last_clu, *new_clu))
			return -EIO;

		if (need_dset) {
			err = exfat_update_file_dentry_set(exfat,
							   inode->dentry_set,
							   inode->dentry_count,
							   NULL, 0,
							   DIV_ROUND_UP(inode->size,
									exfat->clus_size) + 1);
			if (err)
				return -EINVAL;
		}
	} else {
		if (need_dset) {
			err = exfat_update_file_dentry_set(exfat,
							   inode->dentry_set,
							   inode->dentry_count,
							   NULL, *new_clu, 1);
			if (err)
				return -EINVAL;
		}
	}

	if (need_dset && exfat_write_dentry_set(exfat, inode->dentry_set,
						inode->dentry_count,
						inode->dev_offset, NULL))
		return -EIO;

	exfat_bitmap_set(exfat->alloc_bitmap, *new_clu);
	if (inode->size == 0)
		inode->first_clus = *new_clu;
	inode->size += exfat->clus_size;
	return 0;
}

int exfat_add_dentry_set(struct exfat *exfat, struct exfat_dentry_loc *loc,
			 struct exfat_dentry *dset, int dcount,
			 bool need_next_loc)
{
	struct exfat_inode *parent = loc->parent;
	off_t dev_off, next_dev_off;

	if (parent->is_contiguous ||
	    (uint64_t)loc->file_offset > parent->size ||
	    (unsigned int)dcount * DENTRY_SIZE > exfat->clus_size)
		return -EINVAL;

	dev_off = loc->dev_offset;
	if ((uint64_t)loc->file_offset + dcount * DENTRY_SIZE > parent->size) {
		clus_t new_clus;

		if (exfat_alloc_cluster(exfat, parent, &new_clus))
			return -EIO;
		if ((uint64_t)loc->file_offset == parent->size - exfat->clus_size)
			dev_off = exfat_c2o(exfat, new_clus);
	}

	if (exfat_write_dentry_set(exfat, dset, dcount, dev_off, &next_dev_off))
		return -EIO;

	if (need_next_loc) {
		loc->file_offset += dcount * DENTRY_SIZE;
		loc->dev_offset = next_dev_off;
	}
	return 0;
}

int exfat_create_file(struct exfat *exfat, struct exfat_inode *parent,
		      const char *name, unsigned short attr)
{
	struct exfat_dentry *dset;
	int err, dcount;
	struct exfat_lookup_filter filter;
	struct exfat_dentry_loc loc;

	err = exfat_lookup_file(exfat, parent, name, &filter);
	if (err == 0) {
		dset = filter.out.dentry_set;
		dcount = filter.out.dentry_count;
		if ((le16_to_cpu(dset->dentry.file.attr) & attr) != attr)
			err = -EEXIST;
		goto out;
	}

	err = exfat_build_file_dentry_set(exfat, name, attr,
					  &dset, &dcount);
	if (err)
		return err;

	loc.parent = parent;
	loc.file_offset = filter.out.file_offset;
	loc.dev_offset = filter.out.dev_offset;
	err = exfat_add_dentry_set(exfat, &loc, dset, dcount, false);
out:
	free(dset);
	return err;
}
