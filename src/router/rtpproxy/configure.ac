#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.58)
AC_INIT(rtpproxy, 1.2.beta.200901120, sobomax@sippysoft.com)
AM_INIT_AUTOMAKE(rtpproxy, 1.2.beta.200901120)
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([main.c])
AM_CONFIG_HEADER([config.h])

# cross-compile macros
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC

case "${host_os}" in
freebsd*)
  CPPFLAGS=-I/usr/local/include
  LDFLAGS=-L/usr/local/lib
  ;;
linux*)
  CPPFLAGS=-D_BSD_SOURCE
  ;;
solaris*)
  LDFLAGS="-lsocket -lnsl -lxnet -lrt"
  ;;
*)
  ;;
esac

# Checks for libraries.

# GSM
AC_CHECK_HEADERS(gsm.h, found_libgsm=yes)
if test "$found_libgsm" = yes
then
  AC_CHECK_LIB(gsm, gsm_create,
   LIBS_GSM="-lgsm"
   GSM_SUPPORT=ext
   AC_DEFINE([ENABLE_GSM], 1, [Define if you have libgsm library installed]))
fi

# G.729
AC_CHECK_HEADERS(g729_encoder.h, found_libg729=yes)
if test "$found_libg729" = yes
then
  AC_CHECK_LIB(g729, g729_encoder_new,
   LIBS_G729="-lg729"
   G729_SUPPORT=ext
   AC_DEFINE([ENABLE_G729], 1, [Define if you have libg729 library installed])
  )
fi
##if test -z "$G729_SUPPORT"
##then
##  echo "*************************************************************************** $ECHO_C" 1>&6
##  echo "* Please contact sales@sippysoft.com if you need G.729 support in makeann * $ECHO_C" 1>&6
##  echo "*************************************************************************** $ECHO_C" 1>&6
##  sleep 1
##fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h err.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

dnl check for the sockaddr_un.sun_len member
AC_CHECK_MEMBER([struct sockaddr_un.sun_len],
  [AC_DEFINE(HAVE_SOCKADDR_SUN_LEN,1,[Have the sockaddr_un.sun_len member.])],
  [],
  [ #include <sys/types.h>
  #include <sys/un.h>
  ])

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([atexit gettimeofday memset mkdir socket strchr strdup strerror])

if test "x$GCC" = "xyes"; then
  ## We like to use C99 routines when available.  This makes sure that
  ## __STDC_VERSION__ is set such that libc includes make them available.
  AM_CFLAGS="-std=gnu99 -Wall -Wno-uninitialized"
fi

AC_CONFIG_FILES([Makefile])
AC_SUBST(AM_CFLAGS)
AC_SUBST(LIBS_GSM)
AC_SUBST(LIBS_G729)
AC_OUTPUT
