#ifndef __NET_SCHED_CODEL_H
#define __NET_SCHED_CODEL_H

/*
 * Codel - The Controlled-Delay Active Queue Management algorithm
 *
 *  Copyright (C) 2011-2012 Kathleen Nichols <nichols-/Bx750g9OtRBDgjK7y7TUQ@public.gmane.org>
 *  Copyright (C) 2011-2012 Van Jacobson <van-/Bx750g9OtTR7s880joybQ@public.gmane.org>
 *  Copyright (C) 2012 Michael D. Taht <dave.taht-1sxU1Nz/FFnXvvmF7DA6hg@public.gmane.org>
 *  Copyright (C) 2012 Eric Dumazet <edumazet-hpIqsD4AKlfQT0dZR+AlfA@public.gmane.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, provided that this notice is retained in full, this
 * software may be distributed under the terms of the GNU General
 * Public License ("GPL") version 2, in which case the provisions of the
 * GPL apply INSTEAD OF those given above.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 */

#include <linux/types.h>
#include <linux/bug.h>
#include <linux/ktime.h>
#include <net/inet_ecn.h>

/* Controlling Queue Delay (CoDel) algorithm 
 * =========================================
 * Source : Kathleen Nichols and Van Jacobson
 * http://queue.acm.org/detail.cfm?id=2209396
 *
 * Implemented on linux by Dave Taht and Eric Dumazet
 */


/*
 * CoDel uses a 1024 nsec clock, encoded in u32
 * This gives a range of 2199 seconds, because of signed compares
 */
typedef u32 codel_time_t;
#define CODEL_SHIFT 10
#define MS2TIME(a) ((a * NSEC_PER_MSEC) >> CODEL_SHIFT)

static inline codel_time_t codel_get_time(void)
{
	u64 ns = ktime_to_ns(ktime_get());

	return ns >> CODEL_SHIFT;
}

#define codel_time_after(a, b)		((s32)(a) - (s32)(b) > 0)
#define codel_time_after_eq(a, b)	((s32)(a) - (s32)(b) >= 0)
#define codel_time_before(a, b)		((s32)(a) - (s32)(b) < 0)
#define codel_time_before_eq(a, b)	((s32)(a) - (s32)(b) <= 0)

struct codel_skb_cb {
	codel_time_t enqueue_time;
};

static struct codel_skb_cb *get_codel_cb(const struct sk_buff *skb)
{
	qdisc_cb_private_validate(skb, sizeof(struct codel_skb_cb));
	return (struct codel_skb_cb *)qdisc_skb_cb(skb)->data;
}

static codel_time_t codel_get_enqueue_time(const struct sk_buff *skb)
{
	return get_codel_cb(skb)->enqueue_time;
}

static void codel_set_enqueue_time(struct sk_buff *skb)
{
	get_codel_cb(skb)->enqueue_time = codel_get_time();
}

static inline u32 codel_time_to_us(codel_time_t val)
{
	u64 valns = ((u64)val << CODEL_SHIFT);

	do_div(valns, NSEC_PER_USEC);
	return (u32)valns;
}

struct codel_params {
	u32		maxpacket; /* largest packet we've seen so far */
	codel_time_t	target;	  /* target queue size (in time units) */
	codel_time_t	interval; /* width of moving time window */
	bool		ecn;	  /* is ECN enabled */
};

struct codel_vars {
	u32		count;	/* how many drops we've done since the last time
				 * we entered dropping state
				 */
	u32		lastcount;	/* count at entry to dropping state */
	bool		dropping;	/* set to true id in dropping state */

	codel_time_t	first_above_time; /* when we went (or will go) continuously
					   * above target for interval
					   */
	codel_time_t	drop_next;	/* time to drop next packet, or when we dropped last */
	codel_time_t	ldelay; /* sojourn time of last dequeued packet */
};

/* contains stats and some shared info */
struct codel_stats {
	struct Qdisc 	*sch;
	u32		drop_count; /* temp count of dropped packets in dequeue() */

	u32		ecn_mark; /* number of packets we ECN marked instead of dropping */
	u32		states; /* number of codel_dequeue() calls */
	u32		state1; /* number of times ok_to_drop was set to true */
	u32		state2;
	u32		state3;
};

static void codel_params_init(struct codel_params *params,
			      const struct Qdisc *sch)
{
	params->maxpacket = 256;
	params->interval = MS2TIME(100);
	params->target = MS2TIME(5);
	params->ecn = false;
}

static void codel_vars_init(struct codel_vars *vars)
{
	vars->drop_next = 0;
	vars->first_above_time = 0;
	vars->dropping = false; /* exit dropping state */
	vars->count = 0;
	vars->lastcount = 0;
}

static void codel_stats_init(struct codel_stats *stats,
			     struct Qdisc *sch)
{
	stats->sch = sch; /* back pointer for qdisc_drop() calls */
}

