AC_PREREQ(2.63)
AC_INIT([pound],[2.8-gray],[gray+pound@gnu.org.ua],
        [pound],[http://www.apsis.ch/pound])
AC_CONFIG_SRCDIR([pound.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 foreign tar-ustar])

# Checks for programs.
AC_PROG_CC
dnl AC_PROG_RANLIB

# Checks for libraries.
AC_MSG_NOTICE([*** Checking for threads library and/or flags ***])

acx_pthread_ok=no
AC_SUBST([PTHREAD_CFLAGS])
AC_SUBST([PTHREAD_LIBS])

# We used to check for pthread.h first, but this fails if pthread.h
# requires special compiler flags (e.g. on True64 or Sequent).
# It gets checked for in the link test anyway.

# First of all, check if the user has set any of the PTHREAD_LIBS,
# etcetera environment variables, and if threads linking works using
# them:
if test x"$PTHREAD_LIBS$PTHREAD_CFLAGS" != x; then
	save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        AC_MSG_CHECKING([for pthread_join in LIBS=$PTHREAD_LIBS with CFLAGS=$PTHREAD_CFLAGS])
	AC_TRY_LINK([],
	            [pthread_join],
		    [acx_pthread_ok=yes],
		    [PTHREAD_LIBS=""
                     PTHREAD_CFLAGS=""])
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
fi        

# We must check for the threads library under a number of different
# names; the ordering is very important because some systems
# (e.g. DEC) have both -lpthread and -lpthreads, where one of the
# libraries is broken (non-POSIX).

# Create a list of thread flags to try.  Items starting with a "-" are
# C compiler flags, and other items are library names, except for "none"
# which indicates that we try without any flags at all, and "pthread-config"
# which is a program returning the flags for the Pth emulation library.

acx_pthread_flags="pthreads none -Kthread -kthread lthread -pthread -pthreads -mthreads pthread --thread-safe -mt pthread-config"

# The ordering *is* (sometimes) important.  Some notes on the
# individual items follow:

# pthreads: AIX (must check this before -lpthread)
# none: in case threads are in libc; should be tried before -Kthread and
#       other compiler flags to prevent continual compiler warnings
# -Kthread: Sequent (threads in libc, but -Kthread needed for pthread.h)
# -kthread: FreeBSD kernel threads (preferred to -pthread since SMP-able)
# lthread: LinuxThreads port on FreeBSD (also preferred to -pthread)
# -pthread: Linux/gcc (kernel threads), BSD/gcc (userland threads)
# -pthreads: Solaris/gcc
# -mthreads: Mingw32/gcc, Lynx/gcc
# -mt: Sun Workshop C (may only link SunOS threads [-lthread], but it
#      doesn't hurt to check since this sometimes defines pthreads too;
#      also defines -D_REENTRANT)
# pthread: Linux, etcetera
# --thread-safe: KAI C++
# pthread-config: use pthread-config program (for GNU Pth library)

case "${host_cpu}-${host_os}" in
        *solaris*)

        # On Solaris (at least, for some versions), libc contains stubbed
        # (non-functional) versions of the pthreads routines, so link-based
        # tests will erroneously succeed.  (We need to link with -pthread or
        # -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
        # a function called by this macro, so we could check for that, but
        # who knows whether they'll stub that too in a future libc.)  So,
        # we'll just look for -pthreads and -lpthread first:

        acx_pthread_flags="-pthread -pthreads pthread -mt $acx_pthread_flags"
        ;;
esac

if test x"$acx_pthread_ok" = xno; then
    for flag in $acx_pthread_flags; do
        case $flag in
        none)
            AC_MSG_CHECKING([whether pthreads work without any flags])
	    ;;
	-*)
	    AC_MSG_CHECKING([whether pthreads work with $flag])
            PTHREAD_CFLAGS="$flag"
            ;;
	pthread-config)
	    AC_CHECK_PROG([acx_pthread_config],[pthread-config],[yes],[no])
	    if test x"$acx_pthread_config" = xno; then continue; fi
            PTHREAD_CFLAGS="`pthread-config --cflags`"
	    PTHREAD_LIBS="`pthread-config --ldflags` `pthread-config --libs`"
 	    ;;
	*)    
	    AC_MSG_CHECKING([for the pthreads library -l$flag])
	    PTHREAD_LIBS="-l$flag"
	    ;;
	esac

        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Check for various functions.  We must include pthread.h,
        # since some functions may be macros.  (On the Sequent, we
        # need a special flag -Kthread to make this header compile.)
        # We check for pthread_join because it is in -lpthread on IRIX
        # while pthread_create is in libc.  We check for pthread_attr_init
        # due to DEC craziness with -lpthreads.  We check for
        # pthread_cleanup_push because it is one of the few pthread
        # functions on Solaris that doesn't have a non-functional libc stub.
        # We try pthread_create on general principles.
	AC_TRY_LINK([#include <pthread.h>],
	            [pthread_t th;  pthread_join(th, 0);
                     pthread_attr_init(0); pthread_cleanup_push(0, 0);
                     pthread_create(0,0,0,0); pthread_cleanup_pop(0); ],
                    [acx_pthread_ok=yes])
		    
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

	AC_MSG_RESULT([$acx_pthread_ok])
	if test "x$acx_pthread_ok" = xyes; then
                break;
        fi

        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
    done
fi    

# Various other checks:
if test "x$acx_pthread_ok" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Detect AIX lossage: threads are created detached by default
        # and the JOINABLE attribute has a nonstandard name (UNDETACHED).
        AC_MSG_CHECKING([for joinable pthread attribute])
	AC_TRY_LINK([#include <pthread.h>],
                    [int attr=PTHREAD_CREATE_JOINABLE;],
		    [ok=PTHREAD_CREATE_JOINABLE],
		    [ok=unknown])
        if test x"$ok" = xunknown; then
	    AC_TRY_LINK([#include <pthread.h>],
                        [int attr=PTHREAD_CREATE_UNDETACHED;],
 		        [ok=unknown])
	fi
        if test x"$ok" != xPTHREAD_CREATE_JOINABLE; then
	    AC_DEFINE([PTHREAD_CREATE_JOINABLE], [$ok],
	                [Define to the necessary symbol if this constant uses a non-standard name on your system.])
	fi
	AC_MSG_RESULT([$ok])
        if test x"$ok" = xunknown; then
           AC_MSG_WARN([we do not know how to create joinable pthreads])
	fi

	AC_MSG_CHECKING([if more special flags are required for pthreads])
        flag=no
        case "${host_cpu}-${host_os}" in
                *-aix* | *-freebsd*)     flag="-D_THREAD_SAFE";;
                *solaris* | *-osf* | *-hpux*) flag="-D_REENTRANT";;
        esac
	AC_MSG_RESULT([$flag])
        if test "x$flag" != xno; then
                PTHREAD_CFLAGS="$flag $PTHREAD_CFLAGS"
        fi

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
		
        # More AIX lossage: must compile with cc_r
        AC_CHECK_PROG([PTHREAD_CC],[cc_r],[cc_r],[${CC}])
else
        PTHREAD_CC="$CC"
fi

if test x"$acx_pthread_ok" = xyes; then
	CC="$PTHREAD_CC";
fi

AC_MSG_NOTICE([*** Parsing options ***])

AC_SUBST(SSL_CPPFLAGS)
AC_SUBST(SSL_LDFLAGS)
AC_ARG_WITH([ssl],
 AC_HELP_STRING([--with-ssl=directory],[location of OpenSSL package]),
 [SSL_CPPFLAGS="-I${withval}/include"
  SSL_LDFLAGS="-L${withval}/lib ${LDFLAGS}"
  C_SSL="${withval}"],
 [C_SSL=""])

AC_ARG_WITH([t_rsa],
 AC_HELP_STRING([--with-t_rsa=nnn],
                [Timeout for RSA ephemeral keys generation]),
 [T_RSA_KEYS=${withval}],
 [T_RSA_KEYS=7200])

AC_DEFINE_UNQUOTED([T_RSA_KEYS],[$T_RSA_KEYS],
 [Timeout for RSA ephemeral keys generation])
 
AC_ARG_WITH([dh],
 AC_HELP_STRING([--with-dh=nnn],[DH key length parameter (default: 2048, can set to 1024)]),
 [case ${withval} in
  1024|2048) DH_LEN=2048;;
  *) AC_MSG_FAILURE([invalid value for DH key length])
  esac],
 [DH_LEN=2048])

