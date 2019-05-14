/**
 * @file support/junction/path.c
 * @brief Encode and decode FedFS pathnames
 */

/*
 * Copyright 2010, 2011, 2018 Oracle.  All rights reserved.
 *
 * This file is part of nfs-utils.
 *
 * nfs-utils is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * nfs-utils is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2.0 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2.0 along with nfs-utils.  If not, see:
 *
 *	http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include <netinet/in.h>

#include "junction.h"
#include "xlog.h"

#define STRLEN_SLASH	((size_t)1)	/* strlen("/") */

#define XDR_UINT_BYTES	(sizeof(uint32_t))

/**
 * Compute count of XDR 4-octet units from byte count
 *
 * @param bytes number of bytes to convert
 * @return equivalent number of XDR 4-octet units
 */
static inline size_t
nsdb_quadlen(size_t bytes)
{
	return (bytes + 3) >> 2;
}

/**
 * Free array of NUL-terminated C strings
 *
 * @param strings array of char * to be released
 */
void
nsdb_free_string_array(char **strings)
{
	int i;

	if (strings == NULL)
		return;
	for (i = 0; strings[i] != NULL; i++)
		free(strings[i]);
	free(strings);
}

static FedFsStatus
nsdb_alloc_zero_component_pathname(char ***path_array)
{
	char **result;

	xlog(D_GENERAL, "%s: Zero-component pathname", __func__);

	result = (char **)calloc(1, sizeof(char *));
	if (result == NULL)
		return FEDFS_ERR_SVRFAULT;
	result[0] = NULL;
	*path_array = result;
	return FEDFS_OK;
}

/**
 * Sanitize an incoming POSIX path
 *
 * @param pathname NUL-terminated C string containing a POSIX pathname
 * @return NUL-terminated C string containing sanitized path
 *
 * Caller must free the returned pathname with free(3).
 *
 * Remove multiple sequential slashes and any trailing slashes,
 * but leave "/" by itself alone.
 */
static __attribute_malloc__ char *
nsdb_normalize_path(const char *pathname)
{
	size_t i, j, len;
	char *result;

	len = strlen(pathname);
	if (len == 0) {
		xlog(D_CALL, "%s: NULL pathname", __func__);
		return NULL;
	}

	result = malloc(len + 1);
	if (result == NULL)
		return NULL;

	for (i = 0, j = 0; i < len; i++) {
		if (pathname[i] == '/' && pathname[i + 1] == '/')
			continue;
		result[j++] = pathname[i];
	}
	result[j] = '\0';

	if (j > 1 && result[j - 1] == '/')
		result[j - 1] = '\0';

	xlog(D_CALL, "%s: result = '%s'", __func__, result);
	return result;
}

/**
 * Count the number of components in a POSIX pathname
 *
 * @param pathname NUL-terminated C string containing a POSIX pathname
 * @param len OUT: number of bytes the encoded XDR stream will consume
 * @param cnt OUT: component count
 * @return true when successful
 */
static _Bool
nsdb_count_components(const char *pathname, size_t *len,
		unsigned int *cnt)
{
	char *start, *component;
	unsigned int count;
	size_t length;

	/* strtok(3) will tromp on the string */
	start = strdup(pathname);
	if (start == NULL)
		return false;

	length = XDR_UINT_BYTES;
	count = 0;
	component = start;
	for ( ;; ) {
		char *next;
		size_t tmp;

		if (*component == '/')
			component++;
		if (*component == '\0')
			break;
		next = strchrnul(component, '/');
		tmp = (size_t)(next - component);
		if (tmp > 255) {
			free(start);
			return false;
		}
		length += XDR_UINT_BYTES + (nsdb_quadlen(tmp) << 2);
		count++;

		if (*next == '\0')
			break;
		component = next;
	}

	free(start);

	xlog(D_CALL, "%s: length = %zu, count = %u, path = '%s'",
		__func__, length, count, pathname);
	*len = length;
	*cnt = count;
	return true;
}

/**
 * Predicate: is input character set for a POSIX pathname valid UTF-8?
 *
 * @param pathname NUL-terminated C string containing a POSIX path
 * @return true if the string is valid UTF-8
 *
 * XXX: implement this
 */
static _Bool
nsdb_pathname_is_utf8(__attribute__((unused)) const char *pathname)
{
	return true;
}

