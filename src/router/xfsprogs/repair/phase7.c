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
#include "avl.h"
#include "globals.h"
#include "agheader.h"
#include "incore.h"
#include "protos.h"
#include "err_protos.h"
#include "dinode.h"
#include "versions.h"
#include "progress.h"

/* dinoc is a pointer to the IN-CORE dinode core */
static void
set_nlinks(
	xfs_icdinode_t		*dinoc,
	xfs_ino_t		ino,
	__uint32_t		nrefs,
	int			*dirty)
{
	if (dinoc->di_nlink == nrefs)
		return;

	if (!no_modify) {
		*dirty = 1;
		do_warn(_("resetting inode %" PRIu64 " nlinks from %d to %d\n"),
			ino, dinoc->di_nlink, nrefs);

		if (nrefs > XFS_MAXLINK_1)  {
			ASSERT(fs_inode_nlink);
			do_warn(
_("nlinks %d will overflow v1 ino, ino %" PRIu64 " will be converted to version 2\n"),
				nrefs, ino);

		}
		dinoc->di_nlink = nrefs;
	} else  {
		do_warn(_("would have reset inode %" PRIu64 " nlinks from %d to %d\n"),
			ino, dinoc->di_nlink, nrefs);
	}
}

static void
update_inode_nlinks(
	xfs_mount_t 		*mp,
	xfs_ino_t		ino,
	__uint32_t		nlinks)
{
	xfs_trans_t		*tp;
	xfs_inode_t		*ip;
	int			error;
	int			dirty;

	tp = libxfs_trans_alloc(mp, XFS_TRANS_REMOVE);

	error = libxfs_trans_reserve(tp, (no_modify ? 0 : 10),
			XFS_REMOVE_LOG_RES(mp), 0, XFS_TRANS_PERM_LOG_RES,
			XFS_REMOVE_LOG_COUNT);

	ASSERT(error == 0);

	error = libxfs_trans_iget(mp, tp, ino, 0, 0, &ip);

	if (error)  {
		if (!no_modify)
			do_error(
	_("couldn't map inode %" PRIu64 ", err = %d\n"),
				ino, error);
		else  {
			do_warn(
	_("couldn't map inode %" PRIu64 ", err = %d, can't compare link counts\n"),
				ino, error);
			return;
		}
	}

	dirty = 0;

	/*
	 * compare and set links for all inodes
	 */
	set_nlinks(&ip->i_d, ino, nlinks, &dirty);

	if (!dirty)  {
		libxfs_trans_iput(tp, ip, 0);
		libxfs_trans_cancel(tp, XFS_TRANS_RELEASE_LOG_RES);
	} else  {
		libxfs_trans_log_inode(tp, ip, XFS_ILOG_CORE);
		/*
		 * no need to do a bmap finish since
		 * we're not allocating anything
		 */
		ASSERT(error == 0);
		error = libxfs_trans_commit(tp, XFS_TRANS_RELEASE_LOG_RES |
				XFS_TRANS_SYNC);

		ASSERT(error == 0);
	}
}

void
phase7(xfs_mount_t *mp)
{
	ino_tree_node_t		*irec;
	int			i;
	int			j;
	__uint32_t		nrefs;

	if (!no_modify)
		do_log(_("Phase 7 - verify and correct link counts...\n"));
	else
		do_log(_("Phase 7 - verify link counts...\n"));

	/*
	 * for each ag, look at each inode 1 at a time. If the number of
	 * links is bad, reset it, log the inode core, commit the transaction
	 */
	for (i = 0; i < glob_agcount; i++)  {
		irec = findfirst_inode_rec(i);

		while (irec != NULL)  {
			for (j = 0; j < XFS_INODES_PER_CHUNK; j++)  {
				ASSERT(is_inode_confirmed(irec, j));

				if (is_inode_free(irec, j))
					continue;

				ASSERT(no_modify || is_inode_reached(irec, j));
				ASSERT(no_modify ||
						is_inode_referenced(irec, j));

				nrefs = num_inode_references(irec, j);

				if (get_inode_disk_nlinks(irec, j) != nrefs)
					update_inode_nlinks(mp,
						XFS_AGINO_TO_INO(mp, i,
							irec->ino_startnum + j),
						nrefs);
			}
			irec = next_ino_rec(irec);
		}
	}
}
