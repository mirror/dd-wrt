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
#include <signal.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <float.h>
#include "protocol.h"
#include "interfaces.h"
#include "config.h"

#define MAX_DEVICES 128
#define MT_INTERFACE_LEN 128

#define PROGRAM_NAME "MAC-Ping"

#define _(String) String

static int sockfd, insockfd;

static unsigned short ping_size = 38;

struct net_interface interfaces[MAX_INTERFACES];

static struct in_addr sourceip;
static struct in_addr destip;
static unsigned char dstmac[ETH_ALEN];

static int ping_sent = 0;
static int pong_received = 0;
static float min_ms = FLT_MAX;
static float avg_ms = 0;
static float max_ms = 0;

/* Protocol data direction, not used here, but obligatory for protocol.c */
unsigned char mt_direction_fromserver = 0;

static void print_version() {
	fprintf(stderr, PROGRAM_NAME " " PROGRAM_VERSION "\n");
}

static long long int toddiff(struct timeval *tod1, struct timeval *tod2)
{
    long long t1, t2;
    t1 = tod1->tv_sec * 1000000 + tod1->tv_usec;
    t2 = tod2->tv_sec * 1000000 + tod2->tv_usec;
    return t1 - t2;
}

static void display_results() {
	int percent = (int)((100.f/ping_sent) * pong_received);
	if (percent > 100) {
		percent = 0;
	}

	if (percent < 0) {
		percent = 0;
	}

	if (min_ms == FLT_MAX) {
		min_ms = 0;
	}

	printf("\n");
	printf(_("%d packets transmitted, %d packets received, %d%% packet loss\n"), ping_sent, pong_received, 100 - percent);
	printf(_("round-trip min/avg/max = %.2f/%.2f/%.2f ms\n"), min_ms, avg_ms/pong_received, max_ms);

	/* For bash scripting */
	if (pong_received == 0) {
		exit(1);
	}

	exit(0);
}

