/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2009 Neil Brown <neilb@suse.de>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 *
 *    Added by Dale Stephenson
 *    steph@snapserver.com
 */

#include	"mdadm.h"
#include	"md_u.h"
#include	"md_p.h"

int Kill(char *dev, struct supertype *st, int force, int verbose, int noexcl)
{
	/*
	 * Nothing fancy about Kill.  It just zeroes out a superblock
	 * Definitely not safe.
	 * Returns:
	 *  0 - a zero superblock was successfully written out
	 *  1 - failed to write the zero superblock
	 *  2 - failed to open the device.
	 *  4 - failed to find a superblock.
	 */

	int fd, rv = 0;

	if (force)
		noexcl = 1;
	fd = open(dev, O_RDWR|(noexcl ? 0 : O_EXCL));
	if (fd < 0) {
		if (verbose >= 0)
			pr_err("Couldn't open %s for write - not zeroing\n",
				dev);
		return 2;
	}
	if (st == NULL)
		st = guess_super(fd);
	if (st == NULL || st->ss->init_super == NULL) {
		if (verbose >= 0)
			pr_err("Unrecognised md component device - %s\n", dev);
		close(fd);
		return 4;
	}
	st->ignore_hw_compat = 1;
	rv = st->ss->load_super(st, fd, dev);
	if (rv == 0 || (force && rv >= 2)) {
		st->ss->free_super(st);
		st->ss->init_super(st, NULL, NULL, "", NULL, NULL,
				   INVALID_SECTORS);
		if (st->ss->store_super(st, fd)) {
			if (verbose >= 0)
				pr_err("Could not zero superblock on %s\n",
					dev);
			rv = 1;
		} else if (rv) {
			if (verbose >= 0)
				pr_err("superblock zeroed anyway\n");
			rv = 0;
		}
	}
	close(fd);
	return rv;
}

int Kill_subarray(char *dev, char *subarray, int verbose)
{
	/* Delete a subarray out of a container, the subarry must be
	 * inactive.  The subarray string must be a subarray index
	 * number.
	 *
	 * 0 = successfully deleted subarray from all container members
	 * 1 = failed to sync metadata to one or more devices
	 * 2 = failed to find the container, subarray, or other resource
	 *     issue
	 */
	struct supertype supertype, *st = &supertype;
	int fd, rv = 2;

	memset(st, 0, sizeof(*st));

	fd = open_subarray(dev, subarray, st, verbose < 0);
	if (fd < 0)
		return 2;

	if (!st->ss->kill_subarray) {
		if (verbose >= 0)
			pr_err("Operation not supported for %s metadata\n",
			       st->ss->name);
		goto free_super;
	}

	if (is_subarray_active(subarray, st->devnm)) {
		if (verbose >= 0)
			pr_err("Subarray-%s still active, aborting\n",
			       subarray);
		goto free_super;
	}

	if (mdmon_running(st->devnm))
		st->update_tail = &st->updates;

	/* ok we've found our victim, drop the axe */
	rv = st->ss->kill_subarray(st);
	if (rv) {
		if (verbose >= 0)
			pr_err("Failed to delete subarray-%s from %s\n",
			       subarray, dev);
		goto free_super;
	}

	/* FIXME these routines do not report success/failure */
	if (st->update_tail)
		flush_metadata_updates(st);
	else
		st->ss->sync_metadata(st);

	if (verbose >= 0)
		pr_err("Deleted subarray-%s from %s, UUIDs may have changed\n",
		       subarray, dev);

	rv = 0;

 free_super:
	st->ss->free_super(st);
	close(fd);

	return rv;
}
