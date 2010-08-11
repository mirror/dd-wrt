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

#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <linux/if_packet.h>
#include <linux/rtnetlink.h>
#include <linux/neighbour.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>

#include "uloop.h"
#include "list.h"

#define DEBUG
#ifdef DEBUG
#define DPRINTF(level, ...) if (debug >= level) fprintf(stderr, __VA_ARGS__);
#else
#define DPRINTF(...) do {} while(0)
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#define __uc(c) ((unsigned char *)(c))

#define MAC_FMT	"%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_BUF(_c) __uc(_c)[0], __uc(_c)[1], __uc(_c)[2], __uc(_c)[3], __uc(_c)[4], __uc(_c)[5]

#define IP_FMT	"%d.%d.%d.%d"
#define IP_BUF(_c) __uc(_c)[0], __uc(_c)[1], __uc(_c)[2], __uc(_c)[3]

#define DUMMY_IP ((uint8_t *) "\x01\x01\x01\x01")

#define DHCP_FLAG_BROADCAST	(1 << 15)

struct relayd_interface {
	struct list_head list;
	struct uloop_fd fd;
	struct uloop_fd bcast_fd;
	struct sockaddr_ll sll;
	struct sockaddr_ll bcast_sll;
	char ifname[IFNAMSIZ];
	struct list_head hosts;
	uint8_t src_ip[4];
	bool managed;
};

struct relayd_host {
	struct list_head list;
	struct relayd_interface *rif;
	uint8_t lladdr[ETH_ALEN];
	uint8_t ipaddr[4];
	struct uloop_timeout timeout;
	int cleanup_pending;
};

struct arp_packet {
	struct ether_header eth;
	struct ether_arp arp;
} __packed;

struct ip_packet {
	struct ether_header eth;
	struct iphdr iph;
} __packed;

struct dhcp_header {
	uint8_t op, htype, hlen, hops;
	uint32_t xit;
	uint16_t secs, flags;
	struct in_addr ciaddr, yiaddr, siaddr, giaddr;
	unsigned char chaddr[16];
	unsigned char sname[64];
	unsigned char file[128];
} __packed;

struct rtnl_req {
	struct nlmsghdr nl;
	struct rtmsg rt;
};

static int debug;
static LIST_HEAD(interfaces);
static int host_timeout;
static int inet_sock;
static int forward_bcast;
static int forward_dhcp;
static struct uloop_fd rtnl_sock;
static unsigned int rtnl_seq, rtnl_dump_seq;

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

static void rtnl_route_set(struct relayd_host *host, bool add)
{
	static struct {
		struct nlmsghdr nl;
		struct rtmsg rt;
		struct {
			struct rtattr rta;
			uint8_t ipaddr[4];
		} __packed dst;
		struct {
			struct rtattr rta;
			int ifindex;
		} __packed dev;
	} __packed req;

	memset(&req, 0, sizeof(req));

	req.nl.nlmsg_len = sizeof(req);
	req.rt.rtm_family = AF_INET;
	req.rt.rtm_dst_len = 32;

	req.dst.rta.rta_type = RTA_DST;
	req.dst.rta.rta_len = sizeof(req.dst);
	memcpy(req.dst.ipaddr, host->ipaddr, sizeof(req.dst.ipaddr));

	req.dev.rta.rta_type = RTA_OIF;
	req.dev.rta.rta_len = sizeof(req.dev);
	req.dev.ifindex = host->rif->sll.sll_ifindex;

	req.nl.nlmsg_flags = NLM_F_REQUEST;
	req.rt.rtm_table = RT_TABLE_MAIN;
	if (add) {
		req.nl.nlmsg_type = RTM_NEWROUTE;
		req.nl.nlmsg_flags |= NLM_F_CREATE | NLM_F_REPLACE;

		req.rt.rtm_protocol = RTPROT_BOOT;
		req.rt.rtm_scope = RT_SCOPE_LINK;
		req.rt.rtm_type = RTN_UNICAST;
	} else {
		req.nl.nlmsg_type = RTM_DELROUTE;
		req.rt.rtm_scope = RT_SCOPE_NOWHERE;
	}

	send(rtnl_sock.fd, &req, sizeof(req), 0);
}

static void add_route(struct relayd_host *host)
{
	rtnl_route_set(host, true);
}

static void del_route(struct relayd_host *host)
{
	rtnl_route_set(host, false);
}

static void del_host(struct relayd_host *host)
{
	DPRINTF(1, "%s: deleting host "IP_FMT" ("MAC_FMT")\n", host->rif->ifname,
		IP_BUF(host->ipaddr), MAC_BUF(host->lladdr));

	if (host->rif->managed)
		del_route(host);
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
		add_route(host);

	return host;
}

