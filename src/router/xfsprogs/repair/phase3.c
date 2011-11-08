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
#include "threads.h"
#include "progress.h"
#include "prefetch.h"

/*
 * walks an unlinked list, returns 1 on an error (bogus pointer) or
 * I/O error
 */
int
walk_unlinked_list(xfs_mount_t *mp, xfs_agnumber_t agno, xfs_agino_t start_ino)
{
	xfs_buf_t *bp;
	xfs_dinode_t *dip;
	xfs_agino_t current_ino = start_ino;
	xfs_agblock_t agbno;
	int state;

	while (current_ino != NULLAGINO)  {
		if (verify_aginum(mp, agno, current_ino))
			return(1);
		if ((bp = get_agino_buf(mp, agno, current_ino, &dip)) == NULL)
			return(1);
		/*
		 * if this looks like a decent inode, then continue
		 * following the unlinked pointers.  If not, bail.
		 */
		if (verify_dinode(mp, dip, agno, current_ino) == 0)  {
			/*
			 * check if the unlinked list points to an unknown
			 * inode.  if so, put it on the uncertain inode list
			 * and set block map appropriately.
			 */
			if (find_inode_rec(mp, agno, current_ino) == NULL)  {
				add_aginode_uncertain(agno, current_ino, 1);
				agbno = XFS_AGINO_TO_AGBNO(mp, current_ino);

				pthread_mutex_lock(&ag_locks[agno]);
				state = get_bmap(agno, agbno);
				switch (state) {
				case XR_E_BAD_STATE:
					do_error(_(
						"bad state in block map %d\n"),
						state);
					break;
				default:
					/*
					 * the block looks like inodes
					 * so be conservative and try
					 * to scavenge what's in there.
					 * if what's there is completely
					 * bogus, it'll show up later
					 * and the inode will be trashed
					 * anyway, hopefully without
					 * losing too much other data
					 */
					set_bmap(agno, agbno, XR_E_INO);
					break;
				}
				pthread_mutex_unlock(&ag_locks[agno]);
			}
			current_ino = be32_to_cpu(dip->di_next_unlinked);
		} else  {
			current_ino = NULLAGINO;;
		}
		libxfs_putbuf(bp);
	}

	return(0);
}

void
process_agi_unlinked(xfs_mount_t *mp, xfs_agnumber_t agno)
{
	xfs_agnumber_t i;
	xfs_buf_t *bp;
	xfs_agi_t *agip;
	int err = 0;
	int agi_dirty = 0;

	bp = libxfs_readbuf(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)),
			mp->m_sb.sb_sectsize/BBSIZE, 0);
	if (!bp)
		do_error(_("cannot read agi block %" PRId64 " for ag %u\n"),
			XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)), agno);

	agip = XFS_BUF_TO_AGI(bp);

	ASSERT(no_modify || be32_to_cpu(agip->agi_seqno) == agno);

	for (i = 0; i < XFS_AGI_UNLINKED_BUCKETS; i++)  {
		if (be32_to_cpu(agip->agi_unlinked[i]) != NULLAGINO)  {
			err += walk_unlinked_list(mp, agno,
					be32_to_cpu(agip->agi_unlinked[i]));
			/*
			 * clear the list
			 */
			if (!no_modify)  {
				agip->agi_unlinked[i] = cpu_to_be32(NULLAGINO);
				agi_dirty = 1;
			}
		}
	}

	if (err)
		do_warn(_("error following ag %d unlinked list\n"), agno);

	ASSERT(agi_dirty == 0 || (agi_dirty && !no_modify));

	if (agi_dirty && !no_modify)
		libxfs_writebuf(bp, 0);
	else
		libxfs_putbuf(bp);
}

static void
process_ag_func(
	work_queue_t		*wq,
	xfs_agnumber_t 		agno,
	void			*arg)
{
	/*
	 * turn on directory processing (inode discovery) and
	 * attribute processing (extra_attr_check)
	 */
	wait_for_inode_prefetch(arg);
	do_log(_("        - agno = %d\n"), agno);
	process_aginodes(wq->mp, arg, agno, 1, 0, 1);
	cleanup_inode_prefetch(arg);
}

static void
process_ags(
	xfs_mount_t		*mp)
{
	int 			i, j;
	xfs_agnumber_t 		agno;
	work_queue_t		*queues;
	prefetch_args_t		*pf_args[2];

	queues = malloc(thread_count * sizeof(work_queue_t));

	if (ag_stride) {
		/*
		 * create one worker thread for each segment of the volume
		 */
		for (i = 0, agno = 0; i < thread_count; i++) {
			create_work_queue(&queues[i], mp, 1);
			pf_args[0] = NULL;
			for (j = 0; j < ag_stride && agno < mp->m_sb.sb_agcount;
					j++, agno++) {
				pf_args[0] = start_inode_prefetch(agno, 0, pf_args[0]);
				queue_work(&queues[i], process_ag_func, agno, pf_args[0]);
			}
		}
		/*
		 * wait for workers to complete
		 */
		for (i = 0; i < thread_count; i++)
			destroy_work_queue(&queues[i]);
	} else {
		queues[0].mp = mp;
		pf_args[0] = start_inode_prefetch(0, 0, NULL);
		for (i = 0; i < mp->m_sb.sb_agcount; i++) {
			pf_args[(~i) & 1] = start_inode_prefetch(i + 1, 0,
					pf_args[i & 1]);
			process_ag_func(&queues[0], i, pf_args[i & 1]);
		}
	}
	free(queues);
}

void
phase3(xfs_mount_t *mp)
{
	int 			i, j;

	do_log(_("Phase 3 - for each AG...\n"));
	if (!no_modify)
		do_log(_("        - scan and clear agi unlinked lists...\n"));
	else
		do_log(_("        - scan (but don't clear) agi unlinked lists...\n"));

	set_progress_msg(PROG_FMT_AGI_UNLINKED, (__uint64_t) glob_agcount);

	/*
	 * first, let's look at the possibly bogus inodes
	 */
	for (i = 0; i < mp->m_sb.sb_agcount; i++)  {
		/*
		 * walk unlinked list to add more potential inodes to list
		 */
		process_agi_unlinked(mp, i);
		check_uncertain_aginodes(mp, i);
		PROG_RPT_INC(prog_rpt_done[i], 1);
	}
	print_final_rpt();

	/* ok, now that the tree's ok, let's take a good look */

	do_log(_(
	    "        - process known inodes and perform inode discovery...\n"));

	set_progress_msg(PROG_FMT_PROCESS_INO, (__uint64_t) mp->m_sb.sb_icount);

	process_ags(mp);

	print_final_rpt();

	/*
	 * process newly discovered inode chunks
	 */
	do_log(_("        - process newly discovered inodes...\n"));
	set_progress_msg(PROG_FMT_NEW_INODES, (__uint64_t) glob_agcount);
	do  {
		/*
		 * have to loop until no ag has any uncertain
		 * inodes
		 */
		j = 0;
		for (i = 0; i < mp->m_sb.sb_agcount; i++)  {
			j += process_uncertain_aginodes(mp, i);
#ifdef XR_INODE_TRACE
			fprintf(stderr,
				"\t\t phase 3 - process_uncertain_inodes returns %d\n", j);
#endif
			PROG_RPT_INC(prog_rpt_done[i], 1);
		}
	} while (j != 0);
	print_final_rpt();
}
