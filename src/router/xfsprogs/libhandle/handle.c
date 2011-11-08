/*
 * Copyright (c) 1995, 2001-2003, 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <libgen.h>
#include <xfs/xfs.h>
#include <xfs/handle.h>
#include <xfs/parent.h>

/* just pick a value we know is more than big enough */
#define	MAXHANSIZ	64

/*
 *  The actual content of a handle is supposed to be opaque here.
 *  But, to do handle_to_fshandle, we need to know what it is.  Sigh.
 *  However we can get by with knowing only that the first 8 bytes of
 *  a file handle are the file system ID, and that a file system handle
 *  consists of only those 8 bytes.
 */

#define	FSIDSIZE	8

typedef union {
	int	fd;
	char	*path;
} comarg_t;

static int obj_to_handle(char *, int, unsigned int, comarg_t, void**, size_t*);
static int handle_to_fsfd(void *, char **);
static char *path_to_fspath(char *path);


/*
 * Filesystem Handle -> Open File Descriptor Cache
 *
 * Maps filesystem handles to a corresponding open file descriptor for that
 * filesystem. We need this because we're doing handle operations via xfsctl
 * and we need to remember the open file descriptor for each filesystem.
 */

struct fdhash {
	int	fsfd;
	char	fsh[FSIDSIZE];
	struct fdhash *fnxt;
	char	fspath[MAXPATHLEN];
};

static struct fdhash *fdhash_head = NULL;

int
path_to_fshandle(
	char		*path,		/* input,  path to convert */
	void		**fshanp,	/* output, pointer to data */
	size_t		*fshlen)	/* output, size of returned data */
{
	int		result;
	int		fd;
	comarg_t	obj;
	struct fdhash	*fdhp;
	char		*tmppath;
	char		*fspath;

	fspath = path_to_fspath(path);
	if (fspath == NULL)
		return -1;

	fd = open(fspath, O_RDONLY);
	if (fd < 0)
		return -1;

	obj.path = path;
	result = obj_to_handle(fspath, fd, XFS_IOC_PATH_TO_FSHANDLE,
				obj, fshanp, fshlen);
	if (result < 0) {
		close(fd);
		return result;
	}

	if (handle_to_fsfd(*fshanp, &tmppath) >= 0) {
		/* this filesystem is already in the cache */
		close(fd);
	} else {
		/* new filesystem. add it to the cache */
		fdhp = malloc(sizeof(struct fdhash));
		if (fdhp == NULL) {
			errno = ENOMEM;
			return -1;
		}

		fdhp->fsfd = fd;
		strncpy(fdhp->fspath, fspath, sizeof(fdhp->fspath));
		memcpy(fdhp->fsh, *fshanp, FSIDSIZE);

		fdhp->fnxt = fdhash_head;
		fdhash_head = fdhp;
	}

	return result;
}

int
path_to_handle(
	char		*path,		/* input,  path to convert */
	void		**hanp,		/* output, pointer to data */
	size_t		*hlen)		/* output, size of returned data */
{
	int		fd;
	int		result;
	comarg_t	obj;
	char		*fspath;

	fspath = path_to_fspath(path);
	if (fspath == NULL)
		return -1;

	fd = open(fspath, O_RDONLY);
	if (fd < 0)
		return -1;

	obj.path = path;
	result = obj_to_handle(fspath, fd, XFS_IOC_PATH_TO_HANDLE,
				obj, hanp, hlen);
	close(fd);
	return result;
}

/* Given a path, return a suitable "fspath" for use in obtaining
 * an fd for xfsctl calls. For regular files and directories the
 * input path is sufficient. For other types the parent directory
 * is used to avoid issues with opening dangling symlinks and
 * potentially blocking in an open on a named pipe. Also
 * symlinks to files on other filesystems would be a problem,
 * since an fd would be obtained for the wrong fs.
 */
static char *
path_to_fspath(char *path)
{
	static char dirpath[MAXPATHLEN];
	struct stat statbuf;

	if (lstat(path, &statbuf) != 0)
		return NULL;

	if (S_ISREG(statbuf.st_mode) || S_ISDIR(statbuf.st_mode))
		return path;

	strcpy(dirpath, path);
	return dirname(dirpath);
}

