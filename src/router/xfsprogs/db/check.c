// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include <math.h>
#include <sys/time.h>
#include "bmap.h"
#include "check.h"
#include "command.h"
#include "io.h"
#include "type.h"
#include "fprint.h"
#include "faddr.h"
#include "field.h"
#include "sb.h"
#include "output.h"
#include "init.h"
#include "malloc.h"
#include "dir2.h"

typedef enum {
	IS_USER_QUOTA, IS_PROJECT_QUOTA, IS_GROUP_QUOTA,
} qtype_t;

typedef enum {
	DBM_UNKNOWN,	DBM_AGF,	DBM_AGFL,	DBM_AGI,
	DBM_ATTR,	DBM_BTBMAPA,	DBM_BTBMAPD,	DBM_BTBNO,
	DBM_BTCNT,	DBM_BTINO,	DBM_DATA,	DBM_DIR,
	DBM_FREE1,	DBM_FREE2,	DBM_FREELIST,	DBM_INODE,
	DBM_LOG,	DBM_MISSING,	DBM_QUOTA,	DBM_RTBITMAP,
	DBM_RTDATA,	DBM_RTFREE,	DBM_RTSUM,	DBM_SB,
	DBM_SYMLINK,	DBM_BTFINO,	DBM_BTRMAP,	DBM_BTREFC,
	DBM_RLDATA,	DBM_COWDATA,
	DBM_NDBM
} dbm_t;

typedef struct inodata {
	struct inodata	*next;
	nlink_t		link_set;
	nlink_t		link_add;
	bool		isdir;
	bool		isreflink;
	char		security;
	char		ilist;
	xfs_ino_t	ino;
	struct inodata	*parent;
	char		*name;
} inodata_t;
#define	MIN_INODATA_HASH_SIZE	256
#define	MAX_INODATA_HASH_SIZE	65536
#define	INODATA_AVG_HASH_LENGTH	8

typedef struct qinfo {
	xfs_qcnt_t	bc;
	xfs_qcnt_t	ic;
	xfs_qcnt_t	rc;
} qinfo_t;

#define	QDATA_HASH_SIZE	256
typedef	struct qdata {
	struct qdata	*next;
	xfs_dqid_t	id;
	qinfo_t		count;
	qinfo_t		dq;
} qdata_t;

typedef struct blkent {
	xfs_fileoff_t	startoff;
	int		nblks;
	xfs_fsblock_t	blks[1];
} blkent_t;
#define	BLKENT_SIZE(n)	\
	(offsetof(blkent_t, blks) + (sizeof(xfs_fsblock_t) * (n)))

typedef	struct blkmap {
	int		naents;
	int		nents;
	blkent_t	*ents[1];
} blkmap_t;
#define	BLKMAP_SIZE(n)	\
	(offsetof(blkmap_t, ents) + (sizeof(blkent_t *) * (n)))

typedef struct freetab {
	int			naents;
	int			nents;
	xfs_dir2_data_off_t	ents[1];
} freetab_t;
#define	FREETAB_SIZE(n)	\
	(offsetof(freetab_t, ents) + (sizeof(xfs_dir2_data_off_t) * (n)))

typedef struct dirhash {
	struct dirhash		*next;
	__u32			hashval;
	__u32			address;
	int			seen;
} dirhash_t;
#define	DIR_HASH_SIZE	1024
#define	DIR_HASH_FUNC(h,a)	(((h) ^ (a)) % DIR_HASH_SIZE)

static xfs_extlen_t	agffreeblks;
static xfs_extlen_t	agflongest;
static uint64_t	agf_aggr_freeblks;	/* aggregate count over all */
static uint32_t	agfbtreeblks;
static int		lazycount;
static xfs_agino_t	agicount;
static xfs_agino_t	agifreecount;
static xfs_fsblock_t	*blist;
static int		blist_size;
static char		**dbmap;	/* really dbm_t:8 */
static dirhash_t	**dirhash;
static int		error;
static uint64_t	fdblocks;
static uint64_t	frextents;
static uint64_t	icount;
static uint64_t	ifree;
static inodata_t	***inodata;
static int		inodata_hash_size;
static inodata_t	***inomap;
static int		nflag;
static int		pflag;
static int		tflag;
static qdata_t		**qpdata;
static int		qpdo;
static qdata_t		**qudata;
static int		qudo;
static qdata_t		**qgdata;
static int		qgdo;
static unsigned		sbversion;
static int		sbver_err;
static int		serious_error;
static int		sflag;
static xfs_suminfo_t	*sumcompute;
static xfs_suminfo_t	*sumfile;
static const char	*typename[] = {
	"unknown",
	"agf",
	"agfl",
	"agi",
	"attr",
	"btbmapa",
	"btbmapd",
	"btbno",
	"btcnt",
	"btino",
	"data",
	"dir",
	"free1",
	"free2",
	"freelist",
	"inode",
	"log",
	"missing",
	"quota",
	"rtbitmap",
	"rtdata",
	"rtfree",
	"rtsum",
	"sb",
	"symlink",
	"btfino",
	"btrmap",
	"btrefcnt",
	"rldata",
	"cowdata",
	NULL
};

/*
 * Make sure typename has the same number of elements as there are DBM types.
 * This function isn't called anywhere; we just use it to trip up the compiler.
 */
static inline void check_typename(void)
{
	BUILD_BUG_ON(ARRAY_SIZE(typename) != DBM_NDBM + 1);
}

static int		verbose;

#define	CHECK_BLIST(b)	(blist_size && check_blist(b))
#define	CHECK_BLISTA(a,b)	\
	(blist_size && check_blist(XFS_AGB_TO_FSB(mp, a, b)))

typedef void	(*scan_lbtree_f_t)(struct xfs_btree_block *block,
				   int			level,
				   dbm_t		type,
				   xfs_fsblock_t	bno,
				   inodata_t		*id,
				   xfs_rfsblock_t	*totd,
				   xfs_rfsblock_t	*toti,
				   xfs_extnum_t		*nex,
				   blkmap_t		**blkmapp,
				   int			isroot,
				   typnm_t		btype);

typedef void	(*scan_sbtree_f_t)(struct xfs_btree_block *block,
				   int			level,
				   xfs_agf_t		*agf,
				   xfs_agblock_t	bno,
				   int			isroot);

static void		add_blist(xfs_fsblock_t	bno);
static void		add_ilist(xfs_ino_t ino);
static void		addlink_inode(inodata_t *id);
static void		addname_inode(inodata_t *id, char *name, int namelen);
static void		addparent_inode(inodata_t *id, xfs_ino_t parent);
static void		blkent_append(blkent_t **entp, xfs_fsblock_t b,
				      xfs_extlen_t c);
static blkent_t		*blkent_new(xfs_fileoff_t o, xfs_fsblock_t b,
				    xfs_extlen_t c);
static void		blkent_prepend(blkent_t **entp, xfs_fsblock_t b,
				       xfs_extlen_t c);
static blkmap_t		*blkmap_alloc(xfs_extnum_t);
static void		blkmap_free(blkmap_t *blkmap);
static xfs_fsblock_t	blkmap_get(blkmap_t *blkmap, xfs_fileoff_t o);
static int		blkmap_getn(blkmap_t *blkmap, xfs_fileoff_t o, int nb,
				    bmap_ext_t **bmpp);
static void		blkmap_grow(blkmap_t **blkmapp, blkent_t **entp,
				    blkent_t *newent);
static xfs_fileoff_t	blkmap_next_off(blkmap_t *blkmap, xfs_fileoff_t o,
					int *t);
static void		blkmap_set_blk(blkmap_t **blkmapp, xfs_fileoff_t o,
				       xfs_fsblock_t b);
static void		blkmap_set_ext(blkmap_t **blkmapp, xfs_fileoff_t o,
				       xfs_fsblock_t b, xfs_extlen_t c);
static void		blkmap_shrink(blkmap_t *blkmap, blkent_t **entp);
static int		blockfree_f(int argc, char **argv);
static int		blockget_f(int argc, char **argv);
static int		blocktrash_f(int argc, char **argv);
static int		blockuse_f(int argc, char **argv);
static int		check_blist(xfs_fsblock_t bno);
static void		check_dbmap(xfs_agnumber_t agno, xfs_agblock_t agbno,
				    xfs_extlen_t len, dbm_t type,
				    int ignore_reflink);
static int		check_inomap(xfs_agnumber_t agno, xfs_agblock_t agbno,
				     xfs_extlen_t len, xfs_ino_t c_ino);
static void		check_linkcounts(xfs_agnumber_t agno);
static int		check_range(xfs_agnumber_t agno, xfs_agblock_t agbno,
				    xfs_extlen_t len);
static void		check_rdbmap(xfs_rfsblock_t bno, xfs_extlen_t len,
				     dbm_t type);
static int		check_rinomap(xfs_rfsblock_t bno, xfs_extlen_t len,
				      xfs_ino_t c_ino);
static void		check_rootdir(void);
static int		check_rrange(xfs_rfsblock_t bno, xfs_extlen_t len);
static void		check_set_dbmap(xfs_agnumber_t agno,
					xfs_agblock_t agbno, xfs_extlen_t len,
					dbm_t type1, dbm_t type2,
					xfs_agnumber_t c_agno,
					xfs_agblock_t c_agbno);
static void		check_set_rdbmap(xfs_rfsblock_t bno, xfs_extlen_t len,
					 dbm_t type1, dbm_t type2);
static void		check_summary(void);
static void		checknot_dbmap(xfs_agnumber_t agno, xfs_agblock_t agbno,
				       xfs_extlen_t len, int typemask);
static void		checknot_rdbmap(xfs_rfsblock_t bno, xfs_extlen_t len,
					int typemask);
static void		dir_hash_add(xfs_dahash_t hash,
				     xfs_dir2_dataptr_t addr);
static void		dir_hash_check(inodata_t *id, int v);
static void		dir_hash_done(void);
static void		dir_hash_init(void);
static int		dir_hash_see(xfs_dahash_t hash,
				     xfs_dir2_dataptr_t addr);
static inodata_t	*find_inode(xfs_ino_t ino, int add);
static void		free_inodata(xfs_agnumber_t agno);
static int		init(int argc, char **argv);
static char		*inode_name(xfs_ino_t ino, inodata_t **ipp);
static int		ncheck_f(int argc, char **argv);
static char		*prepend_path(char *oldpath, char *parent);
static xfs_ino_t	process_block_dir_v2(blkmap_t *blkmap, int *dot,
					     int *dotdot, inodata_t *id);
static void		process_bmbt_reclist(xfs_bmbt_rec_t *rp,
					     xfs_extnum_t numrecs, dbm_t type,
					     inodata_t *id, xfs_rfsblock_t *tot,
					     blkmap_t **blkmapp);
static void		process_btinode(inodata_t *id, struct xfs_dinode *dip,
					dbm_t type, xfs_rfsblock_t *totd,
					xfs_rfsblock_t *toti, xfs_extnum_t *nex,
					blkmap_t **blkmapp, int whichfork);
static xfs_ino_t	process_data_dir_v2(int *dot, int *dotdot,
					    inodata_t *id, int v,
					    xfs_dablk_t dabno,
					    freetab_t **freetabp);
static xfs_dir2_data_free_t *process_data_dir_v2_freefind(
					struct xfs_dir2_data_hdr *data,
					struct xfs_dir2_data_unused *dup);
static void		process_dir(struct xfs_dinode *dip, blkmap_t *blkmap,
				    inodata_t *id);
static int		process_dir_v2(struct xfs_dinode *dip, blkmap_t *blkmap,
				       int *dot, int *dotdot, inodata_t *id,
				       xfs_ino_t *parent);
static void		process_exinode(inodata_t *id, struct xfs_dinode *dip,
					dbm_t type, xfs_rfsblock_t *totd,
					xfs_rfsblock_t *toti, xfs_extnum_t *nex,
					blkmap_t **blkmapp, int whichfork);
static void		process_inode(xfs_agf_t *agf, xfs_agino_t agino,
				      struct xfs_dinode *dip, int isfree);
static void		process_lclinode(inodata_t *id, struct xfs_dinode *dip,
					 dbm_t type, xfs_rfsblock_t *totd,
					 xfs_rfsblock_t *toti, xfs_extnum_t *nex,
					 blkmap_t **blkmapp, int whichfork);
static xfs_ino_t	process_leaf_node_dir_v2(blkmap_t *blkmap, int *dot,
						 int *dotdot, inodata_t *id,
						 xfs_fsize_t dirsize);
static void		process_leaf_node_dir_v2_free(inodata_t *id, int v,
						      xfs_dablk_t dbno,
						      freetab_t *freetab);
static void		process_leaf_node_dir_v2_int(inodata_t *id, int v,
						     xfs_dablk_t dbno,
						     freetab_t *freetab);
static void		process_quota(qtype_t qtype, inodata_t *id,
				      blkmap_t *blkmap);
static void		process_rtbitmap(blkmap_t *blkmap);
static void		process_rtsummary(blkmap_t *blkmap);
static xfs_ino_t	process_sf_dir_v2(struct xfs_dinode *dip, int *dot,
					  int *dotdot, inodata_t *id);
static void		quota_add(xfs_dqid_t *p, xfs_dqid_t *g, xfs_dqid_t *u,
				  int dq, xfs_qcnt_t bc, xfs_qcnt_t ic,
				  xfs_qcnt_t rc);
static void		quota_add1(qdata_t **qt, xfs_dqid_t id, int dq,
				   xfs_qcnt_t bc, xfs_qcnt_t ic,
				   xfs_qcnt_t rc);
static void		quota_check(char *s, qdata_t **qt);
static void		quota_init(void);
static void		scan_ag(xfs_agnumber_t agno);
static void		scan_freelist(xfs_agf_t *agf);
static void		scan_lbtree(xfs_fsblock_t root, int nlevels,
				    scan_lbtree_f_t func, dbm_t type,
				    inodata_t *id, xfs_rfsblock_t *totd,
				    xfs_rfsblock_t *toti, xfs_extnum_t *nex,
				    blkmap_t **blkmapp, int isroot,
				    typnm_t btype);
static void		scan_sbtree(xfs_agf_t *agf, xfs_agblock_t root,
				    int nlevels, int isroot,
				    scan_sbtree_f_t func, typnm_t btype);
static void		scanfunc_bmap(struct xfs_btree_block *block,
				      int level, dbm_t type, xfs_fsblock_t bno,
				      inodata_t *id, xfs_rfsblock_t *totd,
				      xfs_rfsblock_t *toti, xfs_extnum_t *nex,
				      blkmap_t **blkmapp, int isroot,
				      typnm_t btype);
static void		scanfunc_bno(struct xfs_btree_block *block, int level,
				     xfs_agf_t *agf, xfs_agblock_t bno,
				     int isroot);
static void		scanfunc_cnt(struct xfs_btree_block *block, int level,
				     xfs_agf_t *agf, xfs_agblock_t bno,
				     int isroot);
static void		scanfunc_ino(struct xfs_btree_block *block, int level,
				     xfs_agf_t *agf, xfs_agblock_t bno,
				     int isroot);
static void		scanfunc_fino(struct xfs_btree_block *block, int level,
				     struct xfs_agf *agf, xfs_agblock_t bno,
				     int isroot);
static void		scanfunc_rmap(struct xfs_btree_block *block, int level,
				     struct xfs_agf *agf, xfs_agblock_t bno,
				     int isroot);
static void		scanfunc_refcnt(struct xfs_btree_block *block, int level,
				     struct xfs_agf *agf, xfs_agblock_t bno,
				     int isroot);
static void		set_dbmap(xfs_agnumber_t agno, xfs_agblock_t agbno,
				  xfs_extlen_t len, dbm_t type,
				  xfs_agnumber_t c_agno, xfs_agblock_t c_agbno);
static void		set_inomap(xfs_agnumber_t agno, xfs_agblock_t agbno,
				   xfs_extlen_t len, inodata_t *id);
static void		set_rdbmap(xfs_rfsblock_t bno, xfs_extlen_t len,
				   dbm_t type);
static void		set_rinomap(xfs_rfsblock_t bno, xfs_extlen_t len,
				    inodata_t *id);
static void		setlink_inode(inodata_t *id, nlink_t nlink, int isdir,
				       int security);

static const cmdinfo_t	blockfree_cmd =
	{ "blockfree", NULL, blockfree_f, 0, 0, 0,
	  NULL, N_("free block usage information"), NULL };
static const cmdinfo_t	blockget_cmd =
	{ "blockget", "check", blockget_f, 0, -1, 0,
	  N_("[-s|-v] [-n] [-t] [-b bno]... [-i ino] ..."),
	  N_("get block usage and check consistency"), NULL };
static const cmdinfo_t	blocktrash_cmd =
	{ "blocktrash", NULL, blocktrash_f, 0, -1, 0,
	  N_("[-n count] [-x minlen] [-y maxlen] [-s seed] [-0123] [-t type] ..."),
	  N_("trash randomly selected block(s)"), NULL };
static const cmdinfo_t	blockuse_cmd =
	{ "blockuse", NULL, blockuse_f, 0, 3, 0,
	  N_("[-n] [-c blockcount]"),
	  N_("print usage for current block(s)"), NULL };
static const cmdinfo_t	ncheck_cmd =
	{ "ncheck", NULL, ncheck_f, 0, -1, 0,
	  N_("[-s] [-i ino] ..."),
	  N_("print inode-name pairs"), NULL };


static void
add_blist(
	xfs_fsblock_t	bno)
{
	blist_size++;
	blist = xrealloc(blist, blist_size * sizeof(bno));
	blist[blist_size - 1] = bno;
}

static void
add_ilist(
	xfs_ino_t	ino)
{
	inodata_t	*id;

	id = find_inode(ino, 1);
	if (id == NULL) {
		dbprintf(_("-i %lld bad inode number\n"), ino);
		return;
	}
	id->ilist = 1;
}

static void
addlink_inode(
	inodata_t	*id)
{
	id->link_add++;
	if (verbose || id->ilist)
		dbprintf(_("inode %lld add link, now %u\n"), id->ino,
			id->link_add);
}

static void
addname_inode(
	inodata_t	*id,
	char		*name,
	int		namelen)
{
	if (!nflag || id->name)
		return;
	id->name = xmalloc(namelen + 1);
	memcpy(id->name, name, namelen);
	id->name[namelen] = '\0';
}

static void
addparent_inode(
	inodata_t	*id,
	xfs_ino_t	parent)
{
	inodata_t	*pid;

	pid = find_inode(parent, 1);
	id->parent = pid;
	if (verbose || id->ilist || (pid && pid->ilist))
		dbprintf(_("inode %lld parent %lld\n"), id->ino, parent);
}

static void
blkent_append(
	blkent_t	**entp,
	xfs_fsblock_t	b,
	xfs_extlen_t	c)
{
	blkent_t	*ent;
	int		i;

	ent = *entp;
	*entp = ent = xrealloc(ent, BLKENT_SIZE(c + ent->nblks));
	for (i = 0; i < c; i++)
		ent->blks[ent->nblks + i] = b + i;
	ent->nblks += c;
}

static blkent_t *
blkent_new(
	xfs_fileoff_t	o,
	xfs_fsblock_t	b,
	xfs_extlen_t	c)
{
	blkent_t	*ent;
	int		i;

	ent = xmalloc(BLKENT_SIZE(c));
	ent->nblks = c;
	ent->startoff = o;
	for (i = 0; i < c; i++)
		ent->blks[i] = b + i;
	return ent;
}

static void
blkent_prepend(
	blkent_t	**entp,
	xfs_fsblock_t	b,
	xfs_extlen_t	c)
{
	int		i;
	blkent_t	*newent;
	blkent_t	*oldent;

	oldent = *entp;
	newent = xmalloc(BLKENT_SIZE(oldent->nblks + c));
	newent->nblks = oldent->nblks + c;
	newent->startoff = oldent->startoff - c;
	for (i = 0; i < c; i++)
		newent->blks[i] = b + c;
	for (; i < oldent->nblks + c; i++)
		newent->blks[i] = oldent->blks[i - c];
	xfree(oldent);
	*entp = newent;
}

static blkmap_t *
blkmap_alloc(
	xfs_extnum_t	nex)
{
	blkmap_t	*blkmap;

	if (nex < 1)
		nex = 1;
	blkmap = xmalloc(BLKMAP_SIZE(nex));
	blkmap->naents = nex;
	blkmap->nents = 0;
	return blkmap;
}

static void
blkmap_free(
	blkmap_t	*blkmap)
{
	blkent_t	**entp;
	xfs_extnum_t	i;

	for (i = 0, entp = blkmap->ents; i < blkmap->nents; i++, entp++)
		xfree(*entp);
	xfree(blkmap);
}

static xfs_fsblock_t
blkmap_get(
	blkmap_t	*blkmap,
	xfs_fileoff_t	o)
{
	blkent_t	*ent;
	blkent_t	**entp;
	int		i;

	for (i = 0, entp = blkmap->ents; i < blkmap->nents; i++, entp++) {
		ent = *entp;
		if (o >= ent->startoff && o < ent->startoff + ent->nblks)
			return ent->blks[o - ent->startoff];
	}
	return NULLFSBLOCK;
}

static int
blkmap_getn(
	blkmap_t	*blkmap,
	xfs_fileoff_t	o,
	int		nb,
	bmap_ext_t	**bmpp)
{
	bmap_ext_t	*bmp;
	blkent_t	*ent;
	xfs_fileoff_t	ento;
	blkent_t	**entp;
	int		i;
	int		nex;

	for (i = nex = 0, bmp = NULL, entp = blkmap->ents;
	     i < blkmap->nents;
	     i++, entp++) {
		ent = *entp;
		if (ent->startoff >= o + nb)
			break;
		if (ent->startoff + ent->nblks <= o)
			continue;
		for (ento = ent->startoff;
		     ento < ent->startoff + ent->nblks && ento < o + nb;
		     ento++) {
			if (ento < o)
				continue;
			if (bmp &&
			    bmp[nex - 1].startoff + bmp[nex - 1].blockcount ==
				    ento &&
			    bmp[nex - 1].startblock + bmp[nex - 1].blockcount ==
				    ent->blks[ento - ent->startoff])
				bmp[nex - 1].blockcount++;
			else {
				bmp = realloc(bmp, ++nex * sizeof(*bmp));
				bmp[nex - 1].startoff = ento;
				bmp[nex - 1].startblock =
					ent->blks[ento - ent->startoff];
				bmp[nex - 1].blockcount = 1;
				bmp[nex - 1].flag = 0;
			}
		}
	}
	*bmpp = bmp;
	return nex;
}

