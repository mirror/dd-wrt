# -- acinclude start --
define([concat],
[$1$2]) dnl XXX likely a simpler way to do this
AC_DEFUN([L_MODVER],
[AC_MSG_CHECKING(for module $1)
 if test -f "licensed/$1.c"; then
	MINVER="$2"
	unset MODVER
	MODLINE=`head -1 licensed/$1.c | grep MODVER`
	if test x"$MODLINE" != x; then
		MODVER=`echo "$MODLINE" | cut -d: -f 2`
	fi

	dnl try old syntax if nothing returned
	dnl XXX two single quotes in first argument to split,
	dnl     prevents removal by m4
	if test x"$MODVER" = x; then
		MODVER=`awk '/Id:/{ split($''4,a,".");print a[[2]]; exit }' licensed/$1.c`
	fi
	if test "$MODVER" -lt "$MINVER"; then
		echo "" >&2
		echo "You have version 1.$MODVER of the $1 module, which is outdated." >&2
		echo "This version of Dante requires at least version 1.$MINVER." >&2
		echo "Please contact Inferno Nettverk A/S for an updated" >&2
		echo "version before you attempt to compile." >&2
		echo "Inferno Nettverk A/S can be reached at info@inet.no." >&2
		echo "" >&2
		echo "There is no additional cost for upgrading." >&2
		exit 1
	fi

	#command to run in case of success
	$4

	unset concat(HAVEMOD_, m4_toupper($1))
	AC_DEFINE(concat(HAVE_MODULE_, m4_toupper($1)), 1, [module $1 installed])dnl
	AC_MSG_RESULT(yes)
else
	concat(HAVEMOD_, m4_toupper($1))=un

	AC_MSG_RESULT(no)
fi
AC_LINK_FILES(${concat(HAVEMOD_, m4_toupper($1))}licensed/$1.c, $3/$1.c)
if test x"$5" != xnokey; then
    AC_LINK_FILES(${concat(HAVEMOD_, m4_toupper($1))}licensed/$1_key.c, $3/$1_key.c)
fi
])


AC_DEFUN([L_UNCON_SELECT],
[AC_MSG_CHECKING(for expected select behaviour)
AC_RUN_IFELSE([AC_LANG_SOURCE([
/*
 * ftp.inet.no:/pub/home/michaels/stuff/socket-select.c
 * $ cc socket-select.c && uname -a && ./a.out
 *
 * Thanks to Eric Anderson <anderse@hpl.hp.com>.
 *
*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define BLOCKING(b)	(b ? ("blocking") : ("non-blocking"))

static int selectcheck(int s);
static int dotests(int s, int blocking);

int
main(void)
{
	int s, p;
	struct sigaction sigact;

	sigemptyset(&sigact.sa_mask);
	sigact.sa_handler = SIG_IGN;
	sigact.sa_flags	= 0;
	if (sigaction(SIGPIPE, &sigact, NULL) != 0) {
		perror("sigaction()");
		exit(1);
	}

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket()");
		exit(1);
	}

	if ((p = fcntl(s, F_GETFL, 0)) == -1
	|| fcntl(s, F_SETFL, p | O_NONBLOCK) == -1) {
		perror("fcntl()");
	   exit(1);
	}

	p = dotests(s, 0);
	close(s);

	return p;
}


static int
dotests(int s, int blocking)
{
	int p, rc;
	struct sockaddr_in addr;

	fprintf(stderr, "testing with %s, bound, socket:\n", BLOCKING(blocking));
	bzero(&addr, sizeof(addr));
	addr.sin_family		= AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port			= htons(0);

	/* LINTED pointer casts may be troublesome */
	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		perror("running Linux? bind()");
		exit(1);
	}

	fprintf(stderr, "\tselect() returned %d\n", selectcheck(s));
	p = read(s, NULL, 0);
	fprintf(stderr, "\tread() returned %d, errno = %d (%s)\n",
	p, errno, (strerror(errno)));
	p = write(s, &s, sizeof(s));
	fprintf(stderr, "\twrite() returned %d, errno = %d (%s)\n",
	p, errno, (strerror(errno)));

	fprintf(stderr, "testing with %s, bound, listening, socket:\n", BLOCKING(blocking));
	if (listen(s, 1) != 0) {
		perror("listen()");
		exit(1);
	}
	fprintf(stderr, "\tselect() returned %d\n", rc = selectcheck(s));
	p = read(s, NULL, 0);
	fprintf(stderr, "\tread() returned %d, errno = %d (%s)\n",
	p, errno, (strerror(errno)));
	p = write(s, &s, sizeof(s));
	fprintf(stderr, "\twrite() returned %d, errno = %d (%s)\n",
	p, errno, (strerror(errno)));

	return rc;
}

static int
selectcheck(int s)
{
	fd_set rset, wset, xset;
	struct timeval timeout;
	int ret;

	FD_ZERO(&rset);
	FD_SET(s, &rset);
	wset = xset = rset;

	timeout.tv_sec		= 0;
	timeout.tv_usec	= 0;

	errno = 0;
	ret = select(s + 1, &rset, &wset, &xset, &timeout);

	if (FD_ISSET(s, &rset))
	    fprintf(stderr, "\tsocket is readable\n");

	if (FD_ISSET(s, &wset))
	    fprintf(stderr, "\tsocket is writeable\n");

	if (FD_ISSET(s, &xset))
	    fprintf(stderr, "\tsocket has an exception pending\n");

	return ret;
}])], [AC_MSG_RESULT(yes)
     [$1]],
    [AC_MSG_RESULT(no)
     [$2]],
    [dnl assume yes when cross-compiling
     AC_MSG_RESULT(yes)
     [$1]])])


