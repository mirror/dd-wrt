// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "type.h"
#include "fprint.h"
#include "faddr.h"
#include "field.h"
#include "inode.h"
#include "io.h"
#include "bit.h"
#include "bmap.h"
#include "output.h"
#include "init.h"

void
fa_agblock(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	xfs_agblock_t	bno;

	if (cur_agno == NULLAGNUMBER) {
		dbprintf(_("no current allocation group, cannot set new addr\n"));
		return;
	}
	bno = (xfs_agblock_t)getbitval(obj, bit, bitsz(bno), BVUNSIGNED);
	if (bno == NULLAGBLOCK) {
		dbprintf(_("null block number, cannot set new addr\n"));
		return;
	}
	ASSERT(typtab[next].typnm == next);
	set_cur(&typtab[next], XFS_AGB_TO_DADDR(mp, cur_agno, bno), blkbb,
		DB_RING_ADD, NULL);
}

/*ARGSUSED*/
void
fa_agino(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	xfs_agino_t	agino;

	if (cur_agno == NULLAGNUMBER) {
		dbprintf(_("no current allocation group, cannot set new addr\n"));
		return;
	}
	agino = (xfs_agino_t)getbitval(obj, bit, bitsz(agino), BVUNSIGNED);
	if (agino == NULLAGINO) {
		dbprintf(_("null inode number, cannot set new addr\n"));
		return;
	}
	set_cur_inode(XFS_AGINO_TO_INO(mp, cur_agno, agino));
}

/*ARGSUSED*/
void
fa_attrblock(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	bmap_ext_t	bm;
	uint32_t	bno;
	xfs_fsblock_t	dfsbno;
	xfs_extnum_t	nex;

	bno = (uint32_t)getbitval(obj, bit, bitsz(bno), BVUNSIGNED);
	if (bno == 0) {
		dbprintf(_("null attribute block number, cannot set new addr\n"));
		return;
	}
	nex = 1;
	bmap(bno, 1, XFS_ATTR_FORK, &nex, &bm);
	if (nex == 0) {
		dbprintf(_("attribute block is unmapped\n"));
		return;
	}
	dfsbno = bm.startblock + (bno - bm.startoff);
	ASSERT(typtab[next].typnm == next);
	set_cur(&typtab[next], (int64_t)XFS_FSB_TO_DADDR(mp, dfsbno), blkbb,
		DB_RING_ADD, NULL);
}

void
fa_cfileoffa(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	bmap_ext_t	bm;
	xfs_fileoff_t	bno;
	xfs_fsblock_t	dfsbno;
	xfs_extnum_t	nex;

	bno = (xfs_fileoff_t)getbitval(obj, bit, BMBT_STARTOFF_BITLEN,
		BVUNSIGNED);
	if (bno == NULLFILEOFF) {
		dbprintf(_("null block number, cannot set new addr\n"));
		return;
	}
	nex = 1;
	bmap(bno, 1, XFS_ATTR_FORK, &nex, &bm);
	if (nex == 0) {
		dbprintf(_("file block is unmapped\n"));
		return;
	}
	dfsbno = bm.startblock + (bno - bm.startoff);
	ASSERT(typtab[next].typnm == next);
	set_cur(&typtab[next], XFS_FSB_TO_DADDR(mp, dfsbno), blkbb, DB_RING_ADD,
		NULL);
}

