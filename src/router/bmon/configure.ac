#
# configure.in      Configure Script
#
# Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

AC_INIT(bmon, 2.1.0, tgraf@suug.ch)
AC_CONFIG_HEADER(include/bmon/defs.h)

AC_CANONICAL_SYSTEM

save_CFLAGS="${CFLAGS}"
save_LDFLAGS="${LDFLAGS}"
save_CPPFLAGS="${CPPFLAGS}"

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_INSTALL

AC_C_CONST
AC_C_INLINE

AC_HEADER_TIME
AC_HEADER_DIRENT

AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_TYPE_PID_T

AC_FUNC_FORK

AC_CHECK_HEADERS(getopt.h ncurses/ncurses.h ncurses.h curses.h)
AC_CHECK_HEADERS(dirent.h sys/utsname.h sys/sockio.h netinet6/in6.h)
AC_CHECK_HEADERS(rrd.h fcntl.h netdb.h netinet/in.h sysctl/ioctl.h)
AC_CHECK_HEADERS(sys/param.h sys/socket.h)

AC_CHECK_TYPES(suseconds_t)

AC_CHECK_FUNCS(atexit gettimeofday memset pow socket strcasecmp)
AC_CHECK_FUNCS(strchr strdup strerror strncasecmp strstr strtol)
AC_CHECK_FUNCS(uname)

#
# Check if -lsocket is needed
#
AC_CHECK_FUNC(connect,,[
	AC_CHECK_LIB(socket,connect,LIB_INET="${LIB_INET} -lsocket")
])

#
# Check if -lnsl is needed
#
AC_CHECK_FUNC(gethostbyname,,[
	AC_CHECK_LIB(nsl,gethostbyname,LIB_INET="${LIB_INET} -lnsl")
])

#
# Check if -lresolv is needed
#
AC_CHECK_FUNC(inet_aton,,[
	AC_CHECK_LIB(resolv, inet_aton,LIB_INET="${LIB_INET} -lresolv")
])

AC_CHECK_FUNC(fcloseall,[
	AC_DEFINE_UNQUOTED(HAVE_FCLOSEALL, "1", [have fcloseall])
])


#####################################################################
##
## libcurses check
##
#####################################################################
CURSES="No "
AC_CHECK_LIB(ncurses, initscr, [
  AC_DEFINE_UNQUOTED(HAVE_NCURSES, "1", [have ncurses])
  LCURSES="ncurses"
  CURSES="Yes"
],[
	AC_CHECK_LIB(curses,initscr, [
		AC_DEFINE_UNQUOTED(HAVE_CURSES, "1", [have curses])
		LCURSES="curses"
		  CURSES="Yes"
	],[
		echo
		echo "*** Warning: Building bmon w/o curses support ***"
		echo
	])
])

LIBCURSES="-l$LCURSES"

AC_CHECK_LIB($LCURSES, redrawwin, [
	AC_DEFINE_UNQUOTED(HAVE_REDRAWWIN, "1", [have redrawwin])
])

AC_CHECK_LIB($LCURSES, use_default_colors, [
	  AC_DEFINE_UNQUOTED(HAVE_USE_DEFAULT_COLORS, "1", [have udc])
])

#####################################################################
##
## librrd check
##
#####################################################################
RRD="No "
AC_CHECK_LIB(rrd, rrd_create, [
	AC_DEFINE_UNQUOTED(HAVE_RRD, "1", [have rrd])
	LIBRRD="-lrrd"
	RRD="Yes"
],[
	echo
	echo "*** Warning: Building bmon w/o rrd support ***"
	echo
])

#####################################################################
##
## libdbi check
##
#####################################################################
DBI="No "
AC_CHECK_LIB(dbi, dbi_initialize, [
	AC_DEFINE_UNQUOTED(HAVE_DBI, "1", [have dbi])
	LIBDBI="-ldbi"
	DBI="Yes"
],[
	echo
	echo "*** Warning: Building bmon w/o DBI support ***"
	echo
])

#####################################################################
##
## libasound check
##
#####################################################################
ASOUND="No "
AC_CHECK_LIB(asound, snd_seq_open, [
	AC_DEFINE_UNQUOTED(HAVE_ASOUND, "1", [have asound])
	LIBASOUND="-lasound"
	ASOUND="Yes"
],[
	echo
	echo "*** Warning: Building bmon w/o libasound support ***"
	echo
])

#####################################################################
##
## libkstat check
##
#####################################################################
KSTAT="No "
AC_CHECK_LIB(kstat, kstat_open, [
	AC_DEFINE_UNQUOTED(HAVE_KSTAT, "1", [have kstat])
	LIBKSTAT="-lkstat"
	KSTAT="Yes"
],[
	case ${target_os} in
	    *solaris*)
		echo
		echo "*** Warning: Building bmon on SunOS w/o libkstat ***"
		echo
	    ;;
	esac
])

#####################################################################
##
## libnl check
##
#####################################################################
NL="No "
AC_CHECK_LIB(nl, nl_connect, [
	AC_DEFINE_UNQUOTED(HAVE_NL, "1", [have libnl])
	LIBNL="-lnl"
	NL="Yes"
],[
	case ${target_os} in
	    *linux*)
		echo
		echo "*** Warning: Building bmon on Linuxx w/o libnl ***"
		echo
	    ;;
	esac
])

