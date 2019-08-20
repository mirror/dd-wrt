/*
 * Fair Queue Codel
 *
 *  Copyright (C) 2012,2015 Eric Dumazet <edumazet@google.com>
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
#ifdef HAVE_FQ_CODEL_FAST
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

/* FQ_CODEL */

enum {
	TCQ_FQ_CODEL_FAST_UNSPEC,
	TCQ_FQ_CODEL_FAST_TARGET,
	TCQ_FQ_CODEL_FAST_LIMIT,
	TCQ_FQ_CODEL_FAST_INTERVAL,
	TCQ_FQ_CODEL_FAST_ECN,
	TCQ_FQ_CODEL_FAST_FLOWS,
	TCQ_FQ_CODEL_FAST_QUANTUM,
	TCQ_FQ_CODEL_FAST_CE_THRESHOLD,
	TCQ_FQ_CODEL_FAST_DROP_BATCH_SIZE,
	TCQ_FQ_CODEL_FAST_MEMORY_LIMIT,
	__TCQ_FQ_CODEL_FAST_MAX
};

#define TCQ_FQ_CODEL_FAST_MAX	(__TCQ_FQ_CODEL_FAST_MAX - 1)

enum {
	TCQ_FQ_CODEL_FAST_XSTATS_QDISC,
	TCQ_FQ_CODEL_FAST_XSTATS_CLASS,
};

struct tc_fq_codel_fast_qd_stats {
	__u32	maxpacket;	/* largest packet we've seen so far */
	__u32	drop_overlimit; /* number of time max qdisc
				 * packet limit was hit
				 */
	__u32	ecn_mark;	/* number of packets we ECN marked
				 * instead of being dropped
				 */
	__u32	new_flow_count; /* number of time packets
				 * created a 'new flow'
				 */
	__u32	new_flows_len;	/* count of flows in new list */
	__u32	old_flows_len;	/* count of flows in old list */
	__u32	ce_mark;	/* packets above ce_threshold */
	__u32	memory_usage;	/* in bytes */
	__u32	drop_overmemory;
};

struct tc_fq_codel_fast_cl_stats {
	__s32	deficit;
	__u32	ldelay;		/* in-queue delay seen by most recently
				 * dequeued packet
				 */
	__u32	count;
	__u32	lastcount;
	__u32	dropping;
	__s32	drop_next;
};

struct tc_fq_codel_fast_xstats {
	__u32	type;
	union {
		struct tc_fq_codel_fast_qd_stats qdisc_stats;
		struct tc_fq_codel_fast_cl_stats class_stats;
	};
};
#define PRINT_JSON 0x0
#define PRINT_ANY 0x1
#define PRINT_FP 0x2

#define true 1


#define print_uint(dummy,dummy2, fmt, args...) { \
			    if ((dummy == PRINT_ANY || dummy == PRINT_FP) && fmt!=NULL) fprintf(f, fmt, ##args); \
			    }

#define print_string(dummy,dummy2, fmt, args...) { \
			    if ((dummy == PRINT_ANY || dummy == PRINT_FP) && fmt!=NULL) {fprintf(f, fmt, ##args);} \
			    }

#define print_bool(dummy,dummy2, fmt, istrue) { \
			    if ((dummy == PRINT_ANY || dummy == PRINT_FP) && istrue) fprintf(f, fmt); \
			    }


static void explain(void)
{
	fprintf(stderr, "Usage: ... fq_codel [ limit PACKETS ] [ flows NUMBER ]\n");
	fprintf(stderr, "                    [ memory_limit BYTES ]\n");
	fprintf(stderr, "                    [ target TIME ] [ interval TIME ]\n");
	fprintf(stderr, "                    [ quantum BYTES ] [ [no]ecn ]\n");
	fprintf(stderr, "                    [ sce_threshold TIME ]\n");
}

