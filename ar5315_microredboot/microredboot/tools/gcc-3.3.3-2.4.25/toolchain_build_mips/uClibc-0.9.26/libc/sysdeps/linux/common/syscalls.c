/* vi: set sw=4 ts=4: */
/*
 * Syscalls for uClibc
 *
 * Copyright (C) 2001-2003 by Erik Andersen
 * Written by Erik Andersen <andersen@codpoet.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <features.h>
#undef __OPTIMIZE__
/* We absolutely do _NOT_ want interfaces silently
 *  *  * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <endian.h>


//#define __NR_exit             1
//See _exit.c

//#define __NR_fork             2
#ifdef L___libc_fork
#include <unistd.h>
#	ifdef __UCLIBC_HAS_MMU__
#define __NR___libc_fork __NR_fork
		_syscall0(pid_t, __libc_fork);
#	else
		pid_t __libc_fork(void)
		{
			__set_errno(ENOSYS);
			return -1;
		}
#	endif
weak_alias (__libc_fork, fork)
#endif

//#define __NR_read             3
#ifdef L___libc_read
#include <unistd.h>
#define __NR___libc_read __NR_read
_syscall3(ssize_t, __libc_read, int, fd, __ptr_t, buf, size_t, count);
weak_alias(__libc_read, read)
#endif

//#define __NR_write            4
#ifdef L___libc_write
#include <unistd.h>
#define __NR___libc_write __NR_write
_syscall3(ssize_t, __libc_write, int, fd, const __ptr_t, buf, size_t, count);
weak_alias(__libc_write, write)
/* Stupid libgcc.a from gcc 2.95.x uses __write in pure.o
 * which is a blatent GNU libc-ism... */
weak_alias (__libc_write, __write)
#endif

//#define __NR_open             5
#ifdef L___syscall_open
#define __NR___syscall_open __NR_open
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <sys/param.h>
static inline
_syscall3(int, __syscall_open, const char *, file, int, flags, __kernel_mode_t, mode);
int __libc_open (const char * file, int flags, ...)
{
	mode_t mode;
	if (flags & O_CREAT) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
	return __syscall_open(file, flags, mode);
}
weak_alias(__libc_open, open)

int creat(const char *file, mode_t mode)
{
	  return __libc_open (file, O_WRONLY|O_CREAT|O_TRUNC, mode);
}
#endif

//#define __NR_close            6
#ifdef L___libc_close
#include <unistd.h>
#define __NR___libc_close __NR_close
_syscall1(int, __libc_close, int, fd);
weak_alias(__libc_close, close)
#endif

//#define __NR_waitpid          7
// Implemented using wait4 

//#define __NR_creat            8
// Implemented using open

//#define __NR_link             9
#ifdef L_link
#include <unistd.h>
_syscall2(int, link, const char *, oldpath, const char *, newpath);
#endif

//#define __NR_unlink           10
#ifdef L_unlink
#include <unistd.h>
_syscall1(int, unlink, const char *, pathname);
#endif

//#define __NR_execve           11
#ifdef L___syscall_execve
#define __NR___syscall_execve __NR_execve
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
static inline
_syscall3(int, __syscall_execve, const char *, filename, 
		char *const *, argv, char *const *, envp);
weak_alias(__syscall_execve, execve)
#endif

//#define __NR_chdir            12
#ifdef L___syscall_chdir
#define __NR___syscall_chdir __NR_chdir
#include <string.h>
#include <sys/param.h>
static inline
_syscall1(int, __syscall_chdir, const char *, path);
weak_alias(__syscall_chdir, chdir)
#endif

//#define __NR_time             13
#ifdef L_time
#include <time.h>
#include <sys/time.h>
#ifdef __NR_time
_syscall1(time_t, time, time_t *, t);
#else
time_t time (time_t *t)
{ 
	time_t result;
	struct timeval tv;
	if (gettimeofday (&tv, (struct timezone *) NULL)) {
		result = (time_t) -1;
	} else { result = (time_t) tv.tv_sec; }
	if (t != NULL) { *t = result; }
	return result;
}
#endif
#endif

//#define __NR_mknod            14
#ifdef L___syscall_mknod
#define __NR___syscall_mknod __NR_mknod
#include <sys/stat.h>
_syscall3(int, __syscall_mknod, const char *, path, __kernel_mode_t, mode, __kernel_dev_t, dev);
int mknod(const char *path, mode_t mode, dev_t dev)
{ 
	/* We must convert the dev_t value to a __kernel_dev_t */
	__kernel_dev_t k_dev;
	k_dev = ((major(dev) & 0xff) << 8) | (minor(dev) & 0xff);
	return __syscall_mknod(path, mode, k_dev);
}
#endif

//#define __NR_chmod            15
#ifdef L___syscall_chmod
#include <sys/stat.h>
#define __NR___syscall_chmod __NR_chmod
static inline 
_syscall2(int, __syscall_chmod, const char *, path, __kernel_mode_t, mode);
int chmod(const char *path, mode_t mode)
{
	return __syscall_chmod(path, mode);
}
#endif

/* Old kernels don't have lchown -- do chown instead.  This
 * is sick and wrong, but at least things will compile.  
 * They may not follow links when they should though... */
#ifndef __NR_lchown 
#define __NR_lchown __NR_chown
#endif

//#define __NR_lchown           16
#ifdef L___syscall_lchown
#include <unistd.h>
#define __NR___syscall_lchown __NR_lchown
static inline 
_syscall3(int, __syscall_lchown, const char *, path, __kernel_uid_t, owner, __kernel_gid_t, group);
int lchown(const char *path, uid_t owner, gid_t group)
{
	if (((owner + 1) > (uid_t) ((__kernel_uid_t) -1U))
			|| ((group + 1) > (gid_t) ((__kernel_gid_t) -1U)))
	{
		__set_errno (EINVAL);
		return -1;
	}
	return __syscall_lchown(path, owner, group);
}
#endif


//#define __NR_break            17

//#define __NR_oldstat          18

//#define __NR_lseek            19
#ifdef L___libc_lseek
#include <unistd.h>
#define __NR___libc_lseek __NR_lseek
_syscall3(__off_t, __libc_lseek, int, fildes, __off_t, offset, int, whence);
weak_alias(__libc_lseek, lseek)
#endif

//#define __NR_getpid           20
#ifdef L___libc_getpid
#include <unistd.h>
#if defined (__alpha__)
#define __NR_getpid     __NR_getxpid
#endif
#define __NR___libc_getpid __NR_getpid
_syscall0(pid_t, __libc_getpid);
weak_alias(__libc_getpid, getpid)
weak_alias(__libc_getpid, __getpid)
#endif

//#define __NR_mount            21
#ifdef L_mount
#include <sys/mount.h>
_syscall5(int, mount, const char *, specialfile, const char *, dir,
		  const char *, filesystemtype, unsigned long, rwflag,
		  const void *, data);
#endif

//#define __NR_umount           22
#ifdef L_umount
#include <sys/mount.h>
_syscall1(int, umount, const char *, specialfile);
#endif

//#define __NR_setuid           23
#ifdef L___syscall_setuid
#define __NR___syscall_setuid __NR_setuid
#include <unistd.h>
static inline 
_syscall1(int, __syscall_setuid, __kernel_uid_t, uid);
int setuid(uid_t uid)
{
	if (uid == (uid_t) ~0 || uid != (uid_t) ((__kernel_uid_t) uid)) {
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_setuid(uid));
}
#endif

//#define __NR_getuid           24
#ifdef L___syscall_getuid
#include <unistd.h>
#if defined (__alpha__)
#define __NR_getuid     __NR_getxuid
#endif
#define __NR___syscall_getuid __NR_getuid
static inline 
_syscall0(int, __syscall_getuid);
uid_t getuid(void)
{
	return(__syscall_getuid());
}
#endif

//#define __NR_stime            25
#ifdef L_stime
#include <time.h>
#include <sys/time.h>
#ifdef __NR_stime
_syscall1(int, stime, const time_t *, t);
#else
int stime(const time_t *when)
{ 
	struct timeval tv;
	if (when == NULL) { __set_errno (EINVAL); return -1; }
	tv.tv_sec = *when;
	tv.tv_usec = 0;
	return settimeofday (&tv, (struct timezone *) 0);
}
#endif
#endif

