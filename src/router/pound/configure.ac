# Pound - the reverse-proxy load-balancer                -*- autoconf -*-
# Copyright (C) 2002-2010 Apsis GmbH
# Copyright (C) 2018-2025 Sergey Poznyakoff
#
# Pound is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Pound is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pound.  If not, see <http://www.gnu.org/licenses/>.
AC_PREREQ([2.71])
AC_INIT([pound],[4.19],[gray@gnu.org],
	[pound],[https://github.com/graygnuorg/pound])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/pound.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.16.5 gnu tar-ustar subdir-objects])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
LT_INIT

# Checks for libraries.
AX_PTHREAD([CC="$PTHREAD_CC"],
	   [AC_MSG_ERROR([No suitable pthread library found])])
AC_CHECK_LIB(crypt, crypt)

# Checks for headers
AC_CHECK_HEADERS([getopt.h pthread.h crypt.h openssl/ssl.h openssl/engine.h])

AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_UNSIGNED_LONG_LONG_INT
AC_TYPE_LONG_LONG_INT

# Check whether they want dynamic backends
AC_ARG_ENABLE([dynamic-backends],
 [AS_HELP_STRING([--enable-dynamic-backends],
		 [enable dynamic backend support (default)])],
 [status_dynamic_backends=${enableval}],
 [status_dynamic_backends=probe])

if test $status_dynamic_backends != no; then
  # Check for adns library
  AC_CHECK_HEADERS([adns.h])
  AC_CHECK_LIB([adns], [adns_init])
  if test "$ac_cv_lib_adns_adns_init$ac_cv_header_adns_h" = yesyes; then
    AC_DEFINE([ENABLE_DYNAMIC_BACKENDS], [1],
	      [Define if dynamic backends are supported])
    status_dynamic_backends=yes
  elif test $status_dynamic_backends = yes; then
    AC_MSG_FAILURE([required library adns not found; install it or use --disable-resolver to disable])
  else
    status_dynamic_backends=no
  fi
fi
AM_CONDITIONAL([COND_DYNAMIC_BACKENDS], [test $status_dynamic_backends = yes])

AC_ARG_ENABLE([dns-tests],
 [AS_HELP_STRING([--enable-dns-tests],
		 [enable DNS-based dynamic backend tests])],
 [if test "${status_dynamic_backends}${enableval}" = yesyes; then
    status_dns_tests=yes
  else
    status_dns_tests=no
  fi
 ],
 [status_dns_tests=no])

if test $status_dns_tests = yes; then
  AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([
	      #include <sys/syscall.h>
	      int x[] = { SYS_connect, SYS_sendto, SYS_recvfrom };
	   ],
	   [[void main(){}]])],
	 [],
	 [status_dns_tests=no])
fi

AM_CONDITIONAL([COND_BUILD_FAKEDNS], [test "$status_dns_tests" = yes])

AC_SUBST(SSL_CPPFLAGS)
AC_SUBST(SSL_LDFLAGS)
AC_ARG_WITH([ssl],
 [AS_HELP_STRING([--with-ssl=directory],[location of OpenSSL package])],
 [SSL_CPPFLAGS="-I${withval}/include"
  SSL_LDFLAGS="-L${withval}/lib ${LDFLAGS}"
  C_SSL="${withval}"],
 [C_SSL=""])

saved_CPPFLAGS=$CFLAGS
CPPFLAGS=$SSL_CPPFLAGS
saved_LDFLAGS=$LDFLAGS
LDFLAGS=$SSL_LDFLAGS
AC_MSG_CHECKING([for OpenSSL version 3])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <openssl/opensslv.h>],
 [#if OPENSSL_VERSION_MAJOR >= 3
#error "OpenSSL v3"
#endif
])],[openssl_v3=no],[openssl_v3=yes])
AC_MSG_RESULT([$openssl_v3])
CPPFLAGS=$saved_CPPFLAGS
LDFLAGS=$saved_LDFLAGS
AM_CONDITIONAL([OPENSSL_V3],[test "$openssl_v3" = "yes"])

# Check for structures
AC_CHECK_MEMBERS([struct stat.st_mtim],[],[],
 [#include <sys/types.h>
#include <sys/stat.h>
])