static void
blkmap_grow(
	blkmap_t	**blkmapp,
	blkent_t	**entp,
	blkent_t	*newent)
{
	blkmap_t	*blkmap;
	int		i;
	int		idx;

	blkmap = *blkmapp;
	idx = (int)(entp - blkmap->ents);
	if (blkmap->naents == blkmap->nents) {
		blkmap = xrealloc(blkmap, BLKMAP_SIZE(blkmap->nents + 1));
		*blkmapp = blkmap;
		blkmap->naents++;
	}
	for (i = blkmap->nents; i > idx; i--)
		blkmap->ents[i] = blkmap->ents[i - 1];
	blkmap->ents[idx] = newent;
	blkmap->nents++;
}

static xfs_fileoff_t
blkmap_last_off(
	blkmap_t	*blkmap)
{
	blkent_t	*ent;

	if (!blkmap->nents)
		return NULLFILEOFF;
	ent = blkmap->ents[blkmap->nents - 1];
	return ent->startoff + ent->nblks;
}

static xfs_fileoff_t
blkmap_next_off(
	blkmap_t	*blkmap,
	xfs_fileoff_t	o,
	int		*t)
{
	blkent_t	*ent;
	blkent_t	**entp;

	if (!blkmap->nents)
		return NULLFILEOFF;
	if (o == NULLFILEOFF) {
		*t = 0;
		ent = blkmap->ents[0];
		return ent->startoff;
	}
	entp = &blkmap->ents[*t];
	ent = *entp;
	if (o < ent->startoff + ent->nblks - 1)
		return o + 1;
	entp++;
	if (entp >= &blkmap->ents[blkmap->nents])
		return NULLFILEOFF;
	(*t)++;
	ent = *entp;
	return ent->startoff;
}

static void
blkmap_set_blk(
	blkmap_t	**blkmapp,
	xfs_fileoff_t	o,
	xfs_fsblock_t	b)
{
	blkmap_t	*blkmap;
	blkent_t	*ent;
	blkent_t	**entp;
	blkent_t	*nextent;

	blkmap = *blkmapp;
	for (entp = blkmap->ents; entp < &blkmap->ents[blkmap->nents]; entp++) {
		ent = *entp;
		if (o < ent->startoff - 1) {
			ent = blkent_new(o, b, 1);
			blkmap_grow(blkmapp, entp, ent);
			return;
		}
		if (o == ent->startoff - 1) {
			blkent_prepend(entp, b, 1);
			return;
		}
		if (o >= ent->startoff && o < ent->startoff + ent->nblks) {
			ent->blks[o - ent->startoff] = b;
			return;
		}
		if (o > ent->startoff + ent->nblks)
			continue;
		blkent_append(entp, b, 1);
		if (entp == &blkmap->ents[blkmap->nents - 1])
			return;
		ent = *entp;
		nextent = entp[1];
		if (ent->startoff + ent->nblks < nextent->startoff)
			return;
		blkent_append(entp, nextent->blks[0], nextent->nblks);
		blkmap_shrink(blkmap, &entp[1]);
		return;
	}
	ent = blkent_new(o, b, 1);
	blkmap_grow(blkmapp, entp, ent);
}

static void
blkmap_set_ext(
	blkmap_t	**blkmapp,
	xfs_fileoff_t	o,
	xfs_fsblock_t	b,
	xfs_extlen_t	c)
{
	blkmap_t	*blkmap;
	blkent_t	*ent;
	blkent_t	**entp;
	xfs_extnum_t	i;

	blkmap = *blkmapp;
	if (!blkmap->nents) {
		blkmap->ents[0] = blkent_new(o, b, c);
		blkmap->nents = 1;
		return;
	}
	entp = &blkmap->ents[blkmap->nents - 1];
	ent = *entp;
	if (ent->startoff + ent->nblks == o) {
		blkent_append(entp, b, c);
		return;
	}
	if (ent->startoff + ent->nblks < o) {
		ent = blkent_new(o, b, c);
		blkmap_grow(blkmapp, &blkmap->ents[blkmap->nents], ent);
		return;
	}
	for (i = 0; i < c; i++)
		blkmap_set_blk(blkmapp, o + i, b + i);
}

static void
blkmap_shrink(
	blkmap_t	*blkmap,
	blkent_t	**entp)
{
	int		i;
	int		idx;

	xfree(*entp);
	idx = (int)(entp - blkmap->ents);
	for (i = idx + 1; i < blkmap->nents; i++)
		blkmap->ents[i] = blkmap->ents[i - 1];
	blkmap->nents--;
}

/* ARGSUSED */
static int
blockfree_f(
	int		argc,
	char		**argv)
{
	xfs_agnumber_t	c;
	int		rt;

	if (!dbmap) {
		dbprintf(_("block usage information not allocated\n"));
		return 0;
	}
	rt = mp->m_sb.sb_rextents != 0;
	for (c = 0; c < mp->m_sb.sb_agcount; c++) {
		xfree(dbmap[c]);
		xfree(inomap[c]);
		free_inodata(c);
	}
	if (rt) {
		xfree(dbmap[c]);
		xfree(inomap[c]);
		xfree(sumcompute);
		xfree(sumfile);
		sumcompute = sumfile = NULL;
	}
	xfree(dbmap);
	xfree(inomap);
	xfree(inodata);
	dbmap = NULL;
	inomap = NULL;
	inodata = NULL;
	return 0;
}

/*
 * Check consistency of xfs filesystem contents.
 */
static int
blockget_f(
	int		argc,
	char		**argv)
{
	xfs_agnumber_t	agno;
	int		oldprefix;
	int		sbyell;

	if (dbmap) {
		dbprintf(_("already have block usage information\n"));
		return 0;
	}

	if (!init(argc, argv)) {
		if (serious_error)
			exitcode = 3;
		else
			exitcode = 1;
		return 0;
	}
	oldprefix = dbprefix;
	dbprefix |= pflag;
	for (agno = 0, sbyell = 0; agno < mp->m_sb.sb_agcount; agno++) {
		scan_ag(agno);
		if (sbver_err > 4 && !sbyell && sbver_err >= agno) {
			sbyell = 1;
			dbprintf(_("WARNING: this may be a newer XFS "
				 "filesystem.\n"));
		}
	}
	if (blist_size) {
		xfree(blist);
		blist = NULL;
		blist_size = 0;
	}
	if (serious_error) {
		exitcode = 2;
		dbprefix = oldprefix;
		return 0;
	}
	check_rootdir();
	/*
	 * Check that there are no blocks either
	 * a) unaccounted for or
	 * b) bno-free but not cnt-free
	 */
	if (!tflag) {	/* are we in test mode, faking out freespace? */
		for (agno = 0; agno < mp->m_sb.sb_agcount; agno++)
			checknot_dbmap(agno, 0, mp->m_sb.sb_agblocks,
				(1 << DBM_UNKNOWN) | (1 << DBM_FREE1));
	}
	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++)
		check_linkcounts(agno);
	if (mp->m_sb.sb_rblocks) {
		checknot_rdbmap(0,
			(xfs_extlen_t)(mp->m_sb.sb_rextents *
				       mp->m_sb.sb_rextsize),
			1 << DBM_UNKNOWN);
		check_summary();
	}
	if (mp->m_sb.sb_icount != icount) {
		if (!sflag)
			dbprintf(_("sb_icount %lld, counted %lld\n"),
				mp->m_sb.sb_icount, icount);
		error++;
	}
	if (mp->m_sb.sb_ifree != ifree) {
		if (!sflag)
			dbprintf(_("sb_ifree %lld, counted %lld\n"),
				mp->m_sb.sb_ifree, ifree);
		error++;
	}
	if (mp->m_sb.sb_fdblocks != fdblocks) {
		if (!sflag)
			dbprintf(_("sb_fdblocks %lld, counted %lld\n"),
				mp->m_sb.sb_fdblocks, fdblocks);
		error++;
	}
	if (lazycount && mp->m_sb.sb_fdblocks != agf_aggr_freeblks) {
		if (!sflag)
			dbprintf(_("sb_fdblocks %lld, aggregate AGF count %lld\n"),
				mp->m_sb.sb_fdblocks, agf_aggr_freeblks);
		error++;
	}
	if (mp->m_sb.sb_frextents != frextents) {
		if (!sflag)
			dbprintf(_("sb_frextents %lld, counted %lld\n"),
				mp->m_sb.sb_frextents, frextents);
		error++;
	}
	if (mp->m_sb.sb_bad_features2 != 0 &&
			mp->m_sb.sb_bad_features2 != mp->m_sb.sb_features2) {
		if (!sflag)
			dbprintf(_("sb_features2 (0x%x) not same as "
				"sb_bad_features2 (0x%x)\n"),
				mp->m_sb.sb_features2,
				mp->m_sb.sb_bad_features2);
		error++;
	}
	if ((sbversion & XFS_SB_VERSION_ATTRBIT) &&
					!xfs_has_attr(mp)) {
		if (!sflag)
			dbprintf(_("sb versionnum missing attr bit %x\n"),
				XFS_SB_VERSION_ATTRBIT);
		error++;
	}
	if ((sbversion & XFS_SB_VERSION_QUOTABIT) &&
					!xfs_has_quota(mp)) {
		if (!sflag)
			dbprintf(_("sb versionnum missing quota bit %x\n"),
				XFS_SB_VERSION_QUOTABIT);
		error++;
	}
	if (!(sbversion & XFS_SB_VERSION_ALIGNBIT) &&
					xfs_has_align(mp)) {
		if (!sflag)
			dbprintf(_("sb versionnum extra align bit %x\n"),
				XFS_SB_VERSION_ALIGNBIT);
		error++;
	}
	if (qudo)
		quota_check("user", qudata);
	if (qpdo)
		quota_check("project", qpdata);
	if (qgdo)
		quota_check("group", qgdata);
	if (sbver_err > mp->m_sb.sb_agcount / 2)
		dbprintf(_("WARNING: this may be a newer XFS filesystem.\n"));
	if (error)
		exitcode = 3;
	dbprefix = oldprefix;
	return 0;
}

typedef struct ltab {
	int	min;
	int	max;
} ltab_t;

static void
blocktrash_b(
	int		bit_offset,
	dbm_t		type,
	ltab_t		*ltabp,
	int		mode)
{
	int		bit;
	int		bitno;
	char		*buf;
	int		byte;
	int		len;
	int		mask;
	int		newbit;
	const struct xfs_buf_ops *stashed_ops;
	static char	*modestr[] = {
		N_("zeroed"), N_("set"), N_("flipped"), N_("randomized")
	};
	xfs_agnumber_t	agno;
	xfs_agblock_t	agbno;

	agno = XFS_FSB_TO_AGNO(mp, XFS_DADDR_TO_FSB(mp, iocur_top->bb));
	agbno = XFS_FSB_TO_AGBNO(mp, XFS_DADDR_TO_FSB(mp, iocur_top->bb));
	if (iocur_top->len == 0) {
		dbprintf(_("zero-length block %u/%u buffer to trash??\n"),
				agno, agbno);
		return;
	}
	len = (int)((random() % (ltabp->max - ltabp->min + 1)) + ltabp->min);
	/*
	 * bit_offset >= 0: start fuzzing at this exact bit_offset.
	 * bit_offset < 0: pick an offset at least as high at -(bit_offset + 1).
	 */
	if (bit_offset < 0) {
		bit_offset = -(bit_offset + 1);
		bit_offset += (int)(random() % (int)((iocur_top->len - bit_offset) * NBBY));
	}
	if (bit_offset + len >= iocur_top->len * NBBY)
		len = (iocur_top->len * NBBY) - bit_offset;
	newbit = 0;
	stashed_ops = iocur_top->bp->b_ops;
	iocur_top->bp->b_ops = NULL;
	if ((buf = iocur_top->data) == NULL) {
		dbprintf(_("can't read block %u/%u for trashing\n"), agno, agbno);
		return;
	}
	for (bitno = 0; bitno < len; bitno++) {
		bit = (bit_offset + bitno) % (mp->m_sb.sb_blocksize * NBBY);
		byte = bit / NBBY;
		bit %= NBBY;
		mask = 1 << bit;
		switch (mode) {
		case 0:
			newbit = 0;
			break;
		case 1:
			newbit = 1;
			break;
		case 2:
			newbit = (buf[byte] & mask) == 0;
			break;
		case 3:
			newbit = (int)random() & 1;
			break;
		}
		if (newbit)
			buf[byte] |= mask;
		else
			buf[byte] &= ~mask;
	}
	write_cur();
	iocur_top->bp->b_ops = stashed_ops;
	printf(_("blocktrash: %u/%u %s block %d bit%s starting %d:%d %s\n"),
		agno, agbno, typename[type], len, len == 1 ? "" : "s",
		bit_offset / NBBY, bit_offset % NBBY, modestr[mode]);
}

static int
blocktrash_f(
	int		argc,
	char		**argv)
{
	xfs_agblock_t	agbno;
	xfs_agnumber_t	agno;
	xfs_rfsblock_t	bi;
	xfs_rfsblock_t	blocks;
	int		c;
	int		count;
	int		done;
	int		goodmask;
	int		i;
	ltab_t		*lentab;
	int		lentablen;
	int		max;
	int		min;
	int		mode;
	struct timeval	now;
	char		*p;
	xfs_rfsblock_t	randb;
	uint		seed;
	int		sopt;
	int		tmask;
	bool		this_block = false;
	int		bit_offset = -1;

	optind = 0;
	count = 1;
	min = 1;
	max = 128 * NBBY;
	mode = 2;
	gettimeofday(&now, NULL);
	seed = (unsigned int)(now.tv_sec ^ now.tv_usec);
	sopt = 0;
	tmask = 0;
	goodmask = (1 << DBM_AGF) |
		   (1 << DBM_AGFL) |
		   (1 << DBM_AGI) |
		   (1 << DBM_ATTR) |
		   (1 << DBM_BTBMAPA) |
		   (1 << DBM_BTBMAPD) |
		   (1 << DBM_BTBNO) |
		   (1 << DBM_BTCNT) |
		   (1 << DBM_BTINO) |
		   (1 << DBM_DIR) |
		   (1 << DBM_INODE) |
		   (1 << DBM_LOG) |
		   (1 << DBM_QUOTA) |
		   (1 << DBM_RTBITMAP) |
		   (1 << DBM_RTSUM) |
		   (1 << DBM_SYMLINK) |
		   (1 << DBM_BTFINO) |
		   (1 << DBM_BTRMAP) |
		   (1 << DBM_BTREFC) |
		   (1 << DBM_SB);
	while ((c = getopt(argc, argv, "0123n:o:s:t:x:y:z")) != EOF) {
		switch (c) {
		case '0':
			mode = 0;
			break;
		case '1':
			mode = 1;
			break;
		case '2':
			mode = 2;
			break;
		case '3':
			mode = 3;
			break;
		case 'n':
			count = (int)strtol(optarg, &p, 0);
			if (*p != '\0' || count <= 0) {
				dbprintf(_("bad blocktrash count %s\n"), optarg);
				return 0;
			}
			break;
		case 'o': {
			int relative = 0;
			if (optarg[0] == '+') {
				optarg++;
				relative = 1;
			}
			bit_offset = (int)strtol(optarg, &p, 0);
			if (*p != '\0' || bit_offset < 0) {
				dbprintf(_("bad blocktrash offset %s\n"), optarg);
				return 0;
			}
			if (relative)
				bit_offset = -bit_offset - 1;
			break;
		}
		case 's':
			seed = (uint)strtoul(optarg, &p, 0);
			sopt = 1;
			break;
		case 't':
			for (i = 0; typename[i]; i++) {
				if (strcmp(typename[i], optarg) == 0)
					break;
			}
			if (!typename[i] || (((1 << i) & goodmask) == 0)) {
				dbprintf(_("bad blocktrash type %s\n"), optarg);
				return 0;
			}
			tmask |= 1 << i;
			break;
		case 'x':
			min = (int)strtol(optarg, &p, 0);
			if (*p != '\0' || min <= 0 ||
			    min > mp->m_sb.sb_blocksize * NBBY) {
				dbprintf(_("bad blocktrash min %s\n"), optarg);
				return 0;
			}
			break;
		case 'y':
			max = (int)strtol(optarg, &p, 0);
			if (*p != '\0' || max <= 0 ||
			    max > mp->m_sb.sb_blocksize * NBBY) {
				dbprintf(_("bad blocktrash max %s\n"), optarg);
				return 0;
			}
			break;
		case 'z':
			this_block = true;
			break;
		default:
			dbprintf(_("bad option for blocktrash command\n"));
			return 0;
		}
	}
	if (!this_block && !dbmap) {
		dbprintf(_("must run blockget first\n"));
		return 0;
	}
	if (this_block && iocur_sp == 0) {
		dbprintf(_("nothing on stack\n"));
		return 0;
	}
	if (min > max) {
		dbprintf(_("bad min/max for blocktrash command\n"));
		return 0;
	}
	if (tmask == 0)
		tmask = goodmask & ~((1 << DBM_LOG) | (1 << DBM_SB));
	lentab = xmalloc(sizeof(ltab_t));
	lentab->min = lentab->max = min;
	lentablen = 1;
	for (i = min + 1; i <= max; i++) {
		if ((i & (i - 1)) == 0) {
			lentab = xrealloc(lentab,
				sizeof(ltab_t) * (lentablen + 1));
			lentab[lentablen].min = lentab[lentablen].max = i;
			lentablen++;
		} else
			lentab[lentablen - 1].max = i;
	}
	if (!sopt)
		dbprintf(_("blocktrash: seed %u\n"), seed);
	srandom(seed);
	if (this_block) {
		blocktrash_b(bit_offset, DBM_UNKNOWN,
				&lentab[random() % lentablen], mode);
		goto out;
	}
	for (blocks = 0, agno = 0; agno < mp->m_sb.sb_agcount; agno++) {
		for (agbno = 0, p = dbmap[agno];
		     agbno < mp->m_sb.sb_agblocks;
		     agbno++, p++) {
			if ((1 << *p) & tmask)
				blocks++;
		}
	}
	if (blocks == 0) {
		dbprintf(_("blocktrash: no matching blocks\n"));
		goto out;
	}
	for (i = 0; i < count; i++) {
		randb = (xfs_rfsblock_t)((((int64_t)random() << 32) |
					 random()) % blocks);
		for (bi = 0, agno = 0, done = 0;
		     !done && agno < mp->m_sb.sb_agcount;
		     agno++) {
			for (agbno = 0, p = dbmap[agno];
			     agbno < mp->m_sb.sb_agblocks;
			     agbno++, p++) {
				if (!((1 << *p) & tmask))
					continue;
				if (bi++ < randb)
					continue;
				push_cur();
				set_cur(NULL,
					XFS_AGB_TO_DADDR(mp, agno, agbno),
					blkbb, DB_RING_IGN, NULL);
				blocktrash_b(bit_offset, (dbm_t)*p,
					&lentab[random() % lentablen], mode);
				pop_cur();
				done = 1;
				break;
			}
		}
	}
out:
	xfree(lentab);
	return 0;
}

static int
blockuse_f(
	int		argc,
	char		**argv)
{
	xfs_agblock_t	agbno;
	xfs_agnumber_t	agno;
	int		c;
	int		count;
	xfs_agblock_t	end;
	xfs_fsblock_t	fsb;
	inodata_t	*i;
	char		*p;
	int		shownames;

	if (!dbmap) {
		dbprintf(_("must run blockget first\n"));
		return 0;
	}
	optind = 0;
	shownames = 0;
	fsb = XFS_DADDR_TO_FSB(mp, iocur_top->off >> BBSHIFT);
	agno = XFS_FSB_TO_AGNO(mp, fsb);
	end = agbno = XFS_FSB_TO_AGBNO(mp, fsb);
	while ((c = getopt(argc, argv, "c:n")) != EOF) {
		switch (c) {
		case 'c':
			count = (int)strtol(optarg, &p, 0);
			end = agbno + count - 1;
			if (*p != '\0' || count <= 0 ||
			    end >= mp->m_sb.sb_agblocks) {
				dbprintf(_("bad blockuse count %s\n"), optarg);
				return 0;
			}
			break;
		case 'n':
			if (!nflag) {
				dbprintf(_("must run blockget -n first\n"));
				return 0;
			}
			shownames = 1;
			break;
		default:
			dbprintf(_("bad option for blockuse command\n"));
			return 0;
		}
	}
	while (agbno <= end) {
		p = &dbmap[agno][agbno];
		i = inomap[agno][agbno];
		dbprintf(_("block %llu (%u/%u) type %s"),
			(xfs_fsblock_t)XFS_AGB_TO_FSB(mp, agno, agbno),
			agno, agbno, typename[(dbm_t)*p]);
		if (i) {
			dbprintf(_(" inode %lld"), i->ino);
			if (shownames && (p = inode_name(i->ino, NULL))) {
				dbprintf(" %s", p);
				xfree(p);
			}
		}
		dbprintf("\n");
		agbno++;
	}
	return 0;
}

static int
check_blist(
	xfs_fsblock_t	bno)
{
	int		i;

	for (i = 0; i < blist_size; i++) {
		if (blist[i] == bno)
			return 1;
	}
	return 0;
}

static inline bool
dbmap_boundscheck(
	xfs_agnumber_t	agno,
	xfs_agblock_t	agbno)
{
	if (agno >= mp->m_sb.sb_agcount)
		return false;
	if (agbno >= mp->m_sb.sb_agblocks)
		return false;
	return true;
}

