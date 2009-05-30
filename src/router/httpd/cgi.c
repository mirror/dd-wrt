/*
 * CGI helper functions
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: cgi.c,v 1.1 2005/09/26 16:58:15 seg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <shutils.h>		//Added by Dainel(2004-07-26)
#define assert(a)

#if defined(linux)
/* Use SVID search */
#define __USE_GNU
#include <search.h>
#endif

/* CGI hash table */
static struct hsearch_data htab;
static int htab_count;

static void unescape(char *s)
{
	unsigned int c;

	while ((s = strpbrk(s, "%+"))) {
		/* Parse %xx */
		if (*s == '%') {
			sscanf(s + 1, "%02x", &c);
			*s++ = (char)c;
			strncpy(s, s + 2, strlen(s) + 1);
		}
		/* Space is special */
		else if (*s == '+')
			*s++ = ' ';
	}
}

char *get_cgi(char *name)
{
	ENTRY e, *ep;

	if (!htab.table)
		return NULL;

	e.key = name;
	hsearch_r(e, FIND, &ep, &htab);

	return ep ? ep->data : NULL;
}

void set_cgi(char *name, char *value)
{
	ENTRY e, *ep;

	//cprintf("\nIn set_cgi(), name = %s, value = %s\n", name, value);

	if (!htab.table)
		return;

	e.key = name;
	hsearch_r(e, FIND, &ep, &htab);
	if (ep) {
		//cprintf("\nIn set_cgi(), ep = %s\n", ep);
		ep->data = value;
	} else {
		//cprintf("\nIn set_cgi(), ep = %s(NULL)\n", ep);
		e.data = value;
		hsearch_r(e, ENTER, &ep, &htab);
		htab_count++;
	}
	assert(ep);
}

void init_cgi(char *query)
{
	int len, nel;
	char *q, *name, *value;

	htab_count = 0;

	//cprintf("\nIn init_cgi(), query = %s\n", query);

	/* Clear variables */
	if (!query) {
		hdestroy_r(&htab);
		return;
	}

	/* Parse into individual assignments */
	q = query;
	len = strlen(query);
	nel = 1;
	while (strsep(&q, "&;"))
		nel++;
	hcreate_r(nel, &htab);
	//cprintf("\nIn init_cgi(), nel = %d\n", nel);

	for (q = query; q < (query + len);) {
		/* Unescape each assignment */
		unescape(name = value = q);

		/* Skip to next assignment */
		for (q += strlen(q); q < (query + len) && !*q; q++) ;

		/* Assign variable */
		name = strsep(&value, "=");
		if (value)
			set_cgi(name, value);
	}
	//cprintf("\nIn init_cgi(), AFTER PROCESS query = %s\n", query);
}

int count_cgi()
{
	return htab_count;
}
