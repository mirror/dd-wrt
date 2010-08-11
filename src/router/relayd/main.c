/*
 *   Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License v2 as published by
 *   the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 */
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "relayd.h"

LIST_HEAD(interfaces);
int debug;

static int host_timeout;
static int inet_sock;
static int forward_bcast;
static int forward_dhcp;

static struct relayd_host *find_host_by_ipaddr(struct relayd_interface *rif, const uint8_t *ipaddr)
{
	struct relayd_host *host;

	if (!rif) {
		list_for_each_entry(rif, &interfaces, list) {
			host = find_host_by_ipaddr(rif, ipaddr);
			if (!host)
				continue;

			return host;
		}
		return NULL;
	}

	list_for_each_entry(host, &rif->hosts, list) {
		if (memcmp(ipaddr, host->ipaddr, sizeof(host->ipaddr)) != 0)
			continue;

		return host;
	}
	return NULL;
}

static void add_arp(struct relayd_host *host)
{
	struct sockaddr_in *sin;
	struct arpreq arp;

	strncpy(arp.arp_dev, host->rif->ifname, sizeof(arp.arp_dev));
	arp.arp_flags = ATF_COM;

	arp.arp_ha.sa_family = ARPHRD_ETHER;
	memcpy(arp.arp_ha.sa_data, host->lladdr, ETH_ALEN);

	sin = (struct sockaddr_in *) &arp.arp_pa;
	sin->sin_family = AF_INET;
	memcpy(&sin->sin_addr, host->ipaddr, sizeof(host->ipaddr));

	ioctl(inet_sock, SIOCSARP, &arp);
}

static void del_host(struct relayd_host *host)
{
	DPRINTF(1, "%s: deleting host "IP_FMT" ("MAC_FMT")\n", host->rif->ifname,
		IP_BUF(host->ipaddr), MAC_BUF(host->lladdr));

	if (host->rif->managed)
		relayd_del_route(host);
	uloop_timeout_cancel(&host->timeout);
	list_del(&host->list);
	free(host);
}

static void fill_arp_request(struct arp_packet *pkt, struct relayd_interface *rif,
                             uint8_t spa[4], uint8_t tpa[4])
{
	memset(pkt, 0, sizeof(*pkt));

	pkt->eth.ether_type = htons(ETHERTYPE_ARP);
	memcpy(pkt->eth.ether_shost, rif->sll.sll_addr, ETH_ALEN);

	memcpy(pkt->arp.arp_sha, rif->sll.sll_addr, ETH_ALEN);
	memcpy(pkt->arp.arp_spa, spa, 4);
	memcpy(pkt->arp.arp_tpa, tpa, 4);

	pkt->arp.arp_hrd = htons(ARPHRD_ETHER);
	pkt->arp.arp_pro = htons(ETH_P_IP);
	pkt->arp.arp_hln = ETH_ALEN;
	pkt->arp.arp_pln = 4;
}

static void send_arp_request(struct relayd_host *host)
{
	struct relayd_interface *rif = host->rif;
	struct arp_packet pkt;

	fill_arp_request(&pkt, host->rif, host->rif->src_ip, host->ipaddr);

	pkt.arp.arp_op = htons(ARPOP_REQUEST);
	memcpy(pkt.arp.arp_spa, rif->src_ip, ETH_ALEN);
	memset(pkt.arp.arp_tha, 0, ETH_ALEN);
	memset(pkt.eth.ether_dhost, 0xff, ETH_ALEN);

	DPRINTF(2, "%s: sending ARP who-has "IP_FMT", tell "IP_FMT" ("MAC_FMT")\n",
		rif->ifname, IP_BUF(pkt.arp.arp_tpa),
		IP_BUF(pkt.arp.arp_spa), MAC_BUF(pkt.eth.ether_shost));

	sendto(rif->fd.fd, &pkt, sizeof(pkt), 0,
		(struct sockaddr *) &rif->sll, sizeof(rif->sll));
}

