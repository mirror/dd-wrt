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

#include "lib/misc/lib.h"
#include "lib/device/device.h"

#ifdef UDEV_SYNC_SUPPORT
#include <libudev.h>
#endif

struct ext_registry_item {
	const char *name;
	struct dev_ext *(* dev_ext_get) (struct device *dev);
	int (*dev_ext_release) (struct device *dev);
};

#define EXT_REGISTER(id,name) [id] = { #name, &_dev_ext_get_ ## name, &_dev_ext_release_ ## name }

/*
 * DEV_EXT_NONE
 */
static struct dev_ext *_dev_ext_get_none(struct device *dev)
{
	dev->ext.handle = NULL;
	return &dev->ext;
}

static int _dev_ext_release_none(struct device *dev)
{
	dev->ext.handle = NULL;
	return 1;
}

/*
 * DEV_EXT_UDEV
 */
static struct dev_ext *_dev_ext_get_udev(struct device *dev)
{
#ifdef UDEV_SYNC_SUPPORT
	struct udev *udev;
	struct udev_device *udev_device;

	if (dev->ext.handle)
		return &dev->ext;

	if (!(udev = udev_get_library_context()))
		return_NULL;

	if (!(udev_device = udev_device_new_from_devnum(udev, 'b', dev->dev)))
		return_NULL;

#ifdef HAVE_LIBUDEV_UDEV_DEVICE_GET_IS_INITIALIZED
	if (!udev_device_get_is_initialized(udev_device)) {
		/* Timeout or some other udev db inconsistency! */
		log_error("Udev database has incomplete information about device %s.", dev_name(dev));
		return NULL;
	}
#endif

	dev->ext.handle = (void *) udev_device;
	return &dev->ext;
#else
	return NULL;
#endif
}

static int _dev_ext_release_udev(struct device *dev)
{
#ifdef UDEV_SYNC_SUPPORT
	if (!dev->ext.handle)
		return 1;

	/* udev_device_unref can't fail - it has no return value */
	udev_device_unref((struct udev_device *) dev->ext.handle);
	dev->ext.handle = NULL;
	return 1;
#else
	return 0;
#endif
}

static struct ext_registry_item _ext_registry[DEV_EXT_NUM] = {
	EXT_REGISTER(DEV_EXT_NONE, none),
	EXT_REGISTER(DEV_EXT_UDEV, udev)
};

const char *dev_ext_name(struct device *dev)
{
	return _ext_registry[dev->ext.src].name;
}

struct dev_ext *dev_ext_get(struct device *dev)
{
	struct dev_ext *ext;
	void *handle_ptr;

	handle_ptr = dev->ext.handle;

	if (!(ext = _ext_registry[dev->ext.src].dev_ext_get(dev)))
		log_error("%s: Failed to get external handle [%s].",
			   dev_name(dev), dev_ext_name(dev));
	else if (handle_ptr != dev->ext.handle)
		log_debug_devs("%s: External handle [%s:%p] attached", dev_name(dev),
				dev_ext_name(dev), dev->ext.handle);

	return ext;
}

int dev_ext_release(struct device *dev)
{
	int r;
	void *handle_ptr;

	if (!dev->ext.enabled ||
	    !dev->ext.handle)
		return 1;

	handle_ptr = dev->ext.handle;

	if (!(r = _ext_registry[dev->ext.src].dev_ext_release(dev)))
		log_error("%s: Failed to release external handle [%s:%p]",
			  dev_name(dev), dev_ext_name(dev), dev->ext.handle);
	else
		log_debug_devs("%s: External handle [%s:%p] detached",
				dev_name(dev), dev_ext_name(dev), handle_ptr);

	return r;
}

int dev_ext_enable(struct device *dev, dev_ext_t src)
{
	if (dev->ext.enabled && (dev->ext.src != src) && !dev_ext_release(dev)) {
		log_error("%s: Failed to enable external handle [%s].",
			   dev_name(dev), _ext_registry[src].name); 
		return 0;
	}

	dev->ext.src = src;
	dev->ext.enabled = 1;

	return 1;
}

int dev_ext_disable(struct device *dev)
{
	if (!dev->ext.enabled)
		return 1;

	if (!dev_ext_release(dev)) {
		log_error("%s: Failed to disable external handle [%s].",
			   dev_name(dev), dev_ext_name(dev));
		return 0;
	}

	dev->ext.enabled = 0;
	dev->ext.src = DEV_EXT_NONE;

	return 1;
}