//#define __NR_ptrace           26
//See ptrace.c


//#define __NR_alarm            27
#ifdef L_alarm
#include <unistd.h>
#ifdef __NR_alarm
_syscall1(unsigned int, alarm, unsigned int, seconds);
#else
#include <sys/time.h>
unsigned int alarm (unsigned int seconds)
{
	struct itimerval old, new;
	unsigned int retval;
	new.it_value.tv_usec = 0;
	new.it_interval.tv_sec = 0;
	new.it_interval.tv_usec = 0;
	new.it_value.tv_sec = (long int) seconds;
	if (setitimer (ITIMER_REAL, &new, &old) < 0) { return 0; }
	retval = old.it_value.tv_sec;
	if (old.it_value.tv_usec) { ++retval; }
	return retval;
}
#endif
#endif

//#define __NR_oldfstat         28

//#define __NR_pause            29
#ifdef L___libc_pause
#include <unistd.h>
#ifdef __NR_pause
#define __NR___libc_pause __NR_pause
_syscall0(int, __libc_pause);
weak_alias(__libc_pause, pause)
#else
#include <signal.h>
int __libc_pause (void)
{
	return(__sigpause(sigblock(0), 0));
}
weak_alias(__libc_pause, pause)
#endif
#endif

//#define __NR_utime            30
#ifdef L_utime
#include <utime.h>
#ifdef __NR_utime
_syscall2(int, utime, const char *, file, const struct utimbuf *, times);
#else
#include <stdlib.h>
#include <sys/time.h>
int utime(const char *file, const struct utimbuf *times)
{
	struct timeval timevals[2];
	if (times != NULL) {
		timevals[0].tv_usec = 0L;
		timevals[1].tv_usec = 0L;
		timevals[0].tv_sec = (long int) times->actime;
		timevals[1].tv_sec = (long int) times->modtime;
	} else {
		if (gettimeofday (&timevals[0], NULL) < 0) { return -1; }
		timevals[1] = timevals[0];
	}
	return utimes(file, timevals);
}
#endif
#endif

//#define __NR_utimed
#ifdef L_utimes
#include <utime.h>
#ifdef __NR_utimes
_syscall2(int, utimes, const char *, file, const struct timeval *, tvp);
#else
#include <stdlib.h>
#include <sys/time.h>
int utimes (const char *file, const struct timeval tvp[2])
{
	struct utimbuf buf, *times;
	if (tvp) {
		times = &buf;
		times->actime = tvp[0].tv_sec;
		times->modtime = tvp[1].tv_sec;
	} else { times = NULL; }
	return utime(file, times);
}
#endif
#endif

//#define __NR_stty             31
#ifdef L_stty
#include <sgtty.h>
int stty (int __fd, __const struct sgttyb *__params);
{
	__set_errno(ENOSYS);
	return -1;
}
#endif

//#define __NR_gtty             32
#ifdef L_gtty
#include <sgtty.h>
int gtty (int __fd, struct sgttyb *__params)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif

//#define __NR_access           33
#ifdef L_access
#include <unistd.h>
_syscall2(int, access, const char *, pathname, int, mode);
#endif

//#define __NR_nice             34
#ifdef L_nice
#include <unistd.h>
#ifdef __NR_nice
_syscall1(int, nice, int, inc);
#else
#include <sys/resource.h>
int nice (int incr)
{
	int save, prio, result;
	save = errno;
	__set_errno (0);
	prio = getpriority (PRIO_PROCESS, 0);
	if (prio == -1) {
		if (errno != 0) { return -1; } 
		else { __set_errno (save); }
	}
	result = setpriority (PRIO_PROCESS, 0, prio + incr);
	if (result != -1) { return prio + incr; } else { return -1; }
}
#endif
#endif

//#define __NR_ftime            35

//#define __NR_sync             36
//See sync.c

//#define __NR_kill             37
#ifdef L___syscall_kill
#include <signal.h>
#undef kill
#define __NR___syscall_kill __NR_kill
static inline
_syscall2(int, __syscall_kill, __kernel_pid_t, pid, int, sig);
int kill(pid_t pid, int sig)
{
	return(__syscall_kill(pid, sig));
}
#endif

//#define __NR_rename           38
#ifdef L___syscall_rename
#define __NR___syscall_rename __NR_rename
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <stdio.h>
static inline
_syscall2(int, __syscall_rename, const char *, oldpath, const char *, newpath);
weak_alias(__syscall_rename, rename)
#endif

//#define __NR_mkdir            39
#ifdef L___syscall_mkdir
#include <sys/stat.h>
#define __NR___syscall_mkdir __NR_mkdir
static inline 
_syscall2(int, __syscall_mkdir, const char *, pathname, __kernel_mode_t, mode);
int mkdir(const char * pathname, mode_t mode)
{
	return(__syscall_mkdir(pathname, mode));
}
#endif

//#define __NR_rmdir            40
#ifdef L_rmdir
#include <unistd.h>
_syscall1(int, rmdir, const char *, pathname);
#endif

//#define __NR_dup              41
#ifdef L_dup
#include <unistd.h>
_syscall1(int, dup, int, oldfd);
#endif

//#define __NR_pipe             42
#ifdef L_pipe
#include <unistd.h>
_syscall1(int, pipe, int *, filedes);
#endif

//#define __NR_times            43
#ifdef L_times
#include <sys/times.h>
_syscall1(clock_t, times, struct tms *, buf);
#endif

//#define __NR_prof             44

//#define __NR_brk              45

//#define __NR_setgid           46
#ifdef L___syscall_setgid
#include <unistd.h>
#define __NR___syscall_setgid __NR_setgid
static inline 
_syscall1(int, __syscall_setgid, __kernel_gid_t, gid);
int setgid(gid_t gid)
{
	if (gid == (gid_t) ~0
			|| gid != (gid_t) ((__kernel_gid_t) gid))
	{
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_setgid(gid));
}
#endif

//#define __NR_getgid           47
#ifdef L___syscall_getgid
#include <unistd.h>
#define __NR___syscall_getgid __NR_getgid
#if defined (__alpha__)
#define __NR_getgid     __NR_getxgid
#endif
static inline 
_syscall0(int, __syscall_getgid);
gid_t getgid(void)
{
	return(__syscall_getgid());
}
#endif

//#define __NR_signal           48

//#define __NR_geteuid          49
#ifdef	L___syscall_geteuid
#include <unistd.h>
#	ifdef	__NR_geteuid
#define __NR___syscall_geteuid __NR_geteuid
	static inline 
	_syscall0(int, __syscall_geteuid);
	uid_t geteuid(void)
	{
		return(__syscall_geteuid());
	}
#	else
	uid_t geteuid(void)
	{
		return (getuid());
	}
#	endif
#endif

//#define __NR_getegid          50
#ifdef	L___syscall_getegid
#include <unistd.h>
#	ifdef	__NR_getegid
#define __NR___syscall_getegid __NR_getegid
static inline 
_syscall0(int, __syscall_getegid);
gid_t getegid(void)
{
	return(__syscall_getegid());
}
#	else
	gid_t getegid(void)
	{
		return (getgid());
	}
#	endif
#endif

//#define __NR_acct             51
#ifdef L_acct
#include <unistd.h>
_syscall1(int, acct, const char *, filename);
#endif

//#define __NR_umount2          52
#ifdef L_umount2
#	ifdef __NR_umount2 /* Old kernels don't have umount2 */ 
#		include <sys/mount.h>
		_syscall2(int, umount2, const char *, special_file, int, flags);
#	else
		int umount2(const char * special_file, int flags)
		{
			__set_errno(ENOSYS);
			return -1;
		}
#	endif
#endif

//#define __NR_lock             53

//#define __NR_ioctl            54
#ifdef L___syscall_ioctl
#include <stdarg.h>
#include <sys/ioctl.h>
#define __NR___syscall_ioctl __NR_ioctl
extern int __syscall_ioctl(int fd, int request, void *arg);
_syscall3(int, __syscall_ioctl, int, fd, int, request, void *, arg);
#if !defined (__powerpc__)
#include "ioctl.c"
/* Also see ioctl.c and powerpc/ioctl.c */
#endif
#endif

