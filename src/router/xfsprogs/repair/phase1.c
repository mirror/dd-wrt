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
#include "globals.h"
#include "agheader.h"
#include "protos.h"
#include "err_protos.h"

void
no_sb(void)
{
	do_warn(_("Sorry, could not find valid secondary superblock\n"));
	do_warn(_("Exiting now.\n"));
	exit(1);
}

char *
alloc_ag_buf(int size)
{
	char	*bp;

	bp = (char *)memalign(libxfs_device_alignment(), size);
	if (!bp)
		do_error(_("could not allocate ag header buffer (%d bytes)\n"),
			size);
	return(bp);
}

/*
 * this has got to be big enough to hold 4 sectors
 */
#define MAX_SECTSIZE		(512 * 1024)

/* ARGSUSED */
void
phase1(xfs_mount_t *mp)
{
	xfs_sb_t		*sb;
	char			*ag_bp;
	int			rval;

	do_log(_("Phase 1 - find and verify superblock...\n"));

	primary_sb_modified = 0;
	need_root_inode = 0;
	need_root_dotdot = 0;
	need_rbmino = 0;
	need_rsumino = 0;
	lost_quotas = 0;

	/*
	 * get AG 0 into ag header buf
	 */
	ag_bp = alloc_ag_buf(MAX_SECTSIZE);
	sb = (xfs_sb_t *) ag_bp;

	if (get_sb(sb, 0LL, MAX_SECTSIZE, 0) == XR_EOF)
		do_error(_("error reading primary superblock\n"));

	/*
	 * is this really an sb, verify internal consistency
	 */
	if ((rval = verify_sb(sb, 1)) != XR_OK)  {
		do_warn(_("bad primary superblock - %s !!!\n"),
			err_string(rval));
		if (!find_secondary_sb(sb))
			no_sb();
		primary_sb_modified = 1;
	} else if ((rval = verify_set_primary_sb(sb, 0,
					&primary_sb_modified)) != XR_OK)  {
		do_warn(_("couldn't verify primary superblock - %s !!!\n"),
			err_string(rval));
		if (!find_secondary_sb(sb))
			no_sb();
		primary_sb_modified = 1;
	}

	/*
	 * Check bad_features2 and make sure features2 the same as
	 * bad_features (ORing the two together). Leave bad_features2
	 * set so older kernels can still use it and not mount unsupported
	 * filesystems when it reads bad_features2.
	 */
	if (sb->sb_bad_features2 != 0 &&
			sb->sb_bad_features2 != sb->sb_features2) {
		sb->sb_features2 |= sb->sb_bad_features2;
		sb->sb_bad_features2 = sb->sb_features2;
		primary_sb_modified = 1;
		do_warn(_("superblock has a features2 mismatch, correcting\n"));
	}

	/*
	 * apply any version changes or conversions after the primary
	 * superblock has been verified or repaired
	 *
	 * Send output to stdout as do_log and everything else in repair
	 * is sent to stderr and there is no "quiet" option. xfs_admin
	 * will filter stderr but not stdout. This situation must be improved.
	 */
	if (convert_lazy_count) {
		if (lazy_count && !xfs_sb_version_haslazysbcount(sb)) {
			sb->sb_versionnum |= XFS_SB_VERSION_MOREBITSBIT;
			sb->sb_features2 |= XFS_SB_VERSION2_LAZYSBCOUNTBIT;
			sb->sb_bad_features2 |= XFS_SB_VERSION2_LAZYSBCOUNTBIT;
			primary_sb_modified = 1;
			printf(_("Enabling lazy-counters\n"));
		} else
		if (!lazy_count && xfs_sb_version_haslazysbcount(sb)) {
			sb->sb_features2 &= ~XFS_SB_VERSION2_LAZYSBCOUNTBIT;
			sb->sb_bad_features2 &= ~XFS_SB_VERSION2_LAZYSBCOUNTBIT;
			printf(_("Disabling lazy-counters\n"));
			primary_sb_modified = 1;
		} else {
			printf(_("Lazy-counters are already %s\n"),
				lazy_count ? _("enabled") : _("disabled"));
			exit(0); /* no conversion required, exit */
		}
	}

	if (primary_sb_modified)  {
		if (!no_modify)  {
			do_warn(_("writing modified primary superblock\n"));
			write_primary_sb(sb, sb->sb_sectsize);
		} else  {
			do_warn(_("would write modified primary superblock\n"));
		}
	}

	/*
	 * misc. global var initialization
	 */
	sb_ifree = sb_icount = sb_fdblocks = sb_frextents = 0;

	free(sb);
}
