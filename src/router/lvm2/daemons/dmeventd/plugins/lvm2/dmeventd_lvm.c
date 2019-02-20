/*
 * Copyright (C) 2010-2015 Red Hat, Inc. All rights reserved.
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
#include "dmeventd_lvm.h"
#include "daemons/dmeventd/libdevmapper-event.h"
#include "tools/lvm2cmd.h"

#include <pthread.h>

/*
 * register_device() is called first and performs initialisation.
 * Only one device may be registered or unregistered at a time.
 */
static pthread_mutex_t _register_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Number of active registrations.
 */
static int _register_count = 0;
static struct dm_pool *_mem_pool = NULL;
static void *_lvm_handle = NULL;
static DM_LIST_INIT(_env_registry);

struct env_data {
	struct dm_list list;
	const char *cmd;
	const char *data;
};

DM_EVENT_LOG_FN("#lvm")

static void _lvm2_print_log(int level, const char *file, int line,
			    int dm_errno_or_class, const char *msg)
{
	print_log(level, file, line, dm_errno_or_class, "%s", msg);
}

/*
 * Currently only one event can be processed at a time.
 */
static pthread_mutex_t _event_mutex = PTHREAD_MUTEX_INITIALIZER;

void dmeventd_lvm2_lock(void)
{
	pthread_mutex_lock(&_event_mutex);
}

void dmeventd_lvm2_unlock(void)
{
	pthread_mutex_unlock(&_event_mutex);
}

int dmeventd_lvm2_init(void)
{
	int r = 0;

	pthread_mutex_lock(&_register_mutex);

	if (!_lvm_handle) {
		lvm2_log_fn(_lvm2_print_log);

		if (!(_lvm_handle = lvm2_init()))
			goto out;

		/*
		 * Need some space for allocations.  1024 should be more
		 * than enough for what we need (device mapper name splitting)
		 */
		if (!_mem_pool && !(_mem_pool = dm_pool_create("mirror_dso", 1024))) {
			lvm2_exit(_lvm_handle);
			_lvm_handle = NULL;
			goto out;
		}

		lvm2_disable_dmeventd_monitoring(_lvm_handle);
		/* FIXME Temporary: move to dmeventd core */
		lvm2_run(_lvm_handle, "_memlock_inc");
		log_debug("lvm plugin initilized.");
	}

	_register_count++;
	r = 1;

out:
	pthread_mutex_unlock(&_register_mutex);
	return r;
}

void dmeventd_lvm2_exit(void)
{
	pthread_mutex_lock(&_register_mutex);

	if (!--_register_count) {
		log_debug("lvm plugin shuting down.");
		lvm2_run(_lvm_handle, "_memlock_dec");
		dm_pool_destroy(_mem_pool);
		_mem_pool = NULL;
		dm_list_init(&_env_registry);
		lvm2_exit(_lvm_handle);
		_lvm_handle = NULL;
		log_debug("lvm plugin exited.");
	}

	pthread_mutex_unlock(&_register_mutex);
}

struct dm_pool *dmeventd_lvm2_pool(void)
{
	return _mem_pool;
}

int dmeventd_lvm2_run(const char *cmdline)
{
	return (lvm2_run(_lvm_handle, cmdline) == LVM2_COMMAND_SUCCEEDED);
}

int dmeventd_lvm2_command(struct dm_pool *mem, char *buffer, size_t size,
			  const char *cmd, const char *device)
{
	static char _internal_prefix[] =  "_dmeventd_";
	char *vg = NULL, *lv = NULL, *layer;
	int r;
	struct env_data *env_data;
	const char *env = NULL;

	if (!dm_split_lvm_name(mem, device, &vg, &lv, &layer)) {
		log_error("Unable to determine VG name from %s.",
			  device);
		return 0;
	}

	/* strip off the mirror component designations */
	if ((layer = strstr(lv, "_mimagetmp")) ||
	    (layer = strstr(lv, "_mlog")))
		*layer = '\0';

	if (!strncmp(cmd, _internal_prefix, sizeof(_internal_prefix) - 1)) {
		/* check if ENVVAR wasn't already resolved */
		dm_list_iterate_items(env_data, &_env_registry)
			if (!strcmp(cmd, env_data->cmd)) {
				env = env_data->data;
				break;
			}

		if (!env) {
			/* run lvm2 command to find out setting value */
			dmeventd_lvm2_lock();
			if (!dmeventd_lvm2_run(cmd) ||
			    !(env = getenv(cmd))) {
				log_error("Unable to find configured command.");
				return 0;
			}
			/* output of internal command passed via env var */
			env = dm_pool_strdup(_mem_pool, env); /* copy with lock */
			dmeventd_lvm2_unlock();
			if (!env ||
			    !(env_data = dm_pool_zalloc(_mem_pool, sizeof(*env_data))) ||
			    !(env_data->cmd = dm_pool_strdup(_mem_pool, cmd))) {
				log_error("Unable to allocate env memory.");
				return 0;
			}
			env_data->data = env;
			/* add to ENVVAR registry */
			dm_list_add(&_env_registry, &env_data->list);
		}
		cmd = env;
	}

	r = dm_snprintf(buffer, size, "%s %s/%s", cmd, vg, lv);

	dm_pool_free(mem, vg);

	if (r < 0) {
		log_error("Unable to form LVM command. (too long).");
		return 0;
	}

	return 1;
}
