/**
 * @file support/junction/export-cache.c
 * @brief Try to flush NFSD's exports cache
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


#include "junction.h"
#include "xlog.h"

/**
 * Ordered list of proc files to poke when requesting an NFSD cache flush
 */
static const char *junction_proc_files[] = {
	"/proc/net/rpc/auth.unix.ip/flush",
	"/proc/net/rpc/auth.unix.gid/flush",
	"/proc/net/rpc/nfsd.fh/flush",
	"/proc/net/rpc/nfsd.export/flush",
	NULL,
};

/**
 * Write time into one file
 *
 * @param pathname NUL-terminated C string containing POSIX pathname of file to write
 * @param flushtime NUL-terminated C string containing current time in seconds since the Epoch
 * @return a FedFsStatus code
 */
static FedFsStatus
junction_write_time(const char *pathname, const char *flushtime)
{
	FedFsStatus retval;
	ssize_t len;
	int fd;

	fd = open(pathname, O_RDWR);
	if (fd == -1) {
		xlog(D_GENERAL, "%s: Failed to open %s: %m",
			__func__, pathname);
		/* If the proc files don't exist, no server
		 * is running on this system */
		return FEDFS_ERR_NO_CACHE_UPDATE;
	}

	len = write(fd, flushtime, strlen(flushtime));
	if (len != (ssize_t)strlen(flushtime)) {
		xlog(D_GENERAL, "%s: Failed to write %s: %m",
			__func__, pathname);
		/* If the proc files exist but the update failed,
		 * we don't know the state of the cache */
		retval = FEDFS_ERR_UNKNOWN_CACHE;
	} else
		/* Cache flush succeeded */
		retval = FEDFS_OK;

	(void)close(fd);
	return retval;
}

/**
 * Flush the kernel NFSD's exports cache
 *
 * @return a FedFsStatus code
 */
FedFsStatus
junction_flush_exports_cache(void)
{
	FedFsStatus retval;
	char flushtime[20];
	unsigned int i;
	time_t now;

	xlog(D_CALL, "%s: Flushing NFSD caches...", __func__);

	now = time(NULL);
	if (now == -1) {
		xlog(D_GENERAL, "%s: time(3) failed", __func__);
		return FEDFS_ERR_SVRFAULT;
	}
	snprintf(flushtime, sizeof(flushtime), "%ld\n", now);

	for (i = 0; junction_proc_files[i] != NULL; i++) {
		retval = junction_write_time(junction_proc_files[i], flushtime);
		if (retval != FEDFS_OK)
			return retval;
	}
	return FEDFS_OK;
}
