AC_INIT([GNU libmicrohttpd],[1.0.1],[libmicrohttpd@gnu.org])
CONFIG_STATUS='./po-config.status'

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_PRESERVE_HELP_ORDER

AC_MSG_NOTICE([
This special $as_me is designed to be run only internally as part of distribution tarball building process.
The only purpose of the $as_me is a preparation of the files to update ${PACKAGE_TARNAME}.pot

$as_me is not meant to be started by the user and is not needed to build ${PACKAGE_NAME} library.
])
AC_MSG_NOTICE([Check src/examples/msgs_i18n.c for inspiration how to use ${PACKAGE_TARNAME}.pot])

AM_PROG_INSTALL_SH
AC_PROG_MKDIR_P

AS_VAR_SET_IF([enable_nls], [], [[enable_nls=no]])
AM_GNU_GETTEXT([external],[need-ngettext])
AM_GNU_GETTEXT_REQUIRE_VERSION([0.18])

m4_divert_text([HELP_VAR_END], [[
###############################################################################
This special po-configure is designed to be run only internally as part of
distribution tarball building process.
The only purpose of the po-configure is a preparation of the files to update
libmicrohttpd.pot

po-configure is not meant to be started by the user and is not needed to build
GNU libmicrohttpd library.
###############################################################################
]])

# Hacks for libmicrohttpd
AC_CONFIG_FILES([po/stamp-m],[echo 'timestamp' > po/stamp-m])
AC_SUBST([POMAKEFILEDEPS],["POTFILES.in stamp-m"])
AM_SUBST_NOTMAKE([POMAKEFILEDEPS])
AC_SUBST([MHD_CONFIG_SHELL],["${CONFIG_SHELL}"])
AC_SUBST([MHD_AUX_DIR],['build-aux'])

AC_CONFIG_FILES([po/Makefile.in])
AS_IF([test -z "${XGETTEXT}" || test "x${XGETTEXT}" = "x:"], [AC_MSG_ERROR([Cannot find xgettext. xgettext is required to update distribution tarball files.])])
AC_SUBST([PACKAGE],["${PACKAGE_TARNAME}"])
AC_SUBST([VERSION],["${PACKAGE_VERSION}"])
AC_SUBST([CROSS_COMPILING],["${cross_compiling}"])
AC_SUBST([MKDIR_P])
AC_SUBST([mkdir_p],['$(MKDIR_P)'])

AC_OUTPUT
