/*
 * test_bus.c
 *
 * Tests for bus related functions for the libsysfs testsuite
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
 * this will test the bus related functions provided by libsysfs.
 *
 * extern void sysfs_close_bus(struct sysfs_bus *bus);
 * extern struct sysfs_bus *sysfs_open_bus(const char *name);
 * extern struct sysfs_device *sysfs_get_bus_device(struct sysfs_bus *bus,
 * 						char *id);
 * extern struct sysfs_driver *sysfs_get_bus_driver(struct sysfs_bus *bus,
 * 						char *drvname);
 * extern struct dlist *sysfs_get_bus_drivers(struct sysfs_bus *bus);
 * extern struct dlist *sysfs_get_bus_devices(struct sysfs_bus *bus);
 ******************************************************************************
 */

#include "test-defs.h"
#include <errno.h>

/**
 * extern void sysfs_close_bus(struct sysfs_bus *bus);
 *
 * flags:
 * 	0 -> bus -> valid
 * 	1 -> bus -> null.
 */
int  test_sysfs_close_bus(int flag)
{
	struct sysfs_bus *bus = NULL;
	char *bus_name = NULL;

	switch (flag) {
	case 0:
		bus_name = val_bus_name;
		bus = sysfs_open_bus(bus_name);
		if (bus == NULL) {
			dbg_print("%s: sysfs_open_bus() failed\n",__FUNCTION__);
			return 0;
		}
		break;
	case 1:
		bus = NULL;
		break;
	default:
		return -1;
	}
	sysfs_close_bus(bus);

	dbg_print("%s: returns void\n", __FUNCTION__);
	return 0;
}

/**
 * extern struct sysfs_bus *sysfs_open_bus(const char *name);
 *
 * flag:
 * 	0 -	name -> valid
 * 	1 -	name -> invalid
 * 	2 - 	name -> null
 */
int test_sysfs_open_bus(int flag)
{
	struct sysfs_bus *bus = NULL;
	char *name = NULL;

	switch (flag) {
	case 0:
		name = val_bus_name;
		break;
	case 1:
		name = inval_name;
		break;
	case 2:
		name = NULL;
		break;
	default:
		return -1;
	}
	bus = sysfs_open_bus(name);

	switch (flag) {
	case 0:
		if (bus == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
					__FUNCTION__, flag);
			dbg_print("Bus = %s, path = %s\n\n",
					bus->name, bus->path);
		}
		break;
	case 1:
	case 2:
		if (bus != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
					__FUNCTION__, flag);
		break;
	default:
		return 0;
	}
	if (bus != NULL)
		sysfs_close_bus(bus);

	return 0;
}

/**
 * extern struct sysfs_device *sysfs_get_bus_device(struct sysfs_bus *bus,
 * 						char *id);
 *
 * flag:
 * 	0 	: bus -> valid, id -> valid
 * 	1 	: bus -> valid, id -> invalid
 * 	2 	: bus -> valid, id -> NULL
 * 	3 	: bus -> NULL, id -> valid
 * 	4 	: bus -> NULL, id -> invalid
 * 	5 	: bus -> NULL, id -> NULL
 */
int test_sysfs_get_bus_device(int flag)
{
	struct sysfs_bus *bus = NULL;
	struct sysfs_device *dev = NULL;
	char *bus_name = NULL;
	char *id = NULL;

	switch(flag) {
	case 0:
		bus_name = val_bus_name;
		bus = sysfs_open_bus(bus_name);
		if (bus == NULL) {
			dbg_print("%s: sysfs_open_bus() failed\n",__FUNCTION__);
			return 0;
		}
		id = val_bus_id;
		break;
	case 1:
		bus_name = val_bus_name;
		bus = sysfs_open_bus(bus_name);
		if (bus == NULL) {
			dbg_print("%s: sysfs_open_bus() failed\n",__FUNCTION__);
			return 0;
		}
		id = inval_name;
		break;
	case 2:
		bus_name = val_bus_name;
		bus = sysfs_open_bus(bus_name);
		if (bus == NULL) {
			dbg_print("%s: sysfs_open_bus() failed\n",__FUNCTION__);
			return 0;
		}
		id = NULL;
		break;
	case 3:
		bus = NULL;
		id = val_bus_id;
		break;
	case 4:
		bus = NULL;
		id = inval_name;
		break;
	case 5:
		bus = NULL;
		id = NULL;
		break;
	default:
		return -1;
	}
	dev = sysfs_get_bus_device(bus, id);

	switch (flag) {
	case 0:
		if (dev == NULL) {
			if (errno == 0)
				dbg_print("%s: Device %s not on bus %s\n",
						__FUNCTION__, id, bus_name);
			else
				dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
					__FUNCTION__, flag);
			show_device(dev);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		if (dev != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
					__FUNCTION__, flag);
		break;
	default:
		break;
	}

	if (bus != NULL)
		sysfs_close_bus(bus);

	return 0;
}

