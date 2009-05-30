
/* 
 * listen.c listen for any packet through an interface 
 */

// #define MY_DEBUG
#define MY_DEBUG1

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <rc.h>

// for PF_PACKET
#include <features.h>
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif

enum { L_FAIL, L_ERROR, L_UPGRADE, L_ESTABLISHED, L_SUCCESS };

# define LOG(str, args...) do { printf("DEBUG, "); \
                                printf(str, ## args); \
                                printf("\n"); } while(0)

# define DEBUG(args...) do {;} while(0)

# define DEBUG1(args...) do {;} while(0)

#define start_service(a) eval("startservice",a);
#define stop_service(a) eval("stopservice",a);

struct iphdr {
	u_int8_t version;
	u_int8_t tos;
	u_int16_t tot_len;
	u_int16_t id;
	u_int16_t frag_off;
	u_int8_t ttl;
	u_int8_t protocol;
	u_int16_t check;
	u_int8_t saddr[4];
	u_int8_t daddr[4];
};

struct EthPacket {
	u_int8_t dst_mac[6];
	u_int8_t src_mac[6];
	u_int8_t type[2];
	// struct iphdr ip; //size=20
	u_int8_t version;
	u_int8_t tos;
	u_int16_t tot_len;
	u_int16_t id;
	u_int16_t frag_off;
	u_int8_t ttl;
	u_int8_t protocol;
	u_int16_t check;
	u_int8_t saddr[4];
	u_int8_t daddr[4];

	u_int8_t data[1500 - 20];
};

int
read_interface(char *interface, int *ifindex, u_int32_t *addr,
	       unsigned char *arp)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *sin;

	memset(&ifr, 0, sizeof(struct ifreq));
	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		ifr.ifr_addr.sa_family = AF_INET;
		strcpy(ifr.ifr_name, interface);

		if (addr) {
			if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
				sin = (struct sockaddr_in *)&ifr.ifr_addr;
				*addr = sin->sin_addr.s_addr;
			} else {
				return -1;
			}
		}

		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
			*ifindex = ifr.ifr_ifindex;
		} else {
			return -1;
		}
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
			memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
		} else {
			return -1;
		}
	} else {
		return -1;
	}
	close(fd);
	return 0;
}

int raw_socket(int ifindex)
{
	int fd;
	struct sockaddr_ll sock;

	DEBUG("Opening raw socket on ifindex %d\n", ifindex);
	if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0) {
		DEBUG("socket call failed: \n");
		return -1;
	}

	sock.sll_family = AF_PACKET;
	sock.sll_protocol = htons(ETH_P_IP);
	sock.sll_ifindex = ifindex;
	if (bind(fd, (struct sockaddr *)&sock, sizeof(sock)) < 0) {
		DEBUG("bind call failed: \n");
		close(fd);
		return -1;
	}

	return fd;

}

