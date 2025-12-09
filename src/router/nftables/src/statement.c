/*
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>
#include <syslog.h>
#include <rule.h>

#include <arpa/inet.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_log.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <statement.h>
#include <tcpopt.h>
#include <utils.h>
#include <list.h>
#include <xt.h>
#include <json.h>

#include <netinet/in.h>
#include <linux/netfilter/nf_nat.h>
#include <linux/netfilter/nf_log.h>
#include <linux/netfilter/nf_synproxy.h>

struct stmt *stmt_alloc(const struct location *loc, const struct stmt_ops *ops)
{
	struct stmt *stmt;

	stmt = xzalloc(sizeof(*stmt));
	init_list_head(&stmt->list);
	stmt->location = *loc;
	stmt->type = ops->type;
	return stmt;
}

void stmt_free(struct stmt *stmt)
{
	const struct stmt_ops *ops;

	if (stmt == NULL)
		return;

	ops = stmt_ops(stmt);
	if (ops->destroy)
		ops->destroy(stmt);
	free(stmt);
}

void stmt_list_free(struct list_head *list)
{
	struct stmt *i, *next;

	list_for_each_entry_safe(i, next, list, list) {
		list_del(&i->list);
		stmt_free(i);
	}
}

void stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	const struct stmt_ops *ops = stmt_ops(stmt);

	ops->print(stmt, octx);
}

static void expr_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	expr_print(stmt->expr, octx);
}

static void expr_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->expr);
}

static const struct stmt_ops expr_stmt_ops = {
	.type		= STMT_EXPRESSION,
	.name		= "expression",
	.print		= expr_stmt_print,
	.json		= expr_stmt_json,
	.destroy	= expr_stmt_destroy,
};

struct stmt *expr_stmt_alloc(const struct location *loc, struct expr *expr)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &expr_stmt_ops);
	stmt->expr = expr;
	return stmt;
}

static const struct stmt_ops verdict_stmt_ops = {
	.type		= STMT_VERDICT,
	.name		= "verdict",
	.print		= expr_stmt_print,
	.json		= verdict_stmt_json,
	.destroy	= expr_stmt_destroy,
};

struct stmt *verdict_stmt_alloc(const struct location *loc, struct expr *expr)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &verdict_stmt_ops);
	stmt->expr = expr;
	return stmt;
}

static const char *chain_verdict(const struct expr *expr)
{
	switch (expr->verdict) {
	case NFT_JUMP:
		return "jump";
	case NFT_GOTO:
		return "goto";
	default:
		BUG("unknown chain verdict");
	}
}

static void chain_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	nft_print(octx, "%s {\n", chain_verdict(stmt->chain.expr));
	chain_rules_print(stmt->chain.chain, octx, "\t");
	nft_print(octx, "\t\t}");
}

static void chain_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->chain.expr);
	chain_free(stmt->chain.chain);
}

static const struct stmt_ops chain_stmt_ops = {
	.type		= STMT_CHAIN,
	.name		= "chain",
	.print		= chain_stmt_print,
	.destroy	= chain_stmt_destroy,
};

struct stmt *chain_stmt_alloc(const struct location *loc, struct chain *chain,
			      enum nft_verdicts verdict)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &chain_stmt_ops);
	stmt->chain.chain = chain;
	stmt->chain.expr = verdict_expr_alloc(loc, verdict, NULL);
	stmt->chain.expr->chain_id = chain->handle.chain_id;

	return stmt;
}

static void meter_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	unsigned int flags = octx->flags;

	nft_print(octx, "meter ");
	if (stmt->meter.set) {
		expr_print(stmt->meter.set, octx);
		nft_print(octx, " ");
	}
	nft_print(octx, "size %u { ", stmt->meter.size);
	expr_print(stmt->meter.key, octx);
	nft_print(octx, " ");

	octx->flags |= NFT_CTX_OUTPUT_STATELESS;
	stmt_print(stmt->meter.stmt, octx);
	octx->flags = flags;

	nft_print(octx, " }");

}

static void meter_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->meter.key);
	expr_free(stmt->meter.set);
	stmt_free(stmt->meter.stmt);
	free_const(stmt->meter.name);
}

static const struct stmt_ops meter_stmt_ops = {
	.type		= STMT_METER,
	.name		= "meter",
	.print		= meter_stmt_print,
	.json		= meter_stmt_json,
	.destroy	= meter_stmt_destroy,
};

struct stmt *meter_stmt_alloc(const struct location *loc)
{
	return stmt_alloc(loc, &meter_stmt_ops);
}

static void connlimit_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	nft_print(octx, "ct count %s%u",
		  stmt->connlimit.flags ? "over " : "", stmt->connlimit.count);
}

static const struct stmt_ops connlimit_stmt_ops = {
	.type		= STMT_CONNLIMIT,
	.name		= "connlimit",
	.print		= connlimit_stmt_print,
	.json		= connlimit_stmt_json,
};

struct stmt *connlimit_stmt_alloc(const struct location *loc)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &connlimit_stmt_ops);
	stmt->flags |= STMT_F_STATEFUL;
	return stmt;
}

static void counter_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	nft_print(octx, "counter");

	if (nft_output_stateless(octx))
		return;

	nft_print(octx, " packets %" PRIu64 " bytes %" PRIu64,
		  stmt->counter.packets, stmt->counter.bytes);
}

static const struct stmt_ops counter_stmt_ops = {
	.type		= STMT_COUNTER,
	.name		= "counter",
	.print		= counter_stmt_print,
	.json		= counter_stmt_json,
};

struct stmt *counter_stmt_alloc(const struct location *loc)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &counter_stmt_ops);
	stmt->flags |= STMT_F_STATEFUL;
	return stmt;
}

static void last_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	nft_print(octx, "last");

	if (nft_output_stateless(octx))
		return;

	nft_print(octx, " used ");

	if (stmt->last.set)
		time_print(stmt->last.used, octx);
	else
		nft_print(octx, "never");
}

static const struct stmt_ops last_stmt_ops = {
	.type		= STMT_LAST,
	.name		= "last",
	.print		= last_stmt_print,
	.json		= last_stmt_json,
};

struct stmt *last_stmt_alloc(const struct location *loc)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &last_stmt_ops);
	stmt->flags |= STMT_F_STATEFUL;
	return stmt;
}

static const char *objref_type[NFT_OBJECT_MAX + 1] = {
	[NFT_OBJECT_COUNTER]	= "counter",
	[NFT_OBJECT_QUOTA]	= "quota",
	[NFT_OBJECT_CT_HELPER]	= "ct helper",
	[NFT_OBJECT_LIMIT]	= "limit",
	[NFT_OBJECT_TUNNEL]	= "tunnel",
	[NFT_OBJECT_CT_TIMEOUT] = "ct timeout",
	[NFT_OBJECT_SECMARK]	= "secmark",
	[NFT_OBJECT_SYNPROXY]	= "synproxy",
	[NFT_OBJECT_CT_EXPECT]	= "ct expectation",
};

const char *objref_type_name(uint32_t type)
{
	if (type > NFT_OBJECT_MAX)
		return "unknown";

	return objref_type[type];
}

static void objref_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	switch (stmt->objref.type) {
	case NFT_OBJECT_CT_HELPER:
		nft_print(octx, "ct helper set ");
		break;
	case NFT_OBJECT_CT_TIMEOUT:
		nft_print(octx, "ct timeout set ");
		break;
	case NFT_OBJECT_CT_EXPECT:
		nft_print(octx, "ct expectation set ");
		break;
	case NFT_OBJECT_SECMARK:
		nft_print(octx, "meta secmark set ");
		break;
	default:
		nft_print(octx, "%s name ",
			  objref_type_name(stmt->objref.type));
		break;
	}
	expr_print(stmt->objref.expr, octx);
}

static void objref_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->objref.expr);
}

static const struct stmt_ops objref_stmt_ops = {
	.type		= STMT_OBJREF,
	.name		= "objref",
	.print		= objref_stmt_print,
	.json		= objref_stmt_json,
	.destroy	= objref_stmt_destroy,
};

struct stmt *objref_stmt_alloc(const struct location *loc)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &objref_stmt_ops);
	return stmt;
}

static const char *syslog_level[NFT_LOGLEVEL_MAX + 1] = {
	[NFT_LOGLEVEL_EMERG]	= "emerg",
	[NFT_LOGLEVEL_ALERT]	= "alert",
	[NFT_LOGLEVEL_CRIT]	= "crit",
	[NFT_LOGLEVEL_ERR]	= "err",
	[NFT_LOGLEVEL_WARNING]	= "warn",
	[NFT_LOGLEVEL_NOTICE]	= "notice",
	[NFT_LOGLEVEL_INFO]	= "info",
	[NFT_LOGLEVEL_DEBUG]	= "debug",
	[NFT_LOGLEVEL_AUDIT] 	= "audit"
};

const char *log_level(uint32_t level)
{
	if (level > NFT_LOGLEVEL_MAX)
		return "unknown";

	return syslog_level[level];
}

int log_level_parse(const char *level)
{
	int i;

	for (i = 0; i <= NFT_LOGLEVEL_MAX; i++) {
		if (syslog_level[i] &&
		    !strcmp(level, syslog_level[i]))
			return i;
	}
	return -1;
}

static void log_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	nft_print(octx, "log");
	if (stmt->log.flags & STMT_LOG_PREFIX)
		nft_print(octx, " prefix \"%s\"", stmt->log.prefix);
	if (stmt->log.flags & STMT_LOG_GROUP)
		nft_print(octx, " group %u", stmt->log.group);
	if (stmt->log.flags & STMT_LOG_SNAPLEN)
		nft_print(octx, " snaplen %u", stmt->log.snaplen);
	if (stmt->log.flags & STMT_LOG_QTHRESHOLD)
		nft_print(octx, " queue-threshold %u", stmt->log.qthreshold);
	if ((stmt->log.flags & STMT_LOG_LEVEL) &&
	    stmt->log.level != LOG_WARNING)
		nft_print(octx, " level %s", log_level(stmt->log.level));

	if ((stmt->log.logflags & NF_LOG_MASK) == NF_LOG_MASK) {
		nft_print(octx, " flags all");
	} else {
		if (stmt->log.logflags & (NF_LOG_TCPSEQ | NF_LOG_TCPOPT)) {
			const char *delim = " ";

			nft_print(octx, " flags tcp");
			if (stmt->log.logflags & NF_LOG_TCPSEQ) {
				nft_print(octx, " sequence");
				delim = ",";
			}
			if (stmt->log.logflags & NF_LOG_TCPOPT)
				nft_print(octx, "%soptions",
							delim);
		}
		if (stmt->log.logflags & NF_LOG_IPOPT)
			nft_print(octx, " flags ip options");
		if (stmt->log.logflags & NF_LOG_UID)
			nft_print(octx, " flags skuid");
		if (stmt->log.logflags & NF_LOG_MACDECODE)
			nft_print(octx, " flags ether");
	}
}

static void log_stmt_destroy(struct stmt *stmt)
{
	free_const(stmt->log.prefix);
}

static const struct stmt_ops log_stmt_ops = {
	.type		= STMT_LOG,
	.name		= "log",
	.print		= log_stmt_print,
	.json		= log_stmt_json,
	.destroy	= log_stmt_destroy,
};

struct stmt *log_stmt_alloc(const struct location *loc)
{
	return stmt_alloc(loc, &log_stmt_ops);
}

const char *get_unit(uint64_t u)
{
	switch (u) {
	case 1: return "second";
	case 60: return "minute";
	case 60 * 60: return "hour";
	case 60 * 60 * 24: return "day";
	case 60 * 60 * 24 * 7: return "week";
	}

	return "error";
}

static const char * const data_unit[] = {
	"bytes",
	"kbytes",
	"mbytes",
	NULL
};

const char *get_rate(uint64_t byte_rate, uint64_t *rate)
{
	int i;

	if (!byte_rate) {
		*rate = 0;
		return data_unit[0];
	}

	for (i = 0; data_unit[i + 1] != NULL; i++) {
		if (byte_rate % 1024)
			break;
		byte_rate /= 1024;
	}

	*rate = byte_rate;
	return data_unit[i];
}

static void limit_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	bool inv = stmt->limit.flags & NFT_LIMIT_F_INV;
	const char *data_unit;
	uint64_t rate;

	switch (stmt->limit.type) {
	case NFT_LIMIT_PKTS:
		nft_print(octx, "limit rate %s%" PRIu64 "/%s",
			  inv ? "over " : "", stmt->limit.rate,
			  get_unit(stmt->limit.unit));
		nft_print(octx, " burst %u packets", stmt->limit.burst);
		break;
	case NFT_LIMIT_PKT_BYTES:
		data_unit = get_rate(stmt->limit.rate, &rate);

		nft_print(octx,	"limit rate %s%" PRIu64 " %s/%s",
			  inv ? "over " : "", rate, data_unit,
			  get_unit(stmt->limit.unit));
		if (stmt->limit.burst != 0) {
			uint64_t burst;

			data_unit = get_rate(stmt->limit.burst, &burst);
			nft_print(octx, " burst %" PRIu64 " %s", burst,
				  data_unit);
		}
		break;
	}
}

static const struct stmt_ops limit_stmt_ops = {
	.type		= STMT_LIMIT,
	.name		= "limit",
	.print		= limit_stmt_print,
	.json		= limit_stmt_json,
};

struct stmt *limit_stmt_alloc(const struct location *loc)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &limit_stmt_ops);
	stmt->flags |= STMT_F_STATEFUL;
	return stmt;
}

static void queue_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	struct expr *e = stmt->queue.queue;
	const char *delim = " flags ";

	nft_print(octx, "queue");

	if (stmt->queue.flags & NFT_QUEUE_FLAG_BYPASS) {
		nft_print(octx, "%sbypass", delim);
		delim = ",";
	}

	if (stmt->queue.flags & NFT_QUEUE_FLAG_CPU_FANOUT)
		nft_print(octx, "%sfanout", delim);

	if (e) {
		nft_print(octx, " to ");
		expr_print(stmt->queue.queue, octx);
	} else {
		nft_print(octx, " to 0");
	}
}

static void queue_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->queue.queue);
}

static const struct stmt_ops queue_stmt_ops = {
	.type		= STMT_QUEUE,
	.name		= "queue",
	.print		= queue_stmt_print,
	.json		= queue_stmt_json,
	.destroy	= queue_stmt_destroy,
};

struct stmt *queue_stmt_alloc(const struct location *loc, struct expr *e, uint16_t flags)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &queue_stmt_ops);
	stmt->queue.queue = e;
	stmt->queue.flags = flags;

	return stmt;
}

static void quota_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	bool inv = stmt->quota.flags & NFT_QUOTA_F_INV;
	const char *data_unit;
	uint64_t bytes, used;

	data_unit = get_rate(stmt->quota.bytes, &bytes);
	nft_print(octx, "quota %s%" PRIu64 " %s",
		  inv ? "over " : "", bytes, data_unit);

	if (!nft_output_stateless(octx) && stmt->quota.used) {
		data_unit = get_rate(stmt->quota.used, &used);
		nft_print(octx, " used %" PRIu64 " %s", used, data_unit);
	}
}

static const struct stmt_ops quota_stmt_ops = {
	.type		= STMT_QUOTA,
	.name		= "quota",
	.print		= quota_stmt_print,
	.json		= quota_stmt_json,
};

struct stmt *quota_stmt_alloc(const struct location *loc)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &quota_stmt_ops);
	stmt->flags |= STMT_F_STATEFUL;
	return stmt;
}

static void reject_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	nft_print(octx, "reject");
	switch (stmt->reject.type) {
	case NFT_REJECT_TCP_RST:
		nft_print(octx, " with tcp reset");
		break;
	case NFT_REJECT_ICMPX_UNREACH:
		if (stmt->reject.icmp_code == NFT_REJECT_ICMPX_PORT_UNREACH)
			break;
		nft_print(octx, " with icmpx ");
		expr_print(stmt->reject.expr, octx);
		break;
	case NFT_REJECT_ICMP_UNREACH:
		switch (stmt->reject.family) {
		case NFPROTO_IPV4:
			if (!stmt->reject.verbose_print &&
			     stmt->reject.icmp_code == ICMP_PORT_UNREACH)
				break;
			nft_print(octx, " with icmp ");
			expr_print(stmt->reject.expr, octx);
			break;
		case NFPROTO_IPV6:
			if (!stmt->reject.verbose_print &&
			    stmt->reject.icmp_code == ICMP6_DST_UNREACH_NOPORT)
				break;
			nft_print(octx, " with icmpv6 ");
			expr_print(stmt->reject.expr, octx);
			break;
		}
		break;
	}
}

static void reject_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->reject.expr);
}

static const struct stmt_ops reject_stmt_ops = {
	.type		= STMT_REJECT,
	.name		= "reject",
	.print		= reject_stmt_print,
	.json		= reject_stmt_json,
	.destroy	= reject_stmt_destroy,
};

struct stmt *reject_stmt_alloc(const struct location *loc)
{
	return stmt_alloc(loc, &reject_stmt_ops);
}

static void print_nf_nat_flags(uint32_t flags, struct output_ctx *octx)
{
	const char *delim = " ";

	if (flags == 0)
		return;

	if (flags & NF_NAT_RANGE_PROTO_RANDOM) {
		nft_print(octx, "%srandom", delim);
		delim = ",";
	}

	if (flags & NF_NAT_RANGE_PROTO_RANDOM_FULLY) {
		nft_print(octx, "%sfully-random", delim);
		delim = ",";
	}

	if (flags & NF_NAT_RANGE_PERSISTENT)
		nft_print(octx, "%spersistent", delim);
}

const char *nat_etype2str(enum nft_nat_etypes type)
{
	static const char * const nat_types[] = {
		[NFT_NAT_SNAT]	= "snat",
		[NFT_NAT_DNAT]	= "dnat",
		[NFT_NAT_MASQ]	= "masquerade",
		[NFT_NAT_REDIR]	= "redirect",
	};

	return nat_types[type];
}

static void nat_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	nft_print(octx, "%s", nat_etype2str(stmt->nat.type));
	if (stmt->nat.addr || stmt->nat.proto) {
		switch (stmt->nat.family) {
		case NFPROTO_IPV4:
			nft_print(octx, " ip");
			break;
		case NFPROTO_IPV6:
			nft_print(octx, " ip6");
			break;
		}

		if (stmt->nat.type_flags & STMT_NAT_F_PREFIX)
			nft_print(octx, " prefix");

		nft_print(octx, " to");
	}

	if (stmt->nat.addr) {
		nft_print(octx, " ");
		if (stmt->nat.proto) {
			if (stmt->nat.addr->etype == EXPR_VALUE &&
			    stmt->nat.addr->dtype->type == TYPE_IP6ADDR) {
				nft_print(octx, "[");
				expr_print(stmt->nat.addr, octx);
				nft_print(octx, "]");
			} else if (stmt->nat.addr->etype == EXPR_RANGE &&
				   stmt->nat.addr->left->dtype->type == TYPE_IP6ADDR) {
				nft_print(octx, "[");
				expr_print(stmt->nat.addr->left, octx);
				nft_print(octx, "]-[");
				expr_print(stmt->nat.addr->right, octx);
				nft_print(octx, "]");
			} else {
				expr_print(stmt->nat.addr, octx);
			}
		} else {
			expr_print(stmt->nat.addr, octx);
		}
	}

	if (stmt->nat.proto) {
		if (!stmt->nat.addr)
			nft_print(octx, " ");
		nft_print(octx, ":");
		expr_print(stmt->nat.proto, octx);
	}

	print_nf_nat_flags(stmt->nat.flags, octx);
}

static void nat_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->nat.addr);
	expr_free(stmt->nat.proto);
}

static const struct stmt_ops nat_stmt_ops = {
	.type		= STMT_NAT,
	.name		= "nat",
	.print		= nat_stmt_print,
	.json		= nat_stmt_json,
	.destroy	= nat_stmt_destroy,
};

struct stmt *nat_stmt_alloc(const struct location *loc,
			    enum nft_nat_etypes type)
{
	struct stmt *stmt = stmt_alloc(loc, &nat_stmt_ops);

	stmt->nat.type = type;
	return stmt;
}

const char * const set_stmt_op_names[] = {
	[NFT_DYNSET_OP_ADD]	= "add",
	[NFT_DYNSET_OP_UPDATE]	= "update",
	[NFT_DYNSET_OP_DELETE]  = "delete",
};

static void set_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	unsigned int flags = octx->flags;
	struct stmt *this;

	nft_print(octx, "%s ", set_stmt_op_names[stmt->set.op]);
	expr_print(stmt->set.set, octx);
	nft_print(octx, " { ");
	expr_print(stmt->set.key, octx);
	list_for_each_entry(this, &stmt->set.stmt_list, list) {
		nft_print(octx, " ");
		octx->flags |= NFT_CTX_OUTPUT_STATELESS;
		stmt_print(this, octx);
		octx->flags = flags;
	}
	nft_print(octx, " }");
}

static void set_stmt_destroy(struct stmt *stmt)
{
	struct stmt *this, *next;

	expr_free(stmt->set.key);
	expr_free(stmt->set.set);
	list_for_each_entry_safe(this, next, &stmt->set.stmt_list, list)
		stmt_free(this);
}

static const struct stmt_ops set_stmt_ops = {
	.type		= STMT_SET,
	.name		= "set",
	.print		= set_stmt_print,
	.json		= set_stmt_json,
	.destroy	= set_stmt_destroy,
};

struct stmt *set_stmt_alloc(const struct location *loc)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &set_stmt_ops);
	init_list_head(&stmt->set.stmt_list);

	return stmt;
}

static void map_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	unsigned int flags = octx->flags;
	struct stmt *this;

	nft_print(octx, "%s ", set_stmt_op_names[stmt->map.op]);
	expr_print(stmt->map.set, octx);
	nft_print(octx, " { ");
	expr_print(stmt->map.key, octx);
	list_for_each_entry(this, &stmt->map.stmt_list, list) {
		nft_print(octx, " ");
		octx->flags |= NFT_CTX_OUTPUT_STATELESS;
		stmt_print(this, octx);
		octx->flags = flags;
	}
	nft_print(octx, " : ");
	expr_print(stmt->map.data, octx);
	nft_print(octx, " }");
}

static void map_stmt_destroy(struct stmt *stmt)
{
	struct stmt *this, *next;

	expr_free(stmt->map.key);
	expr_free(stmt->map.data);
	expr_free(stmt->map.set);
	list_for_each_entry_safe(this, next, &stmt->map.stmt_list, list)
		stmt_free(this);
}

static const struct stmt_ops map_stmt_ops = {
	.type		= STMT_MAP,
	.name		= "map",
	.print		= map_stmt_print,
	.destroy	= map_stmt_destroy,
	.json		= map_stmt_json,
};

struct stmt *map_stmt_alloc(const struct location *loc)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &map_stmt_ops);
	init_list_head(&stmt->map.stmt_list);

	return stmt;
}

static void dup_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	nft_print(octx, "dup");
	if (stmt->dup.to != NULL) {
		nft_print(octx, " to ");
		expr_print(stmt->dup.to, octx);

		if (stmt->dup.dev != NULL) {
			nft_print(octx, " device ");
			expr_print(stmt->dup.dev, octx);
		}
	}
}

static void dup_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->dup.to);
	expr_free(stmt->dup.dev);
}

static const struct stmt_ops dup_stmt_ops = {
	.type		= STMT_DUP,
	.name		= "dup",
	.print		= dup_stmt_print,
	.json		= dup_stmt_json,
	.destroy	= dup_stmt_destroy,
};

struct stmt *dup_stmt_alloc(const struct location *loc)
{
	return stmt_alloc(loc, &dup_stmt_ops);
}

static const char * const nfproto_family_name_array[NFPROTO_NUMPROTO] = {
	[NFPROTO_IPV4]	= "ip",
	[NFPROTO_IPV6]	= "ip6",
};

static const char *nfproto_family_name(uint8_t nfproto)
{
	if (nfproto >= NFPROTO_NUMPROTO || !nfproto_family_name_array[nfproto])
		return "unknown";

	return nfproto_family_name_array[nfproto];
}

static void fwd_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	if (stmt->fwd.addr) {
		nft_print(octx, "fwd %s to ",
			  nfproto_family_name(stmt->fwd.family));
		expr_print(stmt->fwd.addr, octx);
		nft_print(octx, " device ");
		expr_print(stmt->fwd.dev, octx);
	} else {
		nft_print(octx, "fwd to ");
		expr_print(stmt->fwd.dev, octx);
	}
}

static void fwd_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->fwd.addr);
	expr_free(stmt->fwd.dev);
}

static const struct stmt_ops fwd_stmt_ops = {
	.type		= STMT_FWD,
	.name		= "fwd",
	.print		= fwd_stmt_print,
	.json		= fwd_stmt_json,
	.destroy	= fwd_stmt_destroy,
};

struct stmt *fwd_stmt_alloc(const struct location *loc)
{
	return stmt_alloc(loc, &fwd_stmt_ops);
}

static void optstrip_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	const struct expr *expr = stmt->optstrip.expr;

	nft_print(octx, "reset ");
	expr_print(expr, octx);
}

static void optstrip_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->optstrip.expr);
}

static const struct stmt_ops optstrip_stmt_ops = {
	.type		= STMT_OPTSTRIP,
	.name		= "optstrip",
	.print		= optstrip_stmt_print,
	.json		= optstrip_stmt_json,
	.destroy	= optstrip_stmt_destroy,
};

struct stmt *optstrip_stmt_alloc(const struct location *loc, struct expr *e)
{
	struct stmt *stmt = stmt_alloc(loc, &optstrip_stmt_ops);

	e->exthdr.flags |= NFT_EXTHDR_F_PRESENT;
	stmt->optstrip.expr = e;

	return stmt;
}

static void tproxy_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	nft_print(octx, "tproxy");

	if (stmt->tproxy.table_family == NFPROTO_INET &&
	    stmt->tproxy.family != NFPROTO_UNSPEC)
		nft_print(octx, " %s", nfproto_family_name(stmt->tproxy.family));
	nft_print(octx, " to");
	if (stmt->tproxy.addr) {
		nft_print(octx, " ");
		if (stmt->tproxy.addr->etype == EXPR_VALUE &&
		    stmt->tproxy.addr->dtype->type == TYPE_IP6ADDR) {
			nft_print(octx, "[");
			expr_print(stmt->tproxy.addr, octx);
			nft_print(octx, "]");
		} else {
			expr_print(stmt->tproxy.addr, octx);
		}
	}
	if (stmt->tproxy.port) {
		if (!stmt->tproxy.addr)
			nft_print(octx, " ");
		nft_print(octx, ":");
		expr_print(stmt->tproxy.port, octx);
	}
}

static void tproxy_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->tproxy.addr);
	expr_free(stmt->tproxy.port);
}

static const struct stmt_ops tproxy_stmt_ops = {
	.type		= STMT_TPROXY,
	.name		= "tproxy",
	.print		= tproxy_stmt_print,
	.json		= tproxy_stmt_json,
	.destroy	= tproxy_stmt_destroy,
};

struct stmt *tproxy_stmt_alloc(const struct location *loc)
{
	return stmt_alloc(loc, &tproxy_stmt_ops);
}

static void xt_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	xt_stmt_xlate(stmt, octx);
}

static const struct stmt_ops xt_stmt_ops = {
	.type		= STMT_XT,
	.name		= "xt",
	.print		= xt_stmt_print,
	.destroy	= xt_stmt_destroy,
	.json		= xt_stmt_json,
};

struct stmt *xt_stmt_alloc(const struct location *loc)
{
	return stmt_alloc(loc, &xt_stmt_ops);
}

static const char *synproxy_sack_to_str(const uint32_t flags)
{
	if (flags & NF_SYNPROXY_OPT_SACK_PERM)
		return " sack-perm";

	return "";
}

static const char *synproxy_timestamp_to_str(const uint32_t flags)
{
	if (flags & NF_SYNPROXY_OPT_TIMESTAMP)
		return " timestamp";

	return "";
}

static void synproxy_stmt_print(const struct stmt *stmt,
				struct output_ctx *octx)
{
	uint32_t flags = stmt->synproxy.flags;
	const char *ts_str = synproxy_timestamp_to_str(flags);
	const char *sack_str = synproxy_sack_to_str(flags);

	if (flags & (NF_SYNPROXY_OPT_MSS | NF_SYNPROXY_OPT_WSCALE))
		nft_print(octx, "synproxy mss %u wscale %u%s%s",
			  stmt->synproxy.mss, stmt->synproxy.wscale,
			  ts_str, sack_str);
	else if (flags & NF_SYNPROXY_OPT_MSS)
		nft_print(octx, "synproxy mss %u%s%s", stmt->synproxy.mss,
			  ts_str, sack_str);
	else if (flags & NF_SYNPROXY_OPT_WSCALE)
		nft_print(octx, "synproxy wscale %u%s%s", stmt->synproxy.wscale,
			  ts_str, sack_str);
	else
		nft_print(octx, "synproxy%s%s", ts_str, sack_str);

}

static const struct stmt_ops synproxy_stmt_ops = {
	.type		= STMT_SYNPROXY,
	.name		= "synproxy",
	.print		= synproxy_stmt_print,
	.json		= synproxy_stmt_json,
};

struct stmt *synproxy_stmt_alloc(const struct location *loc)
{
	return stmt_alloc(loc, &synproxy_stmt_ops);
}

/* For src/optimize.c */
static struct stmt_ops invalid_stmt_ops = {
	.type	= STMT_INVALID,
	.name	= "unsupported",
};

