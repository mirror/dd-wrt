AC_PREREQ([2.68])

m4_define([ksmbd_tools_version], m4_esyscmd_s([awk '/define KSMBD_TOOLS_VERSION / \
	{ gsub(/"/,"",$3); printf "%s", $3 }' include/version.h]))

AC_INIT([ksmbd-tools],
	ksmbd_tools_version,
	[linkinjeon@kernel.org],
	[ksmbd-tools],
	[https://github.com/cifsd-team/ksmbd-tools])

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
	CPPFLAGS="$CPPFLAGS $LIBKRB5_CFLAGS"
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
	AC_DEFINE([CONFIG_KRB5], [], [Define if Kerberos 5 authentication is supported.])
	ksmbd_tools_LIBS=$LIBS
	LIBS="$LIBS $LIBKRB5_LIBS"
	AC_CHECK_FUNCS([krb5_auth_con_getrecvsubkey])
	LIBS=$ksmbd_tools_LIBS
	AC_CACHE_CHECK([for keyvalue in krb5_keyblock], [ac_cv_have_krb5_keyblock_keyvalue], [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <krb5.h>], [krb5_keyblock key; key.keyvalue.data = NULL;])],
			[ac_cv_have_krb5_keyblock_keyvalue="yes"], [ac_cv_have_krb5_keyblock_keyvalue="no"])
	])
	if test "$ac_cv_have_krb5_keyblock_keyvalue" = "yes" ; then
		AC_DEFINE(HAVE_KRB5_KEYBLOCK_KEYVALUE, [], [Define if the krb5_keyblock struct has a keyvalue property.])
	fi
	AC_CACHE_CHECK([for double pointer in krb5_auth_con_getauthenticator], [ac_cv_have_krb5_auth_con_getauthenticator_double_pointer], [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <krb5.h>
				krb5_error_code krb5_auth_con_getauthenticator(krb5_context, krb5_auth_context, krb5_authenticator**);], [])],
			[ac_cv_have_krb5_auth_con_getauthenticator_double_pointer="yes"], [ac_cv_have_krb5_auth_con_getauthenticator_double_pointer="no"])
	])
	if test "$ac_cv_have_krb5_auth_con_getauthenticator_double_pointer" = "yes" ; then
		AC_DEFINE(HAVE_KRB5_AUTH_CON_GETAUTHENTICATOR_DOUBLE_POINTER, [], [Define if the krb5_auth_con_getauthenticator function takes a double pointer.])
	fi
	AC_CACHE_CHECK([for client in krb5_authenticator], [ac_cv_have_krb5_authenticator_client], [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <krb5.h>], [krb5_authenticator authenti; authenti.client = NULL;])],
			[ac_cv_have_krb5_authenticator_client="yes"], [ac_cv_have_krb5_authenticator_client="no"])
	])
	if test "$ac_cv_have_krb5_authenticator_client" = "yes" ; then
		AC_DEFINE(HAVE_KRB5_AUTHENTICATOR_CLIENT, [], [Define if the krb5_authenticator struct has a client property.])
	fi
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