static void send_arp_reply(struct relayd_interface *rif, uint8_t spa[4],
                           uint8_t tha[ETH_ALEN], uint8_t tpa[4])
{
	struct arp_packet pkt;

	fill_arp_request(&pkt, rif, spa, tpa);

	pkt.arp.arp_op = htons(ARPOP_REPLY);
	memcpy(pkt.eth.ether_dhost, tha, ETH_ALEN);
	memcpy(pkt.arp.arp_tha, tha, ETH_ALEN);

	DPRINTF(2, "%s: sending ARP reply to "IP_FMT", "IP_FMT" is at ("MAC_FMT")\n",
		rif->ifname, IP_BUF(pkt.arp.arp_tpa),
		IP_BUF(pkt.arp.arp_spa), MAC_BUF(pkt.eth.ether_shost));

	sendto(rif->fd.fd, &pkt, sizeof(pkt), 0,
		(struct sockaddr *) &rif->sll, sizeof(rif->sll));
}

static void host_entry_timeout(struct uloop_timeout *timeout)
{
	struct relayd_host *host = container_of(timeout, struct relayd_host, timeout);

	/*
	 * When a host is behind a managed interface, we must not expire its host
	 * entry prematurely, as this will cause routes to the node to expire,
	 * leading to loss of connectivity from the other side.
	 * When the timeout is reached, try pinging the host a few times before
	 * giving up on it.
	 */
	if (host->rif->managed && host->cleanup_pending < 2) {
		send_arp_request(host);
		host->cleanup_pending++;
		uloop_timeout_set(&host->timeout, 1000);
		return;
	}
	del_host(host);
}

static struct relayd_host *add_host(struct relayd_interface *rif, const uint8_t *lladdr, const uint8_t *ipaddr)
{
	struct relayd_host *host;

	DPRINTF(1, "%s: adding host "IP_FMT" ("MAC_FMT")\n", rif->ifname,
			IP_BUF(ipaddr), MAC_BUF(lladdr));

	host = calloc(1, sizeof(*host));
	host->rif = rif;
	memcpy(host->ipaddr, ipaddr, sizeof(host->ipaddr));
	memcpy(host->lladdr, lladdr, sizeof(host->lladdr));
	list_add(&host->list, &rif->hosts);
	host->timeout.cb = host_entry_timeout;
	uloop_timeout_set(&host->timeout, host_timeout * 1000);

	add_arp(host);
	if (rif->managed)
		relayd_add_route(host);

	return host;
}

struct relayd_host *relayd_refresh_host(struct relayd_interface *rif, const uint8_t *lladdr, const uint8_t *ipaddr)
{
	struct relayd_host *host;

	host = find_host_by_ipaddr(rif, ipaddr);
	if (!host) {
		host = find_host_by_ipaddr(NULL, ipaddr);

		/* 
		 * When we suddenly see the host appearing on a different interface,
		 * reduce the timeout to make the old entry expire faster, in case the
		 * host has moved.
		 * If the old entry is behind a managed interface, it will be pinged
		 * before we expire it
		 */
		if (host && !host->cleanup_pending)
			uloop_timeout_set(&host->timeout, 1);

		host = add_host(rif, lladdr, ipaddr);
	} else {
		host->cleanup_pending = false;
		uloop_timeout_set(&host->timeout, host_timeout * 1000);
	}

	return host;
}

static void relay_arp_request(struct relayd_interface *from_rif, struct arp_packet *pkt)
{
	struct relayd_interface *rif;
	struct arp_packet reqpkt;

	memcpy(&reqpkt, pkt, sizeof(reqpkt));
	list_for_each_entry(rif, &interfaces, list) {
		if (rif == from_rif)
			continue;

		memcpy(reqpkt.eth.ether_shost, rif->sll.sll_addr, ETH_ALEN);
		memcpy(reqpkt.arp.arp_sha, rif->sll.sll_addr, ETH_ALEN);

		DPRINTF(2, "%s: sending ARP who-has "IP_FMT", tell "IP_FMT" ("MAC_FMT")\n",
			rif->ifname, IP_BUF(reqpkt.arp.arp_tpa),
			IP_BUF(reqpkt.arp.arp_spa), MAC_BUF(reqpkt.eth.ether_shost));

		sendto(rif->fd.fd, &reqpkt, sizeof(reqpkt), 0,
			(struct sockaddr *) &rif->sll, sizeof(rif->sll));
	}
}

