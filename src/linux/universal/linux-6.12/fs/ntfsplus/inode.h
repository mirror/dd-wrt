/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Defines for inode structures NTFS Linux kernel driver. Part of
 * the Linux-NTFS project.
 *
 * Copyright (c) 2001-2007 Anton Altaparmakov
 * Copyright (c) 2002 Richard Russon
 * Copyright (c) 2025 LG Electronics Co., Ltd.
 */

#ifndef _LINUX_NTFS_INODE_H
#define _LINUX_NTFS_INODE_H

#include "misc.h"

enum ntfs_inode_mutex_lock_class {
	NTFS_INODE_MUTEX_PARENT,
	NTFS_INODE_MUTEX_NORMAL,
	NTFS_INODE_MUTEX_PARENT_2,
	NTFS_INODE_MUTEX_NORMAL_2,
	NTFS_REPARSE_MUTEX_PARENT,
	NTFS_EA_MUTEX_NORMAL
};

/*
 * The NTFS in-memory inode structure. It is just used as an extension to the
 * fields already provided in the VFS inode.
 */
struct ntfs_inode {
	rwlock_t size_lock;	/* Lock serializing access to inode sizes. */
	unsigned long state;	/*
				 * NTFS specific flags describing this inode.
				 * See ntfs_inode_state_bits below.
				 */
	__le32 flags;		/* Flags describing the file. (Copy from STANDARD_INFORMATION) */
	unsigned long mft_no;	/* Number of the mft record / inode. */
	u16 seq_no;		/* Sequence number of the mft record. */
	atomic_t count;		/* Inode reference count for book keeping. */
	struct ntfs_volume *vol; /* Pointer to the ntfs volume of this inode. */

	/*
	 * If NInoAttr() is true, the below fields describe the attribute which
	 * this fake inode belongs to. The actual inode of this attribute is
	 * pointed to by base_ntfs_ino and nr_extents is always set to -1 (see
	 * below). For real inodes, we also set the type (AT_DATA for files and
	 * AT_INDEX_ALLOCATION for directories), with the name = NULL and
	 * name_len = 0 for files and name = I30 (global constant) and
	 * name_len = 4 for directories.
	 */
	__le32 type;		/* Attribute type of this fake inode. */
	__le16 *name;		/* Attribute name of this fake inode. */
	u32 name_len;		/* Attribute name length of this fake inode. */
	struct runlist runlist;	/*
				 * If state has the NI_NonResident bit set,
				 * the runlist of the unnamed data attribute
				 * (if a file) or of the index allocation
				 * attribute (directory) or of the attribute
				 * described by the fake inode (if NInoAttr()).
				 * If runlist.rl is NULL, the runlist has not
				 * been read in yet or has been unmapped. If
				 * NI_NonResident is clear, the attribute is
				 * resident (file and fake inode) or there is
				 * no $I30 index allocation attribute
				 * (small directory). In the latter case
				 * runlist.rl is always NULL.
				 */
	s64 lcn_seek_trunc;

	s64 data_size;		/* Copy from the attribute record. */
	s64 initialized_size;	/* Copy from the attribute record. */
	s64 allocated_size;	/* Copy from the attribute record. */

	struct timespec64 i_crtime;

	/*
	 * The following fields are only valid for real inodes and extent
	 * inodes.
	 */
	void *mrec;
	struct mutex mrec_lock;	/*
				 * Lock for serializing access to the
				 * mft record belonging to this inode.
				 */
	struct folio *folio;	/*
				 * The folio containing the mft record of the
				 * inode. This should only be touched by the
				 * (un)map_mft_record*() functions.
				 */
	int folio_ofs;		/*
				 * Offset into the folio at which the mft record
				 * begins. This should only be touched by the
				 * (un)map_mft_record*() functions.
				 */
	s64 mft_lcn[2];		/* s64 number containing the mft record */
	unsigned int mft_lcn_count;

	/*
	 * Attribute list support (only for use by the attribute lookup
	 * functions). Setup during read_inode for all inodes with attribute
	 * lists. Only valid if NI_AttrList is set in state.
	 */
	u32 attr_list_size;	/* Length of attribute list value in bytes. */
	u8 *attr_list;		/* Attribute list value itself. */

