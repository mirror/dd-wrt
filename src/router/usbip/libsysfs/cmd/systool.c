/*
 * systool.c
 *
 * Sysfs utility to list buses, classes, and devices
 *
 * Copyright (C) IBM Corp. 2003-2005
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include "libsysfs.h"
#include "names.h"

#define safestrcpy(to, from)	strncpy(to, from, sizeof(to)-1)
#define safestrcat(to, from)	strncat(to, from, sizeof(to) - strlen(to)-1)

#define safestrcpymax(to, from, max) \
do { \
	to[max-1] = '\0'; \
	strncpy(to, from, max-1); \
} while (0)

#define safestrcatmax(to, from, max) \
do { \
	to[max-1] = '\0'; \
	strncat(to, from, max - strlen(to)-1); \
} while (0)

/* Command Options */
static int show_options = 0;		/* bitmask of show options */
static char *attribute_to_show = NULL;	/* show value for this attribute */
static char *device_to_show = NULL;	/* show only this bus device */
static char sysfs_mnt_path[SYSFS_PATH_MAX]; /* sysfs mount point */
struct pci_access *pacc = NULL;
char *show_bus = NULL;

static void show_device(struct sysfs_device *device, int level);
static void show_class_device(struct sysfs_class_device *dev, int level);

#define SHOW_ATTRIBUTES		0x01	/* show attributes command option */
#define SHOW_ATTRIBUTE_VALUE	0x02	/* show an attribute value option */
#define SHOW_DEVICES		0x04	/* show only devices option */
#define SHOW_DRIVERS		0x08	/* show only drivers option */
#define SHOW_ALL_ATTRIB_VALUES	0x10	/* show all attributes with values */
#define SHOW_CHILDREN		0x20	/* show device children */
#define SHOW_PARENT		0x40	/* show device parent */
#define SHOW_PATH		0x80	/* show device/driver path */
                                                                                
#define SHOW_ALL		0xff

static char cmd_options[] = "aA:b:c:dDhm:pP:v";

/*
 * binary_files - defines existing sysfs binary files. These files will be
 * printed in hex.
 */
static char *binary_files[] = {
	"config",
	"data"
};

static int binfiles = 2;

static unsigned int get_pciconfig_word(int offset, unsigned char *buf)
{
        unsigned short val = (unsigned char)buf[offset] |
	                ((unsigned char)buf[offset+1] << 8);
        return val;
}

/**
 * usage: prints utility usage.
 */
static void usage(void)
{
	fprintf(stdout, "Usage: systool [<options> [device]]\n");
	fprintf(stdout, "\t-a\t\t\tShow attributes\n");
	fprintf(stdout, "\t-b <bus_name>\t\tShow a specific bus\n");
	fprintf(stdout, "\t-c <class_name>\t\tShow a specific class\n");
	fprintf(stdout, "\t-d\t\t\tShow only devices\n");
	fprintf(stdout, "\t-h\t\t\tShow usage\n");
	fprintf(stdout, "\t-m <module_name>\tShow a specific module\n");
	fprintf(stdout, "\t-p\t\t\tShow path to device/driver\n");
	fprintf(stdout, "\t-v\t\t\tShow all attributes with values\n");
	fprintf(stdout, "\t-A <attribute_name>\tShow attribute value\n");
	fprintf(stdout, "\t-D\t\t\tShow only drivers\n");
	fprintf(stdout, "\t-P\t\t\tShow device's parent\n");
}

/**
 * indent: called before printing a line, it adds indent to the line up to
 *	level passed in.
 * @level: number of spaces to indent.
 */
static void indent(int level)
{
	int i;

	for (i = 0; i < level; i++)
		fprintf(stdout, " ");
}

/**
 * remove_end_newline: removes newline on the end of an attribute value
 * @value: string to remove newline from
 */
static void remove_end_newline(char *value)
{
	char *p = value + (strlen(value) - 1);

	if (p && *p == '\n')
		*p = '\0';
}

/**
 * isbinaryvalue: checks to see if attribute is binary or not.
 * @attr: attribute to check.
 * returns 1 if binary, 0 if not.
 */
static int isbinaryvalue(struct sysfs_attribute *attr)
{
	int i;

	if (!attr || !attr->value) 
		return 0;

	for (i = 0; i < binfiles; i++) 
		if ((strcmp(attr->name, binary_files[i])) == 0)
			return 1;

	return 0;
}

/**
 * show_attribute_value: prints out single attribute value.
 * @attr: attricute to print.
 */
static void show_attribute_value(struct sysfs_attribute *attr, int level)
{
	if (!attr)
		return;

	if (attr->method & SYSFS_METHOD_SHOW) {
		if (isbinaryvalue(attr)) {
			int i;
			for (i = 0; i < attr->len; i++) {
				if (!(i % 16) && (i != 0)) {
					fprintf(stdout, "\n");
					indent(level+22);
				} else if (!(i % 8) && (i != 0))
					fprintf(stdout, " ");
				fprintf(stdout, " %02x", 
					(unsigned char)attr->value[i]);
			}
			fprintf(stdout, "\n");

		} else if (attr->value && strlen(attr->value) > 0) {
			remove_end_newline(attr->value);
			fprintf(stdout, "\"%s\"\n", attr->value);
		} else
			fprintf(stdout, "\n");
	} else {
		fprintf(stdout, "<store method only>\n");
	}
}

/**
 * show_attribute: prints out a single attribute
 * @attr: attribute to print.
 */
static void show_attribute(struct sysfs_attribute *attr, int level)
{
	if (!attr) 
		return;

	if (show_options & SHOW_ALL_ATTRIB_VALUES) {
		indent(level);
		fprintf(stdout, "%-20s= ", attr->name);
		show_attribute_value(attr, level);
	} else if ((show_options & SHOW_ATTRIBUTES) || ((show_options 
	    & SHOW_ATTRIBUTE_VALUE) && (strcmp(attr->name, attribute_to_show) 
	    == 0))) {
		indent(level);
		fprintf (stdout, "%-20s", attr->name);
		if (show_options & SHOW_ATTRIBUTE_VALUE && attr->value 
		    != NULL && (strcmp(attr->name, attribute_to_show)) == 0) {
			fprintf(stdout, "= ");
			show_attribute_value(attr, level);
		} else 
			fprintf(stdout, "\n");
	}
}

/**
 * show_attributes: prints out a list of attributes.
 * @attributes: print this dlist of attributes/files.
 */
static void show_attributes(struct dlist *attributes, int level)
{
	if (attributes) {
		struct sysfs_attribute *cur;
		
		dlist_for_each_data(attributes, cur, 
				struct sysfs_attribute) {
			show_attribute(cur, level);
		}
	}
}

/**
 * show_device_parent: prints device's parent (if present)
 * @device: sysfs_device whose parent information is needed
 */
static void show_device_parent(struct sysfs_device *device, int level)
{
	struct sysfs_device *parent;

	parent = sysfs_get_device_parent(device);
	if (parent) {
		fprintf(stdout, "\n");
		indent(level);
		fprintf(stdout, "Device \"%s\"'s parent\n", device->name);
		show_device(parent, (level+2));
	}
}
		
/**
 * show_device: prints out device information.
 * @device: device to print.
 */