AC_SUBST(DH_LEN) 
AC_DEFINE_UNQUOTED([DH_LEN],[$DH_LEN],
 [DH key length])

AC_ARG_WITH([maxbuf],
 AC_HELP_STRING([--with-maxbuf=nnn],[Value of the MAXBUF parameter (default: 4096)]),
 [MAXBUF=${withval}],
 [MAXBUF=4096])

AC_DEFINE_UNQUOTED([MAXBUF],[$MAXBUF],
 [Max. buffer size])
 
AC_SUBST(I_OWNER)
AC_ARG_WITH([owner],
 AC_HELP_STRING([--with-owner=name],
                [The account that will own the files installed by Pound]),
 [I_OWNER="${with_owner}"; C_OWNER="${with_owner}"],
 [C_OWNER=""])

AC_SUBST(I_GRP)
AC_ARG_WITH([group],
 AC_HELP_STRING([--with-group=name],
                [The group that will own the files installed by Pound]),
 [I_GRP="${with_group}"; C_GROUP="${with_group}"],
 [C_GROUP=""])

AC_ARG_ENABLE([super], 
 AC_HELP_STRING([--enable-super],
                [enable or disable running with supervisor process (default: enabled)]),
 [if test ${enableval} = "yes"; then
    SUPERVISOR=1
  else
    SUPERVISOR=0
  fi],
 [SUPERVISOR=1])

AC_DEFINE_UNQUOTED([SUPERVISOR],[$SUPERVISOR],
 [Run supervisor process])
 
AC_ARG_ENABLE([pcreposix],
 AC_HELP_STRING([--enable-pcreposix],
         [enable or disable using the pcreposix library (default: enabled if available)]),
 [C_PCREPOSIX=${enableval}],
 [C_PCREPOSIX=yes])

AC_ARG_ENABLE([tcmalloc],
 AC_HELP_STRING([--enable-tcmalloc],
         [enable or disable using the tcmalloc library (default: enabled if available)]),
 [C_TCMALLOC=${enableval}],
 [C_TCMALLOC=yes])

AC_ARG_ENABLE([hoard],
 AC_HELP_STRING([--enable-hoard],
     [enable or disable using the hoard library (default: enabled if available and tcmalloc NOT available)]),
 [C_HOARD=${enableval}],
 [C_HOARD=yes])


AC_MSG_NOTICE([*** Checking for libraries ***])
AC_CHECK_LIB([dl],[dlopen])
AC_CHECK_LIB([socket],[socket],[LIBS="-lsocket -lnsl ${LIBS}"])
AC_CHECK_LIB([resolv],[hstrerror],[LIBS="-lresolv ${LIBS}"])
AC_CHECK_LIB([crypto],[BIO_new],
             [LIBS="-lcrypto ${LIBS}"],
	     [AC_MSG_FAILURE([Missing OpenSSL (-lcrypto) - aborted],[1])])
AC_CHECK_LIB([ssl],[SSL_CTX_new],
             [LIBS="-lssl ${LIBS}"],
	     [AC_MSG_FAILURE([Missing OpenSSL (-lssl) - aborted])])
if test x"$C_PCREPOSIX" = xyes; then
  AC_CHECK_LIB([pcreposix],[regcomp],[],
    [save_LIBS="$LIBS"
     LIBS="-lpcre $LIBS"
     AC_CHECK_LIB([pcreposix],[regexec],
       [LIBS="-lpcreposix -lpcre $save_LIBS"
        AC_DEFINE([HAVE_LIBPCREPOSIX],[1],[])],
       [LIBS="$save_LIBS"])])
fi

if test x"$C_TCMALLOC" = xyes; then
  AC_CHECK_LIB([tcmalloc],[malloc],
    [LIBS="${LIBS} -ltcmalloc"])
elif test x"$C_HOARD" = xyes; then
  AC_CHECK_LIB([hoard],[malloc],
    [LIBS="${LIBS} -lhoard"])
