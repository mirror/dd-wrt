#check whether to use and where to place pidfile
unset NOPIDFILE
AC_MSG_CHECKING([whether to use server pidfile])
AC_ARG_ENABLE(pidfile,
[  --disable-pidfile       disable server pidfile creation],
[if test x"$enableval" = xno; then
    NOPIDFILE="Disabled, using --disable-pidfile"
 fi])
if test x"$NOPIDFILE" = x; then
   AC_MSG_RESULT([yes])
   AC_DEFINE(HAVE_ENABLED_PIDFILE, 1, [using pidfile for server])
else
   AC_MSG_RESULT([no])
fi

unset PIDFILEPATH
AC_MSG_CHECKING([for pid file location])
AC_ARG_WITH(pidfile,
[  --with-pidfile=FILE     change location of server pidfile],
[PIDFILEPATH="$withval"],
[#default per-platform pidfile locations
 case $host in
     *-*-aix*)
	 PIDFILEPATH="/etc/${SERVNAME}.pid"
	 ;;
     *)
	 PIDFILEPATH="/var/run/${SERVNAME}.pid"
	 ;;
 esac])
AC_MSG_RESULT($PIDFILEPATH)
AC_DEFINE_UNQUOTED(SOCKD_PIDFILE, "$PIDFILEPATH", [pid file location])

#SOCKD_IOMAX/NEGOTIATEMAX override?
AC_MSG_CHECKING([for SOCKD_IOMAX value])
AC_ARG_WITH(iomax,
[  --with-iomax=NUMBER     change number of clients per io process],
[AC_MSG_RESULT($withval)
 FEAT="$FEAT${FEAT:+ }iomax-$withval"
 CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-DSOCKD_IOMAX=$withval"],
[AC_MSG_RESULT(default)])

AC_MSG_CHECKING([for SOCKD_NEGOTIATEMAX value])
AC_ARG_WITH(negmax,
[  --with-negmax=NUMBER    change number of clients per negotiate process],
[AC_MSG_RESULT($withval)
 FEAT="$FEAT${FEAT:+ }negmax-$withval"
 CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-DSOCKD_NEGOTIATEMAX=$withval"],
[AC_MSG_RESULT(default)])

AC_MSG_CHECKING([for SOCKD_BUFSIZE value])
AC_ARG_WITH(bufsize,
[  --with-bufsize=NUMBER    change size of data buffers],
[AC_MSG_RESULT($withval)
 FEAT="$FEAT${FEAT:+ }bufsize-$withval"
 CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-DSOCKD_BUFSIZE=$withval"],
[AC_MSG_RESULT(default)])

#math library used in server
AC_CHECK_LIB(m, lround,
 [SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }-lm"])

#XXX for backwards compatibility
unset NOLIBWRAP
AC_ARG_WITH(libwrap,
[  --without-libwrap       never use libwrap, even if it is available],
[if test x"$withval" = xno; then
    NOLIBWRAP="Disabled, using --without-libwrap"
 fi])

AC_ARG_ENABLE(libwrap,
[  --disable-libwrap       deprecated, use --without-libwrap],
[AC_MSG_WARN(--disable/enable-libwrap is deprecated, use --without-libwrap)
 if test x"$NOLIBWRAP" != x; then
    AC_MSG_WARN(cannot specify both --without-libwrap and --disable-libwrap)
    exit 1
 fi
 if test x"$enableval" = xno; then
    NOLIBWRAP="Disabled, using --disable-libwrap"
 fi])

if test x"$NOLIBWRAP" != x; then
    AC_MSG_CHECKING([for libwrap])
    AC_MSG_RESULT([disabled])
else
    AC_CHECK_HEADERS(tcpd.h)
    if test x"${ac_cv_header_tcpd_h}" != xyes; then
	AC_MSG_CHECKING([for libwrap])
	AC_MSG_RESULT([no, usable tcpd.h not found])
	NOLIBWRAP="Disabled, tcpd.h missing"
    else
	case $host in
	    *-*-linux-*)
	        #XXX needed on at least RedHat
		AC_SEARCH_LIBS(yp_get_default_domain, nsl)
		;;
	esac
	oLIBS=$LIBS
	AC_MSG_CHECKING([for libwrap])
	#normal library testing does not work for libwrap (will not link
	#without allow/deny_severity)
	#XXX include header in compilation test?
	LIBS="$LIBS -lwrap"
	AC_TRY_LINK([
#include <tcpd.h>
int allow_severity;
int deny_severity;
], [hosts_access(0);],
        [AC_MSG_RESULT(yes)
	 SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }-lwrap"
         AC_DEFINE(HAVE_COND_LIBWRAP, 1, [use tcpwrappers])
         FEAT="$FEAT${FEAT:+ }libwrap"],
        [AC_MSG_RESULT(no)
         NOLIBWRAP="Disabled, -lwrap not found"])
        LIBS="$oLIBS"
    fi
fi

unset NOPAM
AC_CHECK_HEADERS(security/pam_appl.h)
AC_SEARCH_LIBS(pam_start, pam)
AC_MSG_CHECKING([for pam])
AC_ARG_WITH(pam,
[  --without-pam           disable pam support @<:@default=detect@:>@],
[if test x"$withval" = xno; then
    NOPAM="Disabled, using --without-pam"
 fi])

if test x"$NOPAM" != x; then
    AC_MSG_RESULT([disabled])
else
    #look for PAM header and lib
    if test x"${ac_cv_header_security_pam_appl_h}" = xno; then
	NOPAM="Disabled, security/pam_appl.h missing"
	AC_MSG_RESULT([no, usable security/pam_appl.h not found])
    else
        if test x"${ac_cv_search_pam_start}" = xno; then
	    NOPAM="Disabled, pam library not found"
	    AC_MSG_RESULT([no, pam library not found])
	else
	    AC_DEFINE(HAVE_COND_PAM, 1, [PAM support])
	    AC_MSG_RESULT([yes])
	    FEAT="$FEAT${FEAT:+ }pam"
	fi
    fi
fi

AC_MSG_CHECKING([whether libcfail should be enabled])
AC_ARG_ENABLE(libcfail,
[  --enable-libcfail       testing option, enable unreliable libc @<:@default=disabled@:>@])
if test x"${enable_libcfail}" = xyes; then
    AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_LIBCFAIL, 1, [use libcfail])
    have_libcfail=t
    FEAT="$FEAT${FEAT:+ }libcfail"
else
    AC_MSG_RESULT([no])
fi
