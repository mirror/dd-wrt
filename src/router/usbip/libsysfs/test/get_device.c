/*
 * get_device.c
 *
 * Utility to get details of a given device
 *
 * Copyright (C) IBM Corp. 2003-2005
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

#include <stdio.h>
#include <stdlib.h>

#include "libsysfs.h"

static void print_usage(void)
{
        fprintf(stdout, "Usage: get_device [bus] [device]\n");
}

int main(int argc, char *argv[])
{
	struct sysfs_device *device = NULL;

	if (argc != 3) {
		print_usage();
		return 1;
	}

	device = sysfs_open_device(argv[1], argv[2]);
	if (device == NULL) {
		fprintf(stdout, "Device \"%s\" not found on bus \"%s\"\n",
					argv[2], argv[1]);
		return 1;
	}
	fprintf(stdout, "device is on bus %s, using driver %s\n",
				device->bus, device->driver_name);

	struct sysfs_device *parent = sysfs_get_device_parent(device);
	if (parent)
		fprintf(stdout, "parent is %s\n", parent->name);
	else
		fprintf(stdout, "no parent\n");
	return 0;
}

