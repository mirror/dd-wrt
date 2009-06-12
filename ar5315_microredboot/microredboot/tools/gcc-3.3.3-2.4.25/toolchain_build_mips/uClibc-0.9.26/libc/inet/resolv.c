/* resolv.c: DNS Resolver
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *                     The Silver Hammer Group, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
*/

/*
 * Portions Copyright (c) 1985, 1993
 *    The Regents of the University of California.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *
 *  5-Oct-2000 W. Greathouse  wgreathouse@smva.com
 *                              Fix memory leak and memory corruption.
 *                              -- Every name resolution resulted in
 *                                 a new parse of resolv.conf and new
 *                                 copy of nameservers allocated by
 *                                 strdup.
 *                              -- Every name resolution resulted in
 *                                 a new read of resolv.conf without
 *                                 resetting index from prior read...
 *                                 resulting in exceeding array bounds.
 *
 *                              Limit nameservers read from resolv.conf
 *
 *                              Add "search" domains from resolv.conf
 *
 *                              Some systems will return a security
 *                              signature along with query answer for
 *                              dynamic DNS entries.
 *                              -- skip/ignore this answer
 *
 *                              Include arpa/nameser.h for defines.
 *
 *                              General cleanup
 *
 * 20-Jun-2001 Michal Moskal <malekith@pld.org.pl>
 *   partial IPv6 support (i.e. gethostbyname2() and resolve_address2()
 *   functions added), IPv6 nameservers are also supported.
 *
 * 6-Oct-2001 Jari Korva <jari.korva@iki.fi>
 *   more IPv6 support (IPv6 support for gethostbyaddr();
 *   address family parameter and improved IPv6 support for get_hosts_byname
 *   and read_etc_hosts; getnameinfo() port from glibc; defined
 *   defined ip6addr_any and in6addr_loopback)
 *
 * 2-Feb-2002 Erik Andersen <andersee@debian.org>
 * Added gethostent(), sethostent(), and endhostent()
 *
 * 17-Aug-2002 Manuel Novoa III <mjn3@codepoet.org>
 *   Fixed __read_etc_hosts_r to return alias list, and modified buffer
 *   allocation accordingly.  See MAX_ALIASES and ALIAS_DIM below.
 *   This fixes the segfault in the Python 2.2.1 socket test.
 *
 * 04-Jan-2003 Jay Kulpinski <jskulpin@berkshire.rr.com>
 *   Fixed __decode_dotted to count the terminating null character
 *   in a host name.
 *
 * 02-Oct-2003 Tony J. White <tjw@tjw.org>
 *   Lifted dn_expand() and dependent ns_name_uncompress(), ns_name_unpack(),
 *   and ns_name_ntop() from glibc 2.3.2 for compatibility with ipsec-tools 
 *   and openldap.
 *
 */

#define __FORCE_GLIBC
#include <features.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <resolv.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/nameser.h>
#include <sys/utsname.h>
#include <sys/un.h>

#define MAX_RECURSE 5
#define REPLY_TIMEOUT 10
#define MAX_RETRIES 3
#define MAX_SERVERS 3
#define MAX_SEARCH 4

#define MAX_ALIASES	5

/* 1:ip + 1:full + MAX_ALIASES:aliases + 1:NULL */
#define 	ALIAS_DIM		(2 + MAX_ALIASES + 1)

#undef DEBUG
/*#define DEBUG*/

#ifdef DEBUG
#define DPRINTF(X,args...) fprintf(stderr, X, ##args)
#else
#define DPRINTF(X,args...)
#endif /* DEBUG */


/* Global stuff (stuff needing to be locked to be thread safe)... */
extern int __nameservers;
extern char * __nameserver[MAX_SERVERS];
extern int __searchdomains;
extern char * __searchdomain[MAX_SEARCH];

#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
extern pthread_mutex_t __resolv_lock;
# define BIGLOCK	__pthread_mutex_lock(&__resolv_lock)
# define BIGUNLOCK	__pthread_mutex_unlock(&__resolv_lock);
#else
# define BIGLOCK
# define BIGUNLOCK
#endif



/* Structs */
struct resolv_header {
	int id;
	int qr,opcode,aa,tc,rd,ra,rcode;
	int qdcount;
	int ancount;
	int nscount;
	int arcount;
};

struct resolv_question {
	char * dotted;
	int qtype;
	int qclass;
};

struct resolv_answer {
	char * dotted;
	int atype;
	int aclass;
	int ttl;
	int rdlength;
	unsigned char * rdata;
	int rdoffset;
};

enum etc_hosts_action {
    GET_HOSTS_BYNAME = 0,
    GETHOSTENT,
    GET_HOSTS_BYADDR,
};

/* function prototypes */
extern int __get_hosts_byname_r(const char * name, int type,
			      struct hostent * result_buf,
			      char * buf, size_t buflen,
			      struct hostent ** result,
			      int * h_errnop);
extern int __get_hosts_byaddr_r(const char * addr, int len, int type,
			      struct hostent * result_buf,
			      char * buf, size_t buflen,
			      struct hostent ** result,
			      int * h_errnop);
extern void __open_etc_hosts(FILE **fp);
extern int __read_etc_hosts_r(FILE *fp, const char * name, int type,
			    enum etc_hosts_action action,
			    struct hostent * result_buf,
			    char * buf, size_t buflen,
			    struct hostent ** result,
			    int * h_errnop);
extern int __dns_lookup(const char * name, int type, int nscount, 
	char ** nsip, unsigned char ** outpacket, struct resolv_answer * a);

extern int __encode_dotted(const char * dotted, unsigned char * dest, int maxlen);
extern int __decode_dotted(const unsigned char * message, int offset, 
	char * dest, int maxlen);
extern int __length_dotted(const unsigned char * message, int offset);
extern int __encode_header(struct resolv_header * h, unsigned char * dest, int maxlen);
extern int __decode_header(unsigned char * data, struct resolv_header * h);
extern int __encode_question(struct resolv_question * q,
	unsigned char * dest, int maxlen);
extern int __decode_question(unsigned char * message, int offset,
	struct resolv_question * q);
extern int __encode_answer(struct resolv_answer * a,
	unsigned char * dest, int maxlen);
extern int __decode_answer(unsigned char * message, int offset,
	struct resolv_answer * a);
extern int __length_question(unsigned char * message, int offset);
extern int __open_nameservers(void);
extern void __close_nameservers(void);
extern int __dn_expand(const u_char *, const u_char *, const u_char *,
	char *, int);
extern int __ns_name_uncompress(const u_char *, const u_char *, 
		const u_char *, char *, size_t);
extern int __ns_name_ntop(const u_char *, char *, size_t);
extern int __ns_name_unpack(const u_char *, const u_char *, const u_char *,
               u_char *, size_t);


#ifdef L_encodeh
int __encode_header(struct resolv_header *h, unsigned char *dest, int maxlen)
{
	if (maxlen < HFIXEDSZ)
		return -1;

	dest[0] = (h->id & 0xff00) >> 8;
	dest[1] = (h->id & 0x00ff) >> 0;
	dest[2] = (h->qr ? 0x80 : 0) |
		((h->opcode & 0x0f) << 3) |
		(h->aa ? 0x04 : 0) | 
		(h->tc ? 0x02 : 0) | 
		(h->rd ? 0x01 : 0);
	dest[3] = (h->ra ? 0x80 : 0) | (h->rcode & 0x0f);
	dest[4] = (h->qdcount & 0xff00) >> 8;
	dest[5] = (h->qdcount & 0x00ff) >> 0;
	dest[6] = (h->ancount & 0xff00) >> 8;
	dest[7] = (h->ancount & 0x00ff) >> 0;
	dest[8] = (h->nscount & 0xff00) >> 8;
	dest[9] = (h->nscount & 0x00ff) >> 0;
	dest[10] = (h->arcount & 0xff00) >> 8;
	dest[11] = (h->arcount & 0x00ff) >> 0;

	return HFIXEDSZ;
}
#endif

#ifdef L_decodeh
int __decode_header(unsigned char *data, struct resolv_header *h)
{
	h->id = (data[0] << 8) | data[1];
	h->qr = (data[2] & 0x80) ? 1 : 0;
	h->opcode = (data[2] >> 3) & 0x0f;
	h->aa = (data[2] & 0x04) ? 1 : 0;
	h->tc = (data[2] & 0x02) ? 1 : 0;
	h->rd = (data[2] & 0x01) ? 1 : 0;
	h->ra = (data[3] & 0x80) ? 1 : 0;
	h->rcode = data[3] & 0x0f;
	h->qdcount = (data[4] << 8) | data[5];
	h->ancount = (data[6] << 8) | data[7];
	h->nscount = (data[8] << 8) | data[9];
	h->arcount = (data[10] << 8) | data[11];

	return HFIXEDSZ;
}
#endif

#ifdef L_encoded
/* Encode a dotted string into nameserver transport-level encoding.
   This routine is fairly dumb, and doesn't attempt to compress
   the data */

int __encode_dotted(const char *dotted, unsigned char *dest, int maxlen)
{
	int used = 0;

	while (dotted && *dotted) {
		char *c = strchr(dotted, '.');
		int l = c ? c - dotted : strlen(dotted);

		if (l >= (maxlen - used - 1))
			return -1;

		dest[used++] = l;
		memcpy(dest + used, dotted, l);
		used += l;

		if (c)
			dotted = c + 1;
		else
			break;
	}

	if (maxlen < 1)
		return -1;

	dest[used++] = 0;

	return used;
}
#endif

#ifdef L_decoded
/* Decode a dotted string from nameserver transport-level encoding.
   This routine understands compressed data. */

int __decode_dotted(const unsigned char *data, int offset,
				  char *dest, int maxlen)
{
	int l;
	int measure = 1;
	int total = 0;
	int used = 0;

	if (!data)
		return -1;

	while ((l=data[offset++])) {
		if (measure)
		    total++;
		if ((l & 0xc0) == (0xc0)) {
			if (measure)
				total++;
			/* compressed item, redirect */
			offset = ((l & 0x3f) << 8) | data[offset];
			measure = 0;
			continue;
		}

		if ((used + l + 1) >= maxlen)
			return -1;

		memcpy(dest + used, data + offset, l);
		offset += l;
		used += l;
		if (measure)
			total += l;

		if (data[offset] != 0)
			dest[used++] = '.';
		else
			dest[used++] = '\0';
	}

	/* The null byte must be counted too */
	if (measure) {
	    total++;
	}

	DPRINTF("Total decode len = %d\n", total);

	return total;
}
#endif

#ifdef L_lengthd

int __length_dotted(const unsigned char *data, int offset)
{
	int orig_offset = offset;
	int l;

	if (!data)
		return -1;

	while ((l = data[offset++])) {

		if ((l & 0xc0) == (0xc0)) {
			offset++;
			break;
		}

		offset += l;
	}

	return offset - orig_offset;
}
#endif

#ifdef L_encodeq
int __encode_question(struct resolv_question *q,
					unsigned char *dest, int maxlen)
{
	int i;

	i = __encode_dotted(q->dotted, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;

	if (maxlen < 4)
		return -1;

	dest[0] = (q->qtype & 0xff00) >> 8;
	dest[1] = (q->qtype & 0x00ff) >> 0;
	dest[2] = (q->qclass & 0xff00) >> 8;
	dest[3] = (q->qclass & 0x00ff) >> 0;

	return i + 4;
}
#endif

#ifdef L_decodeq
int __decode_question(unsigned char *message, int offset,
					struct resolv_question *q)
{
	char temp[256];
	int i;

	i = __decode_dotted(message, offset, temp, sizeof(temp));
	if (i < 0)
		return i;

	offset += i;

	q->dotted = strdup(temp);
	q->qtype = (message[offset + 0] << 8) | message[offset + 1];
	q->qclass = (message[offset + 2] << 8) | message[offset + 3];

	return i + 4;
}
#endif

#ifdef L_lengthq
int __length_question(unsigned char *message, int offset)
{
	int i;

	i = __length_dotted(message, offset);
	if (i < 0)
		return i;

	return i + 4;
}
#endif

#ifdef L_encodea
int __encode_answer(struct resolv_answer *a, unsigned char *dest, int maxlen)
{
	int i;

	i = __encode_dotted(a->dotted, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;

	if (maxlen < (RRFIXEDSZ+a->rdlength))
		return -1;

	*dest++ = (a->atype & 0xff00) >> 8;
	*dest++ = (a->atype & 0x00ff) >> 0;
	*dest++ = (a->aclass & 0xff00) >> 8;
	*dest++ = (a->aclass & 0x00ff) >> 0;
	*dest++ = (a->ttl & 0xff000000) >> 24;
	*dest++ = (a->ttl & 0x00ff0000) >> 16;
	*dest++ = (a->ttl & 0x0000ff00) >> 8;
	*dest++ = (a->ttl & 0x000000ff) >> 0;
	*dest++ = (a->rdlength & 0xff00) >> 8;
	*dest++ = (a->rdlength & 0x00ff) >> 0;
	memcpy(dest, a->rdata, a->rdlength);

	return i + RRFIXEDSZ + a->rdlength;
}
#endif

#ifdef L_decodea
int __decode_answer(unsigned char *message, int offset,
				  struct resolv_answer *a)
{
	char temp[256];
	int i;

	i = __decode_dotted(message, offset, temp, sizeof(temp));
	if (i < 0)
		return i;

	message += offset + i;

	a->dotted = strdup(temp);
	a->atype = (message[0] << 8) | message[1];
	message += 2;
	a->aclass = (message[0] << 8) | message[1];
	message += 2;
	a->ttl = (message[0] << 24) |
		(message[1] << 16) | (message[2] << 8) | (message[3] << 0);
	message += 4;
	a->rdlength = (message[0] << 8) | message[1];
	message += 2;
	a->rdata = message;
	a->rdoffset = offset + i + RRFIXEDSZ;

	DPRINTF("i=%d,rdlength=%d\n", i, a->rdlength);

	return i + RRFIXEDSZ + a->rdlength;
}
#endif

#ifdef L_encodep
int __encode_packet(struct resolv_header *h,
	struct resolv_question **q,
	struct resolv_answer **an,
	struct resolv_answer **ns,
	struct resolv_answer **ar,
	unsigned char *dest, int maxlen)
{
	int i, total = 0;
	int j;

	i = __encode_header(h, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;
	total += i;

	for (j = 0; j < h->qdcount; j++) {
		i = __encode_question(q[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}

	for (j = 0; j < h->ancount; j++) {
		i = __encode_answer(an[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}
	for (j = 0; j < h->nscount; j++) {
		i = __encode_answer(ns[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}
	for (j = 0; j < h->arcount; j++) {
		i = __encode_answer(ar[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}

	return total;
}
#endif

#ifdef L_decodep
int __decode_packet(unsigned char *data, struct resolv_header *h)
{
	return __decode_header(data, h);
}
#endif

#ifdef L_formquery
int __form_query(int id, const char *name, int type, unsigned char *packet,
			   int maxlen)
{
	struct resolv_header h;
	struct resolv_question q;
	int i, j;

	memset(&h, 0, sizeof(h));
	h.id = id;
	h.qdcount = 1;

	q.dotted = (char *) name;
	q.qtype = type;
	q.qclass = C_IN; /* CLASS_IN */

	i = __encode_header(&h, packet, maxlen);
	if (i < 0)
		return i;

	j = __encode_question(&q, packet + i, maxlen - i);
	if (j < 0)
		return j;

	return i + j;
}
#endif

#ifdef L_dnslookup

#ifdef __UCLIBC_HAS_THREADS__
static pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;
# define LOCK	__pthread_mutex_lock(&mylock)
# define UNLOCK	__pthread_mutex_unlock(&mylock);
#else
# define LOCK
# define UNLOCK
#endif

/* Just for the record, having to lock __dns_lookup() just for these two globals
 * is pretty lame.  I think these two variables can probably be de-global-ized, 
 * which should eliminate the need for doing locking here...  Needs a closer 
 * look anyways. */
static int ns=0, id=1;

int __dns_lookup(const char *name, int type, int nscount, char **nsip,
			   unsigned char **outpacket, struct resolv_answer *a)
{
	int i, j, len, fd, pos, rc;
	struct timeval tv;
	fd_set fds;
	struct resolv_header h;
	struct resolv_question q;
	int retries = 0;
	unsigned char * packet = malloc(PACKETSZ);
	char *dns, *lookup = malloc(MAXDNAME);
	int variant = 0;
	struct sockaddr_in sa;
#ifdef __UCLIBC_HAS_IPV6__
	int v6;
	struct sockaddr_in6 sa6;
#endif

	fd = -1;

	if (!packet || !lookup || !nscount)
	    goto fail;

	DPRINTF("Looking up type %d answer for '%s'\n", type, name);

	LOCK;
	ns %= nscount;
	UNLOCK;

	while (retries++ < MAX_RETRIES) {
		if (fd != -1)
			close(fd);

		memset(packet, 0, PACKETSZ);

		memset(&h, 0, sizeof(h));

		/* Mess with globals while under lock */
		LOCK;
		++id;
		id &= 0xffff;
		h.id = id;
		dns = nsip[ns];
		UNLOCK;

		h.qdcount = 1;
		h.rd = 1;

		DPRINTF("encoding header\n", h.rd);

		i = __encode_header(&h, packet, PACKETSZ);
		if (i < 0)
			goto fail;

		strncpy(lookup,name,MAXDNAME);
		BIGLOCK;
		if (variant < __searchdomains && strchr(lookup, '.') == NULL)
		{
		    strncat(lookup,".", MAXDNAME);
		    strncat(lookup,__searchdomain[variant], MAXDNAME);
		}
		BIGUNLOCK;
		DPRINTF("lookup name: %s\n", lookup);
		q.dotted = (char *)lookup;
		q.qtype = type;
		q.qclass = C_IN; /* CLASS_IN */

		j = __encode_question(&q, packet+i, PACKETSZ-i);
		if (j < 0)
			goto fail;

		len = i + j;

		DPRINTF("On try %d, sending query to port %d of machine %s\n",
				retries, NAMESERVER_PORT, dns);

#ifdef __UCLIBC_HAS_IPV6__
		v6 = inet_pton(AF_INET6, dns, &sa6.sin6_addr) > 0;
		fd = socket(v6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
		fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
		if (fd < 0) {
		    continue;
		}

		/* Connect to the UDP socket so that asyncronous errors are returned */		 
#ifdef __UCLIBC_HAS_IPV6__
		if (v6) {
		    sa6.sin6_family = AF_INET6;
		    sa6.sin6_port = htons(NAMESERVER_PORT);
		    /* sa6.sin6_addr is already here */
		    rc = connect(fd, (struct sockaddr *) &sa6, sizeof(sa6));
		} else {
#endif
		    sa.sin_family = AF_INET;
		    sa.sin_port = htons(NAMESERVER_PORT);
		    sa.sin_addr.s_addr = inet_addr(dns);
		    rc = connect(fd, (struct sockaddr *) &sa, sizeof(sa));
#ifdef __UCLIBC_HAS_IPV6__
		}
#endif
		if (rc < 0) {
		    if (errno == ENETUNREACH) {
			/* routing error, presume not transient */
			goto tryall;
		    } else
			/* retry */
			continue;
		}

		DPRINTF("Transmitting packet of length %d, id=%d, qr=%d\n",
				len, h.id, h.qr);

		send(fd, packet, len, 0);

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec = REPLY_TIMEOUT;
		tv.tv_usec = 0;
		if (select(fd + 1, &fds, NULL, NULL, &tv) <= 0) {
		    DPRINTF("Timeout\n");

			/* timed out, so retry send and receive, 
			 * to next nameserver on queue */
			goto again;
		}

		i = recv(fd, packet, 512, 0);
		if (i < HFIXEDSZ) {
			/* too short ! */
			goto again;
		}

		__decode_header(packet, &h);

		DPRINTF("id = %d, qr = %d\n", h.id, h.qr);

		LOCK;
		if ((h.id != id) || (!h.qr)) {
			UNLOCK;
			/* unsolicited */
			goto again;
		}
		UNLOCK;


		DPRINTF("Got response %s\n", "(i think)!");
		DPRINTF("qrcount=%d,ancount=%d,nscount=%d,arcount=%d\n",
				h.qdcount, h.ancount, h.nscount, h.arcount);
		DPRINTF("opcode=%d,aa=%d,tc=%d,rd=%d,ra=%d,rcode=%d\n",
				h.opcode, h.aa, h.tc, h.rd, h.ra, h.rcode);

		if ((h.rcode) || (h.ancount < 1)) {
			/* negative result, not present */
			goto again;
		}

		pos = HFIXEDSZ;

		for (j = 0; j < h.qdcount; j++) {
			DPRINTF("Skipping question %d at %d\n", j, pos);
			i = __length_question(packet, pos);
			DPRINTF("Length of question %d is %d\n", j, i);
			if (i < 0)
				goto again;
			pos += i;
		}
		DPRINTF("Decoding answer at pos %d\n", pos);

		for (j=0;j<h.ancount;j++)
		{
		    i = __decode_answer(packet, pos, a);

		    if (i<0) {
			DPRINTF("failed decode %d\n", i);
			goto again;
		    }
		    /* For all but T_SIG, accept first answer */
		    if (a->atype != T_SIG)
			break;

		    DPRINTF("skipping T_SIG %d\n", i);
		    free(a->dotted);
		    pos += i;
		}

		DPRINTF("Answer name = |%s|\n", a->dotted);
		DPRINTF("Answer type = |%d|\n", a->atype);

		close(fd);

		if (outpacket)
			*outpacket = packet;
		else
			free(packet);
		free(lookup);
		return (0);				/* success! */

	  tryall:
		/* if there are other nameservers, give them a go,
		   otherwise return with error */
		{
		    int sdomains;

		    BIGLOCK;
		    sdomains=__searchdomains;
		    BIGUNLOCK;
		    variant = 0;
		    if (retries >= nscount*(sdomains+1))
			goto fail;
		}

	  again:
		/* if there are searchdomains, try them or fallback as passed */
		{
		    int sdomains;
		    BIGLOCK;
		    sdomains=__searchdomains;
		    BIGUNLOCK;

		    if (variant < sdomains) {
			/* next search */
			variant++;
		    } else {
			/* next server, first search */
			LOCK;
			ns = (ns + 1) % nscount;
			UNLOCK;
			variant = 0;
		    }
		}
	}

fail:
	if (fd != -1)
	    close(fd);
	if (lookup)
	    free(lookup);
	if (packet)
	    free(packet);
	return -1;
}
#endif

#ifdef L_opennameservers

int __nameservers;
char * __nameserver[MAX_SERVERS];
int __searchdomains;
char * __searchdomain[MAX_SEARCH];
#ifdef __UCLIBC_HAS_THREADS__
pthread_mutex_t __resolv_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 *	we currently read formats not quite the same as that on normal
 *	unix systems, we can have a list of nameservers after the keyword.
 */

int __open_nameservers()
{
	FILE *fp;
	int i;
#define RESOLV_ARGS 5
	char szBuffer[128], *p, *argv[RESOLV_ARGS];
	int argc;

	BIGLOCK;
	if (__nameservers > 0) { 
	    BIGUNLOCK;
	    return 0;
	}

	if ((fp = fopen("/etc/resolv.conf", "r")) ||
			(fp = fopen("/etc/config/resolv.conf", "r"))) {

		while (fgets(szBuffer, sizeof(szBuffer), fp) != NULL) {

			for (p = szBuffer; *p && isspace(*p); p++)
				/* skip white space */;
			if (*p == '\0' || *p == '\n' || *p == '#') /* skip comments etc */
				continue;
			argc = 0;
			while (*p && argc < RESOLV_ARGS) {
				argv[argc++] = p;
				while (*p && !isspace(*p) && *p != '\n')
					p++;
				while (*p && (isspace(*p) || *p == '\n')) /* remove spaces */
					*p++ = '\0';
			}

			if (strcmp(argv[0], "nameserver") == 0) {
				for (i = 1; i < argc && __nameservers < MAX_SERVERS; i++) {
					__nameserver[__nameservers++] = strdup(argv[i]);
					DPRINTF("adding nameserver %s\n", argv[i]);
				}
			}

			/* domain and search are mutually exclusive, the last one wins */
			if (strcmp(argv[0],"domain")==0 || strcmp(argv[0],"search")==0) {
				while (__searchdomains > 0) {
					free(__searchdomain[--__searchdomains]);
					__searchdomain[__searchdomains] = NULL;
				}
				for (i=1; i < argc && __searchdomains < MAX_SEARCH; i++) {
					__searchdomain[__searchdomains++] = strdup(argv[i]);
					DPRINTF("adding search %s\n", argv[i]);
				}
			}
		}
		fclose(fp);
	} else {
	    DPRINTF("failed to open %s\n", "resolv.conf");
	}
	DPRINTF("nameservers = %d\n", __nameservers);
	BIGUNLOCK;
	return 0;
}
#endif


#ifdef L_closenameservers

void __close_nameservers(void)
{
	BIGLOCK;
	while (__nameservers > 0) {
		free(__nameserver[--__nameservers]);
		__nameserver[__nameservers] = NULL;
	}
	while (__searchdomains > 0) {
		free(__searchdomain[--__searchdomains]);
		__searchdomain[__searchdomains] = NULL;
	}
	BIGUNLOCK;
}
#endif

#ifdef L_gethostbyname

struct hostent *gethostbyname(const char *name)
{
	static struct hostent h;
	static char buf[sizeof(struct in_addr) +
			sizeof(struct in_addr *)*2 +
			sizeof(char *)*(ALIAS_DIM) + 256/*namebuffer*/ + 32/* margin */];
	struct hostent *hp;

	gethostbyname_r(name, &h, buf, sizeof(buf), &hp, &h_errno);

	return hp;
}
#endif

#ifdef L_gethostbyname2

struct hostent *gethostbyname2(const char *name, int family)
{
#ifndef __UCLIBC_HAS_IPV6__
	return family == AF_INET ? gethostbyname(name) : (struct hostent*)0;
#else /* __UCLIBC_HAS_IPV6__ */
	static struct hostent h;
	static char buf[sizeof(struct in6_addr) +
			sizeof(struct in6_addr *)*2 +
			sizeof(char *)*(ALIAS_DIM) + 256/*namebuffer*/ + 32/* margin */];
	struct hostent *hp;

	gethostbyname2_r(name, family, &h, buf, sizeof(buf), &hp, &h_errno);

	return hp;
#endif /* __UCLIBC_HAS_IPV6__ */
}
#endif

#ifdef L_getnetbyname

struct netent * getnetbyname(const char * name)
{
	return NULL;
}
#endif


#ifdef L_res_init
struct __res_state _res;

int res_init(void)
{
	struct __res_state *rp = &(_res);

	__close_nameservers();
	__open_nameservers();
	rp->retrans = RES_TIMEOUT;
	rp->retry = 4;
	rp->options = RES_INIT;
	rp->id = (u_int) random();
	rp->nsaddr.sin_addr.s_addr = INADDR_ANY;
	rp->nsaddr.sin_family = AF_INET;
	rp->nsaddr.sin_port = htons(NAMESERVER_PORT);
	rp->ndots = 1;
	/** rp->pfcode = 0; **/
	rp->_vcsock = -1;
	/** rp->_flags = 0; **/
	/** rp->qhook = NULL; **/
	/** rp->rhook = NULL; **/
	/** rp->_u._ext.nsinit = 0; **/

	BIGLOCK;
	if(__searchdomains) {
		int i;
		for(i=0; i<__searchdomains; i++) {
			rp->dnsrch[i] = __searchdomain[i];
		}
	}

	if(__nameservers) {
		int i;
		struct in_addr a;
		for(i=0; i<__nameservers; i++) {
			if (inet_aton(__nameserver[i], &a)) {
				rp->nsaddr_list[i].sin_addr = a;
				rp->nsaddr_list[i].sin_family = AF_INET;
				rp->nsaddr_list[i].sin_port = htons(NAMESERVER_PORT);
			}
		}
	}
	rp->nscount = __nameservers;
	BIGUNLOCK;

	return(0);
}

void res_close( void )
{
	return;
}

#endif


#ifdef L_res_query

#ifndef MIN
#define MIN(x, y)	((x) < (y) ? (x) : (y))
#endif

int res_query(const char *dname, int class, int type,
              unsigned char *answer, int anslen)
{
	int i;
	unsigned char * packet = 0;
	struct resolv_answer a;
	int __nameserversXX;
	char ** __nameserverXX;

	__open_nameservers();
	
	if (!dname || class != 1 /* CLASS_IN */)
		return(-1);
		
	memset((char *) &a, '\0', sizeof(a));

	BIGLOCK;
	__nameserversXX=__nameservers;
	__nameserverXX=__nameserver;
	BIGUNLOCK;
	i = __dns_lookup(dname, type, __nameserversXX, __nameserverXX, &packet, &a);
	
	if (i < 0)
		return(-1);
			
	free(a.dotted);
		
	if (a.atype == type) { /* CNAME*/
		if (anslen && answer)
			memcpy(answer, a.rdata, MIN(anslen, a.rdlength));
		if (packet)
			free(packet);
		return(MIN(anslen, a.rdlength));
	}
	if (packet)
		free(packet);
	return 0;
}
#endif

#ifdef L_gethostbyaddr
struct hostent *gethostbyaddr (const void *addr, socklen_t len, int type)
{
	static struct hostent h;
	static char buf[
#ifndef __UCLIBC_HAS_IPV6__
		sizeof(struct in_addr) + sizeof(struct in_addr *)*2 +
#else
		sizeof(struct in6_addr) + sizeof(struct in6_addr *)*2 +
#endif /* __UCLIBC_HAS_IPV6__ */
		sizeof(char *)*(ALIAS_DIM) + 256/*namebuffer*/ + 32/* margin */];
	struct hostent *hp;

	gethostbyaddr_r(addr, len, type, &h, buf, sizeof(buf), &hp, &h_errno);
        
	return hp;
}
#endif


#ifdef L_read_etc_hosts_r

void __open_etc_hosts(FILE **fp)
{
	if ((*fp = fopen("/etc/hosts", "r")) == NULL) {
		*fp = fopen("/etc/config/hosts", "r");
	}
	return;
}

int __read_etc_hosts_r(FILE * fp, const char * name, int type,
		     enum etc_hosts_action action,
		     struct hostent * result_buf,
		     char * buf, size_t buflen,
		     struct hostent ** result,
		     int * h_errnop)
{
	struct in_addr	*in=NULL;
	struct in_addr	**addr_list=NULL;
#ifdef __UCLIBC_HAS_IPV6__
	struct in6_addr	*in6=NULL;
	struct in6_addr	**addr_list6=NULL;
#endif /* __UCLIBC_HAS_IPV6__ */
	char					*cp;
	char					**alias;
	int						aliases, i;
	int		ret=HOST_NOT_FOUND;

	if (buflen < sizeof(char *)*(ALIAS_DIM))
		return ERANGE;
	alias=(char **)buf;
	buf+=sizeof(char **)*(ALIAS_DIM);
	buflen-=sizeof(char **)*(ALIAS_DIM);

	if (action!=GETHOSTENT) {
#ifdef __UCLIBC_HAS_IPV6__
		char *p=buf;
		size_t len=buflen;
#endif /* __UCLIBC_HAS_IPV6__ */
		*h_errnop=NETDB_INTERNAL;
		if (buflen < sizeof(*in))
			return ERANGE;
		in=(struct in_addr*)buf;
		buf+=sizeof(*in);
		buflen-=sizeof(*in);

		if (buflen < sizeof(*addr_list)*2)
			return ERANGE;
		addr_list=(struct in_addr **)buf;
		buf+=sizeof(*addr_list)*2;
		buflen-=sizeof(*addr_list)*2;

#ifdef __UCLIBC_HAS_IPV6__
		if (len < sizeof(*in6))
			return ERANGE;
		in6=(struct in6_addr*)p;
		p+=sizeof(*in6);
		len-=sizeof(*in6);

		if (len < sizeof(*addr_list6)*2)
			return ERANGE;
		addr_list6=(struct in6_addr**)p;
		p+=sizeof(*addr_list6)*2;
		len-=sizeof(*addr_list6)*2;

		if (len < buflen) {
			buflen=len;
			buf=p;
		}
#endif /* __UCLIBC_HAS_IPV6__ */

		if (buflen < 80)
			return ERANGE;

		__open_etc_hosts(&fp);
		if (fp == NULL) {
			result=NULL;
			return errno;
		}
	}

	*h_errnop=HOST_NOT_FOUND;
	while (fgets(buf, buflen, fp)) {
		if ((cp = strchr(buf, '#')))
			*cp = '\0';
		DPRINTF("Looking at: %s\n", buf);
		aliases = 0;

		cp = buf;
		while (*cp) {
			while (*cp && isspace(*cp))
				*cp++ = '\0';
			if (!*cp)
				continue;
			if (aliases < (2+MAX_ALIASES))
				alias[aliases++] = cp;
			while (*cp && !isspace(*cp))
				cp++;
		}
		alias[aliases] = 0;

		if (aliases < 2)
			continue; /* syntax error really */
		
		if (action==GETHOSTENT) {
			/* Return whatever the next entry happens to be. */
			break;
		} else if (action==GET_HOSTS_BYADDR) {
			if (strcmp(name, alias[0]) != 0)
				continue;
		} else {
			/* GET_HOSTS_BYNAME */
			for (i = 1; i < aliases; i++)
				if (strcasecmp(name, alias[i]) == 0)
					break;
			if (i >= aliases)
				continue;
		}

		if (type == AF_INET && inet_pton(AF_INET, alias[0], in) > 0) {
			DPRINTF("Found INET\n");
			addr_list[0] = in;
			addr_list[1] = 0;
			result_buf->h_name = alias[1];
			result_buf->h_addrtype = AF_INET;
			result_buf->h_length = sizeof(*in);
			result_buf->h_addr_list = (char**) addr_list;
			result_buf->h_aliases = alias + 2;
			*result=result_buf;
			ret=NETDB_SUCCESS;
#ifdef __UCLIBC_HAS_IPV6__
        } else if (type == AF_INET6 && inet_pton(AF_INET6, alias[0], in6) > 0) {
			DPRINTF("Found INET6\n");
			addr_list6[0] = in6;
			addr_list6[1] = 0;
			result_buf->h_name = alias[1];
			result_buf->h_addrtype = AF_INET6;
			result_buf->h_length = sizeof(*in6);
			result_buf->h_addr_list = (char**) addr_list6;
			result_buf->h_aliases = alias + 2;
			*result=result_buf;
			ret=NETDB_SUCCESS;
#endif /* __UCLIBC_HAS_IPV6__ */
		} else {
			DPRINTF("Error\n");
			ret=TRY_AGAIN;
			break; /* bad ip address */
        }
        
		if (action!=GETHOSTENT) {
			fclose(fp);
		}
		return ret;
	}
	if (action!=GETHOSTENT) {
		fclose(fp);
	}
	return ret;
}
#endif


#ifdef L_gethostent

#ifdef __UCLIBC_HAS_THREADS__
static pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;
# define LOCK	__pthread_mutex_lock(&mylock)
# define UNLOCK	__pthread_mutex_unlock(&mylock);
#else
# define LOCK
# define UNLOCK
#endif

static int __stay_open;
static FILE * __gethostent_fp;

void endhostent (void)
{
    LOCK;
    __stay_open = 0;
    if (__gethostent_fp) {
	fclose(__gethostent_fp);
    }
    UNLOCK;
}

void sethostent (int stay_open)
{
    LOCK;
    __stay_open = stay_open;
    UNLOCK;
}


struct hostent *gethostent (void)
{
    static struct hostent h;
    static char buf[
#ifndef __UCLIBC_HAS_IPV6__
	    sizeof(struct in_addr) + sizeof(struct in_addr *)*2 +
#else
	    sizeof(struct in6_addr) + sizeof(struct in6_addr *)*2 +
#endif /* __UCLIBC_HAS_IPV6__ */
		sizeof(char *)*(ALIAS_DIM) +
	    80/*namebuffer*/ + 2/* margin */];
    struct hostent *host;

    LOCK;
    if (__gethostent_fp == NULL) {
	__open_etc_hosts(&__gethostent_fp);
	if (__gethostent_fp == NULL) {
	    UNLOCK;
	    return((struct hostent *)NULL);
	}
    }

    __read_etc_hosts_r(__gethostent_fp, NULL, AF_INET, GETHOSTENT, 
		   &h, buf, sizeof(buf), &host, &h_errno);
    if (__stay_open==0) {
	fclose(__gethostent_fp);
    }
    UNLOCK;
    return(host);
}
#endif

#ifdef L_get_hosts_byname_r

int __get_hosts_byname_r(const char * name, int type,
			    struct hostent * result_buf,
			    char * buf, size_t buflen,
			    struct hostent ** result,
			    int * h_errnop)
{
	return(__read_etc_hosts_r(NULL, name, type, GET_HOSTS_BYNAME, result_buf, buf, buflen, result, h_errnop));
}
#endif

#ifdef L_get_hosts_byaddr_r

int __get_hosts_byaddr_r(const char * addr, int len, int type,
			    struct hostent * result_buf,
			    char * buf, size_t buflen,
			    struct hostent ** result,
			    int * h_errnop)
{
#ifndef __UCLIBC_HAS_IPV6__
	char	ipaddr[INET_ADDRSTRLEN];
#else
	char	ipaddr[INET6_ADDRSTRLEN];
#endif /* __UCLIBC_HAS_IPV6__ */

    switch (type) {
	case AF_INET:
		if (len != sizeof(struct in_addr))
			return 0;
		break;
#ifdef __UCLIBC_HAS_IPV6__
	case AF_INET6:
		if (len != sizeof(struct in6_addr))
			return 0;
		break;
#endif /* __UCLIBC_HAS_IPV6__ */
	default:
		return 0;
	}

	inet_ntop(type, addr, ipaddr, sizeof(ipaddr));

	return(__read_etc_hosts_r(NULL, ipaddr, type, GET_HOSTS_BYADDR, 
		    result_buf, buf, buflen, result, h_errnop));
}
#endif

#ifdef L_getnameinfo

#ifndef min
# define min(x,y) (((x) > (y)) ? (y) : (x))
#endif /* min */

int getnameinfo (const struct sockaddr *sa, socklen_t addrlen, char *host,
	     socklen_t hostlen, char *serv, socklen_t servlen,
	     unsigned int flags)
{
	int serrno = errno;
	int ok = 0;
	struct hostent *h = NULL;
    char domain[256];
    
	if (flags & ~(NI_NUMERICHOST|NI_NUMERICSERV|NI_NOFQDN|NI_NAMEREQD|NI_DGRAM))
		return EAI_BADFLAGS;

	if (sa == NULL || addrlen < sizeof (sa_family_t))
		return EAI_FAMILY;

	switch (sa->sa_family) {
	case AF_LOCAL:
		break;
	case AF_INET:
		if (addrlen < sizeof (struct sockaddr_in))
			return EAI_FAMILY;
		break;
#ifdef __UCLIBC_HAS_IPV6__
	case AF_INET6:
		if (addrlen < sizeof (struct sockaddr_in6))
			return EAI_FAMILY;
		break;
#endif /* __UCLIBC_HAS_IPV6__ */
	default:
		return EAI_FAMILY;
	}

	if (host != NULL && hostlen > 0)
		switch (sa->sa_family) {
		case AF_INET:
#ifdef __UCLIBC_HAS_IPV6__
		case AF_INET6:
#endif /* __UCLIBC_HAS_IPV6__ */
			if (!(flags & NI_NUMERICHOST)) {
#ifdef __UCLIBC_HAS_IPV6__
				if (sa->sa_family == AF_INET6)
					h = gethostbyaddr ((const void *) &(((const struct sockaddr_in6 *) sa)->sin6_addr),
						sizeof(struct in6_addr), AF_INET6);
				else
#endif /* __UCLIBC_HAS_IPV6__ */
                    h = gethostbyaddr ((const void *) &(((const struct sockaddr_in *)sa)->sin_addr),
					  sizeof(struct in_addr), AF_INET);

				if (h) {
					char *c;
					if ((flags & NI_NOFQDN)
					    && (getdomainname (domain, sizeof(domain)) == 0)
					    && (c = strstr (h->h_name, domain))
					    && (c != h->h_name) && (*(--c) == '.')) {
						strncpy (host, h->h_name,
							min(hostlen, (size_t) (c - h->h_name)));
						host[min(hostlen - 1, (size_t) (c - h->h_name))] = '\0';
						ok = 1;
					} else {
						strncpy (host, h->h_name, hostlen);
						ok = 1;
					}
				 }
			}

			if (!ok) {
				if (flags & NI_NAMEREQD) {
					errno = serrno;
					return EAI_NONAME;
				} else {
					const char *c;
#ifdef __UCLIBC_HAS_IPV6__
					if (sa->sa_family == AF_INET6) {
						const struct sockaddr_in6 *sin6p;

						sin6p = (const struct sockaddr_in6 *) sa;

						c = inet_ntop (AF_INET6,
							(const void *) &sin6p->sin6_addr, host, hostlen);
#if 0
/* Does scope id need to be supported? */
						uint32_t scopeid;
						scopeid = sin6p->sin6_scope_id;
						if (scopeid != 0) {
							/* Buffer is >= IFNAMSIZ+1.  */
							char scopebuf[IFNAMSIZ + 1];
							char *scopeptr;
							int ni_numericscope = 0;
							size_t real_hostlen = __strnlen (host, hostlen);
							size_t scopelen = 0;

							scopebuf[0] = SCOPE_DELIMITER;
							scopebuf[1] = '\0';
							scopeptr = &scopebuf[1];

							if (IN6_IS_ADDR_LINKLOCAL (&sin6p->sin6_addr)
							    || IN6_IS_ADDR_MC_LINKLOCAL (&sin6p->sin6_addr)) {
								if (if_indextoname (scopeid, scopeptr) == NULL)
									++ni_numericscope;
								else
									scopelen = strlen (scopebuf);
							} else {
								++ni_numericscope;
                            }

							if (ni_numericscope)
								scopelen = 1 + snprintf (scopeptr,
									(scopebuf
									+ sizeof scopebuf
									- scopeptr),
									"%u", scopeid);

							if (real_hostlen + scopelen + 1 > hostlen)
								return EAI_SYSTEM;
							memcpy (host + real_hostlen, scopebuf, scopelen + 1);
						}
#endif
					} else
#endif /* __UCLIBC_HAS_IPV6__ */
						c = inet_ntop (AF_INET,
							(const void *) &(((const struct sockaddr_in *) sa)->sin_addr),
							host, hostlen);

					if (c == NULL) {
						errno = serrno;
						return EAI_SYSTEM;
					}
				}
				ok = 1;
			}
			break;

		case AF_LOCAL:
			if (!(flags & NI_NUMERICHOST)) {
				struct utsname utsname;

				if (!uname (&utsname)) {
					strncpy (host, utsname.nodename, hostlen);
					break;
				};
			};

			if (flags & NI_NAMEREQD) {
				errno = serrno;
				return EAI_NONAME;
			}

			strncpy (host, "localhost", hostlen);
			break;

		default:
			return EAI_FAMILY;
	}

	if (serv && (servlen > 0)) {
		switch (sa->sa_family) {
		case AF_INET:
#ifdef __UCLIBC_HAS_IPV6__
		case AF_INET6:
#endif /* __UCLIBC_HAS_IPV6__ */
			if (!(flags & NI_NUMERICSERV)) {
				struct servent *s;
				s = getservbyport (((const struct sockaddr_in *) sa)->sin_port,
				      ((flags & NI_DGRAM) ? "udp" : "tcp"));
				if (s) {
					strncpy (serv, s->s_name, servlen);
					break;
				}
			}
			snprintf (serv, servlen, "%d",
				ntohs (((const struct sockaddr_in *) sa)->sin_port));
			break;

		case AF_LOCAL:
			strncpy (serv, ((const struct sockaddr_un *) sa)->sun_path, servlen);
			break;
		}
	}
	if (host && (hostlen > 0))
		host[hostlen-1] = 0;
	if (serv && (servlen > 0))
		serv[servlen-1] = 0;
	errno = serrno;
	return 0;
}
#endif


#ifdef L_gethostbyname_r

int gethostbyname_r(const char * name,
			    struct hostent * result_buf,
			    char * buf, size_t buflen,
			    struct hostent ** result,
			    int * h_errnop)
{
	struct in_addr *in;
	struct in_addr **addr_list;
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int nest = 0;
	int __nameserversXX;
	char ** __nameserverXX;

	__open_nameservers();

	*result=NULL;
	if (!name)
		return EINVAL;

	/* do /etc/hosts first */
	if ((i=__get_hosts_byname_r(name, AF_INET, result_buf,
				  buf, buflen, result, h_errnop))==0)
		return i;
	switch (*h_errnop) {
		case HOST_NOT_FOUND:
		case NO_ADDRESS:
			break;
		case NETDB_INTERNAL:
			if (errno == ENOENT) {
			    break;
			}
			/* else fall through */
		default:
			return i;
	}

	DPRINTF("Nothing found in /etc/hosts\n");

	*h_errnop = NETDB_INTERNAL;
	if (buflen < sizeof(*in))
		return ERANGE;
	in=(struct in_addr*)buf;
	buf+=sizeof(*in);
	buflen-=sizeof(*in);

	if (buflen < sizeof(*addr_list)*2)
		return ERANGE;
	addr_list=(struct in_addr**)buf;
	buf+=sizeof(*addr_list)*2;
	buflen-=sizeof(*addr_list)*2;

	addr_list[0] = in;
	addr_list[1] = 0;
	
	if (buflen<256)
		return ERANGE;
	strncpy(buf, name, buflen);

	/* First check if this is already an address */
	if (inet_aton(name, in)) {
	    result_buf->h_name = buf;
	    result_buf->h_addrtype = AF_INET;
	    result_buf->h_length = sizeof(*in);
	    result_buf->h_addr_list = (char **) addr_list;
	    *result=result_buf;
	    *h_errnop = NETDB_SUCCESS;
	    return NETDB_SUCCESS;
	}

	for (;;) {

	BIGLOCK;
	__nameserversXX=__nameservers;
	__nameserverXX=__nameserver;
	BIGUNLOCK;
		i = __dns_lookup(buf, T_A, __nameserversXX, __nameserverXX, &packet, &a);

		if (i < 0) {
			*h_errnop = HOST_NOT_FOUND;
			DPRINTF("__dns_lookup\n");
			return TRY_AGAIN;
		}

		strncpy(buf, a.dotted, buflen);
		free(a.dotted);

		if (a.atype == T_CNAME) {		/* CNAME */
			DPRINTF("Got a CNAME in gethostbyname()\n");
			i = __decode_dotted(packet, a.rdoffset, buf, buflen);
			free(packet);

			if (i < 0) {
				*h_errnop = NO_RECOVERY;
				DPRINTF("__decode_dotted\n");
				return -1;
			}
			if (++nest > MAX_RECURSE) {
				*h_errnop = NO_RECOVERY;
				DPRINTF("recursion\n");
				return -1;
			}
			continue;
		} else if (a.atype == T_A) {	/* ADDRESS */
			memcpy(in, a.rdata, sizeof(*in));
			result_buf->h_name = buf;
			result_buf->h_addrtype = AF_INET;
			result_buf->h_length = sizeof(*in);
			result_buf->h_addr_list = (char **) addr_list;
			free(packet);
			break;
		} else {
			free(packet);
			*h_errnop=HOST_NOT_FOUND;
			return TRY_AGAIN;
		}
	}

	*result=result_buf;
	*h_errnop = NETDB_SUCCESS;
	return NETDB_SUCCESS;
}
#endif

#ifdef L_gethostbyname2_r

int gethostbyname2_r(const char *name, int family,
			    struct hostent * result_buf,
			    char * buf, size_t buflen,
			    struct hostent ** result,
			    int * h_errnop)
{
#ifndef __UCLIBC_HAS_IPV6__
	return family == AF_INET ? gethostbyname_r(name, result_buf, buf, buflen, result, h_errnop) : HOST_NOT_FOUND;
#else /* __UCLIBC_HAS_IPV6__ */
	struct in6_addr *in;
	struct in6_addr **addr_list;
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int nest = 0;
	int __nameserversXX;
	char ** __nameserverXX;

	if (family == AF_INET)
		return gethostbyname_r(name, result_buf, buf, buflen, result, h_errnop);
		
	if (family != AF_INET6)
		return EINVAL;
		
	__open_nameservers();

	*result=NULL;
	if (!name)
		return EINVAL;

	/* do /etc/hosts first */
	if ((i=__get_hosts_byname_r(name, family, result_buf,
				  buf, buflen, result, h_errnop))==0)
		return i;
	switch (*h_errnop) {
		case HOST_NOT_FOUND:
		case NO_ADDRESS:
			break;
		default:
			return i;
	}

	DPRINTF("Nothing found in /etc/hosts\n");

	*h_errnop = NETDB_INTERNAL;
	if (buflen < sizeof(*in))
		return ERANGE;
	in=(struct in6_addr*)buf;
	buf+=sizeof(*in);
	buflen-=sizeof(*in);

	if (buflen < sizeof(*addr_list)*2)
		return ERANGE;
	addr_list=(struct in6_addr**)buf;
	buf+=sizeof(*addr_list)*2;
	buflen-=sizeof(*addr_list)*2;

	addr_list[0] = in;
	addr_list[1] = 0;
	
	if (buflen<256)
		return ERANGE;
	strncpy(buf, name, buflen);

	/* First check if this is already an address */
	if (inet_pton(AF_INET6, name, in)) {
	    result_buf->h_name = buf;
	    result_buf->h_addrtype = AF_INET6;
	    result_buf->h_length = sizeof(*in);
	    result_buf->h_addr_list = (char **) addr_list;
	    *result=result_buf;
	    *h_errnop = NETDB_SUCCESS;
	    return NETDB_SUCCESS;
	}

	for (;;) {
	BIGLOCK;
	__nameserversXX=__nameservers;
	__nameserverXX=__nameserver;
	BIGUNLOCK;

		i = __dns_lookup(buf, T_AAAA, __nameserversXX, __nameserverXX, &packet, &a);

		if (i < 0) {
			*h_errnop = HOST_NOT_FOUND;
			return TRY_AGAIN;
		}

		strncpy(buf, a.dotted, buflen);
		free(a.dotted);

		if (a.atype == T_CNAME) {		/* CNAME */
			DPRINTF("Got a CNAME in gethostbyname()\n");
			i = __decode_dotted(packet, a.rdoffset, buf, buflen);
			free(packet);

			if (i < 0) {
				*h_errnop = NO_RECOVERY;
				return -1;
			}
			if (++nest > MAX_RECURSE) {
				*h_errnop = NO_RECOVERY;
				return -1;
			}
			continue;
		} else if (a.atype == T_AAAA) {	/* ADDRESS */
			memcpy(in, a.rdata, sizeof(*in));
			result_buf->h_name = buf;
			result_buf->h_addrtype = AF_INET6;
			result_buf->h_length = sizeof(*in);
			result_buf->h_addr_list = (char **) addr_list;
			free(packet);
			break;
		} else {
			free(packet);
			*h_errnop=HOST_NOT_FOUND;
			return TRY_AGAIN;
		}
	}

	*result=result_buf;
	*h_errnop = NETDB_SUCCESS;
	return NETDB_SUCCESS;
#endif /* __UCLIBC_HAS_IPV6__ */
}
#endif

#ifdef L_gethostbyaddr_r
int gethostbyaddr_r (const void *addr, socklen_t len, int type,
			    struct hostent * result_buf,
			    char * buf, size_t buflen,
			    struct hostent ** result,
			    int * h_errnop)

{
	struct in_addr *in;
	struct in_addr **addr_list;
#ifdef __UCLIBC_HAS_IPV6__
	char *qp;
	size_t plen;
	struct in6_addr	*in6;
	struct in6_addr	**addr_list6;
#endif /* __UCLIBC_HAS_IPV6__ */
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int nest = 0;
	int __nameserversXX;
	char ** __nameserverXX;

	*result=NULL;
	if (!addr)
		return EINVAL;
        
	switch (type) {
		case AF_INET:
			if (len != sizeof(struct in_addr))
				return EINVAL;
			break;
#ifdef __UCLIBC_HAS_IPV6__
		case AF_INET6:
			if (len != sizeof(struct in6_addr))
				return EINVAL;
			break;
#endif /* __UCLIBC_HAS_IPV6__ */
		default:
			return EINVAL;
	}

	/* do /etc/hosts first */
	if ((i=__get_hosts_byaddr_r(addr, len, type, result_buf,
				  buf, buflen, result, h_errnop))==0)
		return i;
	switch (*h_errnop) {
		case HOST_NOT_FOUND:
		case NO_ADDRESS:
			break;
		default:
			return i;
	}

	__open_nameservers();

#ifdef __UCLIBC_HAS_IPV6__
	qp=buf;
	plen=buflen;
#endif /* __UCLIBC_HAS_IPV6__ */

	*h_errnop = NETDB_INTERNAL;
	if (buflen < sizeof(*in))
		return ERANGE;
	in=(struct in_addr*)buf;
	buf+=sizeof(*in);
	buflen-=sizeof(*in);

	if (buflen < sizeof(*addr_list)*2)
		return ERANGE;
	addr_list=(struct in_addr**)buf;
	buf+=sizeof(*addr_list)*2;
	buflen-=sizeof(*addr_list)*2;

#ifdef __UCLIBC_HAS_IPV6__
	if (plen < sizeof(*in6))
		return ERANGE;
	in6=(struct in6_addr*)qp;
	qp+=sizeof(*in6);
	plen-=sizeof(*in6);

	if (plen < sizeof(*addr_list6)*2)
		return ERANGE;
	addr_list6=(struct in6_addr**)qp;
	qp+=sizeof(*addr_list6)*2;
	plen-=sizeof(*addr_list6)*2;

	if (plen < buflen) {
		buflen=plen;
		buf=qp;
	}
#endif /* __UCLIBC_HAS_IPV6__ */

	if (buflen<256)
		return ERANGE;

	if(type == AF_INET) {
		unsigned char *tmp_addr = (unsigned char *)addr;

		memcpy(&in->s_addr, addr, len);

		addr_list[0] = in;

		sprintf(buf, "%u.%u.%u.%u.in-addr.arpa",
			tmp_addr[3], tmp_addr[2], tmp_addr[1], tmp_addr[0]);
#ifdef __UCLIBC_HAS_IPV6__
	} else {
		memcpy(in6->s6_addr, addr, len);

		addr_list6[0] = in6;
		qp = buf;

		for (i = len - 1; i >= 0; i--) {
			qp += sprintf(qp, "%x.%x.", in6->s6_addr[i] & 0xf,
				(in6->s6_addr[i] >> 4) & 0xf);
    	}
    	strcpy(qp, "ip6.int");
#endif /* __UCLIBC_HAS_IPV6__ */
	}

	addr_list[1] = 0;

	for (;;) {

	BIGLOCK;
	__nameserversXX=__nameservers;
	__nameserverXX=__nameserver;
	BIGUNLOCK;
		i = __dns_lookup(buf, T_PTR, __nameserversXX, __nameserverXX, &packet, &a);

		if (i < 0) {
			*h_errnop = HOST_NOT_FOUND;
			return TRY_AGAIN;
		}

		strncpy(buf, a.dotted, buflen);
		free(a.dotted);

		if (a.atype == T_CNAME) {		/* CNAME */
			DPRINTF("Got a CNAME in gethostbyaddr()\n");
			i = __decode_dotted(packet, a.rdoffset, buf, buflen);
			free(packet);

			if (i < 0) {
				*h_errnop = NO_RECOVERY;
				return -1;
			}
			if (++nest > MAX_RECURSE) {
				*h_errnop = NO_RECOVERY;
				return -1;
			}
			continue;
		} else if (a.atype == T_PTR) {	/* ADDRESS */
			i = __decode_dotted(packet, a.rdoffset, buf, buflen);
			free(packet);

			result_buf->h_name = buf;
			result_buf->h_addrtype = type;

			if(type == AF_INET) {
				result_buf->h_length = sizeof(*in);
#ifdef __UCLIBC_HAS_IPV6__
			} else {
				result_buf->h_length = sizeof(*in6);
#endif /* __UCLIBC_HAS_IPV6__ */
    		}

			result_buf->h_addr_list = (char **) addr_list;
			break;
		} else {
			free(packet);
			*h_errnop = NO_ADDRESS;
			return TRY_AGAIN;
		}
	}

	*result=result_buf;
	*h_errnop = NETDB_SUCCESS;
	return NETDB_SUCCESS;
}
#endif

#ifdef L_res_comp
/*
 * Expand compressed domain name 'comp_dn' to full domain name.
 * 'msg' is a pointer to the begining of the message,
 * 'eomorig' points to the first location after the message,
 * 'exp_dn' is a pointer to a buffer of size 'length' for the result.
 * Return size of compressed name or -1 if there was an error.
 */
int __dn_expand(const u_char *msg, const u_char *eom, const u_char *src,
          char *dst, int dstsiz)
{
	int n = ns_name_uncompress(msg, eom, src, dst, (size_t)dstsiz);

	if (n > 0 && dst[0] == '.')
		dst[0] = '\0';
	return (n);
}
#endif /* L_res_comp */

#ifdef L_ns_name
/*
 * printable(ch)
 *      Thinking in noninternationalized USASCII (per the DNS spec),
 *      is this character visible and not a space when printed ?
 * return:
 *      boolean.
 */
static int printable(int ch) 
{
        return (ch > 0x20 && ch < 0x7f);
}

/*
 * special(ch)
 *      Thinking in noninternationalized USASCII (per the DNS spec),
 *      is this characted special ("in need of quoting") ?
 * return:
 *      boolean.
 */
static int special(int ch) 
{
        switch (ch) {
        case 0x22: /* '"' */
        case 0x2E: /* '.' */
        case 0x3B: /* ';' */
        case 0x5C: /* '\\' */
        /* Special modifiers in zone files. */
        case 0x40: /* '@' */
        case 0x24: /* '$' */
                return (1);
        default:
                return (0);
        }
}

/*
 * ns_name_uncompress(msg, eom, src, dst, dstsiz)
 *      Expand compressed domain name to presentation format.
 * return:
 *      Number of bytes read out of `src', or -1 (with errno set).
 * note:
 *      Root domain returns as "." not "".
 */
int __ns_name_uncompress(const u_char *msg, const u_char *eom, 
		const u_char *src, char *dst, size_t dstsiz)
{
	u_char tmp[NS_MAXCDNAME];
	int n;

	if ((n = ns_name_unpack(msg, eom, src, tmp, sizeof tmp)) == -1)
		return (-1);
	if (ns_name_ntop(tmp, dst, dstsiz) == -1)
		return (-1);
	return (n);
}


/*
 * ns_name_ntop(src, dst, dstsiz)
 *      Convert an encoded domain name to printable ascii as per RFC1035.
 * return:
 *      Number of bytes written to buffer, or -1 (with errno set)
 * notes:
 *      The root is returned as "."
 *      All other domains are returned in non absolute form
 */
int __ns_name_ntop(const u_char *src, char *dst, size_t dstsiz) {
	const u_char *cp;
	char *dn, *eom;
	u_char c;
	u_int n;
	const char digits[] = "0123456789";

	cp = src;
	dn = dst;
	eom = dst + dstsiz;

	while ((n = *cp++) != 0) {
		if ((n & NS_CMPRSFLGS) != 0) {
			/* Some kind of compression pointer. */
			__set_errno (EMSGSIZE);
			return (-1);
		}
		if (dn != dst) {
			if (dn >= eom) {
				__set_errno (EMSGSIZE);
				return (-1);
			}
			*dn++ = '.';
		}
		if (dn + n >= eom) {
			__set_errno (EMSGSIZE);
			return (-1);
		}
		for ((void)NULL; n > 0; n--) {
			c = *cp++;
			if (special(c)) {
				if (dn + 1 >= eom) {
					__set_errno (EMSGSIZE);
					return (-1);
				}
				*dn++ = '\\';
				*dn++ = (char)c;
			} else if (!printable(c)) {
				if (dn + 3 >= eom) {
					__set_errno (EMSGSIZE);
					return (-1);
				}
				*dn++ = '\\';
				*dn++ = digits[c / 100];
				*dn++ = digits[(c % 100) / 10];
				*dn++ = digits[c % 10];
			} else {
				if (dn >= eom) {
					__set_errno (EMSGSIZE);
					return (-1);
				}
				*dn++ = (char)c;
			}
		}
	}
	if (dn == dst) {
		if (dn >= eom) {
			__set_errno (EMSGSIZE);
			return (-1);
		}
		*dn++ = '.';
	}
	if (dn >= eom) {
		__set_errno (EMSGSIZE);
		return (-1);
	}
	*dn++ = '\0';
        return (dn - dst);
}

/*
 * ns_name_unpack(msg, eom, src, dst, dstsiz)
 *      Unpack a domain name from a message, source may be compressed.
 * return:
 *      -1 if it fails, or consumed octets if it succeeds.
 */
int __ns_name_unpack(const u_char *msg, const u_char *eom, const u_char *src,
               u_char *dst, size_t dstsiz)
{
	const u_char *srcp, *dstlim;
	u_char *dstp;
	int n, len, checked;

	len = -1;
	checked = 0;
	dstp = dst;
	srcp = src;
	dstlim = dst + dstsiz;
	if (srcp < msg || srcp >= eom) {
		__set_errno (EMSGSIZE);
		return (-1);
	}
	/* Fetch next label in domain name. */
	while ((n = *srcp++) != 0) {
		/* Check for indirection. */
		switch (n & NS_CMPRSFLGS) {
		case 0:
			/* Limit checks. */
			if (dstp + n + 1 >= dstlim || srcp + n >= eom) {
				__set_errno (EMSGSIZE);
				return (-1);
			}
			checked += n + 1;
			*dstp++ = n;
			memcpy(dstp, srcp, n);
			dstp += n;
			srcp += n;
			break;

		case NS_CMPRSFLGS:
			if (srcp >= eom) {
				__set_errno (EMSGSIZE);
				return (-1);
			}
			if (len < 0)
				len = srcp - src + 1;
			srcp = msg + (((n & 0x3f) << 8) | (*srcp & 0xff));
			if (srcp < msg || srcp >= eom) {  /* Out of range. */
				__set_errno (EMSGSIZE);
				return (-1);
			}
			checked += 2;
			/*
			 * Check for loops in the compressed name;
			 * if we've looked at the whole message,
			 * there must be a loop.
			 */
			if (checked >= eom - msg) {
				__set_errno (EMSGSIZE);
				return (-1);
			}
			break;

		default:
			__set_errno (EMSGSIZE);
			return (-1);                    /* flag error */
		}
	}
	*dstp = '\0';
	if (len < 0)
		len = srcp - src;
	return (len);
}
#endif /* L_ns_name */
