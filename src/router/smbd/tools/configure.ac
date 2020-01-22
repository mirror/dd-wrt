AC_PREREQ([2.68])

m4_define([ksmbd_tools_major_ver], [3])
m4_define([ksmbd_tools_minor_ver], [1])
m4_define([ksmbd_tools_micro_ver], [0])

m4_define([ksmbd_tools_version],
	[ksmbd_tools_major_ver.ksmbd_tools_minor_ver.ksmbd_tools_micro_ver])

AC_INIT([KSMBD tools],
	[m4_esyscmd_s([echo unknown])],
	[linux-cifsd-devel@lists.sourceforge.net])

AC_DEFINE([KSMBD_TOOLS_VERSION],
	"ksmbd_tools_version [m4_esyscmd_s([echo unknown])]",
	["ksmbd-tools version"])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign tar-pax dist-xz subdir-objects])

AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_STDC
AM_SILENT_RULES([yes])
AC_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG([0.9])

# Checks for header files.
AC_CHECK_HEADERS([linux/netlink.h fcntl.h stdlib.h string.h \
		  unistd.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS_ONCE([
	memset
	memcpy
	getpwnam
	sendmsg
	recvmsg
])

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.40], [has_glib2=1], [has_glib2=0])
AS_IF([test "$has_glib2" -eq 0],
      AC_MSG_ERROR([No glib2 (libglib2.0-dev or glib2-devel) was found])
])

has_libnl_ver=0
dnl libnl-genl-3.0.pc pkg-config file just for libnl-3.0 case.
PKG_CHECK_MODULES([LIBNL], [libnl-3.0 >= 3.0 libnl-genl-3.0 >= 3.0], [has_libnl_ver=3], [has_libnl_ver=0])
AS_IF([test "$has_libnl_ver" -eq 0], [
	AC_MSG_ERROR([No libnl (libnl-3-dev or libnl3-devel) and libnl-genl (libnl-genl-3-dev) or were not found])
])

AC_CONFIG_FILES([
	Makefile
	lib/Makefile
	usmbd/Makefile
	smbuseradd/Makefile
	smbshareadd/Makefile
])

AC_OUTPUT