int main(int argc, char **argv)  {
	int optval = 1;
	int print_help = 0;
	int send_packets = 5;
	int fastmode = 0;
	int c;
	struct sockaddr_in si_me;
	struct mt_packet packet;
	int i;

	setlocale(LC_ALL, "");

	while (1) {
		c = getopt(argc, argv, "fs:c:hv?");

		if (c == -1) {
			break;
		}

		switch (c) {
			case 'f':
				fastmode = 1;
				break;

			case 's':
				ping_size = atoi(optarg) - 18;
				break;

			case 'v':
				print_version();
				exit(0);
				break;

			case 'c':
				send_packets = atoi(optarg);
				break;

			case 'h':
			case '?':
				print_help = 1;
				break;

		}
	}

	/* We don't want people to use this for the wrong reasons */
	if (fastmode && (send_packets == 0 || send_packets > 100)) {
		fprintf(stderr, _("Number of packets to send must be more than 0 and less than 100 in fast mode.\n"));
		return 1;
	}

	if (argc - optind < 1 || print_help) {
		print_version();
		fprintf(stderr, _("Usage: %s <MAC> [-h] [-f] [-c <count>] [-s <packet size>]\n"), argv[0]);

		if (print_help) {
			fprintf(stderr, _("\nParameters:\n"
			"  MAC       MAC-Address of the RouterOS/mactelnetd device.\n"
			"  -f        Fast mode, do not wait before sending next ping request.\n"
			"  -s        Specify size of ping packet.\n"
			"  -c        Number of packets to send. (0 = unlimited)\n"
			"  -h        This help.\n"
			"\n"));
		}
		return 1;
	}

	if (ping_size > ETH_FRAME_LEN - 42) {
		fprintf(stderr, _("Packet size must be between 18 and %d\n"), ETH_FRAME_LEN - 42 + 18);
		exit(1);
	}

	/* Mikrotik RouterOS does not answer unless the packet has the correct recipient mac-address in
	 * the ethernet frame. Unlike real MacTelnet connections where the OS is ok with it being a
	 * broadcast mac address.
	 */
	if (geteuid() != 0) {
		fprintf(stderr, _("You need to have root privileges to use %s.\n"), argv[0]);
		return 1;
	}

	/* Get mac-address from string, or check for hostname via mndp */
	if (!query_mndp_or_mac(argv[optind], dstmac, 1)) {
		/* No valid mac address found, abort */
		return 1;
	}

	sockfd = net_init_raw_socket();

	insockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (insockfd < 0) {
		perror("insockfd");
		return 1;
	}

	/* Set initialize address/port */
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(MT_MACTELNET_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	setsockopt(insockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));

	/* Bind to specified address/port */
	if (bind(insockfd, (struct sockaddr *)&si_me, sizeof(si_me))==-1) {
		fprintf(stderr, _("Error binding to %s:%d\n"), inet_ntoa(si_me.sin_addr), MT_MNDP_PORT);
		return 1;
	}

	/* Listen address*/
	inet_pton(AF_INET, (char *)"0.0.0.0", &sourceip);

	/* Set up global info about the connection */
	inet_pton(AF_INET, (char *)"255.255.255.255", &destip);

	srand(time(NULL));

	/* Enumerate available interfaces */
	net_get_interfaces(interfaces, MAX_INTERFACES);

	if (ping_size < sizeof(struct timeval)) {
		ping_size = sizeof(struct timeval);
	}

	signal(SIGINT, display_results);

	for (i = 0; i < send_packets || send_packets == 0; ++i) {
		fd_set read_fds;
		static struct timeval lasttimestamp;
		int reads, result;
		struct timeval timeout;
		int ii;
		int sent = 0;
		int waitforpacket;
		struct timeval timestamp;
		unsigned char pingdata[1500];

		gettimeofday(&timestamp, NULL);
		memcpy(pingdata, &timestamp, sizeof(timestamp));
		for (ii = sizeof(timestamp); ii < ping_size; ++ii) {
			pingdata[ii] = rand() % 256;
		}

		for (ii = 0; ii < MAX_INTERFACES; ++ii) {
			struct net_interface *interface = &interfaces[ii];

			if (!interface->in_use) {
				break;
			}

			if (!interface->has_mac) {
				continue;
			}

			init_pingpacket(&packet, interface->mac_addr, dstmac);
			add_packetdata(&packet, pingdata, ping_size);
			result = net_send_udp(sockfd, interface, interface->mac_addr, dstmac, &sourceip, MT_MACTELNET_PORT, &destip, MT_MACTELNET_PORT, packet.data, packet.size);

			if (result > 0) {
				sent++;
			}

		}
		if (sent == 0) {
			fprintf(stderr, _("Error sending packet.\n"));
			continue;
		}
		ping_sent++;

		FD_ZERO(&read_fds);
		FD_SET(insockfd, &read_fds);

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		waitforpacket = 1;

		while (waitforpacket) {
			/* Wait for data or timeout */
			reads = select(insockfd+1, &read_fds, NULL, NULL, &timeout);
			if (reads <= 0) {
				waitforpacket = 0;
				fprintf(stderr, _("%s ping timeout\n"), ether_ntoa((struct ether_addr *)&dstmac));
				break;
			}

			unsigned char buff[1500];
			struct sockaddr_in saddress;
			unsigned int slen = sizeof(saddress);
			struct mt_mactelnet_hdr pkthdr;

			result = recvfrom(insockfd, buff, 1500, 0, (struct sockaddr *)&saddress, &slen);
			parse_packet(buff, &pkthdr);

			/* TODO: Check that we are the receiving host */
			if (pkthdr.ptype != MT_PTYPE_PONG) {
				/* Wait for the correct packet */
				continue;
			}
			
			struct timeval pongtimestamp;
			struct timeval nowtimestamp;

			waitforpacket = 0;
			gettimeofday(&nowtimestamp, NULL);

			memcpy(&pongtimestamp, pkthdr.data - 4, sizeof(pongtimestamp));
			if (memcmp(pkthdr.data - 4, pingdata, ping_size) == 0) {
				float diff = toddiff(&nowtimestamp, &pongtimestamp) / 1000.0f;

				if (diff < min_ms) {
					min_ms = diff;
				}

				if (diff > max_ms) {
					max_ms = diff;
				}

				avg_ms += diff;

				printf(_("%s %d byte, ping time %.2f ms%s\n"), ether_ntoa((struct ether_addr *)&(pkthdr.srcaddr)), result, diff, (char *)(memcmp(&pongtimestamp,&lasttimestamp,sizeof(lasttimestamp)) == 0 ? " DUP" : ""));
			} else {
				printf(_("%s Reply of %d bytes of unequal data\n"), ether_ntoa((struct ether_addr *)&(pkthdr.srcaddr)), result);
			}
			pong_received++;
			memcpy(&lasttimestamp, &pongtimestamp, sizeof(pongtimestamp));
			if (!fastmode) {
				sleep(1);
			}
		}
	}

	/* Display statistics and exit */
	display_results();

	return 0;
}
