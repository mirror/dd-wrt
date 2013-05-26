/*
 * This code was taken from the libstrfunc project,
 * http://www.spelio.net.ru/soft
 * All rights reserved.
 */
/*-
 * Copyright (c) 1999, 2000, 2001 Lev Walkin <vlm@spelio.net.ru>.
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
 * $Id: sf_lite.c,v 1.4 2004/04/17 22:31:40 vlm Exp $
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

#ifdef	HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef	HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef	HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef	HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include "sf_lite.h"

#define	sf_malloc	malloc
#define	sf_calloc	calloc
#define	sf_realloc	realloc

int eq(const char *a, const char *b) { return !strcmp(a, b); }

/* create or initialize empty slist structure */
slist *
sinit() {
	slist *sl;

	sl=(slist *)sf_calloc(1, sizeof(slist));
	if(!sl)
		return NULL;

	sl->listlen = 4;

	sl->list = (char **)sf_malloc(sizeof(char *) * sl->listlen);
	if(!sl->list) {
		free(sl);
		return NULL;
	}

	sl->lens = (size_t *)sf_malloc(sizeof(size_t) * sl->listlen);
	if(!sl->lens) {
		free(sl->list);
		free(sl);
		return NULL;
	}

	*(sl->list) = NULL;
	*(sl->lens) = 0;

	return sl;
};

/* frees any strings inside the slist object */
void
sclear(slist *sl) {

	if(sl == NULL)
		return;

	if(sl->list) {
		while(sl->count--) {
			if(sl->list[sl->count])
				free(sl->list[sl->count]);
		}
		*(sl->list) = NULL;
		free(sl->list);
		sl->list = 0;
	}

	if(sl->lens) {
		free(sl->lens);
	}

	sl->lens = 0;
	sl->count = 0;
	sl->maxlen = 0;
	sl->listlen = 0;
}

void
sfree(slist *sl) {
	if(!sl)
		return;

	if(sl->list) {
		if(sl->count > 0)
			while(sl->count--)
				free(sl->list[sl->count]);
		free(sl->list);
	};

	if(sl->lens)
		free(sl->lens);

	free(sl);
};

/* Basic adding function */
int
_sf_add_internal(slist *s, void *msg, size_t len) {

	if(s->count + 1 >= s->listlen) {
		void *z;
		size_t newlistlen;

		if((newlistlen = s->listlen << 2) == 0)
			newlistlen = 4;

		z = sf_realloc(s->list, sizeof(char *) * newlistlen);
		if(z)
			s->list = (char **)z;
		else
			return -1;

		z = sf_realloc(s->lens, sizeof(size_t) * newlistlen);
		if(z)
			s->lens = (size_t *)z;
		else
			return -1;
		s->listlen = newlistlen;
	}

	s->list[s->count] = msg;
	s->lens[s->count] = len;

	if(len > s->maxlen)
		s->maxlen = len;

	s->count++;
	s->list[s->count] = NULL;
	s->lens[s->count] = 0;

	return 0;
}


/* Add a string to the end of the slist structure */
int
sadd2(slist *s, char *msg, size_t len) {
	char *tmp;

	if(!s || !msg)
		return -1;

	tmp = (char *)sf_malloc(len+1);
	if(!tmp)
		return -1;
	memcpy(tmp, msg, len);
	tmp[len] = 0;

	if(_sf_add_internal(s, tmp, len) == -1) {
		free(tmp);
		return -1;
	}

	return 0;
};

int
sadd(slist *s, char *msg) {
	if(!s || !msg)
		return -1;
	return sadd2(s, msg, strlen(msg));
}


/* delete i'th string from the list */
int
sdel(slist *s, size_t n) {
	if(!s)
		return -1;

	if(n >= s->count)
		return s->count;

	free(s->list[n]);

	for(s->count--; n <= s->count; n++) {
		s->list[n] = s->list[n+1];
		s->lens[n] = s->lens[n+1];
	};

	return s->count;
}

ssize_t
sfind(slist *sl, const char *what) {
	size_t n;
	size_t wlen;
	char cw;

	if(!sl || !sl->count || !what)
		return -1;

	wlen=strlen(what);
	cw = *what;

	for(n=0; n < sl->count; n++) {

		if(sl->lens[n] != wlen)
			continue;

		if(wlen) {
			if(
				(sl->list[n][0] == cw)
				&& !strcmp(sl->list[n], what)
			) return n;
		} else {
			return n;
		};

	};

	return -1;
}

/****************/
/* SPLIT STRING */
/****************/

