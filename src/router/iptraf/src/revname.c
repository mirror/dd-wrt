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
#include "getpath.h"
#include "rvnamed.h"

char revname_socket[80];

static char *gen_unix_sockname(void)
{
	static char scratch[80];

	srandom(time(NULL));
	snprintf(scratch, 80, "%s-%lu%d%ld", SOCKET_PREFIX, time(NULL),
		 getpid(), random());

	return scratch;
}

int rvnamedactive(void)
{
	int fd;
	fd_set sockset;
	struct rvn rpkt;
	struct sockaddr_un su;
	int sstat;
	struct timeval tv;
	socklen_t fr;
	int br;
	char unix_socket[80];

	strncpy(unix_socket, get_path(T_WORKDIR, gen_unix_sockname()), 80);
	unlink(unix_socket);

	fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	su.sun_family = AF_UNIX;
	strcpy(su.sun_path, unix_socket);
	bind(fd, (struct sockaddr *) &su,
	     sizeof(su.sun_family) + strlen(su.sun_path));

	su.sun_family = AF_UNIX;
	strcpy(su.sun_path, IPTSOCKNAME);

	rpkt.type = RVN_HELLO;

	sendto(fd, &rpkt, sizeof(struct rvn), 0, (struct sockaddr *) &su,
	       sizeof(su.sun_family) + strlen(su.sun_path));

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	FD_ZERO(&sockset);
	FD_SET(fd, &sockset);

	do {
		sstat = select(fd + 1, &sockset, NULL, NULL, &tv);
	} while ((sstat < 0) && (errno != ENOMEM) && (errno == EINTR));

	if (sstat == 1) {
		fr = sizeof(su.sun_family) + strlen(su.sun_path);
		do {
			br = recvfrom(fd, &rpkt, sizeof(struct rvn), 0,
				      (struct sockaddr *) &su, &fr);
		} while ((br < 0) && (errno == EINTR));

		if (br < 0)
			printipcerr();
	}

	close(fd);
	unlink(unix_socket);

	if (sstat == 0)
		return 0;
	else
		return 1;
}

/*
 * Terminate rvnamed process
 */

void killrvnamed(void)
{
	int fd;
	struct sockaddr_un su;
	struct rvn rvnpkt;

	fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	su.sun_family = AF_UNIX;
	strcpy(su.sun_path, IPTSOCKNAME);

	rvnpkt.type = RVN_QUIT;

	sendto(fd, &rvnpkt, sizeof(struct rvn), 0, (struct sockaddr *) &su,
	       sizeof(su.sun_family) + strlen(su.sun_path));

	close(fd);
}

void open_rvn_socket(int *fd)
{
	struct sockaddr_un su;

	strncpy(revname_socket, get_path(T_WORKDIR, gen_unix_sockname()), 80);
	unlink(revname_socket);

	*fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	su.sun_family = AF_UNIX;
	strcpy(su.sun_path, revname_socket);
	bind(*fd, (struct sockaddr *) &su,
	     sizeof(su.sun_family) + strlen(su.sun_path));
}

void close_rvn_socket(int fd)
{
	if (fd > 0) {
		close(fd);
		unlink(revname_socket);
	}
}

int revname(int *lookup, struct in_addr *saddr, struct in6_addr *s6addr,
	    char *target, int rvnfd)
{
	struct hostent *he;
	struct rvn rpkt;
	int br;
	struct sockaddr_un su;
	socklen_t fl;
	fd_set sockset;
	struct timeval tv;
	int sstat = 0;

	memset(target, 0, 45);
	if (*lookup) {
		if (rvnfd > 0) {
			su.sun_family = AF_UNIX;
			strcpy(su.sun_path, IPTSOCKNAME);

			rpkt.type = RVN_REQUEST;
			rpkt.saddr.s_addr = saddr->s_addr;

			if (s6addr != NULL)
				memcpy(rpkt.s6addr.s6_addr, s6addr->s6_addr,
				       16);
			else
				memset(rpkt.s6addr.s6_addr, 0, 4);

			sendto(rvnfd, &rpkt, sizeof(struct rvn), 0,
			       (struct sockaddr *) &su,
			       sizeof(su.sun_family) + strlen(su.sun_path));

			fl = sizeof(su.sun_family) + strlen(su.sun_path);
			do {
				tv.tv_sec = 10;
				tv.tv_usec = 0;

				FD_ZERO(&sockset);
				FD_SET(rvnfd, &sockset);

				do {
					sstat =
					    select(rvnfd + 1, &sockset, NULL,
						   NULL, &tv);
				} while ((sstat < 0) && (errno == EINTR));

				if (FD_ISSET(rvnfd, &sockset))
					br = recvfrom(rvnfd, &rpkt,
						      sizeof(struct rvn), 0,
						      (struct sockaddr *) &su,
						      &fl);
				else
					br = -1;
			} while ((br < 0) && (errno == EINTR));

			if (br < 0) {
				if (saddr->s_addr != 0)
					strcpy(target, inet_ntoa(*saddr));
				else
					inet_ntop(AF_INET6, s6addr, target, 44);
				printipcerr();
				*lookup = 0;
				return RESOLVED;
			}
			strncpy(target, rpkt.fqdn, 44);
			return (rpkt.ready);
		} else {
			if (saddr->s_addr != 0)
				he = gethostbyaddr((char *) saddr,
						   sizeof(struct in_addr),
						   AF_INET);
			else
				he = gethostbyaddr((char *) s6addr,
						   sizeof(struct in6_addr),
						   AF_INET6);

			if (he == NULL) {
				if (saddr->s_addr != 0)
					strcpy(target, inet_ntoa(*saddr));
				else
					inet_ntop(AF_INET6, s6addr, target, 44);
			} else {
				strncpy(target, he->h_name, 44);
			}

			return RESOLVED;
		}
	} else {
		if (saddr->s_addr != 0 || s6addr == NULL)
			strcpy(target, inet_ntoa(*saddr));
		else
			inet_ntop(AF_INET6, s6addr, target, 44);

		return RESOLVED;
	}
	return NOTRESOLVED;
}
