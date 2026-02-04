// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * USB name database manipulation routines
 *
 * Copyright (C) 1999, 2000 Thomas Sailer (sailer@ife.ee.ethz.ch)
 * Copyright (C) 2013 Tom Gundersen (teg@jklm.no)
 */
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#include <libusb.h>
#include <libudev.h>

#include "usb-spec.h"
#include "names.h"
#include "sysfs.h"


/* ---------------------------------------------------------------------- */

static struct udev *udev = NULL;
static struct udev_hwdb *hwdb = NULL;

/* ---------------------------------------------------------------------- */

static const char *names_genericstrtable(const struct genericstrtable *t,
					 unsigned int idx)
{
	for (; t->name; t++)
		if (t->num == idx)
			return t->name;
	return NULL;
}

const char *names_hid(uint8_t hidd)
{
	return names_genericstrtable(hiddescriptors, hidd);
}

const char *names_reporttag(uint8_t rt)
{
	return names_genericstrtable(reports, rt);
}

const char *names_huts(unsigned int data)
{
	return names_genericstrtable(huts, data);
}

const char *names_hutus(unsigned int data)
{
	return names_genericstrtable(hutus, data);
}

const char *names_langid(uint16_t langid)
{
	return names_genericstrtable(langids, langid);
}

const char *names_physdes(uint8_t ph)
{
	return names_genericstrtable(physdess, ph);
}

const char *names_bias(uint8_t b)
{
	return names_genericstrtable(biass, b);
}

const char *names_countrycode(unsigned int countrycode)
{
	return names_genericstrtable(countrycodes, countrycode);
}

static const char *hwdb_get(const char *modalias, const char *key)
{
	struct udev_list_entry *entry;

	udev_list_entry_foreach(entry, udev_hwdb_get_properties_list_entry(hwdb, modalias, 0))
		if (strcmp(udev_list_entry_get_name(entry), key) == 0)
			return udev_list_entry_get_value(entry);

	return NULL;
}

const char *names_vendor(uint16_t vendorid)
{
	char modalias[64];

	sprintf(modalias, "usb:v%04X*", vendorid);
	return hwdb_get(modalias, "ID_VENDOR_FROM_DATABASE");
}

const char *names_product(uint16_t vendorid, uint16_t productid)
{
	char modalias[64];

	sprintf(modalias, "usb:v%04Xp%04X*", vendorid, productid);
	return hwdb_get(modalias, "ID_MODEL_FROM_DATABASE");
}

const char *names_class(uint8_t classid)
{
	char modalias[64];

	sprintf(modalias, "usb:v*p*d*dc%02X*", classid);
	return hwdb_get(modalias, "ID_USB_CLASS_FROM_DATABASE");
}

const char *names_subclass(uint8_t classid, uint8_t subclassid)
{
	char modalias[64];

	sprintf(modalias, "usb:v*p*d*dc%02Xdsc%02X*", classid, subclassid);
	return hwdb_get(modalias, "ID_USB_SUBCLASS_FROM_DATABASE");
}

const char *names_protocol(uint8_t classid, uint8_t subclassid, uint8_t protocolid)
{
	char modalias[64];

	sprintf(modalias, "usb:v*p*d*dc%02Xdsc%02Xdp%02X*", classid, subclassid, protocolid);
	return hwdb_get(modalias, "ID_USB_PROTOCOL_FROM_DATABASE");
}

const char *names_audioterminal(uint16_t termt)
{
	const struct audioterminal *at;

	for (at = audioterminals; at->name; at++)
		if (at->termt == termt)
			return at->name;
	return NULL;
}

const char *names_videoterminal(uint16_t termt)
{
	const struct videoterminal *vt;

	for (vt = videoterminals; vt->name; vt++)
		if (vt->termt == termt)
			return vt->name;
	return NULL;
}

/* ---------------------------------------------------------------------- */

int get_vendor_string(char *buf, size_t size, uint16_t vid)
{
        const char *cp;

        if (size < 1)
                return 0;
        *buf = 0;
        if (!(cp = names_vendor(vid)))
		return 0;
        return snprintf(buf, size, "%s", cp);
}

int get_product_string(char *buf, size_t size, uint16_t vid, uint16_t pid)
{
        const char *cp;

        if (size < 1)
                return 0;
        *buf = 0;
        if (!(cp = names_product(vid, pid)))
		return 0;
        return snprintf(buf, size, "%s", cp);
}

int get_class_string(char *buf, size_t size, uint8_t cls)
{
	const char *cp;

	if (size < 1)
		return 0;
	*buf = 0;
	if (!(cp = names_class(cls)))
		return snprintf(buf, size, "[unknown]");
	return snprintf(buf, size, "%s", cp);
}

int get_subclass_string(char *buf, size_t size, uint8_t cls, uint8_t subcls)
{
	const char *cp;

	if (size < 1)
		return 0;
	*buf = 0;
	if (!(cp = names_subclass(cls, subcls)))
		return snprintf(buf, size, "[unknown]");
	return snprintf(buf, size, "%s", cp);
}

/*
 * Attempt to get friendly vendor and product names from the udev hwdb. If
 * either or both are not present, instead populate those from the device's
 * own string descriptors.
 */
void get_vendor_product_with_fallback(char *vendor, int vendor_len,
				      char *product, int product_len,
				      libusb_device *dev)
{
	struct libusb_device_descriptor desc;
	char sysfs_name[PATH_MAX];
	bool have_vendor, have_product;

	libusb_get_device_descriptor(dev, &desc);

	/* set to "[unknown]" by default unless something below finds a string */
	strncpy(vendor, "[unknown]", vendor_len);
	strncpy(product, "[unknown]", product_len);

	have_vendor = !!get_vendor_string(vendor, vendor_len, desc.idVendor);
	have_product = !!get_product_string(product, product_len,
			desc.idVendor, desc.idProduct);

	if (have_vendor && have_product)
		return;

	if (get_sysfs_name(sysfs_name, sizeof(sysfs_name), dev) >= 0) {
		if (!have_vendor)
			read_sysfs_prop(vendor, vendor_len, sysfs_name, "manufacturer");
		if (!have_product)
			read_sysfs_prop(product, product_len, sysfs_name, "product");
	}
}

int names_init(void)
{
	udev = udev_new();
	if (!udev)
		return -1;

	hwdb = udev_hwdb_new(udev);
	return hwdb ? 0 : -1;
}

void names_exit(void)
{
	hwdb = udev_hwdb_unref(hwdb);
	udev = udev_unref(udev);
}
