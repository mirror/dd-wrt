/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
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

#include <xfs/libxfs.h>
#include <sys/time.h>
#include "bmap.h"
#include "command.h"
#include "frag.h"
#include "io.h"
#include "output.h"
#include "type.h"
#include "init.h"
#include "malloc.h"

typedef struct extent {
	xfs_fileoff_t	startoff;
	xfs_filblks_t	blockcount;
} extent_t;

typedef	struct extmap {
	int		naents;
	int		nents;
	extent_t	ents[1];
} extmap_t;
#define	EXTMAP_SIZE(n)	\
	(offsetof(extmap_t, ents) + (sizeof(extent_t) * (n)))

static int		aflag;
static int		dflag;
static __uint64_t	extcount_actual;
static __uint64_t	extcount_ideal;
static int		fflag;
static int		lflag;
static int		qflag;
static int		Rflag;
static int		rflag;
static int		vflag;

typedef void	(*scan_lbtree_f_t)(struct xfs_btree_block *block,
				   int			level,
				   extmap_t		**extmapp,
				   typnm_t		btype);

typedef void	(*scan_sbtree_f_t)(struct xfs_btree_block *block,
				   int			level,
				   xfs_agf_t		*agf);

static extmap_t		*extmap_alloc(xfs_extnum_t nex);
static xfs_extnum_t	extmap_ideal(extmap_t *extmap);
static void		extmap_set_ext(extmap_t **extmapp, xfs_fileoff_t o,
				       xfs_extlen_t c);
static int		frag_f(int argc, char **argv);
static int		init(int argc, char **argv);
static void		process_bmbt_reclist(xfs_bmbt_rec_t *rp, int numrecs,
					     extmap_t **extmapp);
static void		process_btinode(xfs_dinode_t *dip, extmap_t **extmapp,
					int whichfork);
static void		process_exinode(xfs_dinode_t *dip, extmap_t **extmapp,
					int whichfork);
static void		process_fork(xfs_dinode_t *dip, int whichfork);
static void		process_inode(xfs_agf_t *agf, xfs_agino_t agino,
				      xfs_dinode_t *dip);
static void		scan_ag(xfs_agnumber_t agno);
static void		scan_lbtree(xfs_fsblock_t root, int nlevels,
				    scan_lbtree_f_t func, extmap_t **extmapp,
				    typnm_t btype);
static void		scan_sbtree(xfs_agf_t *agf, xfs_agblock_t root,
				    int nlevels, scan_sbtree_f_t func,
				    typnm_t btype);
static void		scanfunc_bmap(struct xfs_btree_block *block, int level,
				      extmap_t **extmapp, typnm_t btype);
static void		scanfunc_ino(struct xfs_btree_block *block, int level,
				     xfs_agf_t *agf);

static const cmdinfo_t	frag_cmd =
	{ "frag", NULL, frag_f, 0, -1, 0,
	  "[-a] [-d] [-f] [-l] [-q] [-R] [-r] [-v]",
	  "get file fragmentation data", NULL };

static extmap_t *
extmap_alloc(
	xfs_extnum_t	nex)
{
	extmap_t	*extmap;

	if (nex < 1)
		nex = 1;
	extmap = xmalloc(EXTMAP_SIZE(nex));
	extmap->naents = nex;
	extmap->nents = 0;
	return extmap;
}

static xfs_extnum_t
extmap_ideal(
	extmap_t	*extmap)
{
	extent_t	*ep;
	xfs_extnum_t	rval;

	for (ep = &extmap->ents[0], rval = 0;
	     ep < &extmap->ents[extmap->nents];
	     ep++) {
		if (ep == &extmap->ents[0] ||
		    ep->startoff != ep[-1].startoff + ep[-1].blockcount)
			rval++;
	}
	return rval;
}