static void
check_dbmap(
	xfs_agnumber_t	agno,
	xfs_agblock_t	agbno,
	xfs_extlen_t	len,
	dbm_t		type,
	int		ignore_reflink)
{
	xfs_extlen_t	i;
	char		*p;
	dbm_t		d;

	for (i = 0, p = &dbmap[agno][agbno]; i < len; i++, p++) {
		if (!dbmap_boundscheck(agno, agbno + i)) {
			dbprintf(_("block %u/%u beyond end of expected area\n"),
				agno, agbno + i);
			error++;
			break;
		}
		d = (dbm_t)*p;
		if (ignore_reflink && (d == DBM_UNKNOWN || d == DBM_DATA ||
				       d == DBM_RLDATA))
			continue;
		if ((dbm_t)*p != type) {
			if (!sflag || CHECK_BLISTA(agno, agbno + i)) {
				dbprintf(_("block %u/%u expected type %s got "
					 "%s\n"),
					agno, agbno + i, typename[type],
					typename[(dbm_t)*p]);
			}
			error++;
		}
	}
}

void
check_init(void)
{
	add_command(&blockfree_cmd);
	add_command(&blockget_cmd);
	if (expert_mode)
		add_command(&blocktrash_cmd);
	add_command(&blockuse_cmd);
	add_command(&ncheck_cmd);
}

static int
check_inomap(
	xfs_agnumber_t	agno,
	xfs_agblock_t	agbno,
	xfs_extlen_t	len,
	xfs_ino_t	c_ino)
{
	xfs_extlen_t	i;
	inodata_t	**idp;
	int		rval;

	if (!check_range(agno, agbno, len))  {
		dbprintf(_("blocks %u/%u..%u claimed by inode %lld\n"),
			agno, agbno, agbno + len - 1, c_ino);
		return 0;
	}
	for (i = 0, rval = 1, idp = &inomap[agno][agbno]; i < len; i++, idp++) {
		if (*idp && !(*idp)->isreflink) {
			if (!sflag || (*idp)->ilist ||
			    CHECK_BLISTA(agno, agbno + i))
				dbprintf(_("block %u/%u claimed by inode %lld, "
					 "previous inum %lld\n"),
					agno, agbno + i, c_ino, (*idp)->ino);
			error++;
			rval = 0;
		}
	}
	return rval;
}

static void
check_linkcounts(
	xfs_agnumber_t	agno)
{
	inodata_t	*ep;
	inodata_t	**ht;
	int		idx;
	char		*path;

	ht = inodata[agno];
	for (idx = 0; idx < inodata_hash_size; ht++, idx++) {
		ep = *ht;
		while (ep) {
			if (ep->link_set != ep->link_add || ep->link_set == 0) {
				path = inode_name(ep->ino, NULL);
				if (!path && ep->link_add)
					path = xstrdup("?");
				if (!sflag || ep->ilist) {
					if (ep->link_add)
						dbprintf(_("link count mismatch "
							 "for inode %lld (name "
							 "%s), nlink %d, "
							 "counted %d\n"),
							ep->ino, path,
							ep->link_set,
							ep->link_add);
					else if (ep->link_set)
						dbprintf(_("disconnected inode "
							 "%lld, nlink %d\n"),
							ep->ino, ep->link_set);
					else
						dbprintf(_("allocated inode %lld "
							 "has 0 link count\n"),
							ep->ino);
				}
				if (path)
					xfree(path);
				error++;
			} else if (verbose || ep->ilist) {
				path = inode_name(ep->ino, NULL);
				if (path) {
					dbprintf(_("inode %lld name %s\n"),
						ep->ino, path);
					xfree(path);
				}
			}
			ep = ep->next;
		}
	}

}

static int
check_range(
	xfs_agnumber_t  agno,
	xfs_agblock_t   agbno,
	xfs_extlen_t    len)
{
	xfs_extlen_t    i;
	xfs_agblock_t   low = 0;
	xfs_agblock_t   high = 0;
	int             valid_range = 0;
	int             cur, prev = 0;

	if (agno >= mp->m_sb.sb_agcount ||
	    agbno + len - 1 >= mp->m_sb.sb_agblocks) {
		for (i = 0; i < len; i++) {
			cur = !sflag || CHECK_BLISTA(agno, agbno + i) ? 1 : 0;
			if (cur == 1 && prev == 0) {
				low = high = agbno + i;
				valid_range = 1;
			} else if (cur == 0 && prev == 0) {
				/* Do nothing */
			} else if (cur == 0 && prev == 1) {
				if (low == high) {
					dbprintf(_("block %u/%u out of range\n"),
						agno, low);
				} else {
					dbprintf(_("blocks %u/%u..%u "
						"out of range\n"),
						agno, low, high);
				}
				valid_range = 0;
			} else if (cur == 1 && prev == 1) {
				high = agbno + i;
			}
			prev = cur;
		}
		if (valid_range) {
        		if (low == high) {
               			dbprintf(_("block %u/%u out of range\n"),
                       		agno, low);
			} else {
               			dbprintf(_("blocks %u/%u..%u "
               				"out of range\n"),
               				agno, low, high);
      		 	}
		}
		error++;
		return 0;
	}
	return 1;
}

static inline bool
rdbmap_boundscheck(
	xfs_rfsblock_t	bno)
{
	return bno < mp->m_sb.sb_rblocks;
}

static void
check_rdbmap(
	xfs_rfsblock_t	bno,
	xfs_extlen_t	len,
	dbm_t		type)
{
	xfs_extlen_t	i;
	char		*p;

	for (i = 0, p = &dbmap[mp->m_sb.sb_agcount][bno]; i < len; i++, p++) {
		if (!rdbmap_boundscheck(bno + i)) {
			dbprintf(_("rtblock %llu beyond end of expected area\n"),
				bno + i);
			error++;
			break;
		}
		if ((dbm_t)*p != type) {
			if (!sflag || CHECK_BLIST(bno + i))
				dbprintf(_("rtblock %llu expected type %s got "
					 "%s\n"),
					bno + i, typename[type],
					typename[(dbm_t)*p]);
			error++;
		}
	}
}

static int
check_rinomap(
	xfs_rfsblock_t	bno,
	xfs_extlen_t	len,
	xfs_ino_t	c_ino)
{
	xfs_extlen_t	i;
	inodata_t	**idp;
	int		rval;

	if (!check_rrange(bno, len)) {
		dbprintf(_("rtblocks %llu..%llu claimed by inode %lld\n"),
			bno, bno + len - 1, c_ino);
		return 0;
	}
	for (i = 0, rval = 1, idp = &inomap[mp->m_sb.sb_agcount][bno];
	     i < len;
	     i++, idp++) {
		if (*idp) {
			if (!sflag || (*idp)->ilist || CHECK_BLIST(bno + i))
				dbprintf(_("rtblock %llu claimed by inode %lld, "
					 "previous inum %lld\n"),
					bno + i, c_ino, (*idp)->ino);
			error++;
			rval = 0;
		}
	}
	return rval;
}

static void
check_rootdir(void)
{
	inodata_t	*id;

	id = find_inode(mp->m_sb.sb_rootino, 0);
	if (id == NULL) {
		if (!sflag)
			dbprintf(_("root inode %lld is missing\n"),
				mp->m_sb.sb_rootino);
		error++;
	} else if (!id->isdir) {
		if (!sflag || id->ilist)
			dbprintf(_("root inode %lld is not a directory\n"),
				mp->m_sb.sb_rootino);
		error++;
	}
}

static inline void
report_rrange(
	xfs_rfsblock_t	low,
	xfs_rfsblock_t	high)
{
	if (low == high)
		dbprintf(_("rtblock %llu out of range\n"), low);
	else
		dbprintf(_("rtblocks %llu..%llu out of range\n"), low, high);
}

static int
check_rrange(
	xfs_rfsblock_t	bno,
	xfs_extlen_t	len)
{
	xfs_extlen_t	i;
	xfs_rfsblock_t	low = 0;
	xfs_rfsblock_t	high = 0;
	bool		valid_range = false;
	int		cur, prev = 0;

	if (bno + len - 1 >= mp->m_sb.sb_rblocks) {
		for (i = 0; i < len; i++) {
			cur = !sflag || CHECK_BLIST(bno + i) ? 1 : 0;
			if (cur == 1 && prev == 0) {
				low = high = bno + i;
				valid_range = true;
			} else if (cur == 0 && prev == 0) {
				/* Do nothing */
			} else if (cur == 0 && prev == 1) {
				report_rrange(low, high);
				valid_range = false;
			} else if (cur == 1 && prev == 1) {
				high = bno + i;
			}
			prev = cur;
		}
		if (valid_range)
			report_rrange(low, high);
		error++;
		return 0;
	}
	return 1;
}

/*
 * We don't check the accuracy of reference counts -- all we do is ensure
 * that a data block never crosses with non-data blocks.  repair can check
 * those kinds of things.
 *
 * So with that in mind, if we're setting a block to be data or rldata,
 * don't complain so long as the block is currently unknown, data, or rldata.
 * Don't let blocks downgrade from rldata -> data.
 */
static bool
is_reflink(
	dbm_t		type2)
{
	if (!xfs_has_reflink(mp))
		return false;
	if (type2 == DBM_DATA || type2 == DBM_RLDATA)
		return true;
	return false;
}

static void
check_set_dbmap(
	xfs_agnumber_t	agno,
	xfs_agblock_t	agbno,
	xfs_extlen_t	len,
	dbm_t		type1,
	dbm_t		type2,
	xfs_agnumber_t	c_agno,
	xfs_agblock_t	c_agbno)
{
	xfs_extlen_t	i;
	int		mayprint;
	char		*p;

	if (!check_range(agno, agbno, len))  {
		dbprintf(_("blocks %u/%u..%u claimed by block %u/%u\n"), agno,
			agbno, agbno + len - 1, c_agno, c_agbno);
		return;
	}
	check_dbmap(agno, agbno, len, type1, is_reflink(type2));
	mayprint = verbose | blist_size;
	for (i = 0, p = &dbmap[agno][agbno]; i < len; i++, p++) {
		if (!dbmap_boundscheck(agno, agbno + i)) {
			dbprintf(_("block %u/%u beyond end of expected area\n"),
				agno, agbno + i);
			error++;
			break;
		}
		if (*p == DBM_RLDATA && type2 == DBM_DATA)
			;	/* do nothing */
		else if (*p == DBM_DATA && type2 == DBM_DATA)
			*p = (char)DBM_RLDATA;
		else
			*p = (char)type2;
		if (mayprint && (verbose || CHECK_BLISTA(agno, agbno + i)))
			dbprintf(_("setting block %u/%u to %s\n"), agno, agbno + i,
				typename[type2]);
	}
}

static void
check_set_rdbmap(
	xfs_rfsblock_t	bno,
	xfs_extlen_t	len,
	dbm_t		type1,
	dbm_t		type2)
{
	xfs_extlen_t	i;
	int		mayprint;
	char		*p;

	if (!check_rrange(bno, len))
		return;
	check_rdbmap(bno, len, type1);
	mayprint = verbose | blist_size;
	for (i = 0, p = &dbmap[mp->m_sb.sb_agcount][bno]; i < len; i++, p++) {
		if (!rdbmap_boundscheck(bno + i)) {
			dbprintf(_("rtblock %llu beyond end of expected area\n"),
				bno + i);
			error++;
			break;
		}
		*p = (char)type2;
		if (mayprint && (verbose || CHECK_BLIST(bno + i)))
			dbprintf(_("setting rtblock %llu to %s\n"),
				bno + i, typename[type2]);
	}
}

static void
check_summary(void)
{
	xfs_rfsblock_t	bno;
	xfs_suminfo_t	*csp;
	xfs_suminfo_t	*fsp;
	int		log;

	csp = sumcompute;
	fsp = sumfile;
	for (log = 0; log < mp->m_rsumlevels; log++) {
		for (bno = 0;
		     bno < mp->m_sb.sb_rbmblocks;
		     bno++, csp++, fsp++) {
			if (*csp != *fsp) {
				if (!sflag)
					dbprintf(_("rt summary mismatch, size %d "
						 "block %llu, file: %d, "
						 "computed: %d\n"),
						log, bno, *fsp, *csp);
				error++;
			}
		}
	}
}

static void
checknot_dbmap(
	xfs_agnumber_t	agno,
	xfs_agblock_t	agbno,
	xfs_extlen_t	len,
	int		typemask)
{
	xfs_extlen_t	i;
	char		*p;

	if (!check_range(agno, agbno, len))
		return;
	for (i = 0, p = &dbmap[agno][agbno]; i < len; i++, p++) {
		if ((1 << *p) & typemask) {
			if (!sflag || CHECK_BLISTA(agno, agbno + i))
				dbprintf(_("block %u/%u type %s not expected\n"),
					agno, agbno + i, typename[(dbm_t)*p]);
			error++;
		}
	}
}

static void
checknot_rdbmap(
	xfs_rfsblock_t	bno,
	xfs_extlen_t	len,
	int		typemask)
{
	xfs_extlen_t	i;
	char		*p;

	if (!check_rrange(bno, len))
		return;
	for (i = 0, p = &dbmap[mp->m_sb.sb_agcount][bno]; i < len; i++, p++) {
		if ((1 << *p) & typemask) {
			if (!sflag || CHECK_BLIST(bno + i))
				dbprintf(_("rtblock %llu type %s not expected\n"),
					bno + i, typename[(dbm_t)*p]);
			error++;
		}
	}
}

static void
dir_hash_add(
	xfs_dahash_t		hash,
	xfs_dir2_dataptr_t	addr)
{
	int			i;
	dirhash_t		*p;

	i = DIR_HASH_FUNC(hash, addr);
	p = malloc(sizeof(*p));
	p->next = dirhash[i];
	dirhash[i] = p;
	p->hashval = hash;
	p->address = addr;
	p->seen = 0;
}

static void
dir_hash_check(
	inodata_t	*id,
	int		v)
{
	int		i;
	dirhash_t	*p;

	for (i = 0; i < DIR_HASH_SIZE; i++) {
		for (p = dirhash[i]; p; p = p->next) {
			if (p->seen)
				continue;
			if (!sflag || id->ilist || v)
				dbprintf(_("dir ino %lld missing leaf entry for "
					 "%x/%x\n"),
					id->ino, p->hashval, p->address);
			error++;
		}
	}
}

static void
dir_hash_done(void)
{
	int		i;
	dirhash_t	*n;
	dirhash_t	*p;

	for (i = 0; i < DIR_HASH_SIZE; i++) {
		for (p = dirhash[i]; p; p = n) {
			n = p->next;
			free(p);
		}
		dirhash[i] = NULL;
	}
}

static void
dir_hash_init(void)
{
	if (!dirhash)
		dirhash = calloc(DIR_HASH_SIZE, sizeof(*dirhash));
}

static int
dir_hash_see(
	xfs_dahash_t		hash,
	xfs_dir2_dataptr_t	addr)
{
	int			i;
	dirhash_t		*p;

	i = DIR_HASH_FUNC(hash, addr);
	for (p = dirhash[i]; p; p = p->next) {
		if (p->hashval == hash && p->address == addr) {
			if (p->seen)
				return 1;
			p->seen = 1;
			return 0;
		}
	}
	return -1;
}

static inodata_t *
find_inode(
	xfs_ino_t	ino,
	int		add)
{
	xfs_agino_t	agino;
	xfs_agnumber_t	agno;
	inodata_t	*ent;
	inodata_t	**htab;
	xfs_agino_t	ih;

	agno = XFS_INO_TO_AGNO(mp, ino);
	agino = XFS_INO_TO_AGINO(mp, ino);
	if (agno >= mp->m_sb.sb_agcount ||
	    XFS_AGINO_TO_INO(mp, agno, agino) != ino)
		return NULL;
	htab = inodata[agno];
	ih = agino % inodata_hash_size;
	ent = htab[ih];
	while (ent) {
		if (ent->ino == ino)
			return ent;
		ent = ent->next;
	}
	if (!add)
		return NULL;
	ent = xcalloc(1, sizeof(*ent));
	ent->ino = ino;
	ent->next = htab[ih];
	htab[ih] = ent;
	return ent;
}

static void
free_inodata(
	xfs_agnumber_t	agno)
{
	inodata_t	*hp;
	inodata_t	**ht;
	int		i;
	inodata_t	*next;

	ht = inodata[agno];
	for (i = 0; i < inodata_hash_size; i++) {
		hp = ht[i];
		while (hp) {
			next = hp->next;
			if (hp->name)
				xfree(hp->name);
			xfree(hp);
			hp = next;
		}
	}
	xfree(ht);
}

static int
init(
	int		argc,
	char		**argv)
{
	xfs_fsblock_t	bno;
	int		c;
	xfs_ino_t	ino;
	int		rt;

	serious_error = 0;
	if (mp->m_sb.sb_magicnum != XFS_SB_MAGIC) {
		dbprintf(_("bad superblock magic number %x, giving up\n"),
			mp->m_sb.sb_magicnum);
		serious_error = 1;
		return 0;
	}
	if (!sb_logcheck())
		return 0;
	rt = mp->m_sb.sb_rextents != 0;
	dbmap = xmalloc((mp->m_sb.sb_agcount + rt) * sizeof(*dbmap));
	inomap = xmalloc((mp->m_sb.sb_agcount + rt) * sizeof(*inomap));
	inodata = xmalloc(mp->m_sb.sb_agcount * sizeof(*inodata));
	inodata_hash_size =
		(int)max(min(mp->m_sb.sb_icount /
				(INODATA_AVG_HASH_LENGTH * mp->m_sb.sb_agcount),
			     MAX_INODATA_HASH_SIZE),
			 MIN_INODATA_HASH_SIZE);
	for (c = 0; c < mp->m_sb.sb_agcount; c++) {
		dbmap[c] = xcalloc(mp->m_sb.sb_agblocks, sizeof(**dbmap));
		inomap[c] = xcalloc(mp->m_sb.sb_agblocks, sizeof(**inomap));
		inodata[c] = xcalloc(inodata_hash_size, sizeof(**inodata));
	}
	if (rt) {
		dbmap[c] = xcalloc(mp->m_sb.sb_rblocks, sizeof(**dbmap));
		inomap[c] = xcalloc(mp->m_sb.sb_rblocks, sizeof(**inomap));
		sumfile = xcalloc(mp->m_rsumsize, 1);
		sumcompute = xcalloc(mp->m_rsumsize, 1);
	}
	nflag = sflag = tflag = verbose = optind = 0;
	while ((c = getopt(argc, argv, "b:i:npstv")) != EOF) {
		switch (c) {
		case 'b':
			bno = strtoll(optarg, NULL, 10);
			add_blist(bno);
			break;
		case 'i':
			ino = strtoll(optarg, NULL, 10);
			add_ilist(ino);
			break;
		case 'n':
			nflag = 1;
			break;
		case 'p':
			pflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
		case 't':
			tflag = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			dbprintf(_("bad option for blockget command\n"));
			return 0;
		}
	}
	error = sbver_err = serious_error = 0;
	fdblocks = frextents = icount = ifree = 0;
	sbversion = XFS_SB_VERSION_4;
	/*
	 * Note that inoalignmt == 0 is valid when fsb size is large enough for
	 * at least one full inode record per block. Check this case explicitly.
	 */
	if (mp->m_sb.sb_inoalignmt ||
	    (xfs_has_align(mp) &&
	     mp->m_sb.sb_inopblock >= XFS_INODES_PER_CHUNK))
		sbversion |= XFS_SB_VERSION_ALIGNBIT;
	if ((mp->m_sb.sb_uquotino && mp->m_sb.sb_uquotino != NULLFSINO) ||
	    (mp->m_sb.sb_gquotino && mp->m_sb.sb_gquotino != NULLFSINO) ||
	    (mp->m_sb.sb_pquotino && mp->m_sb.sb_pquotino != NULLFSINO))
		sbversion |= XFS_SB_VERSION_QUOTABIT;
	quota_init();
	return 1;
}

static char *
inode_name(
	xfs_ino_t	ino,
	inodata_t	**ipp)
{
	inodata_t	*id;
	char		*npath;
	char		*path;

	id = find_inode(ino, 0);
	if (ipp)
		*ipp = id;
	if (id == NULL)
		return NULL;
	if (id->name == NULL)
		return NULL;
	path = xstrdup(id->name);
	while (id->parent) {
		id = id->parent;
		if (id->name == NULL)
			break;
		npath = prepend_path(path, id->name);
		xfree(path);
		path = npath;
	}
	return path;
}

static int
ncheck_f(
	int		argc,
	char		**argv)
{
	xfs_agnumber_t	agno;
	int		c;
	inodata_t	*hp;
	inodata_t	**ht;
	int		i;
	inodata_t	*id;
	xfs_ino_t	*ilist;
	int		ilist_size;
	xfs_ino_t	*ilp;
	xfs_ino_t	ino;
	char		*p;
	int		security;

	if (!inodata || !nflag) {
		dbprintf(_("must run blockget -n first\n"));
		return 0;
	}
	security = optind = ilist_size = 0;
	ilist = NULL;
	while ((c = getopt(argc, argv, "i:s")) != EOF) {
		switch (c) {
		case 'i':
			ino = strtoll(optarg, NULL, 10);
			ilist = xrealloc(ilist, (ilist_size + 1) *
				sizeof(*ilist));
			ilist[ilist_size++] = ino;
			break;
		case 's':
			security = 1;
			break;
		default:
			dbprintf(_("bad option -%c for ncheck command\n"), c);
			xfree(ilist);
			return 0;
		}
	}
	if (ilist) {
		for (ilp = ilist; ilp < &ilist[ilist_size]; ilp++) {
			ino = *ilp;
			if ((p = inode_name(ino, &hp))) {
				dbprintf("%11llu %s", ino, p);
				if (hp->isdir)
					dbprintf("/.");
				dbprintf("\n");
				xfree(p);
			}
		}
		xfree(ilist);
		return 0;
	}
	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++) {
		ht = inodata[agno];
		for (i = 0; i < inodata_hash_size; i++) {
			hp = ht[i];
			for (hp = ht[i]; hp; hp = hp->next) {
				ino = XFS_AGINO_TO_INO(mp, agno, hp->ino);
				p = inode_name(ino, &id);
				if (!p || !id)
					continue;
				if (!security || id->security) {
					dbprintf("%11llu %s", ino, p);
					if (hp->isdir)
						dbprintf("/.");
					dbprintf("\n");
				}
				xfree(p);
			}
		}
	}
	return 0;
}