dnl addproto - generate AC_DEFINE statements
define([addproto],
[AC_DEFINE_UNQUOTED(HAVE_PROT_$1_$2, [$3], [proto])dnl
ifelse([$4], , , [addproto($1, m4_incr($2), m4_shiftn(3, $@))])])

dnl tproto - generate statements for running AC_COMPILE_IFELSE
define([tproto],
[AC_COMPILE_IFELSE([
 AC_LANG_PROGRAM([
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netdb.h>
#include <unistd.h>

m4_esyscmd([echo "$3" | cut -d, -f 1])dnl
$1(m4_esyscmd([echo "$3" | cut -d, -f 2-]));], [])],
 [addproto(m4_toupper($1), 0, $3)
  AC_MSG_RESULT(ok)],
 [ifelse([$4], ,
  [AC_MSG_RESULT(failure)
   AC_MSG_WARN([missing prototype for $1])
  $2],
 [tproto([$1], [$2], m4_shiftn(3, $@))])])])

dnl L_NSOCKPROTO - determine function prototypes by compilation
AC_DEFUN([L_NSOCKPROTO],[
AC_REQUIRE([AC_COMPILE_IFELSE])dnl
AC_MSG_CHECKING([prototypes for $1])dnl

tproto($@)])

dnl tstdioproto - generate statements for running AC_COMPILE_IFELSE
define([tstdioproto],
[AC_COMPILE_IFELSE([
 AC_LANG_PROGRAM([
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#ifdef $1
#undef $1
#endif

m4_esyscmd([echo "$3" | cut -d, -f 1])dnl
$1(m4_esyscmd([echo "$3" | cut -d, -f 2-]));], [])],
 [addproto(m4_toupper($1), 0, $3)
  AC_MSG_RESULT(ok)],
 [ifelse([$4], ,
  [AC_MSG_RESULT(failure)
   AC_MSG_WARN([missing prototype for $1])
  $2],
 [tstdioproto([$1], [$2], m4_shiftn(3, $@))])])])

dnl L_NSTDIOPROTO - determine stdio function prototypes by compilation
AC_DEFUN([L_NSTDIOPROTO],[
AC_REQUIRE([AC_COMPILE_IFELSE])dnl
AC_MSG_CHECKING([prototypes for $1])dnl

tstdioproto($@)])

