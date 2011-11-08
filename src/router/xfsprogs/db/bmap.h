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

struct	bbmap;
struct	xfs_bmbt_rec;

typedef struct bmap_ext {
	xfs_dfiloff_t	startoff;
	xfs_dfsbno_t	startblock;
	xfs_dfilblks_t	blockcount;
	int		flag;
} bmap_ext_t;

extern void	bmap(xfs_dfiloff_t offset, xfs_dfilblks_t len, int whichfork,
		     int *nexp, bmap_ext_t *bep);
extern void	bmap_init(void);
extern void	convert_extent(struct xfs_bmbt_rec *rp, xfs_dfiloff_t *op,
			       xfs_dfsbno_t *sp, xfs_dfilblks_t *cp, int *fp);
extern void	make_bbmap(struct bbmap *bbmap, int nex, bmap_ext_t *bmp);