//#define __NR_fcntl            55
#ifdef L___syscall_fcntl
#include <stdarg.h>
#include <fcntl.h>
#define __NR___syscall_fcntl __NR_fcntl
#ifdef __UCLIBC_HAS_LFS__
static inline
#endif
_syscall3(int, __syscall_fcntl, int, fd, int, cmd, long, arg);
int __libc_fcntl(int fd, int cmd, ...)
{
	long arg;
	va_list list;
	if (cmd == F_GETLK64 || cmd == F_SETLK64 || cmd == F_SETLKW64) 
	{
			__set_errno(ENOSYS);
			return -1;
	}
	va_start(list, cmd);
	arg = va_arg(list, long);
	va_end(list);
	return(__syscall_fcntl(fd, cmd, arg));
}
weak_alias(__libc_fcntl, fcntl)
#if ! defined __NR_fcntl64 && defined __UCLIBC_HAS_LFS__
weak_alias(__libc_fcntl, fcntl64);
#endif
#endif

//#define __NR_mpx              56

//#define __NR_setpgid          57
#ifdef L___syscall_setpgid
#include <unistd.h>
#define __NR___syscall_setpgid __NR_setpgid
static inline
_syscall2(int, __syscall_setpgid, __kernel_pid_t, pid, __kernel_pid_t, pgid);
int setpgid(pid_t pid, pid_t pgid)
{
	return(__syscall_setpgid(pid, pgid));
}
#endif

//#define __NR_ulimit           58
//See ulimit.c

//#define __NR_oldolduname      59

//#define __NR_umask            60
#ifdef L___syscall_umask
#include <sys/stat.h>
#define __NR___syscall_umask __NR_umask
static inline 
_syscall1(__kernel_mode_t, __syscall_umask, __kernel_mode_t, mode);
mode_t umask(mode_t mode)
{
	return(__syscall_umask(mode));
}
#endif

//#define __NR_chroot           61
#ifdef L___syscall_chroot
#define __NR___syscall_chroot __NR_chroot
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
static inline
_syscall1(int, __syscall_chroot, const char *, path);
weak_alias(__syscall_chroot, chroot)
#endif

//#define __NR_ustat            62
#ifdef L___syscall_ustat
#define __NR___syscall_ustat __NR_ustat
#include <sys/ustat.h>
static inline
_syscall2(int, __syscall_ustat, unsigned short int, kdev_t, struct ustat *, ubuf);
int ustat(dev_t dev, struct ustat *ubuf)
{ 
	/* We must convert the dev_t value to a __kernel_dev_t */
	__kernel_dev_t k_dev;
	k_dev = ((major(dev) & 0xff) << 8) | (minor(dev) & 0xff);
	return __syscall_ustat(k_dev, ubuf);
}
#endif


//#define __NR_dup2             63
#ifdef L_dup2
#include <unistd.h>
_syscall2(int, dup2, int, oldfd, int, newfd);
#endif

//#define __NR_getppid          64
#ifdef	L_getppid
#	include <unistd.h>
#	ifdef	__NR_getppid
	_syscall0(pid_t, getppid);
#	else
	pid_t getppid(void)
	{
		return (getpid());
	}
#	endif
#endif

//#define __NR_getpgrp          65
#ifdef L_getpgrp
#include <unistd.h>
_syscall0(pid_t, getpgrp);
#endif

//#define __NR_setsid           66
#ifdef L_setsid
#include <unistd.h>
_syscall0(pid_t, setsid);
#endif

//#define __NR_sigaction        67
#ifndef __NR_rt_sigaction
#ifdef L___syscall_sigaction
#define __NR___syscall_sigaction __NR_sigaction
#include <signal.h>
#undef sigaction
_syscall3(int, __syscall_sigaction, int, signum, const struct sigaction *, act,
		  struct sigaction *, oldact);
#endif
#endif

//#define __NR_sgetmask         68

//#define __NR_ssetmask         69

//#define __NR_setreuid         70
#ifdef L___syscall_setreuid
#include <unistd.h>
#define __NR___syscall_setreuid __NR_setreuid
static inline
_syscall2(int, __syscall_setreuid, __kernel_uid_t, ruid, __kernel_uid_t, euid);
int setreuid(uid_t ruid, uid_t euid)
{
	if (((ruid + 1) > (uid_t) ((__kernel_uid_t) -1U))
			|| ((euid + 1) > (uid_t) ((__kernel_uid_t) -1U)))
	{
		__set_errno(EINVAL);
		return -1;
	}
	return(__syscall_setreuid(ruid, euid));
}
#endif

//#define __NR_setregid         71
#ifdef L___syscall_setregid
#include <unistd.h>
#define __NR___syscall_setregid __NR_setregid
static inline
_syscall2(int, __syscall_setregid, __kernel_gid_t, rgid, __kernel_gid_t, egid);
int setregid(gid_t rgid, gid_t egid)
{
	if (((rgid + 1) > (gid_t) ((__kernel_gid_t) -1U))
			|| ((egid + 1) > (gid_t) ((__kernel_gid_t) -1U)))
	{
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_setregid(rgid, egid));
}
#endif

//#define __NR_sigsuspend       72
#ifndef __NR_rt_sigsuspend
#define __NR___sigsuspend __NR_sigsuspend
#ifdef L___sigsuspend
#include <signal.h>
#undef sigsuspend
_syscall3(int, __sigsuspend, int, a, unsigned long int, b, unsigned long int, c);

int sigsuspend (const sigset_t *set)
{
	return __sigsuspend(0, 0, set->__val[0]);
}
#endif
#endif

//#define __NR_sigpending       73
#ifndef __NR_rt_sigpending
#ifdef L_sigpending
#include <signal.h>
#undef sigpending
_syscall1(int, sigpending, sigset_t *, set);
#endif
#endif

//#define __NR_sethostname      74
#ifdef L_sethostname
#include <unistd.h>
_syscall2(int, sethostname, const char *, name, size_t, len);
#endif

//#define __NR_setrlimit        75
#ifndef __NR_ugetrlimit
/* Only wrap setrlimit if the new ugetrlimit is not present */ 
#ifdef L___setrlimit
#define __NR___setrlimit __NR_setrlimit
#include <unistd.h>
#include <sys/resource.h>
#define RMIN(x, y) ((x) < (y) ? (x) : (y))
_syscall2(int, __setrlimit, int, resource, const struct rlimit *, rlim);
int setrlimit (__rlimit_resource_t resource, const struct rlimit *rlimits)
{
	struct rlimit rlimits_small;
	/* We might have to correct the limits values.  Since the old values
	 * were signed the new values might be too large.  */
	rlimits_small.rlim_cur = RMIN ((unsigned long int) rlimits->rlim_cur,
				       RLIM_INFINITY >> 1);
	rlimits_small.rlim_max = RMIN ((unsigned long int) rlimits->rlim_max,
				       RLIM_INFINITY >> 1);
	return(__setrlimit(resource, &rlimits_small));
}
#undef RMIN
#endif
#else /* We don't need to wrap setrlimit */
#ifdef L_setrlimit
#include <unistd.h>
struct rlimit;
_syscall2(int, setrlimit, unsigned int, resource, const struct rlimit *, rlim);
#endif
#endif /* __NR_setrlimit */

//#define __NR_getrlimit        76
#ifdef L___getrlimit
/* Only include the old getrlimit if the new one (ugetrlimit) is not around */ 
#ifndef __NR_ugetrlimit
#define __NR___getrlimit __NR_getrlimit
#include <unistd.h>
#include <sys/resource.h>
_syscall2(int, __getrlimit, int, resource, struct rlimit *, rlim);
int getrlimit (__rlimit_resource_t resource, struct rlimit *rlimits)
{
	int result;
	result = __getrlimit(resource, rlimits);

	if (result == -1)
		return result;

	/* We might have to correct the limits values.  Since the old values
	 * were signed the infinity value is too small.  */
	if (rlimits->rlim_cur == RLIM_INFINITY >> 1)
		rlimits->rlim_cur = RLIM_INFINITY;
	if (rlimits->rlim_max == RLIM_INFINITY >> 1)
		rlimits->rlim_max = RLIM_INFINITY;
	return result;
}
#endif
#endif /* __NR_getrlimit */

