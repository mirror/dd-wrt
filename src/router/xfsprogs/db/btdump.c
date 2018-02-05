/*
 * Copyright (C) 2017 Oracle.  All Rights Reserved.
 *
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "libxfs.h"
#include "command.h"
#include "output.h"
#include "init.h"
#include "io.h"
#include "type.h"
#include "input.h"

static void
btdump_help(void)
{
	dbprintf(_(
"\n"
" If the cursor points to a btree block, 'btdump' dumps the btree\n"
" downward from that block.  If the cursor points to an inode,\n"
" the data fork btree root is selected by default.  If the cursor\n"
" points to a directory or extended attribute btree node, the tree\n"
" will be printed downward from that block.\n"
"\n"
" Options:\n"
"   -a -- Display an inode's extended attribute fork btree.\n"
"   -i -- Print internal btree nodes.\n"
"\n"
));

}

static int
eval(
	const char	*fmt, ...)
{
	va_list		ap;
	char		buf[PATH_MAX];
	char		**v;
	int		c;
	int		ret;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	v = breakline(buf, &c);
	ret = command(c, v);
	free(v);
	return ret;
}

static bool
btblock_has_rightsib(
	struct xfs_btree_block	*block,
	bool			long_format)
{
	if (long_format)
		return block->bb_u.l.bb_rightsib != cpu_to_be64(NULLFSBLOCK);
	return block->bb_u.s.bb_rightsib != cpu_to_be32(NULLAGBLOCK);
}

static int
dump_btlevel(
	int			level,
	bool			long_format)
{
	xfs_daddr_t		orig_daddr = iocur_top->bb;
	xfs_daddr_t		last_daddr;
	unsigned int		nr;
	int			ret = 0;

	push_cur_and_set_type();

	nr = 1;
	do {
		last_daddr = iocur_top->bb;
		dbprintf(_("%s level %u block %u daddr %llu\n"),
			 iocur_top->typ->name, level, nr, last_daddr);
		if (level > 0) {
			ret = eval("print keys");
			if (ret)
				goto err;
			ret = eval("print ptrs");
		} else {
			ret = eval("print recs");
		}
		if (ret)
			goto err;
		if (btblock_has_rightsib(iocur_top->data, long_format)) {
			ret = eval("addr rightsib");
			if (ret)
				goto err;
		}
		nr++;
	} while (iocur_top->bb != orig_daddr && iocur_top->bb != last_daddr);

err:
	pop_cur();
	return ret;
}

static int
dump_btree(
	bool		dump_node_blocks,
	bool		long_format)
{
	xfs_daddr_t	orig_daddr = iocur_top->bb;
	xfs_daddr_t	last_daddr;
	int		level;
	int		ret = 0;

	push_cur_and_set_type();

	cur_agno = XFS_FSB_TO_AGNO(mp, XFS_DADDR_TO_FSB(mp, iocur_top->bb));
	level = xfs_btree_get_level(iocur_top->data);
	do {
		last_daddr = iocur_top->bb;
		if (level > 0) {
			if (dump_node_blocks) {
				ret = dump_btlevel(level, long_format);
				if (ret)
					goto err;
			}
			ret = eval("addr ptrs[1]");
		} else {
			ret = dump_btlevel(level, long_format);
		}
		if (ret)
			goto err;
		level--;
	} while (level >= 0 &&
		 iocur_top->bb != orig_daddr &&
		 iocur_top->bb != last_daddr);

err:
	pop_cur();
	return ret;
}

static inline int dump_btree_short(bool dump_node_blocks)
{
	return dump_btree(dump_node_blocks, false);
}

static inline int dump_btree_long(bool dump_node_blocks)
{
	return dump_btree(dump_node_blocks, true);
}

static int
dump_inode(
	bool			dump_node_blocks,
	bool			attrfork)
{
	char			*prefix;
	struct xfs_dinode	*dip;
	int			ret = 0;

	if (attrfork)
		prefix = "a.bmbt";
	else if (xfs_sb_version_hascrc(&mp->m_sb))
		prefix = "u3.bmbt";
	else
		prefix = "u.bmbt";

	dip = iocur_top->data;
	if (attrfork) {
		if (!dip->di_anextents ||
		    dip->di_aformat != XFS_DINODE_FMT_BTREE) {
			dbprintf(_("attr fork not in btree format\n"));
			return 0;
		}
	} else {
		if (!dip->di_nextents ||
		    dip->di_format != XFS_DINODE_FMT_BTREE) {
			dbprintf(_("data fork not in btree format\n"));
			return 0;
		}
	}

	push_cur_and_set_type();

	if (dump_node_blocks) {
		ret = eval("print %s.keys", prefix);
		if (ret)
			goto err;
		ret = eval("print %s.ptrs", prefix);
		if (ret)
			goto err;
	}

	ret = eval("addr %s.ptrs[1]", prefix);
	if (ret)
		goto err;

	ret = dump_btree_long(dump_node_blocks);
	if (ret)
		goto err;

err:
	pop_cur();
	return ret;
}

static bool
dir_has_rightsib(
	void				*block,
	int				level)
{
	struct xfs_dir3_icleaf_hdr	lhdr;
	struct xfs_da3_icnode_hdr	nhdr;

	if (level > 0) {
		M_DIROPS(mp)->node_hdr_from_disk(&nhdr, block);
		return nhdr.forw != 0;
	}
	M_DIROPS(mp)->leaf_hdr_from_disk(&lhdr, block);
	return lhdr.forw != 0;
}

static int
dir_level(
	void				*block)
{
	struct xfs_dir3_icleaf_hdr	lhdr;
	struct xfs_da3_icnode_hdr	nhdr;

	switch (((struct xfs_da_intnode *)block)->hdr.info.magic) {
	case cpu_to_be16(XFS_DIR2_LEAF1_MAGIC):
	case cpu_to_be16(XFS_DIR2_LEAFN_MAGIC):
		M_DIROPS(mp)->leaf_hdr_from_disk(&lhdr, block);
		return 0;
	case cpu_to_be16(XFS_DA_NODE_MAGIC):
		M_DIROPS(mp)->node_hdr_from_disk(&nhdr, block);
		return nhdr.level;
	default:
		return -1;
	}
}

static int
dir3_level(
	void				*block)
{
	struct xfs_dir3_icleaf_hdr	lhdr;
	struct xfs_da3_icnode_hdr	nhdr;

	switch (((struct xfs_da_intnode *)block)->hdr.info.magic) {
	case cpu_to_be16(XFS_DIR3_LEAF1_MAGIC):
	case cpu_to_be16(XFS_DIR3_LEAFN_MAGIC):
		M_DIROPS(mp)->leaf_hdr_from_disk(&lhdr, block);
		return 0;
	case cpu_to_be16(XFS_DA3_NODE_MAGIC):
		M_DIROPS(mp)->node_hdr_from_disk(&nhdr, block);
		return nhdr.level;
	default:
		return -1;
	}
}

static bool
attr_has_rightsib(
	void				*block,
	int				level)
{
        struct xfs_attr_leafblock	lhdr;
	struct xfs_da3_icnode_hdr	nhdr;

	if (level > 0) {
		M_DIROPS(mp)->node_hdr_from_disk(&nhdr, block);
		return nhdr.forw != 0;
	}
	xfs_attr3_leaf_hdr_to_disk(mp->m_attr_geo, &lhdr, block);
	return lhdr.hdr.info.forw != 0;
}

static int
attr_level(
	void				*block)
{
	struct xfs_attr_leafblock	lhdr;
	struct xfs_da3_icnode_hdr	nhdr;

	switch (((struct xfs_da_intnode *)block)->hdr.info.magic) {
	case cpu_to_be16(XFS_ATTR_LEAF_MAGIC):
		xfs_attr3_leaf_hdr_to_disk(mp->m_attr_geo, &lhdr, block);
		return 0;
	case cpu_to_be16(XFS_DA_NODE_MAGIC):
		M_DIROPS(mp)->node_hdr_from_disk(&nhdr, block);
		return nhdr.level;
	default:
		return -1;
	}
}

static int
attr3_level(
	void				*block)
{
	struct xfs_attr_leafblock	lhdr;
	struct xfs_da3_icnode_hdr	nhdr;

	switch (((struct xfs_da_intnode *)block)->hdr.info.magic) {
	case cpu_to_be16(XFS_ATTR3_LEAF_MAGIC):
		xfs_attr3_leaf_hdr_to_disk(mp->m_attr_geo, &lhdr, block);
		return 0;
	case cpu_to_be16(XFS_DA3_NODE_MAGIC):
		M_DIROPS(mp)->node_hdr_from_disk(&nhdr, block);
		return nhdr.level;
	default:
		return -1;
	}
}

struct dabprinter_ops {
	const char		*print_node_entries;
	const char		*print_leaf_entries;
	const char		*go_node_forward;
	const char		*go_leaf_forward;
	const char		*go_down;
	bool			(*has_rightsib)(void *, int);
	int			(*level)(void *);
};

static struct dabprinter_ops attr_print = {
	.print_node_entries	= "btree",
	.print_leaf_entries	= "entries nvlist",
	.go_node_forward	= "hdr.info.forw",
	.go_leaf_forward	= "hdr.info.forw",
	.go_down		= "btree[0].before",
	.has_rightsib		= attr_has_rightsib,
	.level			= attr_level,
};

static struct dabprinter_ops attr3_print = {
	.print_node_entries	= "btree",
	.print_leaf_entries	= "entries nvlist",
	.go_node_forward	= "hdr.info.hdr.forw",
	.go_leaf_forward	= "hdr.info.hdr.forw",
	.go_down		= "btree[0].before",
	.has_rightsib		= attr_has_rightsib,
	.level			= attr3_level,
};

static struct dabprinter_ops dir_print = {
	.print_node_entries	= "nbtree",
	.print_leaf_entries	= "lents",
	.go_node_forward	= "nhdr.info.hdr.forw",
	.go_leaf_forward	= "lhdr.info.hdr.forw",
	.go_down		= "nbtree[0].before",
	.has_rightsib		= dir_has_rightsib,
	.level			= dir_level,
};

static struct dabprinter_ops dir3_print = {
	.print_node_entries	= "nbtree",
	.print_leaf_entries	= "lents",
	.go_node_forward	= "nhdr.info.forw",
	.go_leaf_forward	= "lhdr.info.forw",
	.go_down		= "nbtree[0].before",
	.has_rightsib		= dir_has_rightsib,
	.level			= dir3_level,
};

static int
dump_dablevel(
	int			level,
	struct dabprinter_ops	*dbp)
{
	xfs_daddr_t		orig_daddr = iocur_top->bb;
	xfs_daddr_t		last_daddr;
	unsigned int		nr;
	int			ret = 0;

	push_cur_and_set_type();

	nr = 1;
	do {
		last_daddr = iocur_top->bb;
		dbprintf(_("%s level %u block %u daddr %llu\n"),
			 iocur_top->typ->name, level, nr, last_daddr);
		ret = eval("print %s", level > 0 ? dbp->print_node_entries :
						   dbp->print_leaf_entries);
		if (ret)
			goto err;
		if (dbp->has_rightsib(iocur_top->data, level)) {
			ret = eval("addr %s", level > 0 ? dbp->go_node_forward :
							  dbp->go_leaf_forward);
			if (ret)
				goto err;
		}
		nr++;
	} while (iocur_top->bb != orig_daddr && iocur_top->bb != last_daddr);

err:
	pop_cur();
	return ret;
}

static int
dump_dabtree(
	bool				dump_node_blocks,
	struct dabprinter_ops		*dbp)
{
	xfs_daddr_t			orig_daddr = iocur_top->bb;
	xfs_daddr_t			last_daddr;
	int				level;
	int				ret = 0;

	push_cur_and_set_type();

	cur_agno = XFS_FSB_TO_AGNO(mp, XFS_DADDR_TO_FSB(mp, iocur_top->bb));
	level = dbp->level(iocur_top->data);
	if (level < 0) {
		printf(_("Current location is not part of a dir/attr btree.\n"));
		goto err;
	}

	do {
		last_daddr = iocur_top->bb;
		if (level > 0) {
			if (dump_node_blocks) {
				ret = dump_dablevel(level, dbp);
				if (ret)
					goto err;
			}
			ret = eval("addr %s", dbp->go_down);
		} else {
			ret = dump_dablevel(level, dbp);
		}
		if (ret)
			goto err;
		level--;
	} while (level >= 0 &&
		 iocur_top->bb != orig_daddr &&
		 iocur_top->bb != last_daddr);

err:
	pop_cur();
	return ret;
}

static int
btdump_f(
	int		argc,
	char		**argv)
{
	bool		aflag = false;
	bool		iflag = false;
	bool		crc = xfs_sb_version_hascrc(&mp->m_sb);
	int		c;

	if (cur_typ == NULL) {
		dbprintf(_("no current type\n"));
		return 0;
	}
	while ((c = getopt(argc, argv, "ai")) != EOF) {
		switch (c) {
		case 'a':
			aflag = true;
			break;
		case 'i':
			iflag = true;
			break;
		default:
			dbprintf(_("bad option for btdump command\n"));
			return 0;
		}
	}

	if (optind != argc) {
		dbprintf(_("bad options for btdump command\n"));
		return 0;
	}
	if (aflag && cur_typ->typnm != TYP_INODE) {
		dbprintf(_("attrfork flag doesn't apply here\n"));
		return 0;
	}

	switch (cur_typ->typnm) {
	case TYP_BNOBT:
	case TYP_CNTBT:
	case TYP_INOBT:
	case TYP_FINOBT:
	case TYP_RMAPBT:
	case TYP_REFCBT:
		return dump_btree_short(iflag);
	case TYP_BMAPBTA:
	case TYP_BMAPBTD:
		return dump_btree_long(iflag);
	case TYP_INODE:
		return dump_inode(iflag, aflag);
	case TYP_ATTR:
		return dump_dabtree(iflag, crc ? &attr3_print : &attr_print);
	case TYP_DIR2:
		return dump_dabtree(iflag, crc ? &dir3_print : &dir_print);
	default:
		dbprintf(_("type \"%s\" is not a btree type or inode\n"),
				cur_typ->name);
		return 0;
	}
}

static const cmdinfo_t btdump_cmd =
	{ "btdump", "b", btdump_f, 0, 2, 0, "[-a] [-i]",
	  N_("dump btree"), btdump_help };

void
btdump_init(void)
{
	add_command(&btdump_cmd);
}