void
fa_cfileoffd(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	bbmap_t		bbmap;
	bmap_ext_t	*bmp;
	xfs_fileoff_t	bno;
	xfs_fsblock_t	dfsbno;
	xfs_extnum_t	nb;
	xfs_extnum_t	nex;

	bno = (xfs_fileoff_t)getbitval(obj, bit, BMBT_STARTOFF_BITLEN,
		BVUNSIGNED);
	if (bno == NULLFILEOFF) {
		dbprintf(_("null block number, cannot set new addr\n"));
		return;
	}
	nex = nb = next == TYP_DIR2 ? mp->m_dir_geo->fsbcount : 1;
	bmp = malloc(nb * sizeof(*bmp));
	bmap(bno, nb, XFS_DATA_FORK, &nex, bmp);
	if (nex == 0) {
		dbprintf(_("file block is unmapped\n"));
		free(bmp);
		return;
	}
	dfsbno = bmp->startblock + (bno - bmp->startoff);
	ASSERT(typtab[next].typnm == next);
	if (nex > 1)
		make_bbmap(&bbmap, nex, bmp);
	set_cur(&typtab[next], XFS_FSB_TO_DADDR(mp, dfsbno), nb * blkbb,
		DB_RING_ADD, nex > 1 ? &bbmap: NULL);
	free(bmp);
}

void
fa_cfsblock(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	xfs_fsblock_t	bno;
	int		nb;

	bno = (xfs_fsblock_t)getbitval(obj, bit, BMBT_STARTBLOCK_BITLEN,
		BVUNSIGNED);
	if (bno == NULLFSBLOCK) {
		dbprintf(_("null block number, cannot set new addr\n"));
		return;
	}
	nb = next == TYP_DIR2 ? mp->m_dir_geo->fsbcount : 1;
	ASSERT(typtab[next].typnm == next);
	set_cur(&typtab[next], XFS_FSB_TO_DADDR(mp, bno), nb * blkbb,
		DB_RING_ADD, NULL);
}

void
fa_dfiloffa(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	bmap_ext_t	bm;
	xfs_fileoff_t	bno;
	xfs_fsblock_t	dfsbno;
	xfs_extnum_t	nex;

	bno = (xfs_fileoff_t)getbitval(obj, bit, bitsz(bno), BVUNSIGNED);
	if (bno == NULLFILEOFF) {
		dbprintf(_("null block number, cannot set new addr\n"));
		return;
	}
	nex = 1;
	bmap(bno, 1, XFS_ATTR_FORK, &nex, &bm);
	if (nex == 0) {
		dbprintf(_("file block is unmapped\n"));
		return;
	}
	dfsbno = bm.startblock + (bno - bm.startoff);
	ASSERT(typtab[next].typnm == next);
	set_cur(&typtab[next], XFS_FSB_TO_DADDR(mp, dfsbno), blkbb, DB_RING_ADD,
		NULL);
}

void
fa_dfiloffd(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	bbmap_t		bbmap;
	bmap_ext_t	*bmp;
	xfs_fileoff_t	bno;
	xfs_fsblock_t	dfsbno;
	xfs_extnum_t	nb;
	xfs_extnum_t	nex;

	bno = (xfs_fileoff_t)getbitval(obj, bit, bitsz(bno), BVUNSIGNED);
	if (bno == NULLFILEOFF) {
		dbprintf(_("null block number, cannot set new addr\n"));
		return;
	}
	nex = nb = next == TYP_DIR2 ? mp->m_dir_geo->fsbcount : 1;
	bmp = malloc(nb * sizeof(*bmp));
	bmap(bno, nb, XFS_DATA_FORK, &nex, bmp);
	if (nex == 0) {
		dbprintf(_("file block is unmapped\n"));
		free(bmp);
		return;
	}
	dfsbno = bmp->startblock + (bno - bmp->startoff);
	ASSERT(typtab[next].typnm == next);
	if (nex > 1)
		make_bbmap(&bbmap, nex, bmp);
	set_cur(&typtab[next], XFS_FSB_TO_DADDR(mp, dfsbno), nb * blkbb,
		DB_RING_ADD, nex > 1 ? &bbmap : NULL);
	free(bmp);
}