static void
extmap_set_ext(
	extmap_t	**extmapp,
	xfs_fileoff_t	o,
	xfs_extlen_t	c)
{
	extmap_t	*extmap;
	extent_t	*ent;

	extmap = *extmapp;
	if (extmap->nents == extmap->naents) {
		extmap->naents++;
		extmap = xrealloc(extmap, EXTMAP_SIZE(extmap->naents));
		*extmapp = extmap;
	}
	ent = &extmap->ents[extmap->nents];
	ent->startoff = o;
	ent->blockcount = c;
	extmap->nents++;
}

void
frag_init(void)
{
	add_command(&frag_cmd);
}

/*
 * Get file fragmentation information.
 */
static int
frag_f(
	int		argc,
	char		**argv)
{
	xfs_agnumber_t	agno;
	double		answer;

	if (!init(argc, argv))
		return 0;
	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++)
		scan_ag(agno);
	if (extcount_actual)
		answer = (double)(extcount_actual - extcount_ideal) * 100.0 /
			 (double)extcount_actual;
	else
		answer = 0.0;
	dbprintf(_("actual %llu, ideal %llu, fragmentation factor %.2f%%\n"),
		extcount_actual, extcount_ideal, answer);
	return 0;
}

static int
init(
	int		argc,
	char		**argv)
{
	int		c;

	aflag = dflag = fflag = lflag = qflag = Rflag = rflag = vflag = 0;
	optind = 0;
	while ((c = getopt(argc, argv, "adflqRrv")) != EOF) {
		switch (c) {
		case 'a':
			aflag = 1;
			break;
		case 'd':
			dflag = 1;
			break;
		case 'f':
			fflag = 1;
			break;
		case 'l':
			lflag = 1;
			break;
		case 'q':
			qflag = 1;
			break;
		case 'R':
			Rflag = 1;
			break;
		case 'r':
			rflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		default:
			dbprintf(_("bad option for frag command\n"));
			return 0;
		}
	}
	if (!aflag && !dflag && !fflag && !lflag && !qflag && !Rflag && !rflag)
		aflag = dflag = fflag = lflag = qflag = Rflag = rflag = 1;
	extcount_actual = extcount_ideal = 0;
	return 1;
}

static void
process_bmbt_reclist(
	xfs_bmbt_rec_t		*rp,
	int			numrecs,
	extmap_t		**extmapp)
{
	xfs_dfilblks_t		c;
	int			f;
	int			i;
	xfs_dfiloff_t		o;
	xfs_dfsbno_t		s;

	for (i = 0; i < numrecs; i++, rp++) {
		convert_extent(rp, &o, &s, &c, &f);
		extmap_set_ext(extmapp, (xfs_fileoff_t)o, (xfs_extlen_t)c);
	}
}

static void
process_btinode(
	xfs_dinode_t		*dip,
	extmap_t		**extmapp,
	int			whichfork)
{
	xfs_bmdr_block_t	*dib;
	int			i;
	xfs_bmbt_ptr_t		*pp;

	dib = (xfs_bmdr_block_t *)XFS_DFORK_PTR(dip, whichfork);
	if (be16_to_cpu(dib->bb_level) == 0) {
		xfs_bmbt_rec_t		*rp = XFS_BMDR_REC_ADDR(dib, 1);
		process_bmbt_reclist(rp, be16_to_cpu(dib->bb_numrecs), extmapp);
		return;
	}
	pp = XFS_BMDR_PTR_ADDR(dib, 1,
		xfs_bmdr_maxrecs(mp, XFS_DFORK_SIZE(dip, mp, whichfork), 0));
	for (i = 0; i < be16_to_cpu(dib->bb_numrecs); i++)
		scan_lbtree(be64_to_cpu(pp[i]), be16_to_cpu(dib->bb_level), 
			scanfunc_bmap, extmapp,
			whichfork == XFS_DATA_FORK ? TYP_BMAPBTD : TYP_BMAPBTA);
}

static void
process_exinode(
	xfs_dinode_t		*dip,
	extmap_t		**extmapp,
	int			whichfork)
{
	xfs_bmbt_rec_t		*rp;

	rp = (xfs_bmbt_rec_t *)XFS_DFORK_PTR(dip, whichfork);
	process_bmbt_reclist(rp, XFS_DFORK_NEXTENTS(dip, whichfork), extmapp);
}

