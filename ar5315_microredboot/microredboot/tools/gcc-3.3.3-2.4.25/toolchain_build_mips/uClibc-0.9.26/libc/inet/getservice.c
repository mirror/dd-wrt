/*
** services.c                           /etc/services access functions
**
** This file is part of the NYS Library.
**
** The NYS Library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version.
**
** The NYS Library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with the NYS Library; see the file COPYING.LIB.  If
** not, write to the Free Software Foundation, Inc., 675 Mass Ave,
** Cambridge, MA 02139, USA.
**
**
** Copyright (c) 1983 Regents of the University of California.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. All advertising materials mentioning features or use of this software
**    must display the following acknowledgement:
**	This product includes software developed by the University of
**	California, Berkeley and its contributors.
** 4. Neither the name of the University nor the names of its contributors
**    may be used to endorse or promote products derived from this software
**    without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/


#define __FORCE_GLIBC
#define _GNU_SOURCE
#include <features.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>



#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
static pthread_mutex_t mylock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
# define LOCK	__pthread_mutex_lock(&mylock)
# define UNLOCK	__pthread_mutex_unlock(&mylock);
#else
# define LOCK
# define UNLOCK
#endif




#define	MAXALIASES	35

static FILE *servf = NULL;
static struct servent serv;
static char buf[BUFSIZ+1 + sizeof(char *)*MAXALIASES];
static int serv_stayopen;

void setservent(int f)
{
    LOCK;
    if (servf == NULL)
	servf = fopen(_PATH_SERVICES, "r" );
    else
	rewind(servf);
    serv_stayopen |= f;
    UNLOCK;
}

void endservent(void)
{
    LOCK;
    if (servf) {
	fclose(servf);
	servf = NULL;
    }
    serv_stayopen = 0;
    UNLOCK;
}

struct servent * getservent(void)
{
    struct servent *result;
    getservent_r(&serv, buf, sizeof(buf), &result);
    return result;
}


struct servent *getservbyname(const char *name, const char *proto)
{
    struct servent *result;
    getservbyname_r(name, proto, &serv, buf, sizeof(buf), &result);
    return result;
}


struct servent * getservbyport(int port, const char *proto)
{
    struct servent *result;
    getservbyport_r(port, proto, &serv, buf, sizeof(buf), &result);
    return result;
}

int getservent_r(struct servent * result_buf,
		 char * buf, size_t buflen,
		 struct servent ** result)
{
    char *p;
    register char *cp, **q;
    char **serv_aliases;
    char *line;

    *result=NULL;

    if (buflen < sizeof(*serv_aliases)*MAXALIASES) {
	errno=ERANGE;
	return errno;
    }
    LOCK;
    serv_aliases=(char **)buf;
    buf+=sizeof(*serv_aliases)*MAXALIASES;
    buflen-=sizeof(*serv_aliases)*MAXALIASES;

    if (buflen < BUFSIZ+1) {
	UNLOCK;
	errno=ERANGE;
	return errno;
    }
    line=buf;
    buf+=BUFSIZ+1;
    buflen-=BUFSIZ+1;

    if (servf == NULL && (servf = fopen(_PATH_SERVICES, "r" )) == NULL) {
	UNLOCK;
	errno=EIO;
	return errno;
    }
again:
    if ((p = fgets(line, BUFSIZ, servf)) == NULL) {
	UNLOCK;
	errno=EIO;
	return errno;
    }
    if (*p == '#')
	goto again;
    cp = strpbrk(p, "#\n");
    if (cp == NULL)
	goto again;
    *cp = '\0';
    result_buf->s_name = p;
    p = strpbrk(p, " \t");
    if (p == NULL)
	goto again;
    *p++ = '\0';
    while (*p == ' ' || *p == '\t')
	p++;
    cp = strpbrk(p, ",/");
    if (cp == NULL)
	goto again;
    *cp++ = '\0';
    result_buf->s_port = htons((u_short)atoi(p));
    result_buf->s_proto = cp;
    q = result_buf->s_aliases = serv_aliases;
    cp = strpbrk(cp, " \t");
    if (cp != NULL)
	*cp++ = '\0';
    while (cp && *cp) {
	if (*cp == ' ' || *cp == '\t') {
	    cp++;
	    continue;
	}
	if (q < &serv_aliases[MAXALIASES - 1])
	    *q++ = cp;
	cp = strpbrk(cp, " \t");
	if (cp != NULL)
	    *cp++ = '\0';
    }
    *q = NULL;
    *result=result_buf;
    UNLOCK;
    return 0;
}

int getservbyname_r(const char *name, const char *proto,
	struct servent * result_buf, char * buf, size_t buflen,
	struct servent ** result)
{
    register char **cp;
    int ret;

    LOCK;
    setservent(serv_stayopen);
    while (!(ret=getservent_r(result_buf, buf, buflen, result))) {
	if (strcmp(name, result_buf->s_name) == 0)
	    goto gotname;
	for (cp = result_buf->s_aliases; *cp; cp++)
	    if (strcmp(name, *cp) == 0)
		goto gotname;
	continue;
gotname:
	if (proto == 0 || strcmp(result_buf->s_proto, proto) == 0)
	    break;
    }
    if (!serv_stayopen)
	endservent();
    UNLOCK;
    return *result?0:ret;
}

int getservbyport_r(int port, const char *proto,
	struct servent * result_buf, char * buf,
	size_t buflen, struct servent ** result)
{
    int ret;

    LOCK;
    setservent(serv_stayopen);
    while (!(ret=getservent_r(result_buf, buf, buflen, result))) {
	if (result_buf->s_port != port)
	    continue;
	if (proto == 0 || strcmp(result_buf->s_proto, proto) == 0)
	    break;
    }
    if (!serv_stayopen)
	endservent();
    UNLOCK;
    return *result?0:ret;
}
