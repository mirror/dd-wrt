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
#ifndef _XR_DINODE_H
#define _XR_DINODE_H

struct blkmap;
struct prefetch_args;

int
verify_agbno(xfs_mount_t	*mp,
		xfs_agnumber_t	agno,
		xfs_agblock_t	agbno);

int
verify_dfsbno(xfs_mount_t	*mp,
		xfs_fsblock_t	fsbno);

void
convert_extent(
	xfs_bmbt_rec_t		*rp,
	xfs_fileoff_t		*op,	/* starting offset (blockno in file) */
	xfs_fsblock_t		*sp,	/* starting block (fs blockno) */
	xfs_filblks_t		*cp,	/* blockcount */
	int			*fp);	/* extent flag */

int
process_bmbt_reclist(xfs_mount_t	*mp,
		xfs_bmbt_rec_t		*rp,
		int			*numrecs,
		int			type,
		xfs_ino_t		ino,
		xfs_rfsblock_t		*tot,
		struct blkmap		**blkmapp,
		uint64_t		*first_key,
		uint64_t		*last_key,
		int			whichfork);

int
scan_bmbt_reclist(
	xfs_mount_t		*mp,
	xfs_bmbt_rec_t		*rp,
	int			*numrecs,
	int			type,
	xfs_ino_t		ino,
	xfs_rfsblock_t		*tot,
	int			whichfork);

void
update_rootino(xfs_mount_t *mp);

int
process_dinode(xfs_mount_t *mp,
		xfs_dinode_t *dino,
		xfs_agnumber_t agno,
		xfs_agino_t ino,
		int was_free,
		int *dirty,
		int *used,
		int check_dirs,
		int check_dups,
		int extra_attr_check,
		int *isa_dir,
		xfs_ino_t *parent);

int
verify_dinode(xfs_mount_t *mp,
		xfs_dinode_t *dino,
		xfs_agnumber_t agno,
		xfs_agino_t ino);

int
verify_uncertain_dinode(xfs_mount_t *mp,
		xfs_dinode_t *dino,
		xfs_agnumber_t agno,
		xfs_agino_t ino);

int
verify_inum(xfs_mount_t		*mp,
		xfs_ino_t	ino);

int
verify_aginum(xfs_mount_t	*mp,
		xfs_agnumber_t	agno,
		xfs_agino_t	agino);

int
process_uncertain_aginodes(xfs_mount_t		*mp,
				xfs_agnumber_t	agno);
void
process_aginodes(xfs_mount_t		*mp,
		struct prefetch_args	*pf_args,
		xfs_agnumber_t		agno,
		int			check_dirs,
		int			check_dups,
		int			extra_attr_check);

void
check_uncertain_aginodes(xfs_mount_t	*mp,
			xfs_agnumber_t	agno);

struct xfs_buf *
get_agino_buf(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	xfs_agino_t		agino,
	struct xfs_dinode	**dipp);

void dinode_bmbt_translation_init(void);
char * get_forkname(int whichfork);

#endif /* _XR_DINODE_H */
