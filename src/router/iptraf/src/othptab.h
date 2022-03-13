#ifndef IPTRAF_NG_OTHPTAB_H
#define IPTRAF_NG_OTHPTAB_H

/***

othptab.h - header file for the non-TCP routines

***/

#include "packet.h"

#define NONIP -1
#define IS_IP 1
#define NOT_IP 0

#define NOHTIND 0		/* Bottom or Top (head or tail) indicator printed */
#define TIND 1			/* Tail indicator printed */
#define HIND 2			/* Head indicator printed */

#define VSCRL_OFFSET	60	/* Characters to vertically scroll */

struct othptabent {
	struct sockaddr_storage saddr;
	struct sockaddr_storage daddr;
	char smacaddr[18];	/* FIXME: use dynamicly allocated space */
	char dmacaddr[18];
	unsigned short linkproto;
	char s_fqdn[100];
	char d_fqdn[100];
	int s_fstat;
	int d_fstat;
	unsigned int protocol;
	char iface[IFNAMSIZ];
	unsigned int pkt_length;

	union {
		struct {
			char s_sname[15];
			char d_sname[15];
		} udp;
		struct {
			unsigned int type;
			unsigned int code;
		} icmp;
		struct {
			unsigned char version;
			unsigned char type;
			unsigned long area;
			char routerid[16];
		} ospf;
		struct {
			unsigned short opcode;
			struct in_addr src_ip_address;
			struct in_addr dest_ip_address;
		} arp;
		struct {
			unsigned short opcode;
			char src_mac_address[6];
			char dest_mac_address[6];
		} rarp;
		struct {
			uint8_t type;
			uint8_t code;
		} icmp6;
	} un;
	unsigned int type;
	unsigned int code;
	unsigned int index;
	int is_ip;
	int fragment;
	struct othptabent *prev_entry;
	struct othptabent *next_entry;
};

struct othptable {
	struct othptabent *head;
	struct othptabent *tail;
	struct othptabent *firstvisible;
	struct othptabent *lastvisible;
	unsigned int count;
	unsigned int lastpos;
	unsigned int strindex;	/* starting index of the string to display */
	int htstat;
	unsigned int obmaxy;	/* number of lines in the border window */
	unsigned int oimaxy;	/* number of lines inside the border */
	WINDOW *othpwin;
	PANEL *othppanel;
	WINDOW *borderwin;
	PANEL *borderpanel;
};

/* Added by David Harbaugh for Non-IP protocol identification */

struct packetstruct {
	char *packet_name;	/* Name of packet type   */
	unsigned int protocol;	/* Number of packet type */
};


/* partially stolen from ospf.h from tcpdump */

#define	OSPF_TYPE_UMD	0
#define	OSPF_TYPE_HELLO	1
#define	OSPF_TYPE_DB	2
#define	OSPF_TYPE_LSR	3
#define	OSPF_TYPE_LSU	4
#define	OSPF_TYPE_LSA	5
#define	OSPF_TYPE_MAX	6

struct ospfhdr {
	u_char ospf_version;
	u_char ospf_type;
	u_short ospf_len;
	struct in_addr ospf_routerid;
	struct in_addr ospf_areaid;
	u_short ospf_chksum;
	u_short ospf_authtype;
};

void init_othp_table(struct othptable *table);

void check_icmp_dest_unreachable(struct tcptable *table, struct pkt_hdr *pkt,
				 char *ifname);

struct othptabent *add_othp_entry(struct othptable *table, struct pkt_hdr *pkt,
				  struct sockaddr_storage *saddr,
				  struct sockaddr_storage *daddr,
				  int is_ip,
				  int protocol,
				  char *packet2,
				  char *ifname, struct resolver *res,
				  int logging, FILE *logfile);

void printothpentry(struct othptable *table, struct othptabent *entry,
		    unsigned int screen_idx, int logging, FILE * logfile);

void refresh_othwindow(struct othptable *table);

void destroyothptable(struct othptable *table);

#endif	/* IPTRAF_NG_OTHPTAB_H */
