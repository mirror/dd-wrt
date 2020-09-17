// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

struct	bbmap;
struct	xfs_bmbt_rec;

typedef struct bmap_ext {
	xfs_fileoff_t	startoff;
	xfs_fsblock_t	startblock;
	xfs_filblks_t	blockcount;
	int		flag;
} bmap_ext_t;

extern void	bmap(xfs_fileoff_t offset, xfs_filblks_t len, int whichfork,
		     int *nexp, bmap_ext_t *bep);
extern void	bmap_init(void);
extern void	convert_extent(struct xfs_bmbt_rec *rp, xfs_fileoff_t *op,
			       xfs_fsblock_t *sp, xfs_filblks_t *cp, int *fp);
extern void	make_bbmap(struct bbmap *bbmap, int nex, bmap_ext_t *bmp);
