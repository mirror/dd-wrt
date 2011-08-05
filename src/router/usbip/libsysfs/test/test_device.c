/*
 * test_root.c
 *
 * Tests for device/root device related functions for the libsysfs testsuite
 *
 * Copyright (C) IBM Corp. 2004-2005
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/**
 ******************************************************************************
 * this will test the device/root device related functions provided by libsysfs
 *
 * extern void sysfs_close_device(struct sysfs_device *dev);
 * extern struct sysfs_device *sysfs_open_device
 * 		(const char *bus, const char *bus_id);
 * extern struct sysfs_device *sysfs_get_device_parent
 * 					(struct sysfs_device *dev);
 * extern struct sysfs_device *sysfs_open_device_path
 * 					(const char *path);
 * extern struct sysfs_attribute *sysfs_get_device_attr
 * 			(struct sysfs_device *dev, const char *name);
 * extern struct dlist *sysfs_get_device_attributes
 * 					(struct sysfs_device *device);
 ******************************************************************************
 */

#include "test-defs.h"
#include <errno.h>

/**
 * extern void sysfs_close_device(struct sysfs_device *dev);
 *
 * flag:
 * 	0:	dev -> valid
 * 	1:	dev -> NULL
 */
int test_sysfs_close_device(int flag)
{
	struct sysfs_device *dev = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_dev_path;
		dev = sysfs_open_device_path(path);
		if (dev == NULL) {
			dbg_print("%s: failed to open device at %s\n",
						__FUNCTION__, path);
			return 0;
		}
		break;
	case 1:
		dev = NULL;
		break;
	default:
		return -1;
	}
	sysfs_close_device(dev);

	dbg_print("%s: returns void\n", __FUNCTION__);
	return 0;
}

/**
 * extern struct sysfs_device *sysfs_open_device
 * 		(const char *bus, const char *bus_id);
 *
 * flag:
 * 	0:	bus -> valid, bus_id -> valid
 * 	1:	bus -> valid, bus_id -> invalid
 * 	2:	bus -> valid, bus_id -> invalid
 * 	3:	bus -> invalid, bus_id -> valid
 * 	4:	bus -> invalid, bus_id -> invalid
 * 	5:	bus -> invalid, bus_id -> invalid
 * 	6:	bus -> NULL, bus_id -> valid
 * 	7:	bus -> NULL, bus_id -> invalid
 * 	8:	bus -> NULL, bus_id -> invalid
 */
