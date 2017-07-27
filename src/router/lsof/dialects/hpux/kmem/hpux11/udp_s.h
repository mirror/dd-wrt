/*
 * udp_s.h for HP-UX 10.30 and above
 *
 * This header file defines the UDP connection structure, udp_s, for lsof.
 * Lsof gets the parameters of a UDP connection from tcp_s.  Lsof locates a
 * tcp_s structure by scanning the queue structure chain of a UDP stream,
 * looking for a queue structure whose module name begins with UDP; that queue
 * structure's private data pointer, q_ptr, addresses its associated tcp_s
 * structure.
 *
 * V. Abell
 * February, 1998
 */

#if	!defined(LSOF_UDP_S_H)
#define	LSOF_UDP_S_H

#include "kernbits.h"

typedef struct udp_s {
	int udp_state;			/* connection state */
	KA_T udp_hash_next;
	KA_T udp_ptphn;
	uint16_t udp_checksum;
	uint16_t udp_port[2];		/* source and destination ports */
	uint32_t udp_src;		/* source IP address */
	uint32_t udp_dst;		/* destination IP address */

/*
 * These q4 elements are ignored.

	uint udp_hdr_length;
	int udp_wroff_xtra;
	uint udp_family;
	uint udp_ip_snd_options_len;
	KA_T udp_ip_snd_options;
	int udp_linger;
	union {
	    uchar udpu1_multicast_ttl;
	    u32 udpu1_pad;
	} udp_u1;
	NET32 udp_multicast_if_addr;
	KA_T udp_udph;
	uint udp_priv_stream;
	uint udp_calc_checksum;
	uint udp_debug;
	uint udp_dontroute;
	uint udp_broadcast;
	uint udp_useloopback;
	uint udp_reuseaddr;
	uint udp_reuseport;
	uint udp_multicast_loop;
	uint udp_rx_icmp;
	uint udp_rx_icmp_set;
	uint udp_distribute;
	uint udp_link_status;
	uint udp_copyavoid;
	uint udp_pad_to_bit_31;
	union {
	    uint udpu2_wants_opts;
	   struct udpu2_flags_s udpu2_flags;
	} udp_u2;
	union {
	    char udpu3_iphc[72];
	    iph_t udpu3_iph;
	    u32 udpu3_ipharr[6];
	    uble udpu3_aligner;
	} udp_u3;
	u8 udp_pad2[2];
	u8 udp_type_of_service;
	u8 udp_ttl;
	u8 udp_bound_ip[4];

 * Those q4 elements were ignored.
 */

} udp_s_t;

#endif	/* !defined(LSOF_UDP_S_H) */
