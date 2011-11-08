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

void
get_subvol_stripe_wrapper(
	char		*dev,
	sv_type_t	type,
	int		*sunit,
	int		*swidth,
	int		*sectalign)
{
	struct stat64	sb;

	if (dev == NULL)
		return;

	if (stat64(dev, &sb)) {
		fprintf(stderr, _("Cannot stat %s: %s\n"),
			dev, strerror(errno));
		exit(1);
	}

	if (  dm_get_subvol_stripe(dev, type, sunit, swidth, sectalign, &sb))
		return;
	if (  md_get_subvol_stripe(dev, type, sunit, swidth, sectalign, &sb))
		return;
	if ( lvm_get_subvol_stripe(dev, type, sunit, swidth, sectalign, &sb))
		return;
	if ( xvm_get_subvol_stripe(dev, type, sunit, swidth, sectalign, &sb))
		return;
	if (evms_get_subvol_stripe(dev, type, sunit, swidth, sectalign, &sb))
		return;

	/* ... add new device drivers here */
}

#define DEVICES	"/proc/devices"

/*
 * General purpose routine which dredges through procfs trying to
 * match up device driver names with the associated major numbers
 * being used in the running kernel.
 */
int
get_driver_block_major(
	const char	*driver,
	int		major)
{
	FILE		*f;
	char		buf[64], puf[64];
	int		dmajor, match = 0;

	if ((f = fopen(DEVICES, "r")) == NULL)
		return match;
	while (fgets(buf, sizeof(buf), f))	/* skip to block dev section */
		if (strncmp("Block devices:\n", buf, sizeof(buf)) == 0)
			break;
	while (fgets(buf, sizeof(buf), f))
		if ((sscanf(buf, "%u %s\n", &dmajor, puf) == 2) &&
		    (strncmp(puf, driver, sizeof(puf)) == 0) &&
		    (dmajor == major))
			match = 1;
	fclose(f);
	return match;
}
