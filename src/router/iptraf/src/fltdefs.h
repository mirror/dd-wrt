#ifndef IPTRAF_NG_FLTDEFS_H
#define IPTRAF_NG_FLTDEFS_H

/***

fltdefs.h - declarations for the TCP, UDP, and misc IP filters

***/


#define FLT_FILENAME_MAX 	40

#define FLT_RESOLVE		1
#define FLT_DONTRESOLVE		0

#define F_ALL_IP    0
#define F_TCP       6
#define F_UDP       17
#define F_OTHERIP   59
#define F_ICMP		1
#define F_IGMP		2
#define F_OSPF		89
#define F_IGP		9
#define F_IGRP		88
#define F_GRE		47
#define F_L2TP      115
#define F_IPSEC_AH  51
#define F_IPSEC_ESP 50

#define MATCH_OPPOSITE_ALWAYS       1
#define MATCH_OPPOSITE_USECONFIG    2

/*
 * IP filter parameter entry
 */
struct hostparams {
	char s_fqdn[45];
	char d_fqdn[45];
	char s_mask[20];
	char d_mask[20];
	in_port_t sport1;
	in_port_t sport2;
	in_port_t dport1;
	in_port_t dport2;
	int filters[256];
	char protolist[70];
	char reverse;
	char match_opposite;
};


struct filterent {
	struct hostparams hp;

	unsigned long saddr;
	unsigned long daddr;
	unsigned long smask;
	unsigned long dmask;
	unsigned int index;
	struct filterent *next_entry;
	struct filterent *prev_entry;
};

struct filterlist {
	struct filterent *head;
	struct filterent *tail;
};

#endif	/* IPTRAF_NG_FLTDEFS_H */
