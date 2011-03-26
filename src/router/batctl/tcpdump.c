/*
 * Copyright (C) 2007-2011 B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>

#include "main.h"
#include "tcpdump.h"
#include "packet.h"
#include "bat-hosts.h"
#include "functions.h"


#define LEN_CHECK(buff_len, check_len, desc) \
if ((size_t)(buff_len) < (check_len)) { \
	printf("Warning - dropping received %s packet as it is smaller than expected (%zu): %zu\n", \
		desc, (check_len), (size_t)(buff_len)); \
	return; \
}

static unsigned short dump_level = DUMP_TYPE_BATOGM | DUMP_TYPE_BATICMP | DUMP_TYPE_BATUCAST |
		DUMP_TYPE_BATBCAST | DUMP_TYPE_BATVIS | DUMP_TYPE_BATFRAG | DUMP_TYPE_NONBAT;

static void parse_eth_hdr(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed);

static void tcpdump_usage(void)
{
	printf("Usage: batctl tcpdump [options] interface [interface]\n");
	printf("options:\n");
	printf(" \t -h print this help\n");
	printf(" \t -n don't convert addresses to bat-host names\n");
	printf(" \t -p dump specific packet type\n");
	printf(" \t\t%d - batman ogm packets\n", DUMP_TYPE_BATOGM);
	printf(" \t\t%d - batman icmp packets\n", DUMP_TYPE_BATICMP);
	printf(" \t\t%d - batman unicast packets\n", DUMP_TYPE_BATUCAST);
	printf(" \t\t%d - batman broadcast packets\n", DUMP_TYPE_BATBCAST);
	printf(" \t\t%d - batman vis packets\n", DUMP_TYPE_BATVIS);
	printf(" \t\t%d - batman fragmented packets\n", DUMP_TYPE_BATFRAG);
	printf(" \t\t%d - non batman packets\n", DUMP_TYPE_NONBAT);
	printf(" \t\t%d - batman ogm & non batman packets\n", DUMP_TYPE_BATOGM | DUMP_TYPE_NONBAT);
}

static int print_time(void)
{
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	printf("%02d:%02d:%02d.%06ld ", tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec);
	return 1;
}

static void dump_arp(unsigned char *packet_buff, ssize_t buff_len, int time_printed)
{
	struct ether_arp *arphdr;

	LEN_CHECK((size_t)buff_len, sizeof(struct ether_arp), "ARP");

	if (!time_printed)
		print_time();

	arphdr = (struct ether_arp *)packet_buff;

	switch (ntohs(arphdr->arp_op)) {
	case ARPOP_REQUEST:
		printf("ARP, Request who-has %s", inet_ntoa(*(struct in_addr *)&arphdr->arp_tpa));
		printf(" tell %s (%s), length %zd\n", inet_ntoa(*(struct in_addr *)&arphdr->arp_spa),
			ether_ntoa_long((struct ether_addr *)&arphdr->arp_sha), buff_len);
		break;
	case ARPOP_REPLY:
		printf("ARP, Reply %s is-at %s, length %zd\n", inet_ntoa(*(struct in_addr *)&arphdr->arp_spa),
			ether_ntoa_long((struct ether_addr *)&arphdr->arp_sha), buff_len);
		break;
	default:
		printf("ARP, unknown op code: %i\n", ntohs(arphdr->arp_op));
		break;
	}
}

static void dump_ip(unsigned char *packet_buff, ssize_t buff_len, int time_printed)
{
	struct iphdr *iphdr, *tmp_iphdr;
	struct tcphdr *tcphdr;
	struct udphdr *udphdr, *tmp_udphdr;
	struct icmphdr *icmphdr;

	iphdr = (struct iphdr *)packet_buff;
	LEN_CHECK((size_t)buff_len, (size_t)(iphdr->ihl * 4), "IP");

	if (!time_printed)
		print_time();

	switch (iphdr->protocol) {
	case IPPROTO_ICMP:
		LEN_CHECK((size_t)buff_len - (iphdr->ihl * 4), sizeof(struct icmphdr), "ICMP");

		icmphdr = (struct icmphdr *)(packet_buff + (iphdr->ihl * 4));
		printf("IP %s > ", inet_ntoa(*(struct in_addr *)&iphdr->saddr));

		switch (icmphdr->type) {
		case ICMP_ECHOREPLY:
			printf("%s: ICMP echo reply, id %hu, seq %hu, length %zu\n",
				inet_ntoa(*(struct in_addr *)&iphdr->daddr),
				ntohs(icmphdr->un.echo.id), ntohs(icmphdr->un.echo.sequence),
				(size_t)buff_len - (iphdr->ihl * 4));
			break;
		case ICMP_DEST_UNREACH:
			LEN_CHECK((size_t)buff_len - (iphdr->ihl * 4) - sizeof(struct icmphdr),
				sizeof(struct iphdr) + 8, "ICMP DEST_UNREACH");

			switch (icmphdr->code) {
			case ICMP_PORT_UNREACH:
				tmp_iphdr = (struct iphdr *)(((char *)icmphdr) + sizeof(struct icmphdr));
				tmp_udphdr = (struct udphdr *)(((char *)tmp_iphdr) + (tmp_iphdr->ihl * 4));

				printf("%s: ICMP ", inet_ntoa(*(struct in_addr *)&iphdr->daddr));
				printf("%s udp port %hu unreachable, length %zu\n",
					inet_ntoa(*(struct in_addr *)&tmp_iphdr->daddr),
					ntohs(tmp_udphdr->dest), (size_t)buff_len - (iphdr->ihl * 4));
				break;
			default:
				printf("%s: ICMP unreachable %hhu, length %zu\n",
					inet_ntoa(*(struct in_addr *)&iphdr->daddr),
					icmphdr->code, (size_t)buff_len - (iphdr->ihl * 4));
				break;
			}

			break;
		case ICMP_ECHO:
			printf("%s: ICMP echo request, id %hu, seq %hu, length %zu\n",
				inet_ntoa(*(struct in_addr *)&iphdr->daddr),
				ntohs(icmphdr->un.echo.id), ntohs(icmphdr->un.echo.sequence),
				(size_t)buff_len - (iphdr->ihl * 4));
			break;
		case ICMP_TIME_EXCEEDED:
			printf("%s: ICMP time exceeded in-transit, length %zu\n",
				inet_ntoa(*(struct in_addr *)&iphdr->daddr),
				(size_t)buff_len - (iphdr->ihl * 4));
			break;
		default:
			printf("%s: ICMP type %hhu, length %zu\n",
				inet_ntoa(*(struct in_addr *)&iphdr->daddr), icmphdr->type,
				(size_t)buff_len - (iphdr->ihl * 4));
			break;
		}

		break;
	case IPPROTO_TCP:
		LEN_CHECK((size_t)buff_len - (iphdr->ihl * 4), sizeof(struct tcphdr), "TCP");

		tcphdr = (struct tcphdr *)(packet_buff + (iphdr->ihl * 4));
		printf("IP %s.%i > ", inet_ntoa(*(struct in_addr *)&iphdr->saddr), ntohs(tcphdr->source));
		printf("%s.%i: TCP, flags [%c%c%c%c%c%c], length %zu\n",
			inet_ntoa(*(struct in_addr *)&iphdr->daddr), ntohs(tcphdr->dest),
			(tcphdr->fin ? 'F' : '.'), (tcphdr->syn ? 'S' : '.'),
			(tcphdr->rst ? 'R' : '.'), (tcphdr->psh ? 'P' : '.'),
			(tcphdr->ack ? 'A' : '.'), (tcphdr->urg ? 'U' : '.'),
			(size_t)buff_len - (iphdr->ihl * 4) - sizeof(struct tcphdr));
		break;
	case IPPROTO_UDP:
		LEN_CHECK((size_t)buff_len - (iphdr->ihl * 4), sizeof(struct udphdr), "UDP");

		udphdr = (struct udphdr *)(packet_buff + (iphdr->ihl * 4));
		printf("IP %s.%i > ", inet_ntoa(*(struct in_addr *)&iphdr->saddr), ntohs(udphdr->source));

		switch (ntohs(udphdr->dest)) {
		case 67:
                        LEN_CHECK((size_t)buff_len - (iphdr->ihl * 4) - sizeof(struct udphdr), (size_t) 44, "DHCP");
			printf("%s.67: BOOTP/DHCP, Request from %s, length %zu\n",
				inet_ntoa(*(struct in_addr *)&iphdr->daddr),
				ether_ntoa_long((struct ether_addr *)(((char *)udphdr) + sizeof(struct udphdr) + 28)),
				(size_t)buff_len - (iphdr->ihl * 4) - sizeof(struct udphdr));
			break;
		case 68:
			printf("%s.68: BOOTP/DHCP, Reply, length %zu\n",
				inet_ntoa(*(struct in_addr *)&iphdr->daddr),
				(size_t)buff_len - (iphdr->ihl * 4) - sizeof(struct udphdr));
			break;
		default:
			printf("%s.%i: UDP, length %zu\n",
				inet_ntoa(*(struct in_addr *)&iphdr->daddr), ntohs(udphdr->dest),
				(size_t)buff_len - (iphdr->ihl * 4) - sizeof(struct udphdr));
			break;
		}

		break;
	case IPPROTO_IPV6:
		printf("IP6: not implemented yet\n");
		break;
	default:
		printf("IP unknown protocol: %i\n", iphdr->protocol);
		break;
	}
}

static void dump_vlan(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct vlanhdr *vlanhdr;

	vlanhdr = (struct vlanhdr *)(packet_buff + sizeof(struct ether_header));
	LEN_CHECK((size_t)buff_len, sizeof(struct ether_header) + sizeof(struct vlanhdr), "VLAN");

	if (!time_printed)
		time_printed = print_time();

	vlanhdr->vid = ntohs(vlanhdr->vid);
	printf("vlan %u, p %u, ", vlanhdr->vid, vlanhdr->vid >> 12);

	/* overwrite vlan tags */
	memmove(packet_buff + 4, packet_buff, 2 * ETH_ALEN);

	parse_eth_hdr(packet_buff + 4, buff_len - 4, read_opt, time_printed);
}

