/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2007 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: rtpp_util.c,v 1.14 2008/12/24 10:31:52 sobomax Exp $
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <math.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtpp_util.h"
#include "rtpp_log.h"


#if (__BYTE_ORDER == __BIG_ENDIAN) || \
    (!defined(__VFP_FP__) && (defined(__arm__) || defined(__thumb__)))
typedef union
{
  double value;
  struct
  {
    u_int32_t msw;
    u_int32_t lsw;
  } parts;
} ieee_double_shape_type;

#else

typedef union
{
  double value;
  struct
  {
    u_int32_t lsw;
    u_int32_t msw;
  } parts;
} ieee_double_shape_type;

#endif


#define EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  ieee_double_shape_type ew_u;					\
  ew_u.value = (d);						\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

long int
lround (double x)
{
  int32_t j0;
  u_int32_t i1, i0;
  long int result;
  int sign;

  EXTRACT_WORDS (i0, i1, x);
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  sign = (i0 & 0x80000000) != 0 ? -1 : 1;
  i0 &= 0xfffff;
  i0 |= 0x100000;

  if (j0 < 20)
    {
      if (j0 < 0)
	return j0 < -1 ? 0 : sign;
      else
	{
	  i0 += 0x80000 >> j0;

	  result = i0 >> (20 - j0);
	}
    }
  else if (j0 < (int32_t) (8 * sizeof (long int)) - 1)
    {
      if (j0 >= 52)
	result = ((long int) i0 << (j0 - 20)) | (i1 << (j0 - 52));
      else
	{
	  u_int32_t j = i1 + (0x80000000 >> (j0 - 20));
	  if (j < i1)
	    ++i0;

	  if (j0 == 20)
	    result = (long int) i0;
	  else
	    result = ((long int) i0 << (j0 - 20)) | (j >> (52 - j0));
	}
    }
  else
    {
      /* The number is too large.  It is left implementation defined
	 what happens.  */
      return (long int) x;
    }

  return sign * result;
}



int
ishostseq(struct sockaddr *ia1, struct sockaddr *ia2)
{

    if (ia1->sa_family != ia2->sa_family)
	return 0;

    switch (ia1->sa_family) {
    case AF_INET:
	return (satosin(ia1)->sin_addr.s_addr ==
	  satosin(ia2)->sin_addr.s_addr);

    case AF_INET6:
	return (memcmp(&satosin6(ia1)->sin6_addr.s6_addr[0],
	  &satosin6(ia2)->sin6_addr.s6_addr[0],
	  sizeof(struct in6_addr)) == 0);

    default:
	break;
    }
    /* Can't happen */
    abort();
}

int
ishostnull(struct sockaddr *ia)
{
    struct in6_addr *ap;

    switch (ia->sa_family) {
    case AF_INET:
	return (satosin(ia)->sin_addr.s_addr == INADDR_ANY);

    case AF_INET6:
	ap = &satosin6(ia)->sin6_addr;
	return ((*(const uint32_t *)(const void *)(&ap->s6_addr[0]) == 0) &&
		(*(const uint32_t *)(const void *)(&ap->s6_addr[4]) == 0) &&
		(*(const uint32_t *)(const void *)(&ap->s6_addr[8]) == 0) &&
		(*(const uint32_t *)(const void *)(&ap->s6_addr[12]) == 0));

    default:
	break;
    }

    abort();
}

char *
addr2char_r(struct sockaddr *ia, char *buf, int size)
{
    void *addr;

    switch (ia->sa_family) {
    case AF_INET:
	addr = &(satosin(ia)->sin_addr);
	break;

    case AF_INET6:
	addr = &(satosin6(ia)->sin6_addr);
	break;

    default:
	abort();
    }

    return (char *)((void *)inet_ntop(ia->sa_family, addr, buf, size));
}

const char *
addr2char(struct sockaddr *ia)
{
    static char buf[256];

    return(addr2char_r(ia, buf, sizeof(buf)));
}

double
getdtime(void)
{
    struct timeval timev;

    if (gettimeofday(&timev, NULL) == -1)
	return -1;

    return timev.tv_sec + ((double)timev.tv_usec) / 1000000.0;
}

void
dtime2ts(double dtime, uint32_t *ts_sec, uint32_t *ts_usec)
{

    *ts_sec = trunc(dtime);
    *ts_usec = round(1000000.0 * (dtime - ((double)*ts_sec)));
}

int
resolve(struct sockaddr *ia, int pf, const char *host,
  const char *servname, int flags)
{
    int n;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = flags;	     /* We create listening sockets */
    hints.ai_family = pf;	       /* Protocol family */
    hints.ai_socktype = SOCK_DGRAM;     /* UDP */

    n = getaddrinfo(host, servname, &hints, &res);
    if (n == 0) {
	/* Use the first socket address returned */
	memcpy(ia, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);
    }

    return n;
}

void
seedrandom(void)
{
    int fd;
    unsigned long junk;
    struct timeval tv;

    fd = open("/dev/urandom", O_RDONLY, 0);
    if (fd >= 0) {
	read(fd, &junk, sizeof(junk));
	close(fd);
    }

    gettimeofday(&tv, NULL);
    srandom((getpid() << 16) ^ tv.tv_sec ^ tv.tv_usec ^ junk);
}

