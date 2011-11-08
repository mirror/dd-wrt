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

#include <xfs/libxfs.h>
#include "command.h"
#include "type.h"
#include "fprint.h"
#include "faddr.h"
#include "field.h"
#include "bmap.h"
#include "io.h"
#include "inode.h"
#include "output.h"
#include "init.h"

static int		bmap_f(int argc, char **argv);
static int		bmap_one_extent(xfs_bmbt_rec_t *ep,
					xfs_dfiloff_t *offp, xfs_dfiloff_t eoff,
					int *idxp, bmap_ext_t *bep);
static xfs_fsblock_t	select_child(xfs_dfiloff_t off, xfs_bmbt_key_t *kp,
				     xfs_bmbt_ptr_t *pp, int nrecs);

static const cmdinfo_t	bmap_cmd =
	{ "bmap", NULL, bmap_f, 0, 3, 0, N_("[-ad] [block [len]]"),
	  N_("show block map for current file"), NULL };

void
bmap(
	xfs_dfiloff_t		offset,
	xfs_dfilblks_t		len,
	int			whichfork,
	int			*nexp,
	bmap_ext_t		*bep)
{
	struct xfs_btree_block	*block;
	xfs_fsblock_t		bno;
	xfs_dfiloff_t		curoffset;
	xfs_dinode_t		*dip;
	xfs_dfiloff_t		eoffset;
	xfs_bmbt_rec_t		*ep;
	xfs_dinode_fmt_t	fmt;
	int			fsize;
	xfs_bmbt_key_t		*kp;
	int			n;
	int			nex;
	xfs_fsblock_t		nextbno;
	int			nextents;
	xfs_bmbt_ptr_t		*pp;
	xfs_bmdr_block_t	*rblock;
	typnm_t			typ;
	xfs_bmbt_rec_t		*xp;

	push_cur();
	set_cur_inode(iocur_top->ino);
	nex = *nexp;
	*nexp = 0;
	ASSERT(nex > 0);
	dip = iocur_top->data;
	n = 0;
	eoffset = offset + len - 1;
	curoffset = offset;
	fmt = (xfs_dinode_fmt_t)XFS_DFORK_FORMAT(dip, whichfork);
	typ = whichfork == XFS_DATA_FORK ? TYP_BMAPBTD : TYP_BMAPBTA;
	ASSERT(typtab[typ].typnm == typ);
	ASSERT(fmt == XFS_DINODE_FMT_LOCAL || fmt == XFS_DINODE_FMT_EXTENTS ||
		fmt == XFS_DINODE_FMT_BTREE);
	if (fmt == XFS_DINODE_FMT_EXTENTS) {
		nextents = XFS_DFORK_NEXTENTS(dip, whichfork);
		xp = (xfs_bmbt_rec_t *)XFS_DFORK_PTR(dip, whichfork);
		for (ep = xp; ep < &xp[nextents] && n < nex; ep++) {
			if (!bmap_one_extent(ep, &curoffset, eoffset, &n, bep))
				break;
		}
	} else if (fmt == XFS_DINODE_FMT_BTREE) {
		push_cur();
		bno = NULLFSBLOCK;
		rblock = (xfs_bmdr_block_t *)XFS_DFORK_PTR(dip, whichfork);
		fsize = XFS_DFORK_SIZE(dip, mp, whichfork);
		pp = XFS_BMDR_PTR_ADDR(rblock, 1, xfs_bmdr_maxrecs(mp, fsize, 0));
		kp = XFS_BMDR_KEY_ADDR(rblock, 1);
		bno = select_child(curoffset, kp, pp, 
					be16_to_cpu(rblock->bb_numrecs));
		for (;;) {
			set_cur(&typtab[typ], XFS_FSB_TO_DADDR(mp, bno),
				blkbb, DB_RING_IGN, NULL);
			block = (struct xfs_btree_block *)iocur_top->data;
			if (be16_to_cpu(block->bb_level) == 0)
				break;
			pp = XFS_BMDR_PTR_ADDR(block, 1,
				xfs_bmbt_maxrecs(mp, mp->m_sb.sb_blocksize, 0));
			kp = XFS_BMDR_KEY_ADDR(block, 1);
			bno = select_child(curoffset, kp, pp,
					be16_to_cpu(block->bb_numrecs));
		}
		for (;;) {
			nextbno = be64_to_cpu(block->bb_u.l.bb_rightsib);
			nextents = be16_to_cpu(block->bb_numrecs);
			xp = (xfs_bmbt_rec_t *)
				XFS_BMBT_REC_ADDR(mp, block, 1);
			for (ep = xp; ep < &xp[nextents] && n < nex; ep++) {
				if (!bmap_one_extent(ep, &curoffset, eoffset,
						&n, bep)) {
					nextbno = NULLFSBLOCK;
					break;
				}
			}
			bno = nextbno;
			if (bno == NULLFSBLOCK)
				break;
			set_cur(&typtab[typ], XFS_FSB_TO_DADDR(mp, bno),
				blkbb, DB_RING_IGN, NULL);
			block = (struct xfs_btree_block *)iocur_top->data;
		}
		pop_cur();
	}
	pop_cur();
	*nexp = n;
}