fi    

AC_MSG_CHECKING([for pthread_attr_setstacksize in LIBS=$LIBS with CFLAGS=$CFLAGS"])
AC_TRY_LINK([char pthread_attr_setstacksize ();],
 [return pthread_attr_setstacksize ();],
 [CFLAGS="${CFLAGS} -DNEED_STACK"]);
AC_MSG_RESULT([OK]);

AC_MSG_NOTICE([*** Checking for header files ***])
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h errno.h netdb.h netinet/in.h netinet/tcp.h stdlib.h string.h sys/socket.h sys/un.h sys/time.h unistd.h getopt.h pthread.h sys/types.h sys/poll.h openssl/ssl.h openssl/engine.h time.h pwd.h grp.h signal.h regex.h ctype.h wait.h sys/wait.h sys/stat.h sys/syslog.h syslog.h fcntl.h stdarg.h pcreposix.h pcre/pcreposix.h fnmatch.h])

AC_MSG_NOTICE([*** Checking for additonal information ***])

save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -DSYSLOG_NAMES=1"
AC_MSG_CHECKING([for facilitynames presence])
AC_TRY_LINK([#include <stdio.h>
             #include <syslog.h>
            ],
            [void *attr=(void *)facilitynames;],
	    [ok=yes],
	    [ok=no])
if test x"$ok" = xyes; then
  AC_MSG_RESULT([yes])
  AC_DEFINE([HAVE_FACILITYNAMES],1,
    [The facilitynames array is defined in sys/syslog.h])
else
  AC_MSG_RESULT([missing])
fi  
CFLAGS=$save_CFLAGS

AC_TYPE_SIGNAL
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_HEADER_TIME
AC_TYPE_UNSIGNED_LONG_LONG_INT
AC_TYPE_LONG_LONG_INT
AC_CHECK_TYPE([in_addr_t],[],[CFLAGS="${CFLAGS} -DNEED_INADDRT"],
 [#include <sys/types.h>
  #include <netinet/in.h>
  ])
AC_CHECK_TYPE([in_port_t],[],[CFLAGS="${CFLAGS} -DNEED_INPORTT"],
 [#include <sys/types.h>
  #include <netinet/in.h>
  ])
AC_CHECK_TYPE([time_t],[],[CFLAGS="${CFLAGS} -DNEED_TIMET"],
 [#include <time.h>])

AC_FUNC_FORK
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_STAT
AC_FUNC_STRFTIME

AC_CHECK_FUNCS([getaddrinfo inet_ntop memset regcomp poll socket strcasecmp strchr strdup\
 strerror strncasecmp strspn strtol setsid X509_STORE_set_flags localtime_r gettimeofday])

AC_DEFINE_UNQUOTED([C_SSL], ["$C_SSL"],
 [Location of OpenSSL package])
AC_DEFINE_UNQUOTED([C_OWNER],["$C_OWNER"],
 [Account that owns Pound files])
AC_DEFINE_UNQUOTED([C_GROUP],["$C_GROUP"],
 [Group for the Pound files])

if test -z "${I_OWNER}"
then
    egrep -s '^root' /etc/passwd > /dev/null && I_OWNER=root
    egrep -s '^sys' /etc/passwd > /dev/null && I_OWNER=sys
    egrep -s '^bin' /etc/passwd > /dev/null && I_OWNER=bin
    egrep -s '^daemon' /etc/passwd > /dev/null && I_OWNER=daemon
    egrep -s '^www' /etc/passwd > /dev/null && I_OWNER=www
    egrep -s '^proxy' /etc/passwd > /dev/null && I_OWNER=proxy
fi


if test -z "${I_GRP}"
then
    egrep -s '^root' /etc/group > /dev/null && I_GRP=root
    egrep -s '^sys' /etc/group > /dev/null && I_GRP=sys
    egrep -s '^bin' /etc/group > /dev/null && I_GRP=bin
    egrep -s '^daemon' /etc/group > /dev/null && I_GRP=daemon
    egrep -s '^www' /etc/group > /dev/null && I_GRP=www
    egrep -s '^proxy' /etc/group > /dev/null && I_GRP=proxy
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

