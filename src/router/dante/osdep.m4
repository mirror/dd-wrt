case $host in
    *-*-darwin*)
	AC_DEFINE(HAVE_DARWIN, 1, [enable darwin/osx workarounds])
	;;

    *-*-openbsd*)
	AC_DEFINE(HAVE_OPENBSD_BUGS, 1, [bug workaround])
	;;

    *-*-solaris*)
	AC_DEFINE(HAVE_SENDMSG_DEADLOCK, 1, [bug workaround])
	AC_DEFINE(HAVE_SOLARIS_BUGS, 1, [bug workaround])
	AC_DEFINE(SPT_TYPE, SPT_REUSEARGV, [setproctitle replacement type])
	;;

    *-*-linux-*)
	AC_DEFINE(HAVE_LINUX_BUGS, 1, [bug workaround])
	AC_DEFINE(SPT_TYPE, SPT_REUSEARGV, [setproctitle replacement type])
	;;

    *-*-aix*)
	AC_DEFINE(HAVE_SYSTEM_XMSG_MAGIC, 1, [platform workaround])
	;;
esac

dnl Checking variable sizes
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)

dnl Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h malloc.h paths.h strings.h syslog.h)
AC_CHECK_HEADERS(unistd.h crypt.h stddef.h sys/file.h sys/ioctl.h sys/time.h)
AC_CHECK_HEADERS(shadow.h ifaddrs.h sys/sem.h netinet/in.h rpc/rpc.h)
AC_CHECK_HEADERS(sys/ipc.h arpa/nameser.h net/if_dl.h execinfo.h sys/pstat.h)
AC_CHECK_HEADERS(sys/shm.h valgrind/valgrind.h netinet/tcp_fsm.h)

#some header dependencies for netinet/ip.h, use compilation test
AC_MSG_CHECKING([for netinet/ip.h])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
], [], [AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_NETINET_IP_H, 1, [netinet/ip.h header found])],
   [AC_MSG_RESULT(no)])

case $host in
    #XXX only check for priv.h support on Solaris for now
    *-*-solaris*)
	AC_CHECK_HEADERS(priv.h,
	    [AC_DEFINE(HAVE_PRIVILEGES, 1, [Some privilege type supported])
	     AC_DEFINE(HAVE_SOLARIS_PRIVS, 1, [Solaris priv.h support])])
	;;
esac

AC_CHECK_HEADERS([netinet/ip_var.h], [], [], [
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
])

AC_CHECK_HEADERS([resolv.h], [], [], [
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
])

AC_CHECK_HEADER(sys/sockio.h,
[AC_DEFINE(HAVE_SYS_SOCKIO_H, 1, [sys/sockio.h exists])
 have_sys_sockio_h=t])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_MSG_CHECKING([whether <sys/types.h> defines const])
AC_EGREP_CPP(yes, [
#include <sys/types.h>
#ifdef const
yes
#endif
], [AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)
    check_const="yes"])
if test x$check_const = xyes; then
    AC_C_CONST
fi

#looks for Linux + systems with RTAX_GATEWAY defined in net/route.h
AC_MSG_CHECKING([for supported routing socket communication])
AH_TEMPLATE([HAVE_ROUTE_SOURCE], [routing socket communication supported])
unset no_routeinfo
AC_EGREP_CPP(yes, [
#include <net/route.h>
#ifdef linux
yes
#endif /* linux */
], [AC_DEFINE(HAVE_ROUTE_SOURCE, 1)
    AC_DEFINE(HAVE_ROUTEINFO_LINUX, 1, [Linux type routing socket])
    AC_MSG_RESULT(yes)],
   [AC_EGREP_CPP(yes, [
#include <net/route.h>
#ifdef RTA_GATEWAY
yes
#endif /* RTA_GATEWAY */
],  [AC_DEFINE(HAVE_ROUTE_SOURCE, 1)
     AC_DEFINE(HAVE_ROUTEINFO_BSD, 1, [BSD type routing socket])
     AC_MSG_RESULT(yes)],
     [AC_MSG_RESULT([no, might result in reduced functionality])
      no_routeinfo=t])])

