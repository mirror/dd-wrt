AC_PREREQ([2.68])

m4_define([ksmbd_tools_version], m4_esyscmd_s([
exec awk '/define KSMBD_TOOLS_VERSION / { gsub(/"/,"",$3); printf "%s", $3 }' include/version.h
                                              ]))

AC_INIT([ksmbd-tools],
        [ksmbd_tools_version],
        [linkinjeon@kernel.org],
        [ksmbd-tools],
        [https://github.com/cifsd-team/ksmbd-tools])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign tar-pax subdir-objects])

AC_SUBST([ksmbd_tools_version], ksmbd_tools_version)

AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_STDC
AM_SILENT_RULES([yes])
AC_PROG_LIBTOOL
AC_PROG_SED
AC_PROG_MKDIR_P
AC_PROG_LN_S

AC_SUBST([in_script], [[\
'$(SED) -e "s,[@]sbindir[@],$(sbindir),g" \
        -e "s,[@]sysconfdir[@],$(sysconfdir),g" \
        -e "s,[@]runstatedir[@],$(runstatedir),g" \
        -e "s,[@]ksmbd_tools_version[@],$(ksmbd_tools_version),g"'
                       ]])

AC_ARG_ENABLE([krb5],
              [AC_HELP_STRING([--enable-krb5], [Enable Kerberos 5 authentication @<:@default=no@:>@])],
              [enable_krb5=$enableval],
              [enable_krb5=no])

AS_IF([test "x$enable_krb5" != xno], [
	PKG_CHECK_MODULES([LIBKRB5], [krb5])

	save_CPPFLAGS=$CPPFLAGS
	CPPFLAGS="$CPPFLAGS $LIBKRB5_CFLAGS"
	AC_CHECK_HEADERS([krb5.h])
	CPPFLAGS=$save_CPPFLAGS

	AS_IF([test "x$ac_cv_header_krb5_h" != xyes], [
		AS_IF([test "x$enable_krb5" = xyes], [
			AC_MSG_ERROR([krb5.h was not found.])
		], [
			AC_MSG_WARN([krb5.h was not found, disabling Kerberos 5 support.])
			enable_krb5=no
		])
	])
])

AC_ARG_WITH([rundir],
            [AC_HELP_STRING([--with-rundir=DIR],
                            [Store modifiable per-process data in DIR @<:@LOCALSTATEDIR/run@:>@])],
            [with_rundir=$withval],
            [with_rundir=no])

AS_IF([test "x$with_rundir" = xno], [
	AS_IF([test "x$runstatedir" = x], [
		runstatedir='${localstatedir}/run'
	], [])
], [
	runstatedir=$with_rundir
])

AC_SUBST([runstatedir])

AC_ARG_WITH([systemdsystemunitdir],
            [AC_HELP_STRING([--with-systemdsystemunitdir@<:@=DIR@:>@],
                            [Install systemd unit file to DIR (query pkg-config by default)])],
            [with_systemdsystemunitdir=$withval],
            [with_systemdsystemunitdir=yes])

AS_IF([test "x$with_systemdsystemunitdir" != xno], [
	AS_IF([test "x$with_systemdsystemunitdir" = xyes], [
		PKG_CHECK_VAR([systemdsystemunitdir], [systemd >= 245], [systemdsystemunitdir], [], [])
	], [
		systemdsystemunitdir=$with_systemdsystemunitdir
	])
])

AC_SUBST([systemdsystemunitdir])

save_LIBS=$LIBS
LIBS=
AC_SEARCH_LIBS([pthread_sigmask], [pthread], [], [
	AC_MSG_ERROR([pthread was not found.])
])
PTHREAD_LIBS=$LIBS
LIBS=$save_LIBS

AC_SUBST([PTHREAD_LIBS])

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.44], [have_glib=yes], [have_glib=no])
AS_IF([test "x$have_glib" != xyes], [
	AC_MSG_ERROR([glib (libglib2.0-dev or glib2-devel) was not found.])
])

PKG_CHECK_MODULES([LIBNL], [libnl-3.0 >= 3.0 libnl-genl-3.0 >= 3.0], [have_libnl=yes], [have_libnl=no])
AS_IF([test "x$have_libnl" != xyes], [
	AC_MSG_ERROR([libnl (libnl-3-dev or libnl3-devel) and libnl-genl (libnl-genl-3-dev) were not found.])
])

AS_IF([test "x$enable_krb5" != xno], [
	AC_DEFINE([CONFIG_KRB5], [], [Define if Kerberos 5 authentication is supported.])

	save_LIBS=$LIBS
	LIBS="$LIBS $LIBKRB5_LIBS"
	AC_CHECK_FUNCS([krb5_auth_con_getrecvsubkey])
	LIBS=$save_LIBS

	save_CPPFLAGS=$CPPFLAGS
	CPPFLAGS="$CPPFLAGS $LIBKRB5_CFLAGS"
	AC_CHECK_MEMBER([krb5_keyblock.keyvalue],
	                [ac_cv_have_krb5_keyblock_keyvalue=yes],
	                [ac_cv_have_krb5_keyblock_keyvalue=no],
	                [[#include <krb5.h>]])
	AC_CHECK_MEMBER([krb5_authenticator.client],
	                [ac_cv_have_krb5_authenticator_client=yes],
	                [ac_cv_have_krb5_authenticator_client=no],
	                [[#include <krb5.h>]])
	AC_CACHE_CHECK([for krb5_authenticator** parameter in krb5_auth_con_getauthenticator],
	               [ac_cv_have_krb5_auth_con_getauthenticator_double_pointer],
	               [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <krb5.h>
krb5_error_code krb5_auth_con_getauthenticator(krb5_context, krb5_auth_context, krb5_authenticator**);
	                                                   ]], [])],
	                                  [ac_cv_have_krb5_auth_con_getauthenticator_double_pointer=yes],
	                                  [ac_cv_have_krb5_auth_con_getauthenticator_double_pointer=no])])
	CPPFLAGS=$save_CPPFLAGS

	AS_IF([test "x$ac_cv_have_krb5_keyblock_keyvalue" = xyes], [
		AC_DEFINE([HAVE_KRB5_KEYBLOCK_KEYVALUE], [],
		          [Define if krb5_keyblock has keyvalue member.])
	])
	AS_IF([test "x$ac_cv_have_krb5_authenticator_client" = xyes], [
		AC_DEFINE([HAVE_KRB5_AUTHENTICATOR_CLIENT], [],
		          [Define if krb5_authenticator has client member.])
	])
	AS_IF([test "x$ac_cv_have_krb5_auth_con_getauthenticator_double_pointer" = xyes], [
		AC_DEFINE([HAVE_KRB5_AUTH_CON_GETAUTHENTICATOR_DOUBLE_POINTER], [],
		          [Define if krb5_auth_con_getauthenticator has krb5_authenticator** parameter.])
	])
])

AM_CONDITIONAL(HAVE_LIBKRB5, [test "x$enable_krb5" != xno])

AC_CONFIG_FILES([
	Makefile
	addshare/Makefile
	adduser/Makefile
	control/Makefile
	mountd/Makefile
	tools/Makefile
])

AC_OUTPUT
