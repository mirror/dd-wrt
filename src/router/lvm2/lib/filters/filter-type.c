/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
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

static int _passes_lvm_type_device_filter(struct cmd_context *cmd, struct dev_filter *f, struct device *dev)
{
	struct dev_types *dt = (struct dev_types *) f->private;
	const char *name = dev_name(dev);

	/* Is this a recognised device type? */
	if (!dt->dev_type_array[MAJOR(dev->dev)].max_partitions) {
		log_debug_devs("%s: Skipping: Unrecognised LVM device type %"
			       PRIu64, name, (uint64_t) MAJOR(dev->dev));
		return 0;
	}

	return 1;
}

static void _lvm_type_filter_destroy(struct dev_filter *f)
{
	if (f->use_count)
		log_error(INTERNAL_ERROR "Destroying lvm_type filter while in use %u times.", f->use_count);

	free(f);
}

struct dev_filter *lvm_type_filter_create(struct dev_types *dt)
{
	struct dev_filter *f;

	if (!(f = zalloc(sizeof(struct dev_filter)))) {
		log_error("LVM type filter allocation failed");
		return NULL;
	}

	f->passes_filter = _passes_lvm_type_device_filter;
	f->destroy = _lvm_type_filter_destroy;
	f->use_count = 0;
	f->private = dt;

	log_debug_devs("LVM type filter initialised.");

	return f;
}
