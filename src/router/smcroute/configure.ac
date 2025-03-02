#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([SMCRoute], [2.5.7], [https://github.com/troglobit/smcroute/issues],
	 [smcroute], [https://troglobit.com/smcroute.html])
AC_CONFIG_AUX_DIR(aux)
AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/smcrouted.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile man/Makefile src/Makefile test/Makefile smcroute.service])

# Older versions of autoconf (<2.58) do not have AC_CONFIG_MACRO_DIR()
m4_include([m4/misc.m4])
m4_include([m4/mroute.m4])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_INSTALL

# The pidfile() code needs asprintf(), which relies on -D_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_CHOWN
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit clock_gettime dup2 memset select setenv socket strchr \
	strdup strerror strncasecmp strrchr asprintf])

# Check for usually missing API's
AC_REPLACE_FUNCS([strlcpy strlcat tempfile utimensat])
AC_CONFIG_LIBOBJ_DIR([lib])

# Check for sun_len in struct sockaddr_un and sin_len in sockaddr_in
# These are used on *BSD UNIX and don't exist on Linux
AC_CHECK_SUN_LEN()
AC_CHECK_SIN_LEN()

# Check user options
AC_ARG_ENABLE([mrdisc],
	AS_HELP_STRING([--enable-mrdisc], [enable IPv4 multicast router discovery]))
AC_ARG_ENABLE(test,
        [AS_HELP_STRING([--enable-test], [enable tests, requries unshare, tshark, etc.])],
        [ac_enable_test="$enableval"],
	[ac_enable_test="no"])
AC_ARG_ENABLE([ipv6],
	AS_HELP_STRING([--disable-ipv6], [disable IPv6 support]))
AC_ARG_WITH([libcap],
	AS_HELP_STRING([--without-libcap], [disable libcap, -p USER:GROUP drop-privs support]),,
	[with_libcap=auto])
AC_ARG_WITH([systemd],
	[AS_HELP_STRING([--with-systemd=DIR], [Directory for systemd service files])],,
	[with_systemd=auto])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h glob.h ifaddrs.h limits.h linux/sockios.h \
	net/if.h netinet/in.h netinet/in_var.h net/route.h paths.h stddef.h     \
	sys/capability.h sys/ioctl.h sys/param.h sys/prctl.h sys/socket.h       \
	sys/stat.h sys/time.h sys/types.h syslog.h termios.h unistd.h], [], [],[
	#ifdef HAVE_SYS_SOCKET_H
	# include <sys/socket.h>
	#endif
	#ifdef HAVE_NET_IF_H
	# include <net/if.h>
	#endif
	#ifdef HAVE_NETINET_IN_H
	# include <netinet/in.h>
	#endif
])

# Build w/ mrdisc support?
AS_IF([test "x$enable_mrdisc" = "xyes"],
    AC_DEFINE([ENABLE_MRDISC], 1, [Enable IPv4 multicast router discovery protocol]),
    enable_mrdisc=no)
AM_CONDITIONAL([USE_MRDISC], [test "x$enable_mrdisc" = "xyes"])

# Required to check for libsystemd-dev
PKG_PROG_PKG_CONFIG

# Check where to install the systemd .service file
AS_IF([test "x$PKG_CONFIG" = "x"], [
	with_systemd=no
	AC_MSG_WARN([Cannot find pkg-config tool, disabling systemd check.])])

with_libsystemd=no
AS_IF([test "x$with_systemd" = "xyes" -o "x$with_systemd" = "xauto"], [
	def_systemd=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)
	AS_IF([test "x$def_systemd" = "x"], [
        	AS_IF([test "x$with_systemd" = "xyes"],[
	    		AC_MSG_ERROR([systemd support requested but pkg-config unable to query systemd package])])
		with_systemd=no], [with_systemd="$def_systemd"])])

AS_IF([test "x$with_systemd" != "xno"], [
	PKG_CHECK_MODULES([libsystemd], [libsystemd], [with_libsystemd=yes], [true])
	AC_SUBST([systemddir], [$with_systemd])])
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemd" != "xno"])

AS_IF([test "x$with_libsystemd" != "xno"], [
	AC_DEFINE([HAVE_LIBSYSTEMD], [1], [Define to 1 if you have libsystemd-dev])
	AC_SUBST([DAEMON_TYPE], "notify")], [
	AC_SUBST([DAEMON_TYPE], "simple")])
AM_CONDITIONAL([HAVE_LIBSYSTEMD], [test "x$with_libsystemd" != "xno"])

# Check if we need -lpthread (building statically) and -lrt (older GLIBC)
# Unset cached values when retrying with -lpthread and reset LIBS for each API
need_librt=no
need_pthread=no
old_LIBS=$LIBS
AC_SEARCH_LIBS([clock_gettime], [rt], need_librt=yes)
LIBS=$old_LIBS
AC_SEARCH_LIBS([timer_create],  [rt], need_librt=yes, [
	unset ac_cv_search_timer_create
	AC_SEARCH_LIBS([timer_create],  [rt], need_pthread=yes,,[-lpthread])])
LIBS=$old_LIBS
AC_SEARCH_LIBS([timer_settime], [rt], need_librt=yes, [
	unset ac_cv_search_timer_settime
	AC_SEARCH_LIBS([timer_settime], [rt], need_pthread=yes,,[-lpthread])])