//#define __NR_getrusage        77
#ifdef L_getrusage
#include <unistd.h>
#include <wait.h>
_syscall2(int, getrusage, int, who, struct rusage *, usage);
#endif

//#define __NR_gettimeofday     78
#ifdef L_gettimeofday
#include <sys/time.h>
_syscall2(int, gettimeofday, struct timeval *, tv, struct timezone *, tz);
#endif

//#define __NR_settimeofday     79
#ifdef L_settimeofday
#include <sys/time.h>
_syscall2(int, settimeofday, const struct timeval *, tv,
		  const struct timezone *, tz);
#endif

//#define __NR_getgroups        80
#ifdef L___syscall_getgroups
#include <unistd.h>
#define __NR___syscall_getgroups __NR_getgroups
static inline
_syscall2(int, __syscall_getgroups, int, size, __kernel_gid_t *, list);
#define MIN(a,b) (((a)<(b))?(a):(b))
int getgroups(int n, gid_t *groups)
{
	if (unlikely(n < 0)) {
		__set_errno(EINVAL);
		return -1;
	} else {
		int i, ngids;
		__kernel_gid_t kernel_groups[n = MIN(n, sysconf(_SC_NGROUPS_MAX))];
		ngids = __syscall_getgroups(n, kernel_groups);
		if (n != 0 && ngids > 0) {
			for (i = 0; i < ngids; i++) {
				groups[i] = kernel_groups[i];
			}
		}
		return ngids;
	}
}
#endif

//#define __NR_setgroups        81
#ifdef L___syscall_setgroups
#include <unistd.h>
#include <grp.h>
#define __NR___syscall_setgroups __NR_setgroups
static inline
_syscall2(int, __syscall_setgroups, size_t, size, const __kernel_gid_t *, list);
int setgroups (size_t n, const gid_t *groups)
{
	if (n > (size_t)sysconf(_SC_NGROUPS_MAX)) {
		__set_errno (EINVAL);
		return -1;
	} else {
		size_t i;
		__kernel_gid_t kernel_groups[n];
		for (i = 0; i < n; i++) {
			kernel_groups[i] = (groups)[i];
			if (groups[i] != (gid_t) ((__kernel_gid_t)groups[i])) {
				__set_errno (EINVAL);
				return -1;
			}
		}
		return(__syscall_setgroups(n, kernel_groups));
	}
}
#endif

//#define __NR_select           82
#ifdef L_select
//Used as a fallback if _newselect isn't available...
#ifndef __NR__newselect
#include <unistd.h>
extern int select(int n, fd_set *readfds, fd_set *writefds, 
		fd_set *exceptfds, struct timeval *timeout);
_syscall5(int, select, int, n, fd_set *, readfds, fd_set *, writefds,
		fd_set *, exceptfds, struct timeval *, timeout);
#endif
#endif

//#define __NR_symlink          83
#ifdef L_symlink
#include <unistd.h>
_syscall2(int, symlink, const char *, oldpath, const char *, newpath);
#endif

//#define __NR_oldlstat         84

//#define __NR_readlink         85
#ifdef L_readlink
#include <unistd.h>
_syscall3(int, readlink, const char *, path, char *, buf, size_t, bufsiz);
#endif

//#define __NR_uselib           86
#ifdef L_uselib
#include <unistd.h>
_syscall1(int, uselib, const char *, library);
#endif

//#define __NR_swapon           87
#ifdef L_swapon
#include <sys/swap.h>
_syscall2(int, swapon, const char *, path, int, swapflags);
#endif

//#define __NR_reboot           88
#ifdef L__reboot
#define __NR__reboot __NR_reboot
extern int _reboot(int magic, int magic2, int flag);

_syscall3(int, _reboot, int, magic, int, magic2, int, flag);

int reboot(int flag)
{
	return (_reboot((int) 0xfee1dead, 672274793, flag));
}
#endif

//#define __NR_readdir          89

//#define __NR_mmap             90
#ifdef L__mmap
#define __NR__mmap __NR_mmap
#include <unistd.h>
#include <sys/mman.h>
extern __ptr_t _mmap(unsigned long *buffer);

_syscall1(__ptr_t, _mmap, unsigned long *, buffer);

__ptr_t mmap(__ptr_t addr, size_t len, int prot,
			 int flags, int fd, __off_t offset)
{
	unsigned long buffer[6];

	buffer[0] = (unsigned long) addr;
	buffer[1] = (unsigned long) len;
	buffer[2] = (unsigned long) prot;
	buffer[3] = (unsigned long) flags;
	buffer[4] = (unsigned long) fd;
	buffer[5] = (unsigned long) offset;
	return (__ptr_t) _mmap(buffer);
}
#endif

//#define __NR_munmap           91
#ifdef L_munmap
#include <unistd.h>
#include <sys/mman.h>
_syscall2(int, munmap, void *, start, size_t, length);
#endif

//#define __NR_truncate         92
#ifdef L_truncate
#include <unistd.h>
_syscall2(int, truncate, const char *, path, __off_t, length);
#endif

//#define __NR_ftruncate        93
#ifdef L_ftruncate
#include <unistd.h>
_syscall2(int, ftruncate, int, fd, __off_t, length);
#endif

//#define __NR_fchmod           94
#ifdef L___syscall_fchmod
#include <sys/stat.h>
#define __NR___syscall_fchmod __NR_fchmod
static inline 
_syscall2(int, __syscall_fchmod, int, fildes, __kernel_mode_t, mode);
int fchmod(int fildes, mode_t mode)
{
	return(__syscall_fchmod(fildes, mode));
}
#endif

//#define __NR_fchown           95
#ifdef L___syscall_fchown
#include <unistd.h>
#define __NR___syscall_fchown __NR_fchown
static inline
_syscall3(int, __syscall_fchown, int, fd, __kernel_uid_t, owner, __kernel_gid_t, group);
int fchown(int fd, uid_t owner, gid_t group)
{
	if (((owner + 1) > (uid_t) ((__kernel_uid_t) -1U))
			|| ((group + 1) > (gid_t) ((__kernel_gid_t) -1U)))
	{
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_fchown(fd, owner, group));
}
#endif

//#define __NR_getpriority      96
#ifdef L___syscall_getpriority
#include <sys/resource.h>
#define __NR___syscall_getpriority __NR_getpriority
_syscall2(int, __syscall_getpriority, __priority_which_t, which, id_t, who);
/* The return value of __syscall_getpriority is biased by this value
 * to avoid returning negative values.  */
#define PZERO 20
int getpriority (enum __priority_which which, id_t who)
{
	int res;

	res = __syscall_getpriority(which, who);
	if (res >= 0)
		res = PZERO - res;
	return res;
}
#endif

//#define __NR_setpriority      97
#ifdef L_setpriority
#include <sys/resource.h>
_syscall3(int, setpriority, __priority_which_t, which, id_t, who, int, prio);
#endif

//#define __NR_profil           98

//#define __NR_statfs           99
#ifdef L___syscall_statfs
#define __NR___syscall_statfs __NR_statfs
#include <string.h>
#include <sys/param.h>
#include <sys/vfs.h>
static inline
_syscall2(int, __syscall_statfs, const char *, path, struct statfs *, buf);
weak_alias(__syscall_statfs, statfs)
#endif

//#define __NR_fstatfs          100
#ifdef L_fstatfs
#include <sys/vfs.h>
_syscall2(int, fstatfs, int, fd, struct statfs *, buf);
#endif

//#define __NR_ioperm           101
#ifdef L_ioperm
#	if defined __UCLIBC_HAS_MMU__ && defined __NR_ioperm
		_syscall3(int, ioperm, unsigned long, from, unsigned long, num, int, turn_on);
#	else
		int ioperm(unsigned long from, unsigned long num, int turn_on)
		{
			__set_errno(ENOSYS);
			return -1;
		}
#	endif
#endif

//#define __NR_socketcall       102
#ifdef L___socketcall
#ifdef __NR_socketcall
#define __NR___socketcall __NR_socketcall
_syscall2(int, __socketcall, int, call, unsigned long *, args);
#endif
#endif

