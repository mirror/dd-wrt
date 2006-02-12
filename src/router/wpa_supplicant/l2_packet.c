/*
 * WPA Supplicant - Layer2 packet handling
 * Copyright (c) 2003-2004, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef CONFIG_NATIVE_WINDOWS
#include <sys/ioctl.h>
#endif /* CONFIG_NATIVE_WINDOWS */
#include <errno.h>
#ifdef USE_DNET_PCAP
#include <pcap.h>
#ifndef CONFIG_WINPCAP
#include <dnet.h>
#endif
#else /* USE_DNET_PCAP */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/if.h>
#endif /* USE_DNET_PCAP */

#include "common.h"
#include "eloop.h"
#include "l2_packet.h"


struct l2_packet_data {
#ifdef USE_DNET_PCAP
	pcap_t *pcap;
#ifndef CONFIG_WINPCAP
	eth_t *eth;
#endif
#else /* USE_DNET_PCAP */
	int fd; /* packet socket for EAPOL frames */
#endif /* USE_DNET_PCAP */
	char ifname[100];
	u8 own_addr[ETH_ALEN];
	void (*rx_callback)(void *ctx, unsigned char *src_addr,
			    unsigned char *buf, size_t len);
	void *rx_callback_ctx;
	int rx_l2_hdr; /* whether to include layer 2 (Ethernet) header in calls
			* to rx_callback */
};


int l2_packet_get_own_addr(struct l2_packet_data *l2, u8 *addr)
{
	memcpy(addr, l2->own_addr, ETH_ALEN);
	return 0;
}


void l2_packet_set_rx_l2_hdr(struct l2_packet_data *l2, int rx_l2_hdr)
{
	l2->rx_l2_hdr = rx_l2_hdr;
}


#ifdef USE_DNET_PCAP

#ifndef CONFIG_WINPCAP
static int l2_packet_init_libdnet(struct l2_packet_data *l2)
{
	eth_addr_t own_addr;

	l2->eth = eth_open(l2->ifname);
	if (!l2->eth) {
		printf("Failed to open interface '%s'.\n", l2->ifname);
		perror("eth_open");
		return -1;
	}

	if (eth_get(l2->eth, &own_addr) < 0) {
		printf("Failed to get own hw address from interface '%s'.\n",
		       l2->ifname);
		perror("eth_get");
		eth_close(l2->eth);
		l2->eth = NULL;
		return -1;
	}
	memcpy(l2->own_addr, own_addr.data, ETH_ALEN);

	return 0;
}
#endif


#ifdef CONFIG_WINPCAP
int pcap_sendpacket(pcap_t *p, u_char *buf, int size);
#endif

int l2_packet_send(struct l2_packet_data *l2, u8 *buf, size_t len)
{
#ifdef CONFIG_WINPCAP
	return pcap_sendpacket(l2->pcap, buf, len);
#else
	return eth_send(l2->eth, buf, len);
#endif
}


static void l2_packet_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct l2_packet_data *l2 = eloop_ctx;
	pcap_t *pcap = sock_ctx;
	struct pcap_pkthdr hdr;
	const u_char *packet;
	struct l2_ethhdr *ethhdr;
	unsigned char *buf;
	size_t len;

	packet = pcap_next(pcap, &hdr);

	if (packet == NULL || hdr.caplen < sizeof(*ethhdr))
		return;

	ethhdr = (struct l2_ethhdr *) packet;
	if (l2->rx_l2_hdr) {
		buf = (unsigned char *) ethhdr;
		len = hdr.caplen;
	} else {
		buf = (unsigned char *) (ethhdr + 1);
		len = hdr.caplen - sizeof(*ethhdr);
	}
	l2->rx_callback(l2->rx_callback_ctx, ethhdr->h_source, buf, len);
}