#XXXsys/socket.h?
AC_MSG_CHECKING([for struct ip_opts in <netinet/in.h>])
AC_EGREP_CPP([struct.*ipoption], [
#include <netinet/ip_var.h>
], [AC_DEFINE(HAVE_STRUCT_IPOPTS, 1, [ip_opts defined in netinet/in.h])
    AC_MSG_RESULT(yes)],
   AC_MSG_RESULT(no))

AC_MSG_CHECKING([for struct tcp_info in <netinet/tcp.h>])
AC_EGREP_CPP([struct.*tcp_info], [
#include <netinet/tcp.h>
], [AC_DEFINE(HAVE_TCP_INFO, 1, [tcp_info struct found in netinet/tcp.h])
    AC_MSG_RESULT(yes)

    # check which values are found in the tcp_info struct
    # (limitations in autoconf makes it necessary to list each name explicitly
    #  and not use a list stored in a variable)

    #values not present on some older linux versions
    L_TCPINFOCHECK(tcpi_rcv_rtt)
    L_TCPINFOCHECK(tcpi_rcv_space)
    L_TCPINFOCHECK(tcpi_total_retrans)

    #values not present on freebsd
    L_TCPINFOCHECK(tcpi_advmss)
    L_TCPINFOCHECK(tcpi_ato)
    L_TCPINFOCHECK(tcpi_backoff)
    L_TCPINFOCHECK(tcpi_ca_state)
    L_TCPINFOCHECK(tcpi_fackets)
    L_TCPINFOCHECK(tcpi_last_ack_recv)
    L_TCPINFOCHECK(tcpi_last_ack_sent)
    L_TCPINFOCHECK(tcpi_last_data_recv)
    L_TCPINFOCHECK(tcpi_last_data_sent)
    L_TCPINFOCHECK(tcpi_lost)
    L_TCPINFOCHECK(tcpi_pmtu)
    L_TCPINFOCHECK(tcpi_probes)
    L_TCPINFOCHECK(tcpi_rcv_rtt)
    L_TCPINFOCHECK(tcpi_rcv_ssthresh)
    L_TCPINFOCHECK(tcpi_reordering)
    L_TCPINFOCHECK(tcpi_retrans)
    L_TCPINFOCHECK(tcpi_retransmits)
    L_TCPINFOCHECK(tcpi_sacked)
    L_TCPINFOCHECK(tcpi_unacked)],
   [AC_MSG_RESULT(no)])

AC_MSG_CHECKING([whether <sys/types.h> defines inline])
AC_EGREP_CPP(yes, [
#include <sys/types.h>
#ifdef inline
yes
#endif
], [AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)
    check_inline="yes"])
if test x"${check_inline}" = xyes; then
    AC_C_INLINE
fi

AC_TYPE_UID_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

AC_SYS_LARGEFILE

AC_MSG_CHECKING([for in6_addr])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
], [
struct in6_addr sin6;
], [AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_IN6_ADDR, 1, [in6_addr defined])],
   [AC_MSG_RESULT(no)])

AC_MSG_CHECKING([to see if openlog accepts LOG_PERROR])
AC_EGREP_CPP(yes, [
#include <syslog.h>
#ifdef LOG_PERROR
yes
#endif
], [AC_DEFINE(HAVE_OPENLOG_LOG_PERROR, 1, [openlog supports LOG_PERROR])
    AC_MSG_RESULT(yes)],
    AC_MSG_RESULT(no))

AC_MSG_CHECKING([to see if stdlib.h defines EXIT_FAILURE])
AC_EGREP_CPP(yes, [
#include <stdlib.h>
#ifdef EXIT_FAILURE
yes
#endif
], [AC_MSG_RESULT(yes)],
   [AC_DEFINE(NEED_EXIT_FAILURE, 1, [EXIT_FAILURE not defined in stdlib.h])
    AC_MSG_RESULT(no)])

#XXX actually checks if AF_UNIX should be used instead of AF_LOCAL
AC_MSG_CHECKING([whether <sys/socket.h> uses AF_UNIX])
AC_EGREP_CPP(yes, [
#include <sys/types.h>
#include <sys/socket.h>
#ifdef AF_LOCAL
#else
#ifdef AF_UNIX
yes
#endif
#endif
], [AC_DEFINE(NEED_AF_LOCAL, 1, [need AF_LOCAL definition])
    AC_MSG_RESULT(yes)],
    AC_MSG_RESULT(no))

