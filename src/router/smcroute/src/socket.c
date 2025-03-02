/* Socket helper functions
 *
 * Copyright (C) 2017-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "config.h"
#include "queue.h"

#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "log.h"

struct sock {
	LIST_ENTRY(sock) link;

	int sd;

	void (*cb)(int, void *arg);
	void *arg;
};

static int max_fdnum = -1;
static LIST_HEAD(slist, sock) sock_list = LIST_HEAD_INITIALIZER();


int nfds(void)
{
	return max_fdnum + 1;
}

/*
 * register socket/fd/pipe created elsewhere, optional callback
 */
int socket_register(int sd, void (*cb)(int, void *), void *arg)
{
	struct sock *entry;

	entry = malloc(sizeof(*entry));
	if (!entry) {
		smclog(LOG_ERR, "Failed allocating memory registering socket: %s", strerror(errno));
		exit(EX_OSERR);
	}

	entry->sd  = sd;
	entry->cb  = cb;
	entry->arg = arg;
	LIST_INSERT_HEAD(&sock_list, entry, link);

#if !defined(HAVE_SOCK_CLOEXEC) && defined(HAVE_FCNTL_H)
	fcntl(sd, F_SETFD, fcntl(sd, F_GETFD) | FD_CLOEXEC);
#endif

	/* Keep track for select() */
	if (sd > max_fdnum)
		max_fdnum = sd;

	return sd;
}

/*
 * create socket, with optional callback for reading inbound data
 */
int socket_create(int domain, int type, int proto, void (*cb)(int, void *), void *arg)
{
	int val = 0;
	int sd;

#ifdef HAVE_SOCK_CLOEXEC
	type |= SOCK_CLOEXEC;
#endif
	sd = socket(domain, type, proto);
	if (sd < 0)
		return -1;

	if (domain == AF_UNIX)
		goto done;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (domain == AF_INET6) {
		if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &val, sizeof(val)))
			smclog(LOG_WARNING, "failed disabling IPV6_MULTICAST_LOOP: %s", strerror(errno));
#ifdef IPV6_MULTICAST_ALL
		if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_ALL, &val, sizeof(val)))
			smclog(LOG_WARNING, "failed disabling IPV6_MULTICAST_ALL: %s", strerror(errno));
#endif
	} else
#endif /* HAVE_IPV6_MULTICAST_HOST */
	{

		if (setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, &val, sizeof(val)))
			smclog(LOG_WARNING, "failed disabling IP_MULTICAST_LOOP: %s", strerror(errno));
#ifdef IP_MULTICAST_ALL
		if (setsockopt(sd, IPPROTO_IP, IP_MULTICAST_ALL, &val, sizeof(val)))
			smclog(LOG_WARNING, "failed disabling IP_MULTICAST_ALL: %s", strerror(errno));
#endif
	}
done:
	if (socket_register(sd, cb, arg) < 0) {
		close(sd);
		return -1;
	}

	return sd;
}

int socket_close(int sd)
{
	struct sock *entry, *tmp;

	LIST_FOREACH_SAFE(entry, &sock_list, link, tmp) {
		if (entry->sd == sd) {
			LIST_REMOVE(entry, link);
			close(entry->sd);
			free(entry);

			return 0;
		}
	}

	errno = ENOENT;
	return -1;
}

int socket_poll(struct timeval *timeout)
{
	int num;
	fd_set fds;
	struct sock *entry;

	FD_ZERO(&fds);
	LIST_FOREACH(entry, &sock_list, link)
		FD_SET(entry->sd, &fds);

	num = select(nfds(), &fds, NULL, NULL, timeout);
	if (num <= 0) {
		/* Log all errors, except when signalled, ignore failures. */
		if (num < 0 && EINTR != errno)
			smclog(LOG_WARNING, "Failed select(): %s", strerror(errno));

		return num;
	}

	LIST_FOREACH(entry, &sock_list, link) {
		if (!FD_ISSET(entry->sd, &fds))
			continue;

		if (entry->cb)
			entry->cb(entry->sd, entry->arg);
	}

	return num;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
