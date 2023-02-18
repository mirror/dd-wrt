#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "conffile.h"
#include "xmalloc.h"
#include "xlog.h"
#include "xstat.h"
#include "nfslib.h"
#include "nfsd_path.h"
#include "workqueue.h"

static struct xthread_workqueue *nfsd_wq;

static int
nfsd_path_isslash(const char *path)
{
	return path[0] == '/' && path[1] == '/';
}

static int
nfsd_path_isdot(const char *path)
{
	return path[0] == '.' && path[1] == '/';
}

static const char *
nfsd_path_strip(const char *path)
{
	if (!path || *path == '\0')
		goto out;
	for (;;) {
		if (nfsd_path_isslash(path)) {
			path++;
			continue;
		}
		if (nfsd_path_isdot(path)) {
			path += 2;
			continue;
		}
		break;
	}
out:
	return path;
}

const char *
nfsd_path_nfsd_rootdir(void)
{
	const char *rootdir;

	rootdir = nfsd_path_strip(conf_get_str("exports", "rootdir"));
	if (!rootdir || rootdir[0] ==  '\0')
		return NULL;
	if (rootdir[0] == '/' && rootdir[1] == '\0')
		return NULL;
	return rootdir;
}

char *
nfsd_path_strip_root(char *pathname)
{
	char buffer[PATH_MAX];
	const char *dir = nfsd_path_nfsd_rootdir();

	if (!dir)
		goto out;

	if (realpath(dir, buffer))
		return strstr(pathname, buffer) == pathname ?
			pathname + strlen(buffer) : NULL;

	xlog(D_GENERAL, "%s: failed to resolve path %s: %m", __func__, dir);
out:
	return pathname;
}

char *
nfsd_path_prepend_dir(const char *dir, const char *pathname)
{
	size_t len, dirlen;
	char *ret;

	dirlen = strlen(dir);
	while (dirlen > 0 && dir[dirlen - 1] == '/')
		dirlen--;
	if (!dirlen)
		return NULL;
	while (pathname[0] == '/')
		pathname++;
	len = dirlen + strlen(pathname) + 1;
	ret = xmalloc(len + 1);
	snprintf(ret, len+1, "%.*s/%s", (int)dirlen, dir, pathname);
	return ret;
}

static void
nfsd_setup_workqueue(void)
{
	const char *rootdir = nfsd_path_nfsd_rootdir();

	if (!rootdir)
		return;

	nfsd_wq = xthread_workqueue_alloc();
	if (!nfsd_wq)
		return;
	xthread_workqueue_chroot(nfsd_wq, rootdir);
}

void
nfsd_path_init(void)
{
	nfsd_setup_workqueue();
}

struct nfsd_stat_data {
	const char *pathname;
	struct stat *statbuf;
	int ret;
	int err;
};

static void
nfsd_statfunc(void *data)
{
	struct nfsd_stat_data *d = data;

	d->ret = xstat(d->pathname, d->statbuf);
	if (d->ret < 0)
		d->err = errno;
}

static void
nfsd_lstatfunc(void *data)
{
	struct nfsd_stat_data *d = data;

	d->ret = xlstat(d->pathname, d->statbuf);
	if (d->ret < 0)
		d->err = errno;
}

static int
nfsd_run_stat(struct xthread_workqueue *wq,
		void (*func)(void *),
		const char *pathname,
		struct stat *statbuf)
{
	struct nfsd_stat_data data = {
		pathname,
		statbuf,
		0,
		0
	};
	xthread_work_run_sync(wq, func, &data);
	if (data.ret < 0)
		errno = data.err;
	return data.ret;
}

int
nfsd_path_stat(const char *pathname, struct stat *statbuf)
{
	if (!nfsd_wq)
		return xstat(pathname, statbuf);
	return nfsd_run_stat(nfsd_wq, nfsd_statfunc, pathname, statbuf);
}

int
nfsd_path_lstat(const char *pathname, struct stat *statbuf)
{
	if (!nfsd_wq)
		return xlstat(pathname, statbuf);
	return nfsd_run_stat(nfsd_wq, nfsd_lstatfunc, pathname, statbuf);
}

struct nfsd_statfs64_data {
	const char *pathname;
	struct statfs64 *statbuf;
	int ret;
	int err;
};

static void
nfsd_statfs64func(void *data)
{
	struct nfsd_statfs64_data *d = data;

	d->ret = statfs64(d->pathname, d->statbuf);
	if (d->ret < 0)
		d->err = errno;
}