int
fd_to_handle (
	int		fd,		/* input,  file descriptor */
	void		**hanp,		/* output, pointer to data */
	size_t		*hlen)		/* output, size of returned data */
{
	comarg_t	obj;

	obj.fd = fd;
	return obj_to_handle(NULL, fd, XFS_IOC_FD_TO_HANDLE, obj, hanp, hlen);
}


int
handle_to_fshandle(
	void		*hanp,
	size_t		hlen,
	void		**fshanp,
	size_t		*fshlen)
{
	if (hlen < FSIDSIZE) {
		errno = EINVAL;
		return -1;
	}
	*fshanp = malloc(FSIDSIZE);
	if (*fshanp == NULL) {
		errno = ENOMEM;
		return -1;
	}
	*fshlen = FSIDSIZE;
	memcpy(*fshanp, hanp, FSIDSIZE);
	return 0;
}

static int
handle_to_fsfd(void *hanp, char **path)
{
	struct fdhash	*fdhp;

	/*
	 * Look in cache for matching fsid field in the handle
	 * (which is at the start of the handle).
	 * When found return the file descriptor and path that
	 * we have in the cache.
	 */
	for (fdhp = fdhash_head; fdhp != NULL; fdhp = fdhp->fnxt) {
		if (memcmp(fdhp->fsh, hanp, FSIDSIZE) == 0) {
			*path = fdhp->fspath;
			return fdhp->fsfd;
		}
	}
	errno = EBADF;
	return -1;
}

static int
obj_to_handle(
	char		*fspath,
	int		fsfd,
	unsigned int	opcode,
	comarg_t	obj,
	void		**hanp,
	size_t		*hlen)
{
	char		hbuf [MAXHANSIZ];
	int		ret;
	__uint32_t	handlen;
	xfs_fsop_handlereq_t hreq;

	if (opcode == XFS_IOC_FD_TO_HANDLE) {
		hreq.fd      = obj.fd;
		hreq.path    = NULL;
	} else {
		hreq.fd      = 0;
		hreq.path    = obj.path;
	}

	hreq.oflags   = O_LARGEFILE;
	hreq.ihandle  = NULL;
	hreq.ihandlen = 0;
	hreq.ohandle  = hbuf;
	hreq.ohandlen = &handlen;

	ret = xfsctl(fspath, fsfd, opcode, &hreq);
	if (ret)
		return ret;

	*hanp = malloc(handlen);
	if (*hanp == NULL) {
		errno = ENOMEM;
		return -1;
	}

	memcpy(*hanp, hbuf, handlen);
	*hlen = handlen;
	return 0;
}

int
open_by_fshandle(
	void		*fshanp,
	size_t		fshlen,
	int		rw)
{
	int		fsfd;
	char		*path;
	xfs_fsop_handlereq_t hreq;

	if ((fsfd = handle_to_fsfd(fshanp, &path)) < 0)
		return -1;

	hreq.fd       = 0;
	hreq.path     = NULL;
	hreq.oflags   = rw | O_LARGEFILE;
	hreq.ihandle  = fshanp;
	hreq.ihandlen = fshlen;
	hreq.ohandle  = NULL;
	hreq.ohandlen = NULL;

	return xfsctl(path, fsfd, XFS_IOC_OPEN_BY_HANDLE, &hreq);
}

int
open_by_handle(
	void		*hanp,
	size_t		hlen,
	int		rw)
{
	int		fsfd;
	char		*path;
	xfs_fsop_handlereq_t hreq;

	if ((fsfd = handle_to_fsfd(hanp, &path)) < 0)
		return -1;

	hreq.fd       = 0;
	hreq.path     = NULL;
	hreq.oflags   = rw | O_LARGEFILE;
	hreq.ihandle  = hanp;
	hreq.ihandlen = hlen;
	hreq.ohandle  = NULL;
	hreq.ohandlen = NULL;

	return xfsctl(path, fsfd, XFS_IOC_OPEN_BY_HANDLE, &hreq);
}