dnl define function for identifying socket options on platform
dnl test adds to the SOCKOPTS variable
m4_define([checksockopt],
 [unset _compileok
  AC_MSG_CHECKING(for $1 socket option $2)
  AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
  ], [
   socklen_t optlen;
   int optval;
   int stype;
   int ptype;
   int s;

   if ($1 == SOL_SOCKET || $1 == IPPROTO_TCP) {
      stype = SOCK_STREAM; /* XXX test only TCP in case of SOL_SOCKET */
      ptype = IPPROTO_TCP;
   } else if ($1 == IPPROTO_IP) {
      stype = SOCK_DGRAM; /* XXX test only UDP in case of IPPROTO_IP */
      ptype = IPPROTO_IP;
   } else if ($1 == IPPROTO_IPV6) {
      stype = SOCK_DGRAM;   /* XXX test only UDP in case of IPPROTO_IPV6 */
      ptype = IPPROTO_IPV6; /* set to v6 for ipv6 test */
   } else if ($1 == IPPROTO_UDP) {
      stype = SOCK_DGRAM;
      ptype = IPPROTO_UDP;
   } else {
       fprintf(stderr, "error: unexpected socket type: $1");
       exit(1);
   }

   if((s = socket(PF_INET, stype, ptype)) < 0) {
      perror("socket");
      exit(1);
   }

   optval = 1;
   optlen = sizeof(optval);
   if(setsockopt(s, $1, $2, &optval, optlen) < 0) {
      perror("setsockopt: $1 $2");
      close(s);
      exit(1);
   }], [_compileok=1])

  AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
  ], [
   socklen_t optlen;
   int optval;
   int stype;
   int ptype;
   int s;

   if ($1 == SOL_SOCKET || $1 == IPPROTO_TCP) {
      stype = SOCK_STREAM; /* XXX test only TCP in case of SOL_SOCKET */
      ptype = IPPROTO_TCP;
   } else if ($1 == IPPROTO_IP) {
      stype = SOCK_DGRAM; /* XXX test only UDP in case of IPPROTO_IP */
      ptype = IPPROTO_IP;
   } else if ($1 == IPPROTO_IPV6) {
      stype = SOCK_DGRAM;   /* XXX test only UDP in case of IPPROTO_IPV6 */
      ptype = IPPROTO_IPV6; /* set to v6 for ipv6 test */
   } else if ($1 == IPPROTO_UDP) {
      stype = SOCK_DGRAM;
      ptype = IPPROTO_UDP;
   } else {
       fprintf(stderr, "error: unexpected socket type: $1");
       exit(1);
   }

   if((s = socket(PF_INET6, stype, ptype)) < 0) {
      perror("socket");
      exit(1);
   }

   optval = 1;
   optlen = sizeof(optval);
   if(setsockopt(s, $1, $2, &optval, optlen) < 0) {
      perror("setsockopt: $1 $2");
      close(s);
      exit(1);
   }], [_compileok=1])

  if test x"${_compileok}" != x; then
    AC_MSG_RESULT(yes)
    AC_DEFINE_UNQUOTED(HAVE_$2, 1, [$2 socket option])dnl
    AC_DEFINE_UNQUOTED(SOCKS_$2_LVL, $1, [$2 protocol level])dnl
    AC_DEFINE_UNQUOTED(SOCKS_$2_NAME, "m4_tolower($2)", [$2 value])dnl
    if test x"$1" = x"IPPROTO_IP"; then
       #ipv4-only
       AC_DEFINE_UNQUOTED(SOCKS_$2_IPV4, 1, [$2 IPv4 option])dnl
       AC_DEFINE_UNQUOTED(SOCKS_$2_IPV6, 0, [$2 IPv4 option])dnl
    elif test x"$1" = x"IPPROTO_IPV6"; then
       #ipv6-only
       AC_DEFINE_UNQUOTED(SOCKS_$2_IPV4, 0, [$2 IPv4 option])dnl
       AC_DEFINE_UNQUOTED(SOCKS_$2_IPV6, 1, [$2 IPv4 option])dnl
    else
       #both ipv4 and ipv6
       AC_DEFINE_UNQUOTED(SOCKS_$2_IPV4, 1, [$2 IPv4 option])dnl
       AC_DEFINE_UNQUOTED(SOCKS_$2_IPV6, 1, [$2 IPv4 option])dnl
    fi
    [SOCKOPTS="$SOCKOPTS${SOCKOPTS:+ }$2"]
  else
    AC_MSG_RESULT(no)
  fi])
AC_DEFUN([L_CHECKSOCKOPT],
 [checksockopt($@)])

dnl define function for identifying symbolic arguments to socket options
dnl test adds to the SOCKOPTVALSYMS variable
m4_define([checksockoptvalsym],
 [AC_MSG_CHECKING(for socket option symbol $2 value $3)
  AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
  ], [
   socklen_t optlen;
   int optval;
   int stype;
   int ptype;
   int s;

   if ($1 == SOL_SOCKET || $1 == IPPROTO_TCP) {
      stype = SOCK_STREAM; /* XXX test only TCP in case of SOL_SOCKET */
      ptype = IPPROTO_TCP;
   } else if ($1 == IPPROTO_IP) {
      stype = SOCK_DGRAM; /* XXX test only UDP in case of IPPROTO_IP */
      ptype = IPPROTO_IP;
   } else if ($1 == IPPROTO_UDP) {
      stype = SOCK_DGRAM;
      ptype = IPPROTO_UDP;
   } else {
       fprintf(stderr, "error: unexpected socket type: $1");
       exit(1);
   }

   if((s = socket(PF_INET, stype, ptype)) < 0) {
      perror("socket");
      exit(1);
   }

   optval = $3;
   optlen = sizeof(optval);
   if(setsockopt(s, $1, $2, &optval, optlen) < 0) {
      perror("setsockopt: $1 $2");
      close(s);
      exit(1);
   }], [AC_MSG_RESULT(yes)
     AC_DEFINE_UNQUOTED(SOCKS_$3_SYMNAME, "m4_tolower($3)", [$3 value])dnl
     [SOCKOPTVALSYMS="$SOCKOPTVALSYMS${SOCKOPTVALSYMS:+ }$3"]],
    [AC_MSG_RESULT(no)])])
AC_DEFUN([L_CHECKSOCKOPTVALSYM],
 [checksockoptvalsym($@)])

