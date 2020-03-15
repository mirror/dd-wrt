/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

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
 * parse_dentry_record - Parse a dentry record value and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_dentry_record(struct apfs_drec_hashed_key *key,
			 struct apfs_drec_val *val, int len)
{
	u64 ino, parent_ino;
	struct inode *inode, *parent;
	char *name = (char *)key->name;
	int namelen = le32_to_cpu(key->name_len_and_hash) & 0x3FFU;
	u16 filetype, dtype;
	u64 sibling_id;
	struct sibling *sibling;

	if (len < sizeof(*val))
		report("Dentry record", "value is too small.");

	ino = le64_to_cpu(val->file_id);
	inode = get_inode(ino);
	inode->i_link_count++;

	if (ino == APFS_ROOT_DIR_INO_NUM && strcmp(name, "root"))
		report("Root directory", "wrong name.");
	if (ino == APFS_PRIV_DIR_INO_NUM && strcmp(name, "private-dir"))
		report("Private directory", "wrong name.");

	parent_ino = cat_cnid(&key->hdr);
	check_inode_ids(ino, parent_ino);
	if (parent_ino != APFS_ROOT_DIR_PARENT) {
		parent = get_inode(parent_ino);
		if (!parent->i_seen) /* The b-tree keys are in order */
			report("Dentry record", "parent inode missing");
		if ((parent->i_mode & S_IFMT) != S_IFDIR)
			report("Dentry record", "parent inode not directory.");
		parent->i_child_count++;
	}

	if (!inode->i_first_name) {
		/* No dentry for this inode has been seen before */
		inode->i_first_name = malloc(namelen);
		if (!inode->i_first_name)
			system_error();
		strcpy(inode->i_first_name, name);
		inode->i_first_parent = parent_ino;
	}

	dtype = le16_to_cpu(val->flags) & APFS_DREC_TYPE_MASK;
	if (dtype != le16_to_cpu(val->flags))
		report("Dentry record", "reserved flags in use.");

	/* The mode may have already been set by the inode or another dentry */
	filetype = inode->i_mode >> 12;
	if (filetype && filetype != dtype)
		report("Dentry record", "file mode doesn't match dentry type.");
	if (dtype == 0) /* Don't save a 0, that means the mode is not set */
		report("Dentry record", "invalid dentry type.");
	inode->i_mode |= dtype << 12;

	parse_dentry_xfields((struct apfs_xf_blob *)val->xfields,
			     len - sizeof(*val), &sibling_id);

	if (!sibling_id) /* No sibling record for this dentry */
		return;
	sibling = get_sibling(sibling_id, inode);
	set_or_check_sibling(parent_ino, namelen, key->name, sibling);
}
