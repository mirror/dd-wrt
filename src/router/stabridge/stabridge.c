/*
 To use it setup your wireless client,
 add it to the bridge,
 add ebtables rules:
 1. ebtables -t broute -A BROUTING -p ARP -i eth0 --arp-op Reply --arp-mac-dst ! <bridge mac address> -j DROP
 2. ebtables -t broute -A BROUTING -p ARP -i eth0 --arp-op Request --arp-mac-dst ! <bridge mac addess> -j DROP
 (or only one ebtables -t broute -p ARP -i eth0 --arp-mac-dst ! <bridge mac addess> -j DROP)
 3. ebtables -t nat -A POSTROUTING -o ath0 -j snat --to-src <wireless mac address> --snat-target ACCEPT

 *
 * Copyright (c) 2005 by Wilibox (www.wilibox.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/ethernet.h> /* for ETH_FRAME_LEN */
#include <sys/poll.h>   /* for POLLIN etc */
#include <netpacket/packet.h>
#include <net/if_arp.h>
#include <net/if.h> /* for IFNAMSIZ && ifreq */
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "log.h"


//#define DEBUG_SNAT_ARP_INFO
//#define DEBUG_DNAT_ARP_INFO

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"


#define MAC_DB_LIMIT 256

typedef struct mac_db {
	unsigned char mac[ETH_ALEN];
	unsigned char ip[4];
} __attribute__((packed)) mac_db_t;

typedef struct iface {
        char ifname[IFNAMSIZ]; /* interface name */
        unsigned int ifindex;  /* interface index */
        unsigned char hwaddr[ETH_ALEN]; /* interface MAC address */
} iface_t;

typedef struct	ether_arp {
    struct ether_header ethhdr;            /* ethernet header */
    unsigned short ar_hrd;		/* Format of hardware address.  */
    unsigned short ar_pro;		/* Format of protocol address.  */
    unsigned char ar_hln;		/* Length of hardware address.  */
    unsigned char ar_pln;		/* Length of protocol address.  */
    unsigned short ar_op;		/* ARP opcode (command).  */
    unsigned char arp_sha[ETH_ALEN];	/* sender hardware address */
    unsigned char arp_spa[4];		/* sender protocol address */
    unsigned char arp_tha[ETH_ALEN];	/* target hardware address */
    unsigned char arp_tpa[4];		/* target protocol address */
//    unsigned char pad[18];           /* pad for min. ethernet payload (60 bytes) */
} __attribute__((packed)) ether_arp_t;


static const char* const MODULE = "[stabridge]";
static const char* const STABRIDGE_CONFIG_FILE = "/etc/stabridge_def.conf";
static const char* const BR_DEV = "br0";
static const char* const WLAN_DEV = "ath0";

static const char* const STABRIDGE_ACTIVE_INDEX = "stabridge.active.index";
static const char* const STABRIDGE_ID_STATUS = "stabridge.%d.status";
static const char* const STABRIDGE_ID_DEVNAME = "stabridge.%d.devname";
static const char* const STABRIDGE_ID_WLAN_DEVNAME = "stabridge.%d.wlan.devname";
static const char* const STABRIDGE_ID_DBSIZE = "stabridge.%d.dbsize";
static const char* const STABRIDGE_ID_LAN_ID_DEVNAME = "stabridge.%d.lan.%d.devname";


static mac_db_t* mac_db = NULL;
static int mac_db_max = 0;
static int mac_db_size = MAC_DB_LIMIT;

static volatile char running = 0;

static int get_iface(const char* ifname, iface_t* ifinfo)
{
	struct ifreq ifr;
	int fd;
	int ifindex;

	if (!ifname || !ifinfo)
		return -1;

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd < 0)
	{
		return -1;
	}
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	if (ioctl(fd, SIOCGIFINDEX, &ifr))
	{
		int saved_errno = errno;
		close(fd);
		errno = saved_errno;
		return -1;
	}

	// remember ifindex
	ifindex = ifr.ifr_ifindex;

	if (ioctl(fd, SIOCGIFHWADDR, &ifr))
	{
		int saved_errno = errno;
		close(fd);
		errno = saved_errno;
		return -1;
	}

	memcpy(ifinfo->ifname, ifr.ifr_name, IFNAMSIZ);
	memcpy(ifinfo->hwaddr, &ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);
	ifinfo->ifindex = ifindex;

	close(fd);
	return 0;
}


