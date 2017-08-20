/*
 * ext_attr.c --- extended attribute blocks
 *
 * Copyright (C) 2001 Andreas Gruenbacher, <a.gruenbacher@computer.org>
 *
 * Copyright (C) 2002 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */

#include "config.h"
#include <stdio.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>

#include "ext2_fs.h"
#include "ext2_ext_attr.h"
#include "ext4_acl.h"

#include "ext2fs.h"

#define NAME_HASH_SHIFT 5
#define VALUE_HASH_SHIFT 16

/*
 * ext2_xattr_hash_entry()
 *
 * Compute the hash of an extended attribute.
 */
__u32 ext2fs_ext_attr_hash_entry(struct ext2_ext_attr_entry *entry, void *data)
{
	__u32 hash = 0;
	char *name = ((char *) entry) + sizeof(struct ext2_ext_attr_entry);
	int n;

	for (n = 0; n < entry->e_name_len; n++) {
		hash = (hash << NAME_HASH_SHIFT) ^
		       (hash >> (8*sizeof(hash) - NAME_HASH_SHIFT)) ^
		       *name++;
	}

	/* The hash needs to be calculated on the data in little-endian. */
	if (entry->e_value_block == 0 && entry->e_value_size != 0) {
		__u32 *value = (__u32 *)data;
		for (n = (entry->e_value_size + EXT2_EXT_ATTR_ROUND) >>
			 EXT2_EXT_ATTR_PAD_BITS; n; n--) {
			hash = (hash << VALUE_HASH_SHIFT) ^
			       (hash >> (8*sizeof(hash) - VALUE_HASH_SHIFT)) ^
			       ext2fs_le32_to_cpu(*value++);
		}
	}

	return hash;
}

static errcode_t check_ext_attr_header(struct ext2_ext_attr_header *header)
{
	if ((header->h_magic != EXT2_EXT_ATTR_MAGIC_v1 &&
	     header->h_magic != EXT2_EXT_ATTR_MAGIC) ||
	    header->h_blocks != 1)
		return EXT2_ET_BAD_EA_HEADER;

	return 0;
}

#undef NAME_HASH_SHIFT
#undef VALUE_HASH_SHIFT

errcode_t ext2fs_read_ext_attr3(ext2_filsys fs, blk64_t block, void *buf,
				ext2_ino_t inum)
{
	int		csum_failed = 0;
	errcode_t	retval;

	retval = io_channel_read_blk64(fs->io, block, 1, buf);
	if (retval)
		return retval;

	if (!(fs->flags & EXT2_FLAG_IGNORE_CSUM_ERRORS) &&
	    !ext2fs_ext_attr_block_csum_verify(fs, inum, block, buf))
		csum_failed = 1;

#ifdef WORDS_BIGENDIAN
	ext2fs_swap_ext_attr(buf, buf, fs->blocksize, 1);
#endif

	retval = check_ext_attr_header(buf);
	if (retval == 0 && csum_failed)
		retval = EXT2_ET_EXT_ATTR_CSUM_INVALID;

	return retval;
}

errcode_t ext2fs_read_ext_attr2(ext2_filsys fs, blk64_t block, void *buf)
{
	return ext2fs_read_ext_attr3(fs, block, buf, 0);
}

errcode_t ext2fs_read_ext_attr(ext2_filsys fs, blk_t block, void *buf)
{
	return ext2fs_read_ext_attr2(fs, block, buf);
}

errcode_t ext2fs_write_ext_attr3(ext2_filsys fs, blk64_t block, void *inbuf,
				 ext2_ino_t inum)
{
	errcode_t	retval;
	char		*write_buf;

#ifdef WORDS_BIGENDIAN
	retval = ext2fs_get_mem(fs->blocksize, &write_buf);
	if (retval)
		return retval;
	ext2fs_swap_ext_attr(write_buf, inbuf, fs->blocksize, 1);
#else
	write_buf = (char *) inbuf;
#endif

	retval = ext2fs_ext_attr_block_csum_set(fs, inum, block,
			(struct ext2_ext_attr_header *)write_buf);
	if (retval)
		return retval;

	retval = io_channel_write_blk64(fs->io, block, 1, write_buf);
#ifdef WORDS_BIGENDIAN
	ext2fs_free_mem(&write_buf);
#endif
	if (!retval)
		ext2fs_mark_changed(fs);
	return retval;
}

errcode_t ext2fs_write_ext_attr2(ext2_filsys fs, blk64_t block, void *inbuf)
{
	return ext2fs_write_ext_attr3(fs, block, inbuf, 0);
}

errcode_t ext2fs_write_ext_attr(ext2_filsys fs, blk_t block, void *inbuf)
{
	return ext2fs_write_ext_attr2(fs, block, inbuf);
}

/*
 * This function adjusts the reference count of the EA block.
 */
