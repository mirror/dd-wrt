/*
 * test_utils.c
 *
 * Tests for utility functions for the libsysfs testsuite
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
 * this will test the utility functions provided by libsysfs.
 *
 * extern int sysfs_get_mnt_path(char *mnt_path, size_t len);
 * extern int sysfs_remove_trailing_slash(char *path);
 * extern int sysfs_get_name_from_path(const char *path,
 * 					char *name, size_t len);
 * extern int sysfs_path_is_dir(const char *path);
 * extern int sysfs_path_is_link(const char *path);
 * extern int sysfs_path_is_file(const char *path);
 * extern int sysfs_get_link(const char *path, char *target, size_t len);
 * extern struct dlist *sysfs_open_directory_list(char *name);
 * extern struct dlist *sysfs_open_link_list(char *name);
 * extern void sysfs_close_list(struct dlist *list);
 *****************************************************************************
 */

#include "test-defs.h"
#include <errno.h>

/**
 * extern int sysfs_get_mnt_path(char *mnt_path, size_t len);
 *
 * flag:
 * 	0:	mnt_path -> valid, len -> valid
 * 	1:	mnt_path -> valid, len -> 0
 * 	2:	mnt_path -> NULL, len -> valid
 * 	3:	mnt_path -> NULL, len -> NULL
 */
int test_sysfs_get_mnt_path(int flag)
{
	char *mnt_path = NULL;
	size_t len = 0;
	int ret = 0;

	switch (flag) {
	case 0:
		mnt_path = calloc(1, SYSFS_PATH_MAX);
		len = SYSFS_PATH_MAX;
		break;
	case 1:
		mnt_path = calloc(1, SYSFS_PATH_MAX);
		len = 0;
		break;
	case 2:
		mnt_path = NULL;
		len = SYSFS_PATH_MAX;
		break;
	case 3:
		mnt_path = NULL;
		len = 0;
		break;
	default:
		return -1;
	}
	ret = sysfs_get_mnt_path(mnt_path, len);

	switch (flag) {
	case 0:
		if (ret)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("sysfs is mounted at \"%s\"\n\n", mnt_path);
		}
		break;
	case 1:
	case 2:
	case 3:
		if (!ret)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (mnt_path != NULL) {
		free(mnt_path);
		mnt_path = NULL;
	}

	return 0;
}

/**
 * extern int sysfs_remove_trailing_slash(char *path);
 *
 * flag:
 * 	0:	path -> valid
 * 	1:	path -> NULL
 */