/*
 ------------------------------------------------------------
 */
static int init_snat_socks(int* s, iface_t* s_info, int s_size, int* max_sock)
{
	int i;

	for (i=0; i < s_size; ++i)
	{
		s[i] = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
		if (s[i] == -1 || get_iface(s_info[i].ifname, &s_info[i]) != 0)
		{
			while (--i >= 0)
			{
				close(s[i]);
			}
			return -1;
		}
		if (*max_sock < s[i])
		{
			*max_sock = s[i];
		}
	}
	return 0;
}



/*
 ------------------------------------------------------------
 */
static int bind_snat_socks(int* s, iface_t* s_info, int s_size)
{
	int i;
	struct sockaddr_ll addr;

	for (i=0; i < s_size; ++i)
	{
		memset(&addr, 0, sizeof(addr));
		// fill the socket address for binding
		addr.sll_family = AF_PACKET;
		addr.sll_protocol = htons(ETH_P_ARP);
		addr.sll_ifindex = s_info[i].ifindex;
		addr.sll_pkttype = PACKET_HOST;

		info("%s SNAT will bind to '%s' (index: %d, MAC: '"MACSTR"')\n",
		     MODULE, s_info[i].ifname, s_info[i].ifindex,
		     MAC2STR(s_info[i].hwaddr));

		// attempt binding
		if (bind(s[i], (struct sockaddr*) &addr, sizeof(addr)))
		{
			for (i=0; i < s_size; ++i)
			{
				close(s[i]);
			}
			return -1;
		}
	}
	return 0;
}

/*
 ------------------------------------------------------------
 */
static int prepare_socks(int* w, iface_t* w_info,
			 int* s, iface_t* s_info, int s_size,
			 int*b, iface_t* b_info)
{
	struct sockaddr_ll addr;
	int i;
	int max_sock = 0;

	if (init_snat_socks(s, s_info, s_size, &max_sock) != 0)
	{
		error_errno("%s SNAT Socket error.\n", MODULE);
		return -1;
	}

	*w = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (*w == -1 || get_iface(w_info->ifname, w_info) != 0)
	{
		error_errno("%s Wireless Socket error.\n", MODULE);
		for (i=0; i < s_size; ++i)
		{
			close(s[i]);
		}
		return -3;
	}

	*b = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (*b == -1 || get_iface(b_info->ifname, b_info) != 0)
	{
		error_errno("%s Bridge Socket error.\n", MODULE);
		for (i=0; i < s_size; ++i)
		{
			close(s[i]);
		}
		close(*w);
		return -4;
	}
	if (max_sock < *b)
	{
		max_sock = *b;
	}

	if (bind_snat_socks(s, s_info, s_size) != 0)
	{
		error_errno("%s Bind SNAT socket error.\n", MODULE);
		close(*w);
		close(*b);
		return -5;
	}

	memset(&addr, 0, sizeof(addr));
	// fill the socket address for binding
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_ARP);
	addr.sll_ifindex = b_info->ifindex;
	addr.sll_pkttype = PACKET_HOST;

	info("%s DNAT will bind to '%s' (index: %d, MAC: '"MACSTR"')\n",
	     MODULE, b_info->ifname, b_info->ifindex,
	     MAC2STR(b_info->hwaddr));
	// attempt binding
	if (bind(*b, (struct sockaddr*) &addr, sizeof(addr)))
	{
		error_errno("%s Bind DNAT socket error.\n", MODULE);
		for (i=0; i < s_size; ++i)
		{
			close(s[i]);
		}
		close(*b);
		close(*w);
		return -5;
	}

	memset(&addr, 0, sizeof(addr));
	// fill the socket address for binding
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_ARP);
	addr.sll_ifindex = w_info->ifindex;
	addr.sll_pkttype = PACKET_HOST;

	info("%s Wireless Send will bind to '%s' (index: %d, MAC: '"MACSTR"')\n",
	     MODULE, w_info->ifname, w_info->ifindex,
	     MAC2STR(w_info->hwaddr));
	// attempt binding
	if (bind(*w, (struct sockaddr*) &addr, sizeof(addr)))
	{
		error_errno("%s Bind Wireless socket error.\n", MODULE);
		for (i=0; i < s_size; ++i)
		{
			close(s[i]);
		}
		close(*w);
		close(*b);
		return -5;
	}
	return max_sock;
}

