/*
 * Copyright (c) 2000,2005 Silicon Graphics, Inc.
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
#ifndef __XFS_DIR_SF_H__
#define	__XFS_DIR_SF_H__

/*
 * Directory layout when stored internal to an inode.
 *
 * Small directories are packed as tightly as possible so as to
 * fit into the literal area of the inode.
 */

typedef struct { __uint8_t i[sizeof(xfs_ino_t)]; } xfs_dir_ino_t;

/*
 * The parent directory has a dedicated field, and the self-pointer must
 * be calculated on the fly.
 *
 * Entries are packed toward the top as tight as possible.  The header
 * and the elements much be memcpy'd out into a work area to get correct
 * alignment for the inode number fields.
 */
typedef struct xfs_dir_sf_hdr {		/* constant-structure header block */
	xfs_dir_ino_t	parent;		/* parent dir inode number */
	__u8		count;		/* count of active entries */
} xfs_dir_sf_hdr_t;

typedef struct xfs_dir_sf_entry {
	xfs_dir_ino_t	inumber;	/* referenced inode number */
	__u8		namelen;	/* actual length of name (no NULL) */
	__u8		name[1];	/* name */
} xfs_dir_sf_entry_t;

typedef struct xfs_dir_shortform {
	xfs_dir_sf_hdr_t	hdr;
	xfs_dir_sf_entry_t	list[1];	/* variable sized array */
} xfs_dir_shortform_t;

/*
 * We generate this then sort it, so that readdirs are returned in
 * hash-order.  Else seekdir won't work.
 */
typedef struct xfs_dir_sf_sort {
	__u8		entno;		/* .=0, ..=1, else entry# + 2 */
	__u8		seqno;		/* sequence # with same hash value */
	__u8		namelen;	/* length of name value (no null) */
	__be32		hash;		/* this entry's hash value */
	xfs_intino_t	ino;		/* this entry's inode number */
	__u8		*name;		/* name value, pointer into buffer */
} xfs_dir_sf_sort_t;

static inline void xfs_dir_sf_get_dirino(xfs_dir_ino_t *from, xfs_ino_t *to)
{
	*to = XFS_GET_DIR_INO8(*from);
}

static inline void xfs_dir_sf_put_dirino(xfs_ino_t *from, xfs_dir_ino_t *to)
{
	XFS_PUT_DIR_INO8(*from, *to);
}

static inline int xfs_dir_sf_entsize_byname(int len)
{
	return sizeof(xfs_dir_sf_entry_t) - 1 + len;
}

static inline int xfs_dir_sf_entsize_byentry(xfs_dir_sf_entry_t *sfep)
{
	return sizeof(xfs_dir_sf_entry_t) - 1 + sfep->namelen;
}

static inline xfs_dir_sf_entry_t *xfs_dir_sf_nextentry(xfs_dir_sf_entry_t *sfep)
{
	return (xfs_dir_sf_entry_t *)((char *)sfep +
					xfs_dir_sf_entsize_byentry(sfep));
}

static inline int xfs_dir_sf_allfit(int count, int totallen)
{
	return sizeof(xfs_dir_sf_hdr_t) +
			(sizeof(xfs_dir_sf_entry_t) - 1) * count + totallen;
}

#endif	/* __XFS_DIR_SF_H__ */
