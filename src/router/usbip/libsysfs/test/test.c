/*
 * test.c
 *
 * Main program for the libsysfs testsuite
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
 * this doesn't do much, just loops throug to call each function.
 */

#include "test-defs.h"
#include <errno.h>

/*************************************************/
char *function_name[] = {
	"sysfs_get_mnt_path",
	"sysfs_remove_trailing_slash",
	"sysfs_get_name_from_path",
	"sysfs_path_is_dir",
	"sysfs_path_is_link",
	"sysfs_path_is_file",
	"sysfs_get_link",
	"sysfs_close_list",
	"sysfs_open_directory_list",
	"sysfs_open_link_list",
	"sysfs_close_attribute",
	"sysfs_open_attribute",
	"sysfs_read_attribute",
	"sysfs_write_attribute",
	"sysfs_close_driver",
	"sysfs_open_driver",
	"sysfs_open_driver_path",
	"sysfs_get_driver_attr",
	"sysfs_get_driver_attributes",
	"sysfs_get_driver_devices",
	"sysfs_close_device",
	"sysfs_open_device",
	"sysfs_get_device_parent",
	"sysfs_open_device_path",
	"sysfs_get_device_attr",
	"sysfs_get_device_attributes",
	"sysfs_close_bus",
	"sysfs_open_bus",
	"sysfs_get_bus_device",
	"sysfs_get_bus_driver",
	"sysfs_get_bus_drivers",
	"sysfs_get_bus_devices",
	"sysfs_close_class_device",
	"sysfs_open_class_device_path",
	"sysfs_open_class_device",
	"sysfs_get_classdev_device",
	"sysfs_get_classdev_parent",
	"sysfs_close_class",
	"sysfs_open_class",
	"sysfs_get_class_devices",
	"sysfs_get_class_device",
	"sysfs_get_classdev_attributes",
	"sysfs_get_classdev_attr",
	"sysfs_close_module",
	"sysfs_open_module_path",
	"sysfs_open_module",
	"sysfs_get_module_attr",
	"sysfs_get_module_attributes",
	"sysfs_get_module_parms",
	"sysfs_get_module_sections",
	"sysfs_get_module_parm",
	"sysfs_get_module_section",
};

int (*func_table[])(int) = {
	test_sysfs_get_mnt_path,
	test_sysfs_remove_trailing_slash,
	test_sysfs_get_name_from_path,
	test_sysfs_path_is_dir,
	test_sysfs_path_is_link,
	test_sysfs_path_is_file,
	test_sysfs_get_link,
	test_sysfs_close_list,
	test_sysfs_open_directory_list,
	test_sysfs_open_link_list,
	test_sysfs_close_attribute,
	test_sysfs_open_attribute,
	test_sysfs_read_attribute,
	test_sysfs_write_attribute,
	test_sysfs_close_driver,
	test_sysfs_open_driver,
	test_sysfs_open_driver_path,
	test_sysfs_get_driver_attr,
	test_sysfs_get_driver_attributes,
	test_sysfs_get_driver_devices,
	test_sysfs_close_device,
	test_sysfs_open_device,
	test_sysfs_get_device_parent,
	test_sysfs_open_device_path,
	test_sysfs_get_device_attr,
	test_sysfs_get_device_attributes,
	test_sysfs_close_bus,
	test_sysfs_open_bus,
	test_sysfs_get_bus_device,
	test_sysfs_get_bus_driver,
	test_sysfs_get_bus_drivers,
	test_sysfs_get_bus_devices,
	test_sysfs_close_class_device,
	test_sysfs_open_class_device_path,
	test_sysfs_open_class_device,
	test_sysfs_get_classdev_device,
	test_sysfs_get_classdev_parent,
	test_sysfs_close_class,
	test_sysfs_open_class,
	test_sysfs_get_class_devices,
	test_sysfs_get_class_device,
	test_sysfs_get_classdev_attributes,
	test_sysfs_get_classdev_attr,
	test_sysfs_close_module,
	test_sysfs_open_module_path,
	test_sysfs_open_module,
	test_sysfs_get_module_attr,
	test_sysfs_get_module_attributes,
	test_sysfs_get_module_parms,
	test_sysfs_get_module_sections,
	test_sysfs_get_module_parm,
	test_sysfs_get_module_section,
};

char *dir_paths[] = {
	val_dir_path,
	val_root_dev_path,
	val_class_dev_path,
	val_block_class_dev_path,
	val_drv_path,
	val_drv1_path,
	NULL
};