//#define __NR_syslog           103
#ifdef L__syslog
#include <unistd.h>
#define __NR__syslog		__NR_syslog
extern int _syslog(int type, char *buf, int len);

_syscall3(int, _syslog, int, type, char *, buf, int, len);

int klogctl(int type, char *buf, int len)
{
	return (_syslog(type, buf, len));
}

#endif

//#define __NR_setitimer        104
#ifdef L_setitimer
#include <sys/time.h>
_syscall3(int, setitimer, __itimer_which_t, which,
		  const struct itimerval *, new, struct itimerval *, old);
#endif

//#define __NR_getitimer        105
#ifdef L_getitimer
#include <sys/time.h>
_syscall2(int, getitimer, __itimer_which_t, which, struct itimerval *, value);
#endif

//#define __NR_stat             106
#ifdef L___syscall_stat
#define __NR___syscall_stat __NR_stat
#include <unistd.h>
#define _SYS_STAT_H
#include <bits/stat.h>
#include "xstatconv.h"
_syscall2(int, __syscall_stat, const char *, file_name, struct kernel_stat *, buf);
int stat(const char * file_name, struct stat * buf)
{
	int result;
	struct kernel_stat kbuf;
	result = __syscall_stat(file_name, &kbuf);
	if (result == 0) {
		__xstat_conv(&kbuf, buf);
	}
	return result;
}
#if ! defined __NR_stat64 && defined __UCLIBC_HAS_LFS__
weak_alias(stat, stat64);
#endif
#endif

//#define __NR_lstat            107
#ifdef L___syscall_lstat
#define __NR___syscall_lstat __NR_lstat
#include <unistd.h>
#define _SYS_STAT_H
#include <bits/stat.h>
#include <bits/kernel_stat.h>
#include "xstatconv.h"
_syscall2(int, __syscall_lstat, const char *, file_name, struct kernel_stat *, buf);
int lstat(const char * file_name, struct stat * buf)
{
	int result;
	struct kernel_stat kbuf;
	result = __syscall_lstat(file_name, &kbuf);
	if (result == 0) {
		__xstat_conv(&kbuf, buf);
	}
	return result;
}
#if ! defined __NR_lstat64 && defined __UCLIBC_HAS_LFS__
weak_alias(lstat, lstat64);
#endif
#endif

//#define __NR_fstat            108
#ifdef L___syscall_fstat
#define __NR___syscall_fstat __NR_fstat
#include <unistd.h>
#define _SYS_STAT_H
#include <bits/stat.h>
#include <bits/kernel_stat.h>
#include "xstatconv.h"
_syscall2(int, __syscall_fstat, int, fd, struct kernel_stat *, buf);
int fstat(int fd, struct stat * buf)
{
	int result;
	struct kernel_stat kbuf;
	result = __syscall_fstat(fd, &kbuf);
	if (result == 0) {
		__xstat_conv(&kbuf, buf);
	}
	return result;
}
#if ! defined __NR_fstat64 && defined __UCLIBC_HAS_LFS__
weak_alias(fstat, fstat64);
#endif
#endif

//#define __NR_olduname         109

//#define __NR_iopl             110
#ifdef L_iopl
/* For arm there is a totally different implementation */
#if !defined(__arm__)
/* Tuns out the m68k unistd.h kernel header is broken */
#	if defined __UCLIBC_HAS_MMU__ && defined __NR_iopl && ( !defined(__mc68000__))
		_syscall1(int, iopl, int, level);
#	else
		int iopl(int level)
		{
			__set_errno(ENOSYS);
			return -1;
		}
#	endif
# endif
#endif

//#define __NR_vhangup          111
#ifdef L_vhangup
#include <unistd.h>
_syscall0(int, vhangup);
#endif

//#define __NR_idle             112
//int idle(void);

//#define __NR_vm86old          113

//#define __NR_wait4            114
#ifdef L___syscall_wait4
#define __NR___syscall_wait4 __NR_wait4
static inline
_syscall4(int, __syscall_wait4, __kernel_pid_t, pid, int *, status, int, opts, void *, rusage);
int wait4(pid_t pid, int * status, int opts, void * rusage)
{
	return(__syscall_wait4(pid, status, opts, rusage));
}
#endif

//#define __NR_swapoff          115
#ifdef L_swapoff
#include <sys/swap.h>
_syscall1(int, swapoff, const char *, path);
#endif

//#define __NR_sysinfo          116
#ifdef L_sysinfo
#include <sys/sysinfo.h>
_syscall1(int, sysinfo, struct sysinfo *, info);
#endif

//#define __NR_ipc              117
#ifdef L___ipc
#ifdef __NR_ipc
#define __NR___ipc __NR_ipc
_syscall5(int, __ipc, unsigned int, call, int, first, int, second, int, third, void *, ptr);
#endif
#endif

//#define __NR_fsync            118
#ifdef L___libc_fsync
#include <unistd.h>
#define __NR___libc_fsync __NR_fsync
_syscall1(int, __libc_fsync, int, fd);
weak_alias(__libc_fsync, fsync)
#endif

//#define __NR_sigreturn        119
//int sigreturn(unsigned long __unused);

//#define __NR_clone            120
//See architecture specific implementation...

//#define __NR_setdomainname    121
#ifdef L_setdomainname
#include <unistd.h>
_syscall2(int, setdomainname, const char *, name, size_t, len);
#endif

//#define __NR_uname            122
#ifdef L_uname
#include <sys/utsname.h>
_syscall1(int, uname, struct utsname *, buf);
#endif

//#define __NR_modify_ldt       123
#ifdef __NR_modify_ldt
#ifdef L_modify_ldt
_syscall3(int, modify_ldt, int, func, void *, ptr, unsigned long, bytecount);
weak_alias(modify_ldt, __modify_ldt);
#endif
#endif

//#define __NR_adjtimex         124
#ifdef L_adjtimex
#include <sys/timex.h>
_syscall1(int, adjtimex, struct timex *, buf);
weak_alias(adjtimex, __adjtimex);
weak_alias(adjtimex, ntp_adjtime);
#endif

//#define __NR_mprotect         125
#ifdef L_mprotect
#include <sys/mman.h>
_syscall3(int, mprotect, void *, addr, size_t, len, int, prot);
#endif

//#define __NR_sigprocmask      126
#ifndef __NR_rt_sigprocmask
#ifdef L___syscall_sigprocmask
#include <signal.h>
#define __NR___syscall_sigprocmask __NR_sigprocmask
static inline
_syscall3(int, __syscall_sigprocmask, int, how, const sigset_t *, set,
		sigset_t *, oldset);
#undef sigprocmask
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	if (set &&
#if (SIG_BLOCK == 0) && (SIG_UNBLOCK == 1) && (SIG_SETMASK == 2)
	(((unsigned int) how) > 2)
#else
#warning "compile time assumption violated.. slow path..."
	((how != SIG_BLOCK) && (how != SIG_UNBLOCK) && (how != SIG_SETMASK))
#endif
	   )
	{
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_sigprocmask(how, set, oldset));
}
#endif
#endif

//#define __NR_create_module    127
//See sysdeps/linux/commom/create_module.c

//#define __NR_init_module      128
#ifdef L_init_module
/* This may have 5 arguments (for old 2.0 kernels) or 2 arguments
 * (for 2.2 and 2.4 kernels).  Use the greatest common denominator,
 * and let the kernel cope with whatever it gets.  It's good at that. */
_syscall5(int, init_module, void *, first, void *, second, void *, third, 
			void *, fourth, void *, fifth);
#endif

//#define __NR_delete_module    129
#ifdef L_delete_module
#	ifdef __NR_delete_module
		_syscall1(int, delete_module, const char *, name);
#	else
		int delete_module(const char * name)
		{
			__set_errno(ENOSYS);
			return -1;
		}
#	endif
#endif

//#define __NR_get_kernel_syms  130
#ifdef L_get_kernel_syms
struct kernel_sym;
_syscall1(int, get_kernel_syms, struct kernel_sym *, table);
#endif

//#define __NR_quotactl         131
#ifdef __NR_quotactl
#ifdef L_quotactl
#include <sys/quota.h>
_syscall4(int, quotactl, int, cmd, const char *, special , int, id, caddr_t, addr);
#endif
#endif