void
fa_dfsbno(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	xfs_fsblock_t	bno;

	bno = (xfs_fsblock_t)getbitval(obj, bit, bitsz(bno), BVUNSIGNED);
	if (bno == NULLFSBLOCK) {
		dbprintf(_("null block number, cannot set new addr\n"));
		return;
	}
	ASSERT(typtab[next].typnm == next);
	set_cur(&typtab[next], XFS_FSB_TO_DADDR(mp, bno), blkbb, DB_RING_ADD,
		NULL);
}

/*ARGSUSED*/
void
fa_dirblock(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	bbmap_t		bbmap;
	bmap_ext_t	*bmp;
	uint32_t	bno;
	xfs_fsblock_t	dfsbno;
	xfs_extnum_t	nex;

	bno = (uint32_t)getbitval(obj, bit, bitsz(bno), BVUNSIGNED);
	if (bno == 0) {
		dbprintf(_("null directory block number, cannot set new addr\n"));
		return;
	}
	nex = mp->m_dir_geo->fsbcount;
	bmp = malloc(nex * sizeof(*bmp));
	bmap(bno, mp->m_dir_geo->fsbcount, XFS_DATA_FORK, &nex, bmp);
	if (nex == 0) {
		dbprintf(_("directory block is unmapped\n"));
		free(bmp);
		return;
	}
	dfsbno = bmp->startblock + (bno - bmp->startoff);
	ASSERT(typtab[next].typnm == next);
	if (nex > 1)
		make_bbmap(&bbmap, nex, bmp);
	set_cur(&typtab[next], (int64_t)XFS_FSB_TO_DADDR(mp, dfsbno),
		XFS_FSB_TO_BB(mp, mp->m_dir_geo->fsbcount), DB_RING_ADD,
		nex > 1 ? &bbmap : NULL);
	free(bmp);
}

void
fa_drfsbno(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	xfs_rfsblock_t	bno;

	bno = (xfs_rfsblock_t)getbitval(obj, bit, bitsz(bno), BVUNSIGNED);
	if (bno == NULLRFSBLOCK) {
		dbprintf(_("null block number, cannot set new addr\n"));
		return;
	}
	ASSERT(typtab[next].typnm == next);
	set_cur(&typtab[next], (int64_t)XFS_FSB_TO_BB(mp, bno), blkbb,
		DB_RING_ADD, NULL);
}

/*ARGSUSED*/
void
fa_drtbno(
	void	*obj,
	int	bit,
	typnm_t	next)
{
	xfs_rtblock_t	bno;

	bno = (xfs_rtblock_t)getbitval(obj, bit, bitsz(bno), BVUNSIGNED);
	if (bno == NULLRTBLOCK) {
		dbprintf(_("null block number, cannot set new addr\n"));
		return;
	}
	/* need set_cur to understand rt subvolume */
}

/*ARGSUSED*/
void
fa_ino(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	xfs_ino_t	ino;

	ASSERT(next == TYP_INODE);
	ino = (xfs_ino_t)getbitval(obj, bit, bitsz(ino), BVUNSIGNED);
	if (ino == NULLFSINO) {
		dbprintf(_("null inode number, cannot set new addr\n"));
		return;
	}
	set_cur_inode(ino);
}

void
fa_ino4(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	xfs_ino_t	ino;

	ASSERT(next == TYP_INODE);
	ino = (xfs_ino_t)getbitval(obj, bit, bitize(XFS_INO32_SIZE),
			BVUNSIGNED);
	if (ino == NULLFSINO) {
		dbprintf(_("null inode number, cannot set new addr\n"));
		return;
	}
	set_cur_inode(ino);
}

void
fa_ino8(
	void		*obj,
	int		bit,
	typnm_t		next)
{
	xfs_ino_t	ino;

	ASSERT(next == TYP_INODE);
	ino = (xfs_ino_t)getbitval(obj, bit, bitize(XFS_INO64_SIZE),
			BVUNSIGNED);
	if (ino == NULLFSINO) {
		dbprintf(_("null inode number, cannot set new addr\n"));
		return;
	}
	set_cur_inode(ino);
}