dnl XXX readside/sendside test should be simplified by using conftest.out
AC_DEFUN([L_PIPETYPE], [
unset pipeside
#Some systems seem to base how much can be written to the pipe based
#on the size of the socket receive buffer (read-side), while others
#on the size of the socket send buffer (send-side).
#
#This little hack tries to make an educated guess as to what is the
#case on this particular system.
AC_MSG_CHECKING(read/send-side pipe system)
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/socket.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif /* !MIN */

#if NEED_AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif /* NEED_AF_LOCAL */

#define PACKETSIZE  (1024)

#define PADBYTES    (sizeof(short) * (64))
/*
 * Just a wild guess.  Dante uses sizeof(long).
 */

#define SEND_PIPE   (0)
#define RECV_PIPE   (1)

#define EXIT_OK      (0) /* type successfully determined */
#define EXIT_UNKNOWN (1) /* error: unable to determine type */

static void
setsockets(const int doreverse, const size_t packetsize,
           const int s, const int r,
           int *sndbuf, int *sndbuf_set,
           int *rcvbuf, int *rcvbuf_set);

static size_t
sendtest(const int s, const char *buf, const size_t buflen);

void
reswrite(const char *res);

int
main(void)
{
   size_t sent, packetcount;
   int sndbuf, sndbuf_set, rcvbuf, rcvbuf_set;
   char buf[PACKETSIZE];
   int datapipev[2];

   if (socketpair(AF_LOCAL, SOCK_DGRAM, 0, datapipev) != 0) {
      perror("socketpair()");
      exit(EXIT_UNKNOWN);
   }

   setsockets(0,
              PACKETSIZE,
              datapipev[SEND_PIPE],
              datapipev[RECV_PIPE],
              &sndbuf, &sndbuf_set,
              &rcvbuf, &rcvbuf_set);

   packetcount = MIN(sndbuf, sndbuf_set) / (PACKETSIZE + PADBYTES);

   fprintf(stderr,
           "Requested sndbuf to be %d, is %d.  "
           "Requested rcvbuf to be %d, is %d.\n"
           "Calculated packetcount is %lu\n",
           sndbuf, sndbuf_set,
           rcvbuf, rcvbuf_set,
           (unsigned long)packetcount);

   sent = sendtest(datapipev[SEND_PIPE], buf, PACKETSIZE) / PACKETSIZE;

   if (sent >= packetcount) {
      fprintf(stderr, "status determined by send-side\n");
      reswrite("sendbased");
      exit(EXIT_OK);
   }

   /*
    * Try the reverse.  Perhaps this system wants a large rcvbuf rather than
    * a large sndbuf.
    */
   close(datapipev[SEND_PIPE]);
   close(datapipev[RECV_PIPE]);

   if (socketpair(AF_LOCAL, SOCK_DGRAM, 0, datapipev) != 0) {
      perror("socketpair()");
      exit(EXIT_UNKNOWN);
   }

   setsockets(1,
              PACKETSIZE,
              datapipev[SEND_PIPE],
              datapipev[RECV_PIPE],
              &sndbuf, &sndbuf_set,
              &rcvbuf, &rcvbuf_set);

   packetcount = MIN(rcvbuf, rcvbuf_set) / (PACKETSIZE + PADBYTES);

   fprintf(stderr,
           "Requested sndbuf to be %d, is %d.  "
           "Requested rcvbuf to be %d, is %d.\n"
           "Calculated packetcount is %lu\n",
           sndbuf, sndbuf_set,
           rcvbuf, rcvbuf_set,
           (unsigned long)packetcount);

   sent = sendtest(datapipev[SEND_PIPE], buf, PACKETSIZE) / PACKETSIZE;

   if (sent >= packetcount) {
      fprintf(stderr, "status determined by read-side\n");
      reswrite("recvbased");
      exit(EXIT_OK);
   }

   fprintf(stderr, "status is unknown\n");
   return EXIT_UNKNOWN;
}

static void
setsockets(const int doreverse, const size_t packetsize,
           const int s, const int r, int *sndbuf, int *sndbuf_set,
           int *rcvbuf, int *rcvbuf_set)
{
   socklen_t len;
   int p;

   if ((p = fcntl(s, F_GETFL, 0))        == -1
   ||  fcntl(s, F_SETFL, p | O_NONBLOCK) == -1
   ||  fcntl(r, F_SETFL, p | O_NONBLOCK) == -1) {
      perror("fcntl(F_SETFL/F_GETFL, O_NONBLOCK) failed");
      exit(EXIT_UNKNOWN);
   }

   len = sizeof(*sndbuf_set);

   if (doreverse) {
      *sndbuf = packetsize + PADBYTES;

      if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, sndbuf, sizeof(*sndbuf)) != 0) {
         perror("setsockopt(SO_SNDBUF)");
         exit(EXIT_UNKNOWN);
      }

      if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, sndbuf_set, &len) != 0) {
         perror("getsockopt(SO_SNDBUF)");
         exit(EXIT_UNKNOWN);
      }

      *rcvbuf = *sndbuf_set * 10;
      if (setsockopt(r, SOL_SOCKET, SO_RCVBUF, rcvbuf, sizeof(*rcvbuf)) != 0) {
         perror("setsockopt(SO_RCVBUF)");
         exit(EXIT_UNKNOWN);
      }
   }
   else {
      *rcvbuf = packetsize + PADBYTES;

      if (setsockopt(r, SOL_SOCKET, SO_RCVBUF, rcvbuf, sizeof(*rcvbuf)) != 0) {
         perror("setsockopt(SO_RCVBUF)");
         exit(EXIT_UNKNOWN);
      }

      if (getsockopt(r, SOL_SOCKET, SO_RCVBUF, rcvbuf_set, &len) != 0) {
         perror("getsockopt(SO_RCVBUF)");
         exit(EXIT_UNKNOWN);
      }

      *sndbuf = *rcvbuf_set * 10;
      if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, sndbuf, sizeof(*sndbuf)) != 0) {
         perror("setsockopt(SO_SNDBUF)");
         exit(EXIT_UNKNOWN);
      }
   }

   if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, sndbuf_set, &len) != 0
   ||  getsockopt(r, SOL_SOCKET, SO_RCVBUF, rcvbuf_set, &len) != 0) {
      perror("getsockopt(SO_SNDBUF/SO_RCVBUF)");
      exit(EXIT_UNKNOWN);
   }

   fprintf(stderr, "sndbuf is %lu, rcvbuf is %lu\n",
          (unsigned long)*sndbuf_set, (unsigned long)*rcvbuf_set);

   if (doreverse) {
      if (*rcvbuf_set < *rcvbuf) {
         fprintf(stderr, "failed to set rcvbuf to %lu.  Is %lu\n",
                 (unsigned long)*rcvbuf, (unsigned long)*rcvbuf_set);
         exit(EXIT_UNKNOWN);
      }
   }
   else {
      if (*sndbuf_set < *sndbuf) {
         fprintf(stderr, "failed to set sndbuf to %lu (is %lu)\n",
                 (unsigned long)*sndbuf, (unsigned long)*sndbuf_set);
         exit(EXIT_UNKNOWN);
      }
   }
}