static char *
prepend_path(
	char	*oldpath,
	char	*parent)
{
	int	len;
	char	*path;

	len = (int)(strlen(oldpath) + strlen(parent) + 2);
	path = xmalloc(len);
	snprintf(path, len, "%s/%s", parent, oldpath);
	return path;
}

static xfs_ino_t
process_block_dir_v2(
	blkmap_t	*blkmap,
	int		*dot,
	int		*dotdot,
	inodata_t	*id)
{
	xfs_fsblock_t	b;
	bbmap_t		bbmap;
	bmap_ext_t	*bmp;
	int		nex;
	xfs_ino_t	parent;
	int		v;
	int		i;

	nex = blkmap_getn(blkmap, 0, mp->m_dir_geo->fsbcount, &bmp);
	v = id->ilist || verbose;
	if (nex == 0) {
		if (!sflag || v)
			dbprintf(_("block 0 for directory inode %lld is "
				 "missing\n"),
				id->ino);
		error++;
		return 0;
	}
	push_cur();
	if (nex > 1)
		make_bbmap(&bbmap, nex, bmp);
	set_cur(&typtab[TYP_DIR2], XFS_FSB_TO_DADDR(mp, bmp->startblock),
		mp->m_dir_geo->fsbcount * blkbb, DB_RING_IGN, nex > 1 ? &bbmap : NULL);
	for (i = 0; !v && i < nex; i++) {
		for (b = bmp[i].startblock;
		     !v && b < bmp[i].startblock + bmp[i].blockcount;
		     b++)
			v = CHECK_BLIST(b);
	}
	free(bmp);
	if (iocur_top->data == NULL) {
		if (!sflag || id->ilist || v)
			dbprintf(_("can't read block 0 for directory inode "
				 "%lld\n"),
				id->ino);
		error++;
		pop_cur();
		return 0;
	}
	dir_hash_init();
	parent = process_data_dir_v2(dot, dotdot, id, v, mp->m_dir_geo->datablk,
		NULL);
	dir_hash_check(id, v);
	dir_hash_done();
	pop_cur();
	return parent;
}

static void
process_bmbt_reclist(
	xfs_bmbt_rec_t		*rp,
	xfs_extnum_t		numrecs,
	dbm_t			type,
	inodata_t		*id,
	xfs_rfsblock_t		*tot,
	blkmap_t		**blkmapp)
{
	xfs_agblock_t		agbno;
	xfs_agnumber_t		agno;
	xfs_fsblock_t		b;
	xfs_filblks_t		c;
	xfs_filblks_t		cp;
	int			f;
	xfs_extnum_t		i;
	xfs_agblock_t		iagbno;
	xfs_agnumber_t		iagno;
	xfs_fileoff_t		o;
	xfs_fileoff_t		op;
	xfs_fsblock_t		s;
	int			v;

	cp = op = 0;
	v = verbose || id->ilist;
	iagno = XFS_INO_TO_AGNO(mp, id->ino);
	iagbno = XFS_INO_TO_AGBNO(mp, id->ino);
	for (i = 0; i < numrecs; i++, rp++) {
		convert_extent(rp, &o, &s, &c, &f);
		if (v)
			dbprintf(_("inode %lld extent [%lld,%lld,%lld,%d]\n"),
				id->ino, o, s, c, f);
		if (!sflag && i > 0 && op + cp > o)
			dbprintf(_("bmap rec out of order, inode %lld entry %d\n"),
				id->ino, i);
		op = o;
		cp = c;
		if (type == DBM_RTDATA) {
			if (!sflag && s >= mp->m_sb.sb_rblocks) {
				dbprintf(_("inode %lld bad rt block number %lld, "
					 "offset %lld\n"),
					id->ino, s, o);
				continue;
			}
		} else if (!sflag) {
			agno = XFS_FSB_TO_AGNO(mp, s);
			agbno = XFS_FSB_TO_AGBNO(mp, s);
			if (agno >= mp->m_sb.sb_agcount ||
			    agbno >= mp->m_sb.sb_agblocks) {
				dbprintf(_("inode %lld bad block number %lld "
					 "[%d,%d], offset %lld\n"),
					id->ino, s, agno, agbno, o);
				continue;
			}
			if (agbno + c - 1 >= mp->m_sb.sb_agblocks) {
				dbprintf(_("inode %lld bad block number %lld "
					 "[%d,%d], offset %lld\n"),
					id->ino, s + c - 1, agno,
					agbno + (xfs_agblock_t)c - 1, o);
				continue;
			}
		}
		if (blkmapp && *blkmapp)
			blkmap_set_ext(blkmapp, (xfs_fileoff_t)o,
				(xfs_fsblock_t)s, (xfs_extlen_t)c);
		if (type == DBM_RTDATA) {
			set_rdbmap((xfs_fsblock_t)s, (xfs_extlen_t)c,
				DBM_RTDATA);
			set_rinomap((xfs_fsblock_t)s, (xfs_extlen_t)c, id);
			for (b = (xfs_fsblock_t)s;
			     blist_size && b < s + c;
			     b++, o++) {
				if (CHECK_BLIST(b))
					dbprintf(_("inode %lld block %lld at "
						 "offset %lld\n"),
						id->ino, (xfs_fsblock_t)b, o);
			}
		} else {
			agno = XFS_FSB_TO_AGNO(mp, (xfs_fsblock_t)s);
			agbno = XFS_FSB_TO_AGBNO(mp, (xfs_fsblock_t)s);
			set_dbmap(agno, agbno, (xfs_extlen_t)c, type, iagno,
				iagbno);
			set_inomap(agno, agbno, (xfs_extlen_t)c, id);
			for (b = (xfs_fsblock_t)s;
			     blist_size && b < s + c;
			     b++, o++, agbno++) {
				if (CHECK_BLIST(b))
					dbprintf(_("inode %lld block %lld at "
						 "offset %lld\n"),
						id->ino, (xfs_fsblock_t)b, o);
			}
		}
		*tot += c;
	}
}

static void
process_btinode(
	inodata_t		*id,
	struct xfs_dinode	*dip,
	dbm_t			type,
	xfs_rfsblock_t		*totd,
	xfs_rfsblock_t		*toti,
	xfs_extnum_t		*nex,
	blkmap_t		**blkmapp,
	int			whichfork)
{
	xfs_bmdr_block_t	*dib;
	int			i;
	xfs_bmbt_ptr_t		*pp;

	dib = (xfs_bmdr_block_t *)XFS_DFORK_PTR(dip, whichfork);
	if (be16_to_cpu(dib->bb_level) >= XFS_BM_MAXLEVELS(mp, whichfork)) {
		if (!sflag || id->ilist)
			dbprintf(_("level for ino %lld %s fork bmap root too "
				 "large (%u)\n"),
				id->ino,
				whichfork == XFS_DATA_FORK ? _("data") : _("attr"),
				be16_to_cpu(dib->bb_level));
		error++;
		return;
	}
	if (be16_to_cpu(dib->bb_numrecs) >
			libxfs_bmdr_maxrecs(XFS_DFORK_SIZE(dip, mp, whichfork),
			be16_to_cpu(dib->bb_level) == 0)) {
		if (!sflag || id->ilist)
			dbprintf(_("numrecs for ino %lld %s fork bmap root too "
				 "large (%u)\n"),
				id->ino,
				whichfork == XFS_DATA_FORK ? _("data") : _("attr"),
				be16_to_cpu(dib->bb_numrecs));
		error++;
		return;
	}
	if (be16_to_cpu(dib->bb_level) == 0) {
		xfs_bmbt_rec_t	*rp = XFS_BMDR_REC_ADDR(dib, 1);
		process_bmbt_reclist(rp, be16_to_cpu(dib->bb_numrecs), type,
							id, totd, blkmapp);
		*nex += be16_to_cpu(dib->bb_numrecs);
		return;
	} else {
		pp = XFS_BMDR_PTR_ADDR(dib, 1, libxfs_bmdr_maxrecs(
				XFS_DFORK_SIZE(dip, mp, whichfork), 0));
		for (i = 0; i < be16_to_cpu(dib->bb_numrecs); i++)
			scan_lbtree(get_unaligned_be64(&pp[i]),
					be16_to_cpu(dib->bb_level),
					scanfunc_bmap, type, id, totd, toti,
					nex, blkmapp, 1,
					whichfork == XFS_DATA_FORK ?
						TYP_BMAPBTD : TYP_BMAPBTA);
	}
	if (*nex <= XFS_DFORK_SIZE(dip, mp, whichfork) / sizeof(xfs_bmbt_rec_t)) {
		if (!sflag || id->ilist)
			dbprintf(_("extent count for ino %lld %s fork too low "
				 "(%d) for file format\n"),
				id->ino,
				whichfork == XFS_DATA_FORK ? _("data") : _("attr"),
				*nex);
		error++;
	}
}

static xfs_ino_t
process_data_dir_v2(
	int			*dot,
	int			*dotdot,
	inodata_t		*id,
	int			v,
	xfs_dablk_t		dabno,
	freetab_t		**freetabp)
{
	xfs_dir2_dataptr_t	addr;
	xfs_dir2_data_free_t	*bf;
	int			bf_err;
	struct xfs_dir2_data_hdr *block;
	xfs_dir2_block_tail_t	*btp = NULL;
	inodata_t		*cid;
	int			count;
	struct xfs_dir2_data_hdr *data;
	xfs_dir2_db_t		db;
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_free_t	*dfp;
	xfs_dir2_data_unused_t	*dup;
	char			*endptr;
	int			freeseen;
	freetab_t		*freetab;
	int			i;
	int			lastfree;
	int			lastfree_err;
	xfs_dir2_leaf_entry_t	*lep = NULL;
	xfs_ino_t		lino;
	xfs_ino_t		parent = 0;
	char			*ptr;
	int			stale = 0;
	int			tag_err;
	__be16			*tagp;
	struct xfs_name		xname;

	data = iocur_top->data;
	block = iocur_top->data;
	if (be32_to_cpu(block->magic) != XFS_DIR2_BLOCK_MAGIC &&
			be32_to_cpu(data->magic) != XFS_DIR2_DATA_MAGIC &&
			be32_to_cpu(block->magic) != XFS_DIR3_BLOCK_MAGIC &&
			be32_to_cpu(data->magic) != XFS_DIR3_DATA_MAGIC) {
		if (!sflag || v)
			dbprintf(_("bad directory data magic # %#x for dir ino "
				 "%lld block %d\n"),
				be32_to_cpu(data->magic), id->ino, dabno);
		error++;
		return NULLFSINO;
	}
	db = xfs_dir2_da_to_db(mp->m_dir_geo, dabno);
	bf = libxfs_dir2_data_bestfree_p(mp, data);
	ptr = (char *)data + mp->m_dir_geo->data_entry_offset;
	if (be32_to_cpu(block->magic) == XFS_DIR2_BLOCK_MAGIC ||
	    be32_to_cpu(block->magic) == XFS_DIR3_BLOCK_MAGIC) {
		btp = xfs_dir2_block_tail_p(mp->m_dir_geo, block);
		lep = xfs_dir2_block_leaf_p(btp);
		endptr = (char *)lep;
		if (endptr <= ptr || endptr > (char *)btp) {
			endptr = (char *)data + mp->m_dir_geo->blksize;
			lep = NULL;
			if (!sflag || v)
				dbprintf(_("bad block directory tail for dir ino "
					 "%lld\n"),
					id->ino);
			error++;
		}
	} else
		endptr = (char *)data + mp->m_dir_geo->blksize;
	bf_err = lastfree_err = tag_err = 0;
	count = lastfree = freeseen = 0;
	if (be16_to_cpu(bf[0].length) == 0) {
		bf_err += be16_to_cpu(bf[0].offset) != 0;
		freeseen |= 1 << 0;
	}
	if (be16_to_cpu(bf[1].length) == 0) {
		bf_err += be16_to_cpu(bf[1].offset) != 0;
		freeseen |= 1 << 1;
	}
	if (be16_to_cpu(bf[2].length) == 0) {
		bf_err += be16_to_cpu(bf[2].offset) != 0;
		freeseen |= 1 << 2;
	}
	bf_err += be16_to_cpu(bf[0].length) < be16_to_cpu(bf[1].length);
	bf_err += be16_to_cpu(bf[1].length) < be16_to_cpu(bf[2].length);
	if (freetabp) {
		freetab = *freetabp;
		if (freetab->naents <= db) {
			*freetabp = freetab =
				realloc(freetab, FREETAB_SIZE(db + 1));
			for (i = freetab->naents; i < db; i++)
				freetab->ents[i] = NULLDATAOFF;
			freetab->naents = db + 1;
		}
		if (freetab->nents < db + 1)
			freetab->nents = db + 1;
		freetab->ents[db] = be16_to_cpu(bf[0].length);
	}
	while (ptr < endptr) {
		dup = (xfs_dir2_data_unused_t *)ptr;
		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
			lastfree_err += lastfree != 0;
			tagp = xfs_dir2_data_unused_tag_p(dup);
			if ((be16_to_cpu(dup->length) & (XFS_DIR2_DATA_ALIGN - 1)) ||
					be16_to_cpu(dup->length) == 0 ||
					(char *)tagp >= endptr) {
				if (!sflag || v)
					dbprintf(_("dir %lld block %d bad free "
						 "entry at %d\n"),
						id->ino, dabno,
						(int)((char *)dup -
						      (char *)data));
				error++;
				break;
			}
			tag_err += be16_to_cpu(*tagp) != (char *)dup - (char *)data;
			dfp = process_data_dir_v2_freefind(data, dup);
			if (dfp) {
				i = (int)(dfp - bf);
				bf_err += (freeseen & (1 << i)) != 0;
				freeseen |= 1 << i;
			} else
				bf_err += be16_to_cpu(dup->length) >
						be16_to_cpu(bf[2].length);
			ptr += be16_to_cpu(dup->length);
			lastfree = 1;
			continue;
		}
		dep = (xfs_dir2_data_entry_t *)dup;
		if (dep->namelen == 0) {
			if (!sflag || v)
				dbprintf(_("dir %lld block %d zero length entry "
					 "at %d\n"),
					id->ino, dabno,
					(int)((char *)dep - (char *)data));
			error++;
		}
		tagp = libxfs_dir2_data_entry_tag_p(mp, dep);
		if ((char *)tagp >= endptr) {
			if (!sflag || v)
				dbprintf(_("dir %lld block %d bad entry at %d\n"),
					id->ino, dabno,
					(int)((char *)dep - (char *)data));
			error++;
			break;
		}
		tag_err += be16_to_cpu(*tagp) != (char *)dep - (char *)data;
		addr = xfs_dir2_db_off_to_dataptr(mp->m_dir_geo, db,
			(char *)dep - (char *)data);
		xname.name = dep->name;
		xname.len = dep->namelen;
		dir_hash_add(libxfs_dir2_hashname(mp, &xname), addr);
		ptr += libxfs_dir2_data_entsize(mp, dep->namelen);
		count++;
		lastfree = 0;
		lino = be64_to_cpu(dep->inumber);
		cid = find_inode(lino, 1);
		if (v)
			dbprintf(_("dir %lld block %d entry %*.*s %lld\n"),
				id->ino, dabno, dep->namelen, dep->namelen,
				dep->name, lino);
		if (cid)
			addlink_inode(cid);
		else {
			if (!sflag || v)
				dbprintf(_("dir %lld block %d entry %*.*s bad "
					 "inode number %lld\n"),
					id->ino, dabno, dep->namelen,
					dep->namelen, dep->name, lino);
			error++;
		}
		if (dep->namelen == 2 && dep->name[0] == '.' &&
		    dep->name[1] == '.') {
			if (parent) {
				if (!sflag || v)
					dbprintf(_("multiple .. entries in dir "
						 "%lld (%lld, %lld)\n"),
						id->ino, parent, lino);
				error++;
			} else
				parent = cid ? lino : NULLFSINO;
			(*dotdot)++;
		} else if (dep->namelen != 1 || dep->name[0] != '.') {
			if (cid != NULL) {
				if (!cid->parent)
					cid->parent = id;
				addname_inode(cid, (char *)dep->name,
					dep->namelen);
			}
		} else {
			if (lino != id->ino) {
				if (!sflag || v)
					dbprintf(_("dir %lld entry . inode "
						 "number mismatch (%lld)\n"),
						id->ino, lino);
				error++;
			}
			(*dot)++;
		}
	}
	if (be32_to_cpu(data->magic) == XFS_DIR2_BLOCK_MAGIC ||
	    be32_to_cpu(data->magic) == XFS_DIR3_BLOCK_MAGIC) {
		endptr = (char *)data + mp->m_dir_geo->blksize;
		for (i = stale = 0; lep && i < be32_to_cpu(btp->count); i++) {
			if ((char *)&lep[i] >= endptr) {
				if (!sflag || v)
					dbprintf(_("dir %lld block %d bad count "
						 "%u\n"), id->ino, dabno,
						be32_to_cpu(btp->count));
				error++;
				break;
			}
			if (be32_to_cpu(lep[i].address) == XFS_DIR2_NULL_DATAPTR)
				stale++;
			else if (dir_hash_see(be32_to_cpu(lep[i].hashval),
						be32_to_cpu(lep[i].address))) {
				if (!sflag || v)
					dbprintf(_("dir %lld block %d extra leaf "
						 "entry %x %x\n"),
						id->ino, dabno,
						be32_to_cpu(lep[i].hashval),
						be32_to_cpu(lep[i].address));
				error++;
			}
		}
	}
	bf_err += freeseen != 7;
	if (bf_err) {
		if (!sflag || v)
			dbprintf(_("dir %lld block %d bad bestfree data\n"),
				id->ino, dabno);
		error++;
	}
	if ((be32_to_cpu(data->magic) == XFS_DIR2_BLOCK_MAGIC ||
	     be32_to_cpu(data->magic) == XFS_DIR3_BLOCK_MAGIC) &&
	    count != be32_to_cpu(btp->count) - be32_to_cpu(btp->stale)) {
		if (!sflag || v)
			dbprintf(_("dir %lld block %d bad block tail count %d "
				 "(stale %d)\n"),
				id->ino, dabno, be32_to_cpu(btp->count),
				be32_to_cpu(btp->stale));
		error++;
	}
	if ((be32_to_cpu(data->magic) == XFS_DIR2_BLOCK_MAGIC ||
	     be32_to_cpu(data->magic) == XFS_DIR3_BLOCK_MAGIC) &&
					stale != be32_to_cpu(btp->stale)) {
		if (!sflag || v)
			dbprintf(_("dir %lld block %d bad stale tail count %d\n"),
				id->ino, dabno, be32_to_cpu(btp->stale));
		error++;
	}
	if (lastfree_err) {
		if (!sflag || v)
			dbprintf(_("dir %lld block %d consecutive free entries\n"),
				id->ino, dabno);
		error++;
	}
	if (tag_err) {
		if (!sflag || v)
			dbprintf(_("dir %lld block %d entry/unused tag "
				 "mismatch\n"),
				id->ino, dabno);
		error++;
	}
	return parent;
}

static xfs_dir2_data_free_t *
process_data_dir_v2_freefind(
	struct xfs_dir2_data_hdr *data,
	xfs_dir2_data_unused_t	*dup)
{
	struct xfs_dir2_data_free *bf;
	struct xfs_dir2_data_free *dfp;
	xfs_dir2_data_aoff_t	off;

	off = (xfs_dir2_data_aoff_t)((char *)dup - (char *)data);
	bf = libxfs_dir2_data_bestfree_p(mp, data);
	if (be16_to_cpu(dup->length) <
			be16_to_cpu(bf[XFS_DIR2_DATA_FD_COUNT - 1].length))
		return NULL;
	for (dfp = bf; dfp < &bf[XFS_DIR2_DATA_FD_COUNT]; dfp++) {
		if (be16_to_cpu(dfp->offset) == 0)
			return NULL;
		if (be16_to_cpu(dfp->offset) == off)
			return dfp;
	}
	return NULL;
}

static void
process_dir(
	struct xfs_dinode	*dip,
	blkmap_t		*blkmap,
	inodata_t		*id)
{
	xfs_fsblock_t		bno;
	int			dot;
	int			dotdot;
	xfs_ino_t		parent;

	dot = dotdot = 0;
	if (process_dir_v2(dip, blkmap, &dot, &dotdot, id, &parent))
		return;

	bno = XFS_INO_TO_FSB(mp, id->ino);
	if (dot == 0) {
		if (!sflag || id->ilist || CHECK_BLIST(bno))
			dbprintf(_("no . entry for directory %lld\n"), id->ino);
		error++;
	}
	if (dotdot == 0) {
		if (!sflag || id->ilist || CHECK_BLIST(bno))
			dbprintf(_("no .. entry for directory %lld\n"), id->ino);
		error++;
	} else if (parent == id->ino && id->ino != mp->m_sb.sb_rootino) {
		if (!sflag || id->ilist || CHECK_BLIST(bno))
			dbprintf(_(". and .. same for non-root directory %lld\n"),
				id->ino);
		error++;
	} else if (id->ino == mp->m_sb.sb_rootino && id->ino != parent) {
		if (!sflag || id->ilist || CHECK_BLIST(bno))
			dbprintf(_("root directory %lld has .. %lld\n"), id->ino,
				parent);
		error++;
	} else if (parent != NULLFSINO && id->ino != parent)
		addparent_inode(id, parent);
}

