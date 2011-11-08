/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef __XFS_DIR_LEAF_H__
#define	__XFS_DIR_LEAF_H__

/*
 * Version 1 Directory layout, internal structure, access macros, etc.
 * to allow various xfsprogs tools to read these structures.
 *
 * Large directories are structured around Btrees where all the data
 * elements are in the leaf nodes.  Filenames are hashed into an int,
 * then that int is used as the index into the Btree.  Since the hashval
 * of a filename may not be unique, we may have duplicate keys.  The
 * internal links in the Btree are logical block offsets into the file.
 */

/*========================================================================
 * Directory Structure when equal to XFS_LBSIZE(mp) bytes.
 *========================================================================*/

/*
 * This is the structure of the leaf nodes in the Btree.
 *
 * Struct leaf_entry's are packed from the top.  Names grow from the bottom
 * but are not packed.  The freemap contains run-length-encoded entries
 * for the free bytes after the leaf_entry's, but only the N largest such,
 * smaller runs are dropped.  When the freemap doesn't show enough space
 * for an allocation, we compact the namelist area and try again.  If we
 * still don't have enough space, then we have to split the block.
 *
 * Since we have duplicate hash keys, for each key that matches, compare
 * the actual string.  The root and intermediate node search always takes
 * the first-in-the-block key match found, so we should only have to work
 * "forw"ard.  If none matches, continue with the "forw"ard leaf nodes
 * until the hash key changes or the filename is found.
 *
 * The parent directory and the self-pointer are explicitly represented
 * (ie: there are entries for "." and "..").
 *
 * Note that the count being a __uint16_t limits us to something like a
 * blocksize of 1.3MB in the face of worst case (short) filenames.
 */

#define XFS_DIR_LEAF_MAGIC	0xfeeb

#define XFS_DIR_LEAF_MAPSIZE	3	/* how many freespace slots */


typedef struct xfs_dir_leaf_map {	/* RLE map of free bytes */
	__be16		base;	 	/* base of free region */
	__be16		size; 		/* run length of free region */
} xfs_dir_leaf_map_t;

typedef struct xfs_dir_leaf_hdr {	/* constant-structure header block */
	xfs_da_blkinfo_t info;		/* block type, links, etc. */
	__be16		count;		/* count of active leaf_entry's */
	__be16		namebytes;	/* num bytes of name strings stored */
	__be16		firstused;	/* first used byte in name area */
	__u8		holes;		/* != 0 if blk needs compaction */
	__u8		pad1;
	xfs_dir_leaf_map_t freemap[XFS_DIR_LEAF_MAPSIZE];
} xfs_dir_leaf_hdr_t;

typedef struct xfs_dir_leaf_entry {	/* sorted on key, not name */
	__be32		hashval;	/* hash value of name */
	__be16		nameidx;	/* index into buffer of name */
	__u8		namelen;	/* length of name string */
	__u8		pad2;
} xfs_dir_leaf_entry_t;

typedef struct xfs_dir_leaf_name {
	xfs_dir_ino_t	inumber;	/* inode number for this key */
	__u8		name[1];	/* name string itself */
} xfs_dir_leaf_name_t;

typedef struct xfs_dir_leafblock {
	xfs_dir_leaf_hdr_t	hdr;	/* constant-structure header block */
	xfs_dir_leaf_entry_t	entries[1];	/* var sized array */
	xfs_dir_leaf_name_t	namelist[1];	/* grows from bottom of buf */
} xfs_dir_leafblock_t;

static inline int xfs_dir_leaf_entsize_byname(int len)
{
	return (uint)sizeof(xfs_dir_leaf_name_t)-1 + len;
}

static inline int xfs_dir_leaf_entsize_byentry(xfs_dir_leaf_entry_t *entry)
{
	return (uint)sizeof(xfs_dir_leaf_name_t)-1 + (entry)->namelen;
}

static inline xfs_dir_leaf_name_t *
xfs_dir_leaf_namestruct(xfs_dir_leafblock_t *leafp, int offset)
{
	return (xfs_dir_leaf_name_t *)&((char *)(leafp))[offset];
}

#endif /* __XFS_DIR_LEAF_H__ */