errcode_t ext2fs_adjust_ea_refcount3(ext2_filsys fs, blk64_t blk,
				    char *block_buf, int adjust,
				    __u32 *newcount, ext2_ino_t inum)
{
	errcode_t	retval;
	struct ext2_ext_attr_header *header;
	char	*buf = 0;

	if ((blk >= ext2fs_blocks_count(fs->super)) ||
	    (blk < fs->super->s_first_data_block))
		return EXT2_ET_BAD_EA_BLOCK_NUM;

	if (!block_buf) {
		retval = ext2fs_get_mem(fs->blocksize, &buf);
		if (retval)
			return retval;
		block_buf = buf;
	}

	retval = ext2fs_read_ext_attr3(fs, blk, block_buf, inum);
	if (retval)
		goto errout;

	header = (struct ext2_ext_attr_header *) block_buf;
	header->h_refcount += adjust;
	if (newcount)
		*newcount = header->h_refcount;

	retval = ext2fs_write_ext_attr3(fs, blk, block_buf, inum);
	if (retval)
		goto errout;

errout:
	if (buf)
		ext2fs_free_mem(&buf);
	return retval;
}

errcode_t ext2fs_adjust_ea_refcount2(ext2_filsys fs, blk64_t blk,
				    char *block_buf, int adjust,
				    __u32 *newcount)
{
	return ext2fs_adjust_ea_refcount3(fs, blk, block_buf, adjust,
					  newcount, 0);
}

errcode_t ext2fs_adjust_ea_refcount(ext2_filsys fs, blk_t blk,
					char *block_buf, int adjust,
					__u32 *newcount)
{
	return ext2fs_adjust_ea_refcount2(fs, blk, block_buf, adjust,
					  newcount);
}

/* Manipulate the contents of extended attribute regions */
struct ext2_xattr {
	char *name;
	void *value;
	size_t value_len;
};

struct ext2_xattr_handle {
	errcode_t magic;
	ext2_filsys fs;
	struct ext2_xattr *attrs;
	size_t length, count;
	ext2_ino_t ino;
	unsigned int flags;
	int dirty;
};

static errcode_t ext2fs_xattrs_expand(struct ext2_xattr_handle *h,
				      unsigned int expandby)
{
	struct ext2_xattr *new_attrs;
	errcode_t err;

	err = ext2fs_get_arrayzero(h->length + expandby,
				   sizeof(struct ext2_xattr), &new_attrs);
	if (err)
		return err;

	memcpy(new_attrs, h->attrs, h->length * sizeof(struct ext2_xattr));
	ext2fs_free_mem(&h->attrs);
	h->length += expandby;
	h->attrs = new_attrs;

	return 0;
}

struct ea_name_index {
	int index;
	const char *name;
};

/* Keep these names sorted in order of decreasing specificity. */
static struct ea_name_index ea_names[] = {
	{3, "system.posix_acl_default"},
	{2, "system.posix_acl_access"},
	{8, "system.richacl"},
	{6, "security."},
	{4, "trusted."},
	{7, "system."},
	{1, "user."},
	{0, NULL},
};

static int find_ea_index(char *fullname, char **name, int *index);

/* Push empty attributes to the end and inlinedata to the front. */
static int attr_compare(const void *a, const void *b)
{
	const struct ext2_xattr *xa = a, *xb = b;
	char *xa_suffix, *xb_suffix;
	int xa_idx, xb_idx;
	int cmp;

	if (xa->name == NULL)
		return +1;
	else if (xb->name == NULL)
		return -1;
	else if (!strcmp(xa->name, "system.data"))
		return -1;
	else if (!strcmp(xb->name, "system.data"))
		return +1;

	/*
	 * Duplicate the kernel's sorting algorithm because xattr blocks
	 * require sorted keys.
	 */
	xa_suffix = xa->name;
	xb_suffix = xb->name;
	xa_idx = xb_idx = 0;
	find_ea_index(xa->name, &xa_suffix, &xa_idx);
	find_ea_index(xb->name, &xb_suffix, &xb_idx);
	cmp = xa_idx - xb_idx;
	if (cmp)
		return cmp;
	cmp = strlen(xa_suffix) - strlen(xb_suffix);
	if (cmp)
		return cmp;
	cmp = strcmp(xa_suffix, xb_suffix);
	return cmp;
}

static const char *find_ea_prefix(int index)
{
	struct ea_name_index *e;

	for (e = ea_names; e->name; e++)
		if (e->index == index)
			return e->name;

	return NULL;
}

static int find_ea_index(char *fullname, char **name, int *index)
{
	struct ea_name_index *e;

	for (e = ea_names; e->name; e++) {
		if (strncmp(fullname, e->name, strlen(e->name)) == 0) {
			*name = (char *)fullname + strlen(e->name);
			*index = e->index;
			return 1;
		}
	}
	return 0;
}

errcode_t ext2fs_free_ext_attr(ext2_filsys fs, ext2_ino_t ino,
			       struct ext2_inode_large *inode)
{
	struct ext2_ext_attr_header *header;
	void *block_buf = NULL;
	blk64_t blk;
	errcode_t err;
	struct ext2_inode_large i;

