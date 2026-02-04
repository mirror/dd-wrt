// SPDX-License-Identifier: GPL-2.0-only
/* usbreset -- send a USB port reset to a USB device */
/* Copyright (c) 2009-2016 Alan Stern */
/* To build:  gcc -o usbreset usbreset.c */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <dirent.h>
#include <sys/ioctl.h>

#include <linux/usbdevice_fs.h>

struct usbentry {
	int bus_num;
	int dev_num;
	int vendor_id;
	int product_id;
	char vendor_name[128];
	char product_name[128];
	char serial[128];
};

static char *sysfs_attr(const char *dev, const char *attr)
{
	int fd, len = 0;
	char path[PATH_MAX];
	static char buf[129];

	memset(buf, 0, sizeof(buf));
	snprintf(path, sizeof(path) - 1, "/sys/bus/usb/devices/%s/%s", dev, attr);

	fd = open(path, O_RDONLY);
	if (fd >= 0) {
		len = read(fd, buf, sizeof(buf) - 1);
		close(fd);
	}

	while (--len > 0 && isspace(buf[len]))
		buf[len] = 0;

	return (len >= 0) ? buf : NULL;
}

static struct usbentry *parse_devlist(DIR *d)
{
	char *attr;
	struct dirent *e;
	static struct usbentry dev;

	do {
		e = readdir(d);

		if (!e)
			return NULL;
	} while (!isdigit(e->d_name[0]) || strchr(e->d_name, ':'));

	memset(&dev, 0, sizeof(dev));
	dev.vendor_id = -1;
	dev.product_id = -1;

	attr = sysfs_attr(e->d_name, "busnum");
	if (attr)
		dev.bus_num = strtoul(attr, NULL, 10);

	attr = sysfs_attr(e->d_name, "devnum");
	if (attr)
		dev.dev_num = strtoul(attr, NULL, 10);

	attr = sysfs_attr(e->d_name, "idVendor");
	if (attr)
		dev.vendor_id = strtoul(attr, NULL, 16);

	attr = sysfs_attr(e->d_name, "idProduct");
	if (attr)
		dev.product_id = strtoul(attr, NULL, 16);

	attr = sysfs_attr(e->d_name, "manufacturer");
	if (attr)
		strncpy(dev.vendor_name, attr, sizeof(dev.vendor_name) - 1);

	attr = sysfs_attr(e->d_name, "product");
	if (attr)
		strncpy(dev.product_name, attr, sizeof(dev.product_name) - 1);

	attr = sysfs_attr(e->d_name, "serial");
	if (attr)
		strncpy(dev.serial, attr, sizeof(dev.serial) - 1);

	if (dev.bus_num && dev.dev_num && dev.vendor_id >= 0 && dev.product_id >= 0)
		return &dev;

	return NULL;
}

static void list_devices(void)
{
	DIR *devs = opendir("/sys/bus/usb/devices");
	struct usbentry *dev;
	int max_serial_length = 0;

	if (!devs)
		return;

	/* Calculate the largest size of the serial numbers if present */
	while ((dev = parse_devlist(devs)) != NULL) {
		int serial_length = strlen(dev->serial);
		if (serial_length > max_serial_length)
			max_serial_length = serial_length;
	}
	closedir(devs);

	devs = opendir("/sys/bus/usb/devices");
	if (!devs)
		return;

	while ((dev = parse_devlist(devs)) != NULL) {
		printf("  Number %03d/%03d  ID %04x:%04x  ", dev->bus_num, dev->dev_num, dev->vendor_id,
		       dev->product_id);
		if (strlen(dev->serial) > 0)
			printf("SN:%-*s", max_serial_length, dev->serial);
		else
			printf("   %*s", max_serial_length, "");
		printf("  %s\n", dev->product_name);
	}

	closedir(devs);
}

static struct usbentry *find_device(int *bus, int *dev, int *vid, int *pid, const char *serial, const char *product)
{
	DIR *devs = opendir("/sys/bus/usb/devices");

	struct usbentry *e, *match = NULL;

	if (!devs)
		return NULL;

	while ((e = parse_devlist(devs)) != NULL)
		if ((bus && (e->bus_num == *bus) && (e->dev_num == *dev)) ||
		    (vid && (e->vendor_id == *vid) && (e->product_id == *pid)) ||
		    (serial && !strcasecmp(e->serial, serial)) || (product && !strcasecmp(e->product_name, product))) {
			match = e;
			break;
		}

	closedir(devs);

	return match;
}

static void reset_device(struct usbentry *dev)
{
	int fd;
	char path[PATH_MAX];

	snprintf(path, sizeof(path) - 1, "/dev/bus/usb/%03d/%03d", dev->bus_num, dev->dev_num);

	printf("Resetting %s ... ", dev->product_name);

	fd = open(path, O_WRONLY);
	if (fd > -1) {
		if (ioctl(fd, USBDEVFS_RESET, 0) < 0)
			printf("failed [%s]\n", strerror(errno));
		else
			printf("ok\n");

		close(fd);
	} else {
		printf("can't open [%s]\n", strerror(errno));
	}
}

int main(int argc, char **argv)
{
	int id1, id2;
	struct usbentry *dev;

	if ((argc == 2) && (sscanf(argv[1], "%3d/%3d", &id1, &id2) == 2))
		dev = find_device(&id1, &id2, NULL, NULL, NULL, NULL);
	else if ((argc == 2) && (sscanf(argv[1], "%4x:%4x", &id1, &id2) == 2))
		dev = find_device(NULL, NULL, &id1, &id2, NULL, NULL);
	else if ((argc == 2) && (strncmp(argv[1], "SN:", 3) == 0))
		dev = find_device(NULL, NULL, NULL, NULL, &(argv[1])[3], NULL);
	else if ((argc == 2) && strlen(argv[1]) < 128)
		dev = find_device(NULL, NULL, NULL, NULL, NULL, argv[1]);
	else {
		printf("Usage:\n"
		       "  usbreset VVVV:PPPP - reset by vendor and product id\n"
		       "  usbreset BBB/DDD   - reset by bus and device number\n"
		       "  usbreset SN:SERIAL - reset by serial number\n"
		       "  usbreset \"Product\" - reset by product name\n\n"
		       "Devices:\n");
		list_devices();
		return 1;
	}

	if (!dev) {
		fprintf(stderr, "No such device found\n");
		return 1;
	}

	reset_device(dev);
	return 0;
}
