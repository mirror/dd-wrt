/*
 * iod.c		- Iterate a function on each entry of a directory
 *
 * Copyright (C) 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                           Laboratoire MASI, Institut Blaise Pascal
 *                           Universite Pierre et Marie Curie (Paris VI)
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */

/*
 * History:
 * 93/10/30	- Creation
 */

#include "config.h"
#include "e2p.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>

#define max_long(a,b) ((((long) (a)) > ((long) (b))) ? (a) : (b))

int iterate_on_dir (const char * dir_name,
		    int (*func) (const char *, struct dirent *, void *),
		    void * private)
{
	DIR * dir;
	struct dirent *de, *dep;
	long	name_max = -1;
	int	max_len, len, ret = 0;

#if HAVE_PATHCONF && defined(_PC_NAME_MAX)
	name_max = pathconf(dir_name, _PC_NAME_MAX);
#endif
#ifdef _POSIX_NAME_MAX
	name_max = max_long(name_max, _POSIX_NAME_MAX);
#endif
#ifdef NAME_MAX
	name_max = max_long(name_max, NAME_MAX);
#endif
	name_max = max_long(name_max, 256);
	/* clamp name_max in case the OS returns something crazy */
	if (name_max > 65536)
		name_max = 65536;

	max_len = name_max + sizeof(struct dirent);
	de = malloc(max_len+1);
	if (!de)
		return -1;
	memset(de, 0, max_len+1);

	dir = opendir (dir_name);
	if (dir == NULL) {
		free(de);
		return -1;
	}
	while ((dep = readdir (dir))) {
#ifdef HAVE_RECLEN_DIRENT
		len = dep->d_reclen;
		if (len > max_len)
			len = max_len;
#else
		len = sizeof(struct dirent);
#endif
		memcpy(de, dep, len);
		if ((*func)(dir_name, de, private))
			ret++;
	}
	free(de);
	closedir(dir);
	return ret;
}
