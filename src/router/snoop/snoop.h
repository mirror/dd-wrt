/* snoop.c - Berkley snoop protocol module
 *
 * (C) 2005-2006 Ivan Keberlein <ikeberlein@users.sourceforge.net>
 *               KNET Ltd. http://www.isp.kz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public Licence version 2 as
 * published by Free Software Foundation.
 *
 * See COPYING for details
 *
 */
#ifndef __SNOOP_H__
#define __SNOOP_H__

#include <linux/types.h>
#include <linux/list.h>
#include <linux/timer.h>

#define VERSION         	"0.3-rc7"

#define SNOOP_RXMIT_MAX		8		/* max rxmit count */
#define SNOOP_CONN_TIMEO	60		/* seconds before connection is stale */
#define SNOOP_DEFAULT_RTT	50		/* initial RTT - msecs */
#define SNOOP_MIN_RTT		10		/* minimum RTT - msecs */
#define SNOOP_MAX_RTT		600		/* maximum RTT - msecs */
#define SNOOP_CACHE_MAX		40		/* max packets to cache per connection */

struct sn_conntrack_s;
typedef struct sn_packet_s {
	struct list_head	list;
	struct sn_conntrack_s*	ct;		/* link back to conntrack */
	u32			seq;		/* sequence number of first byte */
	u16			size;		/* amount of actual TCP data */
	unsigned long		send_time;	/* when this packet was sent to WH */
	int			sender_rxmit;	/* set if pkt rxmitted by sender */
	int			rxmit_count;	/* number of local rexmits */
	int			dack_count;	/* number of duplicate ACKS */
	struct sk_buff*		skb;		/* real packet */
} sn_packet_t;

/* incoming packet origin */
enum sn_pkt_origin {
	SNOOP_UNKNOWN = 0x00,		/* nether from WH nor from FH */
	SNOOP_FROM_WH = 0x01,		/* from wireless host */
	SNOOP_FROM_FH = 0x02		/* from fixed host */
};

typedef struct {
	u32		saddr;
	u32		daddr;
	u16		sport;
	u16		dport;
	u32		seq;
	u32		ack_seq;
	u16		window;
	u16		size;	
	u16		ack:1,
			rst:1,
			syn:1,
			fin:1;
	u8		_opt[40];	// max tcp hdr size (60) - min tcp hdr size (20) = 40
	u8		*opt;
	int		optlen;
	struct sk_buff*	skb;
} pkt_info_t;

enum sn_connstate {
	SNOOP_SYNSENT     = 0x01,	/* SYN seen from WH, waiting for SYN,ACK from FH */
	SNOOP_WAITACK	  = 0x02,	/* SYN,ACK from FH wait for ACK from WH */
	SNOOP_ESTABLISHED = 0x03	/* active connection */
};

#define	FL_SNOOP_DESTROYING	0x00	/* the entry is being destroyed */

typedef struct sn_conntrack_s {
	struct hlist_node	link;
	u32			hash;		/* hash index */
	spinlock_t		lock;		/* lock */
	atomic_t		use_count;	/* use count */
	volatile
	unsigned long		flags;		/* misc flags */
	enum sn_connstate	state;		/* connection state */
	u32			waddr;		/* WH IP */
	u32			faddr;		/* FH IP */
	u16			wport;		/* WH TCP port */
	u16			fport;		/* FH TCP port */
	u32			last_seq;	/* last byte of last packet buffered */
	u32			last_ack;	/* last byte recvd by WH */
	u32			last_window;	/* last window size */
	u32			isn;		/* initial sequence number (ISN) */
	u32			risn;		/* receiver ISN */
	int			dack_count;	/* the count DUP ACKS */
	unsigned long		rtt;		/* RTT value for WH */
	struct list_head	pkt_list;	/* cache of UNA segments */
	int			pkt_count;	/* qty of cached segments */
	struct timer_list	rto_timer;	/* retransmission timeout timer */
	struct timer_list	tmo_timer;	/* connection idle timeout timer */
} sn_conntrack_t;

typedef struct {
	struct hlist_head	list;
	rwlock_t		lock;
} sn_hash_bucket_t;

typedef struct snoop_stats_s {
	unsigned long connections;		/* total connections passed */
	unsigned long input_pkts;		/* total packets from FH */
	unsigned long sender_rxmits;		/* total packets retransmitted from FH */
	unsigned long local_rxmits;		/* total packets retransmitted from cache */
	unsigned long cache_misses;		/* total cache misses */
	unsigned long acks;			/* total ACKs */
	unsigned long newacks;			/* total NEW ACKs */
	unsigned long dupacks;			/* total DUP ACKs */
	unsigned long dupacks_dropped;		/* total DUP ACKs dropped */
	unsigned long win_updates;		/* total window updates */
	unsigned long rto;			/* total retransmit timeouts */
} snoop_stats_t;

#endif // ! __SNOOP_H__

