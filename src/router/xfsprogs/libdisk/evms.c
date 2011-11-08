/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <disk/volume.h>
#include "evms.h"

int
mnt_is_evms_subvol(
	dev_t		dev)
{
	if (major(dev) == EVMS_MAJOR)
		return 1;
	return get_driver_block_major("evms", major(dev));
}

int
evms_get_subvol_stripe(
	char		*device,
	sv_type_t	type,
	int		*sunit,
	int		*swidth,
	int		*sectalign,
	struct stat64	*sb)
{
	if (mnt_is_evms_subvol(sb->st_rdev)) {
		evms_vol_stripe_info_t	info;
		int			fd;

		fd = open(device, O_RDONLY);
		if (fd == -1)
			return 0;

		if (ioctl(fd, EVMS_GET_VOL_STRIPE_INFO, &info)) {
			close(fd);
			return 0;
		}

		/* Update sizes */
		*sunit = info.size;
		*swidth = *sunit * info.width;
		*sectalign = 0;

		close(fd);
		return 1;
	}
	return 0;
}
