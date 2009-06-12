/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#define __FORCE_GLIBC
#include <features.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>


#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
static pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;
# define LOCK	__pthread_mutex_lock(&mylock)
# define UNLOCK	__pthread_mutex_unlock(&mylock);
#else
# define LOCK
# define UNLOCK
#endif



#define	MAXALIASES	35
static const char NETDB[] = _PATH_NETWORKS;
static FILE *netf = NULL;
static char line[BUFSIZ+1];
static struct netent net;
static char *net_aliases[MAXALIASES];

int _net_stayopen;

void setnetent(int f)
{
    LOCK;
    if (netf == NULL)
	netf = fopen(NETDB, "r" );
    else
	rewind(netf);
    _net_stayopen |= f;
    UNLOCK;
    return;
}

void endnetent(void)
{
    LOCK;
    if (netf) {
	fclose(netf);
	netf = NULL;
    }
    _net_stayopen = 0;
    UNLOCK;
}

static char * any(register char *cp, char *match)
{
    register char *mp, c;

    while ((c = *cp)) {
	for (mp = match; *mp; mp++)
	    if (*mp == c)
		return (cp);
	cp++;
    }
    return ((char *)0);
}

struct netent * getnetent(void)
{
    char *p;
    register char *cp, **q;

    LOCK;
    if (netf == NULL && (netf = fopen(NETDB, "r" )) == NULL) {
	UNLOCK;
	return (NULL);
    }
again:
    p = fgets(line, BUFSIZ, netf);
    if (p == NULL) {
	UNLOCK;
	return (NULL);
    }
    if (*p == '#')
	goto again;
    cp = any(p, "#\n");
    if (cp == NULL)
	goto again;
    *cp = '\0';
    net.n_name = p;
    cp = any(p, " \t");
    if (cp == NULL)
	goto again;
    *cp++ = '\0';
    while (*cp == ' ' || *cp == '\t')
	cp++;
    p = any(cp, " \t");
    if (p != NULL)
	*p++ = '\0';
    net.n_net = inet_network(cp);
    net.n_addrtype = AF_INET;
    q = net.n_aliases = net_aliases;
    if (p != NULL) 
	cp = p;
    while (cp && *cp) {
	if (*cp == ' ' || *cp == '\t') {
	    cp++;
	    continue;
	}
	if (q < &net_aliases[MAXALIASES - 1])
	    *q++ = cp;
	cp = any(cp, " \t");
	if (cp != NULL)
	    *cp++ = '\0';
    }
    *q = NULL;
    UNLOCK;
    return (&net);
}

