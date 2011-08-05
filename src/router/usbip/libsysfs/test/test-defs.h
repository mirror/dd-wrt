/*
 * test.h
 *
 * Generic defines/declarations for the libsysfs testsuite
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

#ifndef _TESTER_H_
#define _TESTER_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "libsysfs.h"
#include "dlist.h"
#include "test.h"

#define val_drv1_dev_name	"dummy1"
#define val_drv1_attr_name	"dummy2"
#define inval_name 		"invalid_name"
#define inval_path 		"/sys/invalid/path"
#define FUNC_TABLE_SIZE  	(sizeof(func_table)/sizeof(int))

FILE *my_stdout;

#define dbg_print(format, arg...) fprintf(my_stdout, format, ## arg)

/**
 * list of display functions
 */
extern void show_device(struct sysfs_device *device);
extern void show_driver(struct sysfs_driver *driver);
extern void show_device_list(struct dlist *devlist);
extern void show_driver_list(struct dlist *drvlist);
extern void show_attribute(struct sysfs_attribute *attr);
extern void show_attribute_list(struct dlist *list);
extern void show_links_list(struct dlist *linklist);
extern void show_dir_list(struct dlist *dirlist);
extern void show_class_device(struct sysfs_class_device *dev);
extern void show_class_device_list(struct dlist *devlist);
extern void show_list(struct dlist *list);
extern void show_parm_list(struct dlist *list);
extern void show_section_list(struct dlist *list);
extern void show_module(struct sysfs_module *module);

/**
 * list of test functions.....
 */
extern int test_sysfs_get_mnt_path(int flag);
extern int test_sysfs_remove_trailing_slash(int flag);
extern int test_sysfs_get_name_from_path(int flag);
extern int test_sysfs_path_is_dir(int flag);
extern int test_sysfs_path_is_link(int flag);
extern int test_sysfs_path_is_file(int flag);
extern int test_sysfs_get_link(int flag);
extern int test_sysfs_open_directory_list(int flag);
extern int test_sysfs_open_link_list(int flag);
extern int test_sysfs_close_list(int flag);
extern int test_sysfs_close_attribute(int flag);
extern int test_sysfs_open_attribute(int flag);
extern int test_sysfs_read_attribute(int flag);
extern int test_sysfs_write_attribute(int flag);
extern int test_sysfs_close_driver(int flag);
extern int test_sysfs_open_driver(int flag);
extern int test_sysfs_open_driver_path(int flag);
extern int test_sysfs_get_driver_attr(int flag);
extern int test_sysfs_get_driver_attributes(int flag);
extern int test_sysfs_get_driver_devices(int flag);
extern int test_sysfs_get_driver_module(int flag);
extern int test_sysfs_close_device(int flag);
extern int test_sysfs_open_device(int flag);
extern int test_sysfs_get_device_parent(int flag);
extern int test_sysfs_open_device_path(int flag);
extern int test_sysfs_get_device_attr(int flag);
extern int test_sysfs_get_device_attributes(int flag);
extern int test_sysfs_close_bus(int flag);
extern int test_sysfs_open_bus(int flag);
extern int test_sysfs_get_bus_device(int flag);
extern int test_sysfs_get_bus_driver(int flag);
extern int test_sysfs_get_bus_drivers(int flag);
extern int test_sysfs_get_bus_devices(int flag);
extern int test_sysfs_close_class_device(int flag);
extern int test_sysfs_open_class_device_path(int flag);
extern int test_sysfs_open_class_device(int flag);
extern int test_sysfs_get_classdev_device(int flag);
extern int test_sysfs_get_classdev_parent(int flag);
extern int test_sysfs_close_class(int flag);
extern int test_sysfs_open_class(int flag);
extern int test_sysfs_get_class_devices(int flag);
extern int test_sysfs_get_class_device(int flag);
extern int test_sysfs_get_classdev_attributes(int flag);
extern int test_sysfs_get_classdev_attr(int flag);
extern int test_sysfs_open_classdev_attr(int flag);
extern int test_sysfs_close_module(int flag);
extern int test_sysfs_open_module_path(int flag);
extern int test_sysfs_open_module(int flag);
extern int test_sysfs_get_module_attr(int flag);
extern int test_sysfs_get_module_attributes(int flag);
extern int test_sysfs_get_module_parms(int flag);
extern int test_sysfs_get_module_sections(int flag);
extern int test_sysfs_get_module_parm(int flag);
extern int test_sysfs_get_module_section(int flag);

#endif /* _TESTER_H_ */