AC_MSG_CHECKING([for SIGINFO])
AC_EGREP_CPP(yes, [
#include <signal.h>
#ifdef SIGINFO
yes
#endif
], [AC_DEFINE(HAVE_SIGNAL_SIGINFO, 1, [signal.h defined SIGINFO])
    AC_MSG_RESULT(yes)],
    AC_MSG_RESULT(no))

AC_MSG_CHECKING([to see if MSG_WAITALL exists])
AC_EGREP_CPP(yes, [
#include <sys/socket.h>
#ifdef MSG_WAITALL
yes
#endif
], [AC_DEFINE(HAVE_MSG_WAITALL, 1, [sys/socket.h defines MSG_WAITALL])
    AC_MSG_RESULT(yes)],
    AC_MSG_RESULT(no))

AC_MSG_CHECKING([whether realloc with a NULL pointer calls malloc])
AC_TRY_RUN([
#include <stdlib.h>
#ifndef NULL
#define NULL (char *)0
#endif

int main()
{
	/* will assume this test doesn\'t fail because of lack of memory */
	if (realloc(NULL, 1) == NULL)
		return 1;
	else
		return 0;
}], [AC_MSG_RESULT(yes)],
    [AC_DEFINE(HAVE_NOMALLOC_REALLOC, 1, [realloc never calls malloc])
     AC_MSG_RESULT(no)],
    [dnl assume malloc is not called when cross-compiling
     AC_DEFINE(HAVE_NOMALLOC_REALLOC, 1, [realloc never calls malloc])
     AC_MSG_RESULT(no)])

# AC_MSG_CHECKING([whether free can be called with NULL])
# AC_TRY_RUN([
# #include <stdlib.h>
# #ifndef NULL
# #define NULL (char *)0
# #endif

# int main()
# {
#	/* will assume core dump/seg fault if it doesn\'t work */
#	free(NULL);
#	return 0;
# }], [AC_MSG_RESULT(yes)],
#     [AC_DEFINE(HAVE_NONULL_FREE, 1, [free does not accept NULL parameter])
#      AC_MSG_RESULT(no)],
#     [dnl assume NULL is not accepted when cross-compiling
#      AC_DEFINE(HAVE_NONULL_FREE, 1, [free does not accept NULL parameter])
#      AC_MSG_RESULT(no)])
#XXX always set for now
AC_DEFINE(HAVE_NONULL_FREE, 1, [free does not accept NULL parameter])

AC_MSG_CHECKING([if getsockopt needs cast])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
int getsockopt(int, int, int, char *, int *);
], [ 1 ],
 [AC_DEFINE(NEED_GETSOCKOPT_CAST, 1, [getsockopt needs cast])
  AC_MSG_RESULT(yes)],
  AC_MSG_RESULT(no))

#include both <sys/ioctl.h> and <sys/sockio.h>?
if test x$have_sys_sockio_h = xt; then
	AC_MSG_CHECKING([to see if <sys/sockio.h> should be included])
	AC_EGREP_CPP(yes, [
#include <sys/ioctl.h>
#ifdef SIOCATMARK
#else
#include <sys/sockio.h>
#ifdef SIOCATMARK
yes
#endif
#endif
], [AC_DEFINE(NEED_SYS_SOCKIO_H, 1, [sys/sockio.h must be included])
    AC_MSG_RESULT(yes)],
    AC_MSG_RESULT(no))
fi

#XXX should be more generic, check if nonexistent
#SV_INTERRUPT, but not SA_RESTART defined?
AC_MSG_CHECKING([to see if SV_INTERRUPT should be used])
AC_EGREP_CPP(yes, [
#include <signal.h>
#ifdef SA_RESTART
#else
# ifdef SV_INTERRUPT
yes
# endif
#endif
], [AC_DEFINE(NEED_SA_RESTART, 1, [use SA_RESTART, not SV_INTERRUPT])
    AC_MSG_RESULT(yes)],
    AC_MSG_RESULT(no))

