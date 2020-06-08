/*
 * Copyright 2009 Oracle.  All rights reserved.
 * Copyright 2017 Red Hat, Inc.  All rights reserved.
 *
 * This file is part of nfs-utils.
 *
 * nfs-utils is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * nfs-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nfs-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>

#include <string.h>
#include <libgen.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#include "xlog.h"
#include "misc.h"

/*
 * Returns a dynamically allocated, '\0'-terminated buffer
 * containing an appropriate pathname, or NULL if an error
 * occurs.  Caller must free the returned result with free(3).
 */
__attribute__((__malloc__))
char *
generic_make_pathname(const char *base, const char *leaf)
{
	size_t size;
	char *path;
	int len;

	size = strlen(base) + strlen(leaf) + 2;
	if (size > PATH_MAX)
		return NULL;

	path = malloc(size);
	if (path == NULL)
		return NULL;

	len = snprintf(path, size, "%s/%s", base, leaf);
	if ((len < 0) || ((size_t)len >= size)) {
		free(path);
		return NULL;
	}

	return path;
}


/**
 * generic_setup_basedir - set up basedir
 * @progname: C string containing name of program, for error messages
 * @parentdir: C string containing pathname to on-disk state, or NULL
 * @base: character buffer to contain the basedir that is set up
 * @baselen: size of @base in bytes
 *
 * This runs before logging is set up, so error messages are directed
 * to stderr.
 *
 * Returns true and sets up our basedir, if @parentdir was valid
 * and usable; otherwise false is returned.
 */
_Bool
generic_setup_basedir(const char *progname, const char *parentdir, char *base,
		      const size_t baselen)
{
	static char buf[PATH_MAX];
	struct stat st;
	char *path;

	/* First: test length of name and whether it exists */
	if ((strlen(parentdir) >= baselen) || (strlen(parentdir) >= PATH_MAX)) {
		(void)fprintf(stderr, "%s: Directory name too long: %s",
				progname, parentdir);
		return false;
	}
	if (lstat(parentdir, &st) == -1) {
		(void)fprintf(stderr, "%s: Failed to stat %s: %s",
				progname, parentdir, strerror(errno));
		return false;
	}

	/* Ensure we have a clean directory pathname */
	strncpy(buf, parentdir, sizeof(buf)-1);
	path = dirname(buf);
	if (*path == '.') {
		(void)fprintf(stderr, "%s: Unusable directory %s",
				progname, parentdir);
		return false;
	}

	xlog(D_CALL, "Using %s as the state directory", parentdir);
	strcpy(base, parentdir);
	return true;
}
