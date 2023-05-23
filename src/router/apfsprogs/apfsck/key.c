/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <apfs/checksum.h>
#include <apfs/raw.h>
#include <apfs/types.h>
#include <apfs/unicode.h>
#include "apfsck.h"
#include "key.h"
#include "super.h"

/**
 * read_omap_key - Parse an on-disk object map key
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
void read_omap_key(void *raw, int size, struct key *key)
{
	u64 xid;

	if (size != sizeof(struct apfs_omap_key))
		report("Object map", "wrong size of key.");

	xid = le64_to_cpu(((struct apfs_omap_key *)raw)->ok_xid);
	if (!xid)
		report("Object map", "transaction id for key is zero.");

	key->id = le64_to_cpu(((struct apfs_omap_key *)raw)->ok_oid);
	key->type = 0;
	key->number = xid;
	key->name = NULL;
}

/**
 * read_free_queue_key - Parse an on-disk free-space queue key
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
void read_free_queue_key(void *raw, int size, struct key *key)
{
	struct apfs_spaceman_free_queue_key *sfqk;
	u64 xid;

	if (size != sizeof(*sfqk))
		report("Free-space queue", "wrong size of key.");
	sfqk = (struct apfs_spaceman_free_queue_key *)raw;

	xid = le64_to_cpu(sfqk->sfqk_xid);
	if (!xid)
		report("Free-space queue", "transaction id for key is zero.");

	key->id = xid;
	key->type = 0;
	key->number = le64_to_cpu(sfqk->sfqk_paddr);
	key->name = NULL;
}

/**
 * keycmp - Compare two keys
 * @k1, @k2:	keys to compare
 *
 * returns   0 if @k1 and @k2 are equal
 *	   < 0 if @k1 comes before @k2 in the btree
 *	   > 0 if @k1 comes after @k2 in the btree
 */
int keycmp(struct key *k1, struct key *k2)
{
	if (k1->id != k2->id)
		return k1->id < k2->id ? -1 : 1;
	if (k1->type != k2->type)
		return k1->type < k2->type ? -1 : 1;
	if (k1->number != k2->number)
		return k1->number < k2->number ? -1 : 1;
	if (!k1->name) /* Keys of this type have no name */
		return 0;

	/* Normalization seems to be ignored here, even for directory records */
	return strcmp(k1->name, k2->name);
}

/**
 * dentry_hash - Find the key hash for a given filename
 * @name: filename to hash
 */
static u32 dentry_hash(const char *name)
{
	struct unicursor cursor;
	bool case_fold = apfs_is_case_insensitive();
	u32 hash = 0xFFFFFFFF;

	init_unicursor(&cursor, name);

	while (1) {
		unicode_t utf32;

		utf32 = normalize_next(&cursor, case_fold);
		if (!utf32)
			break;

		hash = crc32c(hash, &utf32, sizeof(utf32));
	}

	/* Leave room for the filename length */
	return (hash & 0x3FFFFF) << 10;
}

/**
 * read_dir_rec_key - Parse an on-disk dentry key and check its consistency
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
static void read_dir_rec_key(void *raw, int size, struct key *key)
{
	bool hashed = apfs_is_normalization_insensitive();
	int namelen;

	if (hashed && size < sizeof(struct apfs_drec_hashed_key) + 1)
		report("Hashed directory record", "wrong size of key.");
	if (!hashed && size < sizeof(struct apfs_drec_key) + 1)
		report("Unhashed directory record", "wrong size of key.");

	if (*((char *)raw + size - 1) != 0)
		report("Directory record", "filename lacks NULL-termination.");

	if (hashed) {
		struct apfs_drec_hashed_key *raw_key = raw;

		/* The filename length is ignored for the ordering, so mask it away */
		key->number = le32_to_cpu(raw_key->name_len_and_hash) & ~0x3FFU;
		key->name = (char *)raw_key->name;
		if (key->number != dentry_hash(key->name))
			report("Directory record", "filename hash is corrupted.");
		namelen = le32_to_cpu(raw_key->name_len_and_hash) & 0x3FFU;
		if (size != sizeof(*raw_key) + namelen) {
			report("Hashed directory record",
			       "size of key doesn't match the name length.");
		}
	} else {
		struct apfs_drec_key *raw_key = raw;

		key->number = 0;
		key->name = (char *)raw_key->name;
		namelen = le16_to_cpu(raw_key->name_len);
		if (size != sizeof(*raw_key) + namelen) {
			report("Unhashed directory record",
			       "size of key doesn't match the name length.");
		}
	}

	if (namelen > 256) {
		/* The name must fit in name_buf from parse_subtree() */
		report("Directory record", "name is too long.");
	}
	if (strlen(key->name) + 1 != namelen) {
		/* APFS counts the NULL termination for the filename length */
		report("Directory record", "wrong name length in key.");
	}
}