#XXX seems to be supported on Solaris2.6, but there might be some
#defines that need to be set (should _XOPEN_SOURCE be defined on all
#platforms?)
AC_MSG_CHECKING([if cmsghdr exists in <sys/socket.h>])
case $host in
    *)
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
], [struct cmsghdr foo = {1,1,1};
 struct msghdr bar;
 foo.cmsg_len = 0;
 foo.cmsg_type = SCM_RIGHTS;
 foo.cmsg_level = SOL_SOCKET;
 bar.msg_controllen = 1;
], [AC_DEFINE(HAVE_CMSGHDR, 1, [struct cmsghdr exists])
    AC_MSG_RESULT(yes)],
    AC_MSG_RESULT(no))
	;;
esac

AC_MSG_CHECKING([for CMSG_SPACE in sys/socket.h])
AC_TRY_LINK([
#include <sys/types.h>
#include <sys/socket.h>
], [int d = CMSG_SPACE(4);
   return 0;
], [AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_CMSG_SPACE, 1, CMSG_SPACE exists)],
   [AC_MSG_RESULT(no)])

AC_MSG_CHECKING([for CMSG_LEN in sys/socket.h])
AC_TRY_LINK([
#include <sys/types.h>
#include <sys/socket.h>
], [int d = CMSG_LEN(4);
    return 0;
], [AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_CMSG_LEN, 1, [CMSG_LEN exists])],
   [AC_MSG_RESULT(no)])

unset have_sa_len
AC_MSG_CHECKING([for sa_len in sockaddr])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
], [struct sockaddr sa;
sa.sa_len = 0;
], [AC_DEFINE(HAVE_SOCKADDR_SA_LEN, 1, [sa_len exists in sockaddr])
    have_sa_len=t
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)])

if test x"${have_sa_len}" != x; then
   AC_MSG_CHECKING([for sa_len type])
   oCFLAGS="$CFLAGS"
   CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN"
   unset sa_len_type_found
   for type in uint8_t "unsigned char"; do
       AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>], [
struct sockaddr sa;
$type *sa_len_ptr;
sa_len_ptr = &sa.sa_len;
if (sa_len_ptr != NULL)
   sa_len_ptr = NULL; /* used to avoid warning/error */],
       [AC_DEFINE_UNQUOTED(sa_len_type, [$type], [sa_len type])
        sa_len_type_found=t
        break])
   done
   CFLAGS="$oCFLAGS"

   if test x"${sa_len_type_found}" != x; then
       AC_MSG_RESULT([yes])
   else
       AC_MSG_RESULT([no])
       AC_MSG_WARN([unable to obtain sa_len type, exiting])
       exit 1
   fi
else
   AC_DEFINE(sa_len_type, [socklen_t], [sa_len type])
fi

unset have_ss_len
AC_MSG_CHECKING([for sockaddr_storage ss_len in sockaddr])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
], [struct sockaddr_storage ss;
ss.ss_len = 0;
], [AC_DEFINE(HAVE_SOCKADDR_STORAGE_SS_LEN, 1, [ss_len exists in sockaddr_storage])
    have_ss_len=t
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)])

AC_MSG_CHECKING([for __res_state options type])
oCFLAGS="$CFLAGS"
CFLAGS="$CFLAGS${CFLAGS:+ }$FAILWARN"
unset res_options_type_found
for type in "unsigned int" u_long; do
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <stdio.h>], [
struct __res_state res;
$type *res_options_ptr;
res_options_ptr = &res.options;
if (res_options_ptr != NULL)
   res_options_ptr = NULL; /* used to avoid warning/error */],
    [AC_DEFINE_UNQUOTED(res_options_type_t, [$type], [resolver options type])
     res_options_type_found=t
     break])
done
CFLAGS="$oCFLAGS"
if test x"${res_options_type_found}" != x; then
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
    AC_MSG_WARN([unable to obtain __res_state options type, setting to unsigned long])
    AC_DEFINE_UNQUOTED(res_options_type_t, [unsigned long], [resolver options type])
fi

