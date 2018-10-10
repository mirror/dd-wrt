/*
 * resolving DNS hostname routine
 *
 * Copyright (C) 2010 Jeff Layton (jlayton@samba.org)
 * Copyright (C) 2010 Igor Druzhinin (jaxbrigs@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "mount.h"
#include "util.h"
#include "resolve_host.h"

/*
 * resolve hostname to comma-separated list of address(es)
 */
int resolve_host(const char *host, char *addrstr)
{
	int rc;
	/* 10 for max width of decimal scopeid */
	char tmpbuf[NI_MAXHOST + 1 + 10 + 1];
	const char *ipaddr;
	size_t len;
	struct addrinfo *addrlist, *addr;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;

	rc = getaddrinfo(host, NULL, NULL, &addrlist);
	if (rc != 0)
		return EX_USAGE;

	addr = addrlist;
	while (addr) {
		/* skip non-TCP entries */
		if (addr->ai_socktype != SOCK_STREAM ||
		    addr->ai_protocol != IPPROTO_TCP) {
			addr = addr->ai_next;
			continue;
		}

		switch (addr->ai_addr->sa_family) {
		case AF_INET6:
			sin6 = (struct sockaddr_in6 *)addr->ai_addr;
			ipaddr = inet_ntop(AF_INET6, &sin6->sin6_addr, tmpbuf,
					   sizeof(tmpbuf));
			if (!ipaddr) {
				rc = EX_SYSERR;
				goto resolve_host_out;
			}

			if (sin6->sin6_scope_id) {
				len = strnlen(tmpbuf, sizeof(tmpbuf));
				snprintf(tmpbuf + len, sizeof(tmpbuf) - len, "%%%u",
					 sin6->sin6_scope_id);
			}
			break;
		case AF_INET:
			sin = (struct sockaddr_in *)addr->ai_addr;
			ipaddr = inet_ntop(AF_INET, &sin->sin_addr, tmpbuf,
					   sizeof(tmpbuf));
			if (!ipaddr) {
				rc = EX_SYSERR;
				goto resolve_host_out;
			}

			break;
		default:
			addr = addr->ai_next;
			continue;
		}

		if (addr == addrlist)
			*addrstr = '\0';
		else
			strlcat(addrstr, ",", MAX_ADDR_LIST_LEN);

		strlcat(addrstr, tmpbuf, MAX_ADDR_LIST_LEN);
		addr = addr->ai_next;
	}

resolve_host_out:
	freeaddrinfo(addrlist);
	return rc;
}