int
readlink_by_handle(
	void		*hanp,
	size_t		hlen,
	void		*buf,
	size_t		bufsiz)
{
	int		fd;
	__u32		buflen = (__u32)bufsiz;
	char		*path;
	xfs_fsop_handlereq_t hreq;

	if ((fd = handle_to_fsfd(hanp, &path)) < 0)
		return -1;

	hreq.fd       = 0;
	hreq.path     = NULL;
	hreq.oflags   = O_LARGEFILE;
	hreq.ihandle  = hanp;
	hreq.ihandlen = hlen;
	hreq.ohandle  = buf;
	hreq.ohandlen = &buflen;

	return xfsctl(path, fd, XFS_IOC_READLINK_BY_HANDLE, &hreq);
}

/*ARGSUSED4*/
int
attr_multi_by_handle(
	void		*hanp,
	size_t		hlen,
	void		*buf,
	int		rtrvcnt,
	int		flags)
{
	int		fd;
	char		*path;
	xfs_fsop_attrmulti_handlereq_t amhreq;

	if ((fd = handle_to_fsfd(hanp, &path)) < 0)
		return -1;

	amhreq.hreq.fd       = 0;
	amhreq.hreq.path     = NULL;
	amhreq.hreq.oflags   = O_LARGEFILE;
	amhreq.hreq.ihandle  = hanp;
	amhreq.hreq.ihandlen = hlen;
	amhreq.hreq.ohandle  = NULL;
	amhreq.hreq.ohandlen = NULL;

	amhreq.opcount = rtrvcnt;
	amhreq.ops = buf;

	return xfsctl(path, fd, XFS_IOC_ATTRMULTI_BY_HANDLE, &amhreq);
}

int
attr_list_by_handle(
	void		*hanp,
	size_t		hlen,
	void		*buf,
	size_t		bufsize,
	int		flags,
	struct attrlist_cursor *cursor)
{
	int		error, fd;
	char		*path;
	xfs_fsop_attrlist_handlereq_t alhreq;

	if ((fd = handle_to_fsfd(hanp, &path)) < 0)
		return -1;

	alhreq.hreq.fd       = 0;
	alhreq.hreq.path     = NULL;
	alhreq.hreq.oflags   = O_LARGEFILE;
	alhreq.hreq.ihandle  = hanp;
	alhreq.hreq.ihandlen = hlen;
	alhreq.hreq.ohandle  = NULL;
	alhreq.hreq.ohandlen = NULL;

	memcpy(&alhreq.pos, cursor, sizeof(alhreq.pos));
	alhreq.flags = flags;
	alhreq.buffer = buf;
	alhreq.buflen = bufsize;
	/* prevent needless EINVAL from the kernel */
	if (alhreq.buflen > XATTR_LIST_MAX)
		alhreq.buflen = XATTR_LIST_MAX;

	error = xfsctl(path, fd, XFS_IOC_ATTRLIST_BY_HANDLE, &alhreq);

	memcpy(cursor, &alhreq.pos, sizeof(alhreq.pos));
	return error;
}

int
parents_by_handle(
	void		*hanp,
	size_t		hlen,
	parent_t	*buf,
	size_t		bufsiz,
	unsigned int	*count)
	
{
	errno = EOPNOTSUPP;
	return -1;
}

int
parentpaths_by_handle(
	void		*hanp,
	size_t		hlen,
	parent_t	*buf,
	size_t		bufsiz,
	unsigned int	*count)
{
	errno = EOPNOTSUPP;
	return -1;
}

int
fssetdm_by_handle(
	void		*hanp,
	size_t		hlen,
	struct fsdmidata *fsdmidata)
{
	int		fd;
	char		*path;
	xfs_fsop_setdm_handlereq_t dmhreq;

	if ((fd = handle_to_fsfd(hanp, &path)) < 0)
		return -1;

	dmhreq.hreq.fd       = 0;
	dmhreq.hreq.path     = NULL;
	dmhreq.hreq.oflags   = O_LARGEFILE;
	dmhreq.hreq.ihandle  = hanp;
	dmhreq.hreq.ihandlen = hlen;
	dmhreq.hreq.ohandle  = NULL;
	dmhreq.hreq.ohandlen = NULL;

	dmhreq.data = fsdmidata;

	return xfsctl(path, fd, XFS_IOC_FSSETDM_BY_HANDLE, &dmhreq);
}

/*ARGSUSED1*/
void
free_handle(
	void		*hanp,
	size_t		hlen)
{
	free(hanp);
}