static struct relayd_host *refresh_host(struct relayd_interface *rif, const uint8_t *lladdr, const uint8_t *ipaddr)
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

	refresh_host(rif, pkt->eth.ether_shost, pkt->arp.arp_spa);

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

	refresh_host(rif, pkt->arp.arp_sha, pkt->arp.arp_spa);

	if (!memcmp(pkt->arp.arp_tpa, rif->src_ip, 4))
		return;

	host = find_host_by_ipaddr(NULL, pkt->arp.arp_tpa);
	if (!host)
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

static void del_interface(struct relayd_interface *rif)
{
	struct relayd_host *host, *htmp;

	list_for_each_entry_safe(host, htmp, &rif->hosts, list) {
		del_host(host);
	}
	free(rif);
}

static void cleanup_interfaces(void)
{
	struct relayd_interface *rif, *rtmp;

	list_for_each_entry_safe(rif, rtmp, &interfaces, list) {
		del_interface(rif);
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

#ifndef NDA_RTA
#define NDA_RTA(r) \
    ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#endif

static void rtnl_parse_newneigh(struct nlmsghdr *h)
{
	struct relayd_interface *rif = NULL;
	struct ndmsg *r = NLMSG_DATA(h);
	const uint8_t *lladdr = NULL;
	const uint8_t *ipaddr = NULL;
	struct rtattr *rta;
	int len;

	if (r->ndm_family != AF_INET)
		return;

	list_for_each_entry(rif, &interfaces, list) {
		if (rif->sll.sll_ifindex == r->ndm_ifindex)
			goto found_interface;
	}
	return;

found_interface:
	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(*r));
	for (rta = NDA_RTA(r); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
		switch(rta->rta_type) {
		case NDA_LLADDR:
			lladdr = RTA_DATA(rta);
			break;
		case NDA_DST:
			ipaddr = RTA_DATA(rta);
			break;
		default:
			break;
		}
	}

	if (!lladdr || !ipaddr || (r->ndm_state & (NUD_INCOMPLETE|NUD_FAILED)))
		return;

	if (!memcmp(lladdr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
		return;

	DPRINTF(1, "%s: Found ARP cache entry for host "IP_FMT" ("MAC_FMT")\n",
		rif->ifname, IP_BUF(ipaddr), MAC_BUF(lladdr));
	refresh_host(rif, lladdr, ipaddr);
}

static void rtnl_parse_packet(void *data, int len)
{
	struct nlmsghdr *h;

	for (h = data; NLMSG_OK(h, len); h = NLMSG_NEXT(h, len)) {
		if (h->nlmsg_type == NLMSG_DONE ||
		    h->nlmsg_type == NLMSG_ERROR)
			return;

		if (h->nlmsg_seq != rtnl_dump_seq)
			continue;

		if (h->nlmsg_type == RTM_NEWNEIGH)
			rtnl_parse_newneigh(h);
	}
}

static void rtnl_cb(struct uloop_fd *fd, unsigned int events)
{
	struct sockaddr_nl nladdr;
	static uint8_t buf[16384];
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf),
	};
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	do {
		int len;

		len = recvmsg(rtnl_sock.fd, &msg, 0);
		if (len < 0) {
			if (errno == EINTR)
				continue;

			return;
		}

		if (!len)
			break;

		if (nladdr.nl_pid != 0)
			continue;

		rtnl_parse_packet(buf, len);
	} while (1);
}

static int rtnl_init(void)
{
	struct sockaddr_nl snl_local;
	static struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = RTM_GETNEIGH,
			.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST,
			.nlmsg_pid = 0,
		},
		.g.rtgen_family = AF_INET,
	};

	rtnl_sock.fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (rtnl_sock.fd < 0) {
		perror("socket(AF_NETLINK)");
		return -1;
	}

	snl_local.nl_family = AF_NETLINK;

	if (bind(rtnl_sock.fd, (struct sockaddr *) &snl_local, sizeof(struct sockaddr_nl)) < 0) {
		perror("bind");
		close(rtnl_sock.fd);
		return -1;
	}

	rtnl_sock.cb = rtnl_cb;
	uloop_fd_add(&rtnl_sock, ULOOP_READ | ULOOP_EDGE_TRIGGER);

	rtnl_seq = time(NULL);
	rtnl_dump_seq = rtnl_seq;
	req.nlh.nlmsg_seq = rtnl_seq;
	send(rtnl_sock.fd, &req, sizeof(req), 0);

	return 0;
}

static void die(int signo)
{
	/*
	 * When we hit SIGTERM, clean up interfaces directly, so that we
	 * won't leave our routing in an invalid state.
	 */
	cleanup_interfaces();
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

	while ((ch = getopt(argc, argv, "I:i:t:BDd")) != -1) {
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

	if (init_interfaces() < 0)
		return 1;

	if (rtnl_init() < 0)
		return 1;

	uloop_run();
	uloop_done();

	cleanup_interfaces();
	uloop_fd_delete(&rtnl_sock);
	close(rtnl_sock.fd);
	close(inet_sock);

	return 0;
}