#AIX 6.1 needs sys/select.h, but can be problematic on other platforms
AC_MSG_CHECKING([if sys/select.h is needed])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
], [fd_set *fdp;
    size_t p;
    fdp = NULL;
    p = NFDBITS * sizeof(fd_mask);
], [AC_MSG_RESULT(no)],
   [dnl compilation failure, try with sys/select.h (ignore if this fails)
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
], [fd_set *fdp;
    size_t p;
    fdp = NULL;
    p = NFDBITS * sizeof(fd_mask);],
    [AC_DEFINE(HAVE_SYS_SELECT_H, 1, [sys/select.h needed])
     AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)])])

AC_MSG_CHECKING([to see if malloc_options exists])
AC_TRY_LINK([extern char *malloc_options;],
[ malloc_options = 0; ],
[AC_DEFINE(HAVE_MALLOC_OPTIONS, 1, [support for malloc debugging])
 AC_MSG_RESULT(yes)],
[AC_MSG_RESULT(no)])

AC_MSG_CHECKING([to see if __progname exists])
AC_TRY_LINK([extern char *__progname;],
[ __progname = 0; ],
[AC_DEFINE(HAVE_PROGNAME, 1, [programe name symbol exists])
 AC_MSG_RESULT(yes)],
[AC_MSG_RESULT(no)])

dnl Checks for libraries.
AC_SEARCH_LIBS(crypt, crypt)

#HP-UX 11.00
AC_SEARCH_LIBS(getspnam, sec)

#FreeBSD has setproctitle in -lutil
AC_SEARCH_LIBS(setproctitle, util)

#expected select behavior?
unset nb_select_err
L_UNCON_SELECT([],
 [nb_select_err=t])

if test x"${nb_select_err}" = xt; then
   AC_MSG_WARN([socksify operations on nonblocking sockets might fail on this platform])
fi

#Linux (RedHat 5.2) defines socklen_t in <socketbits.h>, which is
#included by <sys/socket.h>.  check for this first.
AC_MSG_CHECKING([for socklen_t])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
], [socklen_t foo = 1;],
   [AC_MSG_RESULT(yes)
    socklen_found=t],
   [AC_MSG_RESULT(no)
    socklen_found=""])

AH_TEMPLATE([socklen_t], [platform workaround])
if test x"$socklen_found" = x; then
    case $host in
	alpha*-dec-osf* | *-*-aix*)
	    AC_DEFINE(socklen_t, size_t)
	    ;;

	*)
	    AC_DEFINE(socklen_t, int)
	    ;;
    esac
fi

#sig_atomic_t
AC_MSG_CHECKING([for sig_atomic_t in <signal.h>])
AC_EGREP_CPP(sig_atomic_t, [
#include <signal.h>
], [AC_DEFINE(HAVE_SIG_ATOMIC_T, 1, [sig_atomic_t defined in signal.h])
    case $host in
	*-*-aix*)
	    AC_DEFINE(HAVE_VOLATILE_SIG_ATOMIC_T, 1, [platform workaround])
	;;
    esac
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)])

#try to identify number of valid signal values
SIGDEFAULT=128
unset sigmax
AC_MSG_CHECKING([for number of valid signal values])
AC_TRY_RUN([
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
   FILE *fp;
   int minval = 0;
   int i;

   for (i = 1; i < 1000; i++) {
      if (i == SIGKILL || i == SIGSTOP)
         continue;
      if (signal(i, SIG_DFL) == SIG_ERR) {
         perror("signal");
         break;
      }
   }
   minval = i;
   fprintf(stderr, "notice: signal() based value: %d\n", minval);

#ifdef NSIG
   if (NSIG > minval) {
      minval = NSIG;
      fprintf(stderr, "notice: NSIG based value: %d\n", minval);
   }
#endif /* NSIG */

#ifdef SIGRTMAX
   /* note: this value might be changed at runtime (e.g., Linux) */
   if (SIGRTMAX > minval) {
      minval = SIGRTMAX;
      fprintf(stderr, "notice: SIGRTMAX based value: %d\n", minval);
   }
#endif /* SIGRTMAX */

   /* get base 2 value */
   for (i = 0; i < 10; i++) {
      int n = 1 << i;
      if (n > minval) {
         minval = n;
         fprintf(stderr, "notice: base 2 based increase: %d\n", minval);
	 break;
      }
   }

   fprintf(stderr, "notice: setting signal max value to %d\n", minval);
   if ((fp = fopen("conftest.out", "w")) == NULL) {
      perror("fopen");
      exit(1);
   }
   /* write zero for no special handling needed */
   fprintf(fp, "%ld\n", minval);
   fclose(fp);
   exit(0);

   return 0;
}], [sigmax=`test -s conftest.out && cat conftest.out`],
[],
[dnl set a large value when cross-compiling
 sigmax=128
 AC_MSG_RESULT([cross-compiling, setting to $sigmax])
])
if test x"$sigmax" = x; then
   sigmax=$SIGDEFAULT
   AC_MSG_RESULT([got no value, setting to $sigmax])