static void recv_arp_request(struct relayd_interface *rif, struct arp_packet *pkt)
{
	struct relayd_host *host;

	DPRINTF(2, "%s: ARP who-has "IP_FMT", tell "IP_FMT" ("MAC_FMT")\n",
		rif->ifname,
		IP_BUF(pkt->arp.arp_tpa),
		IP_BUF(pkt->arp.arp_spa),
		MAC_BUF(pkt->eth.ether_shost));

	if (!memcmp(pkt->arp.arp_spa, "\x00\x00\x00\x00", 4))
		return;

	relayd_refresh_host(rif, pkt->eth.ether_shost, pkt->arp.arp_spa);

	host = find_host_by_ipaddr(NULL, pkt->arp.arp_tpa);

	/*
	 * If a host is being pinged because of a timeout, do not use the cached
	 * entry here. That way we can avoid giving out stale data in case the node
	 * has moved. We shouldn't relay requests here either, as we might miss our
	 * chance to create a host route.
	 */
	if (host && host->cleanup_pending)
		return;

	relay_arp_request(rif, pkt);
}


static void recv_arp_reply(struct relayd_interface *rif, struct arp_packet *pkt)
{
	struct relayd_host *host;

	DPRINTF(2, "%s: received ARP reply for "IP_FMT" from "MAC_FMT", deliver to "IP_FMT"\n",
		rif->ifname,
		IP_BUF(pkt->arp.arp_spa),
		MAC_BUF(pkt->eth.ether_shost),
		IP_BUF(pkt->arp.arp_tpa));

	relayd_refresh_host(rif, pkt->arp.arp_sha, pkt->arp.arp_spa);

	if (!memcmp(pkt->arp.arp_tpa, rif->src_ip, 4))
		return;

	host = find_host_by_ipaddr(NULL, pkt->arp.arp_tpa);
	if (!host)
		return;

	if (host->rif == rif)
		return;

	send_arp_reply(host->rif, pkt->arp.arp_spa, host->lladdr, host->ipaddr);
}

static void recv_packet(struct uloop_fd *fd, unsigned int events)
{
	struct relayd_interface *rif = container_of(fd, struct relayd_interface, fd);
	struct arp_packet *pkt;
	static char pktbuf[4096];
	int pktlen;

	do {
		if (rif->fd.error)
			uloop_end();

		pktlen = recv(rif->fd.fd, pktbuf, sizeof(pktbuf), 0);
		if (pktlen < 0) {
			if (errno == EINTR)
				continue;

			break;
		}

		if (!pktlen)
			break;

		pkt = (void *)pktbuf;
		if (pkt->arp.arp_op == htons(ARPOP_REPLY))
			recv_arp_reply(rif, pkt);
		else if (pkt->arp.arp_op == htons(ARPOP_REQUEST))
			recv_arp_request(rif, pkt);
		else
			DPRINTF(1, "received unknown packet type: %04x\n", ntohs(pkt->arp.arp_op));

	} while (1);
}

static void forward_bcast_packet(struct relayd_interface *from_rif, void *packet, int len)
{
	struct relayd_interface *rif;
	struct ether_header *eth = packet;

	list_for_each_entry(rif, &interfaces, list) {
		if (rif == from_rif)
			continue;

		DPRINTF(3, "%s: forwarding broadcast packet to %s\n", from_rif->ifname, rif->ifname);
		memcpy(eth->ether_shost, rif->sll.sll_addr, ETH_ALEN);
		send(rif->bcast_fd.fd, packet, len, 0);
	}
}

static uint16_t
chksum(uint16_t sum, const uint8_t *data, uint16_t len)
{
	const uint8_t *last;
	uint16_t t;

	last = data + len - 1;

	while(data < last) {
		t = (data[0] << 8) + data[1];
		sum += t;
		if(sum < t)
			sum++;
		data += 2;
	}

	if(data == last) {
		t = (data[0] << 8) + 0;
		sum += t;
		if(sum < t)
			sum++;
	}

	return sum;
}

