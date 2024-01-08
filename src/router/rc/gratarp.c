/*
 * qos.c
 *
 * Copyright (C) 2017 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
/* gartarp */

struct arph {
	uint16_t hw_type;

#define ARPHDR_ETHER 1

	uint16_t proto_type;

	char ha_len;
	char pa_len;

#define ARPOP_BROADCAST 1
#define ARPOP_REPLY 2
	uint16_t opcode;
	char source_add[ETH_ALEN];
	char source_ip[IP_ALEN];
	char dest_add[ETH_ALEN];
	char dest_ip[IP_ALEN];

} __attribute__((packed));

#define ARP_HLEN sizeof(struct arph) + ETH_HLEN
#define BCAST "\xff\xff\xff\xff\xff\xff"

static int get_iface_attr(int sk, char *iface, char *hw, char *paddr)
{
	int ret;
	struct ifreq ifr;

	strcpy(ifr.ifr_name, iface);

	ret = ioctl(sk, SIOCGIFHWADDR, &ifr);
	if (unlikely(ret == -1)) {
		perror("ioctl SIOCGIFHWADDR");
		return ret;
	}
	memcpy(hw, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	ret = ioctl(sk, SIOCGIFADDR, &ifr);
	if (unlikely(ret == -1)) {
		perror("ioctl SIOCGIFADDR");
		return ret;
	}
	memcpy(paddr, ifr.ifr_addr.sa_data + 2, IP_ALEN);

	return ret;
}

static void setup_eth(struct ether_header *eth, char *hw_addr)
{
	memcpy(eth->ether_shost, hw_addr, ETH_ALEN);
	memcpy(eth->ether_dhost, BCAST, ETH_ALEN);
	eth->ether_type = htons(ETH_P_ARP);
}

static void setup_garp(struct arph *arp, char *hw_addr, char *paddr)
{
	arp->hw_type = htons(ARPHDR_ETHER);
	arp->proto_type = htons(ETH_P_IP);
	arp->ha_len = ETH_ALEN;
	arp->pa_len = IP_ALEN;

	memcpy(arp->source_add, hw_addr, ETH_ALEN);
	memcpy(arp->source_ip, paddr, IP_ALEN);
}

static void setup_garp_broadcast(struct arph *arp, char *paddr)
{
	arp->opcode = htons(ARPOP_BROADCAST);

	bzero(arp->dest_add, ETH_ALEN);
	memcpy(arp->dest_ip, paddr, IP_ALEN);
}

static void setup_garp_reply(struct arph *arp, char *hw_addr, char *paddr)
{
	arp->opcode = htons(ARPOP_REPLY);

	memcpy(arp->dest_add, hw_addr, ETH_ALEN);
	memcpy(arp->dest_ip, paddr, IP_ALEN);
}

/*
 * send_garp 
 *
 * - sends 20 gartuitous arps 
 * in a 200 millisec interval.
 * One as braadcast and one as reply.
 * 
 * 
 * parameter iface: sending interface name
 *
 * returns false on failure
 *         true on success 
 */
static int send_garp(char *iface)
{
	char pkt[ARP_HLEN];
	char iface_hw[ETH_ALEN];
	char iface_paddr[IP_ALEN];
	struct sockaddr_ll link;
	struct ether_header *eth;
	struct arph *arp;
	int rc;
	int sk;
	int n_garps = 10;

	eth = (struct ether_header *)pkt;
	arp = (struct arph *)(pkt + ETH_HLEN);

	sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (unlikely(sk == -1)) {
		perror("socket");
		return sk;
	}

	rc = get_iface_attr(sk, iface, iface_hw, iface_paddr);
	if (unlikely(rc == -1))
		goto out;

	/* set link layer information for driver */
	bzero(&link, sizeof(link));
	link.sll_family = AF_PACKET;
	link.sll_ifindex = if_nametoindex(iface);

	setup_eth(eth, iface_hw);
	setup_garp(arp, iface_hw, iface_paddr);

	while (n_garps--) {
		setup_garp_broadcast(arp, iface_paddr);
		rc = sendto(sk, pkt, ARP_HLEN, 0, (struct sockaddr *)&link,
			    sizeof(struct sockaddr_ll));
		if (unlikely(rc == -1)) {
			perror("sendto");
			goto out;
		}

		setup_garp_reply(arp, iface_hw, iface_paddr);
		rc = sendto(sk, pkt, ARP_HLEN, 0, (struct sockaddr *)&link,
			    sizeof(struct sockaddr_ll));
		if (unlikely(rc == -1)) {
			perror("sendto");
			goto out;
		}
		usleep(200000);
	}

out:
	close(sk);
	return rc;
}

static int gratarp(char *iface)
{
	if (iface) {
		usleep(500000);
		send_garp(iface);
	}

	return 0;
}

static int gratarp_main(int argc, char **argv)
{
	signal(SIGCHLD, SIG_IGN);

	pid_t pid;

	if (argc < 2) {
		fprintf(stderr, "usage: gratarp <interface>\n");
		return 1;
	}

	pid = fork();
	switch (pid) {
	case -1:
		perror("fork failed");
		exit(1);
		break;
	case 0:
		gratarp(argv[1]);
		return 0;
		break;
	default:
		//waitpid(pid, &status, 0);
		// dprintf("parent\n");
		break;
	}

	return 0;
}
