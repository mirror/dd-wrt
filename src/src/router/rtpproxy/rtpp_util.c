/*
 * Copyright (c) 2003, 2004 Maxim Sobolev
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
 * $Id: rtpp_util.c,v 1.3 2005/07/10 18:11:43 sobomax Exp $
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtpp_util.h"

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
getctime(void)
{
    struct timeval timev;

    if (gettimeofday(&timev, NULL) == -1)
	return -1;

    return timev.tv_sec + ((double)timev.tv_usec) / 1000000.0;
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

    fd = open("/dev/random", O_RDONLY, 0);
    if (fd >= 0) {
	read(fd, &junk, sizeof(junk));
	close(fd);
    }

    gettimeofday(&tv, NULL);
    srandom((getpid() << 16) ^ tv.tv_sec ^ tv.tv_usec ^ junk);
}
