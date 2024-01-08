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
#include <shutils.h>

#define MAXALIASES 35

#ifndef _PATH_SERVICES
#define _PATH_SERVICES "/etc/services"
#endif

#define my_setservent(f)                            \
	{                                           \
		servf = fopen(_PATH_SERVICES, "r"); \
	}

#define my_endservent()                \
	{                              \
		if (servf) {           \
			fclose(servf); \
			servf = NULL;  \
		}                      \
	}

static struct servent *my_getservent(FILE *servf, struct servent *serv,
				     char *serv_aliases, char *line)
{
	char *p;
	register char *cp, **q;

	if (servf == NULL && (servf = fopen(_PATH_SERVICES, "r")) == NULL)
		return (NULL);
again:
	if ((p = fgets(line, BUFSIZ, servf)) == NULL) {
		return (NULL);
	}
	if (*p == '#')
		goto again;
	cp = strpbrk(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	serv->s_name = p;
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
	serv->s_port = htons((u_short)atoi(p));
	serv->s_proto = cp;
	q = serv->s_aliases = serv_aliases;
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
	serv->s_name = strdup(serv->s_name);
	serv->s_proto = strdup(serv->s_proto);
	return serv;
}

struct servent *my_getservbyport(int port, const char *proto)
{
	FILE *servf = NULL;
	char *serv_aliases[MAXALIASES];
	int found = 0;
	my_setservent();
	if (!servf) {
		return NULL;
	}
	struct servent *serv = malloc(sizeof(struct servent));
	if (serv == NULL) {
		my_endservent();
		return NULL;
	}
	char *line = malloc(BUFSIZ + 1);
	while ((my_getservent(servf, serv, serv_aliases, line)) != NULL) {
		if (serv->s_port != port) {
			debug_free(serv->s_proto);
			debug_free(serv->s_name);
			continue;
		}
		if (proto == 0 || strcasecmp(serv->s_proto, proto) == 0) {
			found = 1;
			break;
		}
	}
	my_endservent();
	if (!found) {
		debug_free(serv);
		serv = NULL;
	}
	debug_free(line);
	if (serv) {
		return (serv);
	}
	return NULL;
}
