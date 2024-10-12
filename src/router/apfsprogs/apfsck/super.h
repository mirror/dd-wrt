/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _SUPER_H
#define _SUPER_H

#include <apfs/raw.h>
#include <apfs/types.h>
#include "htable.h"
#include "object.h"
#include "spaceman.h"

struct volume_group {
	char vg_id[16];
	bool vg_system_seen;
	bool vg_data_seen;
};

/**
 * An entry in a linked list of b-trees
 */
struct listed_btree {
	struct btree *btree;
	struct listed_btree *next;
};

struct volume_superblock {
	struct apfs_superblock *v_raw;
	struct btree *v_omap;
	struct btree *v_cat;
	struct btree *v_extent_ref;
	struct listed_btree *v_snap_extrefs;	/* Snapshots have their own */
	struct btree *v_snap_meta;
	struct btree *v_snapshots;
	struct btree *v_fext;
	struct htable_entry **v_omap_table;	/* Hash table of omap records */
	struct htable_entry **v_inode_table;	/* Hash table of all inodes */
	struct htable_entry **v_dstream_table;	/* Hash table of all dstreams */
	struct htable_entry **v_cnid_table;	/* Hash table of all cnids */
	struct htable_entry **v_extent_table;	/* Hash table of all extents */
	struct htable_entry **v_snap_table;	/* Hash table of all snapshots */
	struct htable_entry **v_dirstat_table;	/* Hash table of all dir stats */
	struct htable_entry **v_crypto_table;	/* Hash table of all crypto states */

	bool v_in_snapshot;			/* Is this a snapshot volume? */

	/* Volume stats as measured by the fsck */
	u64 v_file_count;	/* Number of files */
	u64 v_dir_count;	/* Number of directories */
	u64 v_symlink_count;	/* Number of symlinks */
	u64 v_special_count;	/* Number of other filesystem objects */
	u64 v_block_count;	/* Number of blocks currently allocated */
	u64 v_snap_count;	/* Number of snapshots */
	u64 v_snap_max_xid;	/* Maximum xid among snapshots */
	bool v_has_root;	/* Is there a root directory? */
	bool v_has_priv;	/* Is there a private directory? */

	/* Volume information read from the on-disk structure */
	u64 v_extref_oid;	/* Object id for the extent reference tree */
	u64 v_omap_oid;		/* Object id for object map tree */
	u64 v_snap_meta_oid;	/* Object id for the snapshot metadata tree */
	u64 v_fext_tree_oid;	/* Object id for the fext tree */
	u64 v_integrity_oid;	/* Object id for the integrity metadata */
	u64 v_first_xid;	/* Transaction that created the volume */
	u64 v_last_xid;		/* Transaction that last modified the volume */
	u64 v_next_obj_id;	/* Next cnid to be assigned */
	u32 v_next_doc_id;	/* Next document identifier to be assigned */
	u32 v_index;		/* Index in the container's volume array */
	bool v_encrypted;	/* Is the volume encrypted? */
	u8 v_hash[32];		/* For a sealed volume, the root SHA-256 */

	struct object v_obj;		/* Object holding the volume sb */
};

/* Superblock data in memory */
struct super_block {
	struct apfs_nx_superblock *s_raw;
	void *s_bitmap;	/* Allocation bitmap for the whole container */
	void *s_ip_bitmap; /* Allocation bitmap for the internal pool */
	struct btree *s_omap;
	struct object *s_reaper;
	unsigned long s_blocksize;
	unsigned char s_blocksize_bits;
	u64 s_block_count; /* Number of blocks in the container */
	u64 s_xid; /* Transaction id for the superblock */
	u64 s_next_oid;	/* Next virtual object id to be used */
	u32 s_max_vols; /* Maximum number of volumes allowed */
	u64 s_data_base; /* Base address of the checkpoint data area */
	u32 s_data_blocks; /* Number of blocks in the checkpoint data area */
	u32 s_data_index; /* Index of first valid block in checkpoint data */
	u32 s_data_len; /* Number of valid blocks in checkpoint data area */
	u64 s_reaper_fs_id; /* Volume id reported by the reaper */

	/* Hash table of ephemeral object mappings for the checkpoint */
	struct htable_entry **s_cpoint_map_table;
	/* Hash table of virtual object mappings for the container */
	struct htable_entry **s_omap_table;

	struct spaceman s_spaceman; /* Information about the space manager */

	/* Information about the one volume group in the container, if any */
	struct volume_group *s_volume_group;

	/* This is excessive in most cases.  TODO: switch to a linked list? */
	struct volume_superblock *s_volumes[APFS_NX_MAX_FILE_SYSTEMS];
};

/*
 * Checkpoint mapping data in memory
 */
struct cpoint_map {
	struct htable_entry m_htable; /* Hash table entry header */

	bool	m_seen;		/* Has this ephemeral oid been seen in use? */
	u32	m_type;		/* Type of the object */
	u32	m_subtype;	/* Subtype of the object */
	u64	m_paddr;	/* Physical address of the object */
	u32	m_size;		/* Size of the object in bytes */
};
#define m_oid	m_htable.h_id	/* Ephemeral object id */

static inline bool apfs_is_case_insensitive(void)
{
	extern struct volume_superblock *vsb;

	return (vsb->v_raw->apfs_incompatible_features &
		cpu_to_le64(APFS_INCOMPAT_CASE_INSENSITIVE)) != 0;
}

static inline bool apfs_is_normalization_insensitive(void)
{
	extern struct volume_superblock *vsb;
	u64 flags = le64_to_cpu(vsb->v_raw->apfs_incompatible_features);

	if (apfs_is_case_insensitive())
		return true;
	if (flags & APFS_INCOMPAT_NORMALIZATION_INSENSITIVE)
		return true;
	return false;
}

static inline bool apfs_volume_is_sealed(void)
{
	u64 flags = le64_to_cpu(vsb->v_raw->apfs_incompatible_features);

	return flags & APFS_INCOMPAT_SEALED_VOLUME;
}

static inline bool apfs_volume_has_extent_prealloc_flag(void)
{
	u64 flags = le64_to_cpu(vsb->v_raw->apfs_incompatible_features);

	return flags & APFS_INCOMPAT_EXTENT_PREALLOC_FLAG;
}

static inline bool apfs_volume_is_in_group(void)
{
	u64 features = le64_to_cpu(vsb->v_raw->apfs_features);

	return features & APFS_FEATURE_VOLGRP_SYSTEM_INO_SPACE;
}

static inline u16 apfs_volume_role(void)
{
	return le16_to_cpu(vsb->v_raw->apfs_role);
}

static inline bool apfs_is_data_volume_in_group(void)
{
	u16 role = apfs_volume_role();

	return apfs_volume_is_in_group() && role == APFS_VOL_ROLE_DATA;
}

static inline bool apfs_is_system_volume_in_group(void)
{
	u16 role = apfs_volume_role();

	return apfs_volume_is_in_group() && role == APFS_VOL_ROLE_SYSTEM;
}

/**
 * uuid_is_null - Check if all bytes of a uuid are zero
 * @uuid: the uuid to check
 *
 * TODO: reuse this for other uuid checks
 */
static inline bool uuid_is_null(char uuid[16])
{
	int i;

	for (i = 0; i < 16; ++i) {
		if (uuid[i])
			return false;
	}
	return true;
}

extern void parse_filesystem(void);
extern struct volume_superblock *alloc_volume_super(bool snap);
extern void read_volume_super(int vol, struct volume_superblock *vsb, struct object *obj);
extern void check_volume_super(void);

#endif	/* _SUPER_H */
