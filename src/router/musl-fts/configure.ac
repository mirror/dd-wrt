dnl Process this file with autoconf to produce a configure script

AC_PREREQ(2.69)
AC_INIT([fts], [1.2.7], [pullmoll@t-online.de])
AM_INIT_AUTOMAKE([1.15])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
LT_INIT

AC_CHECK_HEADERS(assert.h dirent.h errno.h fcntl.h stdlib.h string.h unistd.h sys/param.h sys/stat.h)

AC_CHECK_DECLS([MAX], , , [
#include <sys/param.h>
	])

AC_CHECK_DECLS([UINTMAX_MAX], , , [
#include <limits.h>
	])

AC_CHECK_FUNCS([dirfd])

AC_CHECK_MEMBERS([DIR.dd_fd, DIR.d_fd],,,
[#include <sys/types.h>
#include <dirent.h>
	])

PKG_INSTALLDIR

AC_CONFIG_FILES([Makefile musl-fts.pc])
AC_OUTPUT