/* return interval/sqrt(x) with good precision */
static u32 codel_inv_sqrt(u32 _interval, u32 _x)
{
	u64 interval = _interval;
	unsigned long x = _x;

	/* Scale operands for max precision */

#if BITS_PER_LONG == 64
	x <<= 32; /* On 64bit arches, we can prescale x by 32bits */
	interval <<= 16;
#endif

	while (x < (1UL << (BITS_PER_LONG - 2))) {
		x <<= 2;
		interval <<= 1;
	}
	do_div(interval, int_sqrt(x));
	return (u32)interval;
}

static codel_time_t codel_control_law(codel_time_t t,
				      codel_time_t interval,
				      u32 count)
{
	return t + codel_inv_sqrt(interval, count);
}


static bool codel_should_drop(struct sk_buff *skb,
			      unsigned int *backlog,
			      struct codel_vars *vars,
			      struct codel_params *params,
			      struct codel_stats *stats,
			      codel_time_t now)
{
	bool ok_to_drop;

	if (!skb) {
		vars->first_above_time = 0;
		return false;
	}

	vars->ldelay = now - codel_get_enqueue_time(skb);
	*backlog -= qdisc_pkt_len(skb);

	if (unlikely(qdisc_pkt_len(skb) > params->maxpacket))
		params->maxpacket = qdisc_pkt_len(skb);

	if (codel_time_before(vars->ldelay, params->target) ||
	    *backlog <= params->maxpacket) { 
		/* went below - stay below for at least interval */
		vars->first_above_time = 0;
		return false;
	}
	ok_to_drop = false;
	if (vars->first_above_time == 0) {
		/* just went above from below. If we stay above
		 * for at least interval we'll say it's ok to drop
		 */
		vars->first_above_time = now + params->interval;
	} else if (codel_time_after(now, vars->first_above_time)) {
		ok_to_drop = true;
		stats->state1++;
	}
	return ok_to_drop;
}

typedef struct sk_buff * (*codel_skb_dequeue_t)(struct codel_vars *vars,
						struct Qdisc *sch);

static struct sk_buff *codel_dequeue(struct codel_params *params,
				     struct codel_vars *vars,
				     struct codel_stats *stats,
				     codel_skb_dequeue_t dequeue_func,
				     u32 *backlog)
{
	struct sk_buff *skb = dequeue_func(vars, stats->sch);
	codel_time_t now;
	bool drop;

	stats->states++;
	if (!skb) {
		vars->dropping = false;
		return skb;
	}
	now = codel_get_time();
	drop = codel_should_drop(skb, backlog,
				vars, params, stats,
				now);
	if (vars->dropping) {
		if (!drop) {
			/* sojourn time below target - leave dropping state */
			vars->dropping = false;
		} else if (codel_time_after_eq(now, vars->drop_next)) {
			stats->state2++;
			/* It's time for the next drop. Drop the current
			 * packet and dequeue the next. The dequeue might 
			 * take us out of dropping state. 
			 * If not, schedule the next drop.
			 * A large backlog might result in drop rates so high
			 * that the next drop should happen now, 
			 * hence the while loop.
			 */  
			while (vars->dropping && 
			       codel_time_after_eq(now, vars->drop_next)) {
				vars->count++;
				if (params->ecn && INET_ECN_set_ce(skb)) {
					stats->ecn_mark++;
					vars->drop_next =
						codel_control_law(vars->drop_next,
								  params->interval,
								  vars->count);
					goto end;
				}
				qdisc_drop(skb, stats->sch);
				stats->drop_count++;
				skb = dequeue_func(vars, stats->sch);
				if (!codel_should_drop(skb, backlog,
						       vars, params, stats, now)) {
					/* leave dropping state */
					vars->dropping = false;
				} else {
					/* and schedule the next drop */
					vars->drop_next =
						codel_control_law(vars->drop_next,
								  params->interval,
								  vars->count);
				}
			}
		}
	} else if (drop) {
		if (params->ecn && INET_ECN_set_ce(skb)) {
			stats->ecn_mark++;
		} else {
			qdisc_drop(skb, stats->sch);
			stats->drop_count++;

			skb = dequeue_func(vars, stats->sch);
			drop = codel_should_drop(skb, backlog, vars, params,
						 stats, now);
		}
		vars->dropping = true;
		stats->state3++;
		/* 
		 * if min went above target close to when we last went below it
		 * assume that the drop rate that controlled the queue on the
		 * last cycle is a good starting point to control it now.
		 */
		if (codel_time_before(now - vars->drop_next,
				      16 * params->interval)) {
			vars->count = vars->count - vars->lastcount + 1;
		} else {
			vars->count = 1;
		}
		vars->lastcount = vars->count;
		vars->drop_next = codel_control_law(now, params->interval,
						    vars->count);
	}
end:
	return skb;
}
#endif