static void show_device(struct sysfs_device *device, int level)
{
	struct dlist *attributes;
        unsigned int vendor_id, device_id;
        char buf[128], value[256], path[SYSFS_PATH_MAX];
	
	if (device) {
		indent(level);
		if (show_bus && (!(strcmp(show_bus, "pci")))) {
			fprintf(stdout, "%s ", device->bus_id);
			memset(path, 0, SYSFS_PATH_MAX);
			memset(value, 0, SYSFS_PATH_MAX);
			safestrcpy(path, device->path);
			safestrcat(path, "/config");
			struct sysfs_attribute *attr;
			attr = sysfs_open_attribute(path);
			if (attr) {
				if (!sysfs_read_attribute(attr)) {
					vendor_id = get_pciconfig_word
						(PCI_VENDOR_ID, attr->value);
					device_id = get_pciconfig_word
						(PCI_DEVICE_ID, attr->value);
					fprintf(stdout, "%s\n",
						pci_lookup_name(pacc, buf, 128,
						PCI_LOOKUP_VENDOR | 
						PCI_LOOKUP_DEVICE,
						vendor_id, device_id, 0, 0));
				}
				sysfs_close_attribute(attr);
			} else 
				fprintf(stdout, "\n");
		} else 
			fprintf(stdout, "Device = \"%s\"\n", device->bus_id);

		if (show_options & (SHOW_PATH | SHOW_ALL_ATTRIB_VALUES)) {
			indent(level);
			fprintf(stdout, "Device path = \"%s\"\n", 
							device->path);
		}

		if (show_options & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE |
					SHOW_ALL_ATTRIB_VALUES)) {
			attributes = sysfs_get_device_attributes(device);
			if (attributes) 
				show_attributes(attributes, (level+2));
		}

		if ((device_to_show) && (show_options & SHOW_PARENT)) {
			show_options &= ~SHOW_PARENT;
			show_device_parent(device, (level+2));
		}
		if (show_options ^ SHOW_DEVICES)
			if (!(show_options & SHOW_DRIVERS))
				fprintf(stdout, "\n");
	}
}

/**
 * show_driver_attributes: prints out driver attributes .
 * @driver: print this driver's attributes.
 */
static void show_driver_attributes(struct sysfs_driver *driver, int level)
{
	if (driver) {
		struct dlist *attributes;
	
		attributes = sysfs_get_driver_attributes(driver);
		if (attributes) {
			struct sysfs_attribute *cur;

			dlist_for_each_data(attributes, cur,
					struct sysfs_attribute) {
				show_attribute(cur, (level));
			}
			fprintf(stdout, "\n");
		}
	}
}

/**
 * show_driver: prints out driver information.
 * @driver: driver to print.
 */
static void show_driver(struct sysfs_driver *driver, int level)
{
	struct dlist *devlist;
	
	if (driver) {
		indent(level);
		fprintf(stdout, "Driver = \"%s\"\n", driver->name);
		if (show_options & (SHOW_PATH | SHOW_ALL_ATTRIB_VALUES)) {
			indent(level);
			fprintf(stdout, "Driver path = \"%s\"\n", 
							driver->path);
		}
		if (show_options & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
			    | SHOW_ALL_ATTRIB_VALUES))
			show_driver_attributes(driver, (level+2));
		devlist = sysfs_get_driver_devices(driver);
		if (devlist) {
			struct sysfs_device *cur;
			
			indent(level+2);
			fprintf(stdout, "Devices using \"%s\" are:\n", 
								driver->name);
			dlist_for_each_data(devlist, cur, 
					struct sysfs_device) {
				if (show_options & SHOW_DRIVERS) {
					show_device(cur, (level+4));
					fprintf(stdout, "\n");
				} else {
					indent(level+4);
					fprintf(stdout, "\"%s\"\n", cur->name);
				}
			}
		} 
		fprintf(stdout, "\n");
	}
}

/**
 * show_sysfs_bus: prints out everything on a bus.
 * @busname: bus to print.
 * returns 0 with success or 1 with error.
 */
