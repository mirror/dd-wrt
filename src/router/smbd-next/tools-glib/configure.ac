AC_PREREQ([2.68])

m4_define([ksmbd_tools_version], m4_esyscmd_s(
	 grep "define KSMBD_TOOLS_VERSION " include/version.h | \
		 awk '{print $3}' | sed 's/\"//g'))

AC_INIT([ksmbd-tools],
	ksmbd_tools_version,
	[linkinjeon@kernel.org],
	[ksmbd-tools],
	[https://github.com/cifsd-team/cifsd-tools])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign tar-pax subdir-objects])

AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_STDC
AM_SILENT_RULES([yes])
AC_PROG_LIBTOOL

# enable options
AC_ARG_ENABLE(krb5,
	      [AC_HELP_STRING([--enable-krb5], [Enable kerberos authentication @<:@default=no@:>@:])],
	      enable_krb5=$enableval,
	      enable_krb5="no")

# Checks for header files.
if test "$enable_krb5" != "no"; then
	AC_CHECK_HEADERS([krb5.h])
	if test x$ac_cv_header_krb5_h != xyes; then
		if test "$enable_krb5" = "yes"; then
			AC_MSG_ERROR([krb5.h not found, please install krb5-libs-devel.])
		else
			AC_MSG_WARN([krb5.h not found, disabling krb5.])
			enable_krb5="no"
		fi
	fi
fi

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

if test "$enable_krb5" != "no"; then
	PKG_CHECK_MODULES([LIBKRB5], [krb5])
	AC_DEFINE([CONFIG_KRB5], [], "support kerberos authentication")
fi
AM_CONDITIONAL(HAVE_LIBKRB5, [test "$enable_krb5" != "no"])

AC_CONFIG_FILES([
	Makefile
	lib/Makefile
	mountd/Makefile
	adduser/Makefile
	addshare/Makefile
	control/Makefile
])

AC_OUTPUT