#####################################################################
##
## libm check
##
#####################################################################
M="No "
AC_CHECK_LIB(m, pow, [
	LIBM="-lm"
	M="Yes"
],[
	echo
	echo "*** Error: libm required ***"
	echo
	exit
])

#####################################################################
##
## interface counter overflow workaround
##
#####################################################################
AC_ARG_ENABLE(cnt-workaround,
[  --disable-cnt-workaround Disables interface counter overflow workaround],[
	if test x$enableval = xno; then
		AC_DEFINE_UNQUOTED(DISABLE_OVERFLOW_WORKAROUND,"1",[no overflow workaround])
	fi
])

#####################################################################
##
## librrd
##
#####################################################################
AC_ARG_ENABLE(rrd,
[  --disable-rrd           Disables rrd output],[
	if test x$enableval = xno; then
		RRD="No "
	fi
])

#####################################################################
##
## libasound
##
#####################################################################
AC_ARG_ENABLE(asound,
[  --disable-asound        Disables asound output],[
	if test x$enableval = xno; then
		ASOUND="No "
	fi
])

#####################################################################
##
## libdbi
##
#####################################################################
AC_ARG_ENABLE(dbi,
[  --disable-dbi           Disables DBI output],[
	if test x$enableval = xno; then
		DBI="No "
	fi
])

#####################################################################
##
## curses
##
#####################################################################
AC_ARG_ENABLE(curses,
[  --disable-curses        Disables curses output],[
	if test x$enableval = xno; then
		CURSES="No "
	fi
])

#####################################################################
##
## debug check
##
#####################################################################
DEBUG=0
AC_ARG_ENABLE(debug,
[  --enable-debug          Enable debug mode (default disabled)],[
	if test x$enableval = xyes; then
		DEBUG=1;
	fi
])

#####################################################################
##
## static check
##
#####################################################################
STATIC=0
AC_ARG_ENABLE(static,
[  --enable-static         Enable static linking (default disabled)],[
	if test x$enableval = xyes; then
		STATIC=1;
	fi
])

#####################################################################
##
## target os eval
##
#####################################################################
case ${target_os} in
    linux*)
	AC_DEFINE_UNQUOTED(SYS_LINUX, "1", [operating system])
    ;;

    *solaris*)
	AC_DEFINE_UNQUOTED(SYS_SUNOS, "1", [operating system])
    ;;

    *bsd*)
	AC_DEFINE_UNQUOTED(SYS_BSD, "1", [operating system])
    ;;

    *darwin*)
	AC_DEFINE_UNQUOTED(SYS_BSD, "1", [operating system])
    ;;

    *)
	AC_DEFINE_UNQUOTED(SYS_OTHER, "1", [operating system])
    ;;
esac

#####################################################################
##
## compile decisions
##
#####################################################################
COMPILE_BMON="Yes "
BMON_LIB="$LIB_INET $LIBM"
if test x$CURSES = xYes; then
	BMON_LIB="$BMON_LIB $LIBCURSES"
fi;

if test x$NL = xYes; then
	BMON_LIB="$BMON_LIB $LIBNL"
fi;

if test x$KSTAT = xYes; then
	BMON_LIB="$BMON_LIB $LIBKSTAT"
fi;

if test x$RRD = xYes; then
	BMON_LIB="$BMON_LIB $LIBRRD"
fi;

if test x$ASOUND = xYes; then
	BMON_LIB="$BMON_LIB $LIBASOUND"
fi;

if test x$DBI = xYes; then
	BMON_LIB="$BMON_LIB $LIBDBI"
fi;
	

#####################################################################
##
## export variables
##
#####################################################################
AC_SUBST(COMPILE_BMON)
AC_SUBST(BMON_LIB)

AC_SUBST(DEBUG)
AC_SUBST(STATIC)
AC_SUBST(SYS)
AC_SUBST(ARCH)

AC_SUBST(CURSES)
AC_SUBST(RRD)
AC_SUBST(ASOUND)
AC_SUBST(DBI)

AC_OUTPUT([Makefile.opts])

#####################################################################
##
## status report
##
#####################################################################
echo "
----------------------------------------------------------------------
SUMMARY:

OS: $target_os

Included in Compilation:
  bmon:   $COMPILE_BMON $BMON_LIB

Dependencies:
bmon:
  libm             $M       (required)
  lib(n)curses     $CURSES       (suggested)
  librrd           $RRD       (suggested)
  libdbi           $DBI       (suggested)
  libasound        $ASOUND       (toy)"
case ${target_os} in
  *linux*)
   echo "  libnl            $NL       (suggested)"
   ;;
esac
if test x$target_os = xsolaris; then
   echo "  libkstat         $KSTAT     (required on SunOS)"
fi
echo "
----------------------------------------------------------------------

WARNING:
   Please do not install the bmon executable as SUID root. This
   software package was written with portability and configurability
   in mind and will never fulfill the requirements to be installed
   as SUID root executable.

   If you do please be aware that every user will be able to run
   arbitary commands as root via the key bind interface or by
   providing a malicious netstat binary.

   In fact there is no reason to do so, all supported input modules
   will work fine without superuser privileges. There is probably a
   good reason if your operating system requires special privileges
   which is that a normal user is not supposed to read the statistics
   which should not be worked around using the SUID bit.

   Thanks.
"
