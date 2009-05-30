#define VISUALSOURCE 1

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
 **     This product includes software developed by the University of
 **     California, Berkeley and its contributors.
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
#include <features.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define	MAXALIASES	35

static FILE *servf = NULL;
static char line[BUFSIZ + 1];
static struct servent serv;
static char *serv_aliases[MAXALIASES];
static int serv_stayopen;

void setservent(int f)
{
	if (servf == NULL)
		servf = fopen(_PATH_SERVICES, "r");
	else
		rewind(servf);
	serv_stayopen |= f;
}

void endservent(void)
{
	if (servf) {
		fclose(servf);
		servf = NULL;
	}
	serv_stayopen = 0;
}

struct servent *getservent(void)
{
	char *p;
	register char *cp, **q;

	if (servf == NULL && (servf = fopen(_PATH_SERVICES, "r")) == NULL)
		return (NULL);
again:
	if ((p = fgets(line, BUFSIZ, servf)) == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = strpbrk(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	serv.s_name = p;
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
	serv.s_port = htons((u_short) atoi(p));
	serv.s_proto = cp;
	q = serv.s_aliases = serv_aliases;
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
	return (&serv);
}

struct servent *getservbyname(const char *name, const char *proto)
{
	register struct servent *p;
	register char **cp;

	setservent(serv_stayopen);
	while ((p = getservent()) != NULL) {
		if (strcmp(name, p->s_name) == 0)
			goto gotname;
		for (cp = p->s_aliases; *cp; cp++)
			if (strcmp(name, *cp) == 0)
				goto gotname;
		continue;
	      gotname:
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
			break;
	}
	if (!serv_stayopen)
		endservent();
	return (p);
}

struct servent *my_getservbyport(int port, const char *proto)
{
	register struct servent *p;

	setservent(serv_stayopen);
	while ((p = getservent()) != NULL) {
		if (p->s_port != port)
			continue;
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
			break;
	}
	if (!serv_stayopen)
		endservent();
	return (p);
}
