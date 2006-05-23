#ifndef _IP_CONNTRACK_RATE_H
#define _IP_CONNTRACK_RATE_H

/* estimation interval, in jiffies */
#define IP_CONNTRACK_RATE_INTERVAL (3 * HZ)

/* scale on how many tokens per byte to generate */
#define IP_CONNTRACK_RATE_SCALE 100

/* per conntrack: transfer rate in connection */
struct ip_conntrack_rate {
	/* jiffies of previous received packet */
	unsigned long prev;
	/* average rate of tokens per jiffy */
	u_int32_t avgrate;
};

#ifdef __KERNEL__

/* Count a packet of len into given rate structure. */
extern void
ip_conntrack_rate_count(struct ip_conntrack_rate *ctr, unsigned int len);

/* Return current rate as bytes per second. Note that the returned
   rate is the rate at last received packet, not counting time has
   that passed after it. */
extern u_int32_t
ip_conntrack_rate_get(struct ip_conntrack_rate *ctr);

#endif /* __KERNEL__ */

#endif /* _IP_CONNTRACK_RATE_H */
