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

static struct xthread_workqueue *nfsd_wq = NULL;

struct nfsd_task_t {
        int             ret;
        void*           data;
};
/* Function used to offload tasks that must be ran within the correct
 * chroot environment.
 */
static void
nfsd_run_task(void (*func)(void*), void* data){
        nfsd_wq ? xthread_work_run_sync(nfsd_wq, func, data) : func(data);
};


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
	const char      *pathname;
	struct stat     *statbuf;
        int             (*stat_handler)(const char*, struct stat*);
};

static void
nfsd_handle_stat(void *data)
{
        struct nfsd_task_t*     t = data;
	struct nfsd_stat_data*  d = t->data;
        t->ret = d->stat_handler(d->pathname, d->statbuf);
}

static int
nfsd_run_stat(const char *pathname,
	        struct stat *statbuf,
                int (*handler)(const char*, struct stat*))
{
        struct nfsd_task_t      t;
        struct nfsd_stat_data   d = { pathname, statbuf, handler };
        t.data = &d;
        nfsd_run_task(nfsd_handle_stat, &t);
	return t.ret;
}

int
nfsd_path_stat(const char *pathname, struct stat *statbuf)
{
        return nfsd_run_stat(pathname, statbuf, stat);
}

int
nfsd_path_lstat(const char* pathname, struct stat* statbuf){
        return nfsd_run_stat(pathname, statbuf, lstat);
};

int
nfsd_path_statfs(const char* pathname, struct statfs* statbuf)
{
        return nfsd_run_stat(pathname, (struct stat*)statbuf, (int (*)(const char*, struct stat*))statfs);
};

struct nfsd_realpath_t {
        const char*     path;
        char*           resolved_buf;
        char*           res_ptr;
};

static void
nfsd_realpathfunc(void *data)
{
        struct nfsd_realpath_t *d = data;
        d->res_ptr = realpath(d->path, d->resolved_buf);
}

char*
nfsd_realpath(const char *path, char *resolved_buf)
{
        struct nfsd_realpath_t realpath_buf = {
                .path = path,
                .resolved_buf = resolved_buf
        };
        nfsd_run_task(nfsd_realpathfunc, &realpath_buf);
        return realpath_buf.res_ptr;
}

struct nfsd_rw_data {
	int             fd;
	void*           buf;
	size_t          len;
        ssize_t         bytes_read;
};

static void
nfsd_readfunc(void *data)
{
        struct nfsd_rw_data* t = (struct nfsd_rw_data*)data;
        t->bytes_read = read(t->fd, t->buf, t->len);
}

static ssize_t
nfsd_run_read(int fd, void* buf, size_t len)
{
        struct nfsd_rw_data d = { .fd = fd, .buf = buf, .len = len };
        nfsd_run_task(nfsd_readfunc, &d);
	return d.bytes_read;
}

ssize_t
nfsd_path_read(int fd, void* buf, size_t len)
{
	return nfsd_run_read(fd, buf, len);
}

static void
nfsd_writefunc(void *data)
{
	struct nfsd_rw_data* d = data;
	d->bytes_read = write(d->fd, d->buf, d->len);
}

static ssize_t
nfsd_run_write(int fd, void* buf, size_t len)
{
        struct nfsd_rw_data d = { .fd = fd, .buf = buf, .len = len };
        nfsd_run_task(nfsd_writefunc, &d);
	return d.bytes_read;
}

ssize_t
nfsd_path_write(int fd, void* buf, size_t len)
{
	return nfsd_run_write(fd, buf, len);
}

#if defined(HAVE_NAME_TO_HANDLE_AT)
struct nfsd_handle_data {
	int fd;
	const char *path;
	struct file_handle *fh;
	int *mount_id;
	int flags;
	int ret;
};

static void
nfsd_name_to_handle_func(void *data)
{
	struct nfsd_handle_data *d = data;
	d->ret = name_to_handle_at(d->fd, d->path, d->fh, d->mount_id, d->flags);
}

static int
nfsd_run_name_to_handle_at(int fd, const char *path,
                struct file_handle *fh,
		int *mount_id, int flags)
{
	struct nfsd_handle_data data = {
		fd,
		path,
		fh,
		mount_id,
		flags,
		0
	};

	nfsd_run_task(nfsd_name_to_handle_func, &data);
	return data.ret;
}

int
nfsd_name_to_handle_at(int fd, const char *path,
                struct file_handle *fh,
		int *mount_id, int flags)
{
        return nfsd_run_name_to_handle_at(fd, path, fh, mount_id, flags);
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
