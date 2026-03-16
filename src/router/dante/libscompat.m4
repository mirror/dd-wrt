dnl libscompat.m4 - tests related to replacement code in libscompat directory

unset build_libscompat
#if test x"$prerelease" != x; then
#    #build libscompat for increased testing during prereleases, but
#    #only on platforms without complicated library systems
#    case $host in
#	 *-*-linux*)
#	    build_libscompat=t
#	    ;;
#    esac
#fi

AC_MSG_CHECKING([for timer macros])
AC_TRY_LINK([
#include <sys/time.h>],
[struct timeval tv, tv2, tv3;

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    tv2.tv_sec = 0;
    tv2.tv_usec = 0;
    tv3.tv_sec = 0;
    tv3.tv_usec = 0;

    timeradd(&tv, &tv2, &tv3);
    timersub(&tv3, &tv2, &tv);],
[AC_MSG_RESULT(yes)
 AC_DEFINE(HAVE_TIMER_MACROS, 1, [timeradd(), timersub etc. exist in sys/time.h])],
[AC_MSG_RESULT(no)])

AC_MSG_CHECKING([for SIOCGIFHWADDR])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#ifndef SIOCGIFHWADDR
#error "SIOCGIFHWADDR not defined"
#endif
], [
struct ifreq ifr;
unsigned char c;

c = 0;
memcpy(c, ifr.ifr_hwaddr.sa_data, 1);],
 [AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_SIOCGIFHWADDR, 1, [have MAC retrieval interface])],
 [AC_MSG_RESULT(no)
  AC_DEFINE(HAVE_SIOCGIFHWADDR, 0, [missing MAC retrieval interface])])

AC_MSG_CHECKING([for fd_mask])
AC_TRY_COMPILE([
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
], [fd_mask foo;],
 [AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_FDMASK, 1, [have fd_mask definition])],
 [AC_MSG_RESULT(no)
  AC_DEFINE(HAVE_FDMASK, 0, [no fd_mask definition])])

AC_MSG_CHECKING([for CPU_EQUAL])
AC_TRY_LINK([
#define _GNU_SOURCE
#include <sched.h>],
[   cpu_set_t set1, set2;
    if (CPU_EQUAL(&set1, &set2))
       return 0;
],
[AC_MSG_RESULT(yes)
 AC_DEFINE(HAVE_CPU_EQUAL, 1, [CPU_EQUAL exists in sched.h])],
[AC_MSG_RESULT(no)])

#no replacements for these
AC_CHECK_FUNCS(processor_bind)

case $host in
changequote(<<, >>)dnl
#    *-*-freebsd[1-8]\.*)
changequote([, ])dnl
    *-*-freebsd*) #XXX disable on all versions for now
	AC_MSG_WARN([disabling sched_setscheduler code on this platform])
	;;
    *-*-netbsd*) #XXX appears to have similar problem as freebsd
	AC_MSG_WARN([disabling sched_setscheduler code on this platform])
	;;
    *)
	AC_CHECK_FUNCS(sched_setscheduler)
	;;
esac

AC_CHECK_HEADERS(sched.h)

#getpassphrase() is not limited to 9 character passwords on SunOS
AC_CHECK_FUNC(getpassphrase,
[AC_DEFINE(getpass(p), getpassphrase(p), [use getpassphrase])])

AC_MSG_CHECKING([for sched_setaffinity])
AC_TRY_COMPILE([
#define _GNU_SOURCE
#include <sched.h>
],
[   cpu_set_t set1;
    sched_getaffinity(0, 1, &set1);
    sched_setaffinity(0, 1, &set1);
],
 [AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_SCHED_SETAFFINITY, 1, [have sched_setaffinity])],
 [AC_MSG_RESULT(no)])

AC_CHECK_HEADERS(ifaddrs.h)

AC_CHECK_FUNCS(daemon difftime getifaddrs freeifaddrs hstrerror)
AC_CHECK_FUNCS(inet_pton issetugid memmove seteuid setegid)
AC_CHECK_FUNCS(setproctitle strvis vsyslog bzero strlcpy backtrace)
#inet_ntoa - only checked for incorrect behavior

#try to detect gcc bug (irix 64 problem, affects among others inet_ntoa)
AC_MSG_CHECKING([for incorrect inet_ntoa behaviour])
AC_TRY_RUN([
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

int main(void)
{
    struct sockaddr_in addr;
    char *a, *b = "195.195.195.195";
    addr.sin_addr.s_addr = inet_addr(b);
    a = inet_ntoa(addr.sin_addr);
    if (strcmp(a, b) == 0)
	return 1;
    else
	return 0;
}
], [AC_DEFINE(HAVE_BROKEN_INET_NTOA, 1, [platform bug])
    AC_MSG_RESULT(yes)
    ac_cv_func_inet_ntoa=no],
    [AC_MSG_RESULT(no)],
    [dnl assume working when cross-compiling (rare bug)
     AC_MSG_RESULT(assuming no)])