/**
 * extern struct sysfs_driver *sysfs_get_bus_driver(struct sysfs_bus *bus,
 * 						char *drvname);
 *
 * flag:
 * 	0 	: bus -> valid, drvname -> valid
 * 	1 	: bus -> valid, drvname -> invalid
 * 	2 	: bus -> valid, drvname -> NULL
 * 	3 	: bus -> NULL, drvname -> valid
 * 	4 	: bus -> NULL, drvname -> invalid
 * 	5 	: bus -> NULL, drvname -> NULL
 */
int test_sysfs_get_bus_driver(int flag)
{
	struct sysfs_bus *bus = NULL;
	struct sysfs_driver *drv = NULL;
	char *drvname = NULL;
	char *bus_name = NULL;

	switch(flag) {
	case 0:
		bus_name = val_drv_bus_name;
		bus = sysfs_open_bus(bus_name);
		if (bus == NULL) {
			dbg_print("%s: sysfs_open_bus() failed\n",__FUNCTION__);
			return 0;
		}
		drvname = val_drv_name;
		break;
	case 1:
		bus_name = val_drv_bus_name;
		bus = sysfs_open_bus(bus_name);
		if (bus == NULL) {
			dbg_print("%s: sysfs_open_bus() failed\n",__FUNCTION__);
			return 0;
		}
		drvname = inval_name;
		break;
	case 2:
		bus_name = val_drv_bus_name;
		bus = sysfs_open_bus(bus_name);
		if (bus == NULL) {
			dbg_print("%s: sysfs_open_bus() failed\n",__FUNCTION__);
			return 0;
		}
		drvname = NULL;
		break;
	case 3:
		bus = NULL;
		drvname = val_drv_name;
		break;
	case 4:
		bus = NULL;
		drvname = inval_name;
		break;
	case 5:
		bus = NULL;
		drvname = NULL;
		break;
	default:
		return -1;
	}
	drv = sysfs_get_bus_driver(bus, drvname);

	switch (flag) {
	case 0:
		if (drv == NULL) {
			if (errno == 0)
				dbg_print("%s: Driver %s not on bus %s\n",
						__FUNCTION__, drvname, bus_name);
			else
				dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
					__FUNCTION__, flag);
			show_driver(drv);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		if (drv != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
					__FUNCTION__, flag);
		break;
	default:
		break;
	}

	if (bus != NULL)
		sysfs_close_bus(bus);

	return 0;
}

/**
 * extern struct dlist *sysfs_get_bus_drivers(struct sysfs_bus *bus);
 *
 * flag:
 * 	0 	: bus -> valid
 * 	1 	: bus -> NULL
 */
int  test_sysfs_get_bus_drivers(int flag)
{
	struct sysfs_bus *bus = NULL;
	struct dlist *list = NULL;
	char *bus_name = NULL;

	switch (flag) {
	case 0:
		bus_name = val_bus_name;
		bus = sysfs_open_bus(bus_name);
		if (bus == NULL) {
			dbg_print("%s: sysfs_open_bus() failed\n",__FUNCTION__);
			return 0;
		}
		break;
	case 1:
		bus = NULL;
		break;
	default:
		return -1;
	}
	list = sysfs_get_bus_drivers(bus);

	switch (flag) {
	case 0:
		if (list == NULL) {
			if (errno == 0)
				dbg_print("%s: No drivers registered with bus %s\n",
						__FUNCTION__, bus_name);
			else
				dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
					__FUNCTION__, flag);
			show_driver_list(list);
			dbg_print("\n");
		}
		break;
	case 1:
		if (list != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
					__FUNCTION__, flag);
		break;
	default:
		return 0;
	}
	if (bus != NULL)
		sysfs_close_bus(bus);

	return 0;
}

/**
 * extern struct dlist *sysfs_get_bus_devices(struct sysfs_bus *bus);
 *
 * flag:
 * 	0 	: bus -> valid
 * 	1 	: bus -> NULL
 */
int  test_sysfs_get_bus_devices(int flag)
{
	struct sysfs_bus *bus = NULL;
	struct dlist *list = NULL;
	char *bus_name = NULL;

	switch (flag) {
	case 0:
		bus_name = val_bus_name;
		bus = sysfs_open_bus(bus_name);
		if (bus == NULL) {
			dbg_print("%s: sysfs_open_bus() failed\n",__FUNCTION__);
			return 0;
		}
		break;
	case 1:
		bus = NULL;
		break;
	default:
		return -1;
	}
	list = sysfs_get_bus_devices(bus);

	switch (flag) {
	case 0:
		if (list == NULL) {
			if (errno == 0)
				dbg_print("%s: No devices registered with bus %s\n",
						__FUNCTION__, bus_name);
			else
				dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
					__FUNCTION__, flag);
			show_device_list(list);
			dbg_print("\n");
		}
		break;
	case 1:
		if (list != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
					__FUNCTION__, flag);
		break;
	default:
		return 0;
	}
	if (bus != NULL)
		sysfs_close_bus(bus);

	return 0;
}
