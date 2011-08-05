/*
 * test_dir.c
 *
 * Tests for directory related functions for the libsysfs testsuite
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
 ***************************************************************************
 * this will test the directory related functions provided by libsysfs.
 *
 * extern void sysfs_close_attribute(struct sysfs_attribute *sysattr);
 * extern struct sysfs_attribute *sysfs_open_attribute
 * 					(const char *path);
 * extern int sysfs_read_attribute(struct sysfs_attribute *sysattr);
 * extern int sysfs_write_attribute(struct sysfs_attribute *sysattr,
 * 		const char *new_value, size_t len);
 ****************************************************************************
 */

#include "test-defs.h"
#include <errno.h>

/**
 * extern void sysfs_close_attribute(struct sysfs_attribute *sysattr);
 *
 * flag:
 * 	0:	sysattr -> valid
 * 	1:	sysattr -> NULL
 */
int test_sysfs_close_attribute(int flag)
{
	struct sysfs_attribute *sysattr = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_file_path;
		sysattr = sysfs_open_attribute(path);
		if (sysattr == NULL) {
			dbg_print("%s: Error opening attribute at %s\n",
					__FUNCTION__, val_file_path);
			return 0;
		}
		break;
	case 1:
		sysattr = NULL;
		break;
	default:
		return -1;
	}
	sysfs_close_attribute(sysattr);

	dbg_print("%s: returns void\n", __FUNCTION__);

	return 0;
}

/**
 * extern struct sysfs_attribute *sysfs_open_attribute
 * 					(const char *path);
 *
 * flag:
 * 	0:	path -> valid
 * 	1:	path -> invalid
 * 	2:	path -> NULL
 */
int test_sysfs_open_attribute(int flag)
{
	char *path = NULL;
	struct sysfs_attribute *sysattr = NULL;

	switch (flag) {
	case 0:
		path = val_file_path;
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
	sysattr = sysfs_open_attribute(path);

	switch (flag) {
	case 0:
		if (sysattr == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("Attrib name = %s, at %s\n\n",
					sysattr->name, sysattr->path);
		}
		break;
	case 1:
	case 2:
		if (sysattr != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;

	}
	if (sysattr != NULL) {
		sysfs_close_attribute(sysattr);
		sysattr = NULL;
	}

	return 0;
}

/**
 * extern int sysfs_read_attribute(struct sysfs_attribute *sysattr);
 *
 * flag:
 * 	0:	sysattr -> valid
 * 	1:	sysattr -> NULL
 */
int test_sysfs_read_attribute(int flag)
{
	struct sysfs_attribute *sysattr = NULL;
	int ret = 0;

	switch (flag) {
	case 0:
		sysattr = sysfs_open_attribute(val_file_path);
		if (sysattr == NULL) {
			dbg_print("%s: failed opening attribute at %s\n",
					__FUNCTION__, val_file_path);
			return 0;
		}
		break;
	case 1:
		sysattr = NULL;
		break;
	default:
		return -1;
	}
	ret = sysfs_read_attribute(sysattr);

	switch (flag) {
	case 0:
		if (ret != 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_attribute(sysattr);
			dbg_print("\n");
		}
		break;
	case 1:
		if (ret == 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}

	if (sysattr != NULL)
		sysfs_close_attribute(sysattr);

	return 0;
}

/**
 * extern int sysfs_write_attribute(struct sysfs_attribute *sysattr,
 * 			const char *new_value, size_t len);
 *
 * flag:
 * 	0:	sysattr -> valid, new_value -> valid, len -> valid;
 * 	1:	sysattr -> valid, new_value -> invalid, len -> invalid;
 * 	2:	sysattr -> valid, new_value -> NULL, len -> invalid;
 * 	3:	sysattr -> NULL, new_value -> valid, len -> valid;
 * 	4:	sysattr -> NULL, new_value -> invalid, len -> invalid;
 * 	5:	sysattr -> NULL, new_value -> NULL, len -> invalid;
 */
int test_sysfs_write_attribute(int flag)
{
	struct sysfs_attribute *sysattr = NULL;
	char *new_value = NULL;
	size_t len = 0;
	int ret = 0;

	switch (flag) {
	case 0:
		sysattr = sysfs_open_attribute(val_write_attr_path);
		if (sysattr == NULL) {
			dbg_print("%s: failed opening attribute at %s\n",
					__FUNCTION__, val_write_attr_path);
			return 0;
		}
		if (sysfs_read_attribute(sysattr) != 0) {
			dbg_print("%s: failed reading attribute at %s\n",
					__FUNCTION__, val_write_attr_path);
			return 0;
		}
		new_value = calloc(1, sysattr->len + 1);
		strncpy(new_value, sysattr->value, sysattr->len);
		len = sysattr->len;
		break;
	case 1:
		sysattr = sysfs_open_attribute(val_write_attr_path);
		if (sysattr == NULL) {
			dbg_print("%s: failed opening attribute at %s\n",
					__FUNCTION__, val_write_attr_path);
			return 0;
		}
		new_value = calloc(1, SYSFS_PATH_MAX);
		strncpy(new_value, "this should not get copied in the attrib",
				SYSFS_PATH_MAX);
		len = SYSFS_PATH_MAX;
		break;
	case 2:
		sysattr = sysfs_open_attribute(val_write_attr_path);
		if (sysattr == NULL) {
			dbg_print("%s: failed opening attribute at %s\n",
					__FUNCTION__, val_write_attr_path);
			return 0;
		}
		new_value = NULL;
		len = SYSFS_PATH_MAX;
		break;
	case 3:
		sysattr = sysfs_open_attribute(val_write_attr_path);
		if (sysattr == NULL) {
			dbg_print("%s: failed opening attribute at %s\n",
					__FUNCTION__, val_write_attr_path);
			return 0;
		}
		new_value = calloc(1, sysattr->len + 1);
		strncpy(new_value, sysattr->value, sysattr->len);
		len = sysattr->len;
		sysfs_close_attribute(sysattr);
		sysattr = NULL;
		break;
	case 4:
		sysattr = NULL;
		new_value = calloc(1, SYSFS_PATH_MAX);
		strncpy(new_value, "this should not get copied in the attrib",
				SYSFS_PATH_MAX);
		len = SYSFS_PATH_MAX;
		break;
	case 5:
		sysattr = NULL;
		new_value = NULL;
		len = SYSFS_PATH_MAX;
		break;
	default:
		return -1;
	}
	ret = sysfs_write_attribute(sysattr, new_value, len);

	switch (flag) {
	case 0:
		if (ret != 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
		     	dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("Attribute at %s now has value %s\n\n",
					sysattr->path, sysattr->value);
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		if (ret == 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
		   	dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (sysattr != NULL) {
		sysfs_close_attribute(sysattr);
		sysattr = NULL;
	}
	if (new_value != NULL)
		free(new_value);

	return 0;
}
