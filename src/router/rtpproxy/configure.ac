#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.58)
AC_INIT(rtpproxy, 2.0.beta.20150106, sobomax@sippysoft.com)
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

# cross-compile macros
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MKDIR_P
AM_PROG_CC_C_O
LT_INIT

case "${host_os}" in
freebsd*)
  CPPFLAGS="-I/usr/local/include -pthread"
  LDFLAGS="-L/usr/local/lib"
  #LIBS=
  ;;
linux*)
  CPPFLAGS="-D_BSD_SOURCE -D_ISOC99_SOURCE -DLINUX_XXX"
  LIBS="-lrt"
  ;;
solaris*)
  LIBS="-lsocket -lnsl -lxnet -lrt"
  ;;
*)
  ;;
esac

AC_ARG_ENABLE(systemd,
AS_HELP_STRING([--enable-systemd], [enable systemd support in rtpproxy]),
[],
[enable_systemd=no])

# Checks for libraries.

if test "$enable_systemd" = 'yes'
then
  AC_SEARCH_LIBS(sd_listen_fds,[systemd systemd-daemon],
   [have_sd_listen_fds=yes],
   [AC_MSG_ERROR([Cannot find sd_listen_fds function])],)
  AC_CHECK_HEADERS(systemd/sd-daemon.h,
   [have_systemd_sd_daemon_h=yes],
   [AC_MSG_ERROR([Cannot find <systemd/sd-daemon.h> header])])
  if test x"$have_sd_listen_fds" = x"yes" && \
     test x"$have_systemd_sd_daemon_h" = x"yes"
  then
     AC_DEFINE([HAVE_SYSTEMD_DAEMON],[1],[Define if you have systemd daemon])
  fi

fi

# GSM
AC_CHECK_HEADERS(gsm.h, found_libgsm=yes)
if test "$found_libgsm" = yes
then
  AC_CHECK_LIB(gsm, gsm_create,
   LIBS_GSM="-lgsm"
   AC_DEFINE([ENABLE_GSM], 1, [Define if you have libgsm library installed]))
fi

# G.729
AC_CHECK_HEADERS(g729_encoder.h, found_libg729=yes)
if test "$found_libg729" = yes
then
  AC_CHECK_LIB(g729, g729_encoder_new,
   LIBS_G729="-lg729 -lm"
   AC_DEFINE([ENABLE_G729], 1, [Define if you have G.729 support]),,
   -lm
  )
else
  AC_CHECK_HEADERS(bcg729/encoder.h, found_libbcg729=yes)
  if test "$found_libbcg729" = yes
  then
    AC_CHECK_LIB(bcg729, initBcg729EncoderChannel,
     LIBS_G729=`pkg-config --libs libbcg729`
     AC_DEFINE([ENABLE_G729], 1, [Define if you have G.729 support])
     AC_DEFINE([ENABLE_BCG729], 1, [Define if you have bcg729 library])
    )
  fi
fi

# G.722
AC_CHECK_HEADERS(g722.h, found_libg722=yes)
if test "$found_libg722" = yes
then
  AC_CHECK_LIB(g722, g722_encoder_new,
   LIBS_G722="-lg722"
   AC_DEFINE([ENABLE_G722], 1, [Define if you have libg722 library installed]))
fi

# libsndfile
AC_CHECK_HEADERS(sndfile.h, found_libsndfile=yes)
if test "$found_libsndfile" = yes
then
  AC_CHECK_LIB(sndfile, sf_open,
   LIBS_SNDFILE="-lsndfile"
   AC_DEFINE([ENABLE_SNDFILE], 1, [Define if you have libsndfile library installed]))
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h err.h endian.h sys/endian.h])

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

AM_CONDITIONAL(BUILD_EXTRACTAUDIO, [test "${LIBS_SNDFILE}" != ""])

AC_CONFIG_FILES([Makefile src/Makefile makeann/Makefile tests/Makefile extractaudio/Makefile])
AC_SUBST(AM_CFLAGS)
AC_SUBST(LIBS_GSM)
AC_SUBST(LIBS_G729)
AC_SUBST(LIBS_G722)
AC_SUBST(LIBS_SNDFILE)
AC_OUTPUT
