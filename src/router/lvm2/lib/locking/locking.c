/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
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
#include "lib/misc/lvm-string.h"
#include "lib/activate/activate.h"
#include "lib/commands/toolcontext.h"
#include "lib/mm/memlock.h"
#include "lib/config/defaults.h"
#include "lib/cache/lvmcache.h"
#include "lib/misc/lvm-signal.h"

#include <assert.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>

static struct locking_type _locking;

static int _vg_lock_count = 0;		/* Number of locks held */
static int _vg_write_lock_held = 0;	/* VG write lock held? */
static int _blocking_supported = 0;
static int _file_locking_readonly = 0;
static int _file_locking_sysinit = 0;
static int _file_locking_ignorefail = 0;
static int _file_locking_failed = 0;

static void _unblock_signals(void)
{
	/* Don't unblock signals while any locks are held */
	if (!_vg_lock_count)
		unblock_signals();
}

void reset_locking(void)
{
	int was_locked = _vg_lock_count;

	/* file locking disabled */
	if (!_locking.flags)
		return;

	_vg_lock_count = 0;
	_vg_write_lock_held = 0;

	if (_locking.reset_locking)
		_locking.reset_locking();

	if (was_locked)
		_unblock_signals();

	memlock_reset();
}

static void _update_vg_lock_count(const char *resource, uint32_t flags)
{
	/* Ignore locks not associated with updating VG metadata */
	if (!strcmp(resource, VG_GLOBAL))
		return;

	if ((flags & LCK_TYPE_MASK) == LCK_UNLOCK)
		_vg_lock_count--;
	else
		_vg_lock_count++;

	/* We don't bother to reset this until all VG locks are dropped */
	if ((flags & LCK_TYPE_MASK) == LCK_WRITE)
		_vg_write_lock_held = 1;
	else if (!_vg_lock_count)
		_vg_write_lock_held = 0;
}

/*
 * A mess of options have been introduced over time to override
 * or tweak the behavior of file locking, and indirectly other
 * behaviors.  These options are allowed in different but
 * overlapping sets of commands (see command-lines.in)
 *
 * --nolocking
 *
 * Command won't try to set up or use file locks at all.
 *
 * --readonly
 *
 * Command will grant any read lock request, without trying
 * to acquire an actual file lock.  Command will refuse any
 * write lock request.  (Activation, which uses a write lock,
 * is not allowed.)
 *
 * --ignorelockingfailure
 *
 * Command tries to set up file locks and will use them
 * (both read and write) if successful.  If command fails
 * to set up file locks it falls back to readonly behavior
 * above, while allowing activation.
 *
 * --sysinit
 *
 * The same as ignorelockingfailure.
 *
 * --sysinit --readonly
 *
 * The combination of these two flags acts like --readonly,
 * refusing write lock requests, but makes an exception to
 * allow activation.
 *
 * global/metadata_read_only
 *
 * The command acquires actual read locks and refuses
 * write lock requests.
 */

int init_locking(struct cmd_context *cmd,
		 int file_locking_sysinit, int file_locking_readonly, int file_locking_ignorefail)
{
	int suppress_messages = 0;

	if (file_locking_sysinit || getenv("LVM_SUPPRESS_LOCKING_FAILURE_MESSAGES"))
		suppress_messages = 1;

	_blocking_supported = find_config_tree_bool(cmd, global_wait_for_locks_CFG, NULL);
	_file_locking_readonly = file_locking_readonly;
	_file_locking_sysinit = file_locking_sysinit;
	_file_locking_ignorefail = file_locking_ignorefail;

	log_debug("File locking settings: readonly:%d sysinit:%d ignorelockingfailure:%d global/metadata_read_only:%d global/wait_for_locks:%d.",
		  _file_locking_readonly, _file_locking_sysinit, _file_locking_ignorefail,
		  cmd->metadata_read_only, _blocking_supported);

	if (!init_file_locking(&_locking, cmd, suppress_messages)) {
		log_error_suppress(suppress_messages, "File locking initialisation failed.");

		_file_locking_failed = 1;

		if (file_locking_sysinit || file_locking_ignorefail)
			return 1;

		return 0;
	}

	return 1;
}

void fin_locking(void)
{
	/* file locking disabled */
	if (!_locking.flags)
		return;

	_locking.fin_locking();
}

/*
 * VG locking is by VG name.
 * FIXME This should become VG uuid.
 */
