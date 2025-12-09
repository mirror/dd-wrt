/*
 * Copyright (c) Red Hat GmbH.  Author: Phil Sutter <phil@nwl.cc>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <stdio.h>

#include <expression.h>
#include <list.h>
#include <netlink.h>
#include <rule.h>
#include <rt.h>
#include "nftutils.h"

#include <netdb.h>
#include <netinet/icmp6.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_log.h>
#include <linux/netfilter/nf_nat.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/nf_synproxy.h>
#include <linux/xfrm.h>
#include <pwd.h>
#include <grp.h>
#include <jansson.h>
#include <syslog.h>

static json_t *__nft_json_pack(unsigned int line, const char *fmt, ...)
{
	json_error_t error;
	json_t *value;
	va_list ap;

	va_start(ap, fmt);
	value = json_vpack_ex(&error, 0, fmt, ap);
	va_end(ap);

	if (value)
		return value;

	fprintf(stderr, "%s:%d: json_pack failure (%s)\n", __FILE__, line, error.text);
	return NULL;
}
#define nft_json_pack(...)	__nft_json_pack(__LINE__, __VA_ARGS__)

static int json_array_extend_new(json_t *array, json_t *other_array)
{
	int ret;

	ret = json_array_extend(array, other_array);
	json_decref(other_array);
	return ret;
}

static void json_add_array_new(json_t *obj, const char *name, json_t *array)
{
	if (json_array_size(array))
		json_object_set_new(obj, name, array);
	else
		json_decref(array);
}

static json_t *expr_print_json(const struct expr *expr, struct output_ctx *octx)
{
	const struct expr_ops *ops;
	char buf[1024];
	FILE *fp;

	ops = expr_ops(expr);
	if (ops->json)
		return ops->json(expr, octx);

	fprintf(stderr, "warning: expr ops %s have no json callback\n",
		expr_name(expr));

	fp = octx->output_fp;
	octx->output_fp = fmemopen(buf, 1024, "w");

	ops->print(expr, octx);

	fclose(octx->output_fp);
	octx->output_fp = fp;

	return nft_json_pack("s", buf);
}

static json_t *set_dtype_json(const struct expr *key)
{
	char *namedup = xstrdup(key->dtype->name), *tok;
	json_t *root = NULL;
	char *tok_safe;

	tok = strtok_r(namedup, " .", &tok_safe);
	while (tok) {
		json_t *jtok = json_string(tok);
		if (!root)
			root = jtok;
		else if (json_is_string(root))
			root = nft_json_pack("[o, o]", root, jtok);
		else
			json_array_append_new(root, jtok);
		tok = strtok_r(NULL, " .", &tok_safe);
	}
	free(namedup);
	return root;
}

static json_t *set_key_dtype_json(const struct set *set,
				  struct output_ctx *octx)
{
	bool use_typeof = set->key_typeof_valid;

	if (!use_typeof)
		return set_dtype_json(set->key);

	return nft_json_pack("{s:o}", "typeof", expr_print_json(set->key, octx));
}

static json_t *stmt_print_json(const struct stmt *stmt, struct output_ctx *octx)
{
	const struct stmt_ops *ops = stmt_ops(stmt);
	char buf[1024];
	FILE *fp;

	if (ops->json)
		return ops->json(stmt, octx);

	fprintf(stderr, "warning: stmt ops %s have no json callback\n",
		ops->name);

	fp = octx->output_fp;
	octx->output_fp = fmemopen(buf, 1024, "w");

	ops->print(stmt, octx);

	fclose(octx->output_fp);
	octx->output_fp = fp;

	return nft_json_pack("s", buf);
}

static json_t *set_stmt_list_json(const struct list_head *stmt_list,
				   struct output_ctx *octx)
{
	unsigned int flags = octx->flags;
	json_t *root, *tmp;
	struct stmt *i;

	root = json_array();
	octx->flags |= NFT_CTX_OUTPUT_STATELESS;

	list_for_each_entry(i, stmt_list, list) {
		tmp = stmt_print_json(i, octx);
		json_array_append_new(root, tmp);
	}
	octx->flags = flags;

	return root;
}

static json_t *set_print_json(struct output_ctx *octx, const struct set *set)
{
	json_t *root, *tmp, *datatype_ext = NULL;
	const char *type;

	if (set_is_datamap(set->flags)) {
		type = "map";
		datatype_ext = set_dtype_json(set->data);
	} else if (set_is_objmap(set->flags)) {
		type = "map";
		datatype_ext = json_string(obj_type_name(set->objtype));
	} else if (set_is_meter(set->flags)) {
		type = "meter";
	} else {
		type = "set";
	}

	root = nft_json_pack("{s:s, s:s, s:s, s:o, s:I}",
			"family", family2str(set->handle.family),
			"name", set->handle.set.name,
			"table", set->handle.table.name,
			"type", set_key_dtype_json(set, octx),
			"handle", set->handle.handle.id);

	if (set->comment)
		json_object_set_new(root, "comment", json_string(set->comment));
	if (datatype_ext)
		json_object_set_new(root, "map", datatype_ext);

	if (!(set->flags & (NFT_SET_CONSTANT))) {
		if (set->policy != NFT_SET_POL_PERFORMANCE) {
			tmp = nft_json_pack("s", set_policy2str(set->policy));
			json_object_set_new(root, "policy", tmp);
		}
		if (set->desc.size) {
			tmp = nft_json_pack("i", set->desc.size);
			json_object_set_new(root, "size", tmp);
		}
	}

	tmp = json_array();
	if (set->flags & NFT_SET_CONSTANT)
		json_array_append_new(tmp, nft_json_pack("s", "constant"));
	if (set->flags & NFT_SET_INTERVAL)
		json_array_append_new(tmp, nft_json_pack("s", "interval"));
	if (set->flags & NFT_SET_TIMEOUT)
		json_array_append_new(tmp, nft_json_pack("s", "timeout"));
	if (set->flags & NFT_SET_EVAL)
		json_array_append_new(tmp, nft_json_pack("s", "dynamic"));
	json_add_array_new(root, "flags", tmp);

	if (set->timeout) {
		tmp = json_integer(set->timeout / 1000);
		json_object_set_new(root, "timeout", tmp);
	}
	if (set->gc_int) {
		tmp = nft_json_pack("i", set->gc_int / 1000);
		json_object_set_new(root, "gc-interval", tmp);
	}
	if (set->automerge)
		json_object_set_new(root, "auto-merge", json_true());

	if (!nft_output_terse(octx) && set->init && expr_set(set->init)->size > 0) {
		json_t *array = json_array();
		const struct expr *i;

		list_for_each_entry(i, &expr_set(set->init)->expressions, list)
			json_array_append_new(array, expr_print_json(i, octx));

		json_object_set_new(root, "elem", array);
	}

	if (!list_empty(&set->stmt_list)) {
		json_object_set_new(root, "stmt",
				    set_stmt_list_json(&set->stmt_list, octx));
	}

	return nft_json_pack("{s:o}", type, root);
}

/* XXX: Merge with set_print_json()? */
static json_t *element_print_json(struct output_ctx *octx,
				  const struct set *set)
{
	json_t *root = expr_print_json(set->init, octx);

	return nft_json_pack("{s: {s:s, s:s, s:s, s:o}}", "element",
			 "family", family2str(set->handle.family),
			 "table", set->handle.table.name,
			 "name", set->handle.set.name,
			 "elem", root);
}

static json_t *rule_print_json(struct output_ctx *octx,
			       const struct rule *rule)
{
	const struct stmt *stmt;
	json_t *root, *tmp;

	root = nft_json_pack("{s:s, s:s, s:s, s:I}",
			 "family", family2str(rule->handle.family),
			 "table", rule->handle.table.name,
			 "chain", rule->handle.chain.name,
			 "handle", rule->handle.handle.id);
	if (rule->comment)
		json_object_set_new(root, "comment",
				    json_string(rule->comment));

	tmp = json_array();
	list_for_each_entry(stmt, &rule->stmts, list)
		json_array_append_new(tmp, stmt_print_json(stmt, octx));

	if (json_array_size(tmp))
		json_object_set_new(root, "expr", tmp);
	else {
		fprintf(stderr, "rule without statements?!\n");
		json_decref(tmp);
	}

	return nft_json_pack("{s:o}", "rule", root);
}