static void dump_batman_ogm(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct ether_header *ether_header;
	struct batman_packet *batman_packet;

	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header), sizeof(struct batman_packet), "BAT OGM");

	ether_header = (struct ether_header *)packet_buff;
	batman_packet = (struct batman_packet *)(packet_buff + sizeof(struct ether_header));

	if (!time_printed)
		print_time();

	printf("BAT %s: ",
	       get_name_by_macaddr((struct ether_addr *)batman_packet->orig, read_opt));

	printf("OGM via neigh %s, seq %u, tq %3d, ttl %2d, v %d, flags [%c%c%c%c], length %zu\n",
	        get_name_by_macaddr((struct ether_addr *)ether_header->ether_shost, read_opt),
	        ntohl(batman_packet->seqno), batman_packet->tq,
	        batman_packet->ttl, batman_packet->version,
	        (batman_packet->flags & DIRECTLINK ? 'D' : '.'),
	        (batman_packet->flags & VIS_SERVER ? 'V' : '.'),
	        (batman_packet->flags & PRIMARIES_FIRST_HOP ? 'F' : '.'),
	        (batman_packet->gw_flags ? 'G' : '.'),
	        (size_t)buff_len - sizeof(struct ether_header));
}

static void dump_batman_icmp(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct icmp_packet *icmp_packet;
	char *name;

	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header), sizeof(struct icmp_packet), "BAT ICMP");

	icmp_packet = (struct icmp_packet *)(packet_buff + sizeof(struct ether_header));

	if (!time_printed)
		print_time();

	printf("BAT %s > ", get_name_by_macaddr((struct ether_addr *)icmp_packet->orig, read_opt));

	name = get_name_by_macaddr((struct ether_addr *)icmp_packet->dst, read_opt);

	switch (icmp_packet->msg_type) {
	case ECHO_REPLY:
		printf("%s: ICMP echo reply, id %hhu, seq %hu, ttl %2d, v %d, length %zu\n",
			name, icmp_packet->uid, ntohs(icmp_packet->seqno),
			icmp_packet->ttl, icmp_packet->version,
			(size_t)buff_len - sizeof(struct ether_header));
		break;
	case ECHO_REQUEST:
		printf("%s: ICMP echo request, id %hhu, seq %hu, ttl %2d, v %d, length %zu\n",
			name, icmp_packet->uid, ntohs(icmp_packet->seqno),
			icmp_packet->ttl, icmp_packet->version,
			(size_t)buff_len - sizeof(struct ether_header));
		break;
	case TTL_EXCEEDED:
		printf("%s: ICMP time exceeded in-transit, id %hhu, seq %hu, ttl %2d, v %d, length %zu\n",
			name, icmp_packet->uid, ntohs(icmp_packet->seqno),
			icmp_packet->ttl, icmp_packet->version,
			(size_t)buff_len - sizeof(struct ether_header));
		break;
	default:
		printf("%s: ICMP type %hhu, length %zu\n",
			name, icmp_packet->msg_type, (size_t)buff_len - sizeof(struct ether_header));
		break;
	}
}

static void dump_batman_ucast(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct ether_header *ether_header;
	struct unicast_packet *unicast_packet;

	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header), sizeof(struct unicast_packet), "BAT UCAST");
	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header) - sizeof(struct unicast_packet),
		sizeof(struct ether_header), "BAT UCAST (unpacked)");

	ether_header = (struct ether_header *)packet_buff;
	unicast_packet = (struct unicast_packet *)(packet_buff + sizeof(struct ether_header));

	if (!time_printed)
		time_printed = print_time();

	printf("BAT %s > ",
	       get_name_by_macaddr((struct ether_addr *)ether_header->ether_shost, read_opt));

	printf("%s: UCAST, ttl %hhu, ",
	       get_name_by_macaddr((struct ether_addr *)unicast_packet->dest, read_opt),
	       unicast_packet->ttl);

	parse_eth_hdr(packet_buff + ETH_HLEN + sizeof(struct unicast_packet),
		      buff_len - ETH_HLEN - sizeof(struct unicast_packet),
		      read_opt, time_printed);
}

static void dump_batman_bcast(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct ether_header *ether_header;
	struct bcast_packet *bcast_packet;

	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header), sizeof(struct bcast_packet), "BAT BCAST");
	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header) - sizeof(struct bcast_packet),
	          sizeof(struct ether_header), "BAT BCAST (unpacked)");

	ether_header = (struct ether_header *)packet_buff;
	bcast_packet = (struct bcast_packet *)(packet_buff + sizeof(struct ether_header));

	if (!time_printed)
		time_printed = print_time();

	printf("BAT %s: ",
	       get_name_by_macaddr((struct ether_addr *)ether_header->ether_shost, read_opt));

	printf("BCAST, orig %s, seq %u, ",
	       get_name_by_macaddr((struct ether_addr *)bcast_packet->orig, read_opt),
	       ntohl(bcast_packet->seqno));

	parse_eth_hdr(packet_buff + ETH_HLEN + sizeof(struct bcast_packet),
		      buff_len - ETH_HLEN - sizeof(struct bcast_packet),
		      read_opt, time_printed);
}

