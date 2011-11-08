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

#ifndef _XR_VERSIONS_H
#define _XR_VERSIONS_H

#ifndef EXTERN
#define EXTERN extern
#endif /* EXTERN */

/*
 * possible XFS filesystem features
 *
 * attributes					(6.2)
 * inode version 2 (32-bit link counts)		(6.2)
 * quotas					(6.2+)
 * aligned inodes				(6.2+)
 *
 * bitmask fields happend after 6.2.
 */

/*
 * filesystem feature global vars, set to 1 if the feature
 * is *allowed*, 0 otherwise.  These can be set via command-line
 * options
 */

EXTERN int		fs_attributes_allowed;
EXTERN int		fs_attributes2_allowed;
EXTERN int		fs_inode_nlink_allowed;
EXTERN int		fs_quotas_allowed;
EXTERN int		fs_aligned_inodes_allowed;
EXTERN int		fs_sb_feature_bits_allowed;
EXTERN int		fs_has_extflgbit_allowed;
EXTERN int		fs_shared_allowed;

/*
 * filesystem feature global vars, set to 1 if the feature
 * is on, 0 otherwise
 */

EXTERN int		fs_attributes;
EXTERN int		fs_attributes2;
EXTERN int		fs_inode_nlink;
EXTERN int		fs_quotas;
EXTERN int		fs_aligned_inodes;
EXTERN int		fs_sb_feature_bits;
EXTERN int		fs_has_extflgbit;
EXTERN int		fs_shared;

/*
 * inode chunk alignment, fsblocks
 */

EXTERN xfs_extlen_t	fs_ino_alignment;

/*
 * modify superblock to reflect current state of global fs
 * feature vars above
 */
void			update_sb_version(xfs_mount_t *mp);

/*
 * parse current sb to set above feature vars
 */
int			parse_sb_version(xfs_sb_t *sb);

#endif /* _XR_VERSIONS_H */
