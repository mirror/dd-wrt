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

#define MSG_SKIPPING "%s: Skipping: Partition table signature found"

static int _passes_partitioned_filter(struct cmd_context *cmd, struct dev_filter *f, struct device *dev)
{
	struct dev_types *dt = (struct dev_types *) f->private;
	int ret;

	ret = dev_is_partitioned(dt, dev);

	if (ret == -EAGAIN) {
		/* let pass, call again after scan */
		log_debug_devs("filter partitioned deferred %s", dev_name(dev));
		dev->flags |= DEV_FILTER_AFTER_SCAN;
		return 1;
	}

	if (ret) {
		if (dev->ext.src == DEV_EXT_NONE)
			log_debug_devs(MSG_SKIPPING, dev_name(dev));
		else
			log_debug_devs(MSG_SKIPPING " [%s:%p]", dev_name(dev),
					dev_ext_name(dev), dev->ext.handle);
		return 0;
	}

	return 1;
}

static void _partitioned_filter_destroy(struct dev_filter *f)
{
	if (f->use_count)
		log_error(INTERNAL_ERROR "Destroying partitioned filter while in use %u times.", f->use_count);

	free(f);
}

struct dev_filter *partitioned_filter_create(struct dev_types *dt)
{
	struct dev_filter *f;

	if (!(f = zalloc(sizeof(struct dev_filter)))) {
		log_error("Partitioned filter allocation failed");
		return NULL;
	}

	f->passes_filter = _passes_partitioned_filter;
	f->destroy = _partitioned_filter_destroy;
	f->use_count = 0;
	f->private = dt;

	log_debug_devs("Partitioned filter initialised.");

	return f;
}
