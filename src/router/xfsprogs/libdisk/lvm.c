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

#include "drivers.h"

#ifndef LVM_BLK_MAJOR
#define LVM_BLK_MAJOR	58
#endif

int
mnt_is_lvm_subvol(
	dev_t		dev)
{
	if (major(dev) == LVM_BLK_MAJOR)
		return 1;
	return get_driver_block_major("lvm", major(dev));
}

int
lvm_get_subvol_stripe(
	char		*dfile,
	sv_type_t	type,
	int		*sunit,
	int		*swidth,
	int		*sectalign,
	struct stat64	*sb)
{
	int		lvpipe[2], stripes = 0, stripesize = 0;
	char		*largv[3], buf[1024];
	FILE		*stream;
	char		tmppath[MAXPATHLEN];

	if (!mnt_is_lvm_subvol(sb->st_rdev))
		return 0;

	/* Quest for lvdisplay */
	if (!access("/usr/local/sbin/lvdisplay", R_OK|X_OK))
		largv[0] = "/usr/local/sbin/lvdisplay";
	else if (!access("/usr/sbin/lvdisplay", R_OK|X_OK))
		largv[0] = "/usr/sbin/lvdisplay";
	else if (!access("/sbin/lvdisplay", R_OK|X_OK))
		largv[0] = "/sbin/lvdisplay";
	else {
		fprintf(stderr,
			_("Warning - LVM device, but no lvdisplay(8) found\n"));
		return 0;
	}

	/* realpath gives an absolute pathname */
	largv[1] = realpath(dfile, tmppath);
	largv[2] = NULL;

	/* Open pipe */
	if (pipe(lvpipe) < 0) {
		fprintf(stderr, _("Could not open pipe\n"));
		exit(1);
	}

	/* Spawn lvdisplay */
	switch (fork()) {
	case 0:
		/* Plumbing */
		close(lvpipe[0]);

		if (lvpipe[1] != STDOUT_FILENO)
			dup2(lvpipe[1], STDOUT_FILENO);

		execv(largv[0], largv);

		fprintf(stderr, _("Failed to execute %s\n"), largv[0]);
		exit(1);

	case -1:
		fprintf(stderr, _("Failed forking lvdisplay process\n"));
		exit(1);

	default:
		break;
	}

	close(lvpipe[1]);
	stream = fdopen(lvpipe[0], "r");

	/* Scan stream for keywords */
	while (fgets(buf, 1023, stream) != NULL) {

		if (!strncmp(buf, "Stripes", 7))
			sscanf(buf, "Stripes %d", &stripes);

		if (!strncmp(buf, "Stripe size", 11))
			sscanf(buf, "Stripe size (KByte) %d", &stripesize);
	}

	/* Update sizes */
	*sunit = stripesize << 1;
	*swidth = (stripes * stripesize) << 1;
	*sectalign = 0;

	fclose(stream);

	return 1;
}