static json_t *chain_print_json(const struct chain *chain)
{
	json_t *root;

	root = nft_json_pack("{s:s, s:s, s:s, s:I}",
			 "family", family2str(chain->handle.family),
			 "table", chain->handle.table.name,
			 "name", chain->handle.chain.name,
			 "handle", chain->handle.handle.id);

	if (chain->comment)
		json_object_set_new(root, "comment", json_string(chain->comment));

	if (chain->flags & CHAIN_F_BASECHAIN) {
		json_t *tmp = NULL, *devs = NULL;
		int priority = 0, policy, i;

		if (chain->priority.expr)
			mpz_export_data(&priority, chain->priority.expr->value,
					BYTEORDER_HOST_ENDIAN, sizeof(int));

		if (chain->policy) {
			mpz_export_data(&policy, chain->policy->value,
					BYTEORDER_HOST_ENDIAN, sizeof(int));
		} else {
			policy = NF_ACCEPT;
		}

		if (chain->type.str)
			tmp = nft_json_pack("{s:s, s:s, s:i, s:s}",
					"type", chain->type.str,
					"hook", hooknum2str(chain->handle.family,
							    chain->hook.num),
					"prio", priority,
					"policy", chain_policy2str(policy));
		else
			tmp = NULL;

		for (i = 0; i < chain->dev_array_len; i++) {
			const char *dev = chain->dev_array[i];
			if (!devs)
				devs = json_string(dev);
			else if (json_is_string(devs))
				devs = nft_json_pack("[o, s]", devs, dev);
			else
				json_array_append_new(devs, json_string(dev));
		}
		if (devs)
			json_object_set_new(root, "dev", devs);

		if (tmp) {
			json_object_update(root, tmp);
			json_decref(tmp);
		}
	}

	return nft_json_pack("{s:o}", "chain", root);
}

static json_t *proto_name_json(uint8_t proto)
{
	char name[NFT_PROTONAME_MAXSIZE];

	if (nft_getprotobynumber(proto, name, sizeof(name)))
		return json_string(name);
	return json_integer(proto);
}

static json_t *timeout_policy_json(uint8_t l4, const uint32_t *timeout)
{
	json_t *root = NULL;
	unsigned int i;

	for (i = 0; i < timeout_protocol[l4].array_size; i++) {
		if (timeout[i] == timeout_protocol[l4].dflt_timeout[i])
			continue;

		if (!root)
			root = json_object();
		json_object_set_new(root, timeout_protocol[l4].state_to_name[i],
				    json_integer(timeout[i]));
	}
	return root ? : json_null();
}

static json_t *tunnel_erspan_print_json(const struct obj *obj)
{
	json_t *tunnel;

	switch (obj->tunnel.erspan.version) {
	case 1:
		tunnel = json_pack("{s:i, s:i}",
				   "version", obj->tunnel.erspan.version,
				   "index", obj->tunnel.erspan.v1.index);
		break;
	case 2:
		tunnel = json_pack("{s:i, s:s, s:i}",
				   "version", obj->tunnel.erspan.version,
				   "dir", obj->tunnel.erspan.v2.direction ?
						"egress" : "ingress",
				   "hwid", obj->tunnel.erspan.v2.hwid);
		break;
	default:
		BUG("Unknown tunnel erspan version %d", obj->tunnel.erspan.version);
	}

	return tunnel;
}

static json_t *obj_print_json(struct output_ctx *octx, const struct obj *obj,
			      bool delete)
{
	const char *rate_unit = NULL, *burst_unit = NULL;
	const char *type = obj_type_name(obj->type);
	json_t *root, *tmp, *flags;
	uint64_t rate, burst;

	root = nft_json_pack("{s:s, s:s, s:s, s:I}",
			"family", family2str(obj->handle.family),
			"name", obj->handle.obj.name,
			"table", obj->handle.table.name,
			"handle", obj->handle.handle.id);

	if (delete)
		goto out;

	if (obj->comment) {
		tmp = nft_json_pack("{s:s}", "comment", obj->comment);
		json_object_update(root, tmp);
		json_decref(tmp);
	}

	switch (obj->type) {
	case NFT_OBJECT_COUNTER:
		tmp = nft_json_pack("{s:I, s:I}",
				"packets", obj->counter.packets,
				"bytes", obj->counter.bytes);
		json_object_update(root, tmp);
		json_decref(tmp);
		break;
	case NFT_OBJECT_QUOTA:
		tmp = nft_json_pack("{s:I, s:I, s:b}",
				"bytes", obj->quota.bytes,
				"used", obj->quota.used,
				"inv", obj->quota.flags & NFT_QUOTA_F_INV);
		json_object_update(root, tmp);
		json_decref(tmp);
		break;
	case NFT_OBJECT_SECMARK:
		tmp = nft_json_pack("{s:s}",
				"context", obj->secmark.ctx);
		json_object_update(root, tmp);
		json_decref(tmp);
		break;
	case NFT_OBJECT_CT_HELPER:
		tmp = nft_json_pack("{s:s, s:o, s:s}",
				"type", obj->ct_helper.name, "protocol",
				proto_name_json(obj->ct_helper.l4proto),
				"l3proto", family2str(obj->ct_helper.l3proto));
		json_object_update(root, tmp);
		json_decref(tmp);
		break;
	case NFT_OBJECT_CT_TIMEOUT:
		tmp = timeout_policy_json(obj->ct_timeout.l4proto,
					  obj->ct_timeout.timeout);
		tmp = nft_json_pack("{s:o, s:s, s:o}",
				"protocol",
				proto_name_json(obj->ct_timeout.l4proto),
				"l3proto", family2str(obj->ct_timeout.l3proto),
				"policy", tmp);
		json_object_update(root, tmp);
		json_decref(tmp);
		break;
	case NFT_OBJECT_CT_EXPECT:
		tmp = nft_json_pack("{s:o, s:I, s:I, s:I, s:s}",
				"protocol",
				proto_name_json(obj->ct_expect.l4proto),
				"dport", obj->ct_expect.dport,
				"timeout", obj->ct_expect.timeout,
				"size", obj->ct_expect.size,
				"l3proto", family2str(obj->ct_expect.l3proto));
		json_object_update(root, tmp);
		json_decref(tmp);
		break;
	case NFT_OBJECT_LIMIT:
		rate = obj->limit.rate;
		burst = obj->limit.burst;

		if (obj->limit.type == NFT_LIMIT_PKT_BYTES) {
			rate_unit = get_rate(obj->limit.rate, &rate);
			burst_unit = get_rate(obj->limit.burst, &burst);
		}

		tmp = nft_json_pack("{s:I, s:s}",
				"rate", rate,
				"per", get_unit(obj->limit.unit));

		if (obj->limit.flags & NFT_LIMIT_F_INV)
			json_object_set_new(tmp, "inv", json_true());
		if (rate_unit)
			json_object_set_new(tmp, "rate_unit",
					    json_string(rate_unit));
		if (burst) {
			json_object_set_new(tmp, "burst", json_integer(burst));
			if (burst_unit)
				json_object_set_new(tmp, "burst_unit",
						    json_string(burst_unit));
		}

		json_object_update(root, tmp);
		json_decref(tmp);
		break;
	case NFT_OBJECT_SYNPROXY:
		tmp = nft_json_pack("{s:i, s:i}",
				    "mss", obj->synproxy.mss,
				    "wscale", obj->synproxy.wscale);

		flags = json_array();
		if (obj->synproxy.flags & NF_SYNPROXY_OPT_TIMESTAMP)
			json_array_append_new(flags, json_string("timestamp"));
		if (obj->synproxy.flags & NF_SYNPROXY_OPT_SACK_PERM)
			json_array_append_new(flags, json_string("sack-perm"));
		json_add_array_new(tmp, "flags", flags);

		json_object_update(root, tmp);
		json_decref(tmp);
		break;
	case NFT_OBJECT_TUNNEL:
		tmp = json_pack("{s:i, s:o, s:o, s:i, s:i, s:i, s:i}",
				"id", obj->tunnel.id,
				obj->tunnel.src->dtype->type == TYPE_IPADDR ? "src-ipv4" : "src-ipv6",
				expr_print_json(obj->tunnel.src, octx),
				obj->tunnel.dst->dtype->type == TYPE_IPADDR ? "dst-ipv4" : "dst-ipv6",
				expr_print_json(obj->tunnel.dst, octx),
				"sport", obj->tunnel.sport,
				"dport", obj->tunnel.dport,
				"tos", obj->tunnel.tos,
				"ttl", obj->tunnel.ttl);

		switch (obj->tunnel.type) {
		case TUNNEL_UNSPEC:
			break;
		case TUNNEL_ERSPAN:
			json_object_set_new(tmp, "type", json_string("erspan"));
			json_object_set_new(tmp, "tunnel",
					    tunnel_erspan_print_json(obj));
			break;
		case TUNNEL_VXLAN:
			json_object_set_new(tmp, "type", json_string("vxlan"));
			json_object_set_new(tmp, "tunnel",
					    json_pack("{s:i}",
						      "gbp",
						      obj->tunnel.vxlan.gbp));
			break;
		case TUNNEL_GENEVE:
			struct tunnel_geneve *geneve;
			json_t *opts = json_array();

			list_for_each_entry(geneve, &obj->tunnel.geneve_opts, list) {
				char data_str[256];
				json_t *opt;
				int offset;

				data_str[0] = '0';
				data_str[1] = 'x';
				offset = 2;
				for (uint32_t i = 0; i < geneve->data_len; i++)
					offset += snprintf(data_str + offset,
							   3, "%x", geneve->data[i]);

				opt = json_pack("{s:i, s:i, s:s}",
						"class", geneve->geneve_class,
						"opt-type", geneve->type,
						"data", data_str);
				json_array_append_new(opts, opt);
			}

			json_object_set_new(tmp, "type", json_string("geneve"));
			json_object_set_new(tmp, "tunnel", opts);
			break;
		}
		json_object_update(root, tmp);
		json_decref(tmp);
		break;
	}

out:
	return nft_json_pack("{s:o}", type, root);
}

