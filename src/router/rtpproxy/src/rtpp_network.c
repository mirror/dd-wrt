/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2009 Sippy Software, Inc., http://www.sippysoft.com
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
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtpp_defines.h"
#include "rtpp_network.h"
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

struct bindaddr_list {
    struct sockaddr_storage *bindaddr;
    struct bindaddr_list *next;
};

struct sockaddr *
addr2bindaddr(struct cfg *cf, struct sockaddr *ia, const char **ep)
{
    struct bindaddr_list *bl;

    pthread_mutex_lock(&cf->bindaddr_lock);
    for (bl = cf->bindaddr_list; bl != NULL; bl = bl->next) {
        if (ishostseq(sstosa(bl->bindaddr), ia) != 0) {
            pthread_mutex_unlock(&cf->bindaddr_lock);
            return (sstosa(bl->bindaddr));
        }
    }
    bl = malloc(sizeof(*bl) + sizeof(*bl->bindaddr));
    if (bl == NULL) {
        pthread_mutex_unlock(&cf->bindaddr_lock);
        *ep = strerror(errno);
        return (NULL);
    }
    bl->bindaddr = (struct sockaddr_storage *)((char *)bl + sizeof(*bl));
    memcpy(bl->bindaddr, ia, SA_LEN(ia));
    bl->next = cf->bindaddr_list;
    cf->bindaddr_list = bl;
    pthread_mutex_unlock(&cf->bindaddr_lock);
    return (sstosa(bl->bindaddr));
}

struct sockaddr *
host2bindaddr(struct cfg *cf, const char *host, int pf, const char **ep)
{
    int n;
    struct sockaddr_storage ia;
    struct sockaddr *rval;

    /*
     * If user specified * then change it to NULL,
     * that will make getaddrinfo to return addr_any socket
     */
    if (host && (strcmp(host, "*") == 0))
        host = NULL;

    if ((n = resolve(sstosa(&ia), pf, host, SERVICE, AI_PASSIVE)) != 0) {
        *ep = gai_strerror(n);
        return (NULL);
    }
    rval = addr2bindaddr(cf, sstosa(&ia), ep);
    return (rval);
}

int
local4remote(struct sockaddr *ra, struct sockaddr_storage *la)
{
    int s, r;
    socklen_t llen;

    s = socket(ra->sa_family, SOCK_DGRAM, 0);
    if (s == -1) {
        return (-1);
    }
    if (connect(s, ra, SA_LEN(ra)) == -1) {
        close(s);
        return (-1);
    }
    llen = sizeof(*la);
    r = getsockname(s, sstosa(la), &llen);
    close(s);
    return (r);
}

int
extractaddr(const char *str, char **begin, char **end, int *pf)
{
    const char *t;
    int tpf;

    if (*str != '[') {
	tpf = AF_INET;
	for (t = str; *str != '\0'; str++) {
	    if (!isdigit(*str) && *str != '.')
		break;
	}
    } else {
	tpf = AF_INET6;
	str++;
	for (t = str; *str != '\0'; str++) {
	    if (!isxdigit(*str) && *str != ':')
		break;
	}
	if (*str != ']')
	    return (-1);
    }
    if (t == str)
	return (-1);
    if (tpf == AF_INET6)
	*end = (char *)(str + 1);
    else
	*end = (char *)str;
    *pf = tpf;
    *begin = (char *)t;
    return(str - t);
}

int
setbindhost(struct sockaddr *ia, int pf, const char *bindhost,
  const char *servname)
{
    int n;

    /*
     * If user specified * then change it to NULL,
     * that will make getaddrinfo to return addr_any socket
     */
    if (bindhost && (strcmp(bindhost, "*") == 0))
	bindhost = NULL;

    if ((n = resolve(ia, pf, bindhost, servname, AI_PASSIVE)) != 0) {
	warnx("setbindhost: %s for %s %s", gai_strerror(n), bindhost, servname);
	return -1;
    }
    return 0;
}