#AC_CHECK_FUNCS will add HAVE_foo define as long as function exists,
#check overselves as we only want define set if function is also working.
ac_cv_func_pselect=no
AC_MSG_CHECKING([for working pselect()])
AC_TRY_RUN([
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
   struct timespec ts = { 0, 0 };

   errno = 0;
   if (pselect(0, NULL, NULL, NULL, &ts, NULL) == 0) {
      if (errno == ENOSYS) {
         fprintf(stderr, "pselect: error: returns 0 but errno set to ENOSYS\n");
         return -1;
      } else {
         fprintf(stderr, "pselect: working as expected: returns 0 and errno not ENOSYS (errno = %d)\n", errno);
         return 0;
      }
   } else {
      perror("pselect");
      return -1;
   }
}], [AC_MSG_RESULT(yes)
     ac_cv_func_pselect=yes
     AC_DEFINE(HAVE_PSELECT, 1, [working pselect() support])],
    [AC_MSG_RESULT(no)],
    [dnl assume no when cross-compiling
     AC_MSG_RESULT(assuming no)])

#AC_CHECK_FUNCS will add HAVE_foo define as long as function exists,
#check overselves as we only want define set if function is also working.
ac_cv_func_sockatmark=no
AC_MSG_CHECKING([for working sockatmark])
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/socket.h>

int
main()
{
    int s;
    int r;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	return 1;
    if ((r = sockatmark(s)) == -1)
	return 1;
    return 0;
}], [AC_MSG_RESULT(yes)
     ac_cv_func_sockatmark=yes
     AC_DEFINE(HAVE_SOCKATMARK, 1, [working sockatmark() support])],
    [AC_MSG_RESULT(no)],
    [dnl assume no when cross-compiling
     AC_MSG_RESULT(assuming no)])

#XXX
#if `uname` != OpenBSD; then
#   ac_cv_func_getifaddrs=no
#   AC_MSG_WARN([notice: using libscompat getifaddrs() function])
#fi

#only compile files that are needed, client
unset LIBSCSRC LIBDSCSRC
CLIENTONLY="issetugid"
SERVERONLY="daemon seteuid sockatmark"
SHAREDFUNCS="getifaddrs hstrerror inet_ntoa inet_pton memmove pselect setproctitle strlcpy strvis vsyslog"
#XXXold?: difftime
for func in $CLIENTONLY $SHAREDFUNCS; do
    var=ac_cv_func_${func}
    if test ! -s "libscompat/${func}.c"; then
	AC_MSG_WARN([error: libscompat file for $func missing])
	exit 1
    fi
    if eval "test x\"\$${var}\" = xno"; then
	LIBSCSRC="${LIBSCSRC}${LIBSCSRC:+ }${func}.lo"
	LIBDSCSRC="${LIBDSCSRC}${LIBDSCSRC:+ }libdsocks_la-${func}.lo"
	COMPATFUNCS="$COMPATFUNCS${COMPATFUNCS:+ }$func"
    fi
done
if test x"${build_libscompat}" != x; then
   unset LIBSCSRC LIBDSCSRC #link directly with libscompat i prerelease
fi
AC_SUBST([LIBSCSRC])
AC_SUBST([LIBDSCSRC])

#server
unset SOCKDCOMPAT
for func in $SERVERONLY $SHAREDFUNCS; do
    var=ac_cv_func_${func}
    if test ! -s "libscompat/${func}.c"; then
	AC_MSG_WARN([error: libscompat file for $func missing])
	exit 1
    fi
    if eval "test x\"\$${var}\" = xno"; then
	SOCKDCOMPAT="${SOCKDCOMPAT}${SOCKDCOMPAT:+ }${func}.${OBJEXT}"
	COMPATFUNCS="$COMPATFUNCS${COMPATFUNCS:+ }$func"
    fi
done
if test x"${build_libscompat}" != x; then
   unset SOCKDCOMPAT #link directly with libscompat i prerelease
   COMPATFUNCS="$CLIENTONLY $SERVERONLY $SHAREDFUNCS"
fi
COMPATFUNCS=`echo $COMPATFUNCS | xargs -n1 | sort | uniq | xargs`
AC_SUBST([SOCKDCOMPAT])

if test x"$LIBSCSRC" != x; then
   LINTSCCOMPATLIB="-lscompat"
fi
AC_SUBST([SCCOMPATLIB])
AC_SUBST([LINTSCCOMPATLIB])

AC_DEFINE_UNQUOTED(DANTE_COMPATFILES, "$COMPATFUNCS", [Compat functions enabled in build])

