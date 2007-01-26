#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.58)
AC_INIT(rtpproxy, 0.2, sobomax@sippysoft.com)
AM_INIT_AUTOMAKE(rtpproxy, 0.2)
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([main.c])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# GSM
AC_CHECK_HEADERS(gsm.h, found_libgsm=yes)
if test "$found_libgsm" = yes
then
  AC_CHECK_LIB(gsm, gsm_create,
   LIBS="$LIBS -lgsm"
   GSM_SUPPORT=ext
   AC_DEFINE([ENABLE_GSM], 1, [Define if you have libgsm library installed]))
fi

# G.729
AC_CHECK_HEADERS(g729_encoder.h, found_libg729=yes)
if test "$found_libg729" = yes
then
  AC_CHECK_LIB(g729, g729_encoder_new,
   LIBS="$LIBS -lg729"
   G729_SUPPORT=ext
   AC_DEFINE([ENABLE_G729], 1, [Define if you have libg729 library installed])
  )
fi
if test -z "$G729_SUPPORT"
then
  echo "*************************************************************************** $ECHO_C" 1>&6
  echo "* Please contact sales@sippysoft.com if you need G.729 support in makeann * $ECHO_C" 1>&6
  echo "*************************************************************************** $ECHO_C" 1>&6
  sleep 1
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([atexit gettimeofday memset mkdir socket strchr strdup strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
