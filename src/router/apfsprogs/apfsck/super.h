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

struct volume_superblock {
	struct apfs_superblock *v_raw;
	struct btree *v_omap;
	struct btree *v_cat;
	struct btree *v_extent_ref;
	struct btree *v_snap_meta;
	struct htable_entry **v_omap_table;	/* Hash table of omap records */
	struct htable_entry **v_inode_table;	/* Hash table of all inodes */
	struct htable_entry **v_dstream_table;	/* Hash table of all dstreams */
	struct htable_entry **v_cnid_table;	/* Hash table of all cnids */
	struct htable_entry **v_extent_table;	/* Hash table of all extents */

	/* Volume stats as measured by the fsck */
	u64 v_file_count;	/* Number of files */
	u64 v_dir_count;	/* Number of directories */
	u64 v_symlink_count;	/* Number of symlinks */
	u64 v_special_count;	/* Number of other filesystem objects */
	u64 v_block_count;	/* Number of blocks currently allocated */
	bool v_has_root;	/* Is there a root directory? */
	bool v_has_priv;	/* Is there a private directory? */

	/* Volume information read from the on-disk structure */
	u64 v_first_xid;	/* Transaction that created the volume */
	u64 v_next_obj_id;	/* Next cnid to be assigned */
	u32 v_next_doc_id;	/* Next document identifier to be assigned */

	struct object v_obj;		/* Object holding the volume sb */
};

/* Superblock data in memory */
struct super_block {
	struct apfs_nx_superblock *s_raw;
	void *s_bitmap;	/* Allocation bitmap for the whole container */
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

	/* Hash table of ephemeral object mappings for the checkpoint */
	struct htable_entry **s_cpoint_map_table;
	/* Hash table of virtual object mappings for the container */
	struct htable_entry **s_omap_table;

	struct spaceman s_spaceman; /* Information about the space manager */

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

extern void parse_filesystem(void);

#endif	/* _SUPER_H */