static size_t
sendtest(const int s, const char *buf, const size_t buflen)
{
   ssize_t rc;
   size_t sent;
   int i;

   i        = 1;
   sent     = 0;
   errno    = 0;

   while (1) {
      if ((rc = write(s, buf, buflen)) == -1)
         break;
      else {
         assert(rc == (ssize_t)buflen);

         ++i;
         sent += rc;
      }
   }

   fprintf(stderr,
          "failed sending packet #%d, sent %ld/%ld.  "
          "Total bytes sent: %lu.  Error on last packet: %s\n",
          i,
          (long)rc,
          (unsigned long)buflen,
          (unsigned long)sent,
          strerror(errno));

   return sent;
}

void
reswrite(const char *res)
{
   FILE *fp;
   if ((fp = fopen("conftest.out", "w")) == NULL) {
      perror("fopen");
      exit(1);
   }
   fprintf(fp, "%s\n", res);
   fclose(fp);
}], [pipeside=`cat conftest.out`
     AC_MSG_RESULT([$pipeside])
], [AC_MSG_RESULT(unknown)],
   [dnl XXX assume no when cross-compiling
    AC_MSG_RESULT(cross-compiling, assuming unknown)])

case $pipeside in
    recvbased)
	AC_DEFINE(HAVE_PIPEBUFFER_RECV_BASED, 1, [platform pipe behavior])
	;;
    sendbased)
	AC_DEFINE(HAVE_PIPEBUFFER_SEND_BASED, 1, [platform pipe behavior])
	;;
    *)
	AC_DEFINE(HAVE_PIPEBUFFER_UNKNOWN, 1, [platform pipe behavior])
        AC_MSG_WARN([unable to determine PIPEBUFFER type])
	;;
esac])

dnl define function for valid size for tcp_ipa socket option
dnl takes size as argument returns true if size accepted
m4_define([check_tcpipasize],
 [AC_MSG_CHECKING([whether TCP_IPA argument size is valid: $1])
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <unistd.h>

int main(void)
{
    struct sockaddr_in addr;
    struct tcp_ipa ipa;
    socklen_t len;
    int s;
    int i;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    for (i = 0; i < $1; i++)
        ipa.ipa_ipaddress[i] = inet_addr("127.0.0.1");

    len = $1 * sizeof(u_int32_t);
    if (setsockopt(s, SOL_TCP, TCP_IPA, &ipa, len) == -1) {
        perror("setsockopt");
        exit(1);
    }

    exit(0);
}], [AC_MSG_RESULT(yes)
     $2],
    [$3
     AC_MSG_RESULT(no)])])

AC_DEFUN([L_CHECK_TCPIPASIZE],
 [check_tcpipasize($@)])

