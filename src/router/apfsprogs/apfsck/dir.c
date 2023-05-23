/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <apfs/raw.h>
#include "apfsck.h"
#include "dir.h"
#include "inode.h"
#include "key.h"
#include "super.h"

/**
 * read_sibling_id_xfield - Parse a sibling id xfield and check its consistency
 * @xval:	pointer to the xfield value
 * @len:	remaining length of the dentry value
 * @sibling_id:	on return, the sibling id
 *
 * Returns the length of the xfield value.
 */
static int read_sibling_id_xfield(char *xval, int len, u64 *sibling_id)
{
	__le64 *id_raw;

	if (len < 8)
		report("Sibling link xfield", "doesn't fit in dentry record.");
	id_raw = (__le64 *)xval;

	*sibling_id = le64_to_cpu(*id_raw);

	return sizeof(*id_raw);
}

/**
 * parse_dentry_xfields - Parse and check a dentry extended fields
 * @xblob:	pointer to the beginning of the xfields in the dentry value
 * @len:	length of the xfields
 * @sibling_id: on return, the sibling id (0 if none)
 *
 * Internal consistency of @key must be checked before calling this function.
 */
static void parse_dentry_xfields(struct apfs_xf_blob *xblob, int len,
				 u64 *sibling_id)
{
	struct apfs_x_field *xfield;
	char *xval;
	int xlen;

	*sibling_id = 0;
	if (len == 0) /* No extended fields */
		return;

	len -= sizeof(*xblob);
	if (len < 0)
		report("Dentry record", "no room for extended fields.");
	if (le16_to_cpu(xblob->xf_num_exts) != 1)
		report("Dentry record", "bad xfield count.");

	xfield = (struct apfs_x_field *)xblob->xf_data;
	xval = (char *)xfield + sizeof(*xfield);
	len -= sizeof(*xfield);
	if (len < 0)
		report("Dentry record", "xfield cannot fit.");

	/* The official reference seems to be wrong here */
	if (le16_to_cpu(xblob->xf_used_data) != len)
		report("Dentry record",
		       "value size incompatible with xfields.");

	check_xfield_flags(xfield->x_flags);

	if (xfield->x_type != APFS_DREC_EXT_TYPE_SIBLING_ID)
		report("Dentry xfield", "invalid type.");
	xlen = read_sibling_id_xfield(xval, len, sibling_id);

	if (xlen != le16_to_cpu(xfield->x_size))
		report("Dentry xfield", "wrong size");
	if (xlen != len)
		report("Dentry record", "wrong used space for xfields.");
}

/**
 * check_orphan_name - Check the dentry name for an orphan inode
 * @name: the filename (as reported by the dentry, not the inode)
 * @ino: the inode number
 */
static void check_orphan_name(const char *name, u64 ino)
{
	const char *format = "0x%llx-dead";
	char *buf;
	int buflen;

	buflen = snprintf(NULL, 0, format, (unsigned long long)ino);
	if (buflen < 0)
		system_error();
	buf = calloc(1, ++buflen);
	if (!buf)
		system_error();
	buflen = snprintf(buf, buflen, format, (unsigned long long)ino);
	if (buflen < 0)
		system_error();

	if (strcmp(name, buf) != 0)
		report("Orphan inode", "wrong name.");

	free(buf);
}

