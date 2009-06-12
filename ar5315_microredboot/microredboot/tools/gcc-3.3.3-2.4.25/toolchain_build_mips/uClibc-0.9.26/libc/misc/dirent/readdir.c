#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "dirstream.h"


struct dirent *readdir(DIR * dir)
{
	ssize_t bytes;
	struct dirent *de;

	if (!dir) {
		__set_errno(EBADF);
		return NULL;
	}

#ifdef __UCLIBC_HAS_THREADS__
	__pthread_mutex_lock(&(dir->dd_lock));
#endif

	do {
	    if (dir->dd_size <= dir->dd_nextloc) {
		/* read dir->dd_max bytes of directory entries. */
		bytes = __getdents(dir->dd_fd, dir->dd_buf, dir->dd_max);
		if (bytes <= 0) {
		    de = NULL;
		    goto all_done;
		}
		dir->dd_size = bytes;
		dir->dd_nextloc = 0;
	    }

	    de = (struct dirent *) (((char *) dir->dd_buf) + dir->dd_nextloc);

	    /* Am I right? H.J. */
	    dir->dd_nextloc += de->d_reclen;

	    /* We have to save the next offset here. */
	    dir->dd_nextoff = de->d_off;

	    /* Skip deleted files.  */
	} while (de->d_ino == 0);

all_done:
#ifdef __UCLIBC_HAS_THREADS__
	__pthread_mutex_unlock(&(dir->dd_lock));
#endif
	return de;
}