int
drop_privileges(struct cfg *cf)
{

    if (cf->run_gname != NULL) {
	if (setgid(cf->run_gid) != 0) {
	    rtpp_log_ewrite(RTPP_LOG_ERR, cf->glog, "can't set current group ID: %d", cf->run_gid);
	    return -1;
	}
    }
    if (cf->run_uname == NULL)
	return 0;
    if (setuid(cf->run_uid) != 0) {
	rtpp_log_ewrite(RTPP_LOG_ERR, cf->glog, "can't set current user ID: %d", cf->run_uid);
	return -1;
    }
    return 0;
}

uint16_t
rtpp_in_cksum(void *addr, int len)
{
    int nleft, sum;
    uint16_t *w;
    union {
        uint16_t us;
        uint16_t uc[2];
    } last;
    uint16_t answer;

    nleft = len;
    sum = 0;
    w = (uint16_t *)addr;

    /*
     * Our algorithm is simple, using a 32 bit accumulator (sum), we add
     * sequential 16 bit words to it, and at the end, fold back all the
     * carry bits from the top 16 bits into the lower 16 bits.
     */
    while (nleft > 1)  {
	sum += *w++;
	nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1) {
	last.uc[0] = *(uint8_t *)w;
	last.uc[1] = 0;
	sum += last.us;
    }

    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
    sum += (sum >> 16);                     /* add carry */
    answer = ~sum;                          /* truncate to 16 bits */
    return (answer);
}

void
init_port_table(struct cfg *cf)
{
    int i, j;
    uint16_t portnum;

    /* Generate linear table */
    cf->port_table_len = ((cf->port_max - cf->port_min) / 2) + 1;
    portnum = cf->port_min;
    for (i = 0; i < cf->port_table_len; i += 1) {
	cf->port_table[i] = portnum;
	portnum += 2;
    }
#if !defined(SEQUENTAL_PORTS)
    /* Shuffle elements ramdomly */
    for (i = 0; i < cf->port_table_len; i += 1) {
	j = random() % cf->port_table_len;
	portnum = cf->port_table[i];
	cf->port_table[i] = cf->port_table[j];
	cf->port_table[j] = portnum;
    }
#endif
    /* Set the last used element to be the last element */
    cf->port_table_idx = cf->port_table_len - 1;
}

/*
 * Portable strsep(3) implementation, borrowed from FreeBSD. For license
 * and other information see:
 *
 * $FreeBSD: src/lib/libc/string/strsep.c,v 1.6 2007/01/09 00:28:12 imp Exp $
 */
char *
rtpp_strsep(char **stringp, const char *delim)
{
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;

    if ((s = *stringp) == NULL)
	return (NULL);
    for (tok = s;;) {
	c = *s++;
	spanp = delim;
	do {
	    if ((sc = *spanp++) == c) {
		if (c == 0)
		    s = NULL;
		else
		    s[-1] = 0;
		*stringp = s;
		return (tok);
	    }
	} while (sc != 0);
    }
    /* NOTREACHED */
}

/*
 * Portable daemon(3) implementation, borrowed from FreeBSD. For license
 * and other information see:
 *
 * $FreeBSD: src/lib/libc/gen/daemon.c,v 1.8 2007/01/09 00:27:53 imp Exp $
 */
int
rtpp_daemon(int nochdir, int noclose)
{
    struct sigaction osa, sa;
    int fd;
    pid_t newgrp;
    int oerrno;
    int osa_ok;

    /* A SIGHUP may be thrown when the parent exits below. */
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    osa_ok = sigaction(SIGHUP, &sa, &osa);

    switch (fork()) {
    case -1:
        return (-1);
    case 0:
        break;
    default:
        _exit(0);
    }

    newgrp = setsid();
    oerrno = errno;
    if (osa_ok != -1)
        sigaction(SIGHUP, &osa, NULL);

    if (newgrp == -1) {
        errno = oerrno;
        return (-1);
    }

    if (!nochdir)
        (void)chdir("/");

    if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
        (void)dup2(fd, STDIN_FILENO);
        (void)dup2(fd, STDOUT_FILENO);
        (void)dup2(fd, STDERR_FILENO);
        if (fd > 2)
            (void)close(fd);
    }
    return (0);
}

static int8_t hex2char[128] = {
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,   1,   2,   3,   4,   5,   6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int
url_unquote(uint8_t *buf, int len)
{
    int outlen;
    uint8_t *cp;

    outlen = len;
    while (len > 0) {
        cp = memchr(buf, '%', len);
        if (cp == NULL)
            return (outlen);
        if (cp - buf + 2 > len)
            return (-1);
        if (cp[1] > 127 || cp[2] > 127 ||
          hex2char[cp[1]] == -1 || hex2char[cp[2]] == -1)
            return (-1);
        cp[0] = (hex2char[cp[1]] << 4) | hex2char[cp[2]];
        len -= cp - buf + 3;
        if (len > 0)
            memmove(cp + 1, cp + 3, len);
        buf = cp + 1;
        outlen -= 2;
    }
    return (outlen);
}