/**
 * parse_dentry_record - Parse a dentry record value and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_dentry_record(void *key, struct apfs_drec_val *val, int len)
{
	u64 ino, parent_ino;
	struct inode *inode, *parent;
	char *name;
	int namelen;
	u16 filetype, dtype, flags;
	u64 sibling_id;
	struct sibling *sibling;

	if (apfs_is_normalization_insensitive()) {
		struct apfs_drec_hashed_key *hkey = key;
		name = (char *)hkey->name;
		namelen = le32_to_cpu(hkey->name_len_and_hash) & 0x3FFU;
	} else {
		struct apfs_drec_key *ukey = key;
		name = (char *)ukey->name;
		namelen = le16_to_cpu(ukey->name_len);
	}

	if (len < sizeof(*val))
		report("Dentry record", "value is too small.");

	ino = le64_to_cpu(val->file_id);
	inode = get_inode(ino);
	parent_ino = cat_cnid(key);

	/* The purgeable dentry doesn't count for nlink */
	if (parent_ino != APFS_PURGEABLE_DIR_INO_NUM)
		inode->i_link_count++;

	if (ino == APFS_ROOT_DIR_INO_NUM && strcmp(name, "root"))
		report("Root directory", "wrong name.");
	if (ino == APFS_PRIV_DIR_INO_NUM && strcmp(name, "private-dir"))
		report("Private directory", "wrong name.");

	/* Not required by the specification, but follow what apple does */
	if (parent_ino == APFS_PRIV_DIR_INO_NUM)
		check_orphan_name(name, ino);

	if (parent_ino == APFS_PURGEABLE_DIR_INO_NUM) {
		if (inode->i_purg_name)
			report("Inode", "has two purgeable dentry records.");
		inode->i_purg_name = strdup(name);
		if (!inode->i_purg_name)
			system_error();
	}

	check_inode_ids(ino, parent_ino);
	if (parent_ino != APFS_ROOT_DIR_PARENT && parent_ino != APFS_PURGEABLE_DIR_INO_NUM) {
		parent = get_inode(parent_ino);
		if (!parent->i_seen) /* The b-tree keys are in order */
			report("Dentry record", "parent inode missing");
		if ((parent->i_mode & S_IFMT) != S_IFDIR)
			report("Dentry record", "parent inode not directory.");
		parent->i_child_count++;
	}

	/* The purgeable dentry is never reported as inode name and parent id */
	if (parent_ino != APFS_PURGEABLE_DIR_INO_NUM && !inode->i_first_name) {
		/* No dentry for this inode has been seen before */
		inode->i_first_name = malloc(namelen);
		if (!inode->i_first_name)
			system_error();
		strcpy(inode->i_first_name, name);
		inode->i_first_parent = parent_ino;
	}

	flags = le16_to_cpu(val->flags);
	dtype = flags & APFS_DREC_TYPE_MASK;
	if (flags & ~(APFS_DREC_TYPE_MASK | APFS_DREC_PURGEABLE))
		report("Dentry record", "reserved flags in use.");

	if ((bool)(flags & APFS_DREC_PURGEABLE) != (parent_ino == APFS_PURGEABLE_DIR_INO_NUM))
		report("Dentry record", "the purgeable dir is for purgeable dentries.");

	if (flags & APFS_DREC_PURGEABLE) {
		/* No idea. The inode and other hardlinks have normal modes */
		if (flags & APFS_DREC_PURGEABLE_2 && dtype != 5)
			report("Dentry record", "wrong type for purgeable dentry.");
		if (flags & APFS_DREC_PURGEABLE_8 && dtype != 4)
			report("Dentry record", "wrong type for purgeable dentry.");
	} else {
		/* The mode may have already been set by the inode or another dentry */
		filetype = inode->i_mode >> 12;
		if (filetype && filetype != dtype)
			report("Dentry record", "file mode doesn't match dentry type.");
		if (dtype == 0) /* Don't save a 0, that means the mode is not set */
			report("Dentry record", "invalid dentry type.");
		inode->i_mode |= dtype << 12;
	}

	/*
	 * Orphans aren't counted for the file totals, but I can't tell them
	 * apart inside parse_inode_record(), so do it here instead.
	 */
	if (parent_ino == APFS_PRIV_DIR_INO_NUM) {
		switch (dtype << 12) {
		case S_IFREG:
			vsb->v_file_count--;
			break;
		case S_IFDIR:
			if (inode->i_ino >= APFS_MIN_USER_INO_NUM)
				vsb->v_dir_count--;
			break;
		case S_IFLNK:
			vsb->v_symlink_count--;
			break;
		case S_IFSOCK:
		case S_IFBLK:
		case S_IFCHR:
		case S_IFIFO:
			vsb->v_special_count--;
			break;
		default:
			report("Dentry record", "invalid file mode.");
		}
	}

	parse_dentry_xfields((struct apfs_xf_blob *)val->xfields,
			     len - sizeof(*val), &sibling_id);

	if (!sibling_id) /* No sibling record for this dentry */
		return;
	sibling = get_sibling(sibling_id, inode);
	set_or_check_sibling(parent_ino, namelen, (u8 *)name, sibling);
}

/* TODO: update the checks for cnid reuse, here and elsewhere */
static void free_dirstat(struct htable_entry *entry)
{
	struct dirstat *stats = (struct dirstat *)entry;

	/* The inodes must be parsed before the dirstats */
	assert(!vsb->v_inode_table);

	if (!stats->ds_origin_seen)
		report("Directory stats", "have no inode.");

	if (stats->ds_num_children != stats->ds_child_count)
		report("Directory stats", "wrong child count.");
	if (stats->ds_total_size != stats->ds_child_size)
		report("Directory stats", "wrong total size.");
	if (stats->ds_chained_key)
		report_unknown("Chained key in directory stats");

	free(entry);
}

void free_dirstat_table(struct htable_entry **table)
{
	free_htable(table, free_dirstat);
}

struct dirstat *get_dirstat(u64 oid)
{
	struct htable_entry *entry;

	entry = get_htable_entry(oid, sizeof(struct dirstat), vsb->v_dirstat_table);
	return (struct dirstat *)entry;
}

void parse_dir_stats_record(void *key, struct apfs_dir_stats_val *val, int len)
{
	struct dirstat *stats = NULL;

	if (len != sizeof(*val))
		report("Dir stats record", "wrong size of value.");

	stats = get_dirstat(cat_cnid(key));
	stats->ds_seen = true;
	stats->ds_num_children = le64_to_cpu(val->num_children);
	stats->ds_total_size = le64_to_cpu(val->total_size);
	stats->ds_chained_key = le64_to_cpu(val->chained_key);
}
