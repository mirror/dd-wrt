/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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

#ifndef __LIBXCMD_H__
#define __LIBXCMD_H__

#include "libxfs.h"
#include <sys/time.h>

/*
 * Device topology information.
 */
typedef struct fs_topology {
	int	dsunit;		/* stripe unit - data subvolume */
	int	dswidth;	/* stripe width - data subvolume */
	int	rtswidth;	/* stripe width - rt subvolume */
	int	lsectorsize;	/* logical sector size &*/
	int	psectorsize;	/* physical sector size */
} fs_topology_t;

extern void
get_topology(
	libxfs_init_t		*xi,
	struct fs_topology	*ft,
	int			force_overwrite);

extern void
calc_default_ag_geometry(
	int		blocklog,
	uint64_t	dblocks,
	int		multidisk,
	uint64_t	*agsize,
	uint64_t	*agcount);

extern int
check_overwrite(
	const char	*device);



#endif	/* __LIBXCMD_H__ */