static void
process_fork(
	xfs_dinode_t	*dip,
	int		whichfork)
{
	extmap_t	*extmap;
	int		nex;

	nex = XFS_DFORK_NEXTENTS(dip, whichfork);
	if (!nex)
		return;
	extmap = extmap_alloc(nex);
	switch (XFS_DFORK_FORMAT(dip, whichfork)) {
	case XFS_DINODE_FMT_EXTENTS:
		process_exinode(dip, &extmap, whichfork);
		break;
	case XFS_DINODE_FMT_BTREE:
		process_btinode(dip, &extmap, whichfork);
		break;
	}
	extcount_actual += extmap->nents;
	extcount_ideal += extmap_ideal(extmap);
	xfree(extmap);
}

static void
process_inode(
	xfs_agf_t		*agf,
	xfs_agino_t		agino,
	xfs_dinode_t		*dip)
{
	__uint64_t		actual;
	__uint64_t		ideal;
	xfs_ino_t		ino;
	int			skipa;
	int			skipd;

	ino = XFS_AGINO_TO_INO(mp, be32_to_cpu(agf->agf_seqno), agino);
	switch (be16_to_cpu(dip->di_mode) & S_IFMT) {
	case S_IFDIR:
		skipd = !dflag;
		break;
	case S_IFREG:
		if (!rflag && (be16_to_cpu(dip->di_flags) & XFS_DIFLAG_REALTIME))
			skipd = 1;
		else if (!Rflag &&
			 (ino == mp->m_sb.sb_rbmino ||
			  ino == mp->m_sb.sb_rsumino))
			skipd = 1;
		else if (!qflag &&
			 (ino == mp->m_sb.sb_uquotino ||
			  ino == mp->m_sb.sb_gquotino))
			skipd = 1;
		else
			skipd = !fflag;
		break;
	case S_IFLNK:
		skipd = !lflag;
		break;
	default:
		skipd = 1;
		break;
	}
	actual = extcount_actual;
	ideal = extcount_ideal;
	if (!skipd)
		process_fork(dip, XFS_DATA_FORK);
	skipa = !aflag || !XFS_DFORK_Q(dip);
	if (!skipa)
		process_fork(dip, XFS_ATTR_FORK);
	if (vflag && (!skipd || !skipa))
		dbprintf(_("inode %lld actual %lld ideal %lld\n"),
			ino, extcount_actual - actual, extcount_ideal - ideal);
}

static void
scan_ag(
	xfs_agnumber_t	agno)
{
	xfs_agf_t	*agf;
	xfs_agi_t	*agi;

	push_cur();
	set_cur(&typtab[TYP_AGF],
		XFS_AG_DADDR(mp, agno, XFS_AGF_DADDR(mp)),
		XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);
	if ((agf = iocur_top->data) == NULL) {
		dbprintf(_("can't read agf block for ag %u\n"), agno);
		pop_cur();
		return;
	}
	push_cur();
	set_cur(&typtab[TYP_AGI],
		XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)),
		XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);
	if ((agi = iocur_top->data) == NULL) {
		dbprintf(_("can't read agi block for ag %u\n"), agno);
		pop_cur();
		pop_cur();
		return;
	}
	scan_sbtree(agf, be32_to_cpu(agi->agi_root), 
			be32_to_cpu(agi->agi_level), scanfunc_ino, TYP_INOBT);
	pop_cur();
	pop_cur();
}

static void
scan_lbtree(
	xfs_fsblock_t	root,
	int		nlevels,
	scan_lbtree_f_t	func,
	extmap_t	**extmapp,
	typnm_t		btype)
{
	push_cur();
	set_cur(&typtab[btype], XFS_FSB_TO_DADDR(mp, root), blkbb, DB_RING_IGN,
		NULL);
	if (iocur_top->data == NULL) {
		dbprintf(_("can't read btree block %u/%u\n"),
			XFS_FSB_TO_AGNO(mp, root),
			XFS_FSB_TO_AGBNO(mp, root));
		return;
	}
	(*func)(iocur_top->data, nlevels - 1, extmapp, btype);
	pop_cur();
}

