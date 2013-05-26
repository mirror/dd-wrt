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
 * $Id: sf_lite.h,v 1.2 2004/03/06 05:25:58 vlm Exp $
 */

#ifndef __SF_LITE_H__
#define __SF_LITE_H__

	/***********************/
	/*** 1. String lists ***/
	/***********************/

typedef struct {
    char **list;
    size_t	count;
    size_t	listlen;
	size_t	maxlen;
	size_t	*lens;
} slist;

slist	*sinit();			/* Create empty structure */
void	sclear(slist *);	/* Clear elements of filled structure */
void	sfree(slist *);	/* Destroy entire object completely */

/* Add element to the end of list */
int	sadd(slist *, char *toadd);	/* with strdup(toadd) */
int	sadd2(slist *, char *toadd, size_t len); /* With length. */

int	sdel(slist *, size_t num);	/* Delete specified element */
ssize_t sins(slist *, char *, size_t num); /* Insert element before num */

/* Split functions, my aknowledgements to Larry Wall! */
slist *split(char *what, char *delim, int strict); /* creates new (slist *) */
int splitf(slist *sl, char *what, char *delim, int strict);
int split_network(const char *, unsigned int *ip, unsigned int *mask);
int sf_iaton(const char *cp, unsigned int *pin); /* aka inet_aton */

/* Find element */
ssize_t sfind(slist *, const char *);	/* Case-sensitive */

int eq(const char *a, const char *b);	/* Equality test */

#endif	/* __SF_LITE_H__ */