char *file_paths[] = {
	val_file_path,
	val_write_attr_path,
	NULL
};

char *link_paths[] = {
	val_link_path,
	NULL
};

static int path_is_dir(const char *path)
{
	struct stat astats;

	if ((lstat(path, &astats)) != 0)
		goto direrr;

	if (S_ISDIR(astats.st_mode))
		return 0;

direrr:
	fprintf(stdout, "Config error: %s not a directory\n", path);
	return 1;
}

static int path_is_file(const char *path)
{
	struct stat astats;

	if ((lstat(path, &astats)) != 0)
		goto fileerr;

	if (S_ISREG(astats.st_mode))
		return 0;

fileerr:
	fprintf(stdout, "Config error: %s not a file\n", path);
	return 1;
}

static int path_is_link(const char *path)
{
	struct stat astats;

	if ((lstat(path, &astats)) != 0)
		goto linkerr;

	if (S_ISLNK(astats.st_mode))
		return 0;

linkerr:
	fprintf(stdout, "Config error: %s not a link\n", path);
	return 1;
}

/*
 * Check validity of the test.h file entries
 */
static int check_header(void)
{
	char *var_path = NULL;
	char path1[SYSFS_PATH_MAX];
	int i = 0;

	for (i = 0; dir_paths[i] != NULL; i++) {
		var_path = dir_paths[i];
		if (path_is_dir(var_path) != 0)
			return 1;
	}

	for (i = 0; file_paths[i] != NULL; i++) {
		var_path = file_paths[i];
		if (path_is_file(var_path) != 0)
			return 1;
	}

	for (i = 0; link_paths[i] != NULL; i++) {
		var_path = link_paths[i];
		if (path_is_link(var_path) != 0)
			return 1;
	}

	memset(path1, 0, SYSFS_PATH_MAX);
	strcpy(path1, val_root_dev_path);
	strcat(path1, "/");
	strcat(path1, val_subdir_name);
	if (path_is_dir(path1) != 0)
		return 1;

	memset(path1, 0, SYSFS_PATH_MAX);
	strcpy(path1, val_drv_path);
	strcat(path1, "/");
	strcat(path1, val_drv_dev_name);
	if (path_is_link(path1) != 0)
		return 1;

	memset(path1, 0, SYSFS_PATH_MAX);
	strcpy(path1, val_dir_path);
	strcat(path1, "/devices");
	strcat(path1, "/");
	strcat(path1, val_subdir_link_name);
	if (path_is_link(path1) != 0)
		return 1;

	memset(path1, 0, SYSFS_PATH_MAX);
	strcpy(path1, val_class_dev_path);
	strcat(path1, "/");
	strcat(path1, val_class_dev_attr);
	if (path_is_file(path1) != 0)
		return 1;

	memset(path1, 0, SYSFS_PATH_MAX);
	strcpy(path1, val_dev_path);
	strcat(path1, "/");
	strcat(path1, val_dev_attr);
	if (path_is_file(path1) != 0)
		return 1;

	memset(path1, 0, SYSFS_PATH_MAX);
	strcpy(path1, val_drv_path);
	strcat(path1, "/");
	strcat(path1, val_drv_attr_name);
	if (path_is_file(path1) != 0)
		return 1;

	return 0;
}

static void usage(void)
{
	fprintf(stdout, "testlibsysfs <no-of-times> [log-file]\n");
	fprintf(stdout, "\t eg: testlibsysfs 3 /home/user/test.log\n");
}

int main(int argc, char *argv[])
{
	char std_out[] = "/dev/stdout";
	char *file_name = NULL;
	int i = 0, j = 0, k = 0, num = 1;

	if (argc == 3) {
		file_name = argv[2];
	} else {
		file_name = std_out;
	}
	my_stdout = fopen(file_name,"w");
	if (argc < 2) {
		usage();
		return 0;
	} else
		num = strtol(argv[1], NULL, 0);

	if (check_header() != 0)
		return 1;

	dbg_print("\nTest running %d times\n", num);

	for (k = 0; k < num ; k++) {
		dbg_print("\nThis is the %d test run\n", k+1);

		for (i = 0; i < FUNC_TABLE_SIZE; i++) {
dbg_print("\n**************************************************************\n");
			dbg_print("TESTING: %s, function no: %d\n\n",
					function_name[i], i);
			for (j = 0; ; j++) {
				fflush(my_stdout);
				if (func_table[i](j) == -1)
					break;
			}
dbg_print("**************************************************************\n");
		}
	}

	fclose(my_stdout);
	return 0;
}
