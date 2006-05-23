/* Connection transfer rate estimator for netfilter.
 *
 * Copyright (c) 2004 Nuutti Kotivuori <naked@iki.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/skbuff.h>
//#include <linux/jiffies.h>
#include <linux/netfilter_ipv4/ip_conntrack_rate.h>
#include <linux/netfilter_ipv4/lockhelp.h>

/*
   I wanted to build a simpler and more robust rate estimator than the
   one used in sched/estimator.c. After evaluating a few choices I
   settled with the one given in an example in [RFC2859], which is the
   rate estimator described in [TON98].

   I will copy the example table from [RFC2859] here:

========================================================================
|Initially:                                                            |
|                                                                      |
|      AVG_INTERVAL = a constant;                                      |
|      avg-rate     = CTR;                                             |
|      t-front      = 0;                                               |
|                                                                      |
|Upon each packet's arrival, the rate estimator updates its variables: |
|                                                                      |
|      Bytes_in_win = avg-rate * AVG_INTERVAL;                         |
|      New_bytes    = Bytes_in_win + pkt_size;                         |
|      avg-rate     = New_bytes/( now - t-front + AVG_INTERVAL);       |
|      t-front      = now;                                             |
|                                                                      |
|Where:                                                                |
|      now          = The time of the current packet arrival           |
|      pkt_size     = The packet size in bytes of the arriving packet  |
|      avg-rate     = Measured Arrival Rate of traffic stream          |
|      AVG_INTERVAL = Time window over which history is kept           |
|                                                                      |
|                                                                      |
|              Figure 2. Example Rate Estimator Algorithm              |
|                                                                      |
========================================================================

   Additionally we have to be concerned about overflows, remainders
   and resolution in the algorithm. These are documented in the code
   below.

   References:

   [RFC2859] W. Fang, N. Seddigh and B. Nandy, "A Time Sliding Window
             Three Colour Marker (TSWTCM)", RFC 2859, June 2000.

   [TON98]   D.D. Clark, W. Fang, "Explicit Allocation of Best Effort
             Packet Delivery Service", IEEE/ACM Transactions on
             Networking, August 1998, Vol 6. No. 4, pp. 362-373.
*/

/* There are three important limits which need to be explored: maximum
   expressable rate, minimum expressable rate, minimum packet size to
   be countable.

   Maximum expressable rate depends on the size of the window and the
   scale we have chosen. It is approximately 2^32 / window /
   scale. For example with a window of 3 seconds and a scale of 100,
   the maximum rate is 14 megabytes per second, eg. 115Mbit/s.

   Minimum expressable rate depends on scale and the HZ on the
   architecture. It is HZ / scale. For example on most platforms where
   HZ is now 1000, this is 10 bytes per second, eg. 0.08kbit/s.

   Minimum packet size to be countable depends on the window size,
   scale and HZ. This is basically the smallest packet that when
   arriving immediately after the previous packet can cause the
   average rate to rise from zero to one. It is (HZ * window) /
   scale. For example with a window of 3 seconds, a scale of 100 and a
   HZ of 1000, this would be 30. That is, a continuous stream of
   packets less than 30 bytes long would not be able to rise the rate
   above zero.

   These limitations are a simple consequence of the current
   implementation using integer arithmetics. */

/* Maximum number of tokens in total that we can have in a window is
   limited by the range of the u_int32_t datatype. We prevent the
   overflow of this by first calculating the maximum amount of tokens
   a single packet can add and substracting that from the maximum
   value the window can get. */
#define MAX_PACKET_IN_TOKENS (0x0000ffff * IP_CONNTRACK_RATE_SCALE)
#define MAX_TOKENS_IN_WINDOW (0xffffffff - MAX_PACKET_IN_TOKENS)

/* Synchronizes all accesses to ip_conntrack_rate structures. */
static DECLARE_RWLOCK(rate_lock);

void
ip_conntrack_rate_count(struct ip_conntrack_rate *ctr, unsigned int len)
{
	u_int32_t new_bytes;
	unsigned long now = jiffies;

	WRITE_LOCK(&rate_lock);
	new_bytes = (ctr->avgrate * IP_CONNTRACK_RATE_INTERVAL +
	             len * IP_CONNTRACK_RATE_SCALE);
	if(new_bytes > MAX_TOKENS_IN_WINDOW)
		new_bytes = MAX_TOKENS_IN_WINDOW;
	if(now >= ctr->prev) /* Ignore packets at possible jiffie wraps */
		ctr->avgrate = new_bytes / (now - ctr->prev +
	                                    IP_CONNTRACK_RATE_INTERVAL);
	ctr->prev = now;
	WRITE_UNLOCK(&rate_lock);
}

u_int32_t
ip_conntrack_rate_get(struct ip_conntrack_rate *ctr)
{
	u_int32_t rate;

	READ_LOCK(&rate_lock);
	/* Rate can not overflow here if IP_CONNTRACK_RATE_INTERVAL is
	   atleast HZ. If it is not, we could change the order of
	   calculations at the possible cost of precision. */
	rate = ctr->avgrate * HZ / IP_CONNTRACK_RATE_SCALE;
	READ_UNLOCK(&rate_lock);
	return rate;
}

EXPORT_SYMBOL(ip_conntrack_rate_count);
EXPORT_SYMBOL(ip_conntrack_rate_get);
