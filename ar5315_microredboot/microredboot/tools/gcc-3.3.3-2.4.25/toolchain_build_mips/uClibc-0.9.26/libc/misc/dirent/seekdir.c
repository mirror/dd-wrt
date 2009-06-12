#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "dirstream.h"


void seekdir(DIR * dir, long int offset)
{
	if (!dir) {
		__set_errno(EBADF);
		return;
	}
#ifdef __UCLIBC_HAS_THREADS__
	__pthread_mutex_lock(&(dir->dd_lock));
#endif
	dir->dd_nextoff = lseek(dir->dd_fd, offset, SEEK_SET);
	dir->dd_size = dir->dd_nextloc = 0;
#ifdef __UCLIBC_HAS_THREADS__
	__pthread_mutex_unlock(&(dir->dd_lock));
#endif
}