static json_t *flowtable_print_json(const struct flowtable *ftable)
{
	json_t *root, *devs = NULL;
	int i, priority = 0;

	root = nft_json_pack("{s:s, s:s, s:s, s:I}",
			"family", family2str(ftable->handle.family),
			"name", ftable->handle.flowtable.name,
			"table", ftable->handle.table.name,
			"handle", ftable->handle.handle.id);

	if (ftable->priority.expr) {
		json_t *tmp;

		mpz_export_data(&priority, ftable->priority.expr->value,
				BYTEORDER_HOST_ENDIAN, sizeof(int));

		tmp = nft_json_pack("{s:s, s:i}",
				    "hook", hooknum2str(NFPROTO_NETDEV,
							ftable->hook.num),
				    "prio", priority);
		json_object_update_new(root, tmp);
	}

	for (i = 0; i < ftable->dev_array_len; i++) {
		const char *dev = ftable->dev_array[i];
		if (!devs)
			devs = json_string(dev);
		else if (json_is_string(devs))
			devs = nft_json_pack("[o, s]", devs, dev);
		else
			json_array_append_new(devs, json_string(dev));
	}
	if (devs)
		json_object_set_new(root, "dev", devs);

	return nft_json_pack("{s:o}", "flowtable", root);
}

static json_t *table_flags_json(const struct table *table)
{
	uint32_t flags = table->flags;
	json_t *root = json_array(), *tmp;
	int i = 0;

	while (flags) {
		if (flags & 0x1) {
			tmp = json_string(table_flag_name(i));
			json_array_append_new(root, tmp);
		}
		flags >>= 1;
		i++;
	}
	return root;
}

static json_t *table_print_json(const struct table *table)
{
	json_t *root;

	root = nft_json_pack("{s:s, s:s, s:I}",
			 "family", family2str(table->handle.family),
			 "name", table->handle.table.name,
			 "handle", table->handle.handle.id);
	json_add_array_new(root, "flags", table_flags_json(table));

	if (table->comment)
		json_object_set_new(root, "comment", json_string(table->comment));

	return nft_json_pack("{s:o}", "table", root);
}

static json_t *
__binop_expr_json(int op, const struct expr *expr, struct output_ctx *octx)
{
	json_t *a = json_array();

	if (expr->etype == EXPR_BINOP && expr->op == op) {
		json_array_extend_new(a,
				      __binop_expr_json(op, expr->left, octx));
		json_array_extend_new(a,
				      __binop_expr_json(op, expr->right, octx));
	} else {
		json_array_append_new(a, expr_print_json(expr, octx));
	}
	return a;
}

json_t *binop_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	return nft_json_pack("{s:o}", expr_op_symbols[expr->op],
			 __binop_expr_json(expr->op, expr, octx));
}

/* Workaround to retain backwards compatibility in fib output. */
#define __NFT_CTX_OUTPUT_RELATIONAL	(1 << 30)

json_t *relational_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	json_t *ret;

	octx->flags |= __NFT_CTX_OUTPUT_RELATIONAL;
	ret = nft_json_pack("{s:{s:s, s:o, s:o}}", "match",
			    "op", expr_op_symbols[expr->op] ? : "in",
			    "left", expr_print_json(expr->left, octx),
			    "right", expr_print_json(expr->right, octx));
	octx->flags &= ~__NFT_CTX_OUTPUT_RELATIONAL;

	return ret;
}

json_t *range_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	unsigned int flags = octx->flags;
	json_t *root;

	octx->flags &= ~NFT_CTX_OUTPUT_SERVICE;
	octx->flags |= NFT_CTX_OUTPUT_NUMERIC_PROTO;
	root = nft_json_pack("{s:[o, o]}", "range",
			 expr_print_json(expr->left, octx),
			 expr_print_json(expr->right, octx));
	octx->flags = flags;

	return root;
}

json_t *meta_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	return nft_json_pack("{s:{s:s}}", "meta",
			 "key", meta_templates[expr->meta.key].token);
}

json_t *payload_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	json_t *root;

	if (payload_is_known(expr)) {
		if (expr->payload.inner_desc) {
			root = nft_json_pack("{s:s, s:s, s:s}",
					 "tunnel", expr->payload.inner_desc->name,
					 "protocol", expr->payload.desc->name,
					 "field", expr->payload.tmpl->token);
		} else {
			root = nft_json_pack("{s:s, s:s}",
					 "protocol", expr->payload.desc->name,
					 "field", expr->payload.tmpl->token);
		}
	} else {
		root = nft_json_pack("{s:s, s:i, s:i}",
				 "base", proto_base_tokens[expr->payload.base],
				 "offset", expr->payload.offset,
				 "len", expr->len);
	}

	return nft_json_pack("{s:o}", "payload", root);
}

json_t *ct_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	const char *dirstr = ct_dir2str(expr->ct.direction);
	enum nft_ct_keys key = expr->ct.key;
	json_t *root;

	root = nft_json_pack("{s:s}", "key", ct_templates[key].token);

	if (expr->ct.direction < 0)
		goto out;

	if (dirstr)
		json_object_set_new(root, "dir", json_string(dirstr));
out:
	return nft_json_pack("{s:o}", "ct", root);
}

json_t *concat_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	json_t *array = json_array();
	const struct expr *i;

	list_for_each_entry(i, &expr_concat(expr)->expressions, list)
		json_array_append_new(array, expr_print_json(i, octx));

	return nft_json_pack("{s:o}", "concat", array);
}

json_t *set_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	json_t *array = json_array();
	const struct expr *i;

	list_for_each_entry(i, &expr_set(expr)->expressions, list)
		json_array_append_new(array, expr_print_json(i, octx));

	return nft_json_pack("{s:o}", "set", array);
}

json_t *set_ref_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	if (set_is_anonymous(expr->set->flags)) {
		return expr_print_json(expr->set->init, octx);
	} else {
		return nft_json_pack("s+", "@", expr->set->handle.set.name);
	}
}

json_t *set_elem_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	json_t *root = expr_print_json(expr->key, octx);
	struct stmt *stmt;
	json_t *tmp;

	if (!root)
		return NULL;

	/* these element attributes require formal set elem syntax */
	if (expr->timeout || expr->expiration || expr->comment ||
	    !list_empty(&expr->stmt_list)) {
		root = nft_json_pack("{s:o}", "val", root);

		if (expr->timeout) {
			tmp = json_integer(expr->timeout / 1000);
			json_object_set_new(root, "timeout", tmp);
		}
		if (expr->expiration) {
			tmp = json_integer(expr->expiration / 1000);
			json_object_set_new(root, "expires", tmp);
		}
		if (expr->comment) {
			tmp = json_string(expr->comment);
			json_object_set_new(root, "comment", tmp);
		}
		list_for_each_entry(stmt, &expr->stmt_list, list) {
			tmp = stmt_print_json(stmt, octx);
			/* XXX: detect and complain about clashes? */
			json_object_update_missing(root, tmp);
			json_decref(tmp);
			/* TODO: only one statement per element. */
			break;
		}
		return nft_json_pack("{s:o}", "elem", root);
	}

	return root;
}