	/* Read inode? */
	if (inode == NULL) {
		err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&i,
					     sizeof(struct ext2_inode_large));
		if (err)
			return err;
		inode = &i;
	}

	/* Do we already have an EA block? */
	blk = ext2fs_file_acl_block(fs, (struct ext2_inode *)inode);
	if (blk == 0)
		return 0;

	/* Find block, zero it, write back */
	if ((blk < fs->super->s_first_data_block) ||
	    (blk >= ext2fs_blocks_count(fs->super))) {
		err = EXT2_ET_BAD_EA_BLOCK_NUM;
		goto out;
	}

	err = ext2fs_get_mem(fs->blocksize, &block_buf);
	if (err)
		goto out;

	err = ext2fs_read_ext_attr3(fs, blk, block_buf, ino);
	if (err)
		goto out2;

	/* We only know how to deal with v2 EA blocks */
	header = (struct ext2_ext_attr_header *) block_buf;
	if (header->h_magic != EXT2_EXT_ATTR_MAGIC) {
		err = EXT2_ET_BAD_EA_HEADER;
		goto out2;
	}

	header->h_refcount--;
	err = ext2fs_write_ext_attr3(fs, blk, block_buf, ino);
	if (err)
		goto out2;

	/* Erase link to block */
	ext2fs_file_acl_block_set(fs, (struct ext2_inode *)inode, 0);
	if (header->h_refcount == 0)
		ext2fs_block_alloc_stats2(fs, blk, -1);
	err = ext2fs_iblk_sub_blocks(fs, (struct ext2_inode *)inode, 1);
	if (err)
		goto out2;

	/* Write inode? */
	if (inode == &i) {
		err = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&i,
					      sizeof(struct ext2_inode_large));
		if (err)
			goto out2;
	}

out2:
	ext2fs_free_mem(&block_buf);
out:
	return err;
}

static errcode_t prep_ea_block_for_write(ext2_filsys fs, ext2_ino_t ino,
					 struct ext2_inode_large *inode)
{
	struct ext2_ext_attr_header *header;
	void *block_buf = NULL;
	blk64_t blk, goal;
	errcode_t err;

	/* Do we already have an EA block? */
	blk = ext2fs_file_acl_block(fs, (struct ext2_inode *)inode);
	if (blk != 0) {
		if ((blk < fs->super->s_first_data_block) ||
		    (blk >= ext2fs_blocks_count(fs->super))) {
			err = EXT2_ET_BAD_EA_BLOCK_NUM;
			goto out;
		}

		err = ext2fs_get_mem(fs->blocksize, &block_buf);
		if (err)
			goto out;

		err = ext2fs_read_ext_attr3(fs, blk, block_buf, ino);
		if (err)
			goto out2;

		/* We only know how to deal with v2 EA blocks */
		header = (struct ext2_ext_attr_header *) block_buf;
		if (header->h_magic != EXT2_EXT_ATTR_MAGIC) {
			err = EXT2_ET_BAD_EA_HEADER;
			goto out2;
		}

		/* Single-user block.  We're done here. */
		if (header->h_refcount == 1)
			goto out2;

		/* We need to CoW the block. */
		header->h_refcount--;
		err = ext2fs_write_ext_attr3(fs, blk, block_buf, ino);
		if (err)
			goto out2;
	} else {
		/* No block, we must increment i_blocks */
		err = ext2fs_iblk_add_blocks(fs, (struct ext2_inode *)inode,
					     1);
		if (err)
			goto out;
	}

	/* Allocate a block */
	goal = ext2fs_find_inode_goal(fs, ino, (struct ext2_inode *)inode, 0);
	err = ext2fs_alloc_block2(fs, goal, NULL, &blk);
	if (err)
		goto out2;
	ext2fs_file_acl_block_set(fs, (struct ext2_inode *)inode, blk);
out2:
	if (block_buf)
		ext2fs_free_mem(&block_buf);
out:
	return err;
}


static inline int
posix_acl_xattr_count(size_t size)
{
        if (size < sizeof(posix_acl_xattr_header))
                return -1;
        size -= sizeof(posix_acl_xattr_header);
        if (size % sizeof(posix_acl_xattr_entry))
                return -1;
        return size / sizeof(posix_acl_xattr_entry);
}

/*
 * The lgetxattr function returns data formatted in the POSIX extended
 * attribute format.  The on-disk format uses a more compact encoding.
 * See the ext4_acl_to_disk in fs/ext4/acl.c.
 */
static errcode_t convert_posix_acl_to_disk_buffer(const void *value, size_t size,
						  void *out_buf, size_t *size_out)
{
	posix_acl_xattr_header *header = (posix_acl_xattr_header*) value;
	posix_acl_xattr_entry *entry = (posix_acl_xattr_entry *)(header+1), *end;
	ext4_acl_header *ext_acl;
	size_t s;
	void *e;

	int count;

	if (!value)
		return EINVAL;
	if (size < sizeof(posix_acl_xattr_header))
		return ENOMEM;
	if (header->a_version != ext2fs_cpu_to_le32(POSIX_ACL_XATTR_VERSION))
		return EINVAL;

	count = posix_acl_xattr_count(size);
	ext_acl = out_buf;
	ext_acl->a_version = ext2fs_cpu_to_le32(EXT4_ACL_VERSION);

	if (count <= 0)
		return EINVAL;

	e = (char *) out_buf + sizeof(ext4_acl_header);
	s = sizeof(ext4_acl_header);
	for (end = entry + count; entry != end;entry++) {
		ext4_acl_entry *disk_entry = (ext4_acl_entry*) e;
		disk_entry->e_tag = ext2fs_cpu_to_le16(entry->e_tag);
		disk_entry->e_perm = ext2fs_cpu_to_le16(entry->e_perm);

		switch(entry->e_tag) {
			case ACL_USER_OBJ:
			case ACL_GROUP_OBJ:
			case ACL_MASK:
			case ACL_OTHER:
				e += sizeof(ext4_acl_entry_short);
				s += sizeof(ext4_acl_entry_short);
				break;
			case ACL_USER:
			case ACL_GROUP:
				disk_entry->e_id =  ext2fs_cpu_to_le32(entry->e_id);
				e += sizeof(ext4_acl_entry);
				s += sizeof(ext4_acl_entry);
				break;
		}
	}
	*size_out = s;
	return 0;
}