static int
process_dir_v2(
	struct xfs_dinode	*dip,
	blkmap_t		*blkmap,
	int			*dot,
	int			*dotdot,
	inodata_t		*id,
	xfs_ino_t		*parent)
{
	xfs_fileoff_t		last = 0;
	xfs_fsize_t		size = be64_to_cpu(dip->di_size);

	if (blkmap)
		last = blkmap_last_off(blkmap);
	if (size <= XFS_DFORK_DSIZE(dip, mp) &&
				dip->di_format == XFS_DINODE_FMT_LOCAL)
		*parent = process_sf_dir_v2(dip, dot, dotdot, id);
	else if (last == mp->m_dir_geo->fsbcount &&
			(dip->di_format == XFS_DINODE_FMT_EXTENTS ||
			dip->di_format == XFS_DINODE_FMT_BTREE))
		*parent = process_block_dir_v2(blkmap, dot, dotdot, id);
	else if (last >= mp->m_dir_geo->leafblk + mp->m_dir_geo->fsbcount &&
			(dip->di_format == XFS_DINODE_FMT_EXTENTS ||
			dip->di_format == XFS_DINODE_FMT_BTREE))
		*parent = process_leaf_node_dir_v2(blkmap, dot, dotdot, id, size);
	else  {
		dbprintf(_("bad size (%lld) or format (%d) for directory inode "
			 "%lld\n"),
			size, dip->di_format, id->ino);
		error++;
		return 1;
	}
	return 0;
}

/* ARGSUSED */
static void
process_exinode(
	inodata_t		*id,
	struct xfs_dinode	*dip,
	dbm_t			type,
	xfs_rfsblock_t		*totd,
	xfs_rfsblock_t		*toti,
	xfs_extnum_t		*nex,
	blkmap_t		**blkmapp,
	int			whichfork)
{
	xfs_bmbt_rec_t		*rp;
	xfs_extnum_t		max_nex;

	rp = (xfs_bmbt_rec_t *)XFS_DFORK_PTR(dip, whichfork);
	*nex = xfs_dfork_nextents(dip, whichfork);
	max_nex = xfs_iext_max_nextents(
			xfs_dinode_has_large_extent_counts(dip),
			whichfork);
	if (*nex > max_nex || *nex > XFS_DFORK_SIZE(dip, mp, whichfork) /
						sizeof(xfs_bmbt_rec_t)) {
		if (!sflag || id->ilist)
			dbprintf(_("bad number of extents %llu for inode %lld\n"),
				(unsigned long long)*nex, id->ino);
		error++;
		return;
	}
	process_bmbt_reclist(rp, *nex, type, id, totd, blkmapp);
}

static void
process_inode(
	xfs_agf_t		*agf,
	xfs_agino_t		agino,
	struct xfs_dinode	*dip,
	int			isfree)
{
	blkmap_t		*blkmap;
	xfs_fsblock_t		bno = 0;
	inodata_t		*id = NULL;
	xfs_ino_t		ino;
	xfs_extnum_t		nextents = 0;
	xfs_extnum_t		dnextents;
	int			security;
	xfs_rfsblock_t		totblocks;
	xfs_rfsblock_t		totdblocks = 0;
	xfs_rfsblock_t		totiblocks = 0;
	dbm_t			type;
	xfs_extnum_t		anextents = 0;
	xfs_extnum_t		danextents;
	xfs_rfsblock_t		atotdblocks = 0;
	xfs_rfsblock_t		atotiblocks = 0;
	xfs_qcnt_t		bc = 0;
	xfs_qcnt_t		ic = 0;
	xfs_qcnt_t		rc = 0;
	int			v = 0;
	mode_t			mode;
	uint16_t		diflags;
	uint64_t		diflags2 = 0;
	xfs_nlink_t		nlink;
	xfs_dqid_t		uid;
	xfs_dqid_t		gid;
	xfs_dqid_t		prid;
	static char		okfmts[] = {
		0,				/* type 0 unused */
		1 << XFS_DINODE_FMT_DEV,	/* FIFO */
		1 << XFS_DINODE_FMT_DEV,	/* CHR */
		0,				/* type 3 unused */
		(1 << XFS_DINODE_FMT_LOCAL) |
		(1 << XFS_DINODE_FMT_EXTENTS) |
		(1 << XFS_DINODE_FMT_BTREE),	/* DIR */
		0,				/* type 5 unused */
		1 << XFS_DINODE_FMT_DEV,	/* BLK */
		0,				/* type 7 unused */
		(1 << XFS_DINODE_FMT_EXTENTS) |
		(1 << XFS_DINODE_FMT_BTREE),	/* REG */
		0,				/* type 9 unused */
		(1 << XFS_DINODE_FMT_LOCAL) |
		(1 << XFS_DINODE_FMT_EXTENTS),	/* LNK */
		0,				/* type 11 unused */
		1 << XFS_DINODE_FMT_DEV,	/* SOCK */
		0,				/* type 13 unused */
		1 << XFS_DINODE_FMT_UUID,	/* MNT */
		0				/* type 15 unused */
	};
	static char		*fmtnames[] = {
		"dev", "local", "extents", "btree", "uuid"
	};

	ino = XFS_AGINO_TO_INO(mp, be32_to_cpu(agf->agf_seqno), agino);
	if (!isfree) {
		id = find_inode(ino, 1);
		bno = XFS_INO_TO_FSB(mp, ino);
		blkmap = NULL;
	}
	v = (!sflag || (id && id->ilist) || CHECK_BLIST(bno));
	if (dip->di_magic != cpu_to_be16(XFS_DINODE_MAGIC)) {
		if (isfree || v)
			dbprintf(_("bad magic number %#x for inode %lld\n"),
				be16_to_cpu(dip->di_magic), ino);
		error++;
		return;
	}
	if (!libxfs_dinode_good_version(mp, dip->di_version)) {
		if (isfree || v)
			dbprintf(_("bad version number %#x for inode %lld\n"),
				dip->di_version, ino);
		error++;
		return;
	}
	if (dip->di_version == 1) {
		nlink = be16_to_cpu(dip->di_onlink);
		prid = 0;
	} else {
		nlink = be32_to_cpu(dip->di_nlink);
		prid = (xfs_dqid_t)be16_to_cpu(dip->di_projid_hi) << 16 |
				   be16_to_cpu(dip->di_projid_lo);
	}
	uid = be32_to_cpu(dip->di_uid);
	gid = be32_to_cpu(dip->di_gid);
	diflags = be16_to_cpu(dip->di_flags);
	if (xfs_has_v3inodes(mp))
		diflags2 = be64_to_cpu(dip->di_flags2);
	if (isfree) {
		if (be64_to_cpu(dip->di_nblocks) != 0) {
			if (v)
				dbprintf(_("bad nblocks %lld for free inode "
					 "%lld\n"),
					be64_to_cpu(dip->di_nblocks), ino);
			error++;
		}
		if (nlink != 0) {
			if (v)
				dbprintf(_("bad nlink %d for free inode %lld\n"),
					nlink, ino);
			error++;
		}
		if (dip->di_mode != 0) {
			if (v)
				dbprintf(_("bad mode %#o for free inode %lld\n"),
					be16_to_cpu(dip->di_mode), ino);
			error++;
		}
		return;
	}

	if (be32_to_cpu(dip->di_next_unlinked) != NULLAGINO) {
		if (v)
			dbprintf(_("bad next unlinked %#x for inode %lld\n"),
				be32_to_cpu(dip->di_next_unlinked), ino);
		error++;
	}
	/*
	 * di_mode is a 16-bit uint so no need to check the < 0 case
	 */
	mode = be16_to_cpu(dip->di_mode);
	if ((((mode & S_IFMT) >> 12) > 15) ||
	    (!(okfmts[(mode & S_IFMT) >> 12] & (1 << dip->di_format)))) {
		if (v)
			dbprintf(_("bad format %d for inode %lld type %#o\n"),
				dip->di_format, id->ino, mode & S_IFMT);
		error++;
		return;
	}
	if ((unsigned int)XFS_DFORK_ASIZE(dip, mp) >= XFS_LITINO(mp)) {
		if (v)
			dbprintf(_("bad fork offset %d for inode %lld\n"),
				dip->di_forkoff, id->ino);
		error++;
		return;
	}
	if ((unsigned int)dip->di_aformat > XFS_DINODE_FMT_BTREE)  {
		if (v)
			dbprintf(_("bad attribute format %d for inode %lld\n"),
				dip->di_aformat, id->ino);
		error++;
		return;
	}

	dnextents = xfs_dfork_data_extents(dip);
	danextents = xfs_dfork_attr_extents(dip);

	if (verbose || (id && id->ilist) || CHECK_BLIST(bno))
		dbprintf(_("inode %lld mode %#o fmt %s "
			 "afmt %s "
			 "nex %d anex %d nblk %lld sz %lld%s%s%s%s%s%s%s\n"),
			id->ino, mode, fmtnames[(int)dip->di_format],
			fmtnames[(int)dip->di_aformat],
			dnextents, danextents,
			be64_to_cpu(dip->di_nblocks), be64_to_cpu(dip->di_size),
			diflags & XFS_DIFLAG_REALTIME ? " rt" : "",
			diflags & XFS_DIFLAG_PREALLOC ? " pre" : "",
			diflags & XFS_DIFLAG_IMMUTABLE? " imm" : "",
			diflags & XFS_DIFLAG_APPEND   ? " app" : "",
			diflags & XFS_DIFLAG_SYNC     ? " syn" : "",
			diflags & XFS_DIFLAG_NOATIME  ? " noa" : "",
			diflags & XFS_DIFLAG_NODUMP   ? " nod" : "");
	security = 0;
	switch (mode & S_IFMT) {
	case S_IFDIR:
		type = DBM_DIR;
		if (dip->di_format == XFS_DINODE_FMT_LOCAL)
			break;
		blkmap = blkmap_alloc(dnextents);
		break;
	case S_IFREG:
		if (diflags & XFS_DIFLAG_REALTIME)
			type = DBM_RTDATA;
		else if (id->ino == mp->m_sb.sb_rbmino) {
			type = DBM_RTBITMAP;
			blkmap = blkmap_alloc(dnextents);
			addlink_inode(id);
		} else if (id->ino == mp->m_sb.sb_rsumino) {
			type = DBM_RTSUM;
			blkmap = blkmap_alloc(dnextents);
			addlink_inode(id);
		}
		else if (id->ino == mp->m_sb.sb_uquotino ||
			 id->ino == mp->m_sb.sb_gquotino ||
			 id->ino == mp->m_sb.sb_pquotino) {
			type = DBM_QUOTA;
			blkmap = blkmap_alloc(dnextents);
			addlink_inode(id);
		}
		else
			type = DBM_DATA;
		if (mode & (S_ISUID | S_ISGID))
			security = 1;
		break;
	case S_IFLNK:
		type = DBM_SYMLINK;
		break;
	default:
		security = 1;
		type = DBM_UNKNOWN;
		break;
	}

	id->isreflink = !!(diflags2 & XFS_DIFLAG2_REFLINK);
	setlink_inode(id, nlink, type == DBM_DIR, security);

	switch (dip->di_format) {
	case XFS_DINODE_FMT_LOCAL:
		process_lclinode(id, dip, type, &totdblocks, &totiblocks,
			&nextents, &blkmap, XFS_DATA_FORK);
		break;
	case XFS_DINODE_FMT_EXTENTS:
		process_exinode(id, dip, type, &totdblocks, &totiblocks,
			&nextents, &blkmap, XFS_DATA_FORK);
		break;
	case XFS_DINODE_FMT_BTREE:
		process_btinode(id, dip, type, &totdblocks, &totiblocks,
			&nextents, &blkmap, XFS_DATA_FORK);
		break;
	}
	if (dip->di_forkoff) {
		sbversion |= XFS_SB_VERSION_ATTRBIT;
		switch (dip->di_aformat) {
		case XFS_DINODE_FMT_LOCAL:
			process_lclinode(id, dip, DBM_ATTR, &atotdblocks,
				&atotiblocks, &anextents, NULL, XFS_ATTR_FORK);
			break;
		case XFS_DINODE_FMT_EXTENTS:
			process_exinode(id, dip, DBM_ATTR, &atotdblocks,
				&atotiblocks, &anextents, NULL, XFS_ATTR_FORK);
			break;
		case XFS_DINODE_FMT_BTREE:
			process_btinode(id, dip, DBM_ATTR, &atotdblocks,
				&atotiblocks, &anextents, NULL, XFS_ATTR_FORK);
			break;
		}
	}
	if (qgdo || qpdo || qudo) {
		switch (type) {
		case DBM_DATA:
		case DBM_DIR:
		case DBM_RTBITMAP:
		case DBM_RTSUM:
		case DBM_SYMLINK:
		case DBM_UNKNOWN:
			bc = totdblocks + totiblocks +
			     atotdblocks + atotiblocks;
			ic = 1;
			break;
		case DBM_RTDATA:
			bc = totiblocks + atotdblocks + atotiblocks;
			rc = totdblocks;
			ic = 1;
			break;
		default:
			break;
		}
		if (ic)
			quota_add(&prid, &gid, &uid, 0, bc, ic, rc);
	}
	totblocks = totdblocks + totiblocks + atotdblocks + atotiblocks;
	if (totblocks != be64_to_cpu(dip->di_nblocks)) {
		if (v)
			dbprintf(_("bad nblocks %lld for inode %lld, counted "
				 "%lld\n"),
				be64_to_cpu(dip->di_nblocks), id->ino, totblocks);
		error++;
	}
	if (nextents != dnextents) {
		if (v)
			dbprintf(_("bad nextents %d for inode %lld, counted %d\n"),
				dnextents, id->ino, nextents);
		error++;
	}
	if (anextents != danextents) {
		if (v)
			dbprintf(_("bad anextents %d for inode %lld, counted "
				 "%d\n"),
				danextents, id->ino, anextents);
		error++;
	}
	if (type == DBM_DIR)
		process_dir(dip, blkmap, id);
	else if (type == DBM_RTBITMAP)
		process_rtbitmap(blkmap);
	else if (type == DBM_RTSUM)
		process_rtsummary(blkmap);
	/*
	 * If the CHKD flag is not set, this can legitimately contain garbage;
	 * xfs_repair may have cleared that bit.
	 */
	else if (type == DBM_QUOTA) {
		if (id->ino == mp->m_sb.sb_uquotino &&
		    (mp->m_sb.sb_qflags & XFS_UQUOTA_ACCT) &&
		    (mp->m_sb.sb_qflags & XFS_UQUOTA_CHKD))
			process_quota(IS_USER_QUOTA, id, blkmap);
		else if (id->ino == mp->m_sb.sb_gquotino &&
			 (mp->m_sb.sb_qflags & XFS_GQUOTA_ACCT) &&
			 (mp->m_sb.sb_qflags & XFS_GQUOTA_CHKD))
			process_quota(IS_GROUP_QUOTA, id, blkmap);
		else if (id->ino == mp->m_sb.sb_pquotino &&
			 (mp->m_sb.sb_qflags & XFS_PQUOTA_ACCT) &&
			 (mp->m_sb.sb_qflags & XFS_PQUOTA_CHKD))
			process_quota(IS_PROJECT_QUOTA, id, blkmap);
	}
	if (blkmap)
		blkmap_free(blkmap);
}

/* ARGSUSED */
static void
process_lclinode(
	inodata_t			*id,
	struct xfs_dinode		*dip,
	dbm_t				type,
	xfs_rfsblock_t			*totd,
	xfs_rfsblock_t			*toti,
	xfs_extnum_t			*nex,
	blkmap_t			**blkmapp,
	int				whichfork)
{
	struct xfs_attr_shortform	*asf;
	xfs_fsblock_t			bno;

	bno = XFS_INO_TO_FSB(mp, id->ino);
	if (whichfork == XFS_DATA_FORK && be64_to_cpu(dip->di_size) >
						XFS_DFORK_DSIZE(dip, mp)) {
		if (!sflag || id->ilist || CHECK_BLIST(bno))
			dbprintf(_("local inode %lld data is too large (size "
				 "%lld)\n"),
				id->ino, be64_to_cpu(dip->di_size));
		error++;
	}
	else if (whichfork == XFS_ATTR_FORK) {
		asf = (struct xfs_attr_shortform *)XFS_DFORK_APTR(dip);
		if (be16_to_cpu(asf->hdr.totsize) > XFS_DFORK_ASIZE(dip, mp)) {
			if (!sflag || id->ilist || CHECK_BLIST(bno))
				dbprintf(_("local inode %lld attr is too large "
					 "(size %d)\n"),
					id->ino, be16_to_cpu(asf->hdr.totsize));
			error++;
		}
	}
}

static xfs_ino_t
process_leaf_node_dir_v2(
	blkmap_t		*blkmap,
	int			*dot,
	int			*dotdot,
	inodata_t		*id,
	xfs_fsize_t		dirsize)
{
	xfs_fsblock_t		b;
	bbmap_t			bbmap;
	bmap_ext_t		*bmp;
	xfs_fileoff_t		dbno;
	freetab_t		*freetab;
	int			i;
	xfs_ino_t		lino;
	int			nex;
	xfs_ino_t		parent;
	int			t = 0;
	int			v;
	int			v2;

	v2 = verbose || id->ilist;
	v = parent = 0;
	dbno = NULLFILEOFF;
	freetab = malloc(FREETAB_SIZE(dirsize / mp->m_dir_geo->blksize));
	freetab->naents = (int)(dirsize / mp->m_dir_geo->blksize);
	freetab->nents = 0;
	for (i = 0; i < freetab->naents; i++)
		freetab->ents[i] = NULLDATAOFF;
	dir_hash_init();
	while ((dbno = blkmap_next_off(blkmap, dbno, &t)) != NULLFILEOFF) {
		nex = blkmap_getn(blkmap, dbno, mp->m_dir_geo->fsbcount, &bmp);
		ASSERT(nex > 0);
		for (v = v2, i = 0; !v && i < nex; i++) {
			for (b = bmp[i].startblock;
			     !v && b < bmp[i].startblock + bmp[i].blockcount;
			     b++)
				v = CHECK_BLIST(b);
		}
		if (v)
			dbprintf(_("dir inode %lld block %u=%llu\n"), id->ino,
				(uint32_t)dbno,
				(xfs_fsblock_t)bmp->startblock);
		push_cur();
		if (nex > 1)
			make_bbmap(&bbmap, nex, bmp);
		set_cur(&typtab[TYP_DIR2], XFS_FSB_TO_DADDR(mp, bmp->startblock),
			mp->m_dir_geo->fsbcount * blkbb, DB_RING_IGN,
			nex > 1 ? &bbmap : NULL);
		free(bmp);
		if (iocur_top->data == NULL) {
			if (!sflag || v)
				dbprintf(_("can't read block %u for directory "
					 "inode %lld\n"),
					(uint32_t)dbno, id->ino);
			error++;
			pop_cur();
			dbno += mp->m_dir_geo->fsbcount - 1;
			continue;
		}
		if (dbno < mp->m_dir_geo->leafblk) {
			lino = process_data_dir_v2(dot, dotdot, id, v,
				(xfs_dablk_t)dbno, &freetab);
			if (lino) {
				if (parent) {
					if (!sflag || v)
						dbprintf(_("multiple .. entries "
							 "in dir %lld\n"),
							id->ino);
					error++;
				} else
					parent = lino;
			}
		} else if (dbno < mp->m_dir_geo->freeblk) {
			process_leaf_node_dir_v2_int(id, v, (xfs_dablk_t)dbno,
				freetab);
		} else {
			process_leaf_node_dir_v2_free(id, v, (xfs_dablk_t)dbno,
				freetab);
		}
		pop_cur();
		dbno += mp->m_dir_geo->fsbcount - 1;
	}
	dir_hash_check(id, v);
	dir_hash_done();
	for (i = 0; i < freetab->nents; i++) {
		if (freetab->ents[i] != NULLDATAOFF) {
			if (!sflag || v)
				dbprintf(_("missing free index for data block %d "
					 "in dir ino %lld\n"),
					xfs_dir2_db_to_da(mp->m_dir_geo, i), id->ino);
			error++;
		}
	}
	free(freetab);
	return parent;
}

static void
process_leaf_node_dir_v3_free(
	inodata_t		*id,
	int			v,
	xfs_dablk_t		dabno,
	freetab_t		*freetab)
{
	xfs_dir2_data_off_t	ent;
	struct xfs_dir3_free	*free;
	int			i;
	int			maxent;
	int			used;

	free = iocur_top->data;
	maxent = mp->m_dir_geo->free_max_bests;
	if (be32_to_cpu(free->hdr.firstdb) != xfs_dir2_da_to_db(mp->m_dir_geo,
					dabno - mp->m_dir_geo->freeblk) * maxent) {
		if (!sflag || v)
			dbprintf(_("bad free block firstdb %d for dir ino %lld "
				 "block %d\n"),
				be32_to_cpu(free->hdr.firstdb), id->ino, dabno);
		error++;
		return;
	}
	if (be32_to_cpu(free->hdr.nvalid) > maxent ||
				be32_to_cpu(free->hdr.nused) > maxent ||
				be32_to_cpu(free->hdr.nused) >
					be32_to_cpu(free->hdr.nvalid)) {
		if (!sflag || v)
			dbprintf(_("bad free block nvalid/nused %d/%d for dir "
				 "ino %lld block %d\n"),
				be32_to_cpu(free->hdr.nvalid),
				be32_to_cpu(free->hdr.nused), id->ino, dabno);
		error++;
		return;
	}
	for (used = i = 0; i < be32_to_cpu(free->hdr.nvalid); i++) {
		if (freetab->nents <= be32_to_cpu(free->hdr.firstdb) + i)
			ent = NULLDATAOFF;
		else
			ent = freetab->ents[be32_to_cpu(free->hdr.firstdb) + i];
		if (ent != be16_to_cpu(free->bests[i])) {
			if (!sflag || v)
				dbprintf(_("bad free block ent %d is %d should "
					 "be %d for dir ino %lld block %d\n"),
					i, be16_to_cpu(free->bests[i]), ent,
					id->ino, dabno);
			error++;
		}
		if (be16_to_cpu(free->bests[i]) != NULLDATAOFF)
			used++;
		if (ent != NULLDATAOFF)
			freetab->ents[be32_to_cpu(free->hdr.firstdb) + i] =
								NULLDATAOFF;
	}
	if (used != be32_to_cpu(free->hdr.nused)) {
		if (!sflag || v)
			dbprintf(_("bad free block nused %d should be %d for dir "
				 "ino %lld block %d\n"),
				be32_to_cpu(free->hdr.nused), used, id->ino,
				dabno);
		error++;
	}
}

