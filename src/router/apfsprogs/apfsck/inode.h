/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _INODE_H
#define _INODE_H

#include <sys/stat.h> /* The macros for the inode mode */
#include <apfs/types.h>
#include "htable.h"

struct apfs_inode_key;
struct apfs_sibling_link_key;
struct apfs_sibling_map_key;

/* Flags for the bitmap of seen system xattrs (i_xattr_bmap) */
#define XATTR_BMAP_SYMLINK	0x01	/* Symlink target xattr */
#define XATTR_BMAP_RSRC_FORK	0x02	/* Resource fork xattr */
#define XATTR_BMAP_SECURITY	0x04	/* Security xattr */
#define XATTR_BMAP_FINDER_INFO	0x08	/* Finder info xattr */
#define XATTR_BMAP_COMPRESSED	0x10	/* Compression header xattr */

/*
 * Inode data in memory
 */
struct inode {
	struct htable_entry i_htable; /* Hash table entry header */

	u64		i_private_id;	/* Id of the inode's data stream */
	bool		i_seen;		/* Has this inode been seen? */

	/* Inode information read from its record (or from its dentries) */
	u16		i_mode;		/* File mode */
	union {
		u32	i_nchildren;	/* Number of children of directory */
		u32	i_nlink;	/* Number of hard links to file */
	};
	u64		i_sparse_bytes;	/* Number of sparse bytes */
	u64		i_flags;	/* Internal flags */
	u32		i_rdev;		/* Device ID */
	char		*i_name;	/* Name of primary link */
	u64		i_parent_id;	/* Parent id for the primary link */
	struct dstream	*i_dstream;	/* The inode's dstream (can be NULL) */
	u64		i_purg_flags;	/* Inode purgeable flags */
	char		*i_purg_name;	/* Purgeable dentry name (can be NULL) */
	u32		i_owner;	/* Id of the owner user */
	struct dirstat	*i_dirstat;	/* Directory statistics (can be NULL) */
	struct compress	*i_compress;	/* Compression data (can be NULL) */

	/* Inode stats measured by the fsck */
	u8		i_xattr_bmap;	/* Bitmap of system xattrs for inode */
	u16		i_xfield_bmap;	/* Bitmap of xfields for inode */
	u32		i_child_count;	/* Number of children of directory */
	u32		i_link_count;	/* Number of dentries for file */
	char		*i_first_name;	/* Name of first dentry encountered */
	u64		i_first_parent;	/* Parent id of the first dentry seen */
	struct sibling	*i_siblings;	/* Linked list of siblings for inode */
};
#define i_ino	i_htable.h_id		/* Inode number */

/*
 * Sibling link data in memory
 */
struct sibling {
	struct sibling	*s_next;	/* Next sibling in linked list */
	u64		s_id;		/* Sibling id */
	bool		s_checked;	/* Has this sibling been checked? */
	bool		s_mapped;	/* Has the sibling map been seen? */

	u64		s_parent_ino;	/* Inode number for parent */
	u16		s_name_len;	/* Name length */
	u8		*s_name;	/* In-memory copy of the name */
};

extern void free_inode_table(struct htable_entry **table);
extern struct inode *get_inode(u64 ino);
extern void check_inode_ids(u64 ino, u64 parent_ino);
extern void parse_inode_record(struct apfs_inode_key *key,
			       struct apfs_inode_val *val, int len);
extern struct sibling *get_sibling(u64 id, struct inode *inode);
extern void set_or_check_sibling(u64 parent_id, int namelen, u8 *name,
				 struct sibling *sibling);
extern void parse_sibling_record(struct apfs_sibling_link_key *key,
				 struct apfs_sibling_val *val, int len);
extern void parse_sibling_map_record(struct apfs_sibling_map_key *key,
				     struct apfs_sibling_map_val *val, int len);
extern void check_xfield_flags(u8 flags);

#endif	/* _INODE_H */