/*
 ------------------------------------------------------------
 */
static int snat_handler(int s_idx,
			int w, iface_t w_info,
			int* s, iface_t* s_info, int s_size)
{
	unsigned char buffer[ETH_FRAME_LEN]; /*Buffer for ethernet frame*/
	int length = 0;	/*length of the received frame*/
	int i;

	ether_arp_t* arph;

	length = recv(s[s_idx], buffer, ETH_FRAME_LEN, 0);
	if (length == -1 ||
	    length < sizeof(ether_arp_t)
	   )
	{
		debug("%s Droping invalid length packet\n", MODULE);
		return -1;
	}

	arph = (ether_arp_t*)buffer;
	switch (htons(arph->ar_op))
	{
	case ARPOP_REPLY:
		debug("%s SNAT ARP REPLY\n", MODULE);
		goto dump;
	case ARPOP_REQUEST:
		debug("%s SNAT ARP REQUEST\n", MODULE);
		dump:
#ifdef DEBUG_SNAT_ARP_INFO
		    {
			struct in_addr ip;
                        memcpy(&ip, arph->arp_tpa, sizeof(struct in_addr));
			printf("\t   dst: "MACSTR"    src: "MACSTR" proto: %04x\n"
			       "\ttarget: "MACSTR" sender: "MACSTR"\n"
			       "\tdst ip: %17s ",
			       MAC2STR(arph->ethhdr.ether_dhost),
			       MAC2STR(arph->ethhdr.ether_shost),
			       ntohs(arph->ethhdr.ether_type),
			       MAC2STR(arph->arp_tha),
			       MAC2STR(arph->arp_sha),
			       inet_ntoa(ip)
			      );
                        memcpy(&ip, arph->arp_spa, sizeof(struct in_addr));
			printf("src ip: %17s\n", inet_ntoa(ip));
		    }
#endif //DEBUG_ARP_INFO

		//ADD source MAC to DB
		if (mac_db_max == 0)
		{
			memcpy(mac_db[0].mac, arph->ethhdr.ether_shost, ETH_ALEN);
			memcpy(mac_db[0].ip, arph->arp_spa, sizeof(unsigned int));
			++mac_db_max;
		}
		else
		{
			for (i = 0; i < mac_db_max && i < mac_db_size; ++i)
			{
				if (memcmp(mac_db[i].mac, arph->ethhdr.ether_shost, ETH_ALEN) == 0)
					break;
			}
			if (mac_db_max < MAC_DB_LIMIT && i == mac_db_max)
			{
				memcpy(mac_db[i].mac, arph->ethhdr.ether_shost, ETH_ALEN);
				memcpy(mac_db[i].ip, arph->arp_spa, 4);
				++mac_db_max;
			}
			if (mac_db_max >= mac_db_size)
			{
				warn("%s MAC DB Exeeded. Removing old.\n", MODULE);
				memmove(mac_db, &mac_db[(mac_db_size + 1) / 2],
					mac_db_size - ((mac_db_size + 1) / 2));
				mac_db_max = (mac_db_size + 1) / 2;
			}
		}
		info("%s ARP SNAT'ed: "MACSTR" to "MACSTR"\n",
		     MODULE, MAC2STR(arph->arp_sha),
		     MAC2STR(w_info.hwaddr));
		memcpy(arph->ethhdr.ether_shost, w_info.hwaddr, ETH_ALEN);
		memcpy(arph->arp_sha, w_info.hwaddr, ETH_ALEN);
		send(w, buffer, length, 0);
#ifdef DEBUG_DNAT_ARP_INFO
		log_buffer_dump(DEBUG_LEVEL, buffer, length,
				"SNAT ARP PACKET");
#endif //DEBUG_DNAT_ARP_INFO
		break;
	default:
		debug("%s SNAT Droping not intresting opt code [0x%x] in packet\n", MODULE, arph->ar_op);
		return -2;
	}
	return 0;
}

