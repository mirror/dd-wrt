/*
    Mac-Telnet - Connect to RouterOS or mactelnetd devices via MAC address
    Copyright (C) 2010, Håkon Nessjøen <haakon.nessjoen@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#if defined(__FreeBSD__) || defined(__APPLE__)
#include <net/ethernet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#else
#include <netinet/ether.h>
#endif
#include <arpa/inet.h>
#include <string.h>
#include "config.h"
#include "protocol.h"
#include "interfaces.h"

#define _(String) String

/* This file is also used for the -l option in mactelnet */

static unsigned char mymain = 1;
int mndp(int timeout, int batch_mode);

int mndp_main(int argc, char **argv)  {
	int batch_mode = 0;
	mymain = 0;
	return mndp(0,batch_mode);
}

void sig_alarm(int signo)
{
	exit(0);
}

int mndp(int timeout, int batch_mode)  {

	int sock,result;
	int optval = 1;
	struct sockaddr_in si_me, si_remote;
	unsigned char buff[MT_PACKET_LEN];

	/* mactelnet.c has this set to 1 */
	if (!mymain)
		signal(SIGALRM, sig_alarm);

	setlocale(LC_ALL, "");

	/* Open a UDP socket handle */
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	/* Set initialize address/port */
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(MT_MNDP_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));

	/* Bind to specified address/port */
	if (bind(sock, (struct sockaddr *)&si_me, sizeof(si_me))==-1) {
		fprintf(stderr, _("Error binding to %s:%d\n"), inet_ntoa(si_me.sin_addr), MT_MNDP_PORT);
		return 1;
	}

	/* Write informative message to STDERR to make it easier to use the output in simple scripts */
	fprintf(stderr, _("Searching for MikroTik routers... Abort with CTRL+C.\n"));

	/* Set the socket to allow sending broadcast packets */
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof (optval))==-1) {
		fprintf(stderr, _("Unable to send broadcast packets: Operating in receive only mode.\n"));
	} else {
		/* Request routers identify themselves */
		unsigned int message = 0;

		memset((char *) &si_remote, 0, sizeof(si_remote));
		si_remote.sin_family = AF_INET;
		si_remote.sin_port = htons(MT_MNDP_PORT);
		si_remote.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		if (sendto (sock, &message, sizeof (message), 0, (struct sockaddr *)&si_remote, sizeof(si_remote))==-1) {
			fprintf(stderr, _("Unable to send broadcast packet: Operating in receive only mode.\n"));
		}
	}

	if (batch_mode) {
		printf("%s\n", "MAC-Address,Identity,Platform,Version,Hardware,Uptime,Softid,Ifname,IP");
	} else {
		printf("\n\E[1m%-15s %-17s %s\E[m\n", _("IP"), _("MAC-Address"), _("Identity (platform version hardware) uptime"));
	}

	if (timeout > 0) {
		alarm(timeout);
	}

	while(1) {
		struct mt_mndp_info *packet;
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);
		char ipstr[INET_ADDRSTRLEN];

		memset(&addr, 0, addrlen);

		/* Wait for a UDP packet */
		result = recvfrom(sock, buff, sizeof(buff), 0, (struct sockaddr *)&addr, &addrlen);
		if (result < 0) {
			fprintf(stderr, _("An error occurred. aborting\n"));
			exit(1);
		}

		/* Parse MNDP packet */
		packet = parse_mndp(buff, result);

		if (packet != NULL && !batch_mode) {

			/* Print it */
			printf("%-15s ", inet_ntop(addr.sin_family, &addr.sin_addr, ipstr, sizeof ipstr));
			printf("%-17s %s", _ether_ntoa((struct ether_addr *)packet->address), packet->identity);
			if (packet->platform[0] != 0) {
				printf(" (%s %s %s)", packet->platform, packet->version, packet->hardware);
			}
			if (packet->uptime > 0) {
				printf(_("  up %d days %d hours %d minutes"), packet->uptime / 86400, packet->uptime % 86400 / 3600, (packet->uptime / 60) % 60 );
			}
			if (packet->softid[0] != 0) {
				printf("  %s", packet->softid);
			}
			if (packet->ifname[0] != 0) {
				printf(" %s", packet->ifname);
			}
			putchar('\n');
		} else if (packet != NULL) {
			/* Print it */
			printf("'%s','%s',", _ether_ntoa((struct ether_addr *)packet->address), packet->identity);
			printf("'%s','%s','%s',", packet->platform, packet->version, packet->hardware);
			printf("'%d','%s','%s'", packet->uptime, packet->softid, packet->ifname);
			printf(",'%s'", inet_ntop(addr.sin_family, &addr.sin_addr, ipstr, sizeof ipstr));
			putchar('\n');
			fflush(stdout);
		}
	}

	/* We'll never get here.. */
	return 0;
}