else
   AC_MSG_RESULT(setting to $sigmax)
fi
AC_DEFINE_UNQUOTED(SOCKS_NSIG, $sigmax, [Guess at max number of valid signals])

AC_CHECK_TYPES([int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t,
		in_port_t, in_addr_t],
		, ,
[
#include <sys/types.h>
#include <netinet/in.h>
])
AC_CHECK_TYPE(ssize_t, int)

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF

#doesn't work if test links with -lsocket (Solaris)
AC_SEARCH_LIBS(inet_addr, nsl)

#NOTE: also test for -lsocket in preload.m4. not really needed there
#      as check is also done here (for checks such as L_PIPETYPE() that
#      might need to link with -lsocket, but also having test there makes
#      code in that file easier to read.
AC_SEARCH_LIBS(getsockopt, socket)

AC_CHECK_FUNCS(getprpwnam getspnam getpwnam_shadow bindresvport)
AC_CHECK_FUNCS(getpass)

AC_MSG_CHECKING([for system V getpwnam])
unset getpwnam_alt
if test x"${ac_cv_func_getprpwnam}" = xyes; then
    getpwnam_alt=t
fi

if test x"${ac_cv_func_getspnam}" = xyes; then
    getpwnam_alt=t
fi

#XXX
if test x"${getpwnam_alt}" = x; then
    AC_DEFINE(HAVE_WORKING_GETPWNAM, 1, [system V getpwnam])
    AC_MSG_RESULT(no)
else
    AC_MSG_RESULT(yes)
fi

#try to determine pipe buffer type
L_PIPETYPE()

#obtain highest valid select() timeout seconds value
L_SELECT_MAXTIMEOUT