static int
nfsd_run_statfs64(struct xthread_workqueue *wq,
		  const char *pathname,
		  struct statfs64 *statbuf)
{
	struct nfsd_statfs64_data data = {
		pathname,
		statbuf,
		0,
		0
	};
	xthread_work_run_sync(wq, nfsd_statfs64func, &data);
	if (data.ret < 0)
		errno = data.err;
	return data.ret;
}

int
nfsd_path_statfs64(const char *pathname, struct statfs64 *statbuf)
{
	if (!nfsd_wq)
		return statfs64(pathname, statbuf);
	return nfsd_run_statfs64(nfsd_wq, pathname, statbuf);
}

struct nfsd_realpath_data {
	const char *pathname;
	char *resolved;
	int err;
};

static void
nfsd_realpathfunc(void *data)
{
	struct nfsd_realpath_data *d = data;

	d->resolved = realpath(d->pathname, d->resolved);
	if (!d->resolved)
		d->err = errno;
}

char *
nfsd_realpath(const char *path, char *resolved_path)
{
	struct nfsd_realpath_data data = {
		path,
		resolved_path,
		0
	};

	if (!nfsd_wq)
		return realpath(path, resolved_path);

	xthread_work_run_sync(nfsd_wq, nfsd_realpathfunc, &data);
	if (!data.resolved)
		errno = data.err;
	return data.resolved;
}

struct nfsd_read_data {
	int fd;
	char *buf;
	size_t len;
	ssize_t ret;
	int err;
};

static void
nfsd_readfunc(void *data)
{
	struct nfsd_read_data *d = data;

	d->ret = read(d->fd, d->buf, d->len);
	if (d->ret < 0)
		d->err = errno;
}

static ssize_t
nfsd_run_read(struct xthread_workqueue *wq, int fd, char *buf, size_t len)
{
	struct nfsd_read_data data = {
		fd,
		buf,
		len,
		0,
		0
	};
	xthread_work_run_sync(wq, nfsd_readfunc, &data);
	if (data.ret < 0)
		errno = data.err;
	return data.ret;
}

ssize_t
nfsd_path_read(int fd, char *buf, size_t len)
{
	if (!nfsd_wq)
		return read(fd, buf, len);
	return nfsd_run_read(nfsd_wq, fd, buf, len);
}

struct nfsd_write_data {
	int fd;
	const char *buf;
	size_t len;
	ssize_t ret;
	int err;
};

static void
nfsd_writefunc(void *data)
{
	struct nfsd_write_data *d = data;

	d->ret = write(d->fd, d->buf, d->len);
	if (d->ret < 0)
		d->err = errno;
}

static ssize_t
nfsd_run_write(struct xthread_workqueue *wq, int fd, const char *buf, size_t len)
{
	struct nfsd_write_data data = {
		fd,
		buf,
		len,
		0,
		0
	};
	xthread_work_run_sync(wq, nfsd_writefunc, &data);
	if (data.ret < 0)
		errno = data.err;
	return data.ret;
}

ssize_t
nfsd_path_write(int fd, const char *buf, size_t len)
{
	if (!nfsd_wq)
		return write(fd, buf, len);
	return nfsd_run_write(nfsd_wq, fd, buf, len);
}

#if defined(HAVE_NAME_TO_HANDLE_AT)
struct nfsd_handle_data {
	int fd;
	const char *path;
	struct file_handle *fh;
	int *mount_id;
	int flags;
	int ret;
	int err;
};

static void
nfsd_name_to_handle_func(void *data)
{
	struct nfsd_handle_data *d = data;

	d->ret = name_to_handle_at(d->fd, d->path,
			d->fh, d->mount_id, d->flags);
	if (d->ret < 0)
		d->err = errno;
}

static int
nfsd_run_name_to_handle_at(struct xthread_workqueue *wq,
		int fd, const char *path, struct file_handle *fh,
		int *mount_id, int flags)
{
	struct nfsd_handle_data data = {
		fd,
		path,
		fh,
		mount_id,
		flags,
		0,
		0
	};

	xthread_work_run_sync(wq, nfsd_name_to_handle_func, &data);
	if (data.ret < 0)
		errno = data.err;
	return data.ret;
}

int
nfsd_name_to_handle_at(int fd, const char *path, struct file_handle *fh,
		int *mount_id, int flags)
{
	if (!nfsd_wq)
		return name_to_handle_at(fd, path, fh, mount_id, flags);

	return nfsd_run_name_to_handle_at(nfsd_wq, fd, path, fh,
			mount_id, flags);
}
#else
int
nfsd_name_to_handle_at(int UNUSED(fd), const char *UNUSED(path),
		struct file_handle *UNUSED(fh),
		int *UNUSED(mount_id), int UNUSED(flags))
{
	errno = ENOSYS;
	return -1;
}
#endif