static int _lock_vol(struct cmd_context *cmd, const char *resource, uint32_t flags)
{
	int ret = 0;

	block_signals(flags);

	ret = _locking.lock_resource(cmd, resource, flags, NULL);

	_unblock_signals();

	return ret;
}

int lock_vol(struct cmd_context *cmd, const char *vol, uint32_t flags, const struct logical_volume *lv)
{
	char resource[258] __attribute__((aligned(8)));
	uint32_t lck_type = flags & LCK_TYPE_MASK;

	if (!_blocking_supported)
		flags |= LCK_NONBLOCK;

	if (is_orphan_vg(vol))
		vol = VG_ORPHANS;

	if (!dm_strncpy(resource, vol, sizeof(resource))) {
		log_error(INTERNAL_ERROR "Resource name %s is too long.", vol);
		return 0;
	}

	/*
	 * File locking is disabled by --nolocking.
	 */
	if (!_locking.flags)
		goto out_hold;

	/*
	 * When file locking could not be initialized, --ignorelockingfailure
	 * and --sysinit behave like --readonly, but allow activation.
	 */
	if (_file_locking_failed && (_file_locking_sysinit || _file_locking_ignorefail)) {
		if (lck_type != LCK_WRITE)
			goto out_hold;

		if (cmd->is_activating && strcmp(vol, VG_GLOBAL))
			goto out_hold;

		goto out_fail;
	}

	/*
	 * When --readonly is set, grant read lock requests without trying to
	 * acquire an actual lock, and refuse write lock requests.
	 */
	if (_file_locking_readonly && !_file_locking_sysinit) {
		if (lck_type != LCK_WRITE)
			goto out_hold;

		log_error("Operation prohibited while --readonly is set.");
		goto out_fail;
	}

	/*
	 * When --readonly and --sysinit are set, grant read lock requests without
	 * trying to acquire an actual lock, and refuse write lock requests except
	 * in the case of activation which is permitted.
	 */
	if (_file_locking_readonly && _file_locking_sysinit) {
		if (lck_type != LCK_WRITE)
			goto out_hold;

		if (cmd->is_activating) {
			log_warn("Allowing activation with --readonly --sysinit.");
			goto out_hold;
		}

		log_error("Operation prohibited while --readonly is set.");
		goto out_fail;
	}

	/*
	 * When global/metadata_read_only is set, acquire actual read locks and
	 * refuse write lock requests.
	 */
	if (cmd->metadata_read_only) {
		if ((lck_type == LCK_WRITE) && strcmp(vol, VG_GLOBAL)) {
			log_error("Operation prohibited while global/metadata_read_only is set.");
			goto out_fail;
		}

		/* continue and acquire a read file lock */
	}

	if (!_lock_vol(cmd, resource, flags))
		goto out_fail;

out_hold:
	/*
	 * FIXME: other parts of the code want to check if a VG is
	 * locked by looking in lvmcache.  They shouldn't need to
	 * do that, and we should be able to remove this.
	 */
	if (lck_type != LCK_UNLOCK)
		lvmcache_lock_vgname(resource, lck_type == LCK_READ);
	else if (lck_type == LCK_UNLOCK)
		lvmcache_unlock_vgname(resource);

	/* FIXME: we shouldn't need to keep track of this either. */
	_update_vg_lock_count(resource, flags);
	return 1;

out_fail:
	if (lck_type == LCK_UNLOCK)
		_update_vg_lock_count(resource, flags);
	return 0;
}

/* Lock a list of LVs */
int activate_lvs(struct cmd_context *cmd, struct dm_list *lvs, unsigned exclusive)
{
	struct dm_list *lvh;
	struct lv_list *lvl;

	dm_list_iterate_items(lvl, lvs) {
		if (!activate_lv(cmd, lvl->lv)) {
			log_error("Failed to activate %s", display_lvname(lvl->lv));

			dm_list_uniterate(lvh, lvs, &lvl->list) {
				lvl = dm_list_item(lvh, struct lv_list);
				if (!deactivate_lv(cmd, lvl->lv))
					stack;
			}
			return 0;
		}
	}

	return 1;
}

int vg_write_lock_held(void)
{
	return _vg_write_lock_held;
}

int sync_local_dev_names(struct cmd_context* cmd)
{
	memlock_unlock(cmd);
	fs_unlock();
	return 1;
}

