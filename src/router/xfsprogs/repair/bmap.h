// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#ifndef _XFS_REPAIR_BMAP_H
#define _XFS_REPAIR_BMAP_H

/*
 * Extent descriptor.
 */
typedef struct bmap_ext {
	xfs_fileoff_t	startoff;
	xfs_fsblock_t	startblock;
	xfs_filblks_t	blockcount;
} bmap_ext_t;

/*
 * Block map.
 */
typedef	struct blkmap {
	int		naexts;
	int		nexts;
	bmap_ext_t	exts[1];
} blkmap_t;

#define	BLKMAP_SIZE(n)	\
	(offsetof(blkmap_t, exts) + (sizeof(bmap_ext_t) * (n)))

/*
 * For 32 bit platforms, we are limited to extent arrays of 2^31 bytes, which
 * limits the number of extents in an inode we can check. If we don't limit the
 * valid range, we can overflow the BLKMAP_SIZE() calculation and allocate less
 * memory than we think we needed, and hence walk off the end of the array and
 * corrupt memory.
 */
#if BITS_PER_LONG == 32
#define BLKMAP_NEXTS_MAX	((INT_MAX / sizeof(bmap_ext_t)) - 1)
#else
#define BLKMAP_NEXTS_MAX	INT_MAX
#endif

extern pthread_key_t dblkmap_key;
extern pthread_key_t ablkmap_key;

blkmap_t	*blkmap_alloc(xfs_extnum_t nex, int whichfork);
void		blkmap_free(blkmap_t *blkmap);
void		blkmap_free_final(void);

int		blkmap_set_ext(blkmap_t **blkmapp, xfs_fileoff_t o,
			       xfs_fsblock_t b, xfs_filblks_t c);

xfs_fsblock_t	blkmap_get(blkmap_t *blkmap, xfs_fileoff_t o);
int		blkmap_getn(blkmap_t *blkmap, xfs_fileoff_t o,
			    xfs_filblks_t nb, bmap_ext_t **bmpp,
			    bmap_ext_t *bmpp_single);
xfs_fileoff_t	blkmap_last_off(blkmap_t *blkmap);
xfs_fileoff_t	blkmap_next_off(blkmap_t *blkmap, xfs_fileoff_t o, int *t);

#endif /* _XFS_REPAIR_BMAP_H */