static int show_sysfs_bus(char *busname)
{
	struct sysfs_bus *bus;
	struct sysfs_device *curdev;
	struct sysfs_driver *curdrv;
	struct dlist *devlist;
	struct dlist *drvlist;

	if (!busname) {
		errno = EINVAL;
		return 1;
	}
	bus = sysfs_open_bus(busname);
	if (bus == NULL) {
		fprintf(stderr, "Error opening bus %s\n", busname);
		return 1;
	}

	fprintf(stdout, "Bus = \"%s\"\n", busname);
	if (show_options ^ (SHOW_DEVICES | SHOW_DRIVERS))
		fprintf(stdout, "\n");
	if (show_options & SHOW_DEVICES) {
		devlist = sysfs_get_bus_devices(bus);
		if (devlist) {
			dlist_for_each_data(devlist, curdev, 
						struct sysfs_device) {
				if (!device_to_show || (strcmp(device_to_show,
							curdev->bus_id) == 0)) 
					show_device(curdev, 2);
			}
		}
	}
	if (show_options & SHOW_DRIVERS) {
		drvlist = sysfs_get_bus_drivers(bus);
		if (drvlist) {
			dlist_for_each_data(drvlist, curdrv, 
					struct sysfs_driver) {
				show_driver(curdrv, 2);
			}
		}
	}
	sysfs_close_bus(bus);
	return 0;
}

/**
 * show_classdev_parent: prints the class device's parent if present
 * @dev: class device whose parent is needed
 */ 
static void show_classdev_parent(struct sysfs_class_device *dev, int level)
{
	struct sysfs_class_device *parent;

	parent = sysfs_get_classdev_parent(dev);
	if (parent) {
		fprintf(stdout, "\n");
		indent(level);
		fprintf(stdout, "Class device \"%s\"'s parent is\n", 
								dev->name);
		show_class_device(parent, level+2);
	}
}

/**
 * show_class_device: prints out class device.
 * @dev: class device to print.
 */
static void show_class_device(struct sysfs_class_device *dev, int level)
{
	struct dlist *attributes;
	struct sysfs_device *device;
	
	if (dev) {
		indent(level);
		fprintf(stdout, "Class Device = \"%s\"\n", dev->name);
		if (show_options & (SHOW_PATH | SHOW_ALL_ATTRIB_VALUES)) {
			indent(level);
			fprintf(stdout, "Class Device path = \"%s\"\n",
								dev->path);
		}
		if (show_options & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
		    | SHOW_ALL_ATTRIB_VALUES)) {
			attributes = sysfs_get_classdev_attributes(dev);
			if (attributes)
				show_attributes(attributes, (level+2));
			fprintf(stdout, "\n");
		}
		if (show_options & (SHOW_DEVICES | SHOW_ALL_ATTRIB_VALUES)) {
			device = sysfs_get_classdev_device(dev);
			if (device) {
				show_device(device, (level+2));
			}
		}
		if ((device_to_show) && (show_options & SHOW_PARENT)) {
			show_options &= ~SHOW_PARENT;
			show_classdev_parent(dev, level+2);
		}
		if (show_options & ~(SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
		    | SHOW_ALL_ATTRIB_VALUES))
			fprintf(stdout, "\n");
	}
}

/**
 * show_sysfs_class: prints out sysfs class and all its devices.
 * @classname: class to print.
 * returns 0 with success and 1 with error.
 */
static int show_sysfs_class(char *classname)
{
	struct sysfs_class *cls;
	struct sysfs_class_device *cur;
	struct dlist *clsdevlist;

	if (!classname) {
		errno = EINVAL;
		return 1;
	}
	cls = sysfs_open_class(classname);
	if (cls == NULL) {
		fprintf(stderr, "Error opening class %s\n", classname);
		return 1;
	}
	fprintf(stdout, "Class = \"%s\"\n\n", classname);
	clsdevlist = sysfs_get_class_devices(cls);
	if (clsdevlist) {
		dlist_for_each_data(clsdevlist, cur, 
				struct sysfs_class_device) {
			if (device_to_show == NULL || (strcmp(device_to_show,
			    cur->name) == 0))
				show_class_device(cur, 2);
		}
	}

	sysfs_close_class(cls);
	return 0;
}

static int show_sysfs_module(char *module)
{
	struct sysfs_module *mod = NULL;

	if (!module) {	
		errno = EINVAL;
		return 1;
	}

	mod = sysfs_open_module(module);
	if (mod == NULL) {
		fprintf(stderr, "Error opening module %s\n", module);
		return 1;
	}
	fprintf(stdout, "Module = \"%s\"\n\n", module);
	if (show_options & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
	    | SHOW_ALL_ATTRIB_VALUES)) {
		struct dlist *attributes = NULL;
		struct sysfs_attribute *cur;

		attributes = sysfs_get_module_attributes(mod);
		if (attributes) {
			if (show_options & (SHOW_ATTRIBUTES
			    | SHOW_ALL_ATTRIB_VALUES)) {
				indent(2);
				fprintf(stdout, "Attributes:\n");
			}
			dlist_for_each_data(attributes, cur,
					struct sysfs_attribute) {
				show_attribute(cur, (4));
			}
		}
		attributes = sysfs_get_module_parms(mod);
		if (attributes) {
			if (show_options & (SHOW_ATTRIBUTES 
			    | SHOW_ALL_ATTRIB_VALUES)) {
				fprintf(stdout, "\n");
				indent(2);
				fprintf(stdout, "Parameters:\n");
			}
			dlist_for_each_data(attributes, cur,
					struct sysfs_attribute) {
				show_attribute(cur, (4));
			}
		}
		attributes = sysfs_get_module_sections(mod);
		if (attributes) {
			if (show_options & (SHOW_ATTRIBUTES
			    | SHOW_ALL_ATTRIB_VALUES)) {
				fprintf(stdout, "\n");
				indent(2);
				fprintf(stdout, "Sections:\n");
			}
			dlist_for_each_data(attributes, cur,
					struct sysfs_attribute) {
				show_attribute(cur, (4));
			}
			fprintf(stdout, "\n");
		}
	}

	sysfs_close_module(mod);
	return 0;
}

/**
 * show_default_info: prints current buses, classes, and root devices
 *	supported by sysfs.
 * returns 0 with success or 1 with error.
 */
static int show_default_info(void)
{
	char subsys[SYSFS_NAME_LEN];
	struct dlist *list;
	char *cur;
	int retval = 0;

	safestrcpy(subsys, sysfs_mnt_path);
	safestrcat(subsys, "/");
	safestrcat(subsys, SYSFS_BUS_NAME);
	list = sysfs_open_directory_list(subsys);
	if (list) {
		fprintf(stdout, "Supported sysfs buses:\n");
		dlist_for_each_data(list, cur, char)
			fprintf(stdout, "\t%s\n", cur);
		sysfs_close_list(list);
	}

	safestrcpy(subsys, sysfs_mnt_path);
	safestrcat(subsys, "/");
	safestrcat(subsys, SYSFS_CLASS_NAME);
	list = sysfs_open_directory_list(subsys);
	if (list) {
		fprintf(stdout, "Supported sysfs classes:\n");
		dlist_for_each_data(list, cur, char)
			fprintf(stdout, "\t%s\n", cur);
		sysfs_close_list(list);
	}

	safestrcpy(subsys, sysfs_mnt_path);
	safestrcat(subsys, "/");
	safestrcat(subsys, SYSFS_DEVICES_NAME);
	list = sysfs_open_directory_list(subsys);
	if (list) {
		fprintf(stdout, "Supported sysfs devices:\n");
		dlist_for_each_data(list, cur, char)
			fprintf(stdout, "\t%s\n", cur);
		sysfs_close_list(list);
	}
			
	safestrcpy(subsys, sysfs_mnt_path);
	safestrcat(subsys, "/");
	safestrcat(subsys, SYSFS_MODULE_NAME);
	list = sysfs_open_directory_list(subsys);
	if (list) {
		fprintf(stdout, "Supported sysfs modules:\n");
		dlist_for_each_data(list, cur, char)
			fprintf(stdout, "\t%s\n", cur);
		sysfs_close_list(list);
	}

	return retval;
}
	
