dnl Process this file with autoconf to create configure.

AC_INIT([libnftnl], [1.3.1])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AM_INIT_AUTOMAKE([-Wall foreign tar-pax no-dist-gzip dist-xz
	1.6 subdir-objects])

dnl kernel style compile messages
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Dependencies
PKG_CHECK_MODULES([LIBMNL], [libmnl >= 1.0.4])
AC_PROG_CC
AM_PROG_CC_C_O
AC_EXEEXT
AC_DISABLE_STATIC
LT_INIT
CHECK_GCC_FVISIBILITY
case "$host" in
*-*-linux* | *-*-uclinux*) ;;
*) AC_MSG_ERROR([Linux only, dude!]);;
esac

regular_CPPFLAGS="-D_FILE_OFFSET_BITS=64 -D_REENTRANT"

regular_CFLAGS="-Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wshadow -Wstrict-prototypes \
	-Wformat=2 -Wwrite-strings -pipe"
AC_SUBST([regular_CPPFLAGS])
AC_SUBST([regular_CFLAGS])
AC_CONFIG_FILES([Makefile src/Makefile include/Makefile include/libnftnl/Makefile include/linux/Makefile include/linux/netfilter/Makefile examples/Makefile tests/Makefile libnftnl.pc doxygen.cfg])
AC_OUTPUT
