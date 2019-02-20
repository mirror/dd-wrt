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
#include "lib/config/config.h"
#include "lib/misc/lvm-flock.h"
#include "lib/misc/lvm-signal.h"
#include "lib/locking/locking.h"

#include <sys/file.h>
#include <fcntl.h>

struct lock_list {
	struct dm_list list;
	int lf;
	char *res;
};

static struct dm_list _lock_list;
static int _prioritise_write_locks;

/* Drop lock known to be shared with another file descriptor. */
static void _drop_shared_flock(const char *file, int fd)
{
	log_debug_locking("_drop_shared_flock %s.", file);

	if (close(fd) < 0)
		log_sys_debug("close", file);
}

static void _undo_flock(const char *file, int fd)
{
	struct stat buf1, buf2;

	log_debug_locking("_undo_flock %s", file);
	if (!flock(fd, LOCK_NB | LOCK_EX) &&
	    !stat(file, &buf1) &&
	    !fstat(fd, &buf2) &&
	    is_same_inode(buf1, buf2))
		if (unlink(file))
			log_sys_debug("unlink", file);

	if (close(fd) < 0)
		log_sys_debug("close", file);
}

static int _release_lock(const char *file, int unlock)
{
	struct lock_list *ll;
	struct dm_list *llh, *llt;

	dm_list_iterate_safe(llh, llt, &_lock_list) {
		ll = dm_list_item(llh, struct lock_list);

		if (!file || !strcmp(ll->res, file)) {
			dm_list_del(llh);
			if (unlock) {
				log_very_verbose("Unlocking %s", ll->res);
				if (flock(ll->lf, LOCK_NB | LOCK_UN))
					log_sys_debug("flock", ll->res);
				_undo_flock(ll->res, ll->lf);
			} else
				_drop_shared_flock(ll->res, ll->lf);

			free(ll->res);
			free(llh);

			if (file)
				return 1;
		}
	}

	return 0;
}

void release_flocks(int unlock)
{
	_release_lock(NULL, unlock);
}

static int _do_flock(const char *file, int *fd, int operation, uint32_t nonblock)
{
	int r;
	int old_errno;
	struct stat buf1, buf2;

	log_debug_locking("_do_flock %s %c%c", file,
			  operation == LOCK_EX ? 'W' : 'R', nonblock ? ' ' : 'B');
	do {
		if ((*fd > -1) && close(*fd))
			log_sys_debug("close", file);

		if ((*fd = open(file, O_CREAT | O_APPEND | O_RDWR, 0777)) < 0) {
			log_sys_error("open", file);
			return 0;
		}

		if (nonblock)
			operation |= LOCK_NB;
		else
			sigint_allow();

		r = flock(*fd, operation);
		old_errno = errno;
		if (!nonblock) {
			sigint_restore();
			if (sigint_caught()) {
				log_error("Giving up waiting for lock.");
				break;
			}
		}

		if (r) {
			errno = old_errno;
			log_sys_error("flock", file);
			break;
		}

		if (!stat(file, &buf1) && !fstat(*fd, &buf2) &&
		    is_same_inode(buf1, buf2))
			return 1;
	} while (!nonblock);

	if (close(*fd))
		log_sys_debug("close", file);
	*fd = -1;

	return_0;
}

#define AUX_LOCK_SUFFIX ":aux"

static int _do_write_priority_flock(const char *file, int *fd, int operation, uint32_t nonblock)
{
	int r, fd_aux = -1;
	char *file_aux = alloca(strlen(file) + sizeof(AUX_LOCK_SUFFIX));

	strcpy(file_aux, file);
	strcat(file_aux, AUX_LOCK_SUFFIX);

	if ((r = _do_flock(file_aux, &fd_aux, LOCK_EX, 0))) {
		if (operation == LOCK_EX) {
			r = _do_flock(file, fd, operation, nonblock);
			_undo_flock(file_aux, fd_aux);
		} else {
			_undo_flock(file_aux, fd_aux);
			r = _do_flock(file, fd, operation, nonblock);
		}
	}

	return r;
}

int lock_file(const char *file, uint32_t flags)
{
	int operation;
	uint32_t nonblock = flags & LCK_NONBLOCK;
	int r;

	struct lock_list *ll;
	char state;

	switch (flags & LCK_TYPE_MASK) {
	case LCK_READ:
		operation = LOCK_SH;
		state = 'R';
		break;
	case LCK_WRITE:
		operation = LOCK_EX;
		state = 'W';
		break;
	case LCK_UNLOCK:
		return _release_lock(file, 1);
	default:
		log_error("Unrecognised lock type: %d", flags & LCK_TYPE_MASK);
		return 0;
	}

	if (!(ll = malloc(sizeof(struct lock_list))))
		return_0;

	if (!(ll->res = strdup(file))) {
		free(ll);
		return_0;
	}

	ll->lf = -1;

	log_very_verbose("Locking %s %c%c", ll->res, state,
			 nonblock ? ' ' : 'B');

	(void) dm_prepare_selinux_context(file, S_IFREG);
	if (_prioritise_write_locks)
		r = _do_write_priority_flock(file, &ll->lf, operation, nonblock);
	else 
		r = _do_flock(file, &ll->lf, operation, nonblock);
	(void) dm_prepare_selinux_context(NULL, 0);

	if (r)
		dm_list_add(&_lock_list, &ll->list);
	else {
		free(ll->res);
		free(ll);
		stack;
	}

	return r;
}

void init_flock(struct cmd_context *cmd)
{
	dm_list_init(&_lock_list);

	_prioritise_write_locks =
	    find_config_tree_bool(cmd, global_prioritise_write_locks_CFG, NULL);
}
