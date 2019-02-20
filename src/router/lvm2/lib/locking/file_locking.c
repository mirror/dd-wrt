/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2014 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lib/misc/lib.h"
#include "lib/locking/locking.h"
#include "locking_types.h"
#include "lib/config/config.h"
#include "lib/config/defaults.h"
#include "lib/misc/lvm-string.h"
#include "lib/misc/lvm-flock.h"

#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

static char _lock_dir[PATH_MAX];

static void _fin_file_locking(void)
{
	release_flocks(1);
}

static void _reset_file_locking(void)
{
	release_flocks(0);
}

static int _file_lock_resource(struct cmd_context *cmd, const char *resource,
			       uint32_t flags, const struct logical_volume *lv)
{
	char lockfile[PATH_MAX];

	if (is_orphan_vg(resource) || is_global_vg(resource)) {
		if (dm_snprintf(lockfile, sizeof(lockfile),
				"%s/P_%s", _lock_dir, resource + 1) < 0) {
			log_error("Too long locking filename %s/P_%s.", _lock_dir, resource + 1);
			return 0;
		}
	} else
		if (dm_snprintf(lockfile, sizeof(lockfile), "%s/V_%s", _lock_dir, resource) < 0) {
			log_error("Too long locking filename %s/V_%s.", _lock_dir, resource);
			return 0;
		}

	if (!lock_file(lockfile, flags))
		return_0;

	return 1;
}

int init_file_locking(struct locking_type *locking, struct cmd_context *cmd,
		      int suppress_messages)
{
	int r;
	const char *locking_dir;

	init_flock(cmd);

	locking->lock_resource = _file_lock_resource;
	locking->reset_locking = _reset_file_locking;
	locking->fin_locking = _fin_file_locking;
	locking->flags = LCK_FLOCK;

	/* Get lockfile directory from config file */
	locking_dir = find_config_tree_str(cmd, global_locking_dir_CFG, NULL);
	if (!dm_strncpy(_lock_dir, locking_dir, sizeof(_lock_dir))) {
		log_error("Path for locking_dir %s is invalid.", locking_dir);
		return 0;
	}

	(void) dm_prepare_selinux_context(_lock_dir, S_IFDIR);
	r = dm_create_dir(_lock_dir);
	(void) dm_prepare_selinux_context(NULL, 0);

	if (!r)
		return 0;

	/* Trap a read-only file system */
	if ((access(_lock_dir, R_OK | W_OK | X_OK) == -1) && (errno == EROFS))
		return 0;

	return 1;
}