int test_sysfs_remove_trailing_slash(int flag)
{
	char *path = NULL;
	int ret = 0;

	switch (flag) {
	case 0:
		path = calloc(1, SYSFS_PATH_MAX);
		strcpy(path, "/some/path/is/this/");
		break;
	case 1:
		path = NULL;
		break;
	default:
		return -1;
	}
	ret = sysfs_remove_trailing_slash(path);

	switch (flag) {
	case 0:
		if (ret != 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("Path now is \"%s\"\n\n", path);
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

	if (path != NULL) {
		free(path);
		path = NULL;
	}

	return 0;
}

/**
 * extern int sysfs_get_name_from_path(const char *path,
 * 					char *name, size_t len);
 *
 * flag:
 * 	0:	path -> valid, name -> valid, len -> valid
 * 	1:	path -> valid, name -> NULL, len -> invalid
 * 	2:	path -> NULL, name -> valid, len -> valid
 * 	3:	path -> NULL, name -> NULL, len -> invalid
 */
int test_sysfs_get_name_from_path(int flag)
{
	char *path = NULL;
	char *name = NULL;
	size_t len = SYSFS_NAME_LEN;
	int ret = 0;

	switch (flag) {
	case 0:
		path = val_dir_path;
		name = calloc(1, SYSFS_NAME_LEN);
		break;
	case 1:
		path = val_dir_path;
		name = NULL;
		break;
	case 2:
		path = NULL;
		name = calloc(1, SYSFS_NAME_LEN);
		break;
	case 3:
		path = NULL;
		name = NULL;
		break;
	default:
		return -1;
	}
	ret = sysfs_get_name_from_path(path, name, len);

	switch (flag) {
	case 0:
		if (ret != 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("Name extracted from \"%s\" is \"%s\"\n\n",
					path, name);
		}
		break;
	case 1:
	case 2:
	case 3:
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
	if (name != NULL) {
		free(name);
		name = NULL;
	}

	return 0;
}

/**
 * extern int sysfs_path_is_dir(const char *path);
 *
 * flag:
 * 	0:	path -> valid
 * 	1:	path -> invalid
 * 	2:	path -> NULL
 */
int test_sysfs_path_is_dir(int flag)
{
	char *path = NULL;
	int ret = 0;

	switch (flag) {
	case 0:
		path = val_dir_path;
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
	ret = sysfs_path_is_dir(path);

	switch (flag) {
	case 0:
		if (ret != 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("Path \"%s\" points to a directory\n\n",
						path);
		}
		break;
	case 1:
	case 2:
		if (ret == 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		return 0;
		break;
	}

	return 0;
}

/**
 * extern int sysfs_path_is_link(const char *path);
 *
 * flag:
 * 	0:	path -> valid
 * 	1:	path -> invalid
 * 	2:	path -> NULL
 */
int test_sysfs_path_is_link(int flag)
{
	char *path = NULL;
	int ret = 0;

	switch (flag) {
	case 0:
		path = val_link_path;
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
	ret = sysfs_path_is_link(path);

	switch (flag) {
	case 0:
		if (ret != 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("Path \"%s\" points to a link\n\n",
						path);
		}
		break;
	case 1:
	case 2:
		if (ret == 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		return 0;
		break;
	}

	return 0;
}

/**
 * extern int sysfs_path_is_file(const char *path);
 *
 * flag:
 * 	0:	path -> valid
 * 	1:	path -> invalid
 * 	2:	path -> NULL
 */
int test_sysfs_path_is_file(int flag)
{
	char *path = NULL;
	int ret = 0;

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
	ret = sysfs_path_is_file(path);

	switch (flag) {
	case 0:
		if (ret != 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("Path \"%s\" points to a file\n\n",
						path);
		}
		break;
	case 1:
	case 2:
		if (ret == 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		return 0;
		break;
	}
	return 0;
}

/**
 * extern int sysfs_get_link(const char *path,
 * 					char *target, size_t len);
 *
 * flag:
 * 	0:	path -> valid, target -> valid
 * 	1:	path -> valid, target -> NULL
 * 	2:	path -> NULL, target -> valid
 * 	3:	path -> NULL, target -> NULL
 */
int test_sysfs_get_link(int flag)
{
	char *path = NULL;
	char *target =  NULL;
	size_t len = SYSFS_PATH_MAX;
	int ret = 0;

	switch (flag) {
	case 0:
		path = val_link_path;
		target = calloc(1, SYSFS_PATH_MAX);
		break;
	case 1:
		path = val_link_path;
		target = NULL;
		break;
	case 2:
		path = NULL;
		target = calloc(1, SYSFS_PATH_MAX);
		break;
	case 3:
		path = NULL;
		target = NULL;
		break;
	default:
		return -1;
	}
	ret = sysfs_get_link(path, target, len);

	switch (flag) {
	case 0:
		if (ret != 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("Link at \"%s\" points to \"%s\"\n\n",
						path, target);
		}
		break;
	case 1:
	case 2:
	case 3:
		if (ret == 0)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		return 0;
		break;
	}
	if (target != NULL) {
		free(target);
		target = NULL;
	}

	return 0;
}

/**
 * extern struct dlist *sysfs_open_directory_list(char *name);
 *
 * flag:
 * 	0:	name -> valid
 * 	1:	name -> invalid
 * 	2:	name -> NULL
 *
 */
int  test_sysfs_open_directory_list(int flag)
{
	char *path = NULL;
	struct dlist *list = NULL;

	switch (flag) {
	case 0:
		path = val_dir_path;
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
	list = sysfs_open_directory_list(path);

	switch (flag) {
	case 0:
		if (list == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_list(list);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
		if (list != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (list != NULL) {
		sysfs_close_list(list);
		list = NULL;
	}

	return 0;
}

/**
 * extern struct dlist *sysfs_open_link_list(char *name);
 *
 * flag:
 * 	0:	name -> valid
 * 	1:	name -> invalid
 * 	2:	name -> NULL
 *
 */
int  test_sysfs_open_link_list(int flag)
{
	char *path = NULL;
	struct dlist *list = NULL;

	switch (flag) {
	case 0:
		path = val_link_path;
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
	list = sysfs_open_link_list(path);

	switch (flag) {
	case 0:
		if (list == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_list(list);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
		if (list != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (list != NULL) {
		sysfs_close_list(list);
		list = NULL;
	}

	return 0;
}
/**
 * extern void sysfs_close_list(struct dlist *list);
 *
 * flag:
 * 	0:	list -> valid
 * 	1:	list -> NULL
 */
int test_sysfs_close_list(int flag)
{
	struct dlist *list = NULL;

	switch (flag) {
	case 0:
		list = NULL;
		break;
	default:
		return -1;
	}
	sysfs_close_list(list);

	dbg_print("%s: returns void\n", __FUNCTION__);

	return 0;
}