static errcode_t convert_disk_buffer_to_posix_acl(const void *value, size_t size,
						  void **out_buf, size_t *size_out)
{
	posix_acl_xattr_header *header;
	posix_acl_xattr_entry *entry;
	ext4_acl_header *ext_acl = (ext4_acl_header *) value;
	errcode_t err;
	const char *cp;
	char *out;

	if ((!value) ||
	    (size < sizeof(ext4_acl_header)) ||
	    (ext_acl->a_version != ext2fs_cpu_to_le32(EXT4_ACL_VERSION)))
		return EINVAL;

	err = ext2fs_get_mem(size * 2, &out);
	if (err)
		return err;

	header = (posix_acl_xattr_header *) out;
	header->a_version = ext2fs_cpu_to_le32(POSIX_ACL_XATTR_VERSION);
	entry = (posix_acl_xattr_entry *) (out + sizeof(posix_acl_xattr_header));

	cp = value + sizeof(ext4_acl_header);
	size -= sizeof(ext4_acl_header);

	while (size > 0) {
		const ext4_acl_entry *disk_entry = (const ext4_acl_entry *) cp;

		entry->e_tag = ext2fs_le16_to_cpu(disk_entry->e_tag);
		entry->e_perm = ext2fs_le16_to_cpu(disk_entry->e_perm);

		switch(entry->e_tag) {
			case ACL_USER_OBJ:
			case ACL_GROUP_OBJ:
			case ACL_MASK:
			case ACL_OTHER:
				entry->e_id = 0;
				cp += sizeof(ext4_acl_entry_short);
				size -= sizeof(ext4_acl_entry_short);
				break;
			case ACL_USER:
			case ACL_GROUP:
				entry->e_id = ext2fs_le32_to_cpu(disk_entry->e_id);
				cp += sizeof(ext4_acl_entry);
				size -= sizeof(ext4_acl_entry);
				break;
		default:
			ext2fs_free_mem(&out);
			return EINVAL;
			break;
		}
		entry++;
	}
	*out_buf = out;
	*size_out = ((char *) entry - out);
	return 0;
}


static errcode_t write_xattrs_to_buffer(struct ext2_xattr_handle *handle,
					struct ext2_xattr **pos,
					void *entries_start,
					unsigned int storage_size,
					unsigned int value_offset_correction,
					int write_hash)
{
	struct ext2_xattr *x = *pos;
	struct ext2_ext_attr_entry *e = entries_start;
	char *end = (char *) entries_start + storage_size;
	char *shortname;
	unsigned int entry_size, value_size;
	int idx, ret;

	memset(entries_start, 0, storage_size);
	/* For all remaining x...  */
	for (; x < handle->attrs + handle->length; x++) {
		if (!x->name)
			continue;

		/* Calculate index and shortname position */
		shortname = x->name;
		ret = find_ea_index(x->name, &shortname, &idx);

		/* Calculate entry and value size */
		entry_size = (sizeof(*e) + strlen(shortname) +
			      EXT2_EXT_ATTR_PAD - 1) &
			     ~(EXT2_EXT_ATTR_PAD - 1);
		value_size = ((x->value_len + EXT2_EXT_ATTR_PAD - 1) /
			      EXT2_EXT_ATTR_PAD) * EXT2_EXT_ATTR_PAD;

		/*
		 * Would entry collide with value?
		 * Note that we must leave sufficient room for a (u32)0 to
		 * mark the end of the entries.
		 */
		if ((char *)e + entry_size + sizeof(__u32) > end - value_size)
			break;

		/* Fill out e appropriately */
		e->e_name_len = strlen(shortname);
		e->e_name_index = (ret ? idx : 0);
		e->e_value_offs = end - value_size - (char *)entries_start +
				value_offset_correction;
		e->e_value_block = 0;
		e->e_value_size = x->value_len;

		/* Store name and value */
		end -= value_size;
		memcpy((char *)e + sizeof(*e), shortname, e->e_name_len);
		memcpy(end, x->value, e->e_value_size);

		if (write_hash)
			e->e_hash = ext2fs_ext_attr_hash_entry(e, end);
		else
			e->e_hash = 0;

		e = EXT2_EXT_ATTR_NEXT(e);
		*(__u32 *)e = 0;
	}
	*pos = x;

	return 0;
}