static void dump_batman_frag(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct unicast_frag_packet *unicast_frag_packet;

	LEN_CHECK((size_t)buff_len - ETH_HLEN, sizeof(struct unicast_frag_packet), "BAT FRAG");
	LEN_CHECK((size_t)buff_len - ETH_HLEN - sizeof(struct unicast_frag_packet), (size_t)ETH_HLEN, "BAT FRAG (unpacked)");

	unicast_frag_packet = (struct unicast_frag_packet *)(packet_buff + ETH_HLEN);

	if (!time_printed)
		time_printed = print_time();

	printf("BAT %s > ",
	       get_name_by_macaddr((struct ether_addr *)unicast_frag_packet->orig, read_opt));

	printf("%s: FRAG, seq %hu, ttl %hhu, flags [%c%c], ",
	       get_name_by_macaddr((struct ether_addr *)unicast_frag_packet->dest, read_opt),
	       ntohs(unicast_frag_packet->seqno), unicast_frag_packet->ttl,
	       (unicast_frag_packet->flags & UNI_FRAG_HEAD ? 'H' : '.'),
	       (unicast_frag_packet->flags & UNI_FRAG_LARGETAIL ? 'L' : '.'));

	if (unicast_frag_packet->flags & UNI_FRAG_HEAD)
		parse_eth_hdr(packet_buff + ETH_HLEN + sizeof(struct unicast_frag_packet),
			      buff_len - ETH_HLEN - sizeof(struct unicast_frag_packet),
			      read_opt, time_printed);
	else
		printf("length %zu\n", (size_t)buff_len - ETH_HLEN - sizeof(struct unicast_frag_packet));
}