/**
 * Construct a local POSIX-style pathname from an array of component strings
 *
 * @param path_array array of pointers to NUL-terminated C strings
 * @param pathname OUT: pointer to NUL-terminated UTF-8 C string containing a POSIX-style path
 * @return a FedFsStatus code
 *
 * Caller must free the returned pathname with free(3).
 */
FedFsStatus
nsdb_path_array_to_posix(char * const *path_array, char **pathname)
{
	char *component, *result;
	unsigned int i, count;
	size_t length, len;

	if (path_array == NULL || pathname == NULL)
		return FEDFS_ERR_INVAL;

	if (path_array[0] == NULL) {
		xlog(D_GENERAL, "%s: Zero-component pathname", __func__);
		result = strdup("/");
		if (result == NULL)
			return FEDFS_ERR_SVRFAULT;
		*pathname = result;
		return FEDFS_OK;
	}

	for (length = 0, count = 0;
	     path_array[count] != NULL;
	     count++) {
		component = path_array[count];
		len = strlen(component);

		if (len == 0) {
			xlog(D_GENERAL, "%s: Zero-length component", __func__);
			return FEDFS_ERR_BADNAME;
		}
		if (len > NAME_MAX) {
			xlog(D_GENERAL, "%s: Component length too long", __func__);
			return FEDFS_ERR_NAMETOOLONG;
		}
		if (strchr(component, '/') != NULL) {
			xlog(D_GENERAL, "%s: Local separator character "
					"found in component", __func__);
			return FEDFS_ERR_BADNAME;
		}
		if (!nsdb_pathname_is_utf8(component)) {
			xlog(D_GENERAL, "%s: Bad character in component",
				__func__);
			return FEDFS_ERR_BADCHAR;
		}

		length += STRLEN_SLASH + len;

		if (length > PATH_MAX) {
			xlog(D_GENERAL, "%s: Pathname too long", __func__);
			return FEDFS_ERR_NAMETOOLONG;
		}
	}

	result = calloc(1, length + 1);
	if (result == NULL)
		return FEDFS_ERR_SVRFAULT;

	for (i = 0; i < count; i++) {
		strcat(result, "/");
		strcat(result, path_array[i]);
	}
	*pathname = nsdb_normalize_path(result);
	free(result);
	if (*pathname == NULL)
		return FEDFS_ERR_SVRFAULT;
	return FEDFS_OK;
}

/**
 * Construct an array of component strings from a local POSIX-style pathname
 *
 * @param pathname NUL-terminated C string containing a POSIX-style pathname
 * @param path_array OUT: pointer to array of pointers to NUL-terminated C strings
 * @return a FedFsStatus code
 *
 * Caller must free "path_array" with nsdb_free_string_array().
 */
FedFsStatus
nsdb_posix_to_path_array(const char *pathname, char ***path_array)
{
	char *normalized, *component, **result;
	unsigned int i, count;
	size_t length;

	if (pathname == NULL || path_array == NULL)
		return FEDFS_ERR_INVAL;

	if (!nsdb_pathname_is_utf8(pathname)) {
		xlog(D_GENERAL, "%s: Bad character in pathname", __func__);
		return FEDFS_ERR_BADCHAR;
	}

	normalized = nsdb_normalize_path(pathname);
	if (normalized == NULL)
		return FEDFS_ERR_SVRFAULT;

	if (!nsdb_count_components(normalized, &length, &count)) {
		free(normalized);
		return FEDFS_ERR_BADNAME;
	}

	if (count == 0) {
		free(normalized);
		return nsdb_alloc_zero_component_pathname(path_array);
	}

	result = (char **)calloc(count + 1, sizeof(char *));
	if (result == NULL) {
		free(normalized);
		return FEDFS_ERR_SVRFAULT;
	}

	component = normalized;
	for (i = 0; ; i++) {
		char *next;

		if (*component == '/')
			component++;
		if (*component == '\0')
			break;
		next = strchrnul(component, '/');
		length = (size_t)(next - component);
		if (length > 255) {
			nsdb_free_string_array(result);
			free(normalized);
			return FEDFS_ERR_SVRFAULT;
		}

		result[i] = strndup(component, length);
		if (result[i] == NULL) {
			free(normalized);
			nsdb_free_string_array(result);
			return FEDFS_ERR_SVRFAULT;
		}

		if (*next == '\0')
			break;
		component = next;
	}

	*path_array = result;
	free(normalized);
	return FEDFS_OK;
}