#ifdef CONFIG_WINPCAP
static void l2_packet_receive_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct l2_packet_data *l2 = eloop_ctx;
	pcap_t *pcap = timeout_ctx;
	/* Register new timeout before calling l2_packet_receive() since
	 * receive handler may free this l2_packet instance (which will
	 * cancel this timeout). */
	eloop_register_timeout(0, 100000, l2_packet_receive_timeout,
			       l2, pcap);
	l2_packet_receive(-1, eloop_ctx, timeout_ctx);
}
#endif /* CONFIG_WINPCAP */


static int l2_packet_init_libpcap(struct l2_packet_data *l2,
				  unsigned short protocol)
{
	bpf_u_int32 pcap_maskp, pcap_netp;
	char pcap_filter[100], pcap_err[PCAP_ERRBUF_SIZE];
	struct bpf_program pcap_fp;

	pcap_lookupnet(l2->ifname, &pcap_netp, &pcap_maskp, pcap_err);
	l2->pcap = pcap_open_live(l2->ifname, 2500, 0, 10, pcap_err);
	if (l2->pcap == NULL) {
		fprintf(stderr, "pcap_open_live: %s\n", pcap_err);
		fprintf(stderr, "ifname='%s'\n", l2->ifname);
		return -1;
	}
#ifndef CONFIG_WINPCAP
	if (pcap_datalink(l2->pcap) != DLT_EN10MB) {
		if (pcap_set_datalink(l2->pcap, DLT_EN10MB) < 0) {
			fprintf(stderr, "pcap_set_datalinke(DLT_EN10MB): %s\n",
				pcap_geterr(l2->pcap));
			return -1;
		}
	}
#endif /* CONFIG_WINPCAP */
	snprintf(pcap_filter, sizeof(pcap_filter),
		 "ether dst " MACSTR " and ether proto 0x%x",
		 MAC2STR(l2->own_addr), protocol);
	if (pcap_compile(l2->pcap, &pcap_fp, pcap_filter, 1, pcap_netp) < 0)
	{
		fprintf(stderr, "pcap_compile: %s\n",
			pcap_geterr(l2->pcap));
		return -1;
	}

	if (pcap_setfilter(l2->pcap, &pcap_fp) < 0) {
		fprintf(stderr, "pcap_setfilter: %s\n",
			pcap_geterr(l2->pcap));
		return -1;
	}

	pcap_freecode(&pcap_fp);
#ifdef BIOCIMMEDIATE
	/*
	 * When libpcap uses BPF we must enable "immediate mode" to
	 * receive frames right away; otherwise the system may
	 * buffer them for us.
	 */
	{
		unsigned int on = 1;
		if (ioctl(pcap_fileno(l2->pcap), BIOCIMMEDIATE, &on) < 0) {
			fprintf(stderr, "%s: cannot enable immediate mode on "
				"interface %s: %s\n",
				__func__, l2->ifname, strerror(errno));
			/* XXX should we fail? */
		}
	}
#endif /* BIOCIMMEDIATE */

#ifdef CONFIG_WINPCAP
	eloop_register_timeout(0, 100000, l2_packet_receive_timeout,
			       l2, l2->pcap);
#else
	eloop_register_read_sock(pcap_get_selectable_fd(l2->pcap),
				 l2_packet_receive, l2, l2->pcap);
#endif

	return 0;
}



struct l2_packet_data * l2_packet_init(
	const char *ifname, const u8 *own_addr, unsigned short protocol,
	void (*rx_callback)(void *ctx, unsigned char *src_addr,
			    unsigned char *buf, size_t len),
	void *rx_callback_ctx)
{
	struct l2_packet_data *l2;

	l2 = malloc(sizeof(struct l2_packet_data));
	if (l2 == NULL)
		return NULL;
	memset(l2, 0, sizeof(*l2));
	strncpy(l2->ifname, ifname, sizeof(l2->ifname));
	l2->rx_callback = rx_callback;
	l2->rx_callback_ctx = rx_callback_ctx;

#ifdef CONFIG_WINPCAP
	if (own_addr)
		memcpy(l2->own_addr, own_addr, ETH_ALEN);
#else
	if (l2_packet_init_libdnet(l2))
		return NULL;
#endif

	if (l2_packet_init_libpcap(l2, protocol)) {
#ifndef CONFIG_WINPCAP
		eth_close(l2->eth);
#endif
		free(l2);
		return NULL;
	}

	return l2;
}


