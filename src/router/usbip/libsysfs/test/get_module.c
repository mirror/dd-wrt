/*
 * get_module.c
 *
 * Utility to get details of a given module
 *
 * Copyright (C) IBM Corp. 2003-2005
 *
 *      This program is free software; you can redistribute it and/or modify it+ *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along+ *      with this program; if not, write to the Free Software Foundation, Inc.,+ *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "libsysfs.h"

static void print_usage(void)
{
	fprintf(stdout, "Usage: get_module [name]\n");
}

int main(int argc, char *argv[])
{
	struct sysfs_module *module = NULL;
	struct sysfs_attribute *attr = NULL;
	struct dlist *attrlist = NULL;

	if (argc != 2) {
		print_usage();
		return 1;
	}
	module = sysfs_open_module(argv[1]);
	if (module == NULL) {
		fprintf(stdout, "Module \"%s\" not found\n", argv[1]);
		return 1;
	}

	attrlist = sysfs_get_module_attributes(module);
	if (attrlist != NULL) {
		dlist_for_each_data(attrlist, attr, struct sysfs_attribute)
			fprintf(stdout, "\t%-20s : %s",
				attr->name, attr->value);
	}
	fprintf(stdout, "\n");

	attrlist = sysfs_get_module_parms(module);
	if (attrlist != NULL) {
		fprintf(stdout, "Parameters:\n");
		dlist_for_each_data(attrlist, attr, struct sysfs_attribute)
			fprintf(stdout, "\t%-20s : %s",
				attr->name, attr->value);
	}
	attrlist = sysfs_get_module_sections(module);
	if (attrlist != NULL) {
		fprintf(stdout, "Sections:\n");
		dlist_for_each_data(attrlist, attr, struct sysfs_attribute)
			fprintf(stdout, "\t%-20s : %s",
				attr->name, attr->value);
	}

	sysfs_close_module(module);
	return 0;
}
