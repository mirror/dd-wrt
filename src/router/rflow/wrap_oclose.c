#include "headers.h"
#include <stdarg.h>
#include "wrap_oclose.h"

pthread_mutex_t fd_pool_mutex;	/* This must always be visible */

#ifndef	HAVE_WEAK_OPEN
#ifdef	PSRC_bpf
#warning Your platform does not have open() as a weak alias for _open() the standard C library.
#warning Practically this means that in order for ipcad to work chroot()ed
#warning in presence of interfaces that are not configured when the ipcad starts up,
#warning you would need to create dev/bpf* devices under the chroot directory.
#warning If you did not understand this, you are probably fine.
#endif	/* PSRC_bpf */
#else	/* HAVE_WEAK_OPEN */

/*
 * Real open()/close() defined as weak aliases for _open()/_close().
 * These are their prototypes.
 */
int _open(const char *path, int flags, ...);
int _close(int d);

/*
 * This pool is established to reuse /dev/bpfX file descriptors in case
 * chroot() makes /dev/bpf* devices unavailable.
 */
#define	FD_USED_SIZE	16384
typedef struct bpf_fd_s {
	int fd;
	struct bpf_fd_s *next;
} bpf_fd_t;
static bpf_fd_t *fds_pool_head;	/* File descriptors cache */
static bpf_fd_t *fds_used[FD_USED_SIZE];	/* Currently being used */

int
open(const char *path, int flags, ...) {
	int special;
	int d;

	if(flags & O_CREAT) {
		va_list ap;
		mode_t mode;
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
		return _open(path, flags, mode);
	}

	/*
	 * Enable file descriptors caching (special treatment mode)
	 * if path contains known pattern.
	 */
	special = (path && strncmp(path, "/dev/bpf", 8) == 0)?1:0;

	if(special) {
		pthread_mutex_lock(&fd_pool_mutex);
		if(fds_pool_head) {
			bpf_fd_t *bfd = fds_pool_head;
			fds_pool_head = bfd->next;
			bfd->next = 0;
			assert(fds_used[bfd->fd] == NULL);
			fds_used[bfd->fd] = bfd;
			pthread_mutex_unlock(&fd_pool_mutex);
			return bfd->fd;	/* Use cached copy */
		}
		pthread_mutex_unlock(&fd_pool_mutex);
	}

	d = _open(path, flags);
	if(d == -1)
		return -1;

	if(special && d >= 0 && d < FD_USED_SIZE) {
		bpf_fd_t *bfd = malloc(sizeof(*bfd));
		if(bfd == NULL) {
			close(d);
			errno = EPERM;	/* Simulate real problem */
			return -1;
		}
		bfd->fd = d;
		bfd->next = 0;
		assert(fds_used[d] == NULL);
		fds_used[d] = bfd;	/* Lock-free is OK */
	}

	return d;
}

int
close(int d) {
	if(d >= 0 && d < FD_USED_SIZE) {
		pthread_mutex_lock(&fd_pool_mutex);
		if(fds_used[d]) {
			/*
			 * Instead of closing, put back into the pool.
			 */
			fds_used[d]->next = fds_pool_head;
			fds_pool_head = fds_used[d];
			fds_used[d] = NULL;
			pthread_mutex_unlock(&fd_pool_mutex);
			return 0;
		}
		pthread_mutex_unlock(&fd_pool_mutex);
	}
	return _close(d);
}

#endif	/* HAVE_WEAK_OPEN */
