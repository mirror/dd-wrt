# Process this file with autoconf to produce a configure script.
AC_INIT(sysfsutils, 2.1.0, linux-diag-devel@lists.sourceforge.net)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER([config.h])

# Checks for KLIBC support (should be before AC_PROG_LIBTOOL and AC_PROG_CC)
AC_CHECK_KLIBC

if test "X$KLIBC" == Xyes; then
	# Should disable/enable building of shared libraries ... I am open to better
	# suggestions though (AC_DISABLE_SHARED and co do not do the right thing in
	# a situation like this that its runtime dependent) ...
	enable_shared="no"
	EXTRA_CFLAGS=
else
	# These ones make KLIBC compiles very loud
	EXTRA_CFLAGS="-Wshadow -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs"
fi
AM_CONDITIONAL([KLIBC],[test "X$KLIBC" == Xyes])
AC_SUBST([EXTRA_CFLAGS])

# Checks for LIBTOOL
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_AWK

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h malloc.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_LSTAT
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([bzero isascii memset strchr strerror strrchr strstr strtol])

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 cmd/Makefile
                 test/Makefile])
AC_OUTPUT
