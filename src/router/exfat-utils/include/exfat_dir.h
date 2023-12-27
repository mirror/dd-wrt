/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2021 LG Electronics.
 *
 *   Author(s): Hyunchul Lee <hyc.lee@gmail.com>
 */

#ifndef _DIR_H_
#define _DIR_H_

struct exfat;
struct exfat_inode;
struct exfat_dentry_loc;
struct buffer_desc;

struct exfat_de_iter {
	struct exfat		*exfat;
	struct exfat_inode	*parent;
	struct buffer_desc	*buffer_desc;		/* cluster * 2 */
	__u32			ra_next_clus;
	unsigned int		ra_begin_offset;
	unsigned int		ra_partial_size;
	unsigned int		read_size;		/* cluster size */
	unsigned int		write_size;		/* sector size */
	off_t			de_file_offset;
	off_t			next_read_offset;
	int			max_skip_dentries;
#define INVALID_NAME_NUM_MAX	9999999
	unsigned int		invalid_name_num;

	char *name_hash_bitmap;		/* bitmap of children's name hashes */
};

struct exfat_lookup_filter {
	struct {
		uint8_t		type;
		int		dentry_count;
		/* return 0 if matched, return 1 if not matched,
		 * otherwise return errno
		 */
		int		(*filter)(struct exfat_de_iter *iter,
					  void *param, int *dentry_count);
		void		*param;
	} in;
	struct {
		struct exfat_dentry	*dentry_set;
		int			dentry_count;
		off_t			file_offset;
		/*
		 * If the dentry_set found:
		 *   - device offset where the dentry_set locates.
		 * If the dentry_set not found:
		 *   - device offset where the first empty dentry_set locates
		 *     if in.dentry_count > 0 and there are enough empty dentry.
		 *   - device offset where the last empty dentry_set locates
		 *     if in.dentry_count = 0 or no enough empty dentry.
		 *   - EOF if no empty dentry_set.
		 */
		off_t			dev_offset;
	} out;
};

int exfat_de_iter_init(struct exfat_de_iter *iter, struct exfat *exfat,
		       struct exfat_inode *dir, struct buffer_desc *bd);
int exfat_de_iter_get(struct exfat_de_iter *iter,
		      int ith, struct exfat_dentry **dentry);
int exfat_de_iter_get_dirty(struct exfat_de_iter *iter,
			    int ith, struct exfat_dentry **dentry);
int exfat_de_iter_flush(struct exfat_de_iter *iter);
int exfat_de_iter_advance(struct exfat_de_iter *iter, int skip_dentries);
off_t exfat_de_iter_device_offset(struct exfat_de_iter *iter);
off_t exfat_de_iter_file_offset(struct exfat_de_iter *iter);

int exfat_lookup_dentry_set(struct exfat *exfat, struct exfat_inode *parent,
			    struct exfat_lookup_filter *filter);
int exfat_lookup_file(struct exfat *exfat, struct exfat_inode *parent,
		      const char *name, struct exfat_lookup_filter *filter_out);
int exfat_lookup_file_by_utf16name(struct exfat *exfat,
				 struct exfat_inode *parent,
				 __le16 *utf16_name,
				 struct exfat_lookup_filter *filter_out);

int exfat_create_file(struct exfat *exfat, struct exfat_inode *parent,
		      const char *name, unsigned short attr);
int exfat_update_file_dentry_set(struct exfat *exfat,
				 struct exfat_dentry *dset, int dcount,
				 const char *name,
				 clus_t start_clu, clus_t ccount);
int exfat_build_file_dentry_set(struct exfat *exfat, const char *name,
				unsigned short attr, struct exfat_dentry **dentry_set,
				int *dentry_count);
int exfat_add_dentry_set(struct exfat *exfat, struct exfat_dentry_loc *loc,
			 struct exfat_dentry *dset, int dcount,
			 bool need_next_loc);
void exfat_calc_dentry_checksum(struct exfat_dentry *dentry,
				uint16_t *checksum, bool primary);
uint16_t exfat_calc_name_hash(struct exfat *exfat,
			      __le16 *name, int len);

#endif