static const struct stmt_ops *__stmt_ops_by_type(enum stmt_types type)
{
	switch (type) {
	case STMT_INVALID: return &invalid_stmt_ops;
	case STMT_EXPRESSION: return &expr_stmt_ops;
	case STMT_VERDICT: return &verdict_stmt_ops;
	case STMT_METER: return &meter_stmt_ops;
	case STMT_COUNTER: return &counter_stmt_ops;
	case STMT_PAYLOAD: return &payload_stmt_ops;
	case STMT_META: return &meta_stmt_ops;
	case STMT_LIMIT: return &limit_stmt_ops;
	case STMT_LOG: return &log_stmt_ops;
	case STMT_REJECT: return &reject_stmt_ops;
	case STMT_NAT: return &nat_stmt_ops;
	case STMT_TPROXY: return &tproxy_stmt_ops;
	case STMT_QUEUE: return &queue_stmt_ops;
	case STMT_CT: return &ct_stmt_ops;
	case STMT_SET: return &set_stmt_ops;
	case STMT_DUP: return &dup_stmt_ops;
	case STMT_FWD: return &fwd_stmt_ops;
	case STMT_XT: return &xt_stmt_ops;
	case STMT_QUOTA: return &quota_stmt_ops;
	case STMT_NOTRACK: return &notrack_stmt_ops;
	case STMT_OBJREF: return &objref_stmt_ops;
	case STMT_EXTHDR: return &exthdr_stmt_ops;
	case STMT_FLOW_OFFLOAD: return &flow_offload_stmt_ops;
	case STMT_CONNLIMIT: return &connlimit_stmt_ops;
	case STMT_MAP: return &map_stmt_ops;
	case STMT_SYNPROXY: return &synproxy_stmt_ops;
	case STMT_CHAIN: return &chain_stmt_ops;
	case STMT_OPTSTRIP: return &optstrip_stmt_ops;
	case STMT_LAST: return &last_stmt_ops;
	default:
		break;
	}

	return NULL;
}

const struct stmt_ops *stmt_ops(const struct stmt *stmt)
{
	const struct stmt_ops *ops;

	ops = __stmt_ops_by_type(stmt->type);
	if (!ops)
		BUG("Unknown statement type %d", stmt->type);

	return ops;
}