static int fq_codel_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			      struct nlmsghdr *n)
{
	unsigned int limit = 0;
	unsigned int flows = 0;
	unsigned int target = 0;
	unsigned int interval = 0;
	unsigned int quantum = 0;
	unsigned int ce_threshold = ~0U;
	unsigned int memory = ~0U;
	int ecn = -1;
	struct rtattr *tail;

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_unsigned(&limit, *argv, 0)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "flows") == 0) {
			NEXT_ARG();
			if (get_unsigned(&flows, *argv, 0)) {
				fprintf(stderr, "Illegal \"flows\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (get_unsigned(&quantum, *argv, 0)) {
				fprintf(stderr, "Illegal \"quantum\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "target") == 0) {
			NEXT_ARG();
			if (get_time(&target, *argv)) {
				fprintf(stderr, "Illegal \"target\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "sce_threshold") == 0) {
			NEXT_ARG();
			if (get_time(&ce_threshold, *argv)) {
				fprintf(stderr, "Illegal \"sce_threshold\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "memory_limit") == 0) {
			NEXT_ARG();
			if (get_size(&memory, *argv)) {
				fprintf(stderr, "Illegal \"memory_limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "interval") == 0) {
			NEXT_ARG();
			if (get_time(&interval, *argv)) {
				fprintf(stderr, "Illegal \"interval\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "ecn") == 0) {
			ecn = 1;
		} else if (strcmp(*argv, "noecn") == 0) {
			ecn = 0;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	if (limit)
		addattr_l(n, 1024, TCQ_FQ_CODEL_FAST_LIMIT, &limit, sizeof(limit));
	if (flows)
		addattr_l(n, 1024, TCQ_FQ_CODEL_FAST_FLOWS, &flows, sizeof(flows));
	if (quantum)
		addattr_l(n, 1024, TCQ_FQ_CODEL_FAST_QUANTUM, &quantum, sizeof(quantum));
	if (interval)
		addattr_l(n, 1024, TCQ_FQ_CODEL_FAST_INTERVAL, &interval, sizeof(interval));
	if (target)
		addattr_l(n, 1024, TCQ_FQ_CODEL_FAST_TARGET, &target, sizeof(target));
	if (ecn != -1)
		addattr_l(n, 1024, TCQ_FQ_CODEL_FAST_ECN, &ecn, sizeof(ecn));
	if (ce_threshold != ~0U)
		addattr_l(n, 1024, TCQ_FQ_CODEL_FAST_CE_THRESHOLD,
			  &ce_threshold, sizeof(ce_threshold));
	if (memory != ~0U)
		addattr_l(n, 1024, TCQ_FQ_CODEL_FAST_MEMORY_LIMIT,
			  &memory, sizeof(memory));

	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;
	return 0;
}

static int fq_codel_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCQ_FQ_CODEL_FAST_MAX + 1];
	unsigned int limit;
	unsigned int flows;
	unsigned int interval;
	unsigned int target;
	unsigned int ecn;
	unsigned int quantum;
	unsigned int ce_threshold;
	unsigned int memory_limit;

	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCQ_FQ_CODEL_FAST_MAX, opt);

	if (tb[TCQ_FQ_CODEL_FAST_LIMIT] &&
	    RTA_PAYLOAD(tb[TCQ_FQ_CODEL_FAST_LIMIT]) >= sizeof(__u32)) {
		limit = rta_getattr_u32(tb[TCQ_FQ_CODEL_FAST_LIMIT]);
		print_uint(PRINT_ANY, "limit", "limit %up ", limit);
	}
	if (tb[TCQ_FQ_CODEL_FAST_FLOWS] &&
	    RTA_PAYLOAD(tb[TCQ_FQ_CODEL_FAST_FLOWS]) >= sizeof(__u32)) {
		flows = rta_getattr_u32(tb[TCQ_FQ_CODEL_FAST_FLOWS]);
		print_uint(PRINT_ANY, "flows", "flows %u ", flows);
	}
	if (tb[TCQ_FQ_CODEL_FAST_QUANTUM] &&
	    RTA_PAYLOAD(tb[TCQ_FQ_CODEL_FAST_QUANTUM]) >= sizeof(__u32)) {
		quantum = rta_getattr_u32(tb[TCQ_FQ_CODEL_FAST_QUANTUM]);
		print_uint(PRINT_ANY, "quantum", "quantum %u ", quantum);
	}
	if (tb[TCQ_FQ_CODEL_FAST_TARGET] &&
	    RTA_PAYLOAD(tb[TCQ_FQ_CODEL_FAST_TARGET]) >= sizeof(__u32)) {
		target = rta_getattr_u32(tb[TCQ_FQ_CODEL_FAST_TARGET]);
		print_string(PRINT_FP, NULL, "target %s ",
			     sprint_time(target, b1));
	}
	if (tb[TCQ_FQ_CODEL_FAST_CE_THRESHOLD] &&
	    RTA_PAYLOAD(tb[TCQ_FQ_CODEL_FAST_CE_THRESHOLD]) >= sizeof(__u32)) {
		ce_threshold = rta_getattr_u32(tb[TCQ_FQ_CODEL_FAST_CE_THRESHOLD]);
		print_string(PRINT_FP, NULL, "sce_threshold %s ",
			     sprint_time(ce_threshold, b1));
	}
	if (tb[TCQ_FQ_CODEL_FAST_INTERVAL] &&
	    RTA_PAYLOAD(tb[TCQ_FQ_CODEL_FAST_INTERVAL]) >= sizeof(__u32)) {
		interval = rta_getattr_u32(tb[TCQ_FQ_CODEL_FAST_INTERVAL]);
		print_string(PRINT_FP, NULL, "interval %s ",
			     sprint_time(interval, b1));
	}
	if (tb[TCQ_FQ_CODEL_FAST_MEMORY_LIMIT] &&
	    RTA_PAYLOAD(tb[TCQ_FQ_CODEL_FAST_MEMORY_LIMIT]) >= sizeof(__u32)) {
		memory_limit = rta_getattr_u32(tb[TCQ_FQ_CODEL_FAST_MEMORY_LIMIT]);
		print_string(PRINT_FP, NULL, "memory_limit %s ",
			     sprint_size(memory_limit, b1));
	}
	if (tb[TCQ_FQ_CODEL_FAST_ECN] &&
	    RTA_PAYLOAD(tb[TCQ_FQ_CODEL_FAST_ECN]) >= sizeof(__u32)) {
		ecn = rta_getattr_u32(tb[TCQ_FQ_CODEL_FAST_ECN]);
		if (ecn)
			print_bool(PRINT_ANY, "ecn", "ecn ", true);
	}

	return 0;
}

static int fq_codel_print_xstats(struct qdisc_util *qu, FILE *f,
				 struct rtattr *xstats)
{
	struct tc_fq_codel_fast_xstats _st = {}, *st;

	SPRINT_BUF(b1);

	if (xstats == NULL)
		return 0;

	st = RTA_DATA(xstats);
	if (RTA_PAYLOAD(xstats) < sizeof(*st)) {
		memcpy(&_st, st, RTA_PAYLOAD(xstats));
		st = &_st;
	}
	if (st->type == TCQ_FQ_CODEL_FAST_XSTATS_QDISC) {
		print_uint(PRINT_ANY, "maxpacket", "  maxpacket %u",
			st->qdisc_stats.maxpacket);
		print_uint(PRINT_ANY, "drop_overlimit", " drop_overlimit %u",
			st->qdisc_stats.drop_overlimit);
		print_uint(PRINT_ANY, "new_flow_count", " new_flow_count %u",
			st->qdisc_stats.new_flow_count);
		print_uint(PRINT_ANY, "ecn_mark", " ecn_mark %u",
			st->qdisc_stats.ecn_mark);
		if (st->qdisc_stats.ce_mark)
			print_uint(PRINT_ANY, "ce_mark", " ce_mark %u",
				st->qdisc_stats.ce_mark);
		if (st->qdisc_stats.memory_usage)
			print_uint(PRINT_ANY, "memory_used", " memory_used %u",
				st->qdisc_stats.memory_usage);
		if (st->qdisc_stats.drop_overmemory)
			print_uint(PRINT_ANY, "drop_overmemory", " drop_overmemory %u",
				st->qdisc_stats.drop_overmemory);
		print_uint(PRINT_ANY, "new_flows_len", "\n  new_flows_len %u",
			st->qdisc_stats.new_flows_len);
		print_uint(PRINT_ANY, "old_flows_len", " old_flows_len %u",
			st->qdisc_stats.old_flows_len);
	}
	if (st->type == TCQ_FQ_CODEL_FAST_XSTATS_CLASS) {
		print_uint(PRINT_ANY, "deficit", "  deficit %u",
			st->class_stats.deficit);
		print_uint(PRINT_ANY, "count", " count %u",
			st->class_stats.count);
		print_uint(PRINT_ANY, "lastcount", " lastcount %u",
			st->class_stats.lastcount);
		print_string(PRINT_FP, NULL, " ldelay %s",
			sprint_time(st->class_stats.ldelay, b1));
		if (st->class_stats.dropping) {
			print_bool(PRINT_ANY, "dropping", " dropping", true);
			if (st->class_stats.drop_next < 0) {
				print_string(PRINT_FP, NULL, " drop_next -%s",
					sprint_time(-st->class_stats.drop_next, b1));
			}else {
				print_string(PRINT_FP, NULL, " drop_next %s",
					sprint_time(st->class_stats.drop_next, b1));
			}
		}
	}
	return 0;

}

struct qdisc_util fq_codel_fast_qdisc_util = {
	.id		= "fq_codel_fast",
	.parse_qopt	= fq_codel_parse_opt,
	.print_qopt	= fq_codel_print_opt,
	.print_xstats	= fq_codel_print_xstats,
};
#endif