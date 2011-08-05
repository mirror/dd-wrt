/*
 * testout.c
 *
 * Display routines for the libsysfs testsuite
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
 * Display routines for test functions
 */

#include <test-defs.h>

static void remove_end_newline(char *value)
{
        char *p = value + (strlen(value) - 1);

        if (p != NULL && *p == '\n')
                *p = '\0';
}

void show_device(struct sysfs_device *device)
{
	if (device != NULL)
		dbg_print("Device is \"%s\" at \"%s\"\n",
				device->name, device->path);
}

void show_driver(struct sysfs_driver *driver)
{
	if (driver != NULL)
		dbg_print("Driver is \"%s\" at \"%s\"\n",
				driver->name, driver->path);
}

void show_device_list(struct dlist *devlist)
{
	if (devlist != NULL) {
		struct sysfs_device *dev = NULL;

		dlist_for_each_data(devlist, dev, struct sysfs_device)
			show_device(dev);
	}
}

void show_driver_list(struct dlist *drvlist)
{
	if (drvlist != NULL) {
		struct sysfs_driver *drv = NULL;

		dlist_for_each_data(drvlist, drv, struct sysfs_driver)
			show_driver(drv);
	}
}

void show_attribute(struct sysfs_attribute *attr)
{
	if (attr != NULL) {
		if (attr->value)
			remove_end_newline(attr->value);
		dbg_print("Attr \"%s\" at \"%s\" has a value \"%s\" \n",
				attr->name, attr->path, attr->value);
	}
}

void show_attribute_list(struct dlist *attrlist)
{
	if (attrlist != NULL) {
		struct sysfs_attribute *attr = NULL;

		dlist_for_each_data(attrlist, attr, struct sysfs_attribute)
			show_attribute(attr);
	}
}

void show_class_device(struct sysfs_class_device *dev)
{
	if (dev != NULL)
		dbg_print("Class device \"%s\" belongs to the \"%s\" class\n",
				dev->name, dev->classname);
}

void show_class_device_list(struct dlist *devlist)
{
	if (devlist != NULL) {
		struct sysfs_class_device *dev = NULL;

		dlist_for_each_data(devlist, dev, struct sysfs_class_device)
			show_class_device(dev);
	}
}

void show_list(struct dlist *list)
{
	if (list != NULL) {
		char *name = NULL;

		dlist_for_each_data(list, name, char)
			dbg_print("%s\n", name);
	}
}

void show_parm_list(struct dlist *list)
{
 	if (list != NULL) {
  		char *name = NULL;

		dlist_for_each_data(list, name, char)
 			dbg_print("%s\n", name);
  	}
}

void show_section_list(struct dlist *list)
{
   	if (list != NULL) {
    		char *name = NULL;

     		dlist_for_each_data(list, name, char)
      			dbg_print("%s\n", name);
       	}
}

void show_module(struct sysfs_module *module)
{
	if (module) {
		dbg_print("Module name is %s, path is %s\n",
				module->name, module->path);
 		show_attribute_list(module->attrlist);
  		show_parm_list(module->parmlist);
   		show_section_list(module->sections);
    	}
}