AC_ARG_WITH([t_rsa],
 [AS_HELP_STRING([--with-t_rsa=nnn],[Timeout for RSA ephemeral keys generation])],
 [T_RSA_KEYS=${withval}],
 [T_RSA_KEYS=7200])

AC_DEFINE_UNQUOTED([T_RSA_KEYS],[$T_RSA_KEYS],
 [Timeout for RSA ephemeral keys generation])

AC_ARG_WITH([dh],
 [AS_HELP_STRING([--with-dh=nnn],[DH key length parameter (default: 2048, can set to 1024)])],
 [case ${withval} in
  1024|2048) DH_LEN=${withval};;
  *) AC_MSG_FAILURE([invalid value for DH key length])
  esac],
 [DH_LEN=2048])

AC_SUBST(DH_LEN)
AC_DEFINE_UNQUOTED([DH_LEN],[$DH_LEN],
 [DH key length])

AC_ARG_WITH([maxbuf],
 [AS_HELP_STRING([--with-maxbuf=nnn],[Value of the MAXBUF parameter (default: 4096)])],
 [MAXBUF=${withval}],
 [MAXBUF=4096])

AC_DEFINE_UNQUOTED([MAXBUF],[$MAXBUF],
 [Max. buffer size])

AC_ARG_ENABLE([tcmalloc],
 [AS_HELP_STRING([--enable-tcmalloc],[enable or disable using the tcmalloc library (default: enabled if available)])],
 [status_tcmalloc=${enableval}],
 [status_tcmalloc=probe])

AC_ARG_ENABLE([hoard],
 [AS_HELP_STRING([--enable-hoard],[enable or disable using the hoard library (default: enabled if available and tcmalloc NOT available)])],
 [status_hoard=${enableval}],
 [status_hoard=probe])

memory_allocator=libc
if test $status_tcmalloc != no; then
  AC_CHECK_LIB([tcmalloc],[malloc])
  if test $ac_cv_lib_tcmalloc_malloc = yes; then
    memory_allocator=tcmalloc
  fi
elif test $status_hoard != no; then
  AC_CHECK_LIB([hoard],[malloc])
  if test $ac_cv_lib_hoard_malloc = yes; then
    memory_allocator=hoard
  fi
fi
AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([nsl], [gethostbyaddr])
AC_CHECK_LIB([socket], [socket])
AC_CHECK_LIB([resolv], [hstrerror])

AC_ARG_ENABLE([pthread-cancel-probe],
 [AS_HELP_STRING([--enable-pthread-cancel-probe],
		 [enable early probe of the pthread_cancel function])],
  [case ${enableval} in
   yes) early_pthread_cancel_probe=1;;
   no)  early_pthread_cancel_probe=0;;
   *)   early_pthread_cancel_probe=0;;
   esac],
  [case $host_os in
   *-gnu|*-gnulibc1|*-gnueabi|*-gnueabihf)
	  early_pthread_cancel_probe=1;;
   *)     early_pthread_cancel_probe=0;;
   esac])

AC_DEFINE_UNQUOTED([EARLY_PTHREAD_CANCEL_PROBE],[$early_pthread_cancel_probe],
 [Define to try pthread_cancel before chroot, to force loading necessary libraries])

AC_CHECK_LIB([crypto],[BIO_new],[],
	     [AC_MSG_FAILURE([Missing OpenSSL (-lcrypto) - aborted],[1])])
AC_CHECK_LIB([ssl],[SSL_CTX_new],[],
	     [AC_MSG_FAILURE([Missing OpenSSL (-lssl) - aborted])])

AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <openssl/ssl.h>],
[SSL_CTX *ctx = NULL;
SSL_CTX_set_dh_auto (ctx, 1);
])],
[SET_DH_AUTO=1],[SET_DH_AUTO=0])

AM_CONDITIONAL([SET_DH_AUTO],[test "$SET_DH_AUTO" = 1])
AC_DEFINE_UNQUOTED([SET_DH_AUTO],[$SET_DH_AUTO],
  [Define to 1 of *set_dh_auto macros are available])

PND_PCRE
AM_CONDITIONAL([COND_PCRE], [test $status_pcre = 1])
AM_CONDITIONAL([COND_PCRE2], [test $status_pcre = 2])