static void
scan_sbtree(
	xfs_agf_t	*agf,
	xfs_agblock_t	root,
	int		nlevels,
	scan_sbtree_f_t	func,
	typnm_t		btype)
{
	xfs_agnumber_t	seqno = be32_to_cpu(agf->agf_seqno);

	push_cur();
	set_cur(&typtab[btype], XFS_AGB_TO_DADDR(mp, seqno, root),
		blkbb, DB_RING_IGN, NULL);
	if (iocur_top->data == NULL) {
		dbprintf(_("can't read btree block %u/%u\n"), seqno, root);
		return;
	}
	(*func)(iocur_top->data, nlevels - 1, agf);
	pop_cur();
}

static void
scanfunc_bmap(
	struct xfs_btree_block	*block,
	int			level,
	extmap_t		**extmapp,
	typnm_t			btype)
{
	int			i;
	xfs_bmbt_ptr_t		*pp;
	xfs_bmbt_rec_t		*rp;
	int			nrecs;

	nrecs = be16_to_cpu(block->bb_numrecs);

	if (level == 0) {
		if (nrecs > mp->m_bmap_dmxr[0]) {
			dbprintf(_("invalid numrecs (%u) in %s block\n"),
				   nrecs, typtab[btype].name);
			return;
		}
		rp = XFS_BMBT_REC_ADDR(mp, block, 1);
		process_bmbt_reclist(rp, nrecs, extmapp);
		return;
	}

	if (nrecs > mp->m_bmap_dmxr[1]) {
		dbprintf(_("invalid numrecs (%u) in %s block\n"),
			   nrecs, typtab[btype].name);
		return;
	}
	pp = XFS_BMBT_PTR_ADDR(mp, block, 1, mp->m_bmap_dmxr[0]);
	for (i = 0; i < nrecs; i++)
		scan_lbtree(be64_to_cpu(pp[i]), level, scanfunc_bmap, extmapp, 
									btype);
}

static void
scanfunc_ino(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agf_t		*agf)
{
	xfs_agino_t		agino;
	xfs_agnumber_t		seqno = be32_to_cpu(agf->agf_seqno);
	int			i;
	int			j;
	int			off;
	xfs_inobt_ptr_t		*pp;
	xfs_inobt_rec_t		*rp;

	if (level == 0) {
		rp = XFS_INOBT_REC_ADDR(mp, block, 1);
		for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++) {
			agino = be32_to_cpu(rp[i].ir_startino);
			off = XFS_INO_TO_OFFSET(mp, agino);
			push_cur();
			set_cur(&typtab[TYP_INODE],
				XFS_AGB_TO_DADDR(mp, seqno,
						 XFS_AGINO_TO_AGBNO(mp, agino)),
				XFS_FSB_TO_BB(mp, XFS_IALLOC_BLOCKS(mp)),
				DB_RING_IGN, NULL);
			if (iocur_top->data == NULL) {
				dbprintf(_("can't read inode block %u/%u\n"),
					seqno, XFS_AGINO_TO_AGBNO(mp, agino));
				continue;
			}
			for (j = 0; j < XFS_INODES_PER_CHUNK; j++) {
				if (XFS_INOBT_IS_FREE_DISK(&rp[i], j))
					continue;
				process_inode(agf, agino + j, (xfs_dinode_t *)
					((char *)iocur_top->data + 
					((off + j) << mp->m_sb.sb_inodelog)));
			}
			pop_cur();
		}
		return;
	}
	pp = XFS_INOBT_PTR_ADDR(mp, block, 1, mp->m_inobt_mxr[1]);
	for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++)
		scan_sbtree(agf, be32_to_cpu(pp[i]), level, scanfunc_ino, 
								TYP_INOBT);
}