int test_sysfs_open_device(int flag)
{
	struct sysfs_device *dev = NULL;
	char *bus = NULL;
	char *bus_id = NULL;

	switch (flag) {
	case 0:
		bus = val_bus_name;
		bus_id = val_bus_id;
		break;
	case 1:
		bus = val_bus_name;
		bus_id = inval_name;
		break;
	case 2:
		bus = val_bus_name;
		bus_id = NULL;
		break;
	case 3:
		bus = inval_name;
		bus_id = val_bus_id;
		break;
	case 4:
		bus = inval_name;
		bus_id = inval_name;
		break;
	case 5:
		bus = inval_name;
		bus_id = NULL;
		break;
	case 6:
		bus = NULL;
		bus_id = val_bus_id;
		break;
	case 7:
		bus = NULL;
		bus_id = inval_name;
		break;
	case 8:
		bus = NULL;
		bus_id = NULL;
		break;
	default:
		return -1;
	}
	dev = sysfs_open_device(bus, bus_id);

	switch (flag) {
	case 0:
		if (dev == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_device(dev);
			dbg_print("\n");
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		if (dev == NULL)
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		else
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		break;
	}

	if (dev != NULL)
		sysfs_close_device(dev);
	return 0;
}

/**
 * extern struct sysfs_device *sysfs_get_device_parent
 * 					(struct sysfs_device *dev);
 *
 * flag:
 * 	0:	dev -> valid
 * 	1:	dev -> NULL
 */
int test_sysfs_get_device_parent(int flag)
{
	struct sysfs_device *pdev = NULL;
	struct sysfs_device *dev = NULL;
	char *dev_path = NULL;

	switch (flag) {
	case 0:
		dev_path = val_dev_path;
		dev = sysfs_open_device_path(dev_path);
		if (dev == NULL) {
			dbg_print("%s: failed to open device at %s\n",
					__FUNCTION__, dev_path);
			return 0;
		}
		break;
	case 1:
		dev = NULL;
		break;
	default:
		return -1;
	}
	pdev = sysfs_get_device_parent(dev);

	switch (flag) {
	case 0:
		if (pdev == NULL) {
			if (errno == 0)
				dbg_print("%s: Device at %s does not have a parent\n",
						__FUNCTION__, dev_path);
			else
				dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_device(pdev);
			dbg_print("\n");
		}
		break;
	case 1:
		if (pdev == NULL)
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		else
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
	default:
		break;
	}
	if (dev != NULL) {
		sysfs_close_device(dev);
	}
	return 0;
}

/**
 * extern struct sysfs_device *sysfs_open_device_path
 * 					(const char *path);
 *
 * flag:
 * 	0:	path -> valid
 * 	1:	path -> invalid
 * 	2:	path -> NULL
 *
 */
int test_sysfs_open_device_path(int flag)
{
	struct sysfs_device *dev = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_dev_path;
		break;
	case 1:
		path = inval_path;
		break;
	case 2:
		path = NULL;
		break;
	default:
		return -1;
	}
	dev = sysfs_open_device_path(path);

	switch (flag) {
	case 0:
		if (dev == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_device(dev);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
		if (dev == NULL)
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		else
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
	default:
		break;
	}
	if (dev != NULL)
		sysfs_close_device(dev);
	return 0;
}

/**
 * extern struct sysfs_attribute *sysfs_get_device_attr
 * 		(struct sysfs_device *dev, const char *name);
 *
 * flag:
 * 	0:	dev -> valid, name -> valid
 * 	1:	dev -> valid, name -> invalid
 * 	2:	dev -> valid, name -> NULL
 * 	3:	dev -> NULL, name -> valid
 * 	4:	dev -> NULL, name -> invalid
 * 	5:	dev -> NULL, name -> NULL
 */
int test_sysfs_get_device_attr(int flag)
{
	struct sysfs_device *dev = NULL;
	char *name = NULL;
	char *path = NULL;
	struct sysfs_attribute *attr = NULL;

	switch (flag) {
	case 0:
		path = val_dev_path;
		dev = sysfs_open_device_path(path);
		if (dev == NULL) {
			dbg_print("%s: failed to open device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		name = val_dev_attr;
		break;
	case 1:
		dev = sysfs_open_device_path(path);
		name = inval_name;
		break;
	case 2:
		dev = sysfs_open_device_path(path);
		name = NULL;
		break;
	case 3:
		dev = NULL;
		name = val_dev_attr;
		break;
	case 4:
		dev = NULL;
		name = inval_name;
		break;
	case 5:
		dev = NULL;
		name = NULL;
		break;
	default:
		return -1;
	}
	attr = sysfs_get_device_attr(dev, name);

	switch (flag) {
	case 0:
		if (attr == NULL) {
			if (errno == EACCES)
				dbg_print("%s: attribute %s does not support READ\n",
						__FUNCTION__, name);
			else if (errno == ENOENT)
				dbg_print("%s: attribute %s not defined for device at %s\n",
						__FUNCTION__, name, path);
			else if (errno == 0)
				dbg_print("%s: device at %s does not export attributes\n",
						__FUNCTION__, path);
			else
				dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_attribute(attr);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		if (attr != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			if (errno == EINVAL)
				dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
			else
				dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		}
	default:
		break;
	}

	if (dev != NULL)
		sysfs_close_device(dev);
	return 0;
}

/**
 * extern struct dlist *sysfs_get_device_attributes
 * 					(struct sysfs_device *device);
 *
 * flag:
 * 	0:	device -> valid
 * 	1:	device -> NULL
 */
int test_sysfs_get_device_attributes(int flag)
{
	struct sysfs_device *device = NULL;
	struct dlist *list = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_dev_path;
		device = sysfs_open_device_path(path);
		if (device == NULL) {
			dbg_print("%s: failed to open device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		break;
	case 1:
		device = NULL;
		break;
	default:
		return -1;
	}
	list = sysfs_get_device_attributes(device);

	switch (flag) {
	case 0:
		if (list == NULL) {
			if (errno == 0)
				dbg_print("%s: device at %s does not export attributes\n",
						__FUNCTION__, path);
			else
				dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_attribute_list(list);
			dbg_print("\n");
		}
		break;
	case 1:
		if (errno != EINVAL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
	default:
		break;
	}
	if (device != NULL)
		sysfs_close_device(device);
	return 0;
}
