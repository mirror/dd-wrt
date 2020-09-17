// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

struct blkmap;

void
rtinit(xfs_mount_t		*mp);

int
generate_rtinfo(xfs_mount_t	*mp,
		xfs_rtword_t	*words,
		xfs_suminfo_t	*sumcompute);

#if 0

int
check_summary(xfs_mount_t	*mp);

void
process_rtbitmap(xfs_mount_t	*mp,
		xfs_dinode_t	*dino,
		struct blkmap	*blkmap);

void
process_rtsummary(xfs_mount_t	*mp,
		struct blkmap	*blkmap);
#endif