json_t *prefix_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	json_t *root = expr_print_json(expr->prefix, octx);

	return nft_json_pack("{s:{s:o, s:i}}", "prefix",
			 "addr", root,
			 "len", expr->prefix_len);
}

json_t *list_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	json_t *array = json_array();
	const struct expr *i;

	list_for_each_entry(i, &expr_list(expr)->expressions, list)
		json_array_append_new(array, expr_print_json(i, octx));

	//return nft_json_pack("{s:s, s:o}", "type", "list", "val", array);
	return array;
}

json_t *unary_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	return expr_print_json(expr->arg, octx);
}

json_t *mapping_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	return nft_json_pack("[o, o]",
			 expr_print_json(expr->left, octx),
			 expr_print_json(expr->right, octx));
}

json_t *map_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	const char *type = "map";

	if (expr->mappings->etype == EXPR_SET_REF &&
	    expr->mappings->set->data->dtype->type == TYPE_VERDICT)
		type = "vmap";

	return nft_json_pack("{s:{s:o, s:o}}", type,
			 "key", expr_print_json(expr->map, octx),
			 "data", expr_print_json(expr->mappings, octx));
}

json_t *exthdr_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	const char *desc = expr->exthdr.desc ?
			   expr->exthdr.desc->name : NULL;
	const char *field = expr->exthdr.tmpl->token;
	json_t *root;
	bool is_exists = expr->exthdr.flags & NFT_EXTHDR_F_PRESENT;

	if (expr->exthdr.op == NFT_EXTHDR_OP_TCPOPT) {
		static const char *offstrs[] = { "", "1", "2", "3" };
		unsigned int offset = expr->exthdr.offset / 64;
		const char *offstr = "";

		if (desc) {
			if (offset < 4)
				offstr = offstrs[offset];

			root = nft_json_pack("{s:s+}", "name", desc, offstr);

			if (!is_exists)
				json_object_set_new(root, "field", json_string(field));
		} else {
			root = nft_json_pack("{s:i, s:i, s:i}",
					 "base", expr->exthdr.raw_type,
					 "offset", expr->exthdr.offset,
					 "len", expr->len);
		}

		return nft_json_pack("{s:o}", "tcp option", root);
	}

	if (expr->exthdr.op == NFT_EXTHDR_OP_DCCP) {
		root = nft_json_pack("{s:i}", "type", expr->exthdr.raw_type);
		return nft_json_pack("{s:o}", "dccp option", root);
	}

	root = nft_json_pack("{s:s}", "name", desc);
	if (!is_exists)
		json_object_set_new(root, "field", json_string(field));

	switch (expr->exthdr.op) {
	case NFT_EXTHDR_OP_IPV4:
		return nft_json_pack("{s:o}", "ip option", root);
	case NFT_EXTHDR_OP_SCTP:
		return nft_json_pack("{s:o}", "sctp chunk", root);
	default:
		return nft_json_pack("{s:o}", "exthdr", root);
	}
}

json_t *verdict_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	const struct {
		int verdict;
		const char *name;
		bool chain;
	} verdict_tbl[] = {
		{ NFT_CONTINUE, "continue", false },
		{ NFT_BREAK, "break", false },
		{ NFT_JUMP, "jump", true },
		{ NFT_GOTO, "goto", true },
		{ NFT_RETURN, "return", false },
		{ NF_ACCEPT, "accept", false },
		{ NF_DROP, "drop", false },
		{ NF_QUEUE, "queue", false },
	};
	const char *name = NULL;
	json_t *chain = NULL;
	unsigned int i;

	for (i = 0; i < array_size(verdict_tbl); i++) {
		if (expr->verdict == verdict_tbl[i].verdict) {
			name = verdict_tbl[i].name;
			if (verdict_tbl[i].chain && expr->chain)
				chain = expr_print_json(expr->chain, octx);
			break;
		}
	}
	if (!name) {
		BUG("Unknown verdict %d.", expr->verdict);
		return NULL;
	}
	if (chain)
		return nft_json_pack("{s:{s:o}}", name, "target", chain);
	else
		return nft_json_pack("{s:n}", name);
}

json_t *rt_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	const char *key = rt_templates[expr->rt.key].token;
	json_t *root = nft_json_pack("{s:s}", "key", key);
	const char *family = NULL;

	switch (expr->rt.key) {
	case NFT_RT_NEXTHOP4:
		family = "ip";
		break;
	case NFT_RT_NEXTHOP6:
		family = "ip6";
		break;
	default:
		break;
	}

	if (family)
		json_object_set_new(root, "family", json_string(family));

	return nft_json_pack("{s:o}", "rt", root);
}

json_t *numgen_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	const char *mode;

	switch (expr->numgen.type) {
	case NFT_NG_INCREMENTAL:
		mode = "inc";
		break;
	case NFT_NG_RANDOM:
		mode = "random";
		break;
	default:
		mode = "unknown";
		break;
	}

	return nft_json_pack("{s:{s:s, s:i, s:i}}", "numgen",
			 "mode", mode,
			 "mod", expr->numgen.mod,
			 "offset", expr->numgen.offset);
}

json_t *hash_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	const char *type;
	json_t *root, *jexpr = NULL;

	switch (expr->hash.type) {
	case NFT_HASH_SYM:
		type = "symhash";
		break;
	case NFT_HASH_JENKINS:
	default:
		type = "jhash";
		jexpr = expr_print_json(expr->hash.expr, octx);
		break;
	}

	root = nft_json_pack("{s:i}", "mod", expr->hash.mod);
	if (expr->hash.seed_set)
		json_object_set_new(root, "seed",
				    json_integer(expr->hash.seed));
	if (expr->hash.offset)
		json_object_set_new(root, "offset",
				json_integer(expr->hash.offset));
	if (jexpr)
		json_object_set_new(root, "expr", jexpr);

	return nft_json_pack("{s:o}", type, root);
}

json_t *fib_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	const char *fib_flags[] = { "saddr", "daddr", "mark", "iif", "oif" };
	unsigned int flags = expr->fib.flags & ~NFTA_FIB_F_PRESENT;
	bool check = !(octx->flags & __NFT_CTX_OUTPUT_RELATIONAL);
	json_t *root;

	root = nft_json_pack("{s:s}", "result", fib_result_str(expr, check));

	if (flags) {
		json_t *tmp = json_array();
		unsigned int i;

		for (i = 0; i < array_size(fib_flags); i++) {
			if (flags & (1 << i)) {
				json_array_append_new(tmp, json_string(fib_flags[i]));
				flags &= ~(1 << i);
			}
		}
		if (flags)
			json_array_append_new(tmp, json_integer(flags));

		json_add_array_new(root, "flags", tmp);
	}
	return nft_json_pack("{s:o}", "fib", root);
}

static json_t *symbolic_constant_json(const struct symbol_table *tbl,
				      const struct expr *expr,
				      struct output_ctx *octx)
{
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE);
	const struct symbolic_constant *s;
	uint64_t val = 0;

	/* Export the data in the correct byteorder for comparison */
	assert(expr->len / BITS_PER_BYTE <= sizeof(val));
	mpz_export_data(constant_data_ptr(val, expr->len), expr->value,
			expr->byteorder, len);

	for (s = tbl->symbols; s->identifier != NULL; s++) {
		if (val == s->value)
			break;
	}
	if (!s->identifier)
		return expr_basetype(expr)->json(expr, octx);

	if (nft_output_numeric_symbol(octx))
		return json_integer(val);
	else
		return json_string(s->identifier);
}

json_t *set_elem_catchall_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	return json_string("*");
}

static json_t *datatype_json(const struct expr *expr, struct output_ctx *octx)
{
	const struct datatype *dtype = expr->dtype;

	do {
		if (dtype->json)
			return dtype->json(expr, octx);
		if (dtype->sym_tbl)
			return symbolic_constant_json(dtype->sym_tbl,
						      expr, octx);
		if (dtype->print) {
			char buf[1024];
			FILE *ofp = octx->output_fp;

			octx->output_fp = fmemopen(buf, 1024, "w");
			dtype->print(expr, octx);
			fclose(octx->output_fp);
			octx->output_fp = ofp;

			if (buf[0] == '"') {
				memmove(buf, buf + 1, strlen(buf));
				*strchrnul(buf, '"') = '\0';
			}

			return json_string(buf);
		}
	} while ((dtype = dtype->basetype));

	BUG("datatype %s has no print method or symbol table",
	    expr->dtype->name);
}

