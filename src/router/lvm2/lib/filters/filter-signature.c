/*
 * Copyright (C) 2004 Luca Berra
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
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

#ifdef __linux__

#define BUFSIZE 4096

static int _ignore_signature(struct cmd_context *cmd, struct dev_filter *f __attribute__((unused)),
		      struct device *dev)
{
	char buf[BUFSIZE];
	int ret = 0;

	if (!scan_bcache) {
		/* let pass, call again after scan */
		log_debug_devs("filter signature deferred %s", dev_name(dev));
		dev->flags |= DEV_FILTER_AFTER_SCAN;
		return 1;
	}

	memset(buf, 0, BUFSIZE);

	if (!dev_read_bytes(dev, 0, BUFSIZE, buf)) {
		log_debug_devs("%s: Skipping: error in signature detection",
			       dev_name(dev));
		ret = 0;
		goto out;
	}

	if (dev_is_lvm1(dev, buf, BUFSIZE)) {
		log_debug_devs("%s: Skipping lvm1 device", dev_name(dev));
		ret = 0;
		goto out;
	}

	if (dev_is_pool(dev, buf, BUFSIZE)) {
		log_debug_devs("%s: Skipping gfs-pool device", dev_name(dev));
		ret = 0;
		goto out;
	}
	ret = 1;

out:
	return ret;
}

static void _destroy(struct dev_filter *f)
{
	if (f->use_count)
		log_error(INTERNAL_ERROR "Destroying signature filter while in use %u times.", f->use_count);

	free(f);
}

struct dev_filter *signature_filter_create(struct dev_types *dt)
{
	struct dev_filter *f;

	if (!(f = zalloc(sizeof(*f)))) {
		log_error("md filter allocation failed");
		return NULL;
	}

	f->passes_filter = _ignore_signature;
	f->destroy = _destroy;
	f->use_count = 0;
	f->private = dt;

	log_debug_devs("signature filter initialised.");

	return f;
}

#else

struct dev_filter *signature_filter_create(struct dev_types *dt)
{
	return NULL;
}

#endif