/**
 * check_sysfs_mounted: Checks to see if sysfs is mounted.
 * returns 0 if not and 1 if true.
 */
static int check_sysfs_is_mounted(void)
{
	if (sysfs_get_mnt_path(sysfs_mnt_path, SYSFS_PATH_MAX) != 0) 
		return 0;
	return 1;
}

/* MAIN */
int main(int argc, char *argv[])
{
	char *show_class = NULL;
	char *show_module = NULL;
	char *show_root = NULL;
	int retval = 0;
	int opt;
        char *pci_id_file = "/usr/local/share/pci.ids";
	
	while((opt = getopt(argc, argv, cmd_options)) != EOF) {
		switch(opt) {
		case 'a':
			show_options |= SHOW_ATTRIBUTES;
			break;
		case 'A':
			if ((strlen(optarg) + 1) > SYSFS_NAME_LEN) {
				fprintf(stderr, 
					"Attribute name %s is too long\n",
					optarg);
				exit(1);
			}
			attribute_to_show = optarg;
			show_options |= SHOW_ATTRIBUTE_VALUE;
			break;	
		case 'b':
			show_bus = optarg;	
			break;
		case 'c':
			show_class = optarg;	
			break;
		case 'd':
			show_options |= SHOW_DEVICES;
			break;
		case 'D':
			show_options |= SHOW_DRIVERS;
			break;
		case 'h':
			usage();
			exit(0);
			break;
		case 'm':
			show_module = optarg;
		case 'p':
			show_options |= SHOW_PATH;
			break;
		case 'P':
			show_options |= SHOW_PARENT;
			break;
		case 'v':
			show_options |= SHOW_ALL_ATTRIB_VALUES;
			break;
		default:
			usage();
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	switch(argc) {
	case 0:
		break;
	case 1:
		/* get bus to view */
		if ((strlen(*argv)) < SYSFS_NAME_LEN) {
			device_to_show = *argv;
			show_options |= SHOW_DEVICES;
		} else {
			fprintf(stderr, 
				"Invalid argument - device name too long\n");
			exit(1);
		}
		break;
	default:
		usage();
		exit(1);
	}

	if (check_sysfs_is_mounted() == 0) {
		fprintf(stderr, "Unable to find sysfs mount point!\n");
		exit(1);
	}

	if ((!show_bus && !show_class && !show_module && !show_root) && 
			(show_options & (SHOW_ATTRIBUTES | 
				SHOW_ATTRIBUTE_VALUE | SHOW_DEVICES | 
				SHOW_DRIVERS | SHOW_ALL_ATTRIB_VALUES))) {
		fprintf(stderr, 
			"Please specify a bus, class, module, or root device\n");
		usage();
		exit(1);
	}
	/* default is to print devices */
	if (!(show_options & (SHOW_DEVICES | SHOW_DRIVERS)))
		show_options |= SHOW_DEVICES;

	if (show_bus) {
		if ((!(strcmp(show_bus, "pci"))))  {
			pacc = (struct pci_access *)
				calloc(1, sizeof(struct pci_access));
			pacc->pci_id_file_name = pci_id_file;
			pacc->numeric_ids = 0;
		}
		retval = show_sysfs_bus(show_bus);
	}
	if (show_class)
		retval = show_sysfs_class(show_class);

	if (show_module)
		retval = show_sysfs_module(show_module);

	if (!show_bus && !show_class && !show_module && !show_root) 
		retval = show_default_info();

	if (show_bus) {
		if ((!(strcmp(show_bus, "pci"))))  {
			pci_free_name_list(pacc);
			free (pacc);
			pacc = NULL;
		}
	}
	if (!(show_options ^ SHOW_DEVICES))
		fprintf(stdout, "\n");

	exit(retval);
}
