AC_INIT([bluez-tools], m4_esyscmd([./git-version-gen .tarball-version]), [zxteam@gmail.com], [bluez-tools], [http://code.google.com/p/bluez-tools/])
AC_PREREQ([2.65])
AM_INIT_AUTOMAKE([1.11 no-define foreign -Wall subdir-objects])
AC_CONFIG_HEADERS([config.h])

# The default CFLAGS and CXXFLAGS in Autoconf are "-g -O2" for gcc and just
# "-g" for any other compiler. There doesn't seem to be a standard way of
# getting rid of the -g (which I don't think is needed for a production
# library). This fudge seems to achieve the necessary. First, we remember the
# externally set values of CFLAGS and CXXFLAGS. Then call the AC_PROG_CC and
# AC_PROG_CXX macros to find the compilers - if CFLAGS and CXXFLAGS are not
# set, they will be set to Autoconf's defaults. Afterwards, if the original
# values were not set, remove the -g from the Autoconf defaults.
# (PH 02-May-07)

remember_set_CFLAGS="$CFLAGS"

AC_PROG_CC
AC_PROG_CC_C99

if test "x$remember_set_CFLAGS" = "x"
then
	if test "$CFLAGS" = "-g -O2"
	then
		CFLAGS="-O2"
	elif test "$CFLAGS" = "-g"
	then
		CFLAGS=""
	fi
fi

AC_PROG_INSTALL
AM_PROG_CC_C_O

# Checks for header files.
AC_HEADER_STDC

# Check for the availability of dbus and glib libs
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.36.0])
PKG_CHECK_MODULES([GIO], [gio-2.0 >= 2.26.0 gio-unix-2.0 >= 2.26.0])

# Check for the availability of libreadline
AC_CHECK_HEADERS([readline/readline.h], [HAVE_READLINE_H=1])
AC_CHECK_HEADERS([readline/history.h], [HAVE_HISTORY_H=1])
AC_CHECK_LIB([readline], [readline], [HAVE_LIB_READLINE=1])

if test "$HAVE_READLINE_H" != "1"; then
	echo "** readline/readline.h was not found."
	exit 1
fi
if test "$HAVE_HISTORY_H" != "1"; then
	echo "** readline/history.h was not found."
	exit 1
fi
LIBREADLINE="-lreadline"

AC_SUBST(LIBREADLINE)

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