errcode_t ext2fs_xattrs_write(struct ext2_xattr_handle *handle)
{
	struct ext2_xattr *x;
	struct ext2_inode_large *inode;
	char *start, *block_buf = NULL;
	struct ext2_ext_attr_header *header;
	__u32 ea_inode_magic;
	blk64_t blk;
	unsigned int storage_size;
	unsigned int i;
	errcode_t err;

	EXT2_CHECK_MAGIC(handle, EXT2_ET_MAGIC_EA_HANDLE);
	i = EXT2_INODE_SIZE(handle->fs->super);
	if (i < sizeof(*inode))
		i = sizeof(*inode);
	err = ext2fs_get_memzero(i, &inode);
	if (err)
		return err;

	err = ext2fs_read_inode_full(handle->fs, handle->ino,
				     (struct ext2_inode *)inode,
				     EXT2_INODE_SIZE(handle->fs->super));
	if (err)
		goto out;

	/* If extra_isize isn't set, we need to set it now */
	if (inode->i_extra_isize == 0 &&
	    EXT2_INODE_SIZE(handle->fs->super) > EXT2_GOOD_OLD_INODE_SIZE) {
		char *p = (char *)inode;
		size_t extra = handle->fs->super->s_want_extra_isize;

		if (extra == 0)
			extra = sizeof(__u32);
		memset(p + EXT2_GOOD_OLD_INODE_SIZE, 0, extra);
		inode->i_extra_isize = extra;
	}
	if (inode->i_extra_isize & 3) {
		err = EXT2_ET_INODE_CORRUPTED;
		goto out;
	}

	/*
	 * Force the inlinedata attr to the front and the empty entries
	 * to the end.
	 */
	x = handle->attrs;
	qsort(x, handle->length, sizeof(struct ext2_xattr), attr_compare);

	/* Does the inode have space for EA? */
	if (inode->i_extra_isize < sizeof(inode->i_extra_isize) ||
	    EXT2_INODE_SIZE(handle->fs->super) <= EXT2_GOOD_OLD_INODE_SIZE +
						  inode->i_extra_isize +
						  sizeof(__u32))
		goto write_ea_block;

	/* Write the inode EA */
	ea_inode_magic = EXT2_EXT_ATTR_MAGIC;
	memcpy(((char *) inode) + EXT2_GOOD_OLD_INODE_SIZE +
	       inode->i_extra_isize, &ea_inode_magic, sizeof(__u32));
	storage_size = EXT2_INODE_SIZE(handle->fs->super) -
		EXT2_GOOD_OLD_INODE_SIZE - inode->i_extra_isize -
		sizeof(__u32);
	start = ((char *) inode) + EXT2_GOOD_OLD_INODE_SIZE +
		inode->i_extra_isize + sizeof(__u32);

	err = write_xattrs_to_buffer(handle, &x, start, storage_size, 0, 0);
	if (err)
		goto out;

write_ea_block:
	/* Are we done? */
	if (x >= handle->attrs + handle->count)
		goto skip_ea_block;

	/* Write the EA block */
	err = ext2fs_get_memzero(handle->fs->blocksize, &block_buf);
	if (err)
		goto out;

	storage_size = handle->fs->blocksize -
		sizeof(struct ext2_ext_attr_header);
	start = block_buf + sizeof(struct ext2_ext_attr_header);

	err = write_xattrs_to_buffer(handle, &x, start, storage_size,
				     start - block_buf, 1);
	if (err)
		goto out2;

	if (x < handle->attrs + handle->length) {
		err = EXT2_ET_EA_NO_SPACE;
		goto out2;
	}

	/* Write a header on the EA block */
	header = (struct ext2_ext_attr_header *) block_buf;
	header->h_magic = EXT2_EXT_ATTR_MAGIC;
	header->h_refcount = 1;
	header->h_blocks = 1;

	/* Get a new block for writing */
	err = prep_ea_block_for_write(handle->fs, handle->ino, inode);
	if (err)
		goto out2;

	/* Finally, write the new EA block */
	blk = ext2fs_file_acl_block(handle->fs,
				    (struct ext2_inode *)inode);
	err = ext2fs_write_ext_attr3(handle->fs, blk, block_buf,
				     handle->ino);
	if (err)
		goto out2;

skip_ea_block:
	blk = ext2fs_file_acl_block(handle->fs, (struct ext2_inode *)inode);
	if (!block_buf && blk) {
		/* xattrs shrunk, free the block */
		err = ext2fs_free_ext_attr(handle->fs, handle->ino, inode);
		if (err)
			goto out;
	}

	/* Write the inode */
	err = ext2fs_write_inode_full(handle->fs, handle->ino,
				      (struct ext2_inode *)inode,
				      EXT2_INODE_SIZE(handle->fs->super));
	if (err)
		goto out2;

out2:
	ext2fs_free_mem(&block_buf);
out:
	ext2fs_free_mem(&inode);
	handle->dirty = 0;
	return err;
}