#generate list of errno values
ERRNOSRC="include/errorsymbols_gen.h"
cp /dev/null $ERRNOSRC
ERRVALFILE="errorvals.txt" #for counting unique (numeric) values
cp /dev/null $ERRVALFILE
unset ERRNOVALS
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, E2BIG)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EACCES)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EADDRINUSE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EADDRNOTAVAIL)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EADV)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EAFNOSUPPORT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EAGAIN)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EALREADY)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EAUTH)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADARCH)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADEXEC)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADF)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADFD)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADMACHO)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADMSG)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADR)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADRPC)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADRQC)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBADSLT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBFONT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EBUSY)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECANCELED)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECAPMODE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECHILD)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECHRNG)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECOMM)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECONNABORTED)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECONNREFUSED)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECONNRESET)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDEADLK)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDEADLOCK)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDESTADDRREQ)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDEVERR)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDIRIOCTL)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDOM)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDOOFUS)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDQUOT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EEXIST)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EFAULT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EFBIG)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EFTYPE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EHOSTDOWN)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EHOSTUNREACH)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EIDRM)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EILSEQ)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EINPROGRESS)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EINTR)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EINVAL)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EIO)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EIPSEC)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EISCONN)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EISDIR)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EJUSTRETURN)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EL2HLT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EL2NSYNC)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EL3HLT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EL3RST)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ELAST)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ELIBACC)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ELIBBAD)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ELIBEXEC)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ELIBMAX)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ELIBSCN)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ELNRNG)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ELOCKUNMAPPED)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ELOOP)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EMEDIUMTYPE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EMFILE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EMLINK)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EMSGSIZE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EMULTIHOP)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENAMETOOLONG)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENEEDAUTH)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENETDOWN)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENETRESET)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENETUNREACH)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENFILE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOANO)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOATTR)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOBUFS)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOCSI)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENODATA)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENODEV)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOENT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOEXEC)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOIOCTL)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOLCK)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOLINK)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOMEDIUM)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOMEM)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOMSG)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENONET)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOPKG)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOPOLICY)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOPROTOOPT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOSPC)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOSR)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOSTR)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOSYS)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTACTIVE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTBLK)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTCAPABLE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTCONN)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTDIR)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTEMPTY)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTRECOVERABLE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTSOCK)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTSUP)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTTY)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTUNIQ)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENXIO)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EOPNOTSUPP)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EOVERFLOW)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EOWNERDEAD)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPERM)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPFNOSUPPORT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPIPE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPROCLIM)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPROCUNAVAIL)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPROGMISMATCH)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPROGUNAVAIL)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPROTO)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPROTONOSUPPORT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPROTOTYPE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EPWROFF)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ERANGE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EREMCHG)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EREMOTE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ERESTART)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EROFS)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ERPCMISMATCH)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESHLIBVERS)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESHUTDOWN)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESOCKTNOSUPPORT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESPIPE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESRCH)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESRMNT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESTALE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESTRPIPE)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ETIME)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ETIMEDOUT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ETOOMANYREFS)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ETXTBSY)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EUNATCH)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EUSERS)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EWOULDBLOCK)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EXDEV)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EXFULL)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECLONEME)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ECORRUPT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDESTADDREQ)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EDIST)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EFORMAT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EMEDIA)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOCONNECT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTEMPTY)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTREADY)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ENOTRUST)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESAD)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESOFT)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, ESYSERROR)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EWOULDBLOCK)
L_CHECKERRNO(ERRNOVALS, $ERRVALFILE, EWRPROTECT)

if test x"$ERRNOVALS" = x; then
    AC_MSG_FAILURE([error: no errno symbols found])
fi
ERRNOCNT=`echo $ERRNOVALS | wc -w | awk '{ print $1 }'`

AC_MSG_CHECKING([errno symbols])
unset UNIQUEVALS UNIQUESYMBOLS
if test -s $ERRVALFILE; then
    UNIQUEVALS=`sort $ERRVALFILE | uniq | wc -l | awk '{ print $1 }'`
    UNIQUESYMBOLS=`cat $ERRVALFILE | wc -l | awk '{ print $1 }'`
    if test $ERRNOCNT -ne $UNIQUESYMBOLS; then
	AC_MSG_FAILURE([internal error: errno symbol count mismatch])
    fi
    AC_DEFINE_UNQUOTED(UNIQUE_ERRNO_VALUES, $UNIQUEVALS,
	[Number of unique errno numbers])
    AC_MSG_RESULT([unique symbols: $UNIQUESYMBOLS, unique values: $UNIQUEVALS])
else
    AC_MSG_FAILURE([error: unable to determine errno symbol values])
fi

changequote(<<, >>)dnl
echo "/* NOTICE: $ERRNOSRC: Generated from by configure */" >>$ERRNOSRC
echo "" >>$ERRNOSRC
echo "static const errorsymboltable_t errnosymbolv[] = {" >>$ERRNOSRC
changequote([, ])dnl

#generic list
for errno in $ERRNOVALS; do
    echo "   { \"$errno\",	$errno	}," >>$ERRNOSRC
done

#keyword alias list
maxaliaslen=0 #highest number of values for an alias

#errno values that can match 'no-route' keyword (if found on platform)
ERRNO_NOROUTE="ENETUNREACH EHOSTUNREACH ENETDOWN ETIMEDOUT"
keyword="no-route"
keycnt=0
for keyval in ${ERRNO_NOROUTE}; do
    for errno in $ERRNOVALS; do
	if test x"$keyval" = x"$errno"; then
	    echo "   { \"$keyword\",	$errno	}," >>$ERRNOSRC
	    keycnt=`expr $keycnt + 1`
	fi
    done
done