//#define __NR_getpgid          132
#ifdef L___syscall_getpgid
#define __NR___syscall_getpgid __NR_getpgid
#define __FAVOR_BSD
static inline
_syscall1(__kernel_pid_t, __syscall_getpgid, __kernel_pid_t, pid);
pid_t __getpgid(pid_t pid)
{
	return(__syscall_getpgid(pid));
}
weak_alias(__getpgid, getpgid);
#endif

//#define __NR_fchdir           133
#ifdef L_fchdir
#include <unistd.h>
_syscall1(int, fchdir, int, fd);
#endif

//#define __NR_bdflush          134
#ifdef L_bdflush
#include <sys/kdaemon.h>
_syscall2(int, bdflush, int, __func, long int, __data);
#endif

//#define __NR_sysfs            135
//_syscall3(int, sysfs, int, option, unsigned int, index, char addr);


//#define __NR_personality      136
#ifdef __NR_personality
#ifdef L_personality
#include <sys/personality.h>
_syscall1(int, personality, unsigned long int, __persona);
#endif
#endif

//#define __NR_afs_syscall      137

//#define __NR_setfsuid         138
#ifdef __NR_setfsuid
#ifdef L___syscall_setfsuid
#include <sys/fsuid.h>
#define __NR___syscall_setfsuid __NR_setfsuid
static inline
_syscall1(int, __syscall_setfsuid, __kernel_uid_t, uid);
int setfsuid(uid_t uid)
{
	if (uid != (uid_t) ((__kernel_uid_t) uid))
	{
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_setfsuid(uid));
}
#endif
#endif

//#define __NR_setfsgid         139
#ifdef __NR_setfsgid
#ifdef L___syscall_setfsgid
#include <sys/fsuid.h>
#define __NR___syscall_setfsgid __NR_setfsgid
static inline
_syscall1(int, __syscall_setfsgid, __kernel_gid_t, gid);
int setfsgid(gid_t gid)
{
	if (gid != (gid_t) ((__kernel_gid_t) gid))
	{
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_setfsgid(gid));
}
#endif
#endif

//#define __NR__llseek          140
//See llseek.c

//#define __NR_getdents         141
// See getdents.c

//#define __NR__newselect       142
#ifdef L__newselect
//Used in preference to select when available...
#ifdef __NR__newselect
#include <unistd.h>
extern int _newselect(int n, fd_set *readfds, fd_set *writefds,
					  fd_set *exceptfds, struct timeval *timeout);
_syscall5(int, _newselect, int, n, fd_set *, readfds, fd_set *, writefds,
		fd_set *, exceptfds, struct timeval *, timeout);
weak_alias(_newselect, select);
#endif
#endif

//#define __NR_flock            143
#ifdef L___syscall_flock
#include <sys/file.h>
#define __NR___syscall_flock __NR_flock
static inline
_syscall2(int, __syscall_flock, int, fd, int, operation);
int flock(int fd, int operation)
{
	return(__syscall_flock(fd, operation));
}
#endif

//#define __NR_msync            144
#ifdef L___libc_msync
#include <unistd.h>
#include <sys/mman.h>
#define __NR___libc_msync __NR_msync
_syscall3(int, __libc_msync, void *, addr, size_t, length, int, flags);
weak_alias(__libc_msync, msync);
#endif

//#define __NR_readv            145
#ifdef L_readv
#include <sys/uio.h>
_syscall3(ssize_t, readv, int, filedes, const struct iovec *, vector, int,
		  count);
#endif

//#define __NR_writev           146
#ifdef L_writev
#include <sys/uio.h>
_syscall3(ssize_t, writev, int, filedes, const struct iovec *, vector, int,
		  count);
#endif

//#define __NR_getsid           147
#ifdef L___syscall_getsid
#include <unistd.h>
#define __NR___syscall_getsid __NR_getsid
static inline
_syscall1(__kernel_pid_t, __syscall_getsid, __kernel_pid_t, pid);
pid_t getsid(pid_t pid)
{
	return(__syscall_getsid(pid));
}
#endif

//#define __NR_fdatasync        148
#ifdef __NR_fdatasync
#ifdef L_fdatasync
#include <unistd.h>
_syscall1(int, fdatasync, int, fd);
#endif
#endif

//#define __NR__sysctl          149
#ifdef __NR__sysctl
#ifdef L__sysctl
struct __sysctl_args {
	int *name;
	int nlen;
	void *oldval;
	size_t *oldlenp;
	void *newval;
	size_t newlen;
	unsigned long __unused[4];
};
_syscall1(int, _sysctl, struct __sysctl_args *, args);
int sysctl(int *name, int nlen, void *oldval, size_t *oldlenp,
			void *newval, size_t newlen)
{
	struct __sysctl_args args =
	{
		name: name,
		nlen: nlen,
		oldval: oldval,
		oldlenp: oldlenp,
		newval: newval,
		newlen: newlen
	};

	return _sysctl(&args);
}
#endif
#endif

//#define __NR_mlock            150
#ifdef L_mlock
#include <sys/mman.h>
#	if defined __UCLIBC_HAS_MMU__ && defined __NR_mlock
		_syscall2(int, mlock, const void *, addr, size_t, len);
#	endif	
#endif	

//#define __NR_munlock          151
#ifdef L_munlock
#include <sys/mman.h>
#	if defined __UCLIBC_HAS_MMU__ && defined __NR_munlock
		_syscall2(int, munlock, const void *, addr, size_t, len);
#	endif	
#endif	

//#define __NR_mlockall         152
#ifdef L_mlockall
#include <sys/mman.h>
#	if defined __UCLIBC_HAS_MMU__ && defined __NR_mlockall
		_syscall1(int, mlockall, int, flags);
#	endif	
#endif	

//#define __NR_munlockall       153
#ifdef L_munlockall
#include <sys/mman.h>
#	if defined __UCLIBC_HAS_MMU__ && defined L_munlockall
		_syscall0(int, munlockall);
#	endif	
#endif	

//#define __NR_sched_setparam   154
#ifdef __NR_sched_setparam
#ifdef L___syscall_sched_setparam
#include <sched.h>
#define __NR___syscall_sched_setparam __NR_sched_setparam
static inline
_syscall2(int, __syscall_sched_setparam, __kernel_pid_t, pid, const struct sched_param *, p);
int sched_setparam(pid_t pid, const struct sched_param * p)
{
	return(__syscall_sched_setparam(pid, p));
}
#endif
#endif

//#define __NR_sched_getparam   155
#ifdef __NR_sched_getparam
#ifdef L___syscall_sched_getparam
#include <sched.h>
#define __NR___syscall_sched_getparam __NR_sched_getparam
static inline
_syscall2(int, __syscall_sched_getparam, __kernel_pid_t, pid, struct sched_param *, p);
int sched_getparam(pid_t pid, struct sched_param * p)
{
	return(__syscall_sched_getparam(pid, p));
}
#endif
#endif

//#define __NR_sched_setscheduler       156
#ifdef __NR_sched_setscheduler
#ifdef L___syscall_sched_setscheduler
#include <sched.h>
#define __NR___syscall_sched_setscheduler __NR_sched_setscheduler
static inline
_syscall3(int, __syscall_sched_setscheduler, __kernel_pid_t, pid, int, policy, const struct sched_param *, p);
int sched_setscheduler(pid_t pid, int policy, const struct sched_param * p)
{
	return(__syscall_sched_setscheduler(pid, policy, p));
}
#endif
#endif

//#define __NR_sched_getscheduler       157
#ifdef __NR_sched_getscheduler
#ifdef L___syscall_sched_getscheduler
#include <sched.h>
#define __NR___syscall_sched_getscheduler __NR_sched_getscheduler
static inline
_syscall1(int, __syscall_sched_getscheduler, __kernel_pid_t, pid);
int sched_getscheduler(pid_t pid)
{
	return(__syscall_sched_getscheduler(pid));
}
#endif
#endif

//#define __NR_sched_yield              158
#ifdef __NR_sched_yield
#ifdef L_sched_yield
#include <sched.h>
_syscall0(int, sched_yield);
#endif
#endif