u_int16_t checksum(void *addr, int count)
{
	/* 
	 * Compute Internet Checksum for "count" bytes beginning at location
	 * "addr". 
	 */
	register int32_t sum = 0;
	u_int16_t *source = (u_int16_t *)addr;

	while (count > 1) {
		/* 
		 * This is the inner loop 
		 */
		sum += *source++;
		count -= 2;
	}

	/* 
	 * Add left-over byte, if any 
	 */
	if (count > 0) {
		/* 
		 * Make sure that the left-over byte is added correctly both with
		 * little and big endian hosts 
		 */
		u_int16_t tmp = 0;

		*(unsigned char *)(&tmp) = *(unsigned char *)source;
		sum += tmp;
	}
	/* 
	 * Fold 32-bit sum to 16 bits 
	 */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

int listen_interface(char *interface)
{
	int ifindex = 0;
	fd_set rfds;
	struct EthPacket packet;
	struct timeval tv;
	int retval;
	unsigned char mac[6];
	static int fd;
	int ret = L_SUCCESS;

	int bytes;
	u_int16_t check;

	struct in_addr ipaddr, netmask;

	if (read_interface(interface, &ifindex, NULL, mac) < 0) {
		ret = L_ERROR;
		goto Exit;
	}

	for (;;) {
		if (check_action() != ACT_IDLE) {	// Don't execute during upgrading
			return L_UPGRADE;
		}
		if (check_wan_link(0)) {
			return L_ESTABLISHED;
		}
		tv.tv_sec = 100000;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		fd = raw_socket(ifindex);
		if (fd < 0) {
			printf("FATAL: couldn't listen on socket\n");
			return L_ERROR;
		}
		if (fd >= 0)
			FD_SET(fd, &rfds);
		if (tv.tv_sec > 0) {
			DEBUG("Waitting for select... \n");
			retval = select(fd + 1, &rfds, NULL, NULL, &tv);
		} else
			retval = 0;

		if (retval == 0) {
			printf("no packet recieved! \n\n");
		} else {

			memset(&packet, 0, sizeof(struct EthPacket));
			bytes = read(fd, &packet, sizeof(struct EthPacket));
			if (bytes < 0) {
				DEBUG
				    ("couldn't read on raw listening socket -- ignoring\n");
				usleep(500000);	/* possible down interface, looping
						 * condition */
				ret = L_FAIL;
				goto Exit;
			}

			if (bytes < (int)(sizeof(struct iphdr))) {
				DEBUG("message too short, ignoring\n");
				ret = L_FAIL;
				goto Exit;
			}

			if (strncmp(mac, packet.dst_mac, 6)) {
				DEBUG("dest mac not the router\n");
				ret = L_FAIL;
				goto Exit;
			}

			if (inet_addr(nvram_safe_get("lan_ipaddr")) ==
			    *(u_int32_t *)packet.daddr) {
				DEBUG("dest ip equal to lan ipaddr\n");
				ret = L_FAIL;
				goto Exit;
			}

			DEBUG("inet_addr=%x, packet.daddr=%x",
			      inet_addr(nvram_safe_get("lan_ipaddr")),
			      *(u_int32_t *)packet.daddr);

			// for (i=0; i<34;i++) {
			// if (i%16==0) printf("\n");
			// printf("%02x ",*( ( (u_int8_t *)packet)+i) );
			// }
			// printf ("\n");

			DEBUG1
			    ("%02X%02X%02X%02X%02X%02X,%02X%02X%02X%02X%02X%02X,%02X%02X\n",
			     packet.dst_mac[0], packet.dst_mac[1],
			     packet.dst_mac[2], packet.dst_mac[3],
			     packet.dst_mac[4], packet.dst_mac[5],
			     packet.src_mac[0], packet.src_mac[1],
			     packet.src_mac[2], packet.src_mac[3],
			     packet.src_mac[4], packet.src_mac[5],
			     packet.type[0], packet.type[1]);

			DEBUG("ip.version = %x", packet.version);
			DEBUG("ip.tos = %x", packet.tos);
			DEBUG("ip.tot_len = %x", packet.tot_len);
			DEBUG("ip.id = %x", packet.id);
			DEBUG("ip.ttl= %x", packet.ttl);
			DEBUG("ip.protocol= %x", packet.protocol);
			DEBUG("ip.check=%04x", packet.check);
			DEBUG1("ip.saddr=%08x", *(u_int32_t *)&(packet.saddr));
			DEBUG1("ip.daddr=%08x", *(u_int32_t *)&(packet.daddr));

			if (*(u_int16_t *)packet.type == 0x0800) {
				DEBUG("not ip protocol");
				ret = L_FAIL;
				goto Exit;
			}

			/* 
			 * ignore any extra garbage bytes 
			 */
			bytes = ntohs(packet.tot_len);

			/* 
			 * check IP checksum 
			 */
			check = packet.check;
			packet.check = 0;

			if (check !=
			    checksum(&(packet.version), sizeof(struct iphdr))) {
				DEBUG("bad IP header checksum, ignoring\n");
				DEBUG("check received = %X, should be %X",
				      check, checksum(&(packet.version),
						      sizeof(struct iphdr)));
				ret = L_FAIL;
				goto Exit;
			}

			DEBUG("oooooh!!! got some!\n");
#ifdef HAVE_PPTP
			if (nvram_match("wan_proto", "pptp")) {
				inet_aton(nvram_safe_get("pptp_server_ip"),
					  &ipaddr);
			}
#ifdef HAVE_L2TP
			else
#endif
#endif
#ifdef HAVE_L2TP
			if (nvram_match("wan_proto", "l2tp")) {
				inet_aton(nvram_safe_get("lan_ipaddr"),
					  &ipaddr);
			}
#endif
#if defined(HAVE_PPTP) || defined(HAVE_L2TP)
			else
#endif
			{
				inet_aton(nvram_safe_get("wan_ipaddr"),
					  &ipaddr);
			}

			inet_aton(nvram_safe_get("wan_netmask"), &netmask);
			DEBUG("gateway=%08x", ipaddr.s_addr);
			DEBUG("netmask=%08x", netmask.s_addr);

			if ((ipaddr.s_addr & netmask.s_addr) !=
			    (*(u_int32_t *)&(packet.daddr) & netmask.s_addr)) {
				if (nvram_match("wan_proto", "l2tp")) {
					ret = L_SUCCESS;
					goto Exit;
				} else
				    if (nvram_match("wan_proto", "heartbeat")) {
					ret = L_SUCCESS;
					goto Exit;
				} else {
					ret = L_FAIL;
					goto Exit;
				}
			}

			break;
		}
	}
Exit:
	if (fd)
		close(fd);

	return ret;
}

int main(int argc, char *argv[])
{

	char *interface = argv[1];
	pid_t pid;

	if (argc < 2) {
		cprintf("Usage: %s <interface>\n", argv[0]);
		return 0;
	}

	cprintf("Starting listen on %s\n", interface);

	pid = fork();
	switch (pid) {
	case -1:
		perror("fork failed");
		exit(1);
	case 0:
	      retry:
		for (;;) {
			int ret;

			ret = listen_interface(interface);
			switch (ret) {
			case L_SUCCESS:
				DEBUG1
				    ("**************** received an lan to wan packet **************\n\n");
				start_service("force_to_dial");
				if (nvram_match("wan_proto", "heartbeat"))
					exit(0);

				if (!check_wan_link(0)) {	// Connect fail, we want to re-connect
					// session
					sleep(3);
					goto retry;
				}
				exit(0);
				break;

			case L_UPGRADE:
				DEBUG1("listen: nothing to do...\n");
				exit(0);
				break;

			case L_ESTABLISHED:
				DEBUG1("The link had been established\n");
				exit(0);
				break;

			case L_ERROR:
				DEBUG1("Continue...\n");
				exit(0);
				break;

			case L_FAIL:
			default:
				break;

			}
		}
		break;
	default:
		_exit(0);
		break;
	}
}