static bool forward_dhcp_packet(struct relayd_interface *rif, void *data, int len)
{
	struct ip_packet *pkt = data;
	struct udphdr *udp;
	struct dhcp_header *dhcp;
	int udplen;
	uint16_t sum;

	if (pkt->eth.ether_type != htons(ETH_P_IP))
		return false;

	if (pkt->iph.version != 4)
		return false;

	if (pkt->iph.protocol != IPPROTO_UDP)
		return false;

	udp = (void *) ((char *) &pkt->iph + (pkt->iph.ihl << 2));
	dhcp = (void *) (udp + 1);

	udplen = ntohs(udp->len);
	if (udplen > len - ((char *) udp - (char *) data))
		return false;

	if (udp->dest != htons(67) && udp->source != htons(67))
		return false;

	if (dhcp->op != 1 && dhcp->op != 2)
		return false;

	if (!forward_dhcp)
		return true;

	if (dhcp->op == 2)
		relayd_refresh_host(rif, pkt->eth.ether_shost, (void *) &pkt->iph.saddr);

	DPRINTF(2, "%s: handling DHCP %s\n", rif->ifname, (dhcp->op == 1 ? "request" : "response"));

	dhcp->flags |= htons(DHCP_FLAG_BROADCAST);

	udp->check = 0;
	sum = udplen + IPPROTO_UDP;
	sum = chksum(sum, (void *) &pkt->iph.saddr, 8);
	sum = chksum(sum, (void *) udp, udplen);
	if (sum == 0)
		sum = 0xffff;

	udp->check = htons(~sum);

	forward_bcast_packet(rif, data, len);

	return true;
}

static void recv_bcast_packet(struct uloop_fd *fd, unsigned int events)
{
	struct relayd_interface *rif = container_of(fd, struct relayd_interface, bcast_fd);
	static char pktbuf[4096];
	int pktlen;

	do {
		if (rif->fd.error)
			uloop_end();

		pktlen = recv(rif->bcast_fd.fd, pktbuf, sizeof(pktbuf), 0);
		if (pktlen < 0) {
			if (errno == EINTR)
				continue;

			break;
		}

		if (!pktlen)
			break;

		if (!forward_bcast && !forward_dhcp)
			continue;

		if (forward_dhcp_packet(rif, pktbuf, pktlen))
			continue;

		if (forward_bcast)
			forward_bcast_packet(rif, pktbuf, pktlen);
	} while (1);
}


static int init_interface(struct relayd_interface *rif)
{
	struct sockaddr_ll *sll = &rif->sll;
	struct sockaddr_in *sin;
	struct ifreq ifr;
	int fd = rif->fd.fd;
#ifdef PACKET_RECV_TYPE
	unsigned int pkt_type;
#endif

	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (fd < 0)
		return -1;

	rif->fd.fd = fd;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, rif->ifname);

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("ioctl(SIOCGIFHWADDR)");
		return -1;
	}

	memcpy(sll->sll_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	sll->sll_family = AF_PACKET;
	sll->sll_protocol = htons(ETH_P_ARP);
	sll->sll_pkttype = PACKET_BROADCAST;
	sll->sll_hatype = ARPHRD_ETHER;
	sll->sll_halen = ETH_ALEN;

	if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
		perror("ioctl(SIOCGIFINDEX)");
		return -1;
	}

	sll->sll_ifindex = ifr.ifr_ifindex;

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		memcpy(rif->src_ip, DUMMY_IP, sizeof(rif->src_ip));
	} else {
		sin = (struct sockaddr_in *) &ifr.ifr_addr;
		memcpy(rif->src_ip, &sin->sin_addr.s_addr, sizeof(rif->src_ip));
	}

	if (bind(fd, (struct sockaddr *)sll, sizeof(struct sockaddr_ll)) < 0) {
		perror("bind(ETH_P_ARP)");
		return -1;
	}

	rif->fd.cb = recv_packet;
	uloop_fd_add(&rif->fd, ULOOP_READ | ULOOP_EDGE_TRIGGER);

	if (!forward_bcast && !forward_dhcp)
		return 0;

	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	if (fd < 0)
		return 0;

	rif->bcast_fd.fd = fd;
	rif->bcast_fd.cb = recv_bcast_packet;

	memcpy(&rif->bcast_sll, &rif->sll, sizeof(rif->bcast_sll));
	sll = &rif->bcast_sll;
	sll->sll_protocol = htons(ETH_P_IP);

	if (bind(fd, (struct sockaddr *)sll, sizeof(struct sockaddr_ll)) < 0) {
		perror("bind(ETH_P_IP)");
		return 0;
	}