	union {
		struct { /* It is a directory, $MFT, or an index inode. */
			u32 block_size;		/* Size of an index block. */
			u32 vcn_size;		/* Size of a vcn in this index. */
			__le32 collation_rule;	/* The collation rule for the index. */
			u8 block_size_bits;	/* Log2 of the above. */
			u8 vcn_size_bits;	/* Log2 of the above. */
		} index;
		struct { /* It is a compressed/sparse file/attribute inode. */
			s64 size;		/* Copy of compressed_size from $DATA. */
			u32 block_size;		/* Size of a compression block (cb). */
			u8 block_size_bits;	/* Log2 of the size of a cb. */
			u8 block_clusters;	/* Number of clusters per cb. */
		} compressed;
	} itype;
	struct mutex extent_lock;	/* Lock for accessing/modifying the below . */
	s32 nr_extents;	/*
			 * For a base mft record, the number of attached extent\
			 * inodes (0 if none), for extent records and for fake
			 * inodes describing an attribute this is -1.
			 */
	union {		/* This union is only used if nr_extents != 0. */
		struct ntfs_inode **extent_ntfs_inos;	/*
							 * For nr_extents > 0, array of
							 * the ntfs inodes of the extent
							 * mft records belonging to
							 * this base inode which have
							 * been loaded.
							 */
		struct ntfs_inode *base_ntfs_ino;	/*
							 * For nr_extents == -1, the
							 * ntfs inode of the base mft
							 * record. For fake inodes, the
							 * real (base) inode to which
							 * the attribute belongs.
							 */
	} ext;

	unsigned int i_dealloc_clusters;
	char *target;
};

/*
 * Defined bits for the state field in the ntfs_inode structure.
 * (f) = files only, (d) = directories only, (a) = attributes/fake inodes only
 */
enum {
	NI_Dirty,		/* 1: Mft record needs to be written to disk. */
	NI_AttrListDirty,	/* 1: Mft record contains an attribute list. */
	NI_AttrList,		/* 1: Mft record contains an attribute list. */
	NI_AttrListNonResident,	/*
				 * 1: Attribute list is non-resident. Implies
				 *    NI_AttrList is set.
				 */

	NI_Attr,		/*
				 * 1: Fake inode for attribute i/o.
				 * 0: Real inode or extent inode.
				 */

	NI_MstProtected,	/*
				 * 1: Attribute is protected by MST fixups.
				 * 0: Attribute is not protected by fixups.
				 */
	NI_NonResident,		/*
				 * 1: Unnamed data attr is non-resident (f).
				 * 1: Attribute is non-resident (a).
				 */
	NI_IndexAllocPresent,	/* 1: $I30 index alloc attr is present (d). */
	NI_Compressed,		/*
				 * 1: Unnamed data attr is compressed (f).
				 * 1: Create compressed files by default (d).
				 * 1: Attribute is compressed (a).
				 */
	NI_Encrypted,		/*
				 * 1: Unnamed data attr is encrypted (f).
				 * 1: Create encrypted files by default (d).
				 * 1: Attribute is encrypted (a).
				 */
	NI_Sparse,		/*
				 * 1: Unnamed data attr is sparse (f).
				 * 1: Create sparse files by default (d).
				 * 1: Attribute is sparse (a).
				 */
	NI_SparseDisabled,	/* 1: May not create sparse regions. */
	NI_FullyMapped,
	NI_FileNameDirty,
	NI_BeingDeleted,
	NI_BeingCreated,
	NI_HasEA,
	NI_RunlistDirty,
};

/*
 * NOTE: We should be adding dirty mft records to a list somewhere and they
 * should be independent of the (ntfs/vfs) inode structure so that an inode can
 * be removed but the record can be left dirty for syncing later.
 */

/*
 * Macro tricks to expand the NInoFoo(), NInoSetFoo(), and NInoClearFoo()
 * functions.
 */
