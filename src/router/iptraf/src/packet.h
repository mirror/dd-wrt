/***

packet.h - external declarations for packet.c
           
Written by Gerard Paul Java

***/

/*
 * Number of bytes from captured packet to move into an aligned buffer.
 * 96 bytes should be enough for the IP header, TCP/UDP/ICMP/whatever header
 * with reasonable numbers of options.
 */

#define SNAPSHOT_LEN 96
#define MAX_PACKET_SIZE 17664
#define ALIGNED_BUF_LEN 120

#define min(a, b) ((a > b) ? b : a)

#define INVALID_PACKET 0
#define OK_PACKET 1
#define PACKET_OK 1
#define CHECKSUM_ERROR 2
#define PACKET_FILTERED 3
#define MORE_FRAGMENTS 4

#ifndef ARPHRD_IEEE802_TR
#define ARPHRD_IEEE802_TR 800
#endif

#ifndef VLAN_ETH_HLEN
#define VLAN_ETH_HLEN 18
#endif

extern int isdnfd;

void open_socket(int *fd);
unsigned short getlinktype(unsigned short family, char *ifname,
                           int isdn_fd, struct isdntab *isdntable);
void adjustpacket(char *tpacket, unsigned short family,
                  char **packet, char *aligned_buf, unsigned int *readlen);
void getpacket(int fd, char *buf, struct sockaddr_ll *fromaddr,
               int *ch, int *br, char *ifname, WINDOW * win);
int processpacket(char *tpacket, char **packet, unsigned int *br,
                  unsigned int *total_br, unsigned int *sport,
                  unsigned int *dport, struct sockaddr_ll *fromaddr,
                  unsigned short *linktype, struct filterstate *ofilter,
                  int match_opposite, char *ifname, char *ifptr);
void pkt_cleanup(void);