static errcode_t read_xattrs_from_buffer(struct ext2_xattr_handle *handle,
					 struct ext2_ext_attr_entry *entries,
					 unsigned int storage_size,
					 char *value_start,
					 size_t *nr_read)
{
	struct ext2_xattr *x;
	struct ext2_ext_attr_entry *entry, *end;
	const char *prefix;
	unsigned int remain, prefix_len;
	errcode_t err;
	unsigned int values_size = storage_size +
			((char *)entries - value_start);

	x = handle->attrs;
	while (x->name)
		x++;

	/* find the end */
	end = entries;
	remain = storage_size;
	while (remain >= sizeof(struct ext2_ext_attr_entry) &&
	       !EXT2_EXT_IS_LAST_ENTRY(end)) {

		/* header eats this space */
		remain -= sizeof(struct ext2_ext_attr_entry);

		/* is attribute name valid? */
		if (EXT2_EXT_ATTR_SIZE(end->e_name_len) > remain)
			return EXT2_ET_EA_BAD_NAME_LEN;

		/* attribute len eats this space */
		remain -= EXT2_EXT_ATTR_SIZE(end->e_name_len);
		end = EXT2_EXT_ATTR_NEXT(end);
	}

	entry = entries;
	remain = storage_size;
	while (remain >= sizeof(struct ext2_ext_attr_entry) &&
	       !EXT2_EXT_IS_LAST_ENTRY(entry)) {
		__u32 hash;

		/* header eats this space */
		remain -= sizeof(struct ext2_ext_attr_entry);

		/* attribute len eats this space */
		remain -= EXT2_EXT_ATTR_SIZE(entry->e_name_len);

		/* check value size */
		if (entry->e_value_size > remain)
			return EXT2_ET_EA_BAD_VALUE_SIZE;

		if (entry->e_value_offs + entry->e_value_size > values_size)
			return EXT2_ET_EA_BAD_VALUE_OFFSET;

		if (entry->e_value_size > 0 &&
		    value_start + entry->e_value_offs <
		    (char *)end + sizeof(__u32))
			return EXT2_ET_EA_BAD_VALUE_OFFSET;

		/* e_value_block must be 0 in inode's ea */
		if (entry->e_value_block != 0)
			return EXT2_ET_BAD_EA_BLOCK_NUM;

		hash = ext2fs_ext_attr_hash_entry(entry, value_start +
							 entry->e_value_offs);

		/* e_hash may be 0 in older inode's ea */
		if (entry->e_hash != 0 && entry->e_hash != hash)
			return EXT2_ET_BAD_EA_HASH;

		remain -= entry->e_value_size;

		/* Allocate space for more attrs? */
		if (x == handle->attrs + handle->length) {
			err = ext2fs_xattrs_expand(handle, 4);
			if (err)
				return err;
			x = handle->attrs + handle->length - 4;
		}

		/* Extract name/value */
		prefix = find_ea_prefix(entry->e_name_index);
		prefix_len = (prefix ? strlen(prefix) : 0);
		err = ext2fs_get_memzero(entry->e_name_len + prefix_len + 1,
					 &x->name);
		if (err)
			return err;
		if (prefix)
			memcpy(x->name, prefix, prefix_len);
		if (entry->e_name_len)
			memcpy(x->name + prefix_len,
			       (char *)entry + sizeof(*entry),
			       entry->e_name_len);

		err = ext2fs_get_mem(entry->e_value_size, &x->value);
		if (err)
			return err;
		x->value_len = entry->e_value_size;
		memcpy(x->value, value_start + entry->e_value_offs,
		       entry->e_value_size);
		x++;
		(*nr_read)++;
		entry = EXT2_EXT_ATTR_NEXT(entry);
	}

	return 0;
}

static void xattrs_free_keys(struct ext2_xattr_handle *h)
{
	struct ext2_xattr *a = h->attrs;
	size_t i;

	for (i = 0; i < h->length; i++) {
		if (a[i].name)
			ext2fs_free_mem(&a[i].name);
		if (a[i].value)
			ext2fs_free_mem(&a[i].value);
	}
	h->count = 0;
}

errcode_t ext2fs_xattrs_read(struct ext2_xattr_handle *handle)
{
	struct ext2_inode_large *inode;
	struct ext2_ext_attr_header *header;
	__u32 ea_inode_magic;
	unsigned int storage_size;
	char *start, *block_buf = NULL;
	blk64_t blk;
	size_t i;
	errcode_t err;

	EXT2_CHECK_MAGIC(handle, EXT2_ET_MAGIC_EA_HANDLE);
	i = EXT2_INODE_SIZE(handle->fs->super);
	if (i < sizeof(*inode))
		i = sizeof(*inode);
	err = ext2fs_get_memzero(i, &inode);
	if (err)
		return err;

	err = ext2fs_read_inode_full(handle->fs, handle->ino,
				     (struct ext2_inode *)inode,
				     EXT2_INODE_SIZE(handle->fs->super));
	if (err)
		goto out;

	xattrs_free_keys(handle);

	/* Does the inode have space for EA? */
	if (inode->i_extra_isize < sizeof(inode->i_extra_isize) ||
	    EXT2_INODE_SIZE(handle->fs->super) <= EXT2_GOOD_OLD_INODE_SIZE +
						  inode->i_extra_isize +
						  sizeof(__u32))
		goto read_ea_block;
	if (inode->i_extra_isize & 3) {
		err = EXT2_ET_INODE_CORRUPTED;
		goto out;
	}

	/* Look for EA in the inode */
	memcpy(&ea_inode_magic, ((char *) inode) + EXT2_GOOD_OLD_INODE_SIZE +
	       inode->i_extra_isize, sizeof(__u32));
	if (ea_inode_magic == EXT2_EXT_ATTR_MAGIC) {
		storage_size = EXT2_INODE_SIZE(handle->fs->super) -
			EXT2_GOOD_OLD_INODE_SIZE - inode->i_extra_isize -
			sizeof(__u32);
		start = ((char *) inode) + EXT2_GOOD_OLD_INODE_SIZE +
			inode->i_extra_isize + sizeof(__u32);

		err = read_xattrs_from_buffer(handle,
			(struct ext2_ext_attr_entry *) start, storage_size,
					      start, &handle->count);
		if (err)
			goto out;
	}

read_ea_block:
	/* Look for EA in a separate EA block */
	blk = ext2fs_file_acl_block(handle->fs, (struct ext2_inode *)inode);
	if (blk != 0) {
		if ((blk < handle->fs->super->s_first_data_block) ||
		    (blk >= ext2fs_blocks_count(handle->fs->super))) {
			err = EXT2_ET_BAD_EA_BLOCK_NUM;
			goto out;
		}

		err = ext2fs_get_mem(handle->fs->blocksize, &block_buf);
		if (err)
			goto out;

		err = ext2fs_read_ext_attr3(handle->fs, blk, block_buf,
					    handle->ino);
		if (err)
			goto out3;

		/* We only know how to deal with v2 EA blocks */
		header = (struct ext2_ext_attr_header *) block_buf;
		if (header->h_magic != EXT2_EXT_ATTR_MAGIC) {
			err = EXT2_ET_BAD_EA_HEADER;
			goto out3;
		}

		/* Read EAs */
		storage_size = handle->fs->blocksize -
			sizeof(struct ext2_ext_attr_header);
		start = block_buf + sizeof(struct ext2_ext_attr_header);
		err = read_xattrs_from_buffer(handle,
			(struct ext2_ext_attr_entry *) start, storage_size,
					      block_buf, &handle->count);
		if (err)
			goto out3;

		ext2fs_free_mem(&block_buf);
	}

	ext2fs_free_mem(&block_buf);
	ext2fs_free_mem(&inode);
	return 0;

out3:
	ext2fs_free_mem(&block_buf);
out:
	ext2fs_free_mem(&inode);
	return err;
}