static void
process_leaf_node_dir_v2_free(
	inodata_t		*id,
	int			v,
	xfs_dablk_t		dabno,
	freetab_t		*freetab)
{
	xfs_dir2_data_off_t	ent;
	xfs_dir2_free_t		*free;
	int			i;
	int			maxent;
	int			used;

	free = iocur_top->data;
	if (be32_to_cpu(free->hdr.magic) != XFS_DIR2_FREE_MAGIC &&
	    be32_to_cpu(free->hdr.magic) != XFS_DIR3_FREE_MAGIC) {
		if (!sflag || v)
			dbprintf(_("bad free block magic # %#x for dir ino %lld "
				 "block %d\n"),
				be32_to_cpu(free->hdr.magic), id->ino, dabno);
		error++;
		return;
	}
	if (be32_to_cpu(free->hdr.magic) == XFS_DIR3_FREE_MAGIC) {
		process_leaf_node_dir_v3_free(id, v, dabno, freetab);
		return;
	}
	maxent = mp->m_dir_geo->free_max_bests;
	if (be32_to_cpu(free->hdr.firstdb) != xfs_dir2_da_to_db(mp->m_dir_geo,
					dabno - mp->m_dir_geo->freeblk) * maxent) {
		if (!sflag || v)
			dbprintf(_("bad free block firstdb %d for dir ino %lld "
				 "block %d\n"),
				be32_to_cpu(free->hdr.firstdb), id->ino, dabno);
		error++;
		return;
	}
	if (be32_to_cpu(free->hdr.nvalid) > maxent ||
				be32_to_cpu(free->hdr.nused) > maxent ||
				be32_to_cpu(free->hdr.nused) >
					be32_to_cpu(free->hdr.nvalid)) {
		if (!sflag || v)
			dbprintf(_("bad free block nvalid/nused %d/%d for dir "
				 "ino %lld block %d\n"),
				be32_to_cpu(free->hdr.nvalid),
				be32_to_cpu(free->hdr.nused), id->ino, dabno);
		error++;
		return;
	}
	for (used = i = 0; i < be32_to_cpu(free->hdr.nvalid); i++) {
		if (freetab->nents <= be32_to_cpu(free->hdr.firstdb) + i)
			ent = NULLDATAOFF;
		else
			ent = freetab->ents[be32_to_cpu(free->hdr.firstdb) + i];
		if (ent != be16_to_cpu(free->bests[i])) {
			if (!sflag || v)
				dbprintf(_("bad free block ent %d is %d should "
					 "be %d for dir ino %lld block %d\n"),
					i, be16_to_cpu(free->bests[i]), ent,
					id->ino, dabno);
			error++;
		}
		if (be16_to_cpu(free->bests[i]) != NULLDATAOFF)
			used++;
		if (ent != NULLDATAOFF)
			freetab->ents[be32_to_cpu(free->hdr.firstdb) + i] =
								NULLDATAOFF;
	}
	if (used != be32_to_cpu(free->hdr.nused)) {
		if (!sflag || v)
			dbprintf(_("bad free block nused %d should be %d for dir "
				 "ino %lld block %d\n"),
				be32_to_cpu(free->hdr.nused), used, id->ino,
				dabno);
		error++;
	}
}

/*
 * Get address of the bestcount field in the single-leaf block.
 */
static inline int
xfs_dir3_leaf_ents_count(struct xfs_dir2_leaf *lp)
{
	if (lp->hdr.info.magic == cpu_to_be16(XFS_DIR3_LEAF1_MAGIC) ||
	    lp->hdr.info.magic == cpu_to_be16(XFS_DIR3_LEAFN_MAGIC)) {
		struct xfs_dir3_leaf *lp3 = (struct xfs_dir3_leaf *)lp;

		return be16_to_cpu(lp3->hdr.count);
	}
	return be16_to_cpu(lp->hdr.count);
}

static void
process_leaf_node_dir_v2_int(
	inodata_t		*id,
	int			v,
	xfs_dablk_t		dabno,
	freetab_t		*freetab)
{
	int			i;
	__be16			*lbp;
	xfs_dir2_leaf_t		*leaf;
	struct xfs_dir3_leaf	*leaf3 = NULL;
	xfs_dir2_leaf_entry_t	*lep;
	xfs_dir2_leaf_tail_t	*ltp;
	xfs_da_intnode_t	*node;
	int			stale;
	struct xfs_da3_icnode_hdr nodehdr;
	struct xfs_dir3_icleaf_hdr leafhdr;

	leaf = iocur_top->data;
	libxfs_dir2_leaf_hdr_from_disk(mp, &leafhdr, leaf);

	switch (be16_to_cpu(leaf->hdr.info.magic)) {
	case XFS_DIR3_LEAF1_MAGIC:
	case XFS_DIR3_LEAFN_MAGIC:
	case XFS_DA3_NODE_MAGIC:
		leaf3 = iocur_top->data;
		break;
	}
	switch (be16_to_cpu(leaf->hdr.info.magic)) {
	case XFS_DIR2_LEAF1_MAGIC:
	case XFS_DIR3_LEAF1_MAGIC:
		if (be32_to_cpu(leaf->hdr.info.forw) ||
					be32_to_cpu(leaf->hdr.info.back)) {
			if (!sflag || v)
				dbprintf(_("bad leaf block forw/back pointers "
					 "%d/%d for dir ino %lld block %d\n"),
					be32_to_cpu(leaf->hdr.info.forw),
					be32_to_cpu(leaf->hdr.info.back),
					id->ino, dabno);
			error++;
		}
		if (dabno != mp->m_dir_geo->leafblk) {
			if (!sflag || v)
				dbprintf(_("single leaf block for dir ino %lld "
					 "block %d should be at block %d\n"),
					id->ino, dabno,
					(xfs_dablk_t)mp->m_dir_geo->leafblk);
			error++;
		}
		ltp = xfs_dir2_leaf_tail_p(mp->m_dir_geo, leaf);
		lbp = xfs_dir2_leaf_bests_p(ltp);
		for (i = 0; i < be32_to_cpu(ltp->bestcount); i++) {
			if (freetab->nents <= i || freetab->ents[i] !=
						be16_to_cpu(lbp[i])) {
				if (!sflag || v)
					dbprintf(_("bestfree %d for dir ino %lld "
						 "block %d doesn't match table "
						 "value %d\n"),
						freetab->nents <= i ?
							NULLDATAOFF :
							freetab->ents[i],
						id->ino,
						xfs_dir2_db_to_da(mp->m_dir_geo, i),
						be16_to_cpu(lbp[i]));
			}
			if (freetab->nents > i)
				freetab->ents[i] = NULLDATAOFF;
		}
		break;
	case XFS_DIR2_LEAFN_MAGIC:
	case XFS_DIR3_LEAFN_MAGIC:
		/* if it's at the root location then we can check the
		 * pointers are null XXX */
		break;
	case XFS_DA_NODE_MAGIC:
	case XFS_DA3_NODE_MAGIC:
		node = iocur_top->data;
		libxfs_da3_node_hdr_from_disk(mp, &nodehdr, node);
		if (nodehdr.level < 1 || nodehdr.level > XFS_DA_NODE_MAXDEPTH) {
			if (!sflag || v)
				dbprintf(_("bad node block level %d for dir ino "
					 "%lld block %d\n"),
					nodehdr.level, id->ino,
					dabno);
			error++;
		}
		return;
	default:
		if (!sflag || v)
			dbprintf(_("bad directory data magic # %#x for dir ino "
				 "%lld block %d\n"),
				be16_to_cpu(leaf->hdr.info.magic), id->ino,
				dabno);
		error++;
		return;
	}
	lep = leafhdr.ents;
	for (i = stale = 0; i < xfs_dir3_leaf_ents_count(leaf); i++) {
		if (be32_to_cpu(lep[i].address) == XFS_DIR2_NULL_DATAPTR)
			stale++;
		else if (dir_hash_see(be32_to_cpu(lep[i].hashval),
						be32_to_cpu(lep[i].address))) {
			if (!sflag || v)
				dbprintf(_("dir %lld block %d extra leaf entry "
					 "%x %x\n"), id->ino, dabno,
					be32_to_cpu(lep[i].hashval),
					be32_to_cpu(lep[i].address));
			error++;
		}
	}
	if (leaf3 && stale != be16_to_cpu(leaf3->hdr.stale)) {
		if (!sflag || v)
			dbprintf(_("dir3 %lld block %d stale mismatch "
				 "%d/%d\n"),
				 id->ino, dabno, stale,
				 be16_to_cpu(leaf3->hdr.stale));
		error++;
	} else if (!leaf3 && stale != be16_to_cpu(leaf->hdr.stale)) {
		if (!sflag || v)
			dbprintf(_("dir %lld block %d stale mismatch "
				 "%d/%d\n"),
				 id->ino, dabno, stale,
				 be16_to_cpu(leaf->hdr.stale));
		error++;
	}
}

static void
process_quota(
	qtype_t			qtype,
	inodata_t		*id,
	blkmap_t		*blkmap)
{
	xfs_fsblock_t		bno;
	int			cb;
	struct xfs_dqblk	*dqb;
	xfs_dqid_t		dqid;
	uint8_t			exp_flags = 0;
	uint			i;
	uint			perblock;
	xfs_fileoff_t		qbno;
	char			*s = NULL;
	int			scicb;
	int			t = 0;

	switch (qtype) {
	case IS_USER_QUOTA:
		s = "user";
		exp_flags = XFS_DQTYPE_USER;
		break;
	case IS_PROJECT_QUOTA:
		s = "project";
		exp_flags = XFS_DQTYPE_PROJ;
		break;
	case IS_GROUP_QUOTA:
		s = "group";
		exp_flags = XFS_DQTYPE_GROUP;
		break;
	default:
		ASSERT(0);
	}

	perblock = (uint)(mp->m_sb.sb_blocksize / sizeof(*dqb));
	dqid = 0;
	qbno = NULLFILEOFF;
	while ((qbno = blkmap_next_off(blkmap, qbno, &t)) != NULLFILEOFF) {
		bno = blkmap_get(blkmap, qbno);
		dqid = (xfs_dqid_t)qbno * perblock;
		cb = CHECK_BLIST(bno);
		scicb = !sflag || id->ilist || cb;
		push_cur();
		set_cur(&typtab[TYP_DQBLK], XFS_FSB_TO_DADDR(mp, bno), blkbb,
			DB_RING_IGN, NULL);
		if ((dqb = iocur_top->data) == NULL) {
			if (scicb)
				dbprintf(_("can't read block %lld for %s quota "
					 "inode (fsblock %lld)\n"),
					(xfs_fileoff_t)qbno, s,
					(xfs_fsblock_t)bno);
			error++;
			pop_cur();
			continue;
		}
		for (i = 0; i < perblock; i++, dqid++, dqb++) {
			if (verbose || id->ilist || cb)
				dbprintf(_("%s dqblk %lld entry %d id %u bc "
					 "%lld ic %lld rc %lld\n"),
					s, (xfs_fileoff_t)qbno, i, dqid,
					be64_to_cpu(dqb->dd_diskdq.d_bcount),
					be64_to_cpu(dqb->dd_diskdq.d_icount),
					be64_to_cpu(dqb->dd_diskdq.d_rtbcount));
			if (be16_to_cpu(dqb->dd_diskdq.d_magic) != XFS_DQUOT_MAGIC) {
				if (scicb)
					dbprintf(_("bad magic number %#x for %s "
						 "dqblk %lld entry %d id %u\n"),
						be16_to_cpu(dqb->dd_diskdq.d_magic), s,
						(xfs_fileoff_t)qbno, i, dqid);
				error++;
				continue;
			}
			if (dqb->dd_diskdq.d_version != XFS_DQUOT_VERSION) {
				if (scicb)
					dbprintf(_("bad version number %#x for "
						 "%s dqblk %lld entry %d id "
						 "%u\n"),
						dqb->dd_diskdq.d_version, s,
						(xfs_fileoff_t)qbno, i, dqid);
				error++;
				continue;
			}
			if (dqb->dd_diskdq.d_type & ~XFS_DQTYPE_ANY) {
				if (scicb)
					dbprintf(_("bad flags %#x for %s dqblk "
						 "%lld entry %d id %u\n"),
						dqb->dd_diskdq.d_type, s,
						(xfs_fileoff_t)qbno, i, dqid);
				error++;
				continue;
			}
			if ((dqb->dd_diskdq.d_type & XFS_DQTYPE_REC_MASK)
								!= exp_flags) {
				if (scicb)
					dbprintf(_("wrong type %#x for %s dqblk "
						 "%lld entry %d id %u\n"),
						dqb->dd_diskdq.d_type &
							XFS_DQTYPE_REC_MASK, s,
						(xfs_fileoff_t)qbno, i, dqid);
				error++;
				continue;
			}
			if (be32_to_cpu(dqb->dd_diskdq.d_id) != dqid) {
				if (scicb)
					dbprintf(_("bad id %u for %s dqblk %lld "
						 "entry %d id %u\n"),
						be32_to_cpu(dqb->dd_diskdq.d_id), s,
						(xfs_fileoff_t)qbno, i, dqid);
				error++;
				continue;
			}
			quota_add((qtype == IS_PROJECT_QUOTA) ? &dqid : NULL,
				  (qtype == IS_GROUP_QUOTA) ? &dqid : NULL,
				  (qtype == IS_USER_QUOTA) ? &dqid : NULL,
				  1,
				  be64_to_cpu(dqb->dd_diskdq.d_bcount),
				  be64_to_cpu(dqb->dd_diskdq.d_icount),
				  be64_to_cpu(dqb->dd_diskdq.d_rtbcount));
		}
		pop_cur();
	}
}

static void
process_rtbitmap(
	blkmap_t	*blkmap)
{
	int		bit;
	int		bitsperblock;
	xfs_fileoff_t	bmbno;
	xfs_fsblock_t	bno;
	xfs_rtblock_t	extno;
	int		len;
	int		log;
	int		offs;
	int		prevbit;
	xfs_rfsblock_t	rtbno;
	int		start_bmbno;
	int		start_bit;
	int		t;
	xfs_rtword_t	*words;

	bitsperblock = mp->m_sb.sb_blocksize * NBBY;
	bit = extno = prevbit = start_bmbno = start_bit = 0;
	bmbno = NULLFILEOFF;
	while ((bmbno = blkmap_next_off(blkmap, bmbno, &t)) !=
	       NULLFILEOFF) {
		bno = blkmap_get(blkmap, bmbno);
		if (bno == NULLFSBLOCK) {
			if (!sflag)
				dbprintf(_("block %lld for rtbitmap inode is "
					 "missing\n"),
					(xfs_fileoff_t)bmbno);
			error++;
			continue;
		}
		push_cur();
		set_cur(&typtab[TYP_RTBITMAP], XFS_FSB_TO_DADDR(mp, bno), blkbb,
			DB_RING_IGN, NULL);
		if ((words = iocur_top->data) == NULL) {
			if (!sflag)
				dbprintf(_("can't read block %lld for rtbitmap "
					 "inode\n"),
					(xfs_fileoff_t)bmbno);
			error++;
			pop_cur();
			continue;
		}
		for (bit = 0;
		     bit < bitsperblock && extno < mp->m_sb.sb_rextents;
		     bit++, extno++) {
			if (xfs_isset(words, bit)) {
				rtbno = extno * mp->m_sb.sb_rextsize;
				set_rdbmap(rtbno, mp->m_sb.sb_rextsize,
					DBM_RTFREE);
				frextents++;
				if (prevbit == 0) {
					start_bmbno = (int)bmbno;
					start_bit = bit;
					prevbit = 1;
				}
			} else if (prevbit == 1) {
				len = ((int)bmbno - start_bmbno) *
					bitsperblock + (bit - start_bit);
				log = XFS_RTBLOCKLOG(len);
				offs = XFS_SUMOFFS(mp, log, start_bmbno);
				sumcompute[offs]++;
				prevbit = 0;
			}
		}
		pop_cur();
		if (extno == mp->m_sb.sb_rextents)
			break;
	}
	if (prevbit == 1) {
		len = ((int)bmbno - start_bmbno) * bitsperblock +
			(bit - start_bit);
		log = XFS_RTBLOCKLOG(len);
		offs = XFS_SUMOFFS(mp, log, start_bmbno);
		sumcompute[offs]++;
	}
}

static void
process_rtsummary(
	blkmap_t	*blkmap)
{
	xfs_fsblock_t	bno;
	char		*bytes;
	xfs_fileoff_t	sumbno;
	int		t;

	sumbno = NULLFILEOFF;
	while ((sumbno = blkmap_next_off(blkmap, sumbno, &t)) != NULLFILEOFF) {
		bno = blkmap_get(blkmap, sumbno);
		if (bno == NULLFSBLOCK) {
			if (!sflag)
				dbprintf(_("block %lld for rtsummary inode is "
					 "missing\n"),
					(xfs_fileoff_t)sumbno);
			error++;
			continue;
		}
		push_cur();
		set_cur(&typtab[TYP_RTSUMMARY], XFS_FSB_TO_DADDR(mp, bno),
			blkbb, DB_RING_IGN, NULL);
		if ((bytes = iocur_top->data) == NULL) {
			if (!sflag)
				dbprintf(_("can't read block %lld for rtsummary "
					 "inode\n"),
					(xfs_fileoff_t)sumbno);
			error++;
			pop_cur();
			continue;
		}
		memcpy((char *)sumfile + sumbno * mp->m_sb.sb_blocksize, bytes,
			mp->m_sb.sb_blocksize);
		pop_cur();
	}
}

static xfs_ino_t
process_sf_dir_v2(
	struct xfs_dinode	*dip,
	int			*dot,
	int			*dotdot,
	inodata_t		*id)
{
	inodata_t		*cid;
	int			i;
	int			i8;
	xfs_ino_t		lino;
	int			offset;
	struct xfs_dir2_sf_hdr	*sf;
	xfs_dir2_sf_entry_t	*sfe;
	int			v;

	sf = (struct xfs_dir2_sf_hdr *)XFS_DFORK_DPTR(dip);
	addlink_inode(id);
	v = verbose || id->ilist;
	if (v)
		dbprintf(_("dir %lld entry . %lld\n"), id->ino, id->ino);
	(*dot)++;
	sfe = xfs_dir2_sf_firstentry(sf);
	offset = mp->m_dir_geo->data_first_offset;
	for (i = sf->count - 1, i8 = 0; i >= 0; i--) {
		if ((intptr_t)sfe +
		    libxfs_dir2_sf_entsize(mp, sf, sfe->namelen) -
		    (intptr_t)sf > be64_to_cpu(dip->di_size)) {
			if (!sflag)
				dbprintf(_("dir %llu bad size in entry at %d\n"),
					id->ino,
					(int)((char *)sfe - (char *)sf));
			error++;
			break;
		}
		lino = libxfs_dir2_sf_get_ino(mp, sf, sfe);
		if (lino > XFS_DIR2_MAX_SHORT_INUM)
			i8++;
		cid = find_inode(lino, 1);
		if (cid == NULL) {
			if (!sflag)
				dbprintf(_("dir %lld entry %*.*s bad inode "
					 "number %lld\n"),
					id->ino, sfe->namelen, sfe->namelen,
					sfe->name, lino);
			error++;
		} else {
			addlink_inode(cid);
			if (!cid->parent)
				cid->parent = id;
			addname_inode(cid, (char *)sfe->name, sfe->namelen);
		}
		if (v)
			dbprintf(_("dir %lld entry %*.*s offset %d %lld\n"),
				id->ino, sfe->namelen, sfe->namelen, sfe->name,
				xfs_dir2_sf_get_offset(sfe), lino);
		if (xfs_dir2_sf_get_offset(sfe) < offset) {
			if (!sflag)
				dbprintf(_("dir %lld entry %*.*s bad offset %d\n"),
					id->ino, sfe->namelen, sfe->namelen,
					sfe->name, xfs_dir2_sf_get_offset(sfe));
			error++;
		}
		offset =
			xfs_dir2_sf_get_offset(sfe) +
			libxfs_dir2_sf_entsize(mp, sf, sfe->namelen);
		sfe = libxfs_dir2_sf_nextentry(mp, sf, sfe);
	}
	if (i < 0 && (intptr_t)sfe - (intptr_t)sf !=
					be64_to_cpu(dip->di_size)) {
		if (!sflag)
			dbprintf(_("dir %llu size is %lld, should be %u\n"),
				id->ino, be64_to_cpu(dip->di_size),
				(uint)((char *)sfe - (char *)sf));
		error++;
	}
	if (offset + (sf->count + 2) * sizeof(xfs_dir2_leaf_entry_t) +
	    sizeof(xfs_dir2_block_tail_t) > mp->m_dir_geo->blksize) {
		if (!sflag)
			dbprintf(_("dir %llu offsets too high\n"), id->ino);
		error++;
	}
	lino = libxfs_dir2_sf_get_parent_ino(sf);
	if (lino > XFS_DIR2_MAX_SHORT_INUM)
		i8++;
	cid = find_inode(lino, 1);
	if (cid)
		addlink_inode(cid);
	else {
		if (!sflag)
			dbprintf(_("dir %lld entry .. bad inode number %lld\n"),
				id->ino, lino);
		error++;
	}
	if (v)
		dbprintf(_("dir %lld entry .. %lld\n"), id->ino, lino);
	if (i8 != sf->i8count) {
		if (!sflag)
			dbprintf(_("dir %lld i8count mismatch is %d should be "
				 "%d\n"),
				id->ino, sf->i8count, i8);
		error++;
	}
	(*dotdot)++;
	return cid ? lino : NULLFSINO;
}


static void
quota_add(
	xfs_dqid_t	*prjid,
	xfs_dqid_t	*grpid,
	xfs_dqid_t	*usrid,
	int		dq,
	xfs_qcnt_t	bc,
	xfs_qcnt_t	ic,
	xfs_qcnt_t	rc)
{
	if (qudo && usrid != NULL)
		quota_add1(qudata, *usrid, dq, bc, ic, rc);
	if (qgdo && grpid != NULL)
		quota_add1(qgdata, *grpid, dq, bc, ic, rc);
	if (qpdo && prjid != NULL)
		quota_add1(qpdata, *prjid, dq, bc, ic, rc);
}

