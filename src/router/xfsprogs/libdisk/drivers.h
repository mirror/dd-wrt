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

#include <xfs/libxfs.h>
#include <sys/stat.h>
#include <volume.h>

/*
 * This stuff is all very platform specific.
 */

#ifdef __linux__
extern int   dm_get_subvol_stripe(char*, sv_type_t, int*, int*, int*,
					struct stat64*);
extern int   md_get_subvol_stripe(char*, sv_type_t, int*, int*, int*,
					struct stat64*);
extern int  lvm_get_subvol_stripe(char*, sv_type_t, int*, int*, int*,
					struct stat64*);
extern int  xvm_get_subvol_stripe(char*, sv_type_t, int*, int*, int*,
					struct stat64*);
extern int evms_get_subvol_stripe(char*, sv_type_t, int*, int*, int*,
					struct stat64*);
#else
#define stat64 stat
#define   dm_get_subvol_stripe(dev, type, a, b, c, stat)  (-1)
#define   md_get_subvol_stripe(dev, type, a, b, c, stat)  (-1)
#define  lvm_get_subvol_stripe(dev, type, a, b, c, stat)  (-1)
#define  xvm_get_subvol_stripe(dev, type, a, b, c, stat)  (-1)
#define evms_get_subvol_stripe(dev, type, a, b, c, stat)  (-1)
#endif
