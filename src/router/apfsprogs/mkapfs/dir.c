/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <sys/stat.h> /* The macros for the inode mode */
#include <sys/types.h>
#include <unistd.h>
#include <apfs/checksum.h>
#include <apfs/raw.h>
#include <apfs/types.h>
#include "dir.h"
#include "mkapfs.h"

/**
 * set_key_header - Set the cnid and type on a catalog key
 * @ino:	inode number
 * @type:	record type
 * @key:	key header to set
 */
static void set_key_header(u64 ino, u64 type, struct apfs_key_header *key)
{
	u64 obj_id_and_type;

	obj_id_and_type = type << APFS_OBJ_TYPE_SHIFT;
	obj_id_and_type |= ino;
	key->obj_id_and_type = cpu_to_le64(obj_id_and_type);
}

/**
 * make_special_dentry_key - Make the dentry key for a special directory
 * @ino:	inode number for the parent
 * @name:	directory name
 * @key:	key space to use
 *
 * Returns the length of the key.
 */
static int make_special_dentry_key(u64 ino, char *name,
				   struct apfs_drec_hashed_key *key)
{
	u32 len;
	u32 hash = 0xFFFFFFFF;
	u32 utf32;

	set_key_header(ino, APFS_TYPE_DIR_REC, &key->hdr);
	strcpy((char *)key->name, name);

	len = strlen(name) + 1; /* The null termination is counted */

	/* Special directories don't have unicode characters in their names */
	for (utf32 = *name; utf32; utf32 = *++name)
		hash = crc32c(hash, &utf32, sizeof(utf32));
	hash = (hash & 0x3FFFFF) << 10;

	key->name_len_and_hash = cpu_to_le32(hash | len);
	return sizeof(*key) + len;
}

/**
 * make_special_dentry_val - Make the dentry value for a special directory
 * @ino:	inode number for the directory
 * @val_end:	end of the value space to use
 *
 * Returns the length of the value.
 */
static int make_special_dentry_val(u64 ino, struct apfs_drec_val *val_end)
{
	struct apfs_drec_val *val = (void *)val_end - sizeof(*val);

	val->file_id = cpu_to_le64(ino);
	val->date_added = cpu_to_le64(get_timestamp());
	val->flags = cpu_to_le16(S_IFDIR >> 12);

	return sizeof(*val);
}

/**
 * make_special_inode_key - Make the inode key for a special directory
 * @ino:	inode number for the directory
 * @key:	key space to use
 *
 * Returns the length of the key.
 */
static int make_special_inode_key(u64 ino, struct apfs_inode_key *key)
{
	set_key_header(ino, APFS_TYPE_INODE, &key->hdr);
	return sizeof(*key);
}

/**
 * make_special_inode_val - Make the inode value for a special directory
 * @ino:	inode number for the directory
 * @name:	directory name
 * @val_end:	end of the value space to use
 *
 * Returns the length of the value.
 */
static int make_special_inode_val(u64 ino, char *name,
				  struct apfs_inode_val *val_end)
{
	struct apfs_inode_val *val = (void *)val_end - sizeof(*val);
	struct apfs_xf_blob *xblob;
	struct apfs_x_field *name_xfield;
	int namelen, padded_namelen, i_len;

	namelen = strlen(name) + 1;
	padded_namelen = ROUND_UP(namelen, 8);
	i_len = sizeof(*val) + sizeof(*xblob) +
		sizeof(*name_xfield) + padded_namelen;
	val = (void *)val_end - i_len;

	val->parent_id = cpu_to_le64(APFS_ROOT_DIR_PARENT);
	val->private_id = cpu_to_le64(ino);

	/* Should this be the exact same as the dentry timetamp? */
	val->create_time = val->mod_time = val->change_time =
			   val->access_time = cpu_to_le64(get_timestamp());

	val->default_protection_class =
				cpu_to_le32(APFS_PROTECTION_CLASS_DIR_NONE);

	/* TODO: allow the user to override these fields */
	val->owner = cpu_to_le32(geteuid());
	val->group = cpu_to_le32(getegid());
	val->mode = cpu_to_le16(0755 | S_IFDIR);

	xblob = (struct apfs_xf_blob *)val->xfields;
	xblob->xf_num_exts = cpu_to_le16(1); /* Just the primary name */
	xblob->xf_used_data = cpu_to_le16(padded_namelen);

	name_xfield = (struct apfs_x_field *)xblob->xf_data;
	name_xfield->x_type = APFS_INO_EXT_TYPE_NAME;
	name_xfield->x_flags = APFS_XF_DO_NOT_COPY;
	name_xfield->x_size = cpu_to_le16(namelen);

	strcpy((char *)val_end - padded_namelen, name);
	return i_len;
}

/**
 * make_special_dir_inode - Make inode record for an empty special dir
 * @ino:		inode number for the directory
 * @name:		directory name
 * @next_toc:		next available toc entry (updated on return)
 * @key_area:		start of the key area
 * @key:		start of the available key area (updated on return)
 * @val_area_end:	end of the value area
 * @val_end:		end of the available value area (updated on return)
 */
void make_special_dir_inode(u64 ino, char *name, struct apfs_kvloc **next_toc,
			    void *key_area, void **key,
			    void *val_area_end, void **val_end)
{
	struct apfs_kvloc *kvloc = *next_toc;
	int len;

	len = make_special_inode_key(ino, *key);
	kvloc->k.off = cpu_to_le16(*key - key_area);
	kvloc->k.len = cpu_to_le16(len);
	*key += len;

	len = make_special_inode_val(ino, name, *val_end);
	kvloc->v.off = cpu_to_le16(val_area_end - *val_end + len);
	kvloc->v.len = cpu_to_le16(len);
	*val_end -= len;

	++*next_toc;
}

/**
 * make_special_dir_dentry - Make dentry record for an empty special dir
 * @ino:		inode number for the directory
 * @name:		directory name
 * @next_toc:		next available toc entry (updated on return)
 * @key_area:		start of the key area
 * @key:		start of the available key area (updated on return)
 * @val_area_end:	end of the value area
 * @val_end:		end of the available value area (updated on return)
 */
void make_special_dir_dentry(u64 ino, char *name, struct apfs_kvloc **next_toc,
			     void *key_area, void **key,
			     void *val_area_end, void **val_end)
{
	struct apfs_kvloc *kvloc = *next_toc;
	int len;

	len = make_special_dentry_key(APFS_ROOT_DIR_PARENT, name, *key);
	kvloc->k.off = cpu_to_le16(*key - key_area);
	kvloc->k.len = cpu_to_le16(len);
	*key += len;

	len = make_special_dentry_val(ino, *val_end);
	kvloc->v.off = cpu_to_le16(val_area_end - *val_end + len);
	kvloc->v.len = cpu_to_le16(len);
	*val_end -= len;

	++*next_toc;
}