static void
quota_add1(
	qdata_t		**qt,
	xfs_dqid_t	id,
	int		dq,
	xfs_qcnt_t	bc,
	xfs_qcnt_t	ic,
	xfs_qcnt_t	rc)
{
	qdata_t		*qe;
	int		qh;
	qinfo_t		*qi;

	qh = (int)(id % QDATA_HASH_SIZE);
	qe = qt[qh];
	while (qe) {
		if (qe->id == id) {
			qi = dq ? &qe->dq : &qe->count;
			qi->bc += bc;
			qi->ic += ic;
			qi->rc += rc;
			return;
		}
		qe = qe->next;
	}
	qe = xmalloc(sizeof(*qe));
	qe->id = id;
	qi = dq ? &qe->dq : &qe->count;
	qi->bc = bc;
	qi->ic = ic;
	qi->rc = rc;
	qi = dq ? &qe->count : &qe->dq;
	qi->bc = qi->ic = qi->rc = 0;
	qe->next = qt[qh];
	qt[qh] = qe;
}

static void
quota_check(
	char	*s,
	qdata_t	**qt)
{
	int	i;
	qdata_t	*next;
	qdata_t	*qp;

	for (i = 0; i < QDATA_HASH_SIZE; i++) {
		qp = qt[i];
		while (qp) {
			next = qp->next;
			if (qp->count.bc != qp->dq.bc ||
			    qp->count.ic != qp->dq.ic ||
			    qp->count.rc != qp->dq.rc) {
				if (!sflag) {
					dbprintf(_("%s quota id %u, have/exp"),
						s, qp->id);
					if (qp->count.bc != qp->dq.bc)
						dbprintf(_(" bc %lld/%lld"),
							qp->dq.bc,
							qp->count.bc);
					if (qp->count.ic != qp->dq.ic)
						dbprintf(_(" ic %lld/%lld"),
							qp->dq.ic,
							qp->count.ic);
					if (qp->count.rc != qp->dq.rc)
						dbprintf(_(" rc %lld/%lld"),
							qp->dq.rc,
							qp->count.rc);
					dbprintf("\n");
				}
				error++;
			}
			xfree(qp);
			qp = next;
		}
	}
	xfree(qt);
}

static void
quota_init(void)
{
	qudo = mp->m_sb.sb_uquotino != 0 &&
	       mp->m_sb.sb_uquotino != NULLFSINO &&
	       (mp->m_sb.sb_qflags & XFS_UQUOTA_ACCT) &&
	       (mp->m_sb.sb_qflags & XFS_UQUOTA_CHKD);
	qgdo = mp->m_sb.sb_gquotino != 0 &&
	       mp->m_sb.sb_gquotino != NULLFSINO &&
	       (mp->m_sb.sb_qflags & XFS_GQUOTA_ACCT) &&
	       (mp->m_sb.sb_qflags & XFS_GQUOTA_CHKD);
	qpdo = mp->m_sb.sb_pquotino != 0 &&
	       mp->m_sb.sb_pquotino != NULLFSINO &&
	       (mp->m_sb.sb_qflags & XFS_PQUOTA_ACCT) &&
	       (mp->m_sb.sb_qflags & XFS_PQUOTA_CHKD);
	if (qudo)
		qudata = xcalloc(QDATA_HASH_SIZE, sizeof(qdata_t *));
	if (qgdo)
		qgdata = xcalloc(QDATA_HASH_SIZE, sizeof(qdata_t *));
	if (qpdo)
		qpdata = xcalloc(QDATA_HASH_SIZE, sizeof(qdata_t *));
}

static void
scan_ag(
	xfs_agnumber_t	agno)
{
	xfs_agf_t	*agf;
	xfs_agi_t	*agi;
	int		i;
	xfs_sb_t	tsb;
	xfs_sb_t	*sb = &tsb;

	agffreeblks = agflongest = 0;
	agfbtreeblks = -2;
	agicount = agifreecount = 0;
	push_cur();	/* 1 pushed */
	set_cur(&typtab[TYP_SB],
		XFS_AG_DADDR(mp, agno, XFS_SB_DADDR),
		XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);

	if (!iocur_top->data) {
		dbprintf(_("can't read superblock for ag %u\n"), agno);
		serious_error++;
		goto pop1_out;
	}

	libxfs_sb_from_disk(sb, iocur_top->data);

	if (sb->sb_magicnum != XFS_SB_MAGIC) {
		if (!sflag)
			dbprintf(_("bad sb magic # %#x in ag %u\n"),
				sb->sb_magicnum, agno);
		error++;
	}
	if (!xfs_sb_good_version(sb)) {
		if (!sflag)
			dbprintf(_("bad sb version # %#x in ag %u\n"),
				sb->sb_versionnum, agno);
		error++;
		sbver_err++;
	}
	if (!lazycount && xfs_sb_version_haslazysbcount(sb)) {
		lazycount = 1;
	}
	if (agno == 0 && sb->sb_inprogress != 0) {
		if (!sflag)
			dbprintf(_("mkfs not completed successfully\n"));
		error++;
	}
	if (xfs_sb_version_needsrepair(sb)) {
		if (!sflag)
			dbprintf(_("filesystem needs xfs_repair\n"));
		error++;
	}
	set_dbmap(agno, XFS_SB_BLOCK(mp), 1, DBM_SB, agno, XFS_SB_BLOCK(mp));
	if (sb->sb_logstart && XFS_FSB_TO_AGNO(mp, sb->sb_logstart) == agno)
		set_dbmap(agno, XFS_FSB_TO_AGBNO(mp, sb->sb_logstart),
			sb->sb_logblocks, DBM_LOG, agno, XFS_SB_BLOCK(mp));
	push_cur();	/* 2 pushed */
	set_cur(&typtab[TYP_AGF],
		XFS_AG_DADDR(mp, agno, XFS_AGF_DADDR(mp)),
		XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);
	if ((agf = iocur_top->data) == NULL) {
		dbprintf(_("can't read agf block for ag %u\n"), agno);
		serious_error++;
		goto pop2_out;
	}
	if (be32_to_cpu(agf->agf_magicnum) != XFS_AGF_MAGIC) {
		if (!sflag)
			dbprintf(_("bad agf magic # %#x in ag %u\n"),
				be32_to_cpu(agf->agf_magicnum), agno);
		error++;
	}
	if (!XFS_AGF_GOOD_VERSION(be32_to_cpu(agf->agf_versionnum))) {
		if (!sflag)
			dbprintf(_("bad agf version # %#x in ag %u\n"),
				be32_to_cpu(agf->agf_versionnum), agno);
		error++;
	}
	if (XFS_SB_BLOCK(mp) != XFS_AGF_BLOCK(mp))
		set_dbmap(agno, XFS_AGF_BLOCK(mp), 1, DBM_AGF, agno,
			XFS_SB_BLOCK(mp));
	if (sb->sb_agblocks > be32_to_cpu(agf->agf_length))
		set_dbmap(agno, be32_to_cpu(agf->agf_length),
			sb->sb_agblocks - be32_to_cpu(agf->agf_length),
			DBM_MISSING, agno, XFS_SB_BLOCK(mp));
	push_cur();	/* 3 pushed */
	set_cur(&typtab[TYP_AGI],
		XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)),
		XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);
	if ((agi = iocur_top->data) == NULL) {
		dbprintf(_("can't read agi block for ag %u\n"), agno);
		serious_error++;
		goto pop3_out;
	}
	if (be32_to_cpu(agi->agi_magicnum) != XFS_AGI_MAGIC) {
		if (!sflag)
			dbprintf(_("bad agi magic # %#x in ag %u\n"),
				be32_to_cpu(agi->agi_magicnum), agno);
		error++;
	}
	if (!XFS_AGI_GOOD_VERSION(be32_to_cpu(agi->agi_versionnum))) {
		if (!sflag)
			dbprintf(_("bad agi version # %#x in ag %u\n"),
				be32_to_cpu(agi->agi_versionnum), agno);
		error++;
	}
	if (XFS_SB_BLOCK(mp) != XFS_AGI_BLOCK(mp) &&
	    XFS_AGF_BLOCK(mp) != XFS_AGI_BLOCK(mp))
		set_dbmap(agno, XFS_AGI_BLOCK(mp), 1, DBM_AGI, agno,
			XFS_SB_BLOCK(mp));
	scan_freelist(agf);
	fdblocks--;
	scan_sbtree(agf,
		be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNO]),
		be32_to_cpu(agf->agf_levels[XFS_BTNUM_BNO]),
		1, scanfunc_bno, TYP_BNOBT);
	fdblocks--;
	scan_sbtree(agf,
		be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNT]),
		be32_to_cpu(agf->agf_levels[XFS_BTNUM_CNT]),
		1, scanfunc_cnt, TYP_CNTBT);
	if (agf->agf_roots[XFS_BTNUM_RMAP]) {
		scan_sbtree(agf,
			be32_to_cpu(agf->agf_roots[XFS_BTNUM_RMAP]),
			be32_to_cpu(agf->agf_levels[XFS_BTNUM_RMAP]),
			1, scanfunc_rmap, TYP_RMAPBT);
	}
	if (agf->agf_refcount_root) {
		scan_sbtree(agf,
			be32_to_cpu(agf->agf_refcount_root),
			be32_to_cpu(agf->agf_refcount_level),
			1, scanfunc_refcnt, TYP_REFCBT);
	}
	scan_sbtree(agf,
		be32_to_cpu(agi->agi_root),
		be32_to_cpu(agi->agi_level),
		1, scanfunc_ino, TYP_INOBT);
	if (agi->agi_free_root) {
		scan_sbtree(agf,
			be32_to_cpu(agi->agi_free_root),
			be32_to_cpu(agi->agi_free_level),
			1, scanfunc_fino, TYP_FINOBT);
	}
	if (be32_to_cpu(agf->agf_freeblks) != agffreeblks) {
		if (!sflag)
			dbprintf(_("agf_freeblks %u, counted %u in ag %u\n"),
				be32_to_cpu(agf->agf_freeblks),
				agffreeblks, agno);
		error++;
	}
	if (be32_to_cpu(agf->agf_longest) != agflongest) {
		if (!sflag)
			dbprintf(_("agf_longest %u, counted %u in ag %u\n"),
				be32_to_cpu(agf->agf_longest),
				agflongest, agno);
		error++;
	}
	if (lazycount &&
	    be32_to_cpu(agf->agf_btreeblks) != agfbtreeblks) {
		if (!sflag)
			dbprintf(_("agf_btreeblks %u, counted %u in ag %u\n"),
				be32_to_cpu(agf->agf_btreeblks),
				agfbtreeblks, agno);
		error++;
	}
	agf_aggr_freeblks += agffreeblks + agfbtreeblks;
	if (be32_to_cpu(agi->agi_count) != agicount) {
		if (!sflag)
			dbprintf(_("agi_count %u, counted %u in ag %u\n"),
				be32_to_cpu(agi->agi_count),
				agicount, agno);
		error++;
	}
	if (be32_to_cpu(agi->agi_freecount) != agifreecount) {
		if (!sflag)
			dbprintf(_("agi_freecount %u, counted %u in ag %u\n"),
				be32_to_cpu(agi->agi_freecount),
				agifreecount, agno);
		error++;
	}
	for (i = 0; i < XFS_AGI_UNLINKED_BUCKETS; i++) {
		if (be32_to_cpu(agi->agi_unlinked[i]) != NULLAGINO) {
			if (!sflag) {
				xfs_agino_t agino=be32_to_cpu(agi->agi_unlinked[i]);
				dbprintf(_("agi unlinked bucket %d is %u in ag "
					 "%u (inode=%lld)\n"), i, agino, agno,
					XFS_AGINO_TO_INO(mp, agno, agino));
			}
			error++;
		}
	}
pop3_out:
	pop_cur();
pop2_out:
	pop_cur();
pop1_out:
	pop_cur();
}

struct agfl_state {
	xfs_agnumber_t	agno;
	unsigned int	count;
};

static int
scan_agfl(
	struct xfs_mount	*mp,
	xfs_agblock_t		bno,
	void			*priv)
{
	struct agfl_state	*as = priv;

	set_dbmap(as->agno, bno, 1, DBM_FREELIST, as->agno, XFS_AGFL_BLOCK(mp));
	as->count++;
	return 0;
}

static void
scan_freelist(
	xfs_agf_t		*agf)
{
	xfs_agnumber_t		seqno = be32_to_cpu(agf->agf_seqno);
	struct agfl_state	state;

	if (XFS_SB_BLOCK(mp) != XFS_AGFL_BLOCK(mp) &&
	    XFS_AGF_BLOCK(mp) != XFS_AGFL_BLOCK(mp) &&
	    XFS_AGI_BLOCK(mp) != XFS_AGFL_BLOCK(mp))
		set_dbmap(seqno, XFS_AGFL_BLOCK(mp), 1, DBM_AGFL, seqno,
			XFS_SB_BLOCK(mp));
	if (be32_to_cpu(agf->agf_flcount) == 0)
		return;
	push_cur();
	set_cur(&typtab[TYP_AGFL],
		XFS_AG_DADDR(mp, seqno, XFS_AGFL_DADDR(mp)),
		XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);
	if (iocur_top->data == NULL) {
		dbprintf(_("can't read agfl block for ag %u\n"), seqno);
		serious_error++;
		pop_cur();
		return;
	}

	/* verify agf values before proceeding */
	if (be32_to_cpu(agf->agf_flfirst) >= libxfs_agfl_size(mp) ||
	    be32_to_cpu(agf->agf_fllast) >= libxfs_agfl_size(mp)) {
		dbprintf(_("agf %d freelist blocks bad, skipping "
			  "freelist scan\n"), seqno);
		pop_cur();
		return;
	}

	/* open coded xfs_buf_to_agfl_bno */
	state.count = 0;
	state.agno = seqno;
	libxfs_agfl_walk(mp, agf, iocur_top->bp, scan_agfl, &state);
	if (state.count != be32_to_cpu(agf->agf_flcount)) {
		if (!sflag)
			dbprintf(_("freeblk count %u != flcount %u in ag %u\n"),
					state.count,
					be32_to_cpu(agf->agf_flcount),
					seqno);
		error++;
	}
	fdblocks += state.count;
	agf_aggr_freeblks += state.count;
	pop_cur();
}

static void
scan_lbtree(
	xfs_fsblock_t	root,
	int		nlevels,
	scan_lbtree_f_t	func,
	dbm_t		type,
	inodata_t	*id,
	xfs_rfsblock_t	*totd,
	xfs_rfsblock_t	*toti,
	xfs_extnum_t	*nex,
	blkmap_t	**blkmapp,
	int		isroot,
	typnm_t		btype)
{
	push_cur();
	set_cur(&typtab[btype], XFS_FSB_TO_DADDR(mp, root), blkbb, DB_RING_IGN,
		NULL);
	if (iocur_top->data == NULL) {
		if (!sflag)
			dbprintf(_("can't read btree block %u/%u\n"),
				XFS_FSB_TO_AGNO(mp, root),
				XFS_FSB_TO_AGBNO(mp, root));
		error++;
		pop_cur();
		return;
	}
	(*func)(iocur_top->data, nlevels - 1, type, root, id, totd, toti, nex,
		blkmapp, isroot, btype);
	pop_cur();
}

static void
scan_sbtree(
	xfs_agf_t	*agf,
	xfs_agblock_t	root,
	int		nlevels,
	int		isroot,
	scan_sbtree_f_t	func,
	typnm_t		btype)
{
	xfs_agnumber_t	seqno = be32_to_cpu(agf->agf_seqno);

	push_cur();
	set_cur(&typtab[btype],
		XFS_AGB_TO_DADDR(mp, seqno, root), blkbb, DB_RING_IGN, NULL);
	if (iocur_top->data == NULL) {
		if (!sflag)
			dbprintf(_("can't read btree block %u/%u\n"), seqno, root);
		error++;
		pop_cur();
		return;
	}
	(*func)(iocur_top->data, nlevels - 1, agf, root, isroot);
	pop_cur();
}

static void
scanfunc_bmap(
	struct xfs_btree_block	*block,
	int			level,
	dbm_t			type,
	xfs_fsblock_t		bno,
	inodata_t		*id,
	xfs_rfsblock_t		*totd,
	xfs_rfsblock_t		*toti,
	xfs_extnum_t		*nex,
	blkmap_t		**blkmapp,
	int			isroot,
	typnm_t			btype)
{
	xfs_agblock_t		agbno;
	xfs_agnumber_t		agno;
	int			i;
	xfs_bmbt_ptr_t		*pp;
	xfs_bmbt_rec_t		*rp;

	agno = XFS_FSB_TO_AGNO(mp, bno);
	agbno = XFS_FSB_TO_AGBNO(mp, bno);
	if (be32_to_cpu(block->bb_magic) != XFS_BMAP_MAGIC &&
	    be32_to_cpu(block->bb_magic) != XFS_BMAP_CRC_MAGIC) {
		if (!sflag || id->ilist || CHECK_BLIST(bno))
			dbprintf(_("bad magic # %#x in inode %lld bmbt block "
				 "%u/%u\n"),
				be32_to_cpu(block->bb_magic), id->ino, agno, agbno);
		error++;
	}
	if (be16_to_cpu(block->bb_level) != level) {
		if (!sflag || id->ilist || CHECK_BLIST(bno))
			dbprintf(_("expected level %d got %d in inode %lld bmbt "
				 "block %u/%u\n"),
				level, be16_to_cpu(block->bb_level), id->ino, agno, agbno);
		error++;
	}
	set_dbmap(agno, agbno, 1, type, agno, agbno);
	set_inomap(agno, agbno, 1, id);
	(*toti)++;
	if (level == 0) {
		if (be16_to_cpu(block->bb_numrecs) > mp->m_bmap_dmxr[0] ||
		    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_bmap_dmnr[0])) {
			if (!sflag || id->ilist || CHECK_BLIST(bno))
				dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) "
					 "in inode %lld bmap block %lld\n"),
					be16_to_cpu(block->bb_numrecs), mp->m_bmap_dmnr[0],
					mp->m_bmap_dmxr[0], id->ino,
					(xfs_fsblock_t)bno);
			error++;
			return;
		}
		rp = XFS_BMBT_REC_ADDR(mp, block, 1);
		*nex += be16_to_cpu(block->bb_numrecs);
		process_bmbt_reclist(rp, be16_to_cpu(block->bb_numrecs), type, id, totd,
			blkmapp);
		return;
	}
	if (be16_to_cpu(block->bb_numrecs) > mp->m_bmap_dmxr[1] ||
	    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_bmap_dmnr[1])) {
		if (!sflag || id->ilist || CHECK_BLIST(bno))
			dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in "
				 "inode %lld bmap block %lld\n"),
				be16_to_cpu(block->bb_numrecs), mp->m_bmap_dmnr[1],
				mp->m_bmap_dmxr[1], id->ino, (xfs_fsblock_t)bno);
		error++;
		return;
	}
	pp = XFS_BMBT_PTR_ADDR(mp, block, 1, mp->m_bmap_dmxr[0]);
	for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++)
		scan_lbtree(be64_to_cpu(pp[i]), level, scanfunc_bmap, type, id,
					totd, toti, nex, blkmapp, 0, btype);
}

static void
scanfunc_bno(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agf_t		*agf,
	xfs_agblock_t		bno,
	int			isroot)
{
	int			i;
	xfs_alloc_ptr_t		*pp;
	xfs_alloc_rec_t		*rp;
	xfs_agnumber_t		seqno = be32_to_cpu(agf->agf_seqno);
	xfs_agblock_t		lastblock;

	if (be32_to_cpu(block->bb_magic) != XFS_ABTB_MAGIC &&
	    be32_to_cpu(block->bb_magic) != XFS_ABTB_CRC_MAGIC) {
		dbprintf(_("bad magic # %#x in btbno block %u/%u\n"),
			be32_to_cpu(block->bb_magic), seqno, bno);
		serious_error++;
		return;
	}
	fdblocks++;
	agfbtreeblks++;
	if (be16_to_cpu(block->bb_level) != level) {
		if (!sflag)
			dbprintf(_("expected level %d got %d in btbno block "
				 "%u/%u\n"),
				level, be16_to_cpu(block->bb_level), seqno, bno);
		error++;
	}
	set_dbmap(seqno, bno, 1, DBM_BTBNO, seqno, bno);
	if (level == 0) {
		if (be16_to_cpu(block->bb_numrecs) > mp->m_alloc_mxr[0] ||
		    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_alloc_mnr[0])) {
			dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in "
				 "btbno block %u/%u\n"),
				be16_to_cpu(block->bb_numrecs), mp->m_alloc_mnr[0],
				mp->m_alloc_mxr[0], seqno, bno);
			serious_error++;
			return;
		}
		rp = XFS_ALLOC_REC_ADDR(mp, block, 1);
		lastblock = 0;
		for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++) {
			set_dbmap(seqno, be32_to_cpu(rp[i].ar_startblock),
				be32_to_cpu(rp[i].ar_blockcount), DBM_FREE1,
				seqno, bno);
			if (be32_to_cpu(rp[i].ar_startblock) <= lastblock) {
				dbprintf(_(
		"out-of-order bno btree record %d (%u %u) block %u/%u\n"),
				i, be32_to_cpu(rp[i].ar_startblock),
				be32_to_cpu(rp[i].ar_blockcount),
				be32_to_cpu(agf->agf_seqno), bno);
				serious_error++;
			} else {
				lastblock = be32_to_cpu(rp[i].ar_startblock);
			}
		}
		return;
	}
	if (be16_to_cpu(block->bb_numrecs) > mp->m_alloc_mxr[1] ||
	    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_alloc_mnr[1])) {
		dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in btbno block "
			 "%u/%u\n"),
			be16_to_cpu(block->bb_numrecs), mp->m_alloc_mnr[1],
			mp->m_alloc_mxr[1], seqno, bno);
		serious_error++;
		return;
	}
	pp = XFS_ALLOC_PTR_ADDR(mp, block, 1, mp->m_alloc_mxr[1]);
	for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++)
		scan_sbtree(agf, be32_to_cpu(pp[i]), level, 0, scanfunc_bno, TYP_BNOBT);
}

