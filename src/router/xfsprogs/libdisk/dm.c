/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
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

#include "drivers.h"

int
mnt_is_dm_subvol(
	dev_t		dev)
{
	return get_driver_block_major("device-mapper", major(dev));
}

int
dm_get_subvol_stripe(
	char		*dfile,
	sv_type_t	type,
	int		*sunit,
	int		*swidth,
	int		*sectalign,
	struct stat64	*sb)
{
	int		count, stripes = 0, stripesize = 0;
	int		dmpipe[2];
	char		*largv[7];
	FILE		*stream;
	long long	offset, size;
	static char	*command = "table";	/* dmsetup table /dev/xxx */
	char		major_str[4], minor_str[4];

	if (!mnt_is_dm_subvol(sb->st_rdev))
		return 0;

	/* Quest for dmsetup */
	if (!access("/usr/local/sbin/dmsetup", R_OK|X_OK))
		largv[0] = "/usr/local/sbin/dmsetup";
	else if (!access("/usr/sbin/dmsetup", R_OK|X_OK))
		largv[0] = "/usr/sbin/dmsetup";
	else if (!access("/sbin/dmsetup", R_OK|X_OK))
		largv[0] = "/sbin/dmsetup";
	else {
		fprintf(stderr,
	_("Warning - device mapper device, but no dmsetup(8) found\n"));
		return 0;
	}

	snprintf(major_str, 4, "%d", major(sb->st_rdev));
	snprintf(minor_str, 4, "%d", minor(sb->st_rdev));

	largv[1] = command;
	largv[2] = "-j";
	largv[3] = major_str;
	largv[4] = "-m";
	largv[5] = minor_str;
	largv[6] = NULL;

	/* Open pipe */
	if (pipe(dmpipe) < 0) {
		fprintf(stderr, _("Could not open pipe\n"));
		exit(1);
	}

	/* Spawn dmsetup */
	switch (fork()) {
	case 0:
		/* Plumbing */
		close(dmpipe[0]);

		if (dmpipe[1] != STDOUT_FILENO)
			dup2(dmpipe[1], STDOUT_FILENO);

		execv(largv[0], largv);

		fprintf(stderr, _("Failed to execute %s\n"), largv[0]);
		exit(1);

	case -1:
		fprintf(stderr, _("Failed forking dmsetup process\n"));
		exit(1);

	default:
		break;
	}

	close(dmpipe[1]);
	stream = fdopen(dmpipe[0], "r");
	count = fscanf(stream, "%lld %lld striped %d %d ",
			&offset, &size, &stripes, &stripesize);
	fclose(stream);
	if (count != 4)
		return 0;

	/* Update sizes */
	*sunit = stripesize;
	*swidth = (stripes * stripesize);
	*sectalign = 0;
	return 1;
}