#define NINO_FNS(flag)						\
static inline int NIno##flag(struct ntfs_inode *ni)		\
{								\
	return test_bit(NI_##flag, &(ni)->state);		\
}								\
static inline void NInoSet##flag(struct ntfs_inode *ni)		\
{								\
	set_bit(NI_##flag, &(ni)->state);			\
}								\
static inline void NInoClear##flag(struct ntfs_inode *ni)	\
{								\
	clear_bit(NI_##flag, &(ni)->state);			\
}

/*
 * As above for NInoTestSetFoo() and NInoTestClearFoo().
 */
#define TAS_NINO_FNS(flag)						\
static inline int NInoTestSet##flag(struct ntfs_inode *ni)		\
{									\
	return test_and_set_bit(NI_##flag, &(ni)->state);		\
}									\
static inline int NInoTestClear##flag(struct ntfs_inode *ni)		\
{									\
	return test_and_clear_bit(NI_##flag, &(ni)->state);		\
}

/* Emit the ntfs inode bitops functions. */
NINO_FNS(Dirty)
TAS_NINO_FNS(Dirty)
NINO_FNS(AttrList)
NINO_FNS(AttrListDirty)
NINO_FNS(AttrListNonResident)
NINO_FNS(Attr)
NINO_FNS(MstProtected)
NINO_FNS(NonResident)
NINO_FNS(IndexAllocPresent)
NINO_FNS(Compressed)
NINO_FNS(Encrypted)
NINO_FNS(Sparse)
NINO_FNS(SparseDisabled)
NINO_FNS(FullyMapped)
NINO_FNS(FileNameDirty)
TAS_NINO_FNS(FileNameDirty)
NINO_FNS(BeingDeleted)
NINO_FNS(HasEA)
NINO_FNS(RunlistDirty)

/*
 * The full structure containing a ntfs_inode and a vfs struct inode. Used for
 * all real and fake inodes but not for extent inodes which lack the vfs struct
 * inode.
 */
struct big_ntfs_inode {
	struct ntfs_inode ntfs_inode;
	struct inode vfs_inode;		/* The vfs inode structure. */
};

/**
 * NTFS_I - return the ntfs inode given a vfs inode
 * @inode:	VFS inode
 *
 * NTFS_I() returns the ntfs inode associated with the VFS @inode.
 */
static inline struct ntfs_inode *NTFS_I(struct inode *inode)
{
	return (struct ntfs_inode *)container_of(inode, struct big_ntfs_inode, vfs_inode);
}

static inline struct inode *VFS_I(struct ntfs_inode *ni)
{
	return &((struct big_ntfs_inode *)ni)->vfs_inode;
}

/**
 * ntfs_attr - ntfs in memory attribute structure
 *
 * This structure exists only to provide a small structure for the
 * ntfs_{attr_}iget()/ntfs_test_inode()/ntfs_init_locked_inode() mechanism.
 *
 * NOTE: Elements are ordered by size to make the structure as compact as
 * possible on all architectures.
 */
struct ntfs_attr {
	unsigned long mft_no;
	__le16 *name;
	u32 name_len;
	__le32 type;
	unsigned long state;
};

int ntfs_test_inode(struct inode *vi, void *data);
struct inode *ntfs_iget(struct super_block *sb, unsigned long mft_no);
struct inode *ntfs_attr_iget(struct inode *base_vi, __le32 type,
		__le16 *name, u32 name_len);
struct inode *ntfs_index_iget(struct inode *base_vi, __le16 *name,
		u32 name_len);
struct inode *ntfs_alloc_big_inode(struct super_block *sb);
void ntfs_free_big_inode(struct inode *inode);
int ntfs_drop_big_inode(struct inode *inode);
void ntfs_evict_big_inode(struct inode *vi);
void __ntfs_init_inode(struct super_block *sb, struct ntfs_inode *ni);

static inline void ntfs_init_big_inode(struct inode *vi)
{
	struct ntfs_inode *ni = NTFS_I(vi);

	ntfs_debug("Entering.");
	__ntfs_init_inode(vi->i_sb, ni);
	ni->mft_no = vi->i_ino;
}

struct ntfs_inode *ntfs_new_extent_inode(struct super_block *sb,
		unsigned long mft_no);
void ntfs_clear_extent_inode(struct ntfs_inode *ni);
int ntfs_read_inode_mount(struct inode *vi);
int ntfs_show_options(struct seq_file *sf, struct dentry *root);
int ntfs_truncate_vfs(struct inode *vi, loff_t new_size, loff_t i_size);

int ntfsp_setattr(struct mnt_idmap *idmap, struct dentry *dentry,
		 struct iattr *attr);
int ntfsp_getattr(struct mnt_idmap *idmap, const struct path *path,
		struct kstat *stat, unsigned int request_mask,
		unsigned int query_flags);

int __ntfs_write_inode(struct inode *vi, int sync);
int ntfs_inode_attach_all_extents(struct ntfs_inode *ni);
int ntfs_inode_add_attrlist(struct ntfs_inode *ni);
void ntfs_destroy_ext_inode(struct ntfs_inode *ni);
int ntfs_inode_free_space(struct ntfs_inode *ni, int size);
s64 ntfs_inode_attr_pread(struct inode *vi, s64 pos, s64 count, u8 *buf);
s64 ntfs_inode_attr_pwrite(struct inode *vi, s64 pos, s64 count, u8 *buf,
		bool sync);
int ntfs_inode_close(struct ntfs_inode *ni);

static inline void ntfs_commit_inode(struct inode *vi)
{
	__ntfs_write_inode(vi, 1);
}

int ntfs_inode_sync_filename(struct ntfs_inode *ni);
int ntfs_extend_initialized_size(struct inode *vi, const loff_t offset,
		const loff_t new_size);
void ntfs_set_vfs_operations(struct inode *inode, mode_t mode, dev_t dev);

#endif /* _LINUX_NTFS_INODE_H */