#ifdef PACKET_RECV_TYPE
	pkt_type = (1 << PACKET_BROADCAST);
	setsockopt(fd, SOL_PACKET, PACKET_RECV_TYPE, &pkt_type, sizeof(pkt_type));
#endif

	uloop_fd_add(&rif->bcast_fd, ULOOP_READ | ULOOP_EDGE_TRIGGER);
	relayd_add_interface_routes(rif);
	return 0;
}

static int init_interfaces(void)
{
	struct relayd_interface *rif;
	int ret;

	list_for_each_entry(rif, &interfaces, list) {
		ret = init_interface(rif);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static void cleanup_hosts(void)
{
	struct relayd_interface *rif;
	struct relayd_host *host, *tmp;

	list_for_each_entry(rif, &interfaces, list) {
		list_for_each_entry_safe(host, tmp, &rif->hosts, list) {
			del_host(host);
		}
	}
}

static void free_interfaces(void)
{
	struct relayd_interface *rif, *rtmp;

	list_for_each_entry_safe(rif, rtmp, &interfaces, list) {
		relayd_del_interface_routes(rif);
		list_del(&rif->list);
		free(rif);
	}
}

static int alloc_interface(const char *ifname, bool managed)
{
	struct relayd_interface *rif;

	if (strlen(ifname) >= IFNAMSIZ)
		return -1;

	rif = calloc(1, sizeof(*rif));
	if (!rif)
		return -1;

	INIT_LIST_HEAD(&rif->list);
	INIT_LIST_HEAD(&rif->hosts);
	strcpy(rif->ifname, ifname);
	list_add(&rif->list, &interfaces);
	rif->managed = managed;

	return 0;
}

static void die(int signo)
{
	/*
	 * When we hit SIGTERM, clean up interfaces directly, so that we
	 * won't leave our routing in an invalid state.
	 */
	cleanup_hosts();
	free_interfaces();
	exit(1);
}

static int usage(const char *progname)
{
	fprintf(stderr, "Usage: %s <options>\n"
			"\n"
			"Options:\n"
			"	-d		Enable debug messages\n"
			"	-i <ifname>	Add an interface for relaying\n"
			"	-I <ifname>	Same as -i, except with ARP cache and host route management\n"
			"			You need to specify at least two interfaces\n"
			"	-t <timeout>	Host entry expiry timeout\n"
			"	-T <table>	Set routing table number for automatically added routes\n"
			"	-B		Enable broadcast forwarding\n"
			"	-D		Enable DHCP forwarding\n"
			"\n",
		progname);
	return -1;
}

int main(int argc, char **argv)
{
	bool managed;
	int ifnum = 0;
	int ch;

	debug = 0;
	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (inet_sock < 0) {
		perror("socket(AF_INET)");
		return 1;
	}

	host_timeout = 60;
	forward_bcast = 0;
	uloop_init();

	while ((ch = getopt(argc, argv, "I:i:t:BDdT:")) != -1) {
		switch(ch) {
		case 'I':
			managed = true;
			/* fall through */
		case 'i':
			ifnum++;
			if (alloc_interface(optarg, managed) < 0)
				return 1;

			managed = false;
			break;
		case 't':
			host_timeout = atoi(optarg);
			if (host_timeout <= 0)
				return usage(argv[0]);
			break;
		case 'd':
			debug++;
			break;
		case 'B':
			forward_bcast = 1;
			break;
		case 'D':
			forward_dhcp = 1;
			break;
		case 'T':
			route_table = atoi(optarg);
			if (route_table <= 0)
				return usage(argv[0]);
			break;
		case '?':
		default:
			return usage(argv[0]);
		}
	}

	if (list_empty(&interfaces))
		return usage(argv[0]);

	if (ifnum < 2) {
		fprintf(stderr, "ERROR: Need at least 2 interfaces for relaying\n");
		return -1;
	}

	argc -= optind;
	argv += optind;

	signal(SIGTERM, die);
	signal(SIGHUP, die);
	signal(SIGUSR1, die);
	signal(SIGUSR2, die);

	if (relayd_rtnl_init() < 0)
		return 1;

	if (init_interfaces() < 0)
		return 1;

	uloop_run();
	uloop_done();

	cleanup_hosts();
	free_interfaces();
	relayd_rtnl_done();
	close(inet_sock);

	return 0;
}