/**
 * read_xattr_key - Parse an on-disk xattr key and check its consistency
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
static void read_xattr_key(void *raw, int size, struct key *key)
{
	struct apfs_xattr_key *raw_key;
	int namelen;

	if (size < sizeof(struct apfs_xattr_key) + 1)
		report("Xattr record", "wrong size of key.");
	if (*((char *)raw + size - 1) != 0)
		report("Xattr record", "name lacks NULL-termination.");
	raw_key = raw;

	key->number = 0;
	key->name = (char *)raw_key->name;

	namelen = le16_to_cpu(raw_key->name_len);
	if (namelen > 256) {
		/* The name must fit in name_buf from parse_subtree() */
		report("Xattr record", "name is too long.");
	}
	if (strlen(key->name) + 1 != namelen) {
		/* APFS counts the NULL termination in the string length */
		report("Xattr record", "wrong name length.");
	}
	if (size != sizeof(struct apfs_xattr_key) + namelen) {
		report("Xattr record",
		       "size of key doesn't match the name length.");
	}
}

/**
 * read_snap_name_key - Parse an on-disk snapshot name key and check its
 *			consistency
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 *
 * TODO: this is the same as read_xattr_key(), maybe they could be merged.
 */
static void read_snap_name_key(void *raw, int size, struct key *key)
{
	struct apfs_snap_name_key *raw_key;
	int namelen;

	if (size < sizeof(struct apfs_snap_name_key) + 1)
		report("Snapshot name record", "wrong size of key.");
	if (*((char *)raw + size - 1) != 0)
		report("Snapshot name record", "name lacks NULL-termination.");
	raw_key = raw;

	key->number = 0;
	key->name = (char *)raw_key->name;

	namelen = le16_to_cpu(raw_key->name_len);
	if (namelen > 256) {
		/* The name must fit in name_buf from parse_subtree() */
		report("Snapshot name record", "name is too long.");
	}
	if (strlen(key->name) + 1 != namelen) {
		/* APFS counts the NULL termination in the string length */
		report("Snapshot name record", "wrong name length.");
	}
	if (size != sizeof(struct apfs_snap_name_key) + namelen) {
		report("Snapshot name record",
		       "size of key doesn't match the name length.");
	}
}

/**
 * read_file_extent_key - Parse an on-disk extent key and check its consistency
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
static void read_file_extent_key(void *raw, int size, struct key *key)
{
	struct apfs_file_extent_key *raw_key;

	if (size != sizeof(struct apfs_file_extent_key))
		report("Extent record", "wrong size of key.");
	raw_key = raw;

	key->number = le64_to_cpu(raw_key->logical_addr);
	key->name = NULL;

	if (key->number & (sb->s_blocksize - 1))
		report("Extent record", "offset isn't multiple of block size.");
}

/**
 * read_file_info_key - Parse an on-disk file info key and check its consistency
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
static void read_file_info_key(void *raw, int size, struct key *key)
{
	struct apfs_file_info_key *raw_key;
	u64 info_and_lba;

	if (size != sizeof(struct apfs_file_info_key))
		report("File info record", "wrong size of key.");
	raw_key = raw;

	info_and_lba = le64_to_cpu(raw_key->info_and_lba);
	if ((info_and_lba >> APFS_FILE_INFO_TYPE_SHIFT) != APFS_FILE_INFO_DATA_HASH)
		report("File info record", "undocumented type.");

	key->number = info_and_lba & APFS_FILE_INFO_LBA_MASK;
	key->name = NULL;

	if (key->number & (sb->s_blocksize - 1))
		report("File info record", "offset isn't multiple of block size.");
}

/**
 * read_sibling_link_key - Parse an on-disk sibling link key and check its
 *			   consistency
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
static void read_sibling_link_key(void *raw, int size, struct key *key)
{
	struct apfs_sibling_link_key *raw_key;

	if (size != sizeof(struct apfs_sibling_link_key))
		report("Siblink link record", "wrong size of key.");
	raw_key = raw;

	key->number = le64_to_cpu(raw_key->sibling_id); /* Only guessing */
	key->name = NULL;
}