//#define __NR_sched_get_priority_max   159
#ifdef __NR_sched_get_priority_max
#ifdef L_sched_get_priority_max
#include <sched.h>
_syscall1(int, sched_get_priority_max, int, policy);
#endif
#endif

//#define __NR_sched_get_priority_min   160
#ifdef __NR_sched_get_priority_min
#ifdef L_sched_get_priority_min
#include <sched.h>
_syscall1(int, sched_get_priority_min, int, policy);
#endif
#endif

//#define __NR_sched_rr_get_interval    161
#ifdef __NR_sched_rr_get_interval
#ifdef L___syscall_sched_rr_get_interval
#include <sched.h>
#define __NR___syscall_sched_rr_get_interval __NR_sched_rr_get_interval
static inline
_syscall2(int, __syscall_sched_rr_get_interval, __kernel_pid_t, pid, struct timespec *, tp);
int sched_rr_get_interval(pid_t pid, struct timespec * tp)
{
	return(__syscall_sched_rr_get_interval(pid, tp));
}
#endif
#endif

//#define __NR_nanosleep                162
#ifdef L___libc_nanosleep
#include <time.h>
#define __NR___libc_nanosleep __NR_nanosleep
_syscall2(int, __libc_nanosleep, const struct timespec *, req, struct timespec *, rem);
weak_alias(__libc_nanosleep, nanosleep)
#endif

//#define __NR_mremap                   163
#ifdef L_mremap
#include <unistd.h>
#include <sys/mman.h>
_syscall4(__ptr_t, mremap, __ptr_t, old_address, size_t, old_size, size_t, new_size, int, may_move);
#endif

//#define __NR_setresuid                164
#ifdef __NR_setresuid
#ifdef L___syscall_setresuid
#define __NR___syscall_setresuid __NR_setresuid
static inline
_syscall3(int, __syscall_setresuid, __kernel_uid_t, rgid, __kernel_uid_t, egid, __kernel_uid_t, sgid);
int setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
	if (((ruid + 1) > (uid_t) ((__kernel_uid_t) -1U))
			|| ((euid + 1) > (uid_t) ((__kernel_uid_t) -1U))
			|| ((suid + 1) > (uid_t) ((__kernel_uid_t) -1U)))
	{
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_setresuid(ruid, euid, suid));
}
#endif
#endif

//#define __NR_getresuid                165
#ifdef __NR_getresuid
#ifdef L___syscall_getresuid
#define __NR___syscall_getresuid __NR_getresuid
static inline
_syscall3(int, __syscall_getresuid, __kernel_uid_t *, ruid, __kernel_uid_t *, euid, __kernel_uid_t *, suid);
int getresuid (uid_t *ruid, uid_t *euid, uid_t *suid)
{
	int result;
	__kernel_uid_t k_ruid, k_euid, k_suid;
	result = __syscall_getresuid(&k_ruid, &k_euid, &k_suid);
	if (result == 0) {
		*ruid = (uid_t) k_ruid;
		*euid = (uid_t) k_euid;
		*suid = (uid_t) k_suid;
	}
	return result;
}
#endif
#endif

//#define __NR_vm86                     166

//#define __NR_query_module             167
#ifdef L_query_module
#	ifdef __NR_query_module
		_syscall5(int, query_module, const char *, name, int, which,
				void *, buf, size_t, bufsize, size_t*, ret);
#	else
		int query_module(const char * name, int which,
					void * buf, size_t bufsize, size_t* ret)
		{
			__set_errno(ENOSYS);
			return -1;
		}
#	endif	
#endif	

//#define __NR_poll                     168
#ifdef L_poll
#ifdef __NR_poll
#include <sys/poll.h>
_syscall3(int, poll, struct pollfd *, fds, unsigned long int, nfds, int, timeout);
#else
/* uClinux 2.0 doesn't have poll, emulate it using select */
#include "poll.c"
#endif
#endif

//#define __NR_nfsservctl               169
//nfsservctl	EXTRA	nfsservctl	i:ipp	nfsservctl

//#define __NR_setresgid                170
#ifdef __NR_setresgid
#ifdef L___syscall_setresgid
#define __NR___syscall_setresgid __NR_setresgid
static inline
_syscall3(int, __syscall_setresgid, __kernel_gid_t, rgid, __kernel_gid_t, egid, __kernel_gid_t, sgid);
int setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
	if (((rgid + 1) > (gid_t) ((__kernel_gid_t) -1U))
			|| ((egid + 1) > (gid_t) ((__kernel_gid_t) -1U))
			|| ((sgid + 1) > (gid_t) ((__kernel_gid_t) -1U)))
	{
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_setresgid(rgid, egid, sgid));
}
#endif
#endif

//#define __NR_getresgid                171
#ifdef __NR_getresgid
#ifdef L___syscall_getresgid
#define __NR___syscall_getresgid __NR_getresgid
static inline
_syscall3(int, __syscall_getresgid, __kernel_gid_t *, egid, __kernel_gid_t *, rgid, __kernel_gid_t *, sgid);
int getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid)
{
	int result;
	__kernel_gid_t k_rgid, k_egid, k_sgid;
	result = __syscall_getresgid(&k_rgid, &k_egid, &k_sgid);
	if (result == 0) { 
		*rgid = (gid_t) k_rgid;
		*egid = (gid_t) k_egid;
		*sgid = (gid_t) k_sgid;
	}
	return result;
}
#endif
#endif

//#define __NR_prctl                    172
#ifdef __NR_prctl
#ifdef L_prctl
#include <stdarg.h>
//#include <sys/prctl.h>
_syscall5(int, prctl, int, a, int, b, int, c, int, d, int, e);
#endif
#endif

//#define __NR_rt_sigreturn             173
//#define __NR_rt_sigaction             174
#ifdef __NR_rt_sigaction
#define __NR___syscall_rt_sigaction __NR_rt_sigaction
#ifdef L___syscall_rt_sigaction
#include <signal.h>
#undef sigaction
_syscall4(int, __syscall_rt_sigaction, int, signum, const struct sigaction *, act, 
		struct sigaction *, oldact, size_t, size); 
#endif
#endif

//#define __NR_rt_sigprocmask           175
#ifdef __NR_rt_sigprocmask
#define __NR___rt_sigprocmask __NR_rt_sigprocmask
#ifdef L___rt_sigprocmask
#include <signal.h>
#undef sigprocmask
_syscall4(int, __rt_sigprocmask, int, how, const sigset_t *, set, 
		sigset_t *, oldset, size_t, size);

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) 
{
	if (set &&
#if (SIG_BLOCK == 0) && (SIG_UNBLOCK == 1) && (SIG_SETMASK == 2)
			(((unsigned int) how) > 2)
#else
#warning "compile time assumption violated.. slow path..."
			((how != SIG_BLOCK) && (how != SIG_UNBLOCK) && (how != SIG_SETMASK))
#endif
	   )
	{
		__set_errno (EINVAL);
		return -1;
	}
	return __rt_sigprocmask(how, set, oldset, _NSIG/8);
}
#endif
#endif

//#define __NR_rt_sigpending            176
#ifdef __NR_rt_sigpending
#define __NR___rt_sigpending __NR_rt_sigpending
#ifdef L___rt_sigpending
#include <signal.h>
#undef sigpending
_syscall2(int, __rt_sigpending, sigset_t *, set, size_t, size);

int sigpending(sigset_t *set) 
{
	return __rt_sigpending(set, _NSIG/8);
}
#endif
#endif

//#define __NR_rt_sigtimedwait          177
#ifdef L___rt_sigtimedwait
#include <signal.h>
#define __need_NULL
#include <stddef.h>
#ifdef __NR_rt_sigtimedwait
#define __NR___rt_sigtimedwait __NR_rt_sigtimedwait
_syscall4(int, __rt_sigtimedwait, const sigset_t *, set, siginfo_t *, info, 
		const struct timespec *, timeout, size_t, setsize);

int sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
	return __rt_sigtimedwait (set, info, NULL, _NSIG/8);
}

int sigtimedwait (const sigset_t *set, siginfo_t *info, const struct timespec *timeout)
{
	return __rt_sigtimedwait (set, info, timeout, _NSIG/8);
}
#else
int sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
	if (set==NULL)
		__set_errno (EINVAL);
	else
		__set_errno (ENOSYS);
	return -1;
}

