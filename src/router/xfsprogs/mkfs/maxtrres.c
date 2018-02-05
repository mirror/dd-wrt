/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
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

/*
 * maxtrres.c
 *
 * Compute the maximum transaction reservation for a legal combination
 * of sector size, block size, inode size, directory version, and
 * directory block size.
 */

#include "libxfs.h"
#include "xfs_multidisk.h"

int
max_trans_res(
	unsigned long	agsize,
	int		crcs_enabled,
	int		dirversion,
	int		sectorlog,
	int		blocklog,
	int		inodelog,
	int		dirblocklog,
	int		logversion,
	int		log_sunit,
	int		finobt,
	int		rmapbt,
	int		reflink)
{
	xfs_sb_t	*sbp;
	xfs_mount_t	mount;
	int		maxfsb;

	memset(&mount, 0, sizeof(mount));
	sbp = &mount.m_sb;
	sbp->sb_magicnum = XFS_SB_MAGIC;
	sbp->sb_sectlog = sectorlog;
	sbp->sb_sectsize = 1 << sbp->sb_sectlog;
	sbp->sb_blocklog = blocklog;
	sbp->sb_blocksize = 1 << blocklog;
	sbp->sb_agblocks = agsize;
	sbp->sb_inodelog = inodelog;
	sbp->sb_inopblog = blocklog - inodelog;
	sbp->sb_inodesize = 1 << inodelog;
	sbp->sb_inopblock = 1 << (blocklog - inodelog);
	sbp->sb_dirblklog = dirblocklog - blocklog;

	if (log_sunit > 0) {
		log_sunit <<= blocklog;
		logversion = 2;
	} else
		log_sunit = 1;
	sbp->sb_logsunit = log_sunit;

	sbp->sb_versionnum =
			(crcs_enabled ? XFS_SB_VERSION_5 : XFS_SB_VERSION_4) |
			(dirversion == 2 ? XFS_SB_VERSION_DIRV2BIT : 0) |
			(logversion > 1 ? XFS_SB_VERSION_LOGV2BIT : 0) |
			XFS_DFL_SB_VERSION_BITS;
	if (finobt)
		sbp->sb_features_ro_compat |= XFS_SB_FEAT_RO_COMPAT_FINOBT;
	if (rmapbt)
		sbp->sb_features_ro_compat |= XFS_SB_FEAT_RO_COMPAT_RMAPBT;
	if (reflink)
		sbp->sb_features_ro_compat |= XFS_SB_FEAT_RO_COMPAT_REFLINK;

	libxfs_mount(&mount, sbp, 0,0,0,0);
	maxfsb = libxfs_log_calc_minimum_size(&mount);
	libxfs_umount(&mount);

#if 0
	printf("#define\tMAXTRRES_S%d_B%d_I%d_D%d_V%d_LSU%d\t%d\n",
		sectorlog, blocklog, inodelog, dirblocklog, dirversion,
		log_sunit, maxfsb);
#endif

	return maxfsb;
}
