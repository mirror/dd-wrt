#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include "nfslib.h"
#include "xstat.h"

#ifdef HAVE_FSTATAT
#ifdef HAVE_STATX

static void
statx_copy(struct stat *stbuf, const struct statx *stxbuf)
{
	stbuf->st_dev = makedev(stxbuf->stx_dev_major, stxbuf->stx_dev_minor);
	stbuf->st_ino = stxbuf->stx_ino;
	stbuf->st_mode = stxbuf->stx_mode;
	stbuf->st_nlink = stxbuf->stx_nlink;
	stbuf->st_uid = stxbuf->stx_uid;
	stbuf->st_gid = stxbuf->stx_gid;
	stbuf->st_rdev = makedev(stxbuf->stx_rdev_major, stxbuf->stx_rdev_minor);
	stbuf->st_size = stxbuf->stx_size;
	stbuf->st_blksize = stxbuf->stx_blksize;
	stbuf->st_blocks = stxbuf->stx_blocks;
	stbuf->st_atim.tv_sec = stxbuf->stx_atime.tv_sec;
	stbuf->st_atim.tv_nsec = stxbuf->stx_atime.tv_nsec;
	stbuf->st_mtim.tv_sec = stxbuf->stx_mtime.tv_sec;
	stbuf->st_mtim.tv_nsec = stxbuf->stx_mtime.tv_nsec;
	stbuf->st_ctim.tv_sec = stxbuf->stx_ctime.tv_sec;
	stbuf->st_ctim.tv_nsec = stxbuf->stx_ctime.tv_nsec;
}

static int
statx_do_stat(int fd, const char *pathname, struct stat *statbuf, int flags)
{
	static int statx_supported = 1;
	struct statx stxbuf;
	int ret;

	if (statx_supported) {
		ret = statx(fd, pathname, flags,
				STATX_BASIC_STATS,
				&stxbuf);
		if (ret == 0) {
			statx_copy(statbuf, &stxbuf);
			return 0;
		}
		/* glibc emulation doesn't support AT_STATX_DONT_SYNC */
		if (errno == EINVAL)
			errno = ENOSYS;
		if (errno == ENOSYS)
			statx_supported = 0;
	} else
		errno = ENOSYS;
	return -1;
}

static int
statx_stat_nosync(int fd, const char *pathname, struct stat *statbuf, int flags)
{
	return statx_do_stat(fd, pathname, statbuf, flags | AT_STATX_DONT_SYNC);
}

#else

static int
statx_stat_nosync(int UNUSED(fd), const char *UNUSED(pathname), struct stat *UNUSED(statbuf), int UNUSED(flags))
{
	errno = ENOSYS;
	return -1;
}

#endif /* HAVE_STATX */

int xlstat(const char *pathname, struct stat *statbuf)
{
	if (statx_stat_nosync(AT_FDCWD, pathname, statbuf, AT_NO_AUTOMOUNT|
				AT_SYMLINK_NOFOLLOW) == 0)
		return 0;
	else if (errno != ENOSYS)
		return -1;
	errno = 0;
	return fstatat(AT_FDCWD, pathname, statbuf, AT_NO_AUTOMOUNT |
			AT_SYMLINK_NOFOLLOW);
}

int xstat(const char *pathname, struct stat *statbuf)
{
	if (statx_stat_nosync(AT_FDCWD, pathname, statbuf, AT_NO_AUTOMOUNT) == 0)
		return 0;
	else if (errno != ENOSYS)
		return -1;
	errno = 0;
	return fstatat(AT_FDCWD, pathname, statbuf, AT_NO_AUTOMOUNT);
}

#else

int xlstat(const char *pathname, struct stat *statbuf)
{
	return lstat(pathname, statbuf);
}

int xstat(const char *pathname, struct stat *statbuf)
{
	return stat(pathname, statbuf);
}
#endif