/*
 ------------------------------------------------------------
 */
static int dnat_handler(int b)
{
	unsigned char buffer[ETH_FRAME_LEN]; /*Buffer for ethernet frame*/
	int length = 0;	/*length of the received frame*/
	int i;
	ether_arp_t* arph;

	length = recv(b, buffer, ETH_FRAME_LEN, 0);
	if (length == -1 ||
	    length < sizeof(ether_arp_t)
	   )
	{
		debug("%s Droping invalid length packet\n", MODULE);
		return -1;
	}

	arph = (ether_arp_t*)buffer;
	switch (htons(arph->ar_op))
	{
	case ARPOP_REPLY:
		debug("%s DNAT ARP REPLY\n", MODULE);
#ifdef DEBUG_DNAT_ARP_INFO
		    {
			struct in_addr ip;
                        memcpy(&ip, arph->arp_tpa, sizeof(struct in_addr));
			printf("\t   dst: "MACSTR"    src: "MACSTR" proto: %04x\n"
			       "\ttarget: "MACSTR" sender: "MACSTR"\n"
			       "\tdst ip: %17s ",
			       MAC2STR(arph->ethhdr.ether_dhost),
			       MAC2STR(arph->ethhdr.ether_shost),
			       ntohs(arph->ethhdr.ether_type),
			       MAC2STR(arph->arp_tha),
			       MAC2STR(arph->arp_sha),
			       inet_ntoa(ip)
			      );
                        memcpy(&ip, arph->arp_spa, sizeof(struct in_addr));
			printf("src ip: %17s\n", inet_ntoa(ip));
		    }
#endif //DEBUG_DNAT_ARP_INFO
		for (i = 0; i < mac_db_max && i < MAC_DB_LIMIT; ++i)
		{
			if (memcmp(mac_db[i].ip, arph->arp_tpa, 4) == 0)
			{
				memcpy(arph->ethhdr.ether_dhost, mac_db[i].mac, ETH_ALEN);
				info("%s ARP DNAT'ed: "MACSTR" to "MACSTR"\n", MODULE,
				     MAC2STR(arph->arp_tha), MAC2STR(mac_db[i].mac));
				memcpy(arph->arp_tha, mac_db[i].mac, ETH_ALEN);
				send(b, buffer, length, 0);
			}
		}
#ifdef DEBUG_DNAT_ARP_INFO
		log_buffer_dump(DEBUG_LEVEL, buffer, length,
				"DNAT ARP PACKET");
#endif //DEBUG_DNAT_ARP_INFO
		break;
	default:
		debug("%s DNAT Droping not intresting opt code [0x%x] in packet\n", MODULE, arph->ar_op);
		return -2;
	}
	return 0;
}

typedef struct opts
{
	char daemon;
	char index;
	int mac_db_size;
	const char* wlan;
	const char* br;
	int eths_count;
	const char** eths;
} opts_t;

/*
 ------------------------------------------------------------
 */
static void usage(const char* exe_name)
{
	fprintf(stderr, "Usage: %s [options...]\n"
		"Options:\n"
		"\t-h\t\t- Help. This message.\n"
		"\t-d\t\t- Become a daemon.\n"
		"\t-c <config>\t- Load configuration from config file. Default is '%s'\n"
		"\t-i <index>\t- Load desire configuration from config file. Default is 1\n"
		"\t-s <size>\t- Use MAC DB size. Default is %d if no in configuration found\n"
		"\t-w <devname>\t- Use wireless device name. Default is ath0 if no in configuration found\n"
		"\t-b <devname>\t- Use bridge device name. Default is br0 if no in configuration found\n"
		"\t-e <devname(s)>\t- Use ethernet device(s) name(s) separated by space. Default is eth0 if no in configuration found\n",
		exe_name, STABRIDGE_CONFIG_FILE, MAC_DB_LIMIT);
}

/*
 ------------------------------------------------------------
 */
