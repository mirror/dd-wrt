/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2013 Red Hat, Inc. All rights reserved.
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
#include "lib/device/device.h"

static int _and_p(struct cmd_context *cmd, struct dev_filter *f, struct device *dev)
{
	struct dev_filter **filters;
	int ret;

	for (filters = (struct dev_filter **) f->private; *filters; ++filters) {
		ret = (*filters)->passes_filter(cmd, *filters, dev);

		if (!ret)
			return 0;	/* No 'stack': a filter, not an error. */
	}

	return 1;
}

static int _and_p_with_dev_ext_info(struct cmd_context *cmd, struct dev_filter *f, struct device *dev)
{
	int r;

	dev_ext_enable(dev, external_device_info_source());
	r = _and_p(cmd, f, dev);
	dev_ext_disable(dev);

	return r;
}

static void _composite_destroy(struct dev_filter *f)
{
	struct dev_filter **filters;

	if (f->use_count)
		log_error(INTERNAL_ERROR "Destroying composite filter while in use %u times.", f->use_count);

	for (filters = (struct dev_filter **) f->private; *filters; ++filters)
		(*filters)->destroy(*filters);

	free(f->private);
	free(f);
}

static void _wipe(struct dev_filter *f)
{
	struct dev_filter **filters;

	for (filters = (struct dev_filter **) f->private; *filters; ++filters)
		if ((*filters)->wipe)
			(*filters)->wipe(*filters);
}

struct dev_filter *composite_filter_create(int n, int use_dev_ext_info, struct dev_filter **filters)
{
	struct dev_filter **filters_copy, *cft;

	if (!filters)
		return_NULL;

	if (!(filters_copy = malloc(sizeof(*filters) * (n + 1)))) {
		log_error("Composite filters allocation failed.");
		return NULL;
	}

	memcpy(filters_copy, filters, sizeof(*filters) * n);
	filters_copy[n] = NULL;

	if (!(cft = zalloc(sizeof(*cft)))) {
		log_error("Composite filters allocation failed.");
		free(filters_copy);
		return NULL;
	}

	cft->passes_filter = use_dev_ext_info ? _and_p_with_dev_ext_info : _and_p;
	cft->destroy = _composite_destroy;
	cft->wipe = _wipe;
	cft->use_count = 0;
	cft->private = filters_copy;

	log_debug_devs("Composite filter initialised.");

	return cft;
}
