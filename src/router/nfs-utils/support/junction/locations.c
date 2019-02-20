/**
 * @file support/junction/locations.c
 * @brief Utility functions to manage NFS locations data
 */

/*
 * Copyright 2011, 2018 Oracle.  All rights reserved.
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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "junction.h"

/**
 * Free an array of NUL-terminated C strings
 *
 * @param array array of pointers to C strings
 */
void
nfs_free_string_array(char **array)
{
	unsigned int i;

	if (array == NULL)
		return;
	for (i = 0; array[i] != NULL; i++)
		free(array[i]);
	free(array);
}

/**
 * Duplicate an array of NUL-terminated C strings
 *
 * @param array array of pointers to C strings
 * @return freshly allocated array of points to C strings, or NULL
 *
 * Caller must free the returned array with nfs_free_string_array()
 */
__attribute_malloc__ char **
nfs_dup_string_array(char **array)
{
	unsigned int size, i;
	char **result;

	if (array == NULL)
		return NULL;

	for (size = 0; array[size] != NULL; size++);

	result = calloc(size + 1, sizeof(char *));
	if (result == NULL)
		return NULL;
	for (i = 0; i < size; i++) {
		result[i] = strdup(array[i]);
		if (result[i] == NULL) {
			nfs_free_string_array(result);
			return NULL;
		}
	}
	return result;
}

/**
 * Free a single NFS location
 *
 * @param location pointer to nfs_fsloc data
 */
void
nfs_free_location(struct nfs_fsloc *location)
{
	nfs_free_string_array(location->nfl_rootpath);
	free(location->nfl_hostname);
	free(location);
}

/**
 * Free a list of NFS locations
 *
 * @param locations pointer to list of one or more locations
 */
void
nfs_free_locations(struct nfs_fsloc *locations)
{
	struct nfs_fsloc *fsloc;

	while (locations != NULL) {
		fsloc = locations;
		locations = fsloc->nfl_next;
		nfs_free_location(fsloc);
	}
}

/**
 * Allocate a fresh nfs_fsloc structure
 *
 * @return pointer to new empty nfs_fsloc data structure
 *
 * Caller must free returned locations with nfs_free_location().
 */
struct nfs_fsloc *
nfs_new_location(void)
{
	return calloc(1, sizeof(struct nfs_fsloc));
}
