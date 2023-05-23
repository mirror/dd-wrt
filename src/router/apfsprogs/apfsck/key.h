/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _KEY_H
#define _KEY_H

#include <apfs/types.h>

struct super_block;

/*
 * In-memory representation of a key, as relevant for a b-tree query.
 */
struct key {
	u64		id;
	u64		number;	/* Extent offset or name hash */
	const char	*name;	/* On-disk name string */
	u8		type;	/* Record type (0 for the omap) */
};

/**
 * init_omap_key - Initialize an in-memory key for an omap query
 * @oid:	object id
 * @xid:	current transaction id
 * @key:	apfs_key structure to initialize
 */
static inline void init_omap_key(u64 oid, u64 xid, struct key *key)
{
	key->id = oid;
	key->type = 0;
	key->number = xid;
	key->name = NULL;
}

/**
 * init_extref_key - Initialize an in-memory key for an extentref query
 * @bno:	first block number
 * @key:	apfs_key structure to initialize
 */
static inline void init_extref_key(u64 bno, struct key *key)
{
	key->id = bno;
	key->type = APFS_TYPE_EXTENT;
	key->number = 0;
	key->name = NULL;
}

/**
 * init_fext_key - Initialize an in-memory key for a fext query
 * @id:		dstream id
 * @addr:	logical address
 * @key:	apfs_key structure to initialize
 */
static inline void init_fext_key(u64 id, u64 addr, struct key *key)
{
	key->id = id;
	key->type = 0;
	key->number = addr;
	key->name = NULL;
}

/**
 * init_inode_key - Initialize an in-memory key for an inode query
 * @ino:	inode number
 * @key:	key structure to initialize
 */
static inline void init_inode_key(u64 ino, struct key *key)
{
	key->id = ino;
	key->type = APFS_TYPE_INODE;
	key->number = 0;
	key->name = NULL;
}

/**
 * init_file_extent_key - Initialize an in-memory key for an extent query
 * @id:		extent id
 * @offset:	logical address (0 for a multiple query)
 * @key:	key structure to initialize
 */
static inline void init_file_extent_key(u64 id, u64 offset, struct key *key)
{
	key->id = id;
	key->type = APFS_TYPE_FILE_EXTENT;
	key->number = offset;
	key->name = NULL;
}

/**
 * init_xattr_key - Initialize an in-memory key for a xattr query
 * @ino:	inode number of the parent file
 * @name:	xattr name (NULL for a multiple query)
 * @key:	key structure to initialize
 */
static inline void init_xattr_key(u64 ino, const char *name, struct key *key)
{
	key->id = ino;
	key->type = APFS_TYPE_XATTR;
	key->number = 0;
	key->name = name;
}

/**
 * cat_type - Read the record type of a catalog key
 * @key: the raw catalog key
 *
 * The record type is stored in the last byte of the cnid field; this function
 * returns that value.
 */
static inline int cat_type(struct apfs_key_header *key)
{
	return (le64_to_cpu(key->obj_id_and_type) & APFS_OBJ_TYPE_MASK)
			>> APFS_OBJ_TYPE_SHIFT;
}

/**
 * cat_cnid - Read the cnid value on a catalog key
 * @key: the raw catalog key
 *
 * The cnid value shares the its field with the record type. This function
 * masks that part away and returns the result.
 */
static inline u64 cat_cnid(struct apfs_key_header *key)
{
	return le64_to_cpu(key->obj_id_and_type) & APFS_OBJ_ID_MASK;
}

extern int keycmp(struct key *k1, struct key *k2);
extern void read_cat_key(void *raw, int size, struct key *key);
extern void read_omap_key(void *raw, int size, struct key *key);
extern void read_extentref_key(void *raw, int size, struct key *key);
extern void read_free_queue_key(void *raw, int size, struct key *key);
extern void read_snap_key(void *raw, int size, struct key *key);
extern void read_omap_snap_key(void *raw, int size, struct key *key);
extern void read_fext_key(void *raw, int size, struct key *key);

#endif	/* _KEY_H */