if test x"${ac_cv_func_bzero}" = xno; then
    AC_DEFINE(bzero(b, len), memset((b), 0, (len)), [bzero replacement])
fi

m4_ifdef([dantebuild], [
#causes problems with packaging, allow test to be turned off
AC_ARG_WITH(glibc-secure,
[  --without-glibc-secure  disable libc_enable_secure check @<:@default=detect@:>@],
[GLIBCSEC=$withval])

if test "${GLIBCSEC}" != no; then
    AC_MSG_CHECKING([for __libc_enable_secure])
    AC_TRY_RUN([
extern int __libc_enable_secure;

int main()
{
    if (__libc_enable_secure == 0)
	return 0;

	return 1;
}],[AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_LIBC_ENABLE_SECURE, 1, [Linux version of issetugid()])],
   [AC_MSG_RESULT([no])],
   [dnl assume no when cross-compiling
    AC_MSG_RESULT([assuming no])])
fi
],
[AC_DEFINE(HAVE_LIBC_ENABLE_SECURE, 0, [not used])])
if test x"$GLIBCSEC" = xno; then
   AC_DEFINE(HAVE_LIBC_ENABLE_SECURE_DISABLED, 1, [glibc variable disable])
fi

AC_MSG_CHECKING([for FIONREAD socket support])
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>

int
main(void)
{
	int bufdat, s;

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
	   perror("socket");
	   return -1;
	}

	if (ioctl(s, FIONREAD, &bufdat) == -1) {
	   perror("ioctl");
	   return -1;
	}

	return 0;
}], [AC_MSG_RESULT(yes)
     AC_DEFINE(HAVE_RECVBUF_IOCTL, 1, [readable buffer data])
     AC_DEFINE(RECVBUF_IOCTLVAL, FIONREAD, [readable buffer ioctl])],
    [AC_MSG_RESULT(no)],
    [dnl assume no when cross-compiling
     AC_MSG_RESULT(assuming no)])

AC_MSG_CHECKING([for FIONWRITE socket support])
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>

int
main(void)
{
	int bufdat, s;

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
	   perror("socket");
	   return -1;
	}

	if (ioctl(s, FIONWRITE, &bufdat) == -1) {
	   perror("ioctl");
	   return -1;
	}

	return 0;
}], [AC_MSG_RESULT(yes)
     AC_DEFINE(HAVE_SENDBUF_IOCTL, 1, [send buffer data])
     AC_DEFINE(SENDBUF_IOCTLVAL, FIONWRITE, [send buffer ioctl])],
    [AC_MSG_RESULT(no)

     AC_MSG_CHECKING([for TIOCOUTQ socket support])
     AC_TRY_RUN([
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>

int
main(void)
{
	int bufdat, s;

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
	   perror("socket");
	   return -1;
	}

	if (ioctl(s, TIOCOUTQ, &bufdat) == -1) {
	   perror("ioctl");
	   return -1;
	}

	return 0;
}], [AC_MSG_RESULT(yes)
     AC_DEFINE(HAVE_SENDBUF_IOCTL, TIOCOUTQ, [send buffer data])
     AC_DEFINE(SENDBUF_IOCTLVAL, TIOCOUTQ, [send buffer ioctl])],
    [AC_MSG_RESULT(no)])],
    [dnl assume no when cross-compiling
     AC_MSG_RESULT(no)])

#set to a high value for systems where this is possible
case $host in
    *-*-linux-* | *-*-aix*)
	#nada
	;;

    *)
	CPPFLAGS="$CPPFLAGS${CPPFLAGS:+ }-DFD_SETSIZE=65536"
	;;
esac

#on most platforms FD_SETSIZE can be ignored
case $host in
    *-*-solaris*)
	AC_DEFINE(FD_SETSIZE_LIMITS_SELECT, 1, [limit to FD_SETSIZE])
	;;
    *)
	AC_DEFINE(FD_SETSIZE_LIMITS_SELECT, 0, [ignore FD_SETSIZE])
	;;
esac

#lack of unified buffers result in less optimal shmem.c performance
L_UNIBUF()

#XXX should be in osdep.m4
AC_SEARCH_LIBS(clock_gettime, rt)
AC_MSG_CHECKING([for CLOCK_MONOTONIC clock_gettime() support])
AC_TRY_RUN([
#include <sys/time.h>
#include <time.h>
#include <stdio.h>

int
main(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
	   perror("clock_gettime");
	   return -1;
	}

	return 0;
}], [AC_MSG_RESULT(yes)
     AC_DEFINE(HAVE_CLOCK_GETTIME_MONOTONIC, 1, [monotonic clock_gettime()])],
    [AC_MSG_RESULT(no)],
    [dnl assume no when cross-compiling
     AC_MSG_RESULT(assuming no)])