int sigtimedwait (const sigset_t *set, siginfo_t *info, const struct timespec *timeout)
{
	if (set==NULL)
		__set_errno (EINVAL);
	else
		__set_errno (ENOSYS);
	return -1;
}
#endif
#endif

//#define __NR_rt_sigqueueinfo          178

//#define __NR_rt_sigsuspend            179
#ifdef __NR_rt_sigsuspend
#define __NR___rt_sigsuspend __NR_rt_sigsuspend
#ifdef L___rt_sigsuspend
#include <signal.h>
#undef _sigsuspend
_syscall2(int, __rt_sigsuspend, const sigset_t *, mask, size_t, size);

int sigsuspend (const sigset_t *mask)
{
	return __rt_sigsuspend(mask, _NSIG/8);
}
#endif
#endif

//#define __NR_pread                    180
// See pread_write.c

//#define __NR_pwrite                   181
// See pread_write.c

//#define __NR_chown                    182
#ifdef L___syscall_chown
#include <unistd.h>
#define __NR___syscall_chown __NR_chown
static inline
_syscall3(int, __syscall_chown, const char *, path, __kernel_uid_t, owner, __kernel_gid_t, group);
int chown(const char * path, uid_t owner, gid_t group)
{
	if (((owner + 1) > (uid_t) ((__kernel_uid_t) -1U))
			|| ((group + 1) > (gid_t) ((__kernel_gid_t) -1U)))
	{
		__set_errno (EINVAL);
		return -1;
	}
	return(__syscall_chown(path, owner, group));
}
#endif

//#define __NR_getcwd                   183
// See getcwd.c in this directory

//#define __NR_capget                   184
#ifdef L_capget
#	ifdef __NR_capget
		_syscall2(int, capget, void*, header, void*, data);
#	else
		int capget(void* header, void* data)
		{
			__set_errno(ENOSYS);
			return -1;
		}
#	endif
#endif

//#define __NR_capset                   185
#ifdef L_capset
#	ifdef __NR_capset
		_syscall2(int, capset, void*, header, const void*, data);
#	else
		int capset(void* header, const void* data)
		{
			__set_errno(ENOSYS);
			return -1;
		}
#	endif
#endif

//#define __NR_sigaltstack              186
#ifdef __NR_sigaltstack
#ifdef L_sigaltstack
#include <signal.h>
_syscall2(int, sigaltstack, const struct sigaltstack *, ss, struct sigaltstack *, oss);
#endif
#endif

//#define __NR_sendfile                 187
#ifdef __NR_sendfile
#ifdef L_sendfile
#include <unistd.h>
#include <sys/sendfile.h>
_syscall4(ssize_t,sendfile, int, out_fd, int, in_fd, __off_t *, offset, size_t, count);
#endif
#endif

//#define __NR_getpmsg                  188

//#define __NR_putpmsg                  189

//#define __NR_vfork                    190
//See sysdeps/linux/<arch>vfork.[cS] for architecture specific implementation...

//#define __NR_ugetrlimit		191	/* SuS compliant getrlimit */
#ifdef L___ugetrlimit
#ifdef __NR_ugetrlimit
#define __NR___ugetrlimit __NR_ugetrlimit
#include <unistd.h>
#include <sys/resource.h>
_syscall2(int, __ugetrlimit, enum __rlimit_resource, resource, struct rlimit *, rlim);
int getrlimit (__rlimit_resource_t resource, struct rlimit *rlimits)
{
	return(__ugetrlimit(resource, rlimits));
}
#endif /* __NR_ugetrlimit */
#endif


//#define __NR_mmap2		192


//#define __NR_truncate64         193
//See libc/sysdeps/linux/common/truncate64.c

//#define __NR_ftruncate64        194
//See libc/sysdeps/linux/common/ftruncate64.c


//#define __NR_stat64             195
#ifdef L___syscall_stat64
#if defined __UCLIBC_HAS_LFS__ && defined __NR_stat64
#define __NR___syscall_stat64 __NR_stat64
#include <unistd.h>
#include <sys/stat.h>
#include <bits/kernel_stat.h>
#include "xstatconv.h"
_syscall2(int, __syscall_stat64, const char *, file_name, struct kernel_stat64 *, buf);
int stat64(const char * file_name, struct stat64 * buf)
{
	int result;
	struct kernel_stat64 kbuf;
	result = __syscall_stat64(file_name, &kbuf);
	if (result == 0) {
		__xstat64_conv(&kbuf, buf);
	}
	return result;
}
#endif /* __UCLIBC_HAS_LFS__ */
#endif

//#define __NR_lstat64            196
#ifdef L___syscall_lstat64
#if defined __UCLIBC_HAS_LFS__ && defined __NR_lstat64
#define __NR___syscall_lstat64 __NR_lstat64
#include <unistd.h>
#include <sys/stat.h>
#include <bits/kernel_stat.h>
#include "xstatconv.h"
_syscall2(int, __syscall_lstat64, const char *, file_name, struct kernel_stat64 *, buf);
int lstat64(const char * file_name, struct stat64 * buf)
{
	int result;
	struct kernel_stat64 kbuf;
	result = __syscall_lstat64(file_name, &kbuf);
	if (result == 0) {
		__xstat64_conv(&kbuf, buf);
	}
	return result;
}
#endif /* __UCLIBC_HAS_LFS__ */
#endif

//#define __NR_fstat64            197
#ifdef L___syscall_fstat64
#if defined __UCLIBC_HAS_LFS__ && defined __NR_fstat64
#define __NR___syscall_fstat64 __NR_fstat64
#include <unistd.h>
#include <sys/stat.h>
#include <bits/kernel_stat.h>
#include "xstatconv.h"
_syscall2(int, __syscall_fstat64, int, filedes, struct kernel_stat64 *, buf);
int fstat64(int fd, struct stat64 * buf)
{
	int result;
	struct kernel_stat64 kbuf;
	result = __syscall_fstat64(fd, &kbuf);
	if (result == 0) {
		__xstat64_conv(&kbuf, buf);
	}
	return result;
}
#endif /* __UCLIBC_HAS_LFS__ */
#endif


//#define __NR_lchown32		198
//#define __NR_getuid32		199
//#define __NR_getgid32		200
//#define __NR_geteuid32		201
//#define __NR_getegid32		202
//#define __NR_setreuid32		203
//#define __NR_setregid32		204
//#define __NR_getgroups32	205
//#define __NR_setgroups32	206
//#define __NR_fchown32		207
//#define __NR_setresuid32	208
//#define __NR_getresuid32	209
//#define __NR_setresgid32	210
//#define __NR_getresgid32	211
//#define __NR_chown32		212
//#define __NR_setuid32		213
//#define __NR_setgid32		214
//#define __NR_setfsuid32		215
//#define __NR_setfsgid32		216
//#define __NR_pivot_root		217
#ifdef __NR_pivot_root
#ifdef L_pivot_root
_syscall2(int, pivot_root, const char *, new_root, const char *, put_old);
#endif
#endif

//#define __NR_mincore		218
//#define __NR_madvise		219
#ifdef __NR_madvise
#ifdef L_madvise
_syscall3(int, madvise, void*, __addr, size_t, __len, int, __advice);
#endif
#endif

//#define __NR_madvise1		219	/* delete when C lib stub is removed */

//#define __NR_getdents64		220
// See getdents64.c

//#define __NR_fcntl64		221
#ifdef L___syscall_fcntl64
#include <stdarg.h>
#include <fcntl.h>
#if defined __UCLIBC_HAS_LFS__ && defined __NR_fcntl64
#define __NR___syscall_fcntl64 __NR_fcntl64
static inline
_syscall3(int, __syscall_fcntl64, int, fd, int, cmd, long, arg);
int __libc_fcntl64(int fd, int cmd, ...)
{
	long arg;
	va_list list;
	va_start(list, cmd);
	arg = va_arg(list, long);
	va_end(list);
	return(__syscall_fcntl64(fd, cmd, arg));
}
weak_alias(__libc_fcntl64, fcntl64)
#endif
#endif

//#define __NR_security		223	/* syscall for security modules */
//#define __NR_gettid		224
//#define __NR_readahead		225

