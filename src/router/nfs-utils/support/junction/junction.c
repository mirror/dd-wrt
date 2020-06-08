/**
 * @file support/junction/junction.c
 * @brief Common utilities for managing junctions on the local file system
 */

/*
 * Copyright 2010, 2018 Oracle.  All rights reserved.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>
#include <memory.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>

#include <sys/xattr.h>

#include "junction.h"
#include "junction-internal.h"
#include "xlog.h"

/**
 * Open a file system object
 *
 * @param pathname NUL-terminated C string containing pathname of an object
 * @param fd OUT: a file descriptor number is filled in
 * @return a FedFsStatus code
 */
FedFsStatus
junction_open_path(const char *pathname, int *fd)
{
	int tmp;

	if (pathname == NULL || fd == NULL)
		return FEDFS_ERR_INVAL;

	tmp = open(pathname, O_DIRECTORY);
	if (tmp == -1) {
		switch (errno) {
		case EPERM:
			return FEDFS_ERR_ACCESS;
		case EACCES:
			return FEDFS_ERR_PERM;
		default:
			xlog(D_GENERAL, "%s: Failed to open path %s: %m",
				__func__, pathname);
			return FEDFS_ERR_INVAL;
		}
	}

	*fd = tmp;
	return FEDFS_OK;
}

/**
 * Predicate: is object a directory?
 *
 * @param fd an open file descriptor
 * @param path NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 */
FedFsStatus
junction_is_directory(int fd, const char *path)
{
	struct stat stb;

	if (fstat(fd, &stb) == -1) {
		xlog(D_GENERAL, "%s: failed to stat %s: %m",
				__func__, path);
		return FEDFS_ERR_ACCESS;
	}

	if (!S_ISDIR(stb.st_mode)) {
		xlog(D_CALL, "%s: %s is not a directory",
				__func__, path);
		return FEDFS_ERR_INVAL;
	}

	xlog(D_CALL, "%s: %s is a directory", __func__, path);
	return FEDFS_OK;
}

/**
 * Predicate: is a directory's sticky bit set?
 *
 * @param fd an open file descriptor
 * @param path NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 */
FedFsStatus
junction_is_sticky_bit_set(int fd, const char *path)
{
	struct stat stb;

	if (fstat(fd, &stb) == -1) {
		xlog(D_GENERAL, "%s: failed to stat %s: %m",
				__func__, path);
		return FEDFS_ERR_ACCESS;
	}

	if (stb.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) {
		xlog(D_CALL, "%s: execute bit set on %s",
				__func__, path);
		return FEDFS_ERR_NOTJUNCT;
	}

	if (!(stb.st_mode & S_ISVTX)) {
		xlog(D_CALL, "%s: sticky bit not set on %s",
				__func__, path);
		return FEDFS_ERR_NOTJUNCT;
	}

	xlog(D_CALL, "%s: sticky bit is set on %s", __func__, path);
	return FEDFS_OK;
}

/**
 * Set just a directory's sticky bit
 *
 * @param fd an open file descriptor
 * @param path NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 */
FedFsStatus
junction_set_sticky_bit(int fd, const char *path)
{
	struct stat stb;

	if (fstat(fd, &stb) == -1) {
		xlog(D_GENERAL, "%s: failed to stat %s: %m",
			__func__, path);
		return FEDFS_ERR_ACCESS;
	}

	stb.st_mode &= (unsigned int)~ALLPERMS;
	stb.st_mode |= S_ISVTX;

	if (fchmod(fd, stb.st_mode) == -1) {
		xlog(D_GENERAL, "%s: failed to set sticky bit on %s: %m",
			__func__, path);
		return FEDFS_ERR_ROFS;
	}

	xlog(D_CALL, "%s: set sticky bit on %s", __func__, path);
	return FEDFS_OK;
}

/**
 * Predicate: does a directory have an xattr named "name"?
 *
 * @param fd an open file descriptor
 * @param path NUL-terminated C string containing pathname of a directory
 * @param name NUL-terminated C string containing name of xattr to check
 * @return a FedFsStatus code
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
FedFsStatus
junction_is_xattr_present(int fd, const char *path, const char *name)
{
	ssize_t rc;

	/*
	 * Do not assume the total number of extended attributes
	 * this object may have.
	 */
	rc = fgetxattr(fd, name, NULL, 0);
	if (rc == -1) {
		switch (errno) {
		case EPERM:
			xlog(D_CALL, "%s: no access to xattr %s on %s",
				__func__, name, path);
			return FEDFS_ERR_PERM;
		case ENODATA:
			xlog(D_CALL, "%s: no xattr %s present on %s",
				__func__, name, path);
			return FEDFS_ERR_NOTJUNCT;
		default:
			xlog(D_CALL, "%s: xattr %s not found on %s: %m",
				__func__, name, path);
			return FEDFS_ERR_IO;
		}
	}

	xlog(D_CALL, "%s: xattr %s found on %s",
			__func__, name, path);
	return FEDFS_OK;
}

/**
 * Read the contents of xattr "name"
 *
 * @param fd an open file descriptor
 * @param path NUL-terminated C string containing pathname of a directory
 * @param name NUL-terminated C string containing name of xattr to retrieve
 * @param contents OUT: NUL-terminated C string containing contents of xattr
 * @return a FedFsStatus code
 *
 * If junction_read_xattr() returns FEDFS_OK, the caller must free "*contents"
 * with free(3).
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
FedFsStatus
junction_read_xattr(int fd, const char *path, const char *name, char **contents)
{
	char *xattrbuf = NULL;
	ssize_t len;

	len = fgetxattr(fd, name, xattrbuf, 0);
	if (len < 0) {
		xlog(D_GENERAL, "%s: failed to get size of xattr %s on %s: %m",
			__func__, name, path);
		return FEDFS_ERR_ACCESS;
	}

	xattrbuf = malloc((size_t)len + 1);
	if (xattrbuf == NULL) {
		xlog(D_GENERAL, "%s: failed to get buffer for xattr %s on %s",
			__func__, name, path);
		return FEDFS_ERR_SVRFAULT;
	}

	if (fgetxattr(fd, name, xattrbuf, (size_t)len) == -1) {
		xlog(D_GENERAL, "%s: failed to get xattr %s on %s: %m",
			__func__, name, path);
		free(xattrbuf);
		return FEDFS_ERR_ACCESS;
	}
	xattrbuf[len] = '\0';

	xlog(D_CALL, "%s: read xattr %s from path %s",
			__func__, name, path);
	*contents = xattrbuf;
	return FEDFS_OK;
}

/**
 * Retrieve the contents of xattr "name"
 *
 * @param fd an open file descriptor
 * @param path NUL-terminated C string containing pathname of a directory
 * @param name NUL-terminated C string containing name of xattr to retrieve
 * @param contents OUT: opaque byte array containing contents of xattr
 * @param contentlen OUT: size of "contents"
 * @return a FedFsStatus code
 *
 * If junction_get_xattr() returns FEDFS_OK, the caller must free "*contents"
 * with free(3).
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
FedFsStatus
junction_get_xattr(int fd, const char *path, const char *name, void **contents,
		size_t *contentlen)
{
	void *xattrbuf = NULL;
	ssize_t len;

	len = fgetxattr(fd, name, xattrbuf, 0);
	if (len < 0) {
		xlog(D_GENERAL, "%s: failed to get size of xattr %s on %s: %m",
			__func__, name, path);
		return FEDFS_ERR_ACCESS;
	}

	xattrbuf = malloc((size_t)len);
	if (xattrbuf == NULL) {
		xlog(D_GENERAL, "%s: failed to get buffer for xattr %s on %s",
			__func__, name, path);
		return FEDFS_ERR_SVRFAULT;
	}

	if (fgetxattr(fd, name, xattrbuf, (size_t)len) == -1) {
		xlog(D_GENERAL, "%s: failed to get xattr %s on %s: %m",
			__func__, name, path);
		free(xattrbuf);
		return FEDFS_ERR_ACCESS;
	}

	xlog(D_CALL, "%s: read xattr %s from path %s",
			__func__, name, path);
	*contents = xattrbuf;
	*contentlen = (size_t)len;
	return FEDFS_OK;
}

/**
 * Update the contents of an xattr
 *
 * @param fd an open file descriptor
 * @param path NUL-terminated C string containing pathname of a directory
 * @param name NUL-terminated C string containing name of xattr to set
 * @param contents opaque byte array containing contents of xattr
 * @param contentlen size of "contents"
 * @return a FedFsStatus code
 *
 * The extended attribute is created if it does not exist.
 * Its contents are replaced if it does.
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
FedFsStatus
junction_set_xattr(int fd, const char *path, const char *name,
			const void *contents, const size_t contentlen)
{
	/*
	 * XXX: Eventually should distinguish among several errors:
	 *	object isn't there, no root access, some other issue
	 */
	if (fsetxattr(fd, name, contents, contentlen, 0) == -1) {
		xlog(D_GENERAL, "%s: Failed to set xattr %s on %s: %m",
			__func__, name, path);
		return FEDFS_ERR_IO;
	}

	xlog(D_CALL, "%s: Wrote xattr %s from path %s",
			__func__, name, path);
	return FEDFS_OK;
}