/* split string and add its contents to the slist structure */
/* Split: full version */
int
splitf(slist *s, char *msg, char *dlm, int strict) {
	char *p = msg;
	char *w = NULL;
	int tokens = 0;
	int dlen;
	char cp, cd;

	if(!s || !p)
		return 0;

	if(!dlm) {
		if(strict & 4)
			strict &= ~4;

		if(strict)
			dlm = ":";
		else
			dlm = " \t\n\r";
	}
	cd = *dlm;
	dlen = strlen(dlm);

	if(strict & 2) {

		for(; (cp = *p); p++) {
			if((cd == cp) && (strncmp(p, dlm, dlen) == 0)) {
				if(w) {
					if(sadd2(s, w, p-w) == -1) {
						while(tokens--)	/* cleanup */
							sdel(s, s->count - 1);
						return -1;
					}
					w = NULL;
					tokens++;
				} else if(strict & 1) {
					if(sadd2(s, "", 0) == -1) {
						while(tokens--)	/* cleanup */
							sdel(s, s->count - 1);
						return -1;
					};
					tokens++;
				}
				p += dlen - 1;
			} else {
				if(!w)
					w = p;
			}
		}

	} else {

		for(; (cp = *p); p++) {
			if((cd == cp) || memchr(dlm, cp, dlen)) {
				if(w) {
					if(sadd2(s, w, p-w) == -1) {
						while(tokens--)
							sdel(s, s->count -1);
						return -1;
					};
					w = NULL;
					tokens++;
				} else if(strict & 1) {
					if(sadd2(s, "", 0) == -1) {
						while(tokens--)
							sdel(s, s->count -1);
						return -1;
					}
					tokens++;
				}
			} else {
				if(!w)
					w = p;
			}
		}

	}

	if(w) {
		sadd2(s, w, p-w);
		tokens++;
	}

	return tokens;
}

slist *
split(char *msg, char *delim, int strict) {
	slist *sl;

	sl = sinit();
	if(!sl)
		return NULL;

	if(splitf(sl, msg, delim, strict) == -1) {
		sfree(sl);
		return NULL;
	}

	return sl;
}


int
split_network(const char *ipmp, unsigned int *ip, unsigned int *mask) {
	unsigned long t;
	char *msk = NULL;
	char *ipm;

	if(!ip || !mask)
		return -1;

	if(!ipmp) {
		*ip = 0;
		*mask = 0;
		return -1;
	};

	t = strlen(ipmp) + 1;
	if(t > 32)	/* Too long address/mask */
		return -1;

	msk = (char *)alloca(t);
	memcpy(msk, ipmp, t);
	ipm = msk;

	if((msk=strchr(ipm, '/')) || (msk = strchr(ipm, ' ')))
		*msk++ = 0;

	if(!sf_iaton(ipm, ip)) {
		*ip = *mask = 0;
		return -1;
	};

	if(!msk) {
		*mask = 0xffffffff;
		return 0;
	};

	t = strchr(msk, '.')?1:0;
	if(t && !sf_iaton(msk, mask)) {
		*ip = *mask = 0;
		return -1;
	};
	if(!t) {
		errno=0;
		if(msk[0] == '0' && msk[1] == 'x') {
			t = strtoul(msk, NULL, 16);
			*mask = htonl(t);
		} else {
			t = strtoul(msk, NULL, 10);
			if((t > 32) || errno) return -1;
			if(!t) *mask = 0;
			else *mask = htonl(0xffffffff << (32 - t));
		}
	};

	*ip = *ip & *mask;

	return 0;
}



/*
 * This file was shamelessly stollen from FreeBSD's inet_addr.c to obtain
 * feature portability with other systems.
 * Cosmetic changes, Copyright (c) 2001, Lev Walkin <vlm@spelio.net.ru>.
 *
 * $Id: sf_lite.c,v 1.4 2004/04/17 22:31:40 vlm Exp $
 */

/*
 * ++Copyright++ 1983, 1990, 1993
 * -
 * Copyright (c) 1983, 1990, 1993
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 	This product includes software developed by the University of
 * 	California, Berkeley and its contributors.
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
 * -
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
 * -
 * --Copyright--
 */

/* 
 * Check whether "cp" is a valid ASCII representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
int
sf_iaton(cp, addr)
	register const char *cp;
	unsigned int *addr;
{
	u_long parts[4];
	u_long val;
	char *c;
	char *endptr;
	int gotend, n;

	c = (char *)cp;
	n = 0;
	/*
	 * Run through the string, grabbing numbers until
	 * the end of the string, or some error
	 */
	gotend = 0;
	while (!gotend) {
		errno = 0;
		val = strtoul(c, &endptr, 0);

		if (errno)	/* Fail completely */
			return (0);
		
		/* 
		 * If the whole string is invalid, endptr will equal
		 * c.. this way we can make sure someone hasn't
		 * gone '.12' or something which would get past
		 * the next check.
		 */
		if (endptr == c)
			return (0);
		parts[n] = val;
		c = endptr;

		/* Check the next character past the previous number's end */
		switch (*c) {
		case '.' :
			/* Make sure we only do 3 dots .. */
			if (n == 3)	/* Whoops. Quit. */
				return (0);
			n++;
			c++;
			break;

		case '\0':
			gotend = 1;
			break;

		default:
			if (isspace((unsigned char)*c)) {
				gotend = 1;
				break;
			} else
				return (0);	/* Invalid character, so fail */
		}

	}

	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */

	switch (n) {
	case 0:				/* a -- 32 bits */
		/*
		 * Nothing is necessary here.  Overflow checking was
		 * already done in strtoul().
		 */
		break;
	case 1:				/* a.b -- 8.24 bits */
		if (val > 0xffffff || parts[0] > 0xff)
			return (0);
		val |= parts[0] << 24;
		break;

	case 2:				/* a.b.c -- 8.8.16 bits */
		if (val > 0xffff || parts[0] > 0xff || parts[1] > 0xff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 3:				/* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff || parts[0] > 0xff || parts[1] > 0xff ||
		    parts[2] > 0xff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}

	if (addr != NULL)
		*addr = htonl(val);
	return (1);
}

