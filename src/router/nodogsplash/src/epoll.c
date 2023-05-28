
#include <sys/syscall.h>
#include <sys/epoll.h>
#include <sys/unistd.h>

#ifndef __NR_epoll_create
#define __NR_epoll_create		(__NR_Linux + 248)
#define __NR_epoll_ctl			(__NR_Linux + 249)
#define __NR_epoll_wait			(__NR_Linux + 250)


#ifdef __UCLIBC_HAS_THREADS_NATIVE__
# include <sysdep-cancel.h>
#else
# define SINGLE_THREAD_P 1
#endif

int epoll_create(int size)
{
	return syscall(__NR_epoll_create, size);
}

int epoll_ctl(int fd, int op, int fd2, struct epoll_event *ev)
{
	return syscall(__NR_epoll_ctl, fd, op, fd2, ev);
}

int epoll_wait(int fd, struct epoll_event *ev, int cnt, int to)
{
	if (SINGLE_THREAD_P)
		return syscall(__NR_epoll_wait, fd, ev, cnt, to);
# ifdef __UCLIBC_HAS_THREADS_NATIVE__
	else {
		int oldtype = LIBC_CANCEL_ASYNC ();
		int result = syscall(__NR_epoll_wait, fd, ev, cnt, to);
		LIBC_CANCEL_RESET (oldtype);
		return result;
	]
# endif
}

#endif