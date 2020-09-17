// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#ifndef _XR_VERSIONS_H
#define _XR_VERSIONS_H

#ifndef EXTERN
#define EXTERN extern
#endif /* EXTERN */

/*
 * filesystem feature global vars, set to 1 if the feature
 * is on, 0 otherwise
 */

extern int		fs_attributes;
extern int		fs_attributes2;
extern int		fs_inode_nlink;
extern int		fs_quotas;
extern int		fs_aligned_inodes;
extern int		fs_sb_feature_bits;
extern int		fs_has_extflgbit;

/*
 * inode chunk alignment, fsblocks
 */

extern xfs_extlen_t	fs_ino_alignment;

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
