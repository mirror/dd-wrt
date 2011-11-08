# 
# Check if we have a working fadvise system call
# 
AC_DEFUN([AC_HAVE_FADVISE],
  [ AC_MSG_CHECKING([for fadvise ])
    AC_TRY_COMPILE([
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <fcntl.h>
    ], [
	posix_fadvise(0, 1, 0, POSIX_FADV_NORMAL);
    ],	have_fadvise=yes
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    AC_SUBST(have_fadvise)
  ])

# 
# Check if we have a working madvise system call
# 
AC_DEFUN([AC_HAVE_MADVISE],
  [ AC_MSG_CHECKING([for madvise ])
    AC_TRY_COMPILE([
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <sys/mman.h>
    ], [
	posix_madvise(0, 0, MADV_NORMAL);
    ],	have_madvise=yes
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    AC_SUBST(have_madvise)
  ])

# 
# Check if we have a working mincore system call
# 
AC_DEFUN([AC_HAVE_MINCORE],
  [ AC_MSG_CHECKING([for mincore ])
    AC_TRY_COMPILE([
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <sys/mman.h>
    ], [
	mincore(0, 0, 0);
    ],	have_mincore=yes
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    AC_SUBST(have_mincore)
  ])

# 
# Check if we have a working sendfile system call
# 
AC_DEFUN([AC_HAVE_SENDFILE],
  [ AC_MSG_CHECKING([for sendfile ])
    AC_TRY_COMPILE([
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <sys/sendfile.h>
    ], [
         sendfile(0, 0, 0, 0);
    ],	have_sendfile=yes
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    AC_SUBST(have_sendfile)
  ])

#
# Check if we have a getmntent libc call (IRIX, Linux)
#
AC_DEFUN([AC_HAVE_GETMNTENT],
  [ AC_MSG_CHECKING([for getmntent ])
    AC_TRY_COMPILE([
#include <stdio.h>
#include <mntent.h>
    ], [
         getmntent(0);
    ], have_getmntent=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_getmntent)
  ])

#
# Check if we have a getmntinfo libc call (FreeBSD, Mac OS X)
#
AC_DEFUN([AC_HAVE_GETMNTINFO],
  [ AC_MSG_CHECKING([for getmntinfo ])
    AC_TRY_COMPILE([
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
    ], [
         getmntinfo(0, 0);
    ], have_getmntinfo=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_getmntinfo)
  ])

#
# Check if we have a fallocate libc call (Linux)
#
AC_DEFUN([AC_HAVE_FALLOCATE],
  [ AC_MSG_CHECKING([for fallocate])
    AC_TRY_LINK([
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <fcntl.h>
#include <linux/falloc.h>
    ], [
         fallocate(0, 0, 0, 0);
    ], have_fallocate=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_fallocate)
  ])

#
# Check if we have the fiemap ioctl (Linux)
#
AC_DEFUN([AC_HAVE_FIEMAP],
  [ AC_CHECK_HEADERS([linux/fiemap.h], [ have_fiemap=yes ], [ have_fiemap=no ])
    AC_SUBST(have_fiemap)
  ])