static int store_lan_ifc(opts_t* opts, const char* arg)
{
	const char** tmp = NULL;

	if (opts == NULL || arg == NULL)
	{
		return -1;
	}
	++opts->eths_count;
	if (opts->eths == NULL)
	{
		tmp = malloc(opts->eths_count * sizeof(char*));
	}
	else
	{
		tmp = realloc(opts->eths, opts->eths_count * sizeof(char*));
	}
	if (tmp == NULL)
	{
		--opts->eths_count;
		return -2;
	}
	opts->eths = tmp;
	opts->eths[opts->eths_count - 1] = arg;
	return 0;
}

/*
 ------------------------------------------------------------
 */
static int parse_opts(int argc, char** argv, opts_t* opts)
{
	int c;

	if (opts == NULL)
	{
		usage(argv[0]);
		return -1;
	}

	while (1)
	{
		c = getopt(argc, argv, "hdi:s:w:b:e:");

		if (c == -1)
		{
			/* End of option list */
			break;
		}
		switch (c)
		{

		case 'd':
			/* become a daemon */
			opts->daemon = 1;
			break;

		case 'i':
			/* active configuration index from cfg file*/
			opts->index = atoi(optarg);
			break;

		case 's':
			/* DB size*/
			opts->mac_db_size = atoi(optarg);
			break;

		case 'w':
			/* wireless client device name */
			opts->wlan = optarg;
			break;

		case 'b':
			/* bridge device name */
			opts->br = optarg;
			break;

		case 'e':
			/* LAN(ethernet) device name */
			store_lan_ifc(opts, optarg);
			c = optind;
			while (c < argc && argv[c][0] != '-')
			{
			    store_lan_ifc(opts, argv[c]);
                            ++c;
			}
			break;

		default:
			/* help */
			usage(argv[0]);
			return -1;
		}


	}
	return 0;
}

/*
 ------------------------------------------------------------
 */
void exit_signal_handler(int signo)
{
    debug("Exiting on signal [%d]\n", signo);
    running = 0;
}

/*
 ------------------------------------------------------------
 */
static int exec_cmd(const char* cmd)
{
    FILE* pfd;

    pfd = popen(cmd, "r");
    if (!pfd)
    {
        error_errno("Can not exec '%s'\n", cmd);
        return -1;
    }
    pclose(pfd);
    return 0;
}

/*
 ------------------------------------------------------------
 */