void l2_packet_deinit(struct l2_packet_data *l2)
{
	if (l2 == NULL)
		return;

	if (l2->pcap)
		pcap_close(l2->pcap);
#ifdef CONFIG_WINPCAP
	eloop_cancel_timeout(l2_packet_receive_timeout, l2, l2->pcap);
#else
	if (l2->eth)
		eth_close(l2->eth);
#endif
	free(l2);
}

#else /* USE_DNET_PCAP */

int l2_packet_send(struct l2_packet_data *l2, u8 *buf, size_t len)
{
	int ret;
	ret = send(l2->fd, buf, len, 0);
	if (ret < 0)
		perror("l2_packet_send - send");
	return ret;
}


static void l2_packet_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct l2_packet_data *l2 = eloop_ctx;
	u8 buf[2300];
	int res;
	struct l2_ethhdr *ethhdr;
	unsigned char *pos;
	size_t len;

	res = recv(sock, buf, sizeof(buf), 0);
	if (res < 0) {
		perror("l2_packet_receive - recv");
		return;
	}
	if (res < sizeof(*ethhdr)) {
		printf("l2_packet_receive: Dropped too short %d packet\n",
		       res);
		return;
	}

	ethhdr = (struct l2_ethhdr *) buf;

	if (l2->rx_l2_hdr) {
		pos = (unsigned char *) buf;
		len = res;
	} else {
		pos = (unsigned char *) (ethhdr + 1);
		len = res - sizeof(*ethhdr);
	}
	l2->rx_callback(l2->rx_callback_ctx, ethhdr->h_source, pos, len);
}


struct l2_packet_data * l2_packet_init(
	const char *ifname, const u8 *own_addr, unsigned short protocol,
	void (*rx_callback)(void *ctx, unsigned char *src_addr,
			    unsigned char *buf, size_t len),
	void *rx_callback_ctx)
{
	struct l2_packet_data *l2;
	struct ifreq ifr;
	struct sockaddr_ll ll;

	l2 = malloc(sizeof(struct l2_packet_data));
	if (l2 == NULL)
		return NULL;
	memset(l2, 0, sizeof(*l2));
	strncpy(l2->ifname, ifname, sizeof(l2->ifname));
	l2->rx_callback = rx_callback;
	l2->rx_callback_ctx = rx_callback_ctx;

	l2->fd = socket(PF_PACKET, SOCK_RAW, htons(protocol));
	if (l2->fd < 0) {
		perror("socket(PF_PACKET, SOCK_RAW)");
		free(l2);
		return NULL;
	}
	strncpy(ifr.ifr_name, l2->ifname, sizeof(ifr.ifr_name));
	if (ioctl(l2->fd, SIOCGIFINDEX, &ifr) < 0) {
		perror("ioctl[SIOCGIFINDEX]");
		close(l2->fd);
		free(l2);
		return NULL;
	}

	memset(&ll, 0, sizeof(ll));
	ll.sll_family = PF_PACKET;
	ll.sll_ifindex = ifr.ifr_ifindex;
	ll.sll_protocol = htons(protocol);
	if (bind(l2->fd, (struct sockaddr *) &ll, sizeof(ll)) < 0) {
		perror("bind[PF_PACKET]");
		close(l2->fd);
		free(l2);
		return NULL;
	}

	if (ioctl(l2->fd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("ioctl[SIOCGIFHWADDR]");
		close(l2->fd);
		free(l2);
		return NULL;
	}
	memcpy(l2->own_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	eloop_register_read_sock(l2->fd, l2_packet_receive, l2, NULL);

	return l2;
}


void l2_packet_deinit(struct l2_packet_data *l2)
{
	if (l2 == NULL)
		return;

	if (l2->fd >= 0) {
		eloop_unregister_read_sock(l2->fd);
		close(l2->fd);
	}
		
	free(l2);
}

#endif /* USE_DNET_PCAP */
