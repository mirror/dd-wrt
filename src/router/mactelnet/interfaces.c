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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#ifndef __linux__
#include <net/if_dl.h>
#include <net/bpf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#else
#include <linux/if_packet.h>
#endif
#include "protocol.h"
#include "interfaces.h"

#define _(String) String

struct net_interface *net_get_interface_ptr(struct net_interface *interfaces, int max_devices, char *name, int create) {
	int i;

	for (i = 0; i < max_devices; ++i) {
		if (!interfaces[i].in_use)
			break;
		
		if (strncmp(interfaces[i].name, name, 254) == 0) {
			return interfaces + i;
		}
	}

	if (create && i < max_devices) {
		interfaces[i].in_use = 1;
		strncpy(interfaces[i].name, name, 254);
		interfaces[i].name[254] = '\0';
		return interfaces + i;
	}

	return NULL;
}

#ifdef __linux__
static void net_update_mac(struct net_interface *interfaces, int max_devices) {
	unsigned char emptymac[] = {0, 0, 0, 0, 0, 0};
	struct ifreq ifr;
	int i,tmpsock;

	tmpsock = socket(PF_INET, SOCK_DGRAM, 0);
	if (tmpsock < 0) {
		perror("net_update_mac");
		exit(1);
	}

	for (i = 0; i < max_devices; ++i) {
		if (interfaces[i].in_use) {
			/* Find interface hardware address from device_name */
			strncpy(ifr.ifr_name, interfaces[i].name, 16);
			if (ioctl(tmpsock, SIOCGIFHWADDR, &ifr) == 0) {
				/* Fetch mac address */
				memcpy(interfaces[i].mac_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
				if (memcmp(interfaces[i].mac_addr, &emptymac, ETH_ALEN) != 0) {
					interfaces[i].has_mac = 1;
				}
			}
 
		}
	}
	close(tmpsock);
}

static int get_device_index(char *device_name) {
        struct ifreq ifr;
	int tmpsock;
	
	tmpsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	/* Find interface index from device_name */
	strncpy(ifr.ifr_name, device_name, 16);
	if (ioctl(tmpsock, SIOCGIFINDEX, &ifr) != 0) {
		return -1;
	}

	/* Return interface index */
	return ifr.ifr_ifindex;
}
#endif

int net_get_interfaces(struct net_interface *interfaces, int max_devices) {
	static struct ifaddrs *int_addrs;
	static const struct ifaddrs *int_cursor;
	const struct sockaddr_in *dl_addr;
	int found = 0;

	if (getifaddrs(&int_addrs) < 0) {
		perror("getifaddrs");
		exit(1);
	}

	for (int_cursor = int_addrs; int_cursor != NULL; int_cursor = int_cursor->ifa_next) {
		dl_addr = (const struct sockaddr_in *) int_cursor->ifa_addr;

		if (int_cursor->ifa_addr == NULL)
			continue;

		if (int_cursor->ifa_addr->sa_family == AF_INET) {
			struct net_interface *interface = net_get_interface_ptr(interfaces, max_devices, int_cursor->ifa_name, 1);
			if (interface != NULL) {
				found++;
				memcpy(interface->ipv4_addr, &dl_addr->sin_addr, IPV4_ALEN);
			}
#ifdef __linux__
			interface->ifindex = get_device_index(interface->name);
#endif
		}
#ifndef __linux__
		{
			unsigned char emptymac[] = {0, 0, 0, 0, 0, 0};
			struct sockaddr_dl *sdl = (struct sockaddr_dl *)int_cursor->ifa_addr;
			if (sdl->sdl_alen == ETH_ALEN) {
				struct net_interface *interface = net_get_interface_ptr(interfaces, max_devices, int_cursor->ifa_name, 1);
				memcpy(interface->mac_addr, LLADDR(sdl), ETH_ALEN);
				if (interface != NULL && memcmp(interface->mac_addr, &emptymac, ETH_ALEN) != 0) {
					interface->has_mac = 1;
				}
			}
		}
#endif
	}
	freeifaddrs(int_addrs);

#ifdef __linux__
	net_update_mac(interfaces, max_devices);
#endif

#if 0
	{
		int i = 0;
		for (i = 0; i < max_devices; ++i) {
			if (interfaces[i].in_use) {
				struct in_addr *addr = (struct in_addr *)interfaces[i].ipv4_addr;
				printf("Interface %s:\n", interfaces[i].name);
				printf("\tIP: %s\n", inet_ntoa(*addr));
				printf("\tMAC: %s\n", ether_ntoa((struct ether_addr *)interfaces[i].mac_addr));
#ifdef __linux__
				printf("\tIfIndex: %d\n", interfaces[i].ifindex);
#endif
				printf("\n");
			}
		}
	}
#endif
	return found;
}

unsigned short in_cksum(unsigned short *addr, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;
	
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	
	if (nleft == 1) {
		*(unsigned char *) (&answer) = *(unsigned char *) w;
		sum += answer;
	}
	
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;
	return (answer);
}

unsigned short udp_sum_calc(unsigned char *src_addr,unsigned char *dst_addr, unsigned char *data, unsigned short len) {
	unsigned short prot_udp=17;
	unsigned short padd=0;
	unsigned short word16;
	unsigned int sum = 0;
	int i;

	/* Padding ? */
	padd = (len % 2);
	if (padd) {
		data[len] = 0;
	}

	/* header+data */
	for (i = 0; i < len + padd; i += 2){
		word16 = ((data[i] << 8) & 0xFF00) + (data[i + 1] & 0xFF);
		sum += word16;
	}

	/* source ip */
	for (i = 0; i < IPV4_ALEN; i += 2){
		word16 = ((src_addr[i] << 8) & 0xFF00) + (src_addr[i + 1] & 0xFF);
		sum += word16;
	}

	/* dest ip */
	for (i = 0; i < IPV4_ALEN; i += 2){
		word16 = ((dst_addr[i] << 8) & 0xFF00) + (dst_addr[i + 1] & 0xFF);
		sum += word16;
	}

	sum += prot_udp + len;

	while (sum>>16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	sum = ~sum;

	if (sum == 0)
		sum = 0xFFFF;

	return (unsigned short) sum;
}

int net_init_raw_socket(struct net_interface *interface) {
	int fd;

#ifdef __linux__
	/* Transmit raw packets with this socket */
	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0) {
		perror("raw_socket");
		exit(1);
	}
#else
	/* Transmit raw packets with bpf */
	fd = open("/dev/bpf0", O_RDWR);
	if (fd <= 0) {
		perror("open_bpf");
		exit(1);
	}
#endif

	return fd;
}

int net_send_udp(const int fd, struct net_interface *interface, const unsigned char *sourcemac, const unsigned char *destmac, const struct in_addr *sourceip, const int sourceport, const struct in_addr *destip, const int destport, const unsigned char *data, const int datalen) {
#ifdef __linux__
	struct sockaddr_ll socket_address;
#endif
	/*
	 * Create a buffer for the full ethernet frame
	 * and align header pointers to the correct positions.
	*/
	void* buffer = (void*)malloc(ETH_FRAME_LEN);
	struct ethhdr *eh = (struct ethhdr *)buffer;
	struct iphdr *ip = (struct iphdr *)(buffer + 14);
	struct udphdr *udp = (struct udphdr *)(buffer + 14 + 20);
	unsigned char *rest = (unsigned char *)(buffer + 20 + 14 + sizeof(struct udphdr));

	if (((void *)rest - (void*)buffer) + datalen  > ETH_FRAME_LEN) {
		fprintf(stderr, _("packet size too large\n"));
		free(buffer);
		return 0;
	}

	static unsigned int id = 1;
	int send_result = 0;

	/* Abort if we couldn't allocate enough memory */
	if (buffer == NULL) {
		perror("malloc");
		exit(1);
	}

	/* Init ethernet header */
	memcpy(eh->h_source, sourcemac, ETH_ALEN);
	memcpy(eh->h_dest, destmac, ETH_ALEN);
	eh->h_proto = htons(ETH_P_IP);

#ifdef __linux__
	/* Init SendTo struct */
	socket_address.sll_family   = AF_PACKET;
	socket_address.sll_protocol = htons(ETH_P_IP);
	socket_address.sll_ifindex  = interface->ifindex;
	socket_address.sll_hatype   = ARPHRD_ETHER;
	socket_address.sll_pkttype  = PACKET_OTHERHOST;
	socket_address.sll_halen    = ETH_ALEN;         

	memcpy(socket_address.sll_addr, eh->h_source, ETH_ALEN);
	socket_address.sll_addr[6]  = 0x00;/*not used*/
	socket_address.sll_addr[7]  = 0x00;/*not used*/
#endif

	/* Init IP Header */
	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0x10;
	ip->tot_len = htons(datalen + 8 + 20);
	ip->id = htons(id++);
	ip->frag_off = htons(0x4000);
	ip->ttl = 64;
	ip->protocol = 17; /* UDP */
	ip->check = 0x0000;
	ip->saddr = sourceip->s_addr;
	ip->daddr = destip->s_addr;

	/* Calculate checksum for IP header */
	ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));

	/* Init UDP Header */
	udp->source = htons(sourceport);
	udp->dest = htons(destport);
	udp->len = htons(sizeof(struct udphdr) + datalen);
	udp->check = 0;

	/* Insert actual data */
	memcpy(rest, data, datalen);

	/* Add UDP checksum */
	udp->check = udp_sum_calc((unsigned char *)&(ip->saddr), (unsigned char *)&(ip->daddr), (unsigned char *)udp, sizeof(struct udphdr) + datalen);
	udp->check = htons(udp->check);

#ifdef __linux__
	/* Send the packet */
	send_result = sendto(fd, buffer, datalen + 8 + 14 + 20, 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
	if (send_result == -1)
		perror("sendto");
#else
	{
		struct ifreq req_if;

		/* Pick device to send through */
		strcpy(req_if.ifr_name, interface->name);
		if (ioctl(fd, BIOCSETIF, &req_if) > 0) {
			perror("ioctl_BIOCSETIF");
			exit(1);
		}
	}

	send_result = write(fd, buffer, datalen + 8 + 14 + 20);
	if (send_result == -1)
		perror("bpf_write");
#endif
	free(buffer);
	
	/* Return amount of _data_ bytes sent */
	if (send_result - 8 - 14 - 20 < 0) {
		return 0;
	}

	return send_result - 8 - 14 - 20;
}