static void parse_eth_hdr(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct batman_packet *batman_packet;
	struct ether_header *eth_hdr;

	eth_hdr = (struct ether_header *)packet_buff;

	switch (ntohs(eth_hdr->ether_type)) {
	case ETH_P_ARP:
		if ((dump_level & DUMP_TYPE_NONBAT) || (time_printed))
			dump_arp(packet_buff + ETH_HLEN, buff_len - ETH_HLEN, time_printed);
		break;
	case ETH_P_IP:
		if ((dump_level & DUMP_TYPE_NONBAT) || (time_printed))
			dump_ip(packet_buff + ETH_HLEN, buff_len - ETH_HLEN, time_printed);
		break;
	case ETH_P_8021Q:
		if ((dump_level & DUMP_TYPE_NONBAT) || (time_printed))
			dump_vlan(packet_buff, buff_len, read_opt, time_printed);
		break;
	case ETH_P_BATMAN:
		batman_packet = (struct batman_packet *)(packet_buff + ETH_HLEN);

		switch (batman_packet->packet_type) {
		case BAT_PACKET:
			if (dump_level & DUMP_TYPE_BATOGM)
				dump_batman_ogm(packet_buff, buff_len, read_opt, time_printed);
			break;
		case BAT_ICMP:
			if (dump_level & DUMP_TYPE_BATICMP)
				dump_batman_icmp(packet_buff, buff_len, read_opt, time_printed);
			break;
		case BAT_UNICAST:
			if (dump_level & DUMP_TYPE_BATUCAST)
				dump_batman_ucast(packet_buff, buff_len, read_opt, time_printed);
			break;
		case BAT_BCAST:
			if (dump_level & DUMP_TYPE_BATBCAST)
				dump_batman_bcast(packet_buff, buff_len, read_opt, time_printed);
			break;
		case BAT_VIS:
			if (dump_level & DUMP_TYPE_BATVIS)
				printf("Warning - batman vis packet received: function not implemented yet\n");
			break;
		case BAT_UNICAST_FRAG:
			if (dump_level & DUMP_TYPE_BATFRAG)
				dump_batman_frag(packet_buff, buff_len, read_opt, time_printed);
			break;
		default:
			printf("Warning - packet contains unknown batman packet type: 0x%02x\n", batman_packet->packet_type);
			break;
		}

		break;

	default:
		printf("Warning - packet contains unknown ether type: 0x%04x\n", ntohs(eth_hdr->ether_type));
		break;
	}
}