#any-error; alias for all values
keyword="system-any"
keycnt=0
for errno in $ERRNOVALS; do
    echo "   { \"$keyword\",	$errno	}," >>$ERRNOSRC
    keycnt=`expr $keycnt + 1`
done

if test $keycnt -gt $maxaliaslen; then
    maxaliaslen=$keycnt
fi

echo "};" >>$ERRNOSRC

AC_DEFINE_UNQUOTED(MAX_ERRNO_VALUES_FOR_SYMBOL, $maxaliaslen,
    [Max number of errno values matching any alias keyword])


#generate list of getaddrinfo errors
cp /dev/null $ERRVALFILE
unset GAIERRVALS
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_ADDRFAMILY)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_AGAIN)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_BADFLAGS)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_BADHINTS)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_FAIL)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_FAMILY)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_MEMORY)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_NODATA)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_NONAME)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_OVERFLOW)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_PROTOCOL)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_SERVICE)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_SOCKTYPE)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_SYSTEM)

L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_ALLDONE)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_CANCELED)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_IDN_ENCODE)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_INPROGRESS)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_INTR)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_NOTCANCELED)
L_CHECKGAIERROR(GAIERRVALS, $ERRVALFILE, EAI_BADEXTFLAGS)

if test x"$GAIERRVALS" = x; then
    AC_MSG_FAILURE([error: no getaddrinfo() error symbols found])
fi
ERRNOCNT=`echo $GAIERRVALS | wc -w | awk '{ print $1 }'`

AC_MSG_CHECKING([getaddrinfo() error symbols])
unset UNIQUEVALS UNIQUESYMBOLS
if test -s $ERRVALFILE; then
    UNIQUEVALS=`sort $ERRVALFILE | uniq | wc -l | awk '{ print $1 }'`
    if test $UNIQUEVALS -le 1; then
        AC_MSG_FAILURE([error: getaddrinfo() error value count too low])
    fi
    UNIQUESYMBOLS=`cat $ERRVALFILE | wc -l | awk '{ print $1 }'`
    if test $UNIQUESYMBOLS -le 1; then
        AC_MSG_FAILURE([error: getaddrinfo() error symbol count too low])
    fi

    if test $ERRNOCNT -ne $UNIQUESYMBOLS; then
	AC_MSG_FAILURE([internal error: errno symbol count mismatch])
    fi
    AC_DEFINE_UNQUOTED(UNIQUE_GAIERR_VALUES, $UNIQUEVALS,
	[Number of unique getaddrinfo() error numbers])
    AC_MSG_RESULT([unique symbols: $UNIQUESYMBOLS, unique values: $UNIQUEVALS])
else
    AC_MSG_FAILURE([error: unable to determine getaddrinfo() error symbol values])
fi

changequote(<<, >>)dnl
echo "" >>$ERRNOSRC
echo "static const errorsymboltable_t gaierrsymbolv[] = {" >>$ERRNOSRC
changequote([, ])dnl

#generic list
for errno in $GAIERRVALS; do
    echo "   { \"$errno\",	$errno	}," >>$ERRNOSRC
done

#keyword alias list
maxaliaslen=0 #highest number of values for an alias

#error values that can match 'foo' keyword (if found on platform)
ERRNO_foo="..."
keyword="foo"
keycnt=0
for keyval in ${ERRNO_foo}; do
    for errno in $GAIERRVALS; do
	if test x"$keyval" = x"$errno"; then
	    echo "   { \"$keyword\",	$errno	}," >>$ERRNOSRC
	    keycnt=`expr $keycnt + 1`
	fi
    done
done

#any-error; alias for all values
keyword="dns-any"
keycnt=0
for errno in $GAIERRVALS; do
    echo "   { \"$keyword\",	$errno	}," >>$ERRNOSRC
    keycnt=`expr $keycnt + 1`
done

if test $keycnt -gt $maxaliaslen; then
    maxaliaslen=$keycnt
fi

echo "};" >>$ERRNOSRC

AC_DEFINE_UNQUOTED(MAX_GAIERR_VALUES_FOR_SYMBOL, $maxaliaslen,
    [Max number of gataddrinfo() error values matching any alias keyword])
rm -f $ERRVALFILE
