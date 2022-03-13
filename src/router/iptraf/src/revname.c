/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

revname.c - reverse DNS resolution module for IPTraf.  As of IPTraf 1.1,
this module now communicates with the rvnamed process to resolve in the
background while allowing the foreground process to continue with the
interim IP addresses in the meantime.

***/

#include "iptraf-ng-compat.h"

#include "deskman.h"
#include "error.h"
#include "getpath.h"
#include "revname.h"
#include "rvnamed.h"
#include "sockaddr.h"

static bool resolver_socket_ready(const struct resolver *res,
				  const short events,
				  const int timeout)
{
	struct pollfd pfds[1];
	nfds_t nfds;
	ssize_t ss;

	/* wait for server socket become readable/writable */
	nfds = 0;
	pfds[nfds].fd = res->sock;
	pfds[nfds].events = events;
	nfds++;

	do {
		ss = poll(pfds, nfds, timeout);
	} while ((ss == -1) && (errno == EINTR));
	if ((ss == -1) || (ss == 0) || (pfds[0].revents & POLLERR)) {
		/* poll() error or poll() timeout or res->sock error */
		return false;
	}

	/* all O.K.: socket ready */
	return true;
}

static bool resolver_socket_read_ready(const struct resolver *res,
				       const int timeout)
{
	return resolver_socket_ready(res, POLLIN, timeout);
}

static bool resolver_socket_write_ready(const struct resolver *res,
					const int timeout)
{
	return resolver_socket_ready(res, POLLOUT, timeout);
}


static bool resolver_send_request(const struct resolver *res,
				  const struct rvn *rpkt)
{
	ssize_t ss;

	do {
		ss = send(res->sock, rpkt, sizeof(*rpkt), 0);
	} while ((ss == -1) && (errno == EINTR));
	if ((ss == -1) || (ss < (ssize_t)sizeof(*rpkt))) {
		/* send() error or not enough bytes sent */
		return false;
	}

	/* all O.K. */
	return true;
}

static bool resolver_receive_response(const struct resolver *res,
				      struct rvn *rpkt)
{
	ssize_t ss;

	/* try to receive struct rvn packet */
	do {
		ss = recv(res->sock, rpkt, sizeof(*rpkt), 0);
	} while ((ss == -1) && (errno == EINTR));
	if ((ss == -1) || (ss < (ssize_t)sizeof(*rpkt) ))
		return false;

	/* all O.K. */
	return true;
}

static bool resolver_active(const struct resolver *res)
{
	struct rvn rpkt;

	/* check if we can write;
	 * this first timeout is bigger than others
	 * because we probably just fork()ed the resolver process */
	if (resolver_socket_write_ready(res, 5000) == false)
		return false;

	/* send HELLO packet */
	rpkt.type = RVN_HELLO;
	if (resolver_send_request(res, &rpkt) == false)
		return false;

	/* check if we can read */
	if (resolver_socket_read_ready(res, 1000) == false)
		return false;

	memset(&rpkt, 0, sizeof(rpkt));
	rpkt.type = -1;
	if (resolver_receive_response(res, &rpkt) == false)
		return false;
	if (rpkt.type != RVN_HELLO)
		return false;

	/* all O.K. */
	return true;
}

void resolver_init(struct resolver *res, bool lookup)
{
	int sockets[2];

	res->lookup = lookup;
	res->sock = 0;
	res->server = 0;

	if (lookup == false)
		return;			/* don't do lookups */

	indicate("Starting reverse lookup server");
	if (socketpair(PF_UNIX, SOCK_DGRAM, 0, sockets) == -1) {
		write_error("Can't get communication sockets; lookups will block");
		return;			/* do synchronous lookups */
	}

	pid_t p = fork();
	if (p == -1) {			/* fork() failed */
		write_error("Can't spawn new process; lookups will block");
		return;			/* do synchronous lookups */
	} else if (p == 0) {
		close(sockets[0]);
		rvnamed(sockets[1]);
		exit(0);
	}

	close(sockets[1]);
	res->sock = sockets[0];
	res->server = p;

	indicate("Trying to communicate with reverse lookup server");
	if (resolver_active(res) == false) {
		write_error("Can't communicate with resolver; will not lookup names");
		resolver_destroy(res);
	}
}

static void resolver_quit(struct resolver *res)
{
	/* send QUIT request */
	if (resolver_socket_write_ready(res, 1000) == true) {
		struct rvn rpkt;

		rpkt.type = RVN_QUIT;
		resolver_send_request(res, &rpkt);
	}

	/* check if already exited */
	int r = waitpid(res->server, NULL, WNOHANG);
	if (r == res->server)
		return;
	if (r == -1)
		die_errno("waitpid(resolver_child)");
	if (r != 0)
		die("waitpid(): exited unknown process ???");

	/* wait a little for child to process the QUIT request */
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = 200000000;	/* 200 ms */
	nanosleep(&t, NULL);

	/* check if exited */
	r = waitpid(res->server, NULL, WNOHANG);
	if (r == res->server)
		return;
	if (r == -1)
		die_errno("waitpid(resolver_child)");
	if (r != 0)
		die("waitpid(): exited unknown process ???");

	/* send SIGTERM to terminate the child ... */
	if (kill(res->server, SIGTERM) == -1) {
		if(errno == ESRCH)
			return;		/* already exited ??? */

		die_errno("kill(resolver_child, TERM)");
	}

	/* wait a little */
	t.tv_sec = 0;
	t.tv_nsec = 200000000;	/* 200 ms */
	nanosleep(&t, NULL);

	/* check if exited */
	waitpid(res->server, NULL, WNOHANG);
	/* no return value checking: we've done enough to reap the zombie ... */
}

void resolver_destroy(struct resolver *res)
{
	if (res->server > 0)
		resolver_quit(res);
	res->server = 0;

	if(res->sock > 0)
		close(res->sock);
	res->sock = 0;

	res->lookup = false;
}

int revname(struct resolver *res, struct sockaddr_storage *addr,
	    char *target, size_t target_size)
{
	memset(target, 0, target_size);

	if (res == NULL) {		/* no lookup */
		sockaddr_ntop(addr, target, target_size);
		return RESOLVED;
	}
	if (res->lookup == false) {	/* no lookup */
		sockaddr_ntop(addr, target, target_size);
		return RESOLVED;
	}
	if (res->sock <= 0) {		/* blocking lookup */
		sockaddr_gethostbyaddr(addr, target, target_size);
		return RESOLVED;
	}

	/* non-blocking/async lookup */
	struct rvn rpkt;

	rpkt.type = RVN_REQUEST;
	sockaddr_copy(&rpkt.addr, addr);

	if ((resolver_socket_write_ready(res, 1000) == false) ||
	    (resolver_send_request(res, &rpkt) == false) ||
	    (resolver_socket_read_ready(res, 1000) == false) ||
	    (resolver_receive_response(res, &rpkt) == false)) {
		sockaddr_ntop(addr, target, target_size);
		printipcerr();
		resolver_destroy(res);
		return RESOLVED;
	}

	strncpy(target, rpkt.fqdn, target_size - 1);
	return rpkt.ready;
}