/**
 * read_cat_key - Parse an on-disk catalog key
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
void read_cat_key(void *raw, int size, struct key *key)
{
	if (size < sizeof(struct apfs_key_header))
		report("Catalog tree", "key is too small.");
	key->id = cat_cnid((struct apfs_key_header *)raw);
	key->type = cat_type((struct apfs_key_header *)raw);

	if (!key->type || key->type > APFS_TYPE_MAX_VALID)
		report("Catalog tree", "invalid key type.");

	switch (key->type) {
	case APFS_TYPE_DIR_REC:
		read_dir_rec_key(raw, size, key);
		return;
	case APFS_TYPE_XATTR:
		read_xattr_key(raw, size, key);
		return;
	case APFS_TYPE_FILE_EXTENT:
		read_file_extent_key(raw, size, key);
		return;
	case APFS_TYPE_FILE_INFO:
		read_file_info_key(raw, size, key);
		return;
	case APFS_TYPE_SIBLING_LINK:
		read_sibling_link_key(raw, size, key);
		return;
	case APFS_TYPE_EXTENT:
		report("Catalog tree", "has extent reference record.");
	case APFS_TYPE_SNAP_METADATA:
		report("Catalog tree", "has snapshot metadata record.");
	case APFS_TYPE_SNAP_NAME:
		report("Catalog tree", "has snapshot name record.");
	default:
		/* All other key types are just the header */
		if (size != sizeof(struct apfs_key_header))
			report("Catalog tree record", "wrong size of key.");
		key->number = 0;
		key->name = NULL;
		return;
	}
}

/**
 * read_fext_key - Parse an on-disk fext key
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
void read_fext_key(void *raw, int size, struct key *key)
{
	struct apfs_fext_tree_key *raw_key;

	if (size != sizeof(*raw_key))
		report("File extents tree", "wrong size of key.");
	raw_key = raw;

	key->id = le64_to_cpu(raw_key->private_id);
	key->type = 0;
	key->number = le64_to_cpu(raw_key->logical_addr);
	key->name = NULL;

	if (key->number & (sb->s_blocksize - 1))
		report("Fext record", "offset isn't multiple of block size.");
}

/**
 * read_extentref_key - Parse an on-disk extent reference key
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
void read_extentref_key(void *raw, int size, struct key *key)
{
	int type;

	if (size != sizeof(struct apfs_phys_ext_key))
		report("Extent reference tree", "wrong size of key.");

	type = cat_type((struct apfs_key_header *)raw);
	if (type != APFS_TYPE_EXTENT)
		report("Extent reference tree", "wrong record type.");

	key->id = cat_cnid((struct apfs_key_header *)raw);
	key->type = type;
	key->number = 0;
	key->name = NULL;
}

/**
 * read_snap_key - Parse an on-disk key from the snapshot metadata tree
 * @raw:	pointer to the raw key
 * @size:	size of the raw key
 * @key:	key structure to store the result
 */
void read_snap_key(void *raw, int size, struct key *key)
{
	if (size < sizeof(struct apfs_key_header))
		report("Snapshot metadata tree", "key is too small.");
	key->id = cat_cnid((struct apfs_key_header *)raw);
	key->type = cat_type((struct apfs_key_header *)raw);

	switch (key->type) {
	case APFS_TYPE_SNAP_METADATA:
		if (size != sizeof(struct apfs_key_header))
			report("Snapshot metadata record", "wrong size of key.");
		key->number = 0;
		key->name = NULL;
		return;
	case APFS_TYPE_SNAP_NAME:
		if (key->id != (~0ULL & APFS_OBJ_ID_MASK))
			report("Snapshot name record", "invalid key header.");
		return read_snap_name_key(raw, size, key);
	default:
		report("Snapshot metadata tree", "invalid key type.");
	}
}

void read_omap_snap_key(void *raw, int size, struct key *key)
{
	__le64 *xid;

	if (size != sizeof(*xid))
		report("Omap snapshot tree", "wrong size of key.");
	xid = raw;

	key->id = le64_to_cpu(*xid);
	key->type = 0;
	key->number = 0;
	key->name = NULL;
}