json_t *constant_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	return datatype_json(expr, octx);
}

json_t *socket_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	return nft_json_pack("{s:{s:s}}", "socket", "key",
			 socket_templates[expr->socket.key].token);
}

json_t *osf_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	json_t *root;

	if (expr->osf.flags & NFT_OSF_F_VERSION)
		root = nft_json_pack("{s:s}", "key", "version");
	else
		root = nft_json_pack("{s:s}", "key", "name");

	switch (expr->osf.ttl) {
	case 1:
		json_object_set_new(root, "ttl", json_string("loose"));
		break;
	case 2:
		json_object_set_new(root, "ttl", json_string("skip"));
		break;
	}

	return nft_json_pack("{s:o}", "osf", root);
}

json_t *xfrm_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	const char *name = xfrm_templates[expr->xfrm.key].token;
	const char *family = NULL;
	const char *dirstr;
	json_t *root;

	switch (expr->xfrm.direction) {
	case XFRM_POLICY_IN:
		dirstr = "in";
		break;
	case XFRM_POLICY_OUT:
		dirstr = "out";
		break;
	default:
		return NULL;
	}

	switch (expr->xfrm.key) {
	case NFT_XFRM_KEY_UNSPEC:
	case NFT_XFRM_KEY_SPI:
	case NFT_XFRM_KEY_REQID:
	case __NFT_XFRM_KEY_MAX:
		break;
	case NFT_XFRM_KEY_DADDR_IP4:
	case NFT_XFRM_KEY_SADDR_IP4:
		family = "ip";
		break;
	case NFT_XFRM_KEY_DADDR_IP6:
	case NFT_XFRM_KEY_SADDR_IP6:
		family = "ip6";
		break;
	}

	root = nft_json_pack("{s:s}", "key", name);

	if (family)
		json_object_set_new(root, "family", json_string(family));

	json_object_set_new(root, "dir", json_string(dirstr));
	json_object_set_new(root, "spnum", json_integer(expr->xfrm.spnum));

	return nft_json_pack("{s:o}", "ipsec", root);
}

json_t *tunnel_expr_json(const struct expr *expr, struct output_ctx *octx)
{
	return json_pack("{s:{s:s}}", "tunnel",
			 "key", tunnel_templates[expr->tunnel.key].token);
}

json_t *integer_type_json(const struct expr *expr, struct output_ctx *octx)
{
	char buf[1024] = "0x";

	if (mpz_fits_ulong_p(expr->value))
		return json_integer(mpz_get_ui(expr->value));

	mpz_get_str(buf + 2, 16, expr->value);
	return json_string(buf);
}

json_t *string_type_json(const struct expr *expr, struct output_ctx *octx)
{
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE);
	char data[len+1];

	mpz_export_data(data, expr->value, BYTEORDER_HOST_ENDIAN, len);
	data[len] = '\0';

	return json_string(data);
}

json_t *boolean_type_json(const struct expr *expr, struct output_ctx *octx)
{
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE);
	uint64_t val = 0;

	/* Export the data in the correct byteorder for comparison */
	assert(expr->len / BITS_PER_BYTE <= sizeof(val));
	mpz_export_data(constant_data_ptr(val, expr->len), expr->value,
			expr->byteorder, len);

	return json_boolean((int)val);
}

json_t *inet_protocol_type_json(const struct expr *expr,
				struct output_ctx *octx)
{
	if (!nft_output_numeric_proto(octx)) {
		char name[NFT_PROTONAME_MAXSIZE];

		if (nft_getprotobynumber(mpz_get_uint8(expr->value), name, sizeof(name)))
			return json_string(name);
	}
	return integer_type_json(expr, octx);
}

json_t *inet_service_type_json(const struct expr *expr, struct output_ctx *octx)
{
	uint16_t port = mpz_get_be16(expr->value);
	char name[NFT_SERVNAME_MAXSIZE];

	if (!nft_output_service(octx) ||
	    !nft_getservbyport(port, NULL, name, sizeof(name)))
		return json_integer(ntohs(port));

	return json_string(name);
}

json_t *mark_type_json(const struct expr *expr, struct output_ctx *octx)
{
	return symbolic_constant_json(octx->tbl.mark, expr, octx);
}

json_t *devgroup_type_json(const struct expr *expr, struct output_ctx *octx)
{
	return symbolic_constant_json(octx->tbl.devgroup, expr, octx);
}

json_t *ct_label_type_json(const struct expr *expr, struct output_ctx *octx)
{
	unsigned long bit = mpz_scan1(expr->value, 0);
	const char *labelstr = ct_label2str(octx->tbl.ct_label, bit);

	if (labelstr)
		return json_string(labelstr);

	/* can happen when connlabel.conf is altered after rules were added */
	return json_integer(bit);
}

json_t *time_type_json(const struct expr *expr, struct output_ctx *octx)
{
	return json_integer(mpz_get_uint64(expr->value) / MSEC_PER_SEC);
}

json_t *uid_type_json(const struct expr *expr, struct output_ctx *octx)
{
	uint32_t uid = mpz_get_uint32(expr->value);

	if (nft_output_guid(octx)) {
		struct passwd *pw = getpwuid(uid);

		if (pw)
			return json_string(pw->pw_name);
	}
	return json_integer(uid);
}

json_t *gid_type_json(const struct expr *expr, struct output_ctx *octx)
{
	uint32_t gid = mpz_get_uint32(expr->value);

	if (nft_output_guid(octx)) {
		struct group *gr = getgrgid(gid);

		if (gr)
			return json_string(gr->gr_name);
	}
	return json_integer(gid);
}

json_t *expr_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	return expr_print_json(stmt->expr, octx);
}

json_t *flow_offload_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	return nft_json_pack("{s:{s:s, s:s+}}", "flow",
			 "op", "add", "flowtable",
			 "@", stmt->flow.table_name);
}

json_t *payload_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	return nft_json_pack("{s: {s:o, s:o}}", "mangle",
			 "key", expr_print_json(stmt->payload.expr, octx),
			 "value", expr_print_json(stmt->payload.val, octx));
}

json_t *exthdr_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	return nft_json_pack("{s: {s:o, s:o}}", "mangle",
			 "key", expr_print_json(stmt->exthdr.expr, octx),
			 "value", expr_print_json(stmt->exthdr.val, octx));
}

json_t *quota_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	const char *data_unit;
	uint64_t bytes;
	json_t *root;

	data_unit = get_rate(stmt->quota.bytes, &bytes);
	root = nft_json_pack("{s:I, s:s}",
			 "val", bytes,
			 "val_unit", data_unit);

	if (stmt->quota.flags & NFT_QUOTA_F_INV)
		json_object_set_new(root, "inv", json_true());
	if (!nft_output_stateless(octx) && stmt->quota.used) {
		data_unit = get_rate(stmt->quota.used, &bytes);
		json_object_set_new(root, "used", json_integer(bytes));
		json_object_set_new(root, "used_unit", json_string(data_unit));
	}

	return nft_json_pack("{s:o}", "quota", root);
}

json_t *ct_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	struct expr expr = {
		.ct = {
			.key = stmt->ct.key,
			.direction = stmt->ct.direction,
			.nfproto = 0,
		},
	};

	return nft_json_pack("{s:{s:o, s:o}}", "mangle",
			 "key", ct_expr_json(&expr, octx),
			 "value", expr_print_json(stmt->ct.expr, octx));
}

json_t *limit_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	const char *rate_unit = NULL, *burst_unit = NULL;
	bool inv = stmt->limit.flags & NFT_LIMIT_F_INV;
	uint64_t burst = stmt->limit.burst;
	uint64_t rate = stmt->limit.rate;
	json_t *root;

	if (stmt->limit.type == NFT_LIMIT_PKT_BYTES) {
		rate_unit = get_rate(stmt->limit.rate, &rate);
		burst_unit = get_rate(stmt->limit.burst, &burst);
	}

	root = nft_json_pack("{s:I, s:I, s:s}",
			 "rate", rate,
			 "burst", burst,
			 "per", get_unit(stmt->limit.unit));
	if (inv)
		json_object_set_new(root, "inv", json_boolean(inv));
	if (rate_unit)
		json_object_set_new(root, "rate_unit", json_string(rate_unit));
	if (burst_unit)
		json_object_set_new(root, "burst_unit",
				    json_string(burst_unit));

	return nft_json_pack("{s:o}", "limit", root);
}