AC_ARG_ENABLE([lua],
 [AS_HELP_STRING([--enable-lua],
		 [enable Lua extensions])],
 [status_lua=${withval}],
 [status_lua=probe])

if test $status_lua != no; then
  PKG_PROG_PKG_CONFIG
  PKG_CHECK_MODULES([LUA], [lua >= 5.3],
      [status_lua=yes
       AC_DEFINE([ENABLE_LUA], [1],
		 [Define if Lua extension language is supported])],
      [status_lua=no])
  AC_MSG_CHECKING([For liblua 5.3 or newer])
  AC_MSG_RESULT([$status_lua])
fi

AM_CONDITIONAL([COND_LUA], [test $status_lua = yes])

# **********************
# Event monitoring
# **********************
AC_CHECK_HEADERS([sys/inotify.h sys/event.h])
AC_CHECK_FUNCS([inotify_init kqueue])

if test "$ac_cv_header_sys_inotify_h/$ac_cv_func_inotify_init" = yes/yes; then
  status_fsevmon=inotify
elif test "$ac_cv_header_sys_event_h/$ac_cv_func_kqueue" = yes/yes; then
  status_fsevmon=kqueue
else
  status_fsevmon=none
fi

AC_ARG_WITH(fsevmon,
	[AS_HELP_STRING([--without-fsevmon],
	  [disable filesystem event monitoring API])],
	[case $withval in
	 inotify)
	     if test $status_fsevmon != inotify; then
	       AC_MSG_ERROR([inotify is requested, but not available])
	     fi
	     ;;
	 kqueue)
	     if test $status_fsevmon != kqueue; then
	       AC_MSG_ERROR([kqueue is requested, but not available])
	     fi
	     ;;
	 yes) ;;
	 no|periodic) status_fsevmon=none
	     ;;
	 *) AC_MSG_ERROR([unsupported value for --with-fsevmon: $witval])
	 esac])

AC_ARG_WITH(inotify,[],
	    [case $withval$status_fsevmon in
	     noinotify)
	       status_fsevmon=none
	       ;;
	     no*|yesinotify)
	       ;;
	     *)
	       AC_MSG_ERROR([--with-inotify doesn't take arguments])
	     esac])

AM_CONDITIONAL([COND_INOTIFY],[test $status_fsevmon = inotify])
AM_CONDITIONAL([COND_KQUEUE],[test $status_fsevmon = kqueue])

AC_MSG_CHECKING([if building from git clone])
if test -d .git; then
    build_from_git=yes
else
    build_from_git=no
fi
AC_MSG_RESULT([$build_from_git])
AM_CONDITIONAL([FROM_GIT],[test "$build_from_git" = yes])

# Documentation
AC_ARG_VAR([MAKEINFO_INIT_FILE],
 [Initialization file for generation of the html manual])

AM_CONDITIONAL([COND_MAKEINFO_INIT_FILE],[test -n "$MAKEINFO_INIT_FILE"])

AC_CONFIG_COMMANDS([status],[
cat <<EOF

*******************************************************************
Pound configuration parameters:

Buffer size ................................... $bufsize
Regular expressions ........................... POSIX$status_pcre
Memory allocator .............................. $memory_allocator
Early pthread_cancel probe .................... $status_pthread_cancel_probe
Dynamic backends .............................. $status_dynamic_backends
Test dynamic backends ......................... $status_dns_tests
Filesystem event monitoring ................... $status_fsevmon
Lua support ................................... $status_lua
*******************************************************************

EOF
],
[bufsize=$MAXBUF
if test $status_pcre != no; then
  status_pcre=", PCRE$status_pcre"
else
  status_pcre=""
fi
memory_allocator=$memory_allocator
if test "$early_pthread_cancel_probe" = 1; then
  status_pthread_cancel_probe=yes
else
  status_pthread_cancel_probe=no
fi
status_dynamic_backends=$status_dynamic_backends
status_dns_tests=$status_dns_tests
status_lua=$status_lua
status_fsevmon=$status_fsevmon
])

AC_CONFIG_TESTDIR(tests)
AC_CONFIG_FILES([tests/Makefile tests/atlocal])
AM_MISSING_PROG([AUTOM4TE], [autom4te])

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 doc/Makefile])
AC_OUTPUT
