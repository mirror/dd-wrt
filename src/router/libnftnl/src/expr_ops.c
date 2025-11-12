#include <string.h>
#include <linux_list.h>

#include "expr_ops.h"

/* Unfortunately, __attribute__((constructor)) breaks library static linking */
extern struct expr_ops expr_ops_bitwise;
extern struct expr_ops expr_ops_byteorder;
extern struct expr_ops expr_ops_cmp;
extern struct expr_ops expr_ops_connlimit;
extern struct expr_ops expr_ops_counter;
extern struct expr_ops expr_ops_ct;
extern struct expr_ops expr_ops_dup;
extern struct expr_ops expr_ops_exthdr;
extern struct expr_ops expr_ops_fwd;
extern struct expr_ops expr_ops_immediate;
extern struct expr_ops expr_ops_inner;
extern struct expr_ops expr_ops_last;
extern struct expr_ops expr_ops_limit;
extern struct expr_ops expr_ops_log;
extern struct expr_ops expr_ops_lookup;
extern struct expr_ops expr_ops_masq;
extern struct expr_ops expr_ops_match;
extern struct expr_ops expr_ops_meta;
extern struct expr_ops expr_ops_ng;
extern struct expr_ops expr_ops_nat;
extern struct expr_ops expr_ops_tproxy;
extern struct expr_ops expr_ops_objref;
extern struct expr_ops expr_ops_payload;
extern struct expr_ops expr_ops_range;
extern struct expr_ops expr_ops_redir;
extern struct expr_ops expr_ops_reject;
extern struct expr_ops expr_ops_rt;
extern struct expr_ops expr_ops_queue;
extern struct expr_ops expr_ops_quota;
extern struct expr_ops expr_ops_target;
extern struct expr_ops expr_ops_dynset;
extern struct expr_ops expr_ops_hash;
extern struct expr_ops expr_ops_fib;
extern struct expr_ops expr_ops_flow;
extern struct expr_ops expr_ops_socket;
extern struct expr_ops expr_ops_synproxy;
extern struct expr_ops expr_ops_tunnel;
extern struct expr_ops expr_ops_osf;
extern struct expr_ops expr_ops_xfrm;

static struct expr_ops expr_ops_notrack = {
	.name	= "notrack",
};

static struct expr_ops *expr_ops[] = {
	&expr_ops_bitwise,
	&expr_ops_byteorder,
	&expr_ops_cmp,
	&expr_ops_connlimit,
	&expr_ops_counter,
	&expr_ops_ct,
	&expr_ops_dup,
	&expr_ops_exthdr,
	&expr_ops_fwd,
	&expr_ops_immediate,
	&expr_ops_inner,
	&expr_ops_last,
	&expr_ops_limit,
	&expr_ops_log,
	&expr_ops_lookup,
	&expr_ops_masq,
	&expr_ops_match,
	&expr_ops_meta,
	&expr_ops_ng,
	&expr_ops_nat,
	&expr_ops_tproxy,
	&expr_ops_notrack,
	&expr_ops_payload,
	&expr_ops_range,
	&expr_ops_redir,
	&expr_ops_reject,
	&expr_ops_rt,
	&expr_ops_queue,
	&expr_ops_quota,
	&expr_ops_target,
	&expr_ops_dynset,
	&expr_ops_hash,
	&expr_ops_fib,
	&expr_ops_objref,
	&expr_ops_flow,
	&expr_ops_socket,
	&expr_ops_synproxy,
	&expr_ops_tunnel,
	&expr_ops_osf,
	&expr_ops_xfrm,
	NULL,
};

struct expr_ops *nftnl_expr_ops_lookup(const char *name)
{
	int i = 0;

	while (expr_ops[i] != NULL) {
		if (strcmp(expr_ops[i]->name, name) == 0)
			return expr_ops[i];

		i++;
	}
	return NULL;
}
