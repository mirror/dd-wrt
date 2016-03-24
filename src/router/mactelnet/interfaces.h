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
#ifndef _INTERFACES_H
#define _INTERFACES_H 1

#define MAX_INTERFACES 32

struct net_interface {
	char name[256];
	unsigned char ipv4_addr[IPV4_ALEN];
	unsigned char mac_addr[ETH_ALEN];

	/* used by mactelnetd */
	int socketfd;

#ifdef __linux__
	int ifindex;
#endif
	int has_mac;
	int in_use;
	struct net_interface *prev;
	struct net_interface *next;
};


extern int net_get_interfaces(struct net_interface **interfaces);
extern struct net_interface *net_get_interface_ptr(struct net_interface **interfaces, char *name, int create);
extern int net_init_raw_socket();
extern int net_send_udp(const int socket, struct net_interface *interface, const unsigned char *sourcemac, const unsigned char *destmac, const struct in_addr *sourceip, const int sourceport, const struct in_addr *destip, const int destport, const unsigned char *data, const int datalen);
extern unsigned short in_cksum(unsigned short *addr, int len);
#endif