errcode_t ext2fs_xattrs_iterate(struct ext2_xattr_handle *h,
				int (*func)(char *name, char *value,
					    size_t value_len, void *data),
				void *data)
{
	struct ext2_xattr *x;
	int ret;

	EXT2_CHECK_MAGIC(h, EXT2_ET_MAGIC_EA_HANDLE);
	for (x = h->attrs; x < h->attrs + h->length; x++) {
		if (!x->name)
			continue;

		ret = func(x->name, x->value, x->value_len, data);
		if (ret & XATTR_CHANGED)
			h->dirty = 1;
		if (ret & XATTR_ABORT)
			return 0;
	}

	return 0;
}

errcode_t ext2fs_xattr_get(struct ext2_xattr_handle *h, const char *key,
			   void **value, size_t *value_len)
{
	struct ext2_xattr *x;
	char *val;
	errcode_t err;

	EXT2_CHECK_MAGIC(h, EXT2_ET_MAGIC_EA_HANDLE);
	for (x = h->attrs; x < h->attrs + h->length; x++) {
		if (!x->name || strcmp(x->name, key))
			continue;

		if (!(h->flags & XATTR_HANDLE_FLAG_RAW) &&
		    ((strcmp(key, "system.posix_acl_default") == 0) ||
		     (strcmp(key, "system.posix_acl_access") == 0))) {
			err = convert_disk_buffer_to_posix_acl(x->value, x->value_len,
							       value, value_len);
			return err;
		} else {
			err = ext2fs_get_mem(x->value_len, &val);
			if (err)
				return err;
			memcpy(val, x->value, x->value_len);
			*value = val;
			*value_len = x->value_len;
			return 0;
		}
	}

	return EXT2_ET_EA_KEY_NOT_FOUND;
}

errcode_t ext2fs_xattr_inode_max_size(ext2_filsys fs, ext2_ino_t ino,
				      size_t *size)
{
	struct ext2_ext_attr_entry *entry;
	struct ext2_inode_large *inode;
	__u32 ea_inode_magic;
	unsigned int minoff;
	char *start;
	size_t i;
	errcode_t err;

	i = EXT2_INODE_SIZE(fs->super);
	if (i < sizeof(*inode))
		i = sizeof(*inode);
	err = ext2fs_get_memzero(i, &inode);
	if (err)
		return err;

	err = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)inode,
				     EXT2_INODE_SIZE(fs->super));
	if (err)
		goto out;

	/* Does the inode have size for EA? */
	if (EXT2_INODE_SIZE(fs->super) <= EXT2_GOOD_OLD_INODE_SIZE +
						  inode->i_extra_isize +
						  sizeof(__u32)) {
		err = EXT2_ET_INLINE_DATA_NO_SPACE;
		goto out;
	}

	minoff = EXT2_INODE_SIZE(fs->super) - sizeof(*inode) - sizeof(__u32);
	memcpy(&ea_inode_magic, ((char *) inode) + EXT2_GOOD_OLD_INODE_SIZE +
	       inode->i_extra_isize, sizeof(__u32));
	if (ea_inode_magic == EXT2_EXT_ATTR_MAGIC) {
		/* has xattrs.  calculate the size */
		start= ((char *) inode) + EXT2_GOOD_OLD_INODE_SIZE +
			inode->i_extra_isize + sizeof(__u32);
		entry = (struct ext2_ext_attr_entry *) start;
		while (!EXT2_EXT_IS_LAST_ENTRY(entry)) {
			if (!entry->e_value_block && entry->e_value_size) {
				unsigned int offs = entry->e_value_offs;
				if (offs < minoff)
					minoff = offs;
			}
			entry = EXT2_EXT_ATTR_NEXT(entry);
		}
		*size = minoff - ((char *)entry - (char *)start) - sizeof(__u32);
	} else {
		/* no xattr.  return a maximum size */
		*size = EXT2_EXT_ATTR_SIZE(minoff -
					   EXT2_EXT_ATTR_LEN(strlen("data")) -
					   EXT2_EXT_ATTR_ROUND - sizeof(__u32));
	}

