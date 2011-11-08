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

#include <libxfs.h>

#define EXTERN
#include "versions.h"
#undef EXTERN
#include "err_protos.h"
#include "globals.h"

void
update_sb_version(xfs_mount_t *mp)
{
	xfs_sb_t	*sb;
	__uint16_t	vn;

	sb = &mp->m_sb;

	if (fs_attributes && !xfs_sb_version_hasattr(sb))  {
		ASSERT(fs_attributes_allowed);
		xfs_sb_version_addattr(sb);
	}

	if (fs_attributes2 && !xfs_sb_version_hasattr2(sb))  {
		ASSERT(fs_attributes2_allowed);
		xfs_sb_version_addattr2(sb);
	}

	if (fs_inode_nlink && !xfs_sb_version_hasnlink(sb))  {
		ASSERT(fs_inode_nlink_allowed);
		xfs_sb_version_addnlink(sb);
	}

	/*
	 * fix up the superblock version number and feature bits,
	 * turn off quota bits and flags if the filesystem doesn't
	 * have quotas.
	 */
	if (fs_quotas)  {
		if (!xfs_sb_version_hasquota(sb))  {
			ASSERT(fs_quotas_allowed);
			xfs_sb_version_addquota(sb);
		}

		/*
		 * protect against stray bits in the quota flag field
		 */
		if (sb->sb_qflags & ~(XFS_UQUOTA_ACCT|XFS_UQUOTA_ENFD|
				XFS_UQUOTA_CHKD|XFS_GQUOTA_ACCT|
				XFS_OQUOTA_ENFD|XFS_OQUOTA_CHKD|
				XFS_PQUOTA_ACCT))  {
			/*
			 * update the incore superblock, if we're in
			 * no_modify mode, it'll never get flushed out
			 * so this is ok.
			 */
			do_warn(_("bogus quota flags 0x%x set in superblock"),
				sb->sb_qflags & ~(XFS_UQUOTA_ACCT|
				XFS_UQUOTA_ENFD|
				XFS_UQUOTA_CHKD|XFS_GQUOTA_ACCT|
				XFS_OQUOTA_ENFD|XFS_OQUOTA_CHKD|
				XFS_PQUOTA_ACCT));

			sb->sb_qflags &= (XFS_UQUOTA_ACCT|XFS_UQUOTA_ENFD|
				XFS_UQUOTA_CHKD|XFS_GQUOTA_ACCT|
				XFS_OQUOTA_ENFD|XFS_OQUOTA_CHKD|
				XFS_PQUOTA_ACCT);

			if (!no_modify)
				do_warn(_(", bogus flags will be cleared\n"));
			else
				do_warn(_(", bogus flags would be cleared\n"));
		}
	} else  {
		sb->sb_qflags = 0;

		if (xfs_sb_version_hasquota(sb))  {
			lost_quotas = 1;
			vn = sb->sb_versionnum;
			vn &= ~XFS_SB_VERSION_QUOTABIT;

			if (!(vn & XFS_SB_VERSION_ALLFBITS))
				vn = xfs_sb_version_toold(vn);

			ASSERT(vn != 0);
			sb->sb_versionnum = vn;
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
	int issue_warning;

	fs_attributes = 0;
	fs_attributes2 = 0;
	fs_inode_nlink = 0;
	fs_quotas = 0;
	fs_aligned_inodes = 0;
	fs_sb_feature_bits = 0;
	fs_ino_alignment = 0;
	fs_has_extflgbit = 0;
	have_uquotino = 0;
	have_gquotino = 0;
	issue_warning = 0;

	/*
	 * ok, check to make sure that the sb isn't newer
	 * than we are
	 */
	if (xfs_sb_version_hasextflgbit(sb))  {
		fs_has_extflgbit = 1;
		if (!fs_has_extflgbit_allowed)  {
			issue_warning = 1;
			do_warn(
			_("This filesystem has uninitialized extent flags.\n"));
		}
	}

	if (xfs_sb_version_hasshared(sb))  {
		fs_shared = 1;
		if (!fs_shared_allowed)  {
			issue_warning = 1;
			do_warn(_("This filesystem is marked shared.\n"));
		}
	}

	if (issue_warning)  {
		do_warn(
_("This filesystem uses feature(s) not yet supported in this release.\n"
  "Please run a more recent version of xfs_repair.\n"));
		return(1);
	}

	if (!xfs_sb_good_version(sb))  {
		do_warn(_("WARNING:  unknown superblock version %d\n"),
			XFS_SB_VERSION_NUM(sb));
		do_warn(
_("This filesystem contains features not understood by this program.\n"));
		return(1);
	}

	if (XFS_SB_VERSION_NUM(sb) == XFS_SB_VERSION_4)  {
		if (!fs_sb_feature_bits_allowed)  {
			if (!no_modify)  {
				do_warn(
_("WARNING:  you have disallowed superblock-feature-bits-allowed\n"
  "\tbut this superblock has feature bits.  The superblock\n"
  "\twill be downgraded.  This may cause loss of filesystem meta-data\n"));
			} else   {
				do_warn(
_("WARNING:  you have disallowed superblock-feature-bits-allowed\n"
  "\tbut this superblock has feature bits.  The superblock\n"
  "\twould be downgraded.  This might cause loss of filesystem\n"
  "\tmeta-data.\n"));
			}
		} else   {
			fs_sb_feature_bits = 1;
		}
	}

	if (xfs_sb_version_hasattr(sb))  {
		if (!fs_attributes_allowed)  {
			if (!no_modify)  {
				do_warn(
_("WARNING:  you have disallowed attributes but this filesystem\n"
  "\thas attributes.  The filesystem will be downgraded and\n"
  "\tall attributes will be removed.\n"));
			} else  {
				do_warn(
_("WARNING:  you have disallowed attributes but this filesystem\n"
  "\thas attributes.  The filesystem would be downgraded and\n"
  "\tall attributes would be removed.\n"));
			}
		} else   {
			fs_attributes = 1;
		}
	}

	if (xfs_sb_version_hasattr2(sb))  {
		if (!fs_attributes2_allowed)  {
			if (!no_modify)  {
				do_warn(
_("WARNING:  you have disallowed attr2 attributes but this filesystem\n"
  "\thas attributes.  The filesystem will be downgraded and\n"
  "\tall attr2 attributes will be removed.\n"));
			} else  {
				do_warn(
_("WARNING:  you have disallowed attr2 attributes but this filesystem\n"
  "\thas attributes.  The filesystem would be downgraded and\n"
  "\tall attr2 attributes would be removed.\n"));
			}
		} else   {
			fs_attributes2 = 1;
		}
	}

	if (xfs_sb_version_hasnlink(sb))  {
		if (!fs_inode_nlink_allowed)  {
			if (!no_modify)  {
				do_warn(
_("WARNING:  you have disallowed version 2 inodes but this filesystem\n"
  "\thas version 2 inodes.  The filesystem will be downgraded and\n"
  "\tall version 2 inodes will be converted to version 1 inodes.\n"
  "\tThis may cause some hard links to files to be destroyed\n"));
			} else  {
				do_warn(
_("WARNING:  you have disallowed version 2 inodes but this filesystem\n"
  "\thas version 2 inodes.  The filesystem would be downgraded and\n"
  "\tall version 2 inodes would be converted to version 1 inodes.\n"
  "\tThis might cause some hard links to files to be destroyed\n"));
			}
		} else   {
			fs_inode_nlink = 1;
		}
	}

	if (xfs_sb_version_hasquota(sb))  {
		if (!fs_quotas_allowed)  {
			if (!no_modify)  {
				do_warn(
_("WARNING:  you have disallowed quotas but this filesystem\n"
  "\thas quotas.  The filesystem will be downgraded and\n"
  "\tall quota information will be removed.\n"));
			} else  {
				do_warn(
_("WARNING:  you have disallowed quotas but this filesystem\n"
  "\thas quotas.  The filesystem would be downgraded and\n"
  "\tall quota information would be removed.\n"));
			}
		} else   {
			fs_quotas = 1;

			if (sb->sb_uquotino != 0 &&
					sb->sb_uquotino != NULLFSINO)
				have_uquotino = 1;

			if (sb->sb_gquotino != 0 &&
					sb->sb_gquotino != NULLFSINO)
				have_gquotino = 1;
		}
	}

	if (xfs_sb_version_hasalign(sb))  {
		if (fs_aligned_inodes_allowed)  {
			fs_aligned_inodes = 1;
			fs_ino_alignment = sb->sb_inoalignmt;
		} else   {
			if (!no_modify)  {
				do_warn(
_("WARNING:  you have disallowed aligned inodes but this filesystem\n"
  "\thas aligned inodes.  The filesystem will be downgraded.\n"
  "\tThis will permanently degrade the performance of this filesystem.\n"));
			} else  {
				do_warn(
_("WARNING:  you have disallowed aligned inodes but this filesystem\n"
  "\thas aligned inodes.  The filesystem would be downgraded.\n"
  "\tThis would permanently degrade the performance of this filesystem.\n"));
			}
		}
	}

	/*
	 * calculate maximum file offset for this geometry
	 */
	fs_max_file_offset = 0x7fffffffffffffffLL >> sb->sb_blocklog;

	return(0);
}