# Check for libcap to not trigger false positives on FreeBSD et al
LIBS=$old_LIBS
AC_SEARCH_LIBS([cap_set_flag], [cap],, ac_cv_header_sys_capability_h=no)
LIBS=$old_LIBS

# Check for RFC 3678-style struct group_req (Linux, FreeBSD, Solaris, macOS, AIX)
AC_CHECK_MEMBER([struct group_req.gr_interface],
	AC_DEFINE([HAVE_STRUCT_GROUP_REQ], [1], [Define to 1 if you have RFC 3678 struct group_req]),
	[], [[#include <netinet/in.h>]])

# Check for Linux-style extension struct ip_mreqn (Linux, FreeBSD)
AC_CHECK_MEMBER([struct ip_mreqn.imr_ifindex],
	AC_DEFINE([HAVE_STRUCT_IP_MREQN], [1], [Define to 1 if you have a Linux-style struct ip_mreqn]),
	[], [[#include <netinet/in.h>]])

# Check for IPv4 support
AC_CHECK_MROUTE()

# If IPv6 is enabled we must probe the system some more
AS_IF([test "x$enable_ipv6" != "xno"],
        AC_CHECK_MROUTE6())

# Only enable support for dropping root privileges if auto/yes && header exists
AS_IF([test "x$with_libcap" != "xno" -a "x$ac_cv_header_sys_capability_h" = "xyes"], [
	with_libcap=yes
	AC_DEFINE([ENABLE_LIBCAP], [], [Define to enable support for libcap.])])
AM_CONDITIONAL(USE_LIBCAP, [test "x$with_libcap" != "xno" -a "x$ac_cv_header_sys_capability_h" = "xyes"])

AM_CONDITIONAL([ENABLE_TEST], [test "x$ac_enable_test" != "xno"])

# Mac OS does not (yet) support SOCK_CLOEXEC
AC_CACHE_CHECK([for SOCK_CLOEXEC support], [ac_cv_sock_cloexec],
	[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/socket.h>

int main()
{
    return socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, 0) == -1;
}]])],[ac_cv_sock_cloexec=yes],[ac_cv_sock_cloexec=no],[ac_cv_sock_cloexec=no])])

AS_IF([test "$ac_cv_sock_cloexec" = "yes" ],
   AC_DEFINE([HAVE_SOCK_CLOEXEC], 1, [Define if the SOCK_CLOEXEC flag is supported]))

AS_IF([test "$need_librt" != "no"],   LIB_RT=-lrt)
AC_SUBST([LIB_RT])

AS_IF([test "$need_pthread" != "no"], LIB_PTHREAD=-lpthread)
AC_SUBST([LIB_PTHREAD])

# Expand $sbindir early, into $SBINDIR, for systemd unit file
# NOTE: This does *not* take prefix/exec_prefix override at "make
#       install" into account, unfortunately.
test "x$prefix" = xNONE && prefix=$ac_default_prefix
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
SYSCONFDIR=`eval echo $sysconfdir`
SYSCONFDIR=`eval echo $SYSCONFDIR`
AC_SUBST(SYSCONFDIR)
SBINDIR=`eval echo $sbindir`
SBINDIR=`eval echo $SBINDIR`
AC_SUBST(SBINDIR)
DOCDIR=`eval echo $docdir`
DOCDIR=`eval echo $DOCDIR`
AC_SUBST(DOCDIR)

AS_IF([test "x$ac_cv_header_sys_capability_h" = "xno"], [
	AS_IF([test "x$with_libcap" = "xyes"], [
		dnl configure: error: ...
		AC_MSG_ERROR(
		  [Cannot find libcap or its headers, install libcap-dev first.]
		  [CentOS/RHEL 6: libcap is broken, recommended to disable it.]
		  [Use  --without-libcap to disable this feature.])
	])
	AS_IF([test "x$with_libcap" = "xauto"], [
		dnl configure: WARNING: ...
		AC_MSG_WARN(
		    [As a safety measure, SMCRoute use libcap to drop root privs]
		    [after startup.  Install libcap and headers from libcap-dev,]
		    [or similar, to enable this feature.])
		AC_MSG_NOTICE([Use  --without-libcap to disable this message.])
	])
	with_libcap=no])

# Workaround for as-of-yet unreleased runstatedir support, planned for
# autoconf 2.70, which some major distros have backported.
AS_IF([test -z "$runstatedir"], runstatedir="$localstatedir/run")
AC_SUBST(runstatedir)

AC_OUTPUT

# Expand directories for configuration summary, unexpanded defaults:
# sysconfdir  => ${prefix}/etc
# runstatedir => ${localstatedir}/run
SYSCONFDIR=`eval echo $sysconfdir`
RUNSTATEDIR=`eval echo $runstatedir`
RUNSTATEDIR=`eval echo $RUNSTATEDIR`

cat <<EOF

------------------ Summary ------------------
 $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix.........: $prefix
  Sysconfdir.....: $SYSCONFDIR
  Runstatedir....: $RUNSTATEDIR
  C Compiler.....: $CC $CFLAGS $CPPFLAGS $LDFLAGS $LIBS

 Optional features:
  IPv6...........: $enable_ipv6
  MRDISC RFC4286.: $enable_mrdisc
  libcap.........: $with_libcap
  systemd........: $with_systemd
  libsystemd.....: $with_libsystemd
  Unit tests.....: $ac_enable_test

------------- Compiler version --------------
$($CC --version || true)
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