static int
bmap_f(
	int		argc,
	char		**argv)
{
	int		afork = 0;
	bmap_ext_t	be;
	int		c;
	xfs_dfiloff_t	co, cosave;
	int		dfork = 0;
	xfs_dinode_t	*dip;
	xfs_dfiloff_t	eo;
	xfs_dfilblks_t	len;
	int		nex;
	char		*p;
	int		whichfork;

	if (iocur_top->ino == NULLFSINO) {
		dbprintf(_("no current inode\n"));
		return 0;
	}
	optind = 0;
	if (argc) while ((c = getopt(argc, argv, "ad")) != EOF) {
		switch (c) {
		case 'a':
			afork = 1;
			break;
		case 'd':
			dfork = 1;
			break;
		default:
			dbprintf(_("bad option for bmap command\n"));
			return 0;
		}
	}
	if (afork + dfork == 0) {
		push_cur();
		set_cur_inode(iocur_top->ino);
		dip = iocur_top->data;
		if (be32_to_cpu(dip->di_nextents))
			dfork = 1;
		if (be16_to_cpu(dip->di_anextents))
			afork = 1;
		pop_cur();
	}
	if (optind < argc) {
		co = (xfs_dfiloff_t)strtoull(argv[optind], &p, 0);
		if (*p != '\0') {
			dbprintf(_("bad block number for bmap %s\n"),
				argv[optind]);
			return 0;
		}
		optind++;
		if (optind < argc) {
			len = (xfs_dfilblks_t)strtoull(argv[optind], &p, 0);
			if (*p != '\0') {
				dbprintf(_("bad len for bmap %s\n"), argv[optind]);
				return 0;
			}
			eo = co + len - 1;
		} else
			eo = co;
	} else {
		co = 0;
		eo = -1;
	}
	cosave = co;
	for (whichfork = XFS_DATA_FORK;
	     whichfork <= XFS_ATTR_FORK;
	     whichfork++) {
		if (whichfork == XFS_DATA_FORK && !dfork)
			continue;
		if (whichfork == XFS_ATTR_FORK && !afork)
			continue;
		for (;;) {
			nex = 1;
			bmap(co, eo - co + 1, whichfork, &nex, &be);
			if (nex == 0)
				break;
			dbprintf(_("%s offset %lld startblock %llu (%u/%u) count "
				 "%llu flag %u\n"),
				whichfork == XFS_DATA_FORK ? _("data") : _("attr"),
				be.startoff, be.startblock,
				XFS_FSB_TO_AGNO(mp, be.startblock),
				XFS_FSB_TO_AGBNO(mp, be.startblock),
				be.blockcount, be.flag);
			co = be.startoff + be.blockcount;
		}
		co = cosave;
	}
	return 0;
}

void
bmap_init(void)
{
	add_command(&bmap_cmd);
}

static int
bmap_one_extent(
	xfs_bmbt_rec_t		*ep,
	xfs_dfiloff_t		*offp,
	xfs_dfiloff_t		eoff,
	int			*idxp,
	bmap_ext_t		*bep)
{
	xfs_dfilblks_t		c;
	xfs_dfiloff_t		curoffset;
	int			f;
	int			idx;
	xfs_dfiloff_t		o;
	xfs_dfsbno_t		s;

	convert_extent(ep, &o, &s, &c, &f);
	curoffset = *offp;
	idx = *idxp;
	if (o + c <= curoffset)
		return 1;
	if (o > eoff)
		return 0;
	if (o < curoffset) {
		c -= curoffset - o;
		s += curoffset - o;
		o = curoffset;
	}
	if (o + c - 1 > eoff)
		c -= (o + c - 1) - eoff;
	bep[idx].startoff = o;
	bep[idx].startblock = s;
	bep[idx].blockcount = c;
	bep[idx].flag = f;
	*idxp = idx + 1;
	*offp = o + c;
	return 1;
}

void
convert_extent(
	xfs_bmbt_rec_t		*rp,
	xfs_dfiloff_t		*op,
	xfs_dfsbno_t		*sp,
	xfs_dfilblks_t		*cp,
	int			*fp)
{
	xfs_bmbt_irec_t		irec;

	libxfs_bmbt_disk_get_all(rp, &irec);

	*fp = irec.br_state == XFS_EXT_UNWRITTEN;
	*op = irec.br_startoff;
	*sp = irec.br_startblock;
	*cp = irec.br_blockcount;
}

void
make_bbmap(
	bbmap_t		*bbmap,
	int		nex,
	bmap_ext_t	*bmp)
{
	int		d;
	xfs_dfsbno_t	dfsbno;
	int		i;
	int		j;
	int		k;

	for (i = 0, d = 0; i < nex; i++) {
		dfsbno = bmp[i].startblock;
		for (j = 0; j < bmp[i].blockcount; j++, dfsbno++) {
			for (k = 0; k < blkbb; k++)
				bbmap->b[d++] =
					XFS_FSB_TO_DADDR(mp, dfsbno) + k;
		}
	}
}

static xfs_fsblock_t
select_child(
	xfs_dfiloff_t	off,
	xfs_bmbt_key_t	*kp,
	xfs_bmbt_ptr_t	*pp,
	int		nrecs)
{
	int		i;

	for (i = 0; i < nrecs; i++) {
		if (be64_to_cpu(kp[i].br_startoff) == off)
			return be64_to_cpu(pp[i]);
		if (be64_to_cpu(kp[i].br_startoff) > off) {
			if (i == 0)
				return be64_to_cpu(pp[i]);
			else
				return be64_to_cpu(pp[i - 1]);
		}
	}
	return be64_to_cpu(pp[nrecs - 1]);
}
