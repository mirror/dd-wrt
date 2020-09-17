// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"

#include "err_protos.h"
#include "globals.h"
#include "versions.h"

/*
 * filesystem feature global vars, set to 1 if the feature
 * is on, 0 otherwise
 */

int fs_attributes;
int fs_attributes2;
int fs_inode_nlink;
int fs_quotas;
int fs_aligned_inodes;
int fs_sb_feature_bits;
int fs_has_extflgbit;

/*
 * inode chunk alignment, fsblocks
 */

xfs_extlen_t	fs_ino_alignment;

void
update_sb_version(xfs_mount_t *mp)
{
	xfs_sb_t	*sb;

	sb = &mp->m_sb;

	if (fs_attributes && !xfs_sb_version_hasattr(sb))
		xfs_sb_version_addattr(sb);

	if (fs_attributes2 && !xfs_sb_version_hasattr2(sb))
		xfs_sb_version_addattr2(sb);

	/* V2 inode conversion is now always going to happen */
	if (!(sb->sb_versionnum & XFS_SB_VERSION_NLINKBIT))
		sb->sb_versionnum |= XFS_SB_VERSION_NLINKBIT;

	/*
	 * fix up the superblock version number and feature bits,
	 * turn off quota bits and flags if the filesystem doesn't
	 * have quotas.
	 */
	if (fs_quotas)  {
		if (!xfs_sb_version_hasquota(sb))
			xfs_sb_version_addquota(sb);

		/*
		 * protect against stray bits in the quota flag field
		 */
		if (sb->sb_qflags & ~XFS_MOUNT_QUOTA_ALL) {
			/*
			 * update the incore superblock, if we're in
			 * no_modify mode, it'll never get flushed out
			 * so this is ok.
			 */
			do_warn(_("bogus quota flags 0x%x set in superblock"),
				sb->sb_qflags & ~XFS_MOUNT_QUOTA_ALL);

			sb->sb_qflags &= XFS_MOUNT_QUOTA_ALL;

			if (!no_modify)
				do_warn(_(", bogus flags will be cleared\n"));
			else
				do_warn(_(", bogus flags would be cleared\n"));
		}
	} else  {
		sb->sb_qflags = 0;

		if (xfs_sb_version_hasquota(sb))  {
			lost_quotas = 1;
			sb->sb_versionnum &= ~XFS_SB_VERSION_QUOTABIT;
		}
	}

	if (!fs_aligned_inodes && xfs_sb_version_hasalign(sb))
		sb->sb_versionnum &= ~XFS_SB_VERSION_ALIGNBIT;
}

/*
 * returns 0 if things are fine, 1 if we don't understand
 * this superblock version.  Sets superblock geometry-dependent
 * global variables.
 */
int
parse_sb_version(xfs_sb_t *sb)
{
	fs_attributes = 0;
	fs_attributes2 = 0;
	fs_inode_nlink = 1;
	fs_quotas = 0;
	fs_aligned_inodes = 0;
	fs_sb_feature_bits = 0;
	fs_ino_alignment = 0;
	fs_has_extflgbit = 1;
	have_uquotino = 0;
	have_gquotino = 0;
	have_pquotino = 0;

	if (sb->sb_versionnum & XFS_SB_VERSION_SHAREDBIT) {
		do_warn(_("Shared Version bit set. Not supported. Ever.\n"));
		return 1;
	}

	/*
	 * ok, check to make sure that the sb isn't newer
	 * than we are
	 */
	if (!xfs_sb_good_version(sb))  {
		do_warn(_("WARNING:  unknown superblock version %d\n"),
			XFS_SB_VERSION_NUM(sb));
		do_warn(
_("This filesystem contains features not understood by this program.\n"));
		return(1);
	}

	if (XFS_SB_VERSION_NUM(sb) >= XFS_SB_VERSION_4)
		fs_sb_feature_bits = 1;

	/* Look for V5 feature flags we don't know about */
	if (XFS_SB_VERSION_NUM(sb) >= XFS_SB_VERSION_5 &&
	    (xfs_sb_has_compat_feature(sb, XFS_SB_FEAT_COMPAT_UNKNOWN) ||
	     xfs_sb_has_ro_compat_feature(sb, XFS_SB_FEAT_RO_COMPAT_UNKNOWN) ||
	     xfs_sb_has_incompat_feature(sb, XFS_SB_FEAT_INCOMPAT_UNKNOWN))) {
		do_warn(
_("Superblock has unknown compat/rocompat/incompat features (0x%x/0x%x/0x%x).\n"
  "Using a more recent xfs_repair is recommended.\n"),
			sb->sb_features_compat & XFS_SB_FEAT_COMPAT_UNKNOWN,
			sb->sb_features_ro_compat & XFS_SB_FEAT_RO_COMPAT_UNKNOWN,
			sb->sb_features_incompat & XFS_SB_FEAT_INCOMPAT_UNKNOWN);
		return 1;
	}

	if (xfs_sb_version_hasattr(sb))
		fs_attributes = 1;

	if (xfs_sb_version_hasattr2(sb))
		fs_attributes2 = 1;

	if (!(sb->sb_versionnum & XFS_SB_VERSION_NLINKBIT)) {
		if (!no_modify) {
			do_warn(
_("WARNING: you have a V1 inode filesystem. It will be converted to a\n"
  "\tversion 2 inode filesystem. If you do not want this, run an older\n"
  "\tversion of xfs_repair.\n"));
		} else  {
			do_warn(
_("WARNING: you have a V1 inode filesystem. It would be converted to a\n"
  "\tversion 2 inode filesystem. If you do not want this, run an older\n"
  "\tversion of xfs_repair.\n"));
		}
	}

	if (xfs_sb_version_hasquota(sb))  {
		fs_quotas = 1;

		if (sb->sb_uquotino != 0 && sb->sb_uquotino != NULLFSINO)
			have_uquotino = 1;

		if (sb->sb_gquotino != 0 && sb->sb_gquotino != NULLFSINO)
			have_gquotino = 1;

		if (sb->sb_pquotino != 0 && sb->sb_pquotino != NULLFSINO)
			have_pquotino = 1;
	}

	if (xfs_sb_version_hasalign(sb))  {
		fs_aligned_inodes = 1;
		fs_ino_alignment = sb->sb_inoalignmt;
	}

	/*
	 * calculate maximum file offset for this geometry
	 */
	fs_max_file_offset = 0x7fffffffffffffffLL >> sb->sb_blocklog;

	return(0);
}
