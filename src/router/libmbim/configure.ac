dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.68])

dnl The libmbim version number
m4_define([mbim_major_version], [1])
m4_define([mbim_minor_version], [15])
m4_define([mbim_micro_version], [0])
m4_define([mbim_version],
          [mbim_major_version.mbim_minor_version.mbim_micro_version])

dnl libtool versioning for libmbim-glib (-version-info c:r:a)
dnl    If the interface is unchanged, but the implementation has changed or
dnl        been fixed, then increment r.
dnl    Otherwise, increment c and zero r.
dnl        If the interface has grown (that is, the new library is compatible
dnl            with old code), increment a.
dnl        If the interface has changed in an incompatible way (that is,
dnl            functions have changed or been removed), then zero a.
m4_define([mbim_glib_lt_current],  [6])
m4_define([mbim_glib_lt_revision], [0])
m4_define([mbim_glib_lt_age],      [2])


AC_INIT([libmbim], [mbim_version], [libmbim-devel@lists.freedesktop.org])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 no-define no-dist-gzip dist-xz tar-ustar -Wno-portability])
AM_MAINTAINER_MODE([enable])

dnl Support silent build rules. Disable
dnl by either passing --disable-silent-rules to configure or passing V=1
dnl to make
AM_SILENT_RULES([yes])

dnl Required programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

dnl Initialize libtool
LT_PREREQ([2.2])
LT_INIT

dnl Specific warnings to always use
LIBMBIM_COMPILER_WARNINGS

dnl Version stuff
MBIM_MAJOR_VERSION=mbim_major_version
MBIM_MINOR_VERSION=mbim_minor_version
MBIM_MICRO_VERSION=mbim_micro_version
MBIM_VERSION=mbim_version
AC_SUBST(MBIM_MAJOR_VERSION)
AC_SUBST(MBIM_MINOR_VERSION)
AC_SUBST(MBIM_MICRO_VERSION)
AC_SUBST(MBIM_VERSION)

dnl libtool version stuff
MBIM_GLIB_LT_CURRENT=mbim_glib_lt_current
MBIM_GLIB_LT_REVISION=mbim_glib_lt_revision
MBIM_GLIB_LT_AGE=mbim_glib_lt_age
AC_SUBST(MBIM_GLIB_LT_CURRENT)
AC_SUBST(MBIM_GLIB_LT_REVISION)
AC_SUBST(MBIM_GLIB_LT_AGE)

dnl Required dependency versions
GLIB_MIN_VERSION=2.36

dnl General dependencies for libmbim-glib
PKG_CHECK_MODULES(LIBMBIM_GLIB,
                  glib-2.0 >= $GLIB_MIN_VERSION
                  gobject-2.0
                  gio-2.0
                  gio-unix-2.0)
AC_SUBST(LIBMBIM_GLIB_CFLAGS)
AC_SUBST(LIBMBIM_GLIB_LIBS)

dnl General dependencies for mbimcli
PKG_CHECK_MODULES(MBIMCLI,
                  glib-2.0 >= $GLIB_MIN_VERSION
                  gobject-2.0
                  gio-2.0
                  json-c)
AC_SUBST(MBIMCLI_CFLAGS)
AC_SUBST(MBIMCLI_LIBS)

dnl General dependencies for mbim-proxy
PKG_CHECK_MODULES(MBIMPROXY,
                  glib-2.0 >= $GLIB_MIN_VERSION
                  gobject-2.0
                  gio-2.0)
AC_SUBST(MBIMPROXY_CFLAGS)
AC_SUBST(MBIMPROXY_LIBS)

GLIB_MKENUMS=`$PKG_CONFIG --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_MKENUMS)

# MBIM username
MBIM_USERNAME="root"
AC_ARG_ENABLE(mbim-username,
              AS_HELP_STRING([--enable-mbim-username=<username>], [user allowed to access MBIM devices]))
if test -n "$enable_mbim_username" ; then
    MBIM_USERNAME_ENABLED=yes
    AC_DEFINE(MBIM_USERNAME_ENABLED, 1, [Define if we enable MBIM username])
    MBIM_USERNAME="$enable_mbim_username"
    AC_SUBST(MBIM_USERNAME)
    AC_DEFINE_UNQUOTED(MBIM_USERNAME, "$MBIM_USERNAME", [Define the MBIM username])
else
    MBIM_USERNAME_ENABLED=no
fi

AM_CONDITIONAL([MBIM_USERNAME_ENABLED], [test "x$MBIM_USERNAME_ENABLED" = "xyes"])

# udev support (auto)
GUDEV_VERSION=147

PKG_CHECK_MODULES(GUDEV, [gudev-1.0 >= $GUDEV_VERSION], [have_gudev=yes],[have_gudev=no])

AC_ARG_WITH(udev,
            AS_HELP_STRING([--with-udev], [Build with udev support [[default=auto]]]),, [with_udev=auto])

if test "x$with_udev" = "xauto"; then
   if test "x$have_gudev" = "xno"; then
       with_udev="no"
   else
       with_udev="yes"
   fi
fi

case $with_udev in
    yes)
        if test "x$have_gudev" = "xno"; then
            AC_MSG_ERROR([Couldn't find gudev >= $GUDEV_VERSION. Install it, or otherwise configure using --without-udev to disable udev support.])
        else
            AC_DEFINE(WITH_UDEV, 1, [Define if you want udev support])
            AC_SUBST(GUDEV_CFLAGS)
            AC_SUBST(GUDEV_LIBS)
        fi
        ;;
    *)
        with_udev=no
        ;;
esac

# udev base directory
AC_ARG_WITH(udev-base-dir, AS_HELP_STRING([--with-udev-base-dir=DIR], [where udev base directory is]))
if test -n "$with_udev_base_dir" ; then
    UDEV_BASE_DIR="$with_udev_base_dir"
else
    UDEV_BASE_DIR="/lib/udev"
fi
AC_SUBST(UDEV_BASE_DIR)

AC_CONFIG_FILES([Makefile
                 build-aux/Makefile
                 build-aux/templates/Makefile
                 build-aux/mbim-codegen/Makefile
                 data/Makefile
                 data/pkg-config/Makefile
                 data/pkg-config/mbim-glib.pc
                 src/Makefile
                 src/libmbim-glib/Makefile
                 src/libmbim-glib/mbim-version.h
                 src/libmbim-glib/generated/Makefile
                 src/libmbim-glib/test/Makefile
                 src/mbimcli/Makefile
                 src/mbim-proxy/Makefile
                 utils/Makefile])

if test "x$MBIM_USERNAME_ENABLED" = "xyes"; then
    AC_CONFIG_FILES([src/mbim-proxy/76-mbim-proxy-device-ownership.rules])
fi

AC_OUTPUT

echo "
    libmbim $VERSION
    ==============================================

    Build:
      compiler:             ${CC}
      cflags:               ${CFLAGS}
      ldflags:              ${LDFLAGS}
      Maintainer mode:      ${USE_MAINTAINER_MODE}

    System paths:
      prefix:               ${prefix}
      udev base directory:  ${UDEV_BASE_DIR}

    Features:
      udev support:         ${with_udev}
      MBIM username:        ${MBIM_USERNAME_ENABLED} (${MBIM_USERNAME})
"