json_t *fwd_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root, *tmp;

	root = nft_json_pack("{s:o}", "dev", expr_print_json(stmt->fwd.dev, octx));

	if (stmt->fwd.addr) {
		tmp = json_string(family2str(stmt->fwd.family));
		json_object_set_new(root, "family", tmp);

		tmp = expr_print_json(stmt->fwd.addr, octx);
		json_object_set_new(root, "addr", tmp);
	}

	return nft_json_pack("{s:o}", "fwd", root);
}

json_t *notrack_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	return nft_json_pack("{s:n}", "notrack");
}

json_t *dup_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root;

	if (stmt->dup.to) {
		root = nft_json_pack("{s:o}", "addr", expr_print_json(stmt->dup.to, octx));
		if (stmt->dup.dev)
			json_object_set_new(root, "dev",
					    expr_print_json(stmt->dup.dev, octx));
	} else {
		root = json_null();
	}
	return nft_json_pack("{s:o}", "dup", root);
}

json_t *meta_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root;

	root = nft_json_pack("{s:{s:s}}", "meta",
			 "key", meta_templates[stmt->meta.key].token);
	root = nft_json_pack("{s:o, s:o}",
			 "key", root,
			 "value", expr_print_json(stmt->meta.expr, octx));

	return nft_json_pack("{s:o}", "mangle", root);
}

json_t *log_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root = json_object(), *flags;

	if (stmt->log.flags & STMT_LOG_PREFIX)
		json_object_set_new(root, "prefix", json_string(stmt->log.prefix));

	if (stmt->log.flags & STMT_LOG_GROUP)
		json_object_set_new(root, "group",
				    json_integer(stmt->log.group));
	if (stmt->log.flags & STMT_LOG_SNAPLEN)
		json_object_set_new(root, "snaplen",
				    json_integer(stmt->log.snaplen));
	if (stmt->log.flags & STMT_LOG_QTHRESHOLD)
		json_object_set_new(root, "queue-threshold",
				    json_integer(stmt->log.qthreshold));
	if ((stmt->log.flags & STMT_LOG_LEVEL) &&
	    stmt->log.level != LOG_WARNING)
		json_object_set_new(root, "level",
				    json_string(log_level(stmt->log.level)));

	flags = json_array();

	if ((stmt->log.logflags & NF_LOG_MASK) == NF_LOG_MASK) {
		json_array_append_new(flags, json_string("all"));
	} else {
		if (stmt->log.logflags & NF_LOG_TCPSEQ)
			json_array_append_new(flags,
					      json_string("tcp sequence"));
		if (stmt->log.logflags & NF_LOG_TCPOPT)
			json_array_append_new(flags,
					      json_string("tcp options"));
		if (stmt->log.logflags & NF_LOG_IPOPT)
			json_array_append_new(flags, json_string("ip options"));
		if (stmt->log.logflags & NF_LOG_UID)
			json_array_append_new(flags, json_string("skuid"));
		if (stmt->log.logflags & NF_LOG_MACDECODE)
			json_array_append_new(flags, json_string("ether"));
	}
	json_add_array_new(root, "flags", flags);

	if (!json_object_size(root)) {
		json_decref(root);
		root = json_null();
	}

	return nft_json_pack("{s:o}", "log", root);
}

static json_t *nat_flags_json(uint32_t flags)
{
	json_t *array = json_array();

	if (flags & NF_NAT_RANGE_PROTO_RANDOM)
		json_array_append_new(array, json_string("random"));
	if (flags & NF_NAT_RANGE_PROTO_RANDOM_FULLY)
		json_array_append_new(array, json_string("fully-random"));
	if (flags & NF_NAT_RANGE_PERSISTENT)
		json_array_append_new(array, json_string("persistent"));
	if (flags & NF_NAT_RANGE_NETMAP)
		json_array_append_new(array, json_string("netmap"));
	return array;
}

static json_t *nat_type_flags_json(uint32_t type_flags)
{
	json_t *array = json_array();

	if (type_flags & STMT_NAT_F_PREFIX)
		json_array_append_new(array, json_string("prefix"));

	return array;
}

json_t *nat_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root = json_object();
	json_t *array = nat_flags_json(stmt->nat.flags);

	switch (stmt->nat.family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
		json_object_set_new(root, "family",
				    json_string(family2str(stmt->nat.family)));
		break;
	}

	if (stmt->nat.addr)
		json_object_set_new(root, "addr",
				    expr_print_json(stmt->nat.addr, octx));

	if (stmt->nat.proto)
		json_object_set_new(root, "port",
				    expr_print_json(stmt->nat.proto, octx));

	json_add_array_new(root, "flags", array);

	if (stmt->nat.type_flags) {
		array = nat_type_flags_json(stmt->nat.type_flags);

		json_add_array_new(root, "type_flags", array);
	}

	if (!json_object_size(root)) {
		json_decref(root);
		root = json_null();
	}

	return nft_json_pack("{s:o}", nat_etype2str(stmt->nat.type), root);
}

json_t *reject_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root, *jexpr = NULL;
	const char *type = NULL;

	switch (stmt->reject.type) {
	case NFT_REJECT_TCP_RST:
		type = "tcp reset";
		break;
	case NFT_REJECT_ICMPX_UNREACH:
		type = "icmpx";
		jexpr = expr_print_json(stmt->reject.expr, octx);
		break;
	case NFT_REJECT_ICMP_UNREACH:
		switch (stmt->reject.family) {
		case NFPROTO_IPV4:
			type = "icmp";
			jexpr = expr_print_json(stmt->reject.expr, octx);
			break;
		case NFPROTO_IPV6:
			type = "icmpv6";
			jexpr = expr_print_json(stmt->reject.expr, octx);
			break;
		}
	}

	if (!type && !jexpr)
		return nft_json_pack("{s:n}", "reject");

	root = json_object();
	if (type)
		json_object_set_new(root, "type", json_string(type));
	if (jexpr)
		json_object_set_new(root, "expr", jexpr);

	return nft_json_pack("{s:o}", "reject", root);
}

json_t *counter_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	if (nft_output_stateless(octx))
		return nft_json_pack("{s:n}", "counter");

	return nft_json_pack("{s:{s:I, s:I}}", "counter",
			 "packets", stmt->counter.packets,
			 "bytes", stmt->counter.bytes);
}

json_t *last_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	if (nft_output_stateless(octx) || stmt->last.set == 0)
		return nft_json_pack("{s:n}", "last");

	return nft_json_pack("{s:{s:I}}", "last", "used", stmt->last.used);
}

json_t *set_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root;

	root = nft_json_pack("{s:s, s:o, s:s+}",
			 "op", set_stmt_op_names[stmt->set.op],
			 "elem", expr_print_json(stmt->set.key, octx),
			 "set", "@", stmt->set.set->set->handle.set.name);

	if (!list_empty(&stmt->set.stmt_list)) {
		json_object_set_new(root, "stmt",
				    set_stmt_list_json(&stmt->set.stmt_list,
						       octx));
	}

	return nft_json_pack("{s:o}", "set", root);
}

json_t *map_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root;

	root = nft_json_pack("{s:s, s:o, s:o, s:s+}",
			 "op", set_stmt_op_names[stmt->map.op],
			 "elem", expr_print_json(stmt->map.key, octx),
			 "data", expr_print_json(stmt->map.data, octx),
			 "map", "@", stmt->map.set->set->handle.set.name);

	if (!list_empty(&stmt->map.stmt_list)) {
		json_object_set_new(root, "stmt",
				    set_stmt_list_json(&stmt->map.stmt_list,
						       octx));
	}

	return nft_json_pack("{s:o}", "map", root);
}

json_t *objref_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	const char *name;

	if (stmt->objref.type > NFT_OBJECT_MAX)
		name = "unknown";
	else
		name = objref_type_name(stmt->objref.type);

	return nft_json_pack("{s:o}", name, expr_print_json(stmt->objref.expr, octx));
}

json_t *meter_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	unsigned int flags = octx->flags;
	json_t *root, *tmp;

	octx->flags |= NFT_CTX_OUTPUT_STATELESS;
	tmp = stmt_print_json(stmt->meter.stmt, octx);
	octx->flags = flags;

	root = nft_json_pack("{s:o, s:o, s:i}",
			 "key", expr_print_json(stmt->meter.key, octx),
			 "stmt", tmp,
			 "size", stmt->meter.size);
	if (stmt->meter.set) {
		tmp = json_string(stmt->meter.set->set->handle.set.name);
		json_object_set_new(root, "name", tmp);
	}

	return nft_json_pack("{s:o}", "meter", root);
}

