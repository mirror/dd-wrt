# Pound - the reverse-proxy load-balancer                -*- autoconf -*-
# Copyright (C) 2002-2010 Apsis GmbH
# Copyright (C) 2018-2022 Sergey Poznyakoff
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
AC_PREREQ([2.69])
AC_INIT([pound],[4.1.91],[gray@gnu.org],
	[pound],[https://github.com/graygnuorg/pound])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/pound.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.15 gnu tar-ustar])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
AX_PTHREAD([CC="$PTHREAD_CC"],
	   [AC_MSG_ERROR([No suitable pthread library found])])

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

AC_SUBST(I_OWNER)
AC_ARG_WITH([owner],
 [AS_HELP_STRING([--with-owner=name],[The account that will own the files installed by Pound])],
 [I_OWNER="${with_owner}"])

AC_SUBST(I_GRP)
AC_ARG_WITH([group],
 [AS_HELP_STRING([--with-group=name],[The group that will own the files installed by Pound])],
 [I_GRP="${with_group}"])

AC_ARG_ENABLE([super],
 [AS_HELP_STRING([--enable-super],[enable or disable running with supervisor process (default: enabled)])],
 [if test ${enableval} = "yes"; then
    SUPERVISOR=1
  else
    SUPERVISOR=0
  fi],
 [SUPERVISOR=1])

AC_DEFINE_UNQUOTED([SUPERVISOR],[$SUPERVISOR],
 [Compile supervisor code])

AC_ARG_ENABLE([pcreposix],
 [AS_HELP_STRING([--enable-pcreposix],[enable or disable using the pcreposix library (default: enabled if available)])],
 [status_pcreposix=${enableval}],
 [status_pcreposix=yes])

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

if test "$status_pcreposix" != no; then
  AC_CHECK_LIB([pcre],[pcre_compile])
  AC_CHECK_LIB([pcreposix],[regcomp])
  if test $ac_cv_lib_pcreposix_regcomp = yes; then
    status_pcreposix=yes
    AC_DEFINE([HAVE_LIBPCREPOSIX],[1],[Define if compiling with libpcreposix])
  else
    status_pcreposix=no
  fi
fi

AC_CHECK_HEADERS([getopt.h pthread.h openssl/ssl.h openssl/engine.h pcreposix.h pcre/pcreposix.h])

AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_UNSIGNED_LONG_LONG_INT
AC_TYPE_LONG_LONG_INT

AC_MSG_CHECKING([for default pound owner user name])
if test -z "${I_OWNER}"; then
    for u in proxy www daemon bin sys root
    do
	if id $u >/dev/null 2>&1; then
	    I_OWNER=$u
	    break
	fi
    done
fi
AC_MSG_RESULT([$I_OWNER])

AC_MSG_CHECKING([for default pound owner group name])
if test -z "${I_GRP}"; then
    I_GRP=$(id -n -g $I_OWNER)
    if test -z $I_GRP; then
	for g in proxy www daemon bin sys root
	do
	    if grep "^$g:" /etc/group >/dev/null 2>&1; then
		I_GRP=$g
		break
	    fi
	done
    fi
fi
AC_MSG_RESULT([$I_GRP])

AC_MSG_CHECKING([if building from git clone])
if test -d .git; then
    build_from_git=yes
else
    build_from_git=no
fi
AC_MSG_RESULT([$build_from_git])
AM_CONDITIONAL([FROM_GIT],[test "$build_from_git" = yes])

AC_CONFIG_COMMANDS([status],[
cat <<EOF

*******************************************************************
Pound configuration parameters:

Buffer size ................................... $bufsize
Owner user .................................... $owner_user
Owner group ................................... $owner_group
Supervisor process ............................ $status_supervisor
PCRE POSIX library ............................ $status_pcreposix
Memory allocator .............................. $memory_allocator

*******************************************************************

EOF
],
[bufsize=$MAXBUF
owner_user=$I_OWNER
owner_group=$I_GRP
if test $SUPERVISOR = 1; then
  status_supervisor=yes
else
  status_supervisor=no
fi
status_pcreposix=$status_pcreposix
memory_allocator=$memory_allocator
])

AC_CONFIG_TESTDIR(tests)
AC_CONFIG_FILES([tests/Makefile tests/atlocal])
AM_MISSING_PROG([AUTOM4TE], [autom4te])

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 doc/Makefile])
AC_OUTPUT