AC_DEFUN([L_UNIBUF],
[AC_MSG_CHECKING(for sufficiently unified buffer cache)
 AC_RUN_IFELSE([AC_LANG_SOURCE([
changequote(<<, >>)dnl

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

#define ELEMENTS(array) (sizeof(array) / sizeof(array[0]))
#define FILENAME        ".tmpfile"
#define TESTITERATIONS  (128)

int
main(void)
{
   FILE *fp;
   size_t testi, i;
   int array[2];

   assert(ELEMENTS(array) % 2 == 0);

   if ((fp = fopen(FILENAME, "w+")) == NULL) {
      fprintf(stderr, "fopen(%s) failed: %s", FILENAME, strerror(errno));
      exit(1);
   }

   if (ftruncate(fileno(fp), (off_t)sizeof(array)) == -1) {
      perror("ftruncate()");
      exit(1);
   }

   fprintf(stderr, "created file %s of size %lu, mmap()'ing it  ...\n",
          FILENAME, (unsigned long)sizeof(array));

   fclose(fp);

   for (testi = 0; testi < TESTITERATIONS; ++testi) {
      /*
       * This test mmap(2)'s a file, moves some of the mmap(2)-ed memory
       * around, unmap(2)s, and then truncate(2)s the file to a smaller size.
       *
       * Afterwards it again mmap(2)'s the same file using the smaller
       * (truncated) size and checks that the contents, up to the smaller
       * truncated size, is correct and the same as it was before the unmap(2).
       *
       * On OpenBSD this for some reason fails and we end up with
       * old data from the previous iteration in the remapped array. :-/
       */
      off_t truncatedsize;
      int *map, beforeunmap[ELEMENTS(array)], afterremap[ELEMENTS(array)];

      /* just to make sure user does not change one but not the other. */
      assert(sizeof(*map) == sizeof(*array));

      for (i = 0; i < ELEMENTS(array); ++i)
         array[i] = (int)random();

      if ((fp = fopen(FILENAME, "r+")) == NULL) {
         fprintf(stderr, "fopen(%s) failed: %s", FILENAME, strerror(errno));
         exit(1);
      }

      if ((map = mmap(NULL,
                      sizeof(array),
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      fileno(fp),
                      (off_t)0)) == MAP_FAILED) {
         perror("mmap()");
         exit(1);
      }

      fclose(fp);

      memcpy(map,         array, sizeof(array));
      memcpy(beforeunmap, map,   sizeof(array));

      assert(memcmp(map,         array, sizeof(array)) == 0);
      assert(memcmp(beforeunmap, array, sizeof(array)) == 0);

      array[0] = array[1]; /* 0xdeadbeef */
      map[0]   = array[1]; /* 0xdeadbeef */

      memcpy(beforeunmap, map, sizeof(array));

      assert(memcmp(map,         array, sizeof(array)) == 0);
      assert(memcmp(beforeunmap, array, sizeof(array)) == 0);

      truncatedsize = (off_t)(sizeof(array) / 2);
      assert(memcmp(map,         array, (size_t)truncatedsize) == 0);
      assert(memcmp(beforeunmap, array, (size_t)truncatedsize) == 0);

#if 0
      /*
       * with a unified buffercache the below msync(2) should not
       * be needed as the subsequent open(2) and mmap(2)-ed should
       * read from the buffercache if the data is still there, or
       * from file if already flushed from cache to disk.
       */

      if (msync(map, truncatedsize, MS_ASYNC) == -1) {
         perror("msync(2)");
         exit(1);
      }
#endif
      if (munmap(map, sizeof(array)) == -1) {
         perror("munmap()");
         exit(1);
      }

      if (truncate(FILENAME, truncatedsize) == -1) {
         perror("truncate()");
         exit(1);
      }

      if ((fp = fopen(FILENAME, "r+")) == NULL) {
         fprintf(stderr, "fopen(%s) failed: %s", FILENAME, strerror(errno));
         exit(1);
      }

      if ((map = mmap(NULL,
                      truncatedsize,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      fileno(fp),
                      (off_t)0)) == MAP_FAILED) {
         perror("mmap()");
         exit(1);
      }

      fclose(fp);

      bzero(afterremap, sizeof(afterremap));
      memcpy(afterremap, map, (size_t)truncatedsize);

      if (beforeunmap[0] != afterremap[0]) {
         fprintf(stderr, "on iteration %lu re-mapped() array index 0 of size %lu "
                "does not match what we unmapped() previously\n",
                (unsigned long)testi + 1,
                (unsigned long)sizeof(beforeunmap[0]));

         exit(1);
      }

      if (memcmp(beforeunmap, afterremap, (size_t)truncatedsize) != 0) {
         fprintf(stderr, "on iteration %lu re-mapped() data of size %lu (%s) "
                "does not match what we unmapped() previously\n",
                (unsigned long)testi + 1,
                (unsigned long)truncatedsize,
                truncatedsize == sizeof(array) ? "not truncated" : "truncated");

         exit(1);
      }

      if (truncate(FILENAME, (size_t)sizeof(array)) == -1) {
         perror("truncate()");
         exit(1);
      }
   }

   fprintf(stderr, "tested through %lu iterations, all ok\n", (unsigned long)testi);

   if (unlink(FILENAME) != 0) {
      perror("unlink()");
      exit(1);
   }

   return 0;
}
changequote([, ])dnl
])], [AC_MSG_RESULT(yes)
       AC_DEFINE(HAVE_UNIFIED_BUFFERCACHE, 1, [unified vm buffers])],
      [AC_MSG_RESULT(no)
       AC_MSG_WARN([assuming buffercache not unified on this platform])],
      [dnl assume no when cross-compiling
       AC_MSG_RESULT(no)
       AC_MSG_WARN([assuming buffercache not unified on this platform])])])

