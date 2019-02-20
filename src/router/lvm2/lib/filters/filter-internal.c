/*
 * Copyright (C) 2006 Red Hat, Inc. All rights reserved.
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

#include "base/memory/zalloc.h"
#include "lib/misc/lib.h"
#include "lib/filters/filter.h"

static DM_LIST_INIT(_allow_devs);

int internal_filter_allow(struct dm_pool *mem, struct device *dev)
{
	struct device_list *devl;

	if (!(devl = dm_pool_alloc(mem, sizeof(*devl)))) {
		log_error("device_list element allocation failed");
		return 0;
	}
	devl->dev = dev;

	dm_list_add(&_allow_devs, &devl->list);
	return 1;
}

void internal_filter_clear(void)
{
	dm_list_init(&_allow_devs);
}

static int _passes_internal(struct cmd_context *cmd, struct dev_filter *f __attribute__((unused)),
			    struct device *dev)
{
	struct device_list *devl;

	if (!internal_filtering())
		return 1;
	
	dm_list_iterate_items(devl, &_allow_devs) {
		if (devl->dev == dev)
			return 1;
	}
	
	log_debug_devs("%s: Skipping for internal filtering.", dev_name(dev));
	return 0;
}

static void _destroy(struct dev_filter *f)
{
	if (f->use_count)
		log_error(INTERNAL_ERROR "Destroying internal filter while in use %u times.", f->use_count);

	free(f);
}

struct dev_filter *internal_filter_create(void)
{
	struct dev_filter *f;

	if (!(f = zalloc(sizeof(*f)))) {
		log_error("md filter allocation failed");
		return NULL;
	}

	f->passes_filter = _passes_internal;
	f->destroy = _destroy;
	f->use_count = 0;

	log_debug_devs("Internal filter initialised.");

	return f;
}