static void
scanfunc_cnt(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agf_t		*agf,
	xfs_agblock_t		bno,
	int			isroot)
{
	xfs_agnumber_t		seqno = be32_to_cpu(agf->agf_seqno);
	int			i;
	xfs_alloc_ptr_t		*pp;
	xfs_alloc_rec_t		*rp;
	xfs_extlen_t		lastcount;

	if (be32_to_cpu(block->bb_magic) != XFS_ABTC_MAGIC &&
	    be32_to_cpu(block->bb_magic) != XFS_ABTC_CRC_MAGIC) {
		dbprintf(_("bad magic # %#x in btcnt block %u/%u\n"),
			be32_to_cpu(block->bb_magic), seqno, bno);
		serious_error++;
		return;
	}
	fdblocks++;
	agfbtreeblks++;
	if (be16_to_cpu(block->bb_level) != level) {
		if (!sflag)
			dbprintf(_("expected level %d got %d in btcnt block "
				 "%u/%u\n"),
				level, be16_to_cpu(block->bb_level), seqno, bno);
		error++;
	}
	set_dbmap(seqno, bno, 1, DBM_BTCNT, seqno, bno);
	if (level == 0) {
		if (be16_to_cpu(block->bb_numrecs) > mp->m_alloc_mxr[0] ||
		    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_alloc_mnr[0])) {
			dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in "
				 "btbno block %u/%u\n"),
				be16_to_cpu(block->bb_numrecs), mp->m_alloc_mnr[0],
				mp->m_alloc_mxr[0], seqno, bno);
			serious_error++;
			return;
		}
		rp = XFS_ALLOC_REC_ADDR(mp, block, 1);
		lastcount = 0;
		for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++) {
			check_set_dbmap(seqno, be32_to_cpu(rp[i].ar_startblock),
				be32_to_cpu(rp[i].ar_blockcount), DBM_FREE1, DBM_FREE2,
				seqno, bno);
			fdblocks += be32_to_cpu(rp[i].ar_blockcount);
			agffreeblks += be32_to_cpu(rp[i].ar_blockcount);
			if (be32_to_cpu(rp[i].ar_blockcount) > agflongest)
				agflongest = be32_to_cpu(rp[i].ar_blockcount);
			if (be32_to_cpu(rp[i].ar_blockcount) < lastcount) {
				dbprintf(_(
		"out-of-order cnt btree record %d (%u %u) block %u/%u\n"),
					 i, be32_to_cpu(rp[i].ar_startblock),
					 be32_to_cpu(rp[i].ar_blockcount),
					 be32_to_cpu(agf->agf_seqno), bno);
			} else {
				lastcount = be32_to_cpu(rp[i].ar_blockcount);
			}
		}
		return;
	}
	if (be16_to_cpu(block->bb_numrecs) > mp->m_alloc_mxr[1] ||
	    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_alloc_mnr[1])) {
		dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in btbno block "
			 "%u/%u\n"),
			be16_to_cpu(block->bb_numrecs), mp->m_alloc_mnr[1],
			mp->m_alloc_mxr[1], seqno, bno);
		serious_error++;
		return;
	}
	pp = XFS_ALLOC_PTR_ADDR(mp, block, 1, mp->m_alloc_mxr[1]);
	for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++)
		scan_sbtree(agf, be32_to_cpu(pp[i]), level, 0, scanfunc_cnt, TYP_CNTBT);
}

static void
scanfunc_ino(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agf_t		*agf,
	xfs_agblock_t		bno,
	int			isroot)
{
	xfs_agino_t		agino;
	xfs_agnumber_t		seqno = be32_to_cpu(agf->agf_seqno);
	int			i;
	int			isfree;
	int			j;
	int			freecount;
	int			nfree;
	int			off;
	xfs_inobt_ptr_t		*pp;
	xfs_inobt_rec_t		*rp;
	xfs_agblock_t		agbno;
	xfs_agblock_t		end_agbno;
	struct xfs_dinode	*dip;
	int			blks_per_buf;
	int			inodes_per_buf;
	int			ioff;
	struct xfs_ino_geometry	*igeo = M_IGEO(mp);

	if (xfs_has_sparseinodes(mp))
		blks_per_buf = igeo->blocks_per_cluster;
	else
		blks_per_buf = igeo->ialloc_blks;
	inodes_per_buf = min(XFS_FSB_TO_INO(mp, blks_per_buf),
			     XFS_INODES_PER_CHUNK);

	if (be32_to_cpu(block->bb_magic) != XFS_IBT_MAGIC &&
	    be32_to_cpu(block->bb_magic) != XFS_IBT_CRC_MAGIC) {
		dbprintf(_("bad magic # %#x in inobt block %u/%u\n"),
			be32_to_cpu(block->bb_magic), seqno, bno);
		serious_error++;
		return;
	}
	if (be16_to_cpu(block->bb_level) != level) {
		if (!sflag)
			dbprintf(_("expected level %d got %d in inobt block "
				 "%u/%u\n"),
				level, be16_to_cpu(block->bb_level), seqno, bno);
		error++;
	}
	set_dbmap(seqno, bno, 1, DBM_BTINO, seqno, bno);
	if (level == 0) {
		if (be16_to_cpu(block->bb_numrecs) > igeo->inobt_mxr[0] ||
		    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < igeo->inobt_mnr[0])) {
			dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in "
				 "inobt block %u/%u\n"),
				be16_to_cpu(block->bb_numrecs), igeo->inobt_mnr[0],
				igeo->inobt_mxr[0], seqno, bno);
			serious_error++;
			return;
		}
		rp = XFS_INOBT_REC_ADDR(mp, block, 1);
		for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++) {
			agino = be32_to_cpu(rp[i].ir_startino);
			agbno = XFS_AGINO_TO_AGBNO(mp, agino);
			off = XFS_AGINO_TO_OFFSET(mp, agino);
			end_agbno = agbno + igeo->ialloc_blks;
			if (off == 0) {
				if ((sbversion & XFS_SB_VERSION_ALIGNBIT) &&
				    mp->m_sb.sb_inoalignmt &&
				    (XFS_INO_TO_AGBNO(mp, agino) %
				     mp->m_sb.sb_inoalignmt))
					sbversion &= ~XFS_SB_VERSION_ALIGNBIT;
			}

			push_cur();

			ioff = 0;
			nfree = 0;
			while (agbno < end_agbno &&
			       ioff < XFS_INODES_PER_CHUNK) {
				if (xfs_inobt_is_sparse_disk(&rp[i], ioff))
					goto next_buf;

				if (off < XFS_INODES_PER_CHUNK)
					set_dbmap(seqno, agbno, blks_per_buf,
						  DBM_INODE, seqno, bno);

				icount += inodes_per_buf;
				agicount += inodes_per_buf;

				set_cur(&typtab[TYP_INODE],
					XFS_AGB_TO_DADDR(mp, seqno, agbno),
					XFS_FSB_TO_BB(mp, blks_per_buf),
					DB_RING_IGN, NULL);
				if (iocur_top->data == NULL) {
					if (!sflag)
						dbprintf(_("can't read inode block "
							   "%u/%u\n"), seqno,
							 agbno);
					error++;
					goto next_buf;
				}

				for (j = 0; j < inodes_per_buf; j++) {
					isfree = XFS_INOBT_IS_FREE_DISK(&rp[i], ioff + j);
					if (isfree)
						nfree++;
					dip = (struct xfs_dinode *)((char *)iocur_top->data +
						((off + j) << mp->m_sb.sb_inodelog));
					process_inode(agf, agino + ioff + j, dip, isfree);
				}

next_buf:
				agbno += blks_per_buf;
				ioff += inodes_per_buf;
			}

			if (xfs_has_sparseinodes(mp))
				freecount = rp[i].ir_u.sp.ir_freecount;
			else
				freecount = be32_to_cpu(rp[i].ir_u.f.ir_freecount);

			ifree += freecount;
			agifreecount += freecount;

			if (nfree != freecount) {
				if (!sflag)
					dbprintf(_("ir_freecount/free mismatch, "
						 "inode chunk %u/%u, freecount "
						 "%d nfree %d\n"),
						seqno, agino, freecount, nfree);
				error++;
			}
			pop_cur();
		}
		return;
	}
	if (be16_to_cpu(block->bb_numrecs) > igeo->inobt_mxr[1] ||
	    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < igeo->inobt_mnr[1])) {
		dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in inobt block "
			 "%u/%u\n"),
			be16_to_cpu(block->bb_numrecs), igeo->inobt_mnr[1],
			igeo->inobt_mxr[1], seqno, bno);
		serious_error++;
		return;
	}
	pp = XFS_INOBT_PTR_ADDR(mp, block, 1, igeo->inobt_mxr[1]);
	for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++)
		scan_sbtree(agf, be32_to_cpu(pp[i]), level, 0, scanfunc_ino, TYP_INOBT);
}

static void
scanfunc_fino(
	struct xfs_btree_block	*block,
	int			level,
	struct xfs_agf		*agf,
	xfs_agblock_t		bno,
	int			isroot)
{
	xfs_agino_t		agino;
	xfs_agnumber_t		seqno = be32_to_cpu(agf->agf_seqno);
	int			i;
	int			off;
	xfs_inobt_ptr_t		*pp;
	struct xfs_inobt_rec	*rp;
	xfs_agblock_t		agbno;
	xfs_agblock_t		end_agbno;
	int			blks_per_buf;
	int			inodes_per_buf;
	int			ioff;
	struct xfs_ino_geometry	*igeo = M_IGEO(mp);

	if (xfs_has_sparseinodes(mp))
		blks_per_buf = igeo->blocks_per_cluster;
	else
		blks_per_buf = igeo->ialloc_blks;
	inodes_per_buf = min(XFS_FSB_TO_INO(mp, blks_per_buf),
			     XFS_INODES_PER_CHUNK);

	if (be32_to_cpu(block->bb_magic) != XFS_FIBT_MAGIC &&
	    be32_to_cpu(block->bb_magic) != XFS_FIBT_CRC_MAGIC) {
		dbprintf(_("bad magic # %#x in finobt block %u/%u\n"),
			be32_to_cpu(block->bb_magic), seqno, bno);
		serious_error++;
		return;
	}
	if (be16_to_cpu(block->bb_level) != level) {
		if (!sflag)
			dbprintf(_("expected level %d got %d in finobt block "
				 "%u/%u\n"),
				level, be16_to_cpu(block->bb_level), seqno, bno);
		error++;
	}
	set_dbmap(seqno, bno, 1, DBM_BTFINO, seqno, bno);
	if (level == 0) {
		if (be16_to_cpu(block->bb_numrecs) > igeo->inobt_mxr[0] ||
		    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < igeo->inobt_mnr[0])) {
			dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in "
				 "finobt block %u/%u\n"),
				be16_to_cpu(block->bb_numrecs), igeo->inobt_mnr[0],
				igeo->inobt_mxr[0], seqno, bno);
			serious_error++;
			return;
		}
		rp = XFS_INOBT_REC_ADDR(mp, block, 1);
		for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++) {
			agino = be32_to_cpu(rp[i].ir_startino);
			agbno = XFS_AGINO_TO_AGBNO(mp, agino);
			off = XFS_AGINO_TO_OFFSET(mp, agino);
			end_agbno = agbno + igeo->ialloc_blks;
			if (off == 0) {
				if ((sbversion & XFS_SB_VERSION_ALIGNBIT) &&
				    mp->m_sb.sb_inoalignmt &&
				    (XFS_INO_TO_AGBNO(mp, agino) %
				     mp->m_sb.sb_inoalignmt))
					sbversion &= ~XFS_SB_VERSION_ALIGNBIT;
			}

			ioff = 0;
			while (agbno < end_agbno &&
			       ioff < XFS_INODES_PER_CHUNK) {
				if (xfs_inobt_is_sparse_disk(&rp[i], ioff))
					goto next_buf;

                                check_set_dbmap(seqno, agbno,
                                        (xfs_extlen_t)max(1,
                                                inodes_per_buf >>
                                                mp->m_sb.sb_inopblog),
                                        DBM_INODE, DBM_INODE, seqno, bno);

next_buf:
				agbno += blks_per_buf;
				ioff += inodes_per_buf;
			}

		}
		return;
	}
	if (be16_to_cpu(block->bb_numrecs) > igeo->inobt_mxr[1] ||
	    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < igeo->inobt_mnr[1])) {
		dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in finobt block "
			 "%u/%u\n"),
			be16_to_cpu(block->bb_numrecs), igeo->inobt_mnr[1],
			igeo->inobt_mxr[1], seqno, bno);
		serious_error++;
		return;
	}
	pp = XFS_INOBT_PTR_ADDR(mp, block, 1, igeo->inobt_mxr[1]);
	for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++)
		scan_sbtree(agf, be32_to_cpu(pp[i]), level, 0, scanfunc_fino, TYP_FINOBT);
}

static void
scanfunc_rmap(
	struct xfs_btree_block	*block,
	int			level,
	struct xfs_agf		*agf,
	xfs_agblock_t		bno,
	int			isroot)
{
	xfs_agnumber_t		seqno = be32_to_cpu(agf->agf_seqno);
	int			i;
	xfs_rmap_ptr_t		*pp;
	struct xfs_rmap_rec	*rp;
	xfs_agblock_t		lastblock;

	if (be32_to_cpu(block->bb_magic) != XFS_RMAP_CRC_MAGIC) {
		dbprintf(_("bad magic # %#x in rmapbt block %u/%u\n"),
			be32_to_cpu(block->bb_magic), seqno, bno);
		serious_error++;
		return;
	}
	if (be16_to_cpu(block->bb_level) != level) {
		if (!sflag)
			dbprintf(_("expected level %d got %d in rmapbt block "
				 "%u/%u\n"),
				level, be16_to_cpu(block->bb_level), seqno, bno);
		error++;
	}
	if (!isroot) {
		fdblocks++;
		agfbtreeblks++;
	}
	set_dbmap(seqno, bno, 1, DBM_BTRMAP, seqno, bno);
	if (level == 0) {
		if (be16_to_cpu(block->bb_numrecs) > mp->m_rmap_mxr[0] ||
		    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_rmap_mnr[0])) {
			dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in "
				 "rmapbt block %u/%u\n"),
				be16_to_cpu(block->bb_numrecs), mp->m_rmap_mnr[0],
				mp->m_rmap_mxr[0], seqno, bno);
			serious_error++;
			return;
		}
		rp = XFS_RMAP_REC_ADDR(block, 1);
		lastblock = 0;
		for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++) {
			if (be32_to_cpu(rp[i].rm_startblock) < lastblock) {
				dbprintf(_(
		"out-of-order rmap btree record %d (%u %u) block %u/%u\n"),
					 i, be32_to_cpu(rp[i].rm_startblock),
					 be32_to_cpu(rp[i].rm_startblock),
					 be32_to_cpu(agf->agf_seqno), bno);
			} else {
				lastblock = be32_to_cpu(rp[i].rm_startblock);
			}
		}
		return;
	}
	if (be16_to_cpu(block->bb_numrecs) > mp->m_rmap_mxr[1] ||
	    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_rmap_mnr[1])) {
		dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in rmapbt "
			 "block %u/%u\n"),
			be16_to_cpu(block->bb_numrecs), mp->m_rmap_mnr[1],
			mp->m_rmap_mxr[1], seqno, bno);
		serious_error++;
		return;
	}
	pp = XFS_RMAP_PTR_ADDR(block, 1, mp->m_rmap_mxr[1]);
	for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++)
		scan_sbtree(agf, be32_to_cpu(pp[i]), level, 0, scanfunc_rmap,
				TYP_RMAPBT);
}

static void
scanfunc_refcnt(
	struct xfs_btree_block	*block,
	int			level,
	struct xfs_agf		*agf,
	xfs_agblock_t		bno,
	int			isroot)
{
	xfs_agnumber_t		seqno = be32_to_cpu(agf->agf_seqno);
	int			i;
	xfs_refcount_ptr_t	*pp;
	struct xfs_refcount_rec	*rp;
	xfs_agblock_t		lastblock;

	if (be32_to_cpu(block->bb_magic) != XFS_REFC_CRC_MAGIC) {
		dbprintf(_("bad magic # %#x in refcntbt block %u/%u\n"),
			be32_to_cpu(block->bb_magic), seqno, bno);
		serious_error++;
		return;
	}
	if (be16_to_cpu(block->bb_level) != level) {
		if (!sflag)
			dbprintf(_("expected level %d got %d in refcntbt block "
				 "%u/%u\n"),
				level, be16_to_cpu(block->bb_level), seqno, bno);
		error++;
	}
	set_dbmap(seqno, bno, 1, DBM_BTREFC, seqno, bno);
	if (level == 0) {
		if (be16_to_cpu(block->bb_numrecs) > mp->m_refc_mxr[0] ||
		    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_refc_mnr[0])) {
			dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in "
				 "refcntbt block %u/%u\n"),
				be16_to_cpu(block->bb_numrecs), mp->m_refc_mnr[0],
				mp->m_refc_mxr[0], seqno, bno);
			serious_error++;
			return;
		}
		rp = XFS_REFCOUNT_REC_ADDR(block, 1);
		lastblock = 0;
		for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++) {
			if (be32_to_cpu(rp[i].rc_refcount) == 1) {
				xfs_agblock_t	agbno;
				char		*msg;

				agbno = be32_to_cpu(rp[i].rc_startblock);
				if (agbno & XFS_REFC_COWFLAG) {
					agbno &= ~XFS_REFC_COWFLAG;
					msg = _(
		"leftover CoW extent (%u/%u) len %u\n");
				} else {
					msg = _(
		"leftover CoW extent at unexpected address (%u/%u) len %u\n");
				}
				dbprintf(msg,
					seqno,
					agbno,
					be32_to_cpu(rp[i].rc_blockcount));
				set_dbmap(seqno,
					agbno,
					be32_to_cpu(rp[i].rc_blockcount),
					DBM_COWDATA, seqno, bno);
			} else {
				set_dbmap(seqno,
					be32_to_cpu(rp[i].rc_startblock),
					be32_to_cpu(rp[i].rc_blockcount),
					DBM_RLDATA, seqno, bno);
			}
			if (be32_to_cpu(rp[i].rc_startblock) < lastblock) {
				dbprintf(_(
		"out-of-order refcnt btree record %d (%u %u) block %u/%u\n"),
					 i, be32_to_cpu(rp[i].rc_startblock),
					 be32_to_cpu(rp[i].rc_startblock),
					 be32_to_cpu(agf->agf_seqno), bno);
			} else {
				lastblock = be32_to_cpu(rp[i].rc_startblock) +
					    be32_to_cpu(rp[i].rc_blockcount);
			}
		}
		return;
	}
	if (be16_to_cpu(block->bb_numrecs) > mp->m_refc_mxr[1] ||
	    (isroot == 0 && be16_to_cpu(block->bb_numrecs) < mp->m_refc_mnr[1])) {
		dbprintf(_("bad btree nrecs (%u, min=%u, max=%u) in refcntbt "
			 "block %u/%u\n"),
			be16_to_cpu(block->bb_numrecs), mp->m_refc_mnr[1],
			mp->m_refc_mxr[1], seqno, bno);
		serious_error++;
		return;
	}
	pp = XFS_REFCOUNT_PTR_ADDR(block, 1, mp->m_refc_mxr[1]);
	for (i = 0; i < be16_to_cpu(block->bb_numrecs); i++)
		scan_sbtree(agf, be32_to_cpu(pp[i]), level, 0, scanfunc_refcnt,
				TYP_REFCBT);
}

static void
set_dbmap(
	xfs_agnumber_t	agno,
	xfs_agblock_t	agbno,
	xfs_extlen_t	len,
	dbm_t		type,
	xfs_agnumber_t	c_agno,
	xfs_agblock_t	c_agbno)
{
	check_set_dbmap(agno, agbno, len, DBM_UNKNOWN, type, c_agno, c_agbno);
}

static void
set_inomap(
	xfs_agnumber_t	agno,
	xfs_agblock_t	agbno,
	xfs_extlen_t	len,
	inodata_t	*id)
{
	xfs_extlen_t	i;
	inodata_t	**idp;
	int		mayprint;

	if (!check_inomap(agno, agbno, len, id->ino))
		return;
	mayprint = verbose | id->ilist | blist_size;
	for (i = 0, idp = &inomap[agno][agbno]; i < len; i++, idp++) {
		*idp = id;
		if (mayprint &&
		    (verbose || id->ilist || CHECK_BLISTA(agno, agbno + i)))
			dbprintf(_("setting inode to %lld for block %u/%u\n"),
				id->ino, agno, agbno + i);
	}
}

static void
set_rdbmap(
	xfs_rfsblock_t	bno,
	xfs_extlen_t	len,
	dbm_t		type)
{
	check_set_rdbmap(bno, len, DBM_UNKNOWN, type);
}

static void
set_rinomap(
	xfs_rfsblock_t	bno,
	xfs_extlen_t	len,
	inodata_t	*id)
{
	xfs_extlen_t	i;
	inodata_t	**idp;
	int		mayprint;

	if (!check_rinomap(bno, len, id->ino))
		return;
	mayprint = verbose | id->ilist | blist_size;
	for (i = 0, idp = &inomap[mp->m_sb.sb_agcount][bno];
	     i < len;
	     i++, idp++) {
		*idp = id;
		if (mayprint && (verbose || id->ilist || CHECK_BLIST(bno + i)))
			dbprintf(_("setting inode to %lld for rtblock %llu\n"),
				id->ino, bno + i);
	}
}

static void
setlink_inode(
	inodata_t	*id,
	nlink_t		nlink,
	int		isdir,
	int		security)
{
	id->link_set = nlink;
	id->isdir = isdir;
	id->security = security;
	if (verbose || id->ilist)
		dbprintf(_("inode %lld nlink %u %s dir\n"), id->ino, nlink,
			isdir ? "is" : "not");
}
