
// What if we made this generic AND parallel?

typedef struct {
  u32 time_next_packet;
  // pad this
  u64 rate_ns;
  u16 rate_shift;
} shaper;

static int shaper_overhead(shaper *s; sk_buff *skb) {
}

static int next_packet (shaper *s; sk_buff *skb)
{
}
						    

static int cake_advance_shaper(struct cake_sched_data *q,
			       struct cake_tin_data *b,
			       struct sk_buff *skb,
			       ktime_t now, bool drop)
{
	u32 len = get_cobalt_cb(skb)->adjusted_len;

	/* charge packet bandwidth to this tin
	 * and to the global shaper.
	 */
	if (q->rate_ns) {
		u64 tin_dur = (len * b->tin_rate_ns) >> b->tin_rate_shft;
		u64 global_dur = (len * q->rate_ns) >> q->rate_shft;
		u64 failsafe_dur = global_dur + (global_dur >> 1);

		if (ktime_before(b->time_next_packet, now))
			b->time_next_packet = ktime_add_ns(b->time_next_packet,
							   tin_dur);

		else if (ktime_before(b->time_next_packet,
				      ktime_add_ns(now, tin_dur)))
			b->time_next_packet = ktime_add_ns(now, tin_dur);

		q->time_next_packet = ktime_add_ns(q->time_next_packet,
						   global_dur);
		if (!drop)
			q->failsafe_next_packet = \
				ktime_add_ns(q->failsafe_next_packet,
					     failsafe_dur);
	}
	return len;
}


	} else {
		/* In shaped mode, choose:
		 * - Highest-priority tin with queue and meeting schedule, or
		 * - The earliest-scheduled tin with queue.
		 */
		ktime_t best_time = ns_to_ktime(KTIME_MAX);
		int tin, best_tin = 0;

		for (tin = 0; tin < q->tin_cnt; tin++) {
			b = q->tins + tin;
			if ((b->sparse_flow_count + b->bulk_flow_count) > 0) {
				ktime_t time_to_pkt = \
					ktime_sub(b->time_next_packet, now);

				if (ktime_to_ns(time_to_pkt) <= 0 ||
				    ktime_compare(time_to_pkt,
						  best_time) <= 0) {
					best_time = time_to_pkt;
					best_tin = tin;
				}
			}
		}

		q->cur_tin = best_tin;
		b = q->tins + best_tin;

		/* No point in going further if no packets to deliver. */
		if (unlikely(!(b->sparse_flow_count + b->bulk_flow_count)))
			return NULL;
	}

static void cake_set_rate(struct cake_tin_data *b, u64 rate, u32 mtu,
			  u64 target_ns, u64 rtt_est_ns)
{
	/* convert byte-rate into time-per-byte
	 * so it will always unwedge in reasonable time.
	 */
	static const u64 MIN_RATE = 64;
	u32 byte_target = mtu;
	u64 byte_target_ns;
	u8  rate_shft = 0;
	u64 rate_ns = 0;

	b->flow_quantum = 1514;
	if (rate) {
		b->flow_quantum = max(min(rate >> 12, 1514ULL), 300ULL);
		rate_shft = 34;
		rate_ns = ((u64)NSEC_PER_SEC) << rate_shft;
		rate_ns = div64_u64(rate_ns, max(MIN_RATE, rate));
		while (!!(rate_ns >> 34)) {
			rate_ns >>= 1;
			rate_shft--;
		}
	} /* else unlimited, ie. zero delay */

	b->tin_rate_bps  = rate;
	b->tin_rate_ns   = rate_ns;
	b->tin_rate_shft = rate_shft;

	byte_target_ns = (byte_target * rate_ns) >> rate_shft;

	b->cparams.target = max((byte_target_ns * 3) / 2, target_ns);
	b->cparams.interval = max(rtt_est_ns +
				     b->cparams.target - target_ns,
				     b->cparams.target * 2);
	b->cparams.mtu_time = byte_target_ns;
	b->cparams.p_inc = 1 << 24; /* 1/256 */
	b->cparams.p_dec = 1 << 20; /* 1/4096 */
}


	if (ktime_after(q->time_next_packet, now) && sch->q.qlen) {
		u64 next = min(ktime_to_ns(q->time_next_packet),
			       ktime_to_ns(q->failsafe_next_packet));

		qdisc_watchdog_schedule_ns(&q->watchdog, next);
	} else if (!sch->q.qlen) {
		int i;

		for (i = 0; i < q->tin_cnt; i++) {
			if (q->tins[i].decaying_flow_count) {
				ktime_t next = \
					ktime_add_ns(now,
						     q->tins[i].cparams.target);

				qdisc_watchdog_schedule_ns(&q->watchdog,
							   ktime_to_ns(next));
				break;
			}
		}
	}


	/* ensure shaper state isn't stale */
	if (!b->tin_backlog) {
		if (ktime_before(b->time_next_packet, now))
			b->time_next_packet = now;

		if (!sch->q.qlen) {
			if (ktime_before(q->time_next_packet, now)) {
				q->failsafe_next_packet = now;
				q->time_next_packet = now;
			} else if (ktime_after(q->time_next_packet, now) &&
				   ktime_after(q->failsafe_next_packet, now)) {
				u64 next = \
					min(ktime_to_ns(q->time_next_packet),
					    ktime_to_ns(
						   q->failsafe_next_packet));
				sch->qstats.overlimits++;
				qdisc_watchdog_schedule_ns(&q->watchdog, next);
			}
		}
	}

  