/**
 * Remove one xattr
 *
 * @param fd an open file descriptor
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @param name NUL-terminated C string containing name of xattr to set
 * @return a FedFsStatus code
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
FedFsStatus
junction_remove_xattr(int fd, const char *pathname, const char *name)
{
	/*
	 * XXX: Eventually should distinguish among several errors:
	 *	object isn't there, no root access, some other issue
	 */
	if (fremovexattr(fd, name) == -1) {
		xlog(D_GENERAL, "%s: failed to remove xattr %s from %s: %m",
			__func__, name, pathname);
		return FEDFS_ERR_ACCESS;
	}
	xlog(D_CALL, "%s: removed xattr %s from path %s",
			__func__, name, pathname);
	return FEDFS_OK;
}

/**
 * Retrieve object's mode bits.
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @param mode OUT: mode bits
 * @return a FedFsStatus code
 */
FedFsStatus
junction_get_mode(const char *pathname, mode_t *mode)
{
	FedFsStatus retval;
	struct stat stb;
	int fd;

	retval = junction_open_path(pathname, &fd);
	if (retval != FEDFS_OK)
		return retval;

	if (fstat(fd, &stb) == -1) {
		xlog(D_GENERAL, "%s: failed to stat %s: %m",
			__func__, pathname);
		(void)close(fd);
		return FEDFS_ERR_ACCESS;
	}
	(void)close(fd);

	xlog(D_CALL, "%s: pathname %s has mode %o",
		__func__, pathname, stb.st_mode);
	*mode = stb.st_mode;
	return FEDFS_OK;

}

/**
 * Save the object's mode in an xattr.  Saved mode is human-readable.
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 */
FedFsStatus
junction_save_mode(const char *pathname)
{
	FedFsStatus retval;
	mode_t mode;
	char buf[8];
	int fd;

	retval = junction_get_mode(pathname, &mode);
	if (retval != FEDFS_OK)
		return retval;
	(void)snprintf(buf, sizeof(buf), "%o", ALLPERMS & mode);

	retval = junction_open_path(pathname, &fd);
	if (retval != FEDFS_OK)
		return retval;

	retval = junction_set_xattr(fd, pathname, JUNCTION_XATTR_NAME_MODE,
				buf, strlen(buf));
	if (retval != FEDFS_OK)
		goto out;

	retval = junction_set_sticky_bit(fd, pathname);
	if (retval != FEDFS_OK) {
		(void)junction_remove_xattr(fd, pathname,
						JUNCTION_XATTR_NAME_MODE);
		goto out;
	}

	xlog(D_CALL, "%s: saved mode %o to %s", __func__, mode, pathname);
	retval = FEDFS_OK;

out:
	(void)close(fd);
	return retval;

}

/**
 * Restore an object's mode bits
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 */
FedFsStatus
junction_restore_mode(const char *pathname)
{
	FedFsStatus retval;
	char *buf = NULL;
	mode_t mode;
	int fd;

	retval = junction_open_path(pathname, &fd);
	if (retval != FEDFS_OK)
		return retval;

	retval = junction_read_xattr(fd, pathname, JUNCTION_XATTR_NAME_MODE, &buf);
	if (retval != FEDFS_OK)
		goto out;

	retval = FEDFS_ERR_SVRFAULT;
	if (sscanf((char *)buf, "%o", &mode) != 1) {
		xlog(D_GENERAL, "%s: failed to parse saved mode on %s",
			__func__, pathname);
		goto out;
	}

	retval = FEDFS_ERR_ROFS;
	if (fchmod(fd, mode) == -1) {
		xlog(D_GENERAL, "%s: failed to set mode of %s to %o: %m",
			__func__, pathname, mode);
		goto out;
	}

	xlog(D_CALL, "%s: restored mode %o to %s", __func__, mode, pathname);
	retval = FEDFS_OK;

out:
	free(buf);
	(void)close(fd);
	return retval;
}