m4_define([tcpinfoval],
 [AC_MSG_CHECKING(for tcp_info value $1)
  AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>], [

struct tcp_info tcp_info;
void *p;

p = &tcp_info.$1;],
 [AC_DEFINE(HAVE_TCP_INFO_$2, 1, [$1 found in tcp_info struct])
  AC_MSG_RESULT(yes)],
 [AC_MSG_RESULT(no)])])

AC_DEFUN([L_TCPINFOCHECK],
 [tcpinfoval($1, m4_toupper($1))])


dnl define function for obtaining highest timeout values supported by select()
AC_DEFUN([L_SELECT_MAXTIMEOUT],
[DEFAULTMAX=33333333 #corresponds roughly to one year
 AC_MSG_CHECKING(for errorless select behavior with high timeouts)
 AC_RUN_IFELSE([AC_LANG_SOURCE([
changequote(<<, >>)dnl
#include <sys/types.h>
#include <sys/time.h>

#include <errno.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

void sigalrm(int signal);
int checkval(long val);
void binsearch(long startval, long endval);

#define MAXVAL LONG_MAX

int resval = -1; /* -1: fail, 0: ok */

int
main(void)
{
   if (checkval(MAXVAL) == 0) {
      fprintf(stderr, "notice: highest acceptable value equal to MAXVAL\n");
      FILE *fp;
      if ((fp = fopen("conftest.out", "w")) == NULL) {
         perror("fopen");
         exit(-1);
      }
      /* write zero for no special handling needed */
      fprintf(fp, "%ld\n", (long)0);
      fclose(fp);
      exit(0);
   }

   binsearch(0, MAXVAL);
}

void
binsearch(long startval, long endval)
{
   long midval = (endval - startval) / 2 + startval;

/*   printf("binsearch: %ld - %ld (%ld)\n", startval, endval, midval);*/

   if (midval == startval || midval == endval) {
      FILE *fp;
      if ((fp = fopen("conftest.out", "w")) == NULL) {
         perror("fopen");
         exit(-1);
      }
      fprintf(fp, "%ld\n", midval);
      fclose(fp);
      exit(0);
   }

   if (checkval(midval) == -1)
      /* value failed, must lie below current value*/
      binsearch(startval, midval);
   else
      /* value succeeded, must lie above current value*/
      binsearch(midval, endval);
}

int
checkval(long val)
{
   struct timeval timeout;
   int rc;

   timeout.tv_sec  = val;
   timeout.tv_usec = 0;

   if (signal(SIGALRM, sigalrm) == SIG_ERR) {
      perror("signal(SIGALRM)");
      exit(1);
   }

   alarm(1);

   resval = -1;
   if ((rc = select(0, NULL, NULL, NULL, &timeout)) == -1) {
      if (errno == EINTR)
         return resval;
      perror("select()");
      return -1;
   }

   if (rc == 0)
      fprintf(stderr, "expected alarm to trigger, not select to timeout ...\n");
   else
      fprintf(stderr, "select(2) returned %d ...very unexpected\n", rc);

   return resval;
}

void
sigalrm(int sig)
{
   resval = 0;
/*   printf("got sig, ok val\n");*/
}
changequote([, ])dnl
])], [MAXVAL=`cat conftest.out`
      if test x"$MAXVAL" = x; then
         AC_MSG_RESULT([test failure])
         AC_MSG_WARN([unable to determine max select value, using default])
         AC_DEFINE_UNQUOTED(HAVE_SELECT_MAXTIMEOUT, $DEFAULTMAX, [max timeout value])dnl
      elif test $MAXVAL -eq 0; then
         AC_MSG_RESULT([yes])
         AC_DEFINE_UNQUOTED(HAVE_SELECT_MAXTIMEOUT, 0, [max timeout value])dnl
      elif test $MAXVAL -lt $DEFAULTMAX; then
         AC_MSG_RESULT([unexpected test result])
         AC_MSG_WARN([unexpectedly low max select value ... using default])
         AC_DEFINE_UNQUOTED(HAVE_SELECT_MAXTIMEOUT, $DEFAULTMAX, [max timeout value])dnl
      else
         AC_MSG_RESULT([no, using max $MAXVAL])
         AC_DEFINE_UNQUOTED(HAVE_SELECT_MAXTIMEOUT, $MAXVAL, [max timeout value])dnl
      fi],
      [AC_MSG_RESULT([unknown])
       AC_MSG_WARN([unable to determine max select value, using default])
       AC_DEFINE_UNQUOTED(HAVE_SELECT_MAXTIMEOUT, $DEFAULTMAX, [max timeout value])dnl],
      [dnl use default when cross-compiling
       AC_MSG_RESULT([unknown])
       AC_MSG_WARN([unable to determine max select value, using default])
       AC_DEFINE_UNQUOTED(HAVE_SELECT_MAXTIMEOUT, $DEFAULTMAX, [max timeout value])dnl])])


