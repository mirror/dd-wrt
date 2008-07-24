/**************************************************************************
* Copyright 2006 StorLink Semiconductors, Inc.  All rights reserved.
*--------------------------------------------------------------------------
* Name			: sl351x_toe.h
* Description	:
*		Define for TOE driver of Storlink SL351x
*
* History
*
*	Date		Writer		Description
*----------------------------------------------------------------------------
*				Xiaochong	Create
*
****************************************************************************/
#ifndef __SL351x_TOE_H
#define __SL351x_TOE_H	1
#include <net/sock.h>
#include <asm/arch/sl351x_gmac.h>
#include <linux/timer.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
/*
 * TOE_CONN_T is data structure of tcp connection info, used at both
 * device layer and kernel tcp layer
 * skb is the jumbo frame
 */

struct toe_conn{
	__u8	qid;		// connection qid 0~63.
	__u8	ip_ver;		// 0: not used; 4: ipv4; 6: ipv6.
	/* hash key of the connection */
	__u16	source;
	__u16	dest;
	__u32	saddr[4];
	__u32	daddr[4];

	__u32	seq;
	__u32	ack_seq;

	/* these fields are used to set TOE QHDR */
	__u32	ack_threshold;
	__u32	seq_threshold;
	__u16	max_pktsize;

	/* used by sw toe, accumulated ack_seq of ack frames */
	__u16	ack_cnt;
	/* used by sw toe, accumulated data frames held at driver */
	__u16	cur_pktsize;

	__u8	status;
#define	TCP_CONN_UNDEFINE		0X00
#define	TCP_CONN_CREATION		0X01
#define	TCP_CONN_CONNECTING		0X02
#define	TCP_CONN_ESTABLISHED	0X04
#define	TCP_CONN_RESET			0X08	// this is used for out-of-order
                      			    	// or congestion window is small
#define	TCP_CONN_CLOSING		0X10
#define	TCP_CONN_CLOSED			0x11

	__u16	hash_entry_index;	/* associated hash entry */

	// one timer per connection. Otherwise all connections should be scanned
	// in a timeout interrupt, and timeout interrupt is triggered no matter
	// a connection is actually timeout or not.
	struct timer_list	rx_timer;
	unsigned long		last_rx_jiffies;
	GMAC_INFO_T			*gmac;
	struct net_device	*dev;

	//	for generating pure ack frame.
	struct ethhdr		l2_hdr;
	struct iphdr		l3_hdr;

	spinlock_t			conn_lock;
	DMA_RWPTR_T			toeq_rwptr;
	GMAC_RXDESC_T		*curr_desc;
	struct sk_buff		*curr_rx_skb;
};

struct jumbo_frame {
	struct sk_buff	*skb0;		// the head of jumbo frame
	struct sk_buff	*tail;		// the tail of jumbo frame
	struct iphdr	*iphdr0;	// the ip hdr of skb0.
	struct tcphdr	*tcphdr0;	// the tcp hdr of skb0.
};

#endif // __SL351x_TOE_H