json_t *queue_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root, *flags;

	root = json_object();

	if (stmt->queue.queue)
		json_object_set_new(root, "num",
				    expr_print_json(stmt->queue.queue, octx));

	flags = json_array();
	if (stmt->queue.flags & NFT_QUEUE_FLAG_BYPASS)
		json_array_append_new(flags, json_string("bypass"));
	if (stmt->queue.flags & NFT_QUEUE_FLAG_CPU_FANOUT)
		json_array_append_new(flags, json_string("fanout"));
	json_add_array_new(root, "flags", flags);

	if (!json_object_size(root)) {
		json_decref(root);
		root = json_null();
	}

	return nft_json_pack("{s:o}", "queue", root);
}

json_t *verdict_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	return expr_print_json(stmt->expr, octx);
}

json_t *connlimit_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root = nft_json_pack("{s:i}", "val", stmt->connlimit.count);

	if (stmt->connlimit.flags & NFT_CONNLIMIT_F_INV)
		json_object_set_new(root, "inv", json_true());

	return nft_json_pack("{s:o}", "ct count", root);
}

json_t *tproxy_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *tmp, *root = json_object();

	if (stmt->tproxy.table_family == NFPROTO_INET &&
	    stmt->tproxy.family != NFPROTO_UNSPEC) {
		tmp = json_string(family2str(stmt->tproxy.family));
		json_object_set_new(root, "family", tmp);
	}

	if (stmt->tproxy.addr) {
		tmp = expr_print_json(stmt->tproxy.addr, octx);
		json_object_set_new(root, "addr", tmp);
	}

	if (stmt->tproxy.port) {
		tmp = expr_print_json(stmt->tproxy.port, octx);
		json_object_set_new(root, "port", tmp);
	}

	return nft_json_pack("{s:o}", "tproxy", root);
}

json_t *synproxy_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	json_t *root = json_object(), *flags = json_array();

	if (stmt->synproxy.flags & NF_SYNPROXY_OPT_MSS)
		json_object_set_new(root, "mss",
				    json_integer(stmt->synproxy.mss));
	if (stmt->synproxy.flags & NF_SYNPROXY_OPT_WSCALE)
		json_object_set_new(root, "wscale",
				    json_integer(stmt->synproxy.wscale));
	if (stmt->synproxy.flags & NF_SYNPROXY_OPT_TIMESTAMP)
		json_array_append_new(flags, json_string("timestamp"));
	if (stmt->synproxy.flags & NF_SYNPROXY_OPT_SACK_PERM)
		json_array_append_new(flags, json_string("sack-perm"));

	json_add_array_new(root, "flags", flags);

	if (!json_object_size(root)) {
		json_decref(root);
		root = json_null();
	}

	return nft_json_pack("{s:o}", "synproxy", root);
}

json_t *optstrip_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	return nft_json_pack("{s:o}", "reset",
			 expr_print_json(stmt->optstrip.expr, octx));
}

json_t *xt_stmt_json(const struct stmt *stmt, struct output_ctx *octx)
{
	static const char *xt_typename[NFT_XT_MAX] = {
		[NFT_XT_MATCH]          = "match",
		[NFT_XT_TARGET]         = "target",
		[NFT_XT_WATCHER]        = "watcher",
	};

	return nft_json_pack("{s:{s:s, s:s}}", "xt",
			 "type", xt_typename[stmt->xt.type],
			 "name", stmt->xt.name);
}

static json_t *table_print_json_full(struct netlink_ctx *ctx,
				     struct table *table)
{
	json_t *root = json_array(), *rules = json_array(), *tmp;
	struct flowtable *flowtable;
	struct chain *chain;
	struct rule *rule;
	struct obj *obj;
	struct set *set;

	tmp = table_print_json(table);
	json_array_append_new(root, tmp);

	/* both maps and rules may refer to chains, list them first */
	list_for_each_entry(chain, &table->chain_cache.list, cache.list) {
		tmp = chain_print_json(chain);
		json_array_append_new(root, tmp);
	}
	list_for_each_entry(obj, &table->obj_cache.list, cache.list) {
		tmp = obj_print_json(&ctx->nft->output, obj, false);
		json_array_append_new(root, tmp);
	}
	list_for_each_entry(set, &table->set_cache.list, cache.list) {
		if (set_is_anonymous(set->flags))
			continue;
		tmp = set_print_json(&ctx->nft->output, set);
		json_array_append_new(root, tmp);
	}
	list_for_each_entry(flowtable, &table->ft_cache.list, cache.list) {
		tmp = flowtable_print_json(flowtable);
		json_array_append_new(root, tmp);
	}
	list_for_each_entry(chain, &table->chain_cache.list, cache.list) {
		list_for_each_entry(rule, &chain->rules, list) {
			tmp = rule_print_json(&ctx->nft->output, rule);
			json_array_append_new(rules, tmp);
		}
	}

	json_array_extend_new(root, rules);

	return root;
}

static json_t *do_list_ruleset_json(struct netlink_ctx *ctx, struct cmd *cmd)
{
	unsigned int family = cmd->handle.family;
	json_t *root = json_array();
	struct table *table;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (family != NFPROTO_UNSPEC &&
		    table->handle.family != family)
			continue;

		json_array_extend_new(root, table_print_json_full(ctx, table));
	}

	return root;
}

static json_t *do_list_tables_json(struct netlink_ctx *ctx, struct cmd *cmd)
{
	unsigned int family = cmd->handle.family;
	json_t *root = json_array();
	struct table *table;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (family != NFPROTO_UNSPEC &&
		    table->handle.family != family)
			continue;

		json_array_append_new(root, table_print_json(table));
	}

	return root;
}

static json_t *do_list_table_json(struct netlink_ctx *ctx,
				  struct cmd *cmd, struct table *table)
{
	return table_print_json_full(ctx, table);
}

static json_t *do_list_chain_json(struct netlink_ctx *ctx,
				  struct cmd *cmd, struct table *table)
{
	json_t *root = json_array();
	struct chain *chain;
	struct rule *rule;

	list_for_each_entry(chain, &table->chain_cache.list, cache.list) {
		if (chain->handle.family != cmd->handle.family ||
		    strcmp(cmd->handle.chain.name, chain->handle.chain.name))
			continue;

		json_array_append_new(root, chain_print_json(chain));

		list_for_each_entry(rule, &chain->rules, list) {
			json_t *tmp = rule_print_json(&ctx->nft->output, rule);

			json_array_append_new(root, tmp);
		}
	}

	return root;
}

static json_t *do_list_chains_json(struct netlink_ctx *ctx, struct cmd *cmd)
{
	json_t *root = json_array();
	struct table *table;
	struct chain *chain;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (cmd->handle.family != NFPROTO_UNSPEC &&
		    cmd->handle.family != table->handle.family)
			continue;

		list_for_each_entry(chain, &table->chain_cache.list, cache.list) {
			json_t *tmp = chain_print_json(chain);

			json_array_append_new(root, tmp);
		}
	}

	return root;
}

static json_t *do_list_set_json(struct netlink_ctx *ctx,
				struct cmd *cmd, struct table *table)
{
	struct set *set = cmd->set;

	if (!set) {
		set = set_cache_find(table, cmd->handle.set.name);
		if (set == NULL)
			return json_null();
	}

	return nft_json_pack("[o]", set_print_json(&ctx->nft->output, set));
}

static json_t *do_list_sets_json(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct output_ctx *octx = &ctx->nft->output;
	json_t *root = json_array();
	struct table *table;
	struct set *set;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (cmd->handle.family != NFPROTO_UNSPEC &&
		    cmd->handle.family != table->handle.family)
			continue;

		list_for_each_entry(set, &table->set_cache.list, cache.list) {
			if (cmd->obj == CMD_OBJ_SETS &&
			    !set_is_literal(set->flags))
				continue;
			if (cmd->obj == CMD_OBJ_METERS &&
			    !set_is_meter(set->flags))
				continue;
			if (cmd->obj == CMD_OBJ_MAPS &&
			    !map_is_literal(set->flags))
				continue;
			json_array_append_new(root, set_print_json(octx, set));
		}
	}

	return root;
}

static json_t *do_list_obj_json(struct netlink_ctx *ctx,
				struct cmd *cmd, uint32_t type)
{
	json_t *root = json_array(), *tmp;
	struct table *table;
	struct obj *obj;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (cmd->handle.family != NFPROTO_UNSPEC &&
		    cmd->handle.family != table->handle.family)
			continue;

		if (cmd->handle.table.name &&
		    strcmp(cmd->handle.table.name, table->handle.table.name))
			continue;

