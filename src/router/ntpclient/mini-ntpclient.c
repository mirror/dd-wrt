/* Stripped-down & simplified ntpclient by tofu
 *
 * Copyright (C) 1997-2010  Larry Doolittle <larry@doolittle.boa.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2,
 * June 1991) as published by the Free Software Foundation.  At the
 * time of writing, that license was published by the FSF with the URL
 * http://www.gnu.org/copyleft/gpl.html, and is incorporated herein by
 * reference.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/timex.h>

#define JAN_1970  0x83aa7e80	/* 2208988800 1970 - 1900 in seconds */
#define NTP_PORT  123

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) ( 4294*(x) + ( (1981*(x))>>11 ) )

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via settimeofday) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x) ( (x) * 15.2587890625 )

#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6

static int getaddrbyname(char *host, struct sockaddr_storage *ss, int isipv6)
{
	int err;
	static int netdown = 0;
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *rp;

	if (!host || !ss) {
		errno = EINVAL;
		return 1;
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	memset(ss, 0, sizeof(struct sockaddr_storage));
	err = getaddrinfo(host, NULL, &hints, &result);
	if (err) {
		syslog(LOG_ERR, "Failed resolving address to hostname %s: %s", host, gai_strerror(err));
		netdown = errno = ENETDOWN;
		return 1;
	}

	/* The first result will be used. IPV4 has higher priority */
	err = 1;
	for (rp = result; rp; rp = rp->ai_next) {
		if (rp->ai_family == AF_INET) {
			memcpy(ss, (struct sockaddr_in *)(rp->ai_addr), sizeof(struct sockaddr_in));
			err = 0;
			break;
		}
		if (isipv6 && rp->ai_family == AF_INET6) {
			memcpy(ss, (struct sockaddr_in6 *)(rp->ai_addr), sizeof(struct sockaddr_in6));
			err = 0;
			break;
		}
	}
	freeaddrinfo(result);

	if (err) {
		errno = EAGAIN;
		return 1;
	}

	if (netdown) {
		syslog(LOG_NOTICE, "Network up, resolved address to hostname %s", host);
		netdown = 0;
	}

	return 0;
}

static char *prognm = NULL;

static int usage(int code)
{
	printf("Usage: %s <server> [server [...]]\n", prognm);
	return code;
}

static void send_packet(int sd)
{
	uint32_t data[12];
	struct timeval now;

	memset(data, 0, sizeof(data));
	data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24) | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
	data[1] = htonl(1 << 16);	/* Root Delay (seconds) */
	data[2] = htonl(1 << 16);	/* Root Dispersion (seconds) */
	gettimeofday(&now, NULL);
	data[10] = htonl(now.tv_sec + JAN_1970);	/* Transmit Timestamp coarse */
	data[11] = htonl(NTPFRAC(now.tv_usec));	/* Transmit Timestamp fine   */
	send(sd, data, 48, 0);
}

static int set_time(char *srv, char *addr, uint32_t * data)
{
	struct timeval tv;

	tv.tv_sec = ntohl(((uint32_t *) data)[10]) - JAN_1970;
	tv.tv_usec = USEC(ntohl(((uint32_t *) data)[11]));
	if (settimeofday(&tv, NULL) < 0) {
		perror("settimeofday");
		return 1;	/* Ouch, this should not happen :-( */
	}

	syslog(LOG_DAEMON | LOG_INFO, "Time set from %s [%s].\n", srv, addr);

	return 0;		/* All good, time set! */
}

static int query_server(char *srv)
{
	int sd, rc, len;
	struct pollfd pfd;
	struct sockaddr_storage ss;
	struct sockaddr_in *ipv4 = NULL;
	struct sockaddr_in6 *ipv6 = NULL;
	int isipv6 = 1;

	sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sd == -1) {
	      again:;
		isipv6 = 0;
		sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sd == -1) {
			return -1;	/* Fatal error, cannot even create a socket? */
		}
	}
	int he = getaddrbyname(srv, &ss, isipv6);
	if (he) {
		syslog(LOG_DAEMON | LOG_ERR, "Failed resolving server %s: %s\n", srv, strerror(errno));
		close(sd);

		return usage(1);	/* Failure in name resolution. */
	}
	char deststr[INET6_ADDRSTRLEN];

	/* Prefer IPv4 over IPv6, for now */
	if (ss.ss_family == AF_INET) {
		ipv4 = (struct sockaddr_in *)(&ss);
		ipv4->sin_port = htons(NTP_PORT);
		len = sizeof(struct sockaddr_in);
		inet_ntop(AF_INET, &ipv4->sin_addr, deststr, sizeof(deststr));
	} else if (ss.ss_family == AF_INET6) {
		ipv6 = (struct sockaddr_in6 *)(&ss);
		ipv6->sin6_port = htons(NTP_PORT);
		len = sizeof(struct sockaddr_in6);
		inet_ntop(AF_INET6, &ipv6->sin6_addr, deststr, sizeof(deststr));
	} else {
		syslog(LOG_ERR, "Unsupported address family for %s\n", srv);
		return -1;
	}
	syslog(LOG_DAEMON | LOG_DEBUG, "Connecting to %s [%s] ...\n", srv, deststr);

	if (connect(sd, (struct sockaddr *)&ss, len) == -1) {
		close(sd);
		if (isipv6)
			goto again;
		syslog(LOG_DAEMON | LOG_ERR, "Failed connecting to %s [%s]: %s", srv, deststr, strerror(errno));
		return 1;	/* Cannot connect to server, try next. */
	}

	/* Send NTP query to server ... */
	send_packet(sd);

	/* Wait for reply from server ... */
	pfd.fd = sd;
	pfd.events = POLLIN;
	rc = poll(&pfd, 1, 3000);
	if (rc == 1) {
		int len;
		uint32_t packet[12];

		//syslog(LOG_DAEMON | LOG_DEBUG, "Received packet from server ...");
		len = recv(sd, packet, sizeof(packet), 0);
		if (len == sizeof(packet)) {
			close(sd);

			/* Looks good, try setting time on host ... */
			if (set_time(srv, deststr, packet))
				return -1;	/* Fatal error */

			return 0;	/* All done! :) */
		}
	} else if (rc == 0) {
		syslog(LOG_DAEMON | LOG_DEBUG, "Timed out waiting for %s [%s].", srv, deststr);
	}

	close(sd);

	return 1;		/* No luck, try next server. */
}

/* Connects to each server listed on the command line and sets the time */
int main(int argc, char *argv[])
{
	int i;

	prognm = argv[0];
	if (argc <= 1)
		return usage(1);
	openlog("ntpclient", LOG_PID | LOG_NDELAY, LOG_DAEMON);

	for (i = 1; i < argc; ++i) {
		char *srv;
		char buf[256];

		strcpy(buf, argv[i]);
		srv = strtok(buf, " ");
		while (srv) {
			int rc = query_server(srv);

			/* Done, time set! */
			if (0 == rc)
				goto ok;

			/* Fatal error, exit now. */
			if (-1 == rc)
				goto err;

			/* No response, or other error, try next server */
			srv = strtok(NULL, " ");
		}
	}
      err:;
	closelog();
	return 1;
      ok:;
	closelog();
	return 0;
}
