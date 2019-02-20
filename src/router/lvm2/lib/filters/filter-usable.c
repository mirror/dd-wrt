/*
 * Copyright (C) 2014 Red Hat, Inc. All rights reserved.
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
#include "lib/activate/activate.h"
#ifdef UDEV_SYNC_SUPPORT
#include <libudev.h>
#include "lib/device/dev-ext-udev-constants.h"
#endif

struct filter_data {
	filter_mode_t mode;
	int skip_lvs;
};

static const char *_too_small_to_hold_pv_msg = "Too small to hold a PV";

static int _native_check_pv_min_size(struct device *dev)
{
	uint64_t size;
	int ret = 0;

	/* Check it's not too small */
	if (!dev_get_size(dev, &size)) {
		log_debug_devs("%s: Skipping: dev_get_size failed", dev_name(dev));
		goto out;
	}

	if (size < pv_min_size()) {
		log_debug_devs("%s: Skipping: %s", dev_name(dev),
				_too_small_to_hold_pv_msg);
		goto out;
	}

	ret = 1;
out:
	return ret;
}

#ifdef UDEV_SYNC_SUPPORT
static int _udev_check_pv_min_size(struct device *dev)
{
	struct dev_ext *ext;
	const char *size_str;
	char *endp;
	uint64_t size;

	if (!(ext = dev_ext_get(dev)))
		return_0;

	if (!(size_str = udev_device_get_sysattr_value((struct udev_device *)ext->handle, DEV_EXT_UDEV_SYSFS_ATTR_SIZE))) {
		log_debug_devs("%s: Skipping: failed to get size from sysfs [%s:%p]",
				dev_name(dev), dev_ext_name(dev), dev->ext.handle);
		return 0;
	}

	errno = 0;
	size = strtoull(size_str, &endp, 10);
	if (errno || !endp || *endp) {
		log_debug_devs("%s: Skipping: failed to parse size from sysfs [%s:%p]",
				dev_name(dev), dev_ext_name(dev), dev->ext.handle);
		return 0;
	}

	if (size < pv_min_size()) {
		log_debug_devs("%s: Skipping: %s [%s:%p]", dev_name(dev),
				_too_small_to_hold_pv_msg,
				dev_ext_name(dev), dev->ext.handle);
		return 0;
	}

	return 1;
}
#else
static int _udev_check_pv_min_size(struct device *dev)
{
	return 1;
}
#endif

static int _check_pv_min_size(struct device *dev)
{
	if (dev->ext.src == DEV_EXT_NONE)
		return _native_check_pv_min_size(dev);

	if (dev->ext.src == DEV_EXT_UDEV)
		return _udev_check_pv_min_size(dev);

	log_error(INTERNAL_ERROR "Missing hook for PV min size check "
		  "using external device info source %s", dev_ext_name(dev));

	return 0;
}

static int _passes_usable_filter(struct cmd_context *cmd, struct dev_filter *f, struct device *dev)
{
	struct filter_data *data = f->private;
	filter_mode_t mode = data->mode;
	int skip_lvs = data->skip_lvs;
	struct dev_usable_check_params ucp = {0};
	int r = 1;

	/* further checks are done on dm devices only */
	if (dm_is_dm_major(MAJOR(dev->dev))) {
		switch (mode) {
		case FILTER_MODE_NO_LVMETAD:
			ucp.check_empty = 1;
			ucp.check_blocked = 1;
			ucp.check_suspended = ignore_suspended_devices();
			ucp.check_error_target = 1;
			ucp.check_reserved = 1;
			ucp.check_lv = skip_lvs;
			break;
		case FILTER_MODE_PRE_LVMETAD:
			ucp.check_empty = 1;
			ucp.check_blocked = 1;
			ucp.check_suspended = 0;
			ucp.check_error_target = 1;
			ucp.check_reserved = 1;
			ucp.check_lv = skip_lvs;
			break;
		case FILTER_MODE_POST_LVMETAD:
			ucp.check_empty = 0;
			ucp.check_blocked = 1;
			ucp.check_suspended = ignore_suspended_devices();
			ucp.check_error_target = 0;
			ucp.check_reserved = 0;
			ucp.check_lv = skip_lvs;
			break;
		}

		if (!(r = device_is_usable(dev, ucp)))
			log_debug_devs("%s: Skipping unusable device.", dev_name(dev));
	}

	if (r) {
		/* check if the device is not too small to hold a PV */
		switch (mode) {
		case FILTER_MODE_NO_LVMETAD:
			/* fall through */
		case FILTER_MODE_PRE_LVMETAD:
			r = _check_pv_min_size(dev);
			break;
		case FILTER_MODE_POST_LVMETAD:
			/* nothing to do here */
			break;
		}
	}

	return r;
}

static void _usable_filter_destroy(struct dev_filter *f)
{
	if (f->use_count)
		log_error(INTERNAL_ERROR "Destroying usable device filter while in use %u times.", f->use_count);

	free(f->private);
	free(f);
}

struct dev_filter *usable_filter_create(struct cmd_context *cmd, struct dev_types *dt __attribute__((unused)), filter_mode_t mode)
{
	struct filter_data *data;
	struct dev_filter *f;

	if (!(f = zalloc(sizeof(struct dev_filter)))) {
		log_error("Usable device filter allocation failed");
		return NULL;
	}

	f->passes_filter = _passes_usable_filter;
	f->destroy = _usable_filter_destroy;
	f->use_count = 0;

	if (!(data = zalloc(sizeof(struct filter_data)))) {
		log_error("Usable device filter mode allocation failed");
		free(f);
		return NULL;
	}

	data->mode = mode;

	data->skip_lvs = !find_config_tree_bool(cmd, devices_scan_lvs_CFG, NULL);

	f->private = data;

	log_debug_devs("Usable device filter initialised (scan_lvs %d).", !data->skip_lvs);

	return f;
}