		list_for_each_entry(obj, &table->obj_cache.list, cache.list) {
			if (obj->type != type ||
			    (cmd->handle.obj.name &&
			     strcmp(cmd->handle.obj.name, obj->handle.obj.name)))
				continue;

			tmp = obj_print_json(&ctx->nft->output, obj, false);
			json_array_append_new(root, tmp);
		}
	}

	return root;
}

static json_t *do_list_flowtable_json(struct netlink_ctx *ctx,
				      struct cmd *cmd, struct table *table)
{
	json_t *root = json_array();
	struct flowtable *ft;

	ft = ft_cache_find(table, cmd->handle.flowtable.name);
	if (!ft)
		return json_null();

	json_array_append_new(root, flowtable_print_json(ft));

	return root;
}

static json_t *do_list_flowtables_json(struct netlink_ctx *ctx, struct cmd *cmd)
{
	json_t *root = json_array(), *tmp;
	struct flowtable *flowtable;
	struct table *table;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (cmd->handle.family != NFPROTO_UNSPEC &&
		    cmd->handle.family != table->handle.family)
			continue;

		list_for_each_entry(flowtable, &table->ft_cache.list, cache.list) {
			tmp = flowtable_print_json(flowtable);
			json_array_append_new(root, tmp);
		}
	}

	return root;
}

static json_t *generate_json_metainfo(void)
{
	return nft_json_pack("{s: {s:s, s:s, s:i}}", "metainfo",
			 "version", PACKAGE_VERSION,
			 "release_name", RELEASE_NAME,
			 "json_schema_version", JSON_SCHEMA_VERSION);
}

int do_command_list_json(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct table *table = NULL;
	json_t *root = NULL;

	if (cmd->handle.table.name) {
		table = table_cache_find(&ctx->nft->cache.table_cache,
					 cmd->handle.table.name,
					 cmd->handle.family);
		if (!table) {
			errno = ENOENT;
			return -1;
		}
	}

	switch (cmd->obj) {
	case CMD_OBJ_TABLE:
		if (!cmd->handle.table.name) {
			root = do_list_tables_json(ctx, cmd);
			break;
		}
		root = do_list_table_json(ctx, cmd, table);
		break;
	case CMD_OBJ_CHAIN:
		root = do_list_chain_json(ctx, cmd, table);
		break;
	case CMD_OBJ_CHAINS:
		root = do_list_chains_json(ctx, cmd);
		break;
	case CMD_OBJ_SETS:
		root = do_list_sets_json(ctx, cmd);
		break;
	case CMD_OBJ_SET:
		root = do_list_set_json(ctx, cmd, table);
		break;
	case CMD_OBJ_RULES:
	case CMD_OBJ_RULESET:
		root = do_list_ruleset_json(ctx, cmd);
		break;
	case CMD_OBJ_METERS:
		root = do_list_sets_json(ctx, cmd);
		break;
	case CMD_OBJ_METER:
		root = do_list_set_json(ctx, cmd, table);
		break;
	case CMD_OBJ_MAPS:
		root = do_list_sets_json(ctx, cmd);
		break;
	case CMD_OBJ_MAP:
		root = do_list_set_json(ctx, cmd, table);
		break;
	case CMD_OBJ_COUNTER:
	case CMD_OBJ_COUNTERS:
		root = do_list_obj_json(ctx, cmd, NFT_OBJECT_COUNTER);
		break;
	case CMD_OBJ_QUOTA:
	case CMD_OBJ_QUOTAS:
		root = do_list_obj_json(ctx, cmd, NFT_OBJECT_QUOTA);
		break;
	case CMD_OBJ_CT_HELPER:
	case CMD_OBJ_CT_HELPERS:
		root = do_list_obj_json(ctx, cmd, NFT_OBJECT_CT_HELPER);
		break;
	case CMD_OBJ_CT_TIMEOUT:
	case CMD_OBJ_CT_TIMEOUTS:
		root = do_list_obj_json(ctx, cmd, NFT_OBJECT_CT_TIMEOUT);
	case CMD_OBJ_CT_EXPECT:
	case CMD_OBJ_CT_EXPECTATIONS:
		root = do_list_obj_json(ctx, cmd, NFT_OBJECT_CT_EXPECT);
		break;
	case CMD_OBJ_LIMIT:
	case CMD_OBJ_LIMITS:
		root = do_list_obj_json(ctx, cmd, NFT_OBJECT_LIMIT);
		break;
	case CMD_OBJ_SECMARK:
	case CMD_OBJ_SECMARKS:
		root = do_list_obj_json(ctx, cmd, NFT_OBJECT_SECMARK);
		break;
	case CMD_OBJ_SYNPROXY:
	case CMD_OBJ_SYNPROXYS:
		root = do_list_obj_json(ctx, cmd, NFT_OBJECT_SYNPROXY);
		break;
	case CMD_OBJ_TUNNEL:
	case CMD_OBJ_TUNNELS:
		root = do_list_obj_json(ctx, cmd, NFT_OBJECT_TUNNEL);
		break;
	case CMD_OBJ_FLOWTABLE:
		root = do_list_flowtable_json(ctx, cmd, table);
		break;
	case CMD_OBJ_FLOWTABLES:
		root = do_list_flowtables_json(ctx, cmd);
		break;
	case CMD_OBJ_HOOKS:
		return 0;
	case CMD_OBJ_MONITOR:
	case CMD_OBJ_MARKUP:
	case CMD_OBJ_SETELEMS:
	case CMD_OBJ_RULE:
	case CMD_OBJ_EXPR:
	case CMD_OBJ_ELEMENTS:
		errno = EOPNOTSUPP;
		return -1;
	case CMD_OBJ_INVALID:
		BUG("invalid command object type %u", cmd->obj);
		break;
	}

	if (!json_is_array(root)) {
		json_t *tmp = json_array();

		json_array_append_new(tmp, root);
		root = tmp;
	}

	json_array_insert_new(root, 0, generate_json_metainfo());

	root = nft_json_pack("{s:o}", "nftables", root);
	json_dumpf(root, ctx->nft->output.output_fp, 0);
	json_decref(root);
	fprintf(ctx->nft->output.output_fp, "\n");
	fflush(ctx->nft->output.output_fp);
	return 0;
}

static void monitor_print_json(struct netlink_mon_handler *monh,
			       const char *cmd, json_t *obj)
{
	struct nft_ctx *nft = monh->ctx->nft;

	obj = nft_json_pack("{s:o}", cmd, obj);
	if (nft_output_echo(&nft->output) && !nft->json_root) {
		json_array_append_new(nft->json_echo, obj);
	} else {
		json_dumpf(obj, nft->output.output_fp, 0);
		json_decref(obj);
	}
}

void monitor_print_table_json(struct netlink_mon_handler *monh,
			      const char *cmd, struct table *t)
{
	monitor_print_json(monh, cmd, table_print_json(t));
}

void monitor_print_chain_json(struct netlink_mon_handler *monh,
			      const char *cmd, struct chain *c)
{
	monitor_print_json(monh, cmd, chain_print_json(c));
}

void monitor_print_set_json(struct netlink_mon_handler *monh,
			    const char *cmd, struct set *s)
{
	struct output_ctx *octx = &monh->ctx->nft->output;

	monitor_print_json(monh, cmd, set_print_json(octx, s));
}

void monitor_print_element_json(struct netlink_mon_handler *monh,
				const char *cmd, struct set *s)
{
	struct output_ctx *octx = &monh->ctx->nft->output;

	monitor_print_json(monh, cmd, element_print_json(octx, s));
}

void monitor_print_obj_json(struct netlink_mon_handler *monh,
			    const char *cmd, struct obj *o, bool delete)
{
	struct output_ctx *octx = &monh->ctx->nft->output;

	monitor_print_json(monh, cmd, obj_print_json(octx, o, delete));
}

void monitor_print_flowtable_json(struct netlink_mon_handler *monh,
				  const char *cmd, struct flowtable *ft)
{
	monitor_print_json(monh, cmd, flowtable_print_json(ft));
}

void monitor_print_rule_json(struct netlink_mon_handler *monh,
			     const char *cmd, struct rule *r)
{
	struct output_ctx *octx = &monh->ctx->nft->output;

	monitor_print_json(monh, cmd, rule_print_json(octx, r));
}

void json_alloc_echo(struct nft_ctx *nft)
{
	nft->json_echo = json_array();
	if (!nft->json_echo)
		memory_allocation_error();
}