out:
	ext2fs_free_mem(&inode);
	return err;
}

errcode_t ext2fs_xattr_set(struct ext2_xattr_handle *handle,
			   const char *key,
			   const void *value,
			   size_t value_len)
{
	struct ext2_xattr *x, *last_empty;
	char *new_value;
	errcode_t err;

	EXT2_CHECK_MAGIC(handle, EXT2_ET_MAGIC_EA_HANDLE);
	last_empty = NULL;

	err = ext2fs_get_mem(value_len, &new_value);
	if (err)
		return err;
	if (!(handle->flags & XATTR_HANDLE_FLAG_RAW) &&
	    ((strcmp(key, "system.posix_acl_default") == 0) ||
	     (strcmp(key, "system.posix_acl_access") == 0))) {
		err = convert_posix_acl_to_disk_buffer(value, value_len,
						       new_value, &value_len);
		if (err)
			goto errout;
	} else
		memcpy(new_value, value, value_len);

	for (x = handle->attrs; x < handle->attrs + handle->length; x++) {
		if (!x->name) {
			last_empty = x;
			continue;
		}

		/* Replace xattr */
		if (strcmp(x->name, key) == 0) {
			ext2fs_free_mem(&x->value);
			x->value = new_value;
			x->value_len = value_len;
			handle->dirty = 1;
			return 0;
		}
	}

	/* Add attr to empty slot */
	if (last_empty) {
		err = ext2fs_get_mem(strlen(key) + 1, &last_empty->name);
		if (err)
			goto errout;
		strcpy(last_empty->name, key);
		last_empty->value = new_value;
		last_empty->value_len = value_len;
		handle->dirty = 1;
		handle->count++;
		return 0;
	}

	/* Expand array, append slot */
	err = ext2fs_xattrs_expand(handle, 4);
	if (err)
		goto errout;

	x = handle->attrs + handle->length - 4;
	err = ext2fs_get_mem(strlen(key) + 1, &x->name);
	if (err)
		goto errout;
	strcpy(x->name, key);

	err = ext2fs_get_mem(value_len, &x->value);
	if (err)
		goto errout;
	memcpy(x->value, value, value_len);
	x->value_len = value_len;
	handle->dirty = 1;
	handle->count++;
	return 0;
errout:
	ext2fs_free_mem(&new_value);
	return err;
}

errcode_t ext2fs_xattr_remove(struct ext2_xattr_handle *handle,
			      const char *key)
{
	struct ext2_xattr *x;

	EXT2_CHECK_MAGIC(handle, EXT2_ET_MAGIC_EA_HANDLE);
	for (x = handle->attrs; x < handle->attrs + handle->length; x++) {
		if (!x->name)
			continue;

		if (strcmp(x->name, key) == 0) {
			ext2fs_free_mem(&x->name);
			ext2fs_free_mem(&x->value);
			x->value_len = 0;
			handle->dirty = 1;
			handle->count--;
			return 0;
		}
	}

	/* no key found, success! */
	return 0;
}

errcode_t ext2fs_xattrs_open(ext2_filsys fs, ext2_ino_t ino,
			     struct ext2_xattr_handle **handle)
{
	struct ext2_xattr_handle *h;
	errcode_t err;

	if (!ext2fs_has_feature_xattr(fs->super) &&
	    !ext2fs_has_feature_inline_data(fs->super))
		return EXT2_ET_MISSING_EA_FEATURE;

	err = ext2fs_get_memzero(sizeof(*h), &h);
	if (err)
		return err;

	h->magic = EXT2_ET_MAGIC_EA_HANDLE;
	h->length = 4;
	err = ext2fs_get_arrayzero(h->length, sizeof(struct ext2_xattr),
				   &h->attrs);
	if (err) {
		ext2fs_free_mem(&h);
		return err;
	}
	h->count = 0;
	h->ino = ino;
	h->fs = fs;
	*handle = h;
	return 0;
}

errcode_t ext2fs_xattrs_close(struct ext2_xattr_handle **handle)
{
	struct ext2_xattr_handle *h = *handle;
	errcode_t err;

	EXT2_CHECK_MAGIC(h, EXT2_ET_MAGIC_EA_HANDLE);
	if (h->dirty) {
		err = ext2fs_xattrs_write(h);
		if (err)
			return err;
	}

	xattrs_free_keys(h);
	ext2fs_free_mem(&h->attrs);
	ext2fs_free_mem(handle);
	return 0;
}

errcode_t ext2fs_xattrs_count(struct ext2_xattr_handle *handle, size_t *count)
{
	EXT2_CHECK_MAGIC(handle, EXT2_ET_MAGIC_EA_HANDLE);
	*count = handle->count;
	return 0;
}

errcode_t ext2fs_xattrs_flags(struct ext2_xattr_handle *handle,
			      unsigned int *new_flags, unsigned int *old_flags)
{
	EXT2_CHECK_MAGIC(handle, EXT2_ET_MAGIC_EA_HANDLE);
	if (old_flags)
		*old_flags = handle->flags;
	if (new_flags)
		handle->flags = *new_flags;
	return 0;
}