dnl test function that tries to determine if a specified errno error exists
m4_define([checkerrno],
 [AC_MSG_CHECKING(for errno symbol $3)
  AC_PREPROC_IFELSE(
       [AC_LANG_PROGRAM([[#include <errno.h>]],
                        [[
#ifdef $3
errnoval: $3
#else
#error "errno value $3 not defined"
#endif]])],
       [AC_MSG_RESULT([OK])
        $1="$$1 $3"
        cat conftest.i | grep errnoval: >>$2],
       [AC_MSG_RESULT([no])])])
AC_DEFUN([L_CHECKERRNO],
 [checkerrno($@)])

dnl test function that tries to determine if a specified getaddrinfo
dnl error exists

dnl test function that tries to determine if a specified errno error exists
m4_define([checkgaierror],
 [AC_MSG_CHECKING(for getaddrinfo() error $3)
  AC_PREPROC_IFELSE(
       [AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>]],
                        [[
#ifdef $3
gaierrval: $3
#else
#error "gai error value $3 not defined"
#endif]])],
       [AC_MSG_RESULT([OK])
        $1="$$1 $3"
        AC_DEFINE(HAVE_ERR_$3, 1, [$3 gai error found])
        cat conftest.i | grep gaierrval: >>$2],
       [AC_MSG_RESULT([no])])])
AC_DEFUN([L_CHECKGAIERROR],
 [checkgaierror($@)])


#L_GETDEFINEDINT
#args:
# 1: define name
# 2: headers to be included
# 3: new define to set value to
AC_DEFUN([L_GETDEFINEDINT], [
   AC_MSG_CHECKING(for $1 value)
   AC_TRY_RUN([
$2

#include <stdio.h>

int
main(void)
{
   FILE *fp;
   int val;

   val = $1;

   if ((fp = fopen("conftest.out", "w")) == NULL) {
      perror("fopen");
      exit(-1);
   }
   fprintf(fp, "%ld\n", val);
   fclose(fp);

   return 0;
}], [ac_cv_definedint=$(cat conftest.out)
     AC_MSG_RESULT([yes])
     AC_DEFINE_UNQUOTED($3, [$ac_cv_definedint], [value of $1])],
    [AC_MSG_RESULT([no])])])

AC_DEFUN([L_TCP_KEEPCNT_MAX], [
AC_MSG_CHECKING(for TCP_KEEPCNT_MAX value)
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>

#if 0

#include "include/monitor.h"

#else

/*
 * XXX this value should be fetched from Dante include/monitor.h.
 */
#define TEST_NE_TCP_KEEPCNT (1000)

#endif

int
main(void)
{
   socklen_t len;
   int val, s, lastgoodvalue;
   int startval;

   if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket()");
      exit(1);
   }


   /*
    * Find maxvalue for TCP_KEEPCNT.  MAX_TCP_KEEPCNT, but not exported to
    * userland on Linux, by michaels@inet.no.
    */
   lastgoodvalue = -1;
   len           = sizeof(val);

   startval = 127;
   for (val = 127; val <= TEST_NE_TCP_KEEPCNT; val += 10)
      if (setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, &val, len) == 0) {
         fprintf(stderr, "good value: %d", val);
         lastgoodvalue = val;
      } else {
         perror("setsockname: TCP_KEEPCNT");
         break;
      }

   /* warn if TEST_NE_TCP_KEEPCNT is too low relative to startval */
   if (val == 127)
      fprintf(stderr, "warning: no values were tested");

   if (lastgoodvalue == -1)
      exit(1);
   else {
      FILE *fp;

      fprintf(stderr, "max value for TCP_KEEPCNT is %d\n", lastgoodvalue);
      if ((fp = fopen("conftest.out", "w")) == NULL) {
         perror("fopen");
         exit(-1);
      }
      fprintf(fp, "%ld\n", lastgoodvalue);
      fclose(fp);
   }

   return 0;
}], [tcp_keepcnt_max=`cat conftest.out`
     AC_MSG_RESULT([${tcp_keepcnt_max}])
],  [unset tcp_keepcnt_max
     AC_MSG_RESULT(unknown)],
    [dnl XXX assume unknown when cross-compiling
     AC_MSG_RESULT(cross-compiling, assuming unknown)])

if test x"${tcp_keepcnt_max}" != x; then
   AC_DEFINE_UNQUOTED(MAX_TCP_KEEPCNT, ${tcp_keepcnt_max}, [max tcp_keepcnt value])dnl
fi])


# -- acinclude end --