int tcpdump(int argc, char **argv)
{
	struct ifreq req;
	struct timeval tv;
	struct dump_if *dump_if, *dump_if_tmp;
	struct list_head_first dump_if_list;
	fd_set wait_sockets, tmp_wait_sockets;
	ssize_t read_len;
	int ret = EXIT_FAILURE, res, optchar, found_args = 1, max_sock = 0, tmp;
	int read_opt = USE_BAT_HOSTS;
	unsigned char packet_buff[2000];

	while ((optchar = getopt(argc, argv, "hnp:")) != -1) {
		switch (optchar) {
		case 'h':
			tcpdump_usage();
			return EXIT_SUCCESS;
		case 'n':
			read_opt &= ~USE_BAT_HOSTS;
			found_args += 1;
			break;
		case 'p':
			tmp = strtol(optarg, NULL , 10);
			if ((tmp > 0) && (tmp <= dump_level))
				dump_level = tmp;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		default:
			tcpdump_usage();
			return EXIT_FAILURE;
		}
	}

	if (argc <= found_args) {
		printf("Error - target interface not specified\n");
		tcpdump_usage();
		return EXIT_FAILURE;
	}

	bat_hosts_init(read_opt);

	/* init interfaces list */
	INIT_LIST_HEAD_FIRST(dump_if_list);
	FD_ZERO(&wait_sockets);

	while (argc > found_args) {

		dump_if = malloc(sizeof(struct dump_if));
		memset(dump_if, 0, sizeof(struct dump_if));
		INIT_LIST_HEAD(&dump_if->list);

		dump_if->dev = argv[found_args];

		if (strlen(dump_if->dev) > IFNAMSIZ - 1) {
			printf("Error - interface name too long: %s\n", dump_if->dev);
			goto out;
		}

		dump_if->raw_sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

		if (dump_if->raw_sock < 0) {
			printf("Error - can't create raw socket: %s\n", strerror(errno));
			goto out;
		}

		memset(&req, 0, sizeof (struct ifreq));
		strncpy(req.ifr_name, dump_if->dev, IFNAMSIZ);

		res = ioctl(dump_if->raw_sock, SIOCGIFINDEX, &req);

		if (res < 0) {
			printf("Error - can't create raw socket (SIOCGIFINDEX): %s\n", strerror(errno));
			close(dump_if->raw_sock);
			goto out;
		}

		dump_if->addr.sll_family   = AF_PACKET;
		dump_if->addr.sll_protocol = htons(ETH_P_ALL);
		dump_if->addr.sll_ifindex  = req.ifr_ifindex;

		res = bind(dump_if->raw_sock, (struct sockaddr *)&dump_if->addr, sizeof(struct sockaddr_ll));

		if (res < 0) {
			printf("Error - can't bind raw socket: %s\n", strerror(errno));
			close(dump_if->raw_sock);
			goto out;
		}

		if (dump_if->raw_sock > max_sock)
			max_sock = dump_if->raw_sock;

		FD_SET(dump_if->raw_sock, &wait_sockets);
		list_add_tail(&dump_if->list, &dump_if_list);
		found_args++;
	}

	while (1) {

		memcpy(&tmp_wait_sockets, &wait_sockets, sizeof(fd_set));

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		res = select(max_sock + 1, &tmp_wait_sockets, NULL, NULL, &tv);

		if (res == 0)
			continue;

		if (res < 0) {
			printf("Error - can't select on raw socket: %s\n", strerror(errno));
			continue;
		}

		list_for_each_entry(dump_if, &dump_if_list, list) {
			if (!FD_ISSET(dump_if->raw_sock, &tmp_wait_sockets))
				continue;

			read_len = read(dump_if->raw_sock, packet_buff, sizeof(packet_buff));

			if (read_len < 0) {
				printf("Error - can't read from interface '%s': %s\n", dump_if->dev, strerror(errno));
				continue;
			}

			if ((size_t)read_len < sizeof(struct ether_header)) {
				printf("Warning - dropping received packet as it is smaller than expected (%zu): %zd\n",
					sizeof(struct ether_header), read_len);
				continue;
			}

			parse_eth_hdr(packet_buff, read_len, read_opt, 0);
			fflush(stdout);
		}

	}

out:
	list_for_each_entry_safe(dump_if, dump_if_tmp, &dump_if_list, list) {
		if (dump_if->raw_sock)
			close(dump_if->raw_sock);

		list_del((struct list_head *)&dump_if_list, &dump_if->list, &dump_if_list);
		free(dump_if);
	}

	bat_hosts_free();
	return ret;
}
