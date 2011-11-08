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
#include "md.h"

int
mnt_is_md_subvol(
	dev_t		dev,
	enum md_type	*type)
{
	*type = MD_TYPE_MD;
	if (major(dev) == MD_MAJOR)
		return 1;
	if (get_driver_block_major("md", major(dev)))
		return 1;
	*type = MD_TYPE_MDP;
	if (get_driver_block_major("mdp", major(dev)))
		return 1;
	return 0;
}

int
md_get_subvol_stripe(
	char		*dfile,
	sv_type_t	type,
	int		*sunit,
	int		*swidth,
	int		*sectalign,
	struct stat64	*sb)
{
	char		*pc;
	char		*dfile2 = NULL;
	enum md_type	md_type;

	if (mnt_is_md_subvol(sb->st_rdev, &md_type)) {
		struct md_array_info	md;
		int			fd;

		if (md_type == MD_TYPE_MDP) {
			pc = strrchr(dfile, 'd');
			if (pc)
				pc = strchr(pc, 'p');
			if (!pc) {
				fprintf(stderr,
					_("Error getting MD array device from %s\n"),
					dfile);
				exit(1);
			}
			dfile2 = malloc(pc - dfile + 1);
			if (dfile2 == NULL) {
				fprintf(stderr,
					_("Couldn't malloc device string\n"));
				exit(1);
			}
			strncpy(dfile2, dfile, pc - dfile);
			dfile2[pc - dfile + 1] = '\0';
		}
		/* Open device */
		fd = open(dfile2 ? dfile2 : dfile, O_RDONLY);
		if (fd == -1) {
			free(dfile2);
			return 0;
		}

		/* Is this thing on... */
		if (ioctl(fd, GET_ARRAY_INFO, &md)) {
			fprintf(stderr,
				_("Error getting MD array info from %s\n"),
				dfile2 ? dfile2 : dfile);
			exit(1);
		}
		close(fd);
		free(dfile2);

		/*
		 * Ignore levels we don't want aligned (e.g. linear)
		 * and deduct disk(s) from stripe width on RAID4/5/6
		 */
		switch (md.level) {
		case 6:
			md.raid_disks--;
			/* fallthrough */
		case 5:
		case 4:
			md.raid_disks--;
			/* fallthrough */
		case 1:
		case 0:
		case 10:
			break;
		default:
			return 0;
		}

		/* Update sizes */
		*sunit = md.chunk_size >> 9;
		*swidth = *sunit * md.raid_disks;
		*sectalign = (md.level == 4 || md.level == 5 || md.level == 6);

		return 1;
	}
	return 0;
}