static int get_ip(const char* ifname, char* result, int size)
{
    struct in_addr *addr;
    struct ifreq req;
    struct sockaddr_in *sin = (struct sockaddr_in *)&req.ifr_addr;
    int tempsock;

    if ((tempsock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }

    strcpy (req.ifr_name, ifname);
    if (ioctl(tempsock, SIOCGIFADDR, &req) < 0)
    {
        close(tempsock);
	return(-1);
    }
    close(tempsock);
    addr = &sin->sin_addr;
    strncpy(result,inet_ntoa(*addr), size);
    return 0;

}

/*
 ------------------------------------------------------------
 */
#define EBTABLES_ADD_CMD "ebtables -t broute -A BROUTING -i %s -p ARP --arp-ip-dst %s --arp-mac-dst "MACSTR" -j ACCEPT"
#define EBTABLES_DEL_CMD "ebtables -t broute -D BROUTING 1"
static int init_ebtables(iface_t* b_info, iface_t* s_info, int s_size)
{
    char cmd[255];
    int i;
    char ip[16];

    if (!b_info)
	return -1;

    if (get_ip(b_info->ifname, ip, sizeof(ip)) < 0)
	return -2;

    if (s_info && s_size > 0)
    {

	for (i=0; i < s_size; ++i)
	{
	    if (!s_info[i].ifname)
                continue;
	    snprintf(cmd, sizeof(cmd), EBTABLES_ADD_CMD,
                     s_info[i].ifname, ip, MAC2STR(b_info->hwaddr)
		    );
            exec_cmd(cmd);
	}
    }
    return 0;
}

/*
 ------------------------------------------------------------
 */
static int destroy_ebtables(int s_size)
{
    int i;


    if (s_size > 0)
    {

	for (i=0; i < s_size; ++i)
	{
            exec_cmd(EBTABLES_DEL_CMD);
	}
    }
    return 0;
}

/*
 ------------------------------------------------------------
 */
void segv_signal_handler(int signo)
{
	error("Invalid memory reference. Exiting\n");
	exit(-1);
}

/*
 =====================================================================
 */
int main(int c, char** v)
{
	int max_sock = 0;
	fd_set rfds;
	int res;
	int i;

	opts_t opts = { 0, 1, MAC_DB_LIMIT, WLAN_DEV, BR_DEV, 0, NULL};

	//Bridge device socket
	iface_t b_info;
	int b; /*socketdescriptor*/

	//Wireless interface info (DNAT)
	int w = -1; /*socketdescriptor*/
	iface_t w_info;

	//SNAT devices info
	int* s = NULL; /*socketdescriptor*/

	iface_t* s_info = NULL;
	int s_size = 1;

	if (parse_opts(c, v, &opts) != 0)
	{
		if (opts.eths != NULL)
		{
			free(opts.eths);
		}
		return -1;
	}

	if (opts.eths_count != 0)
	{
		s_size = opts.eths_count;
	}
	else
	{
		s_size = 1;
	}
	//Allocate interface infos
	if ((s_info = calloc(sizeof(iface_t) * s_size, 1)) == NULL)
	{
		error_errno("%s No RAM for %d SNAT(s)\n", MODULE, s_size);
		if (opts.eths != NULL)
		{
			free(opts.eths);
		}
		return -10;
	}
	if (opts.eths_count == 0)
	{
		memcpy(s_info[0].ifname, "eth0", strlen("eth0"));
	}
	else
	{
		for (i = 0; i < s_size; ++i)
		{
			memcpy(s_info[i].ifname, opts.eths[i], strlen(opts.eths[i]));
		}
		//opts.eths is not needed any more so free it
		free(opts.eths);
	}

	if ((s = calloc(sizeof(int) * s_size, 1)) == NULL)
	{
		error_errno("%s No RAM for %d SNAT socket(s)\n", MODULE, s_size);
		free(s_info);
		return -11;
	}

	strcpy(w_info.ifname, opts.wlan);
	strcpy(b_info.ifname, opts.br);

	//Allocate MAC DB
	mac_db_size = opts.mac_db_size;
	if ((mac_db = calloc(sizeof(mac_db_t) * mac_db_size, 1)) == NULL)
	{
		error_errno("%s No RAM for %d size MAC DB\n", MODULE, mac_db_size);
		free(s_info);
		free(s);
		return -12;
	}


	if ((max_sock = prepare_socks(&w, &w_info, s, s_info, s_size, &b, &b_info)) < 0)
	{
		free(s_info);
		free(s);
		free(mac_db);
		return -20;
	}

	signal(SIGHUP, exit_signal_handler);
	signal(SIGQUIT, exit_signal_handler);
	signal(SIGTERM, exit_signal_handler);
	signal(SIGINT, exit_signal_handler);
	signal(SIGSEGV, segv_signal_handler);

        init_ebtables(&b_info, s_info, s_size);

	if (opts.daemon != 0)
	{
		daemon(0, 0);
	}
        running = 1;
	while (running)
	{
		FD_ZERO(&rfds);
		for (i = 0; i < s_size; i++)
			FD_SET(s[i], &rfds);
		FD_SET(b, &rfds);
		res = select(max_sock + 1, &rfds, NULL, NULL, NULL);
		if (res < 0)
		{
		        continue;
		}
		for (i = 0; i < s_size; i++)
		{
			if (FD_ISSET(s[i], &rfds))
			{
				//SNAT handle
				snat_handler(i, w, w_info, s, s_info, s_size);
			}
		}
		if (FD_ISSET(b, &rfds))
		{
			//DNAT handle
			dnat_handler(b);
		}
	}
	destroy_ebtables(s_size);
	for (i=0; i < s_size; ++i)
	{
	        close(s[i]);
	}
	free(s);
	close(b);
	close(w);
	free(s_info);
	free(mac_db);
        return 0;
}
