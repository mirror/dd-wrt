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
