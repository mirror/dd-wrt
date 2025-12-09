/*
 * Copyright (c) Red Hat GmbH.  Author: Phil Sutter <phil@nwl.cc>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <errno.h>
#include <syslog.h>

#include <erec.h>
#include <expression.h>
#include <tcpopt.h>
#include <list.h>
#include <netlink.h>
#include <parser.h>
#include <rule.h>
#include <cmd.h>
#include <sctp_chunk.h>
#include <socket.h>

#include <netdb.h>
#include <netinet/icmp6.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <linux/xfrm.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include <linux/netfilter/nf_log.h>
#include <linux/netfilter/nf_nat.h>
#include <linux/netfilter/nf_synproxy.h>
#include <linux/netfilter/nf_tables.h>
#include <jansson.h>

#include <mnl.h>
#include <libnftnl/rule.h>
#include <linux/netfilter/nfnetlink.h>

#define CTX_F_RHS	(1 << 0)
#define CTX_F_STMT	(1 << 1)
#define CTX_F_PRIMARY	(1 << 2)
#define CTX_F_DTYPE	(1 << 3)
#define CTX_F_SET_RHS	(1 << 4)
#define CTX_F_MANGLE	(1 << 5)
#define CTX_F_SES	(1 << 6)	/* set_elem_expr_stmt */
#define CTX_F_MAP	(1 << 7)	/* LHS of map_expr */
#define CTX_F_CONCAT	(1 << 8)	/* inside concat_expr */
#define CTX_F_COLLAPSED	(1 << 9)

struct json_ctx {
	struct nft_ctx *nft;
	struct list_head *msgs;
	struct list_head *cmds;
	uint32_t flags;
};

#define is_RHS(ctx)	(ctx->flags & CTX_F_RHS)
#define is_STMT(ctx)	(ctx->flags & CTX_F_STMT)
#define is_PRIMARY(ctx)	(ctx->flags & CTX_F_PRIMARY)
#define is_DTYPE(ctx)	(ctx->flags & CTX_F_DTYPE)
#define is_SET_RHS(ctx)	(ctx->flags & CTX_F_SET_RHS)

static char *ctx_flags_to_string(struct json_ctx *ctx)
{
	static char buf[1024];
	const char *sep = "";

	buf[0] = '\0';

	if (is_RHS(ctx)) {
		strcat(buf, sep);
		strcat(buf, "RHS");
		sep = ", ";
	}
	if (is_STMT(ctx)) {
		strcat(buf, sep);
		strcat(buf, "STMT");
		sep = ", ";
	}
	if (is_PRIMARY(ctx)) {
		strcat(buf, sep);
		strcat(buf, "PRIMARY");
		sep = ", ";
	}
	if (is_DTYPE(ctx)) {
		strcat(buf, sep);
		strcat(buf, "DTYPE");
		sep = ", ";
	}
	if (is_SET_RHS(ctx)) {
		strcat(buf, sep);
		strcat(buf, "SET_RHS");
		sep = ", ";
	}
	return buf;
}

/* common parser entry points */

static struct expr *json_parse_expr(struct json_ctx *ctx, json_t *root);
static struct expr *json_parse_rhs_expr(struct json_ctx *ctx, json_t *root);
static struct expr *json_parse_stmt_expr(struct json_ctx *ctx, json_t *root);
static struct expr *json_parse_primary_expr(struct json_ctx *ctx, json_t *root);
static struct expr *json_parse_set_rhs_expr(struct json_ctx *ctx, json_t *root);
static struct expr *json_parse_set_elem_expr_stmt(struct json_ctx *ctx, json_t *root);
static struct expr *json_parse_map_lhs_expr(struct json_ctx *ctx, json_t *root);
static struct expr *json_parse_concat_elem_expr(struct json_ctx *ctx, json_t *root);
static struct stmt *json_parse_stmt(struct json_ctx *ctx, json_t *root);

/* parsing helpers */

const struct location *int_loc = &internal_location;
static struct input_descriptor json_indesc;

static void json_lib_error(struct json_ctx *ctx, json_error_t *err)
{
	struct location loc = {
		.indesc = &json_indesc,
		.line_offset = err->position - err->column,
		.first_line = err->line,
		.first_column = err->column,
		/* no information where problematic part ends :( */
		.last_column = err->column,
	};

	erec_queue(error(&loc, "%s", err->text), ctx->msgs);
}

__attribute__((format(printf, 2, 3)))
static void json_error(struct json_ctx *ctx, const char *fmt, ...)
{
	struct error_record *erec;
	va_list ap;

	va_start(ap, fmt);
	erec = erec_vcreate(EREC_ERROR, int_loc, fmt, ap);
	va_end(ap);
	erec_queue(erec, ctx->msgs);
}

static const char *json_typename(const json_t *val)
{
	const char *type_name[] = {
		[JSON_OBJECT] = "object",
		[JSON_ARRAY] = "array",
		[JSON_STRING] = "string",
		[JSON_INTEGER] = "integer",
		[JSON_REAL] = "real",
		[JSON_TRUE] = "true",
		[JSON_FALSE] = "false",
		[JSON_NULL] = "null"
	};

	return type_name[json_typeof(val)];
}

static int json_unpack_err(struct json_ctx *ctx,
			   json_t *root, const char *fmt, ...)
{
	json_error_t err;
	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = json_vunpack_ex(root, &err, 0, fmt, ap);
	va_end(ap);

	if (rc)
		json_lib_error(ctx, &err);
	return rc;
}

static int json_unpack_stmt(struct json_ctx *ctx, json_t *root,
			    const char **key, json_t **value)
{
	assert(key);
	assert(value);

	if (json_object_size(root) != 1) {
		const char *dump = json_dumps(root, 0);

		json_error(ctx, "Malformed object (too many properties): '%s'.",
			   dump);
		free_const(dump);
		return 1;
	}

	json_object_foreach(root, *key, *value)
		return 0;

	/* not reached */
	return 1;
}

/**
 * parse_flags_array - parse JSON property as an array of flags
 *
 * @ctx:		JSON parser context
 * @obj:		JSON object to extract property from
 * @key:		name of property containing the flags array
 * @flag_parser:	Callback parsing a single flag, returns 0 on error
 *
 * The property value may be a string representing a single flag or an array of
 * strings representing a number of flags whose values are ORed together.
 *
 * @return: Combined flag value, 0 if no such property found or -1 if data is
 *          malformed or flag parsing failed.
 */
static int parse_flags_array(struct json_ctx *ctx, json_t *obj, const char *key,
			     unsigned int (*flag_parser)(const char *flag))
{
	json_t *value = json_object_get(obj, key), *tmp;
	size_t index;
	int ret = 0;

	if (!value)
		return 0;

	if (json_is_string(value)) {
		ret = flag_parser(json_string_value(value));
		return ret ?: -1;
	}

	if (!json_is_array(value)) {
		json_error(ctx,
			   "Expecting string or array in '%s' property.", key);
		return -1;
	}

	json_array_foreach(value, index, tmp) {
		int flag = 0;

		if (json_is_string(tmp))
			flag = flag_parser(json_string_value(tmp));

		if (!flag) {
			json_error(ctx,
				   "Invalid flag in '%s' property array at index %zu.",
				   key, index);
			return -1;
		}

		ret |= flag;
	}

	return ret;
}

static int parse_family(const char *name, uint32_t *family)
{
	unsigned int i;
	struct {
		const char *name;
		int val;
	} family_tbl[] = {
		{ "ip", NFPROTO_IPV4 },
		{ "ip6", NFPROTO_IPV6 },
		{ "inet", NFPROTO_INET },
		{ "arp", NFPROTO_ARP },
		{ "bridge", NFPROTO_BRIDGE },
		{ "netdev", NFPROTO_NETDEV }
	};

	assert(family);

	for (i = 0; i < array_size(family_tbl); i++) {
		if (strcmp(name, family_tbl[i].name))
			continue;

		*family = family_tbl[i].val;
		return 0;
	}
	return -1;
}

static int json_parse_family(struct json_ctx *ctx, json_t *root)
{
	const char *family;

	if (!json_unpack(root, "{s:s}", "family", &family)) {
		uint32_t familyval;

		if (parse_family(family, &familyval) ||
		    (familyval != NFPROTO_IPV6 &&
		     familyval != NFPROTO_IPV4)) {
			json_error(ctx, "Invalid family '%s'.", family);
			return -1;
		}
		return familyval;
	}

	return NFPROTO_UNSPEC;
}

static bool is_keyword(const char *keyword)
{
	const char *keywords[] = {
		"ether",
		"ip",
		"ip6",
		"vlan",
		"arp",
		"dnat",
		"snat",
		"ecn",
		"reset",
		"original",
		"reply",
		"label",
	};
	unsigned int i;

	for (i = 0; i < array_size(keywords); i++) {
		if (!strcmp(keyword, keywords[i]))
			return true;
	}
	return false;
}

static bool is_constant(const char *keyword)
{
	const char *constants[] = {
		"tcp",
		"udp",
		"udplite",
		"esp",
		"ah",
		"icmp",
		"icmpv6",
		"comp",
		"dccp",
		"sctp",
		"redirect",
	};
	unsigned int i;

	for (i = 0; i < array_size(constants); i++) {
		if (!strcmp(keyword, constants[i]))
			return true;
	}
	return false;
}

static struct expr *json_parse_constant(struct json_ctx *ctx, const char *name)
{
	const struct {
		const char *name;
		uint8_t data;
		const struct datatype *dtype;
	} constant_tbl[] = {
		{ "tcp", IPPROTO_TCP, &inet_protocol_type },
		{ "udp", IPPROTO_UDP, &inet_protocol_type },
		{ "udplite", IPPROTO_UDPLITE, &inet_protocol_type },
		{ "esp", IPPROTO_ESP, &inet_protocol_type },
		{ "ah", IPPROTO_AH, &inet_protocol_type },
		{ "icmp", IPPROTO_ICMP, &inet_protocol_type },
		{ "icmpv6", IPPROTO_ICMPV6, &inet_protocol_type },
		{ "comp", IPPROTO_COMP, &inet_protocol_type },
		{ "dccp", IPPROTO_DCCP, &inet_protocol_type },
		{ "sctp", IPPROTO_SCTP, &inet_protocol_type },
		{ "redirect", ICMP_REDIRECT, &icmp_type_type },
	};
	unsigned int i;

	for (i = 0; i < array_size(constant_tbl); i++) {
		if (strcmp(name, constant_tbl[i].name))
			continue;
		return constant_expr_alloc(int_loc,
					   constant_tbl[i].dtype,
					   BYTEORDER_HOST_ENDIAN,
					   BITS_PER_BYTE,
					   &constant_tbl[i].data);
	}
	json_error(ctx, "Unknown constant '%s'.", name);
	return NULL;
}

/* this is a combination of symbol_expr, integer_expr, boolean_expr ... */
static struct expr *json_parse_immediate(struct json_ctx *ctx, json_t *root)
{
	enum symbol_types symtype = SYMBOL_VALUE;
	const char *str;
	char buf[64] = {};

	switch (json_typeof(root)) {
	case JSON_STRING:
		str = json_string_value(root);
		if (str[0] == '@') {
			symtype = SYMBOL_SET;
			str++;
		} else if (str[0] == '*' && str[1] == '\0') {
			return set_elem_catchall_expr_alloc(int_loc);
		} else if (is_keyword(str)) {
			return symbol_expr_alloc(int_loc,
						 SYMBOL_VALUE, NULL, str);
		} else if (is_constant(str)) {
			return json_parse_constant(ctx, str);
		}
		break;
	case JSON_INTEGER:
		snprintf(buf, sizeof(buf),
			 "%" JSON_INTEGER_FORMAT, json_integer_value(root));
		str = buf;
		break;
	case JSON_TRUE:
	case JSON_FALSE:
		buf[0] = json_is_true(root);
		return constant_expr_alloc(int_loc, &boolean_type,
					   BYTEORDER_HOST_ENDIAN,
					   BITS_PER_BYTE, buf);
	default:
		json_error(ctx, "Unexpected JSON type %s for immediate value.",
			   json_typename(root));
		return NULL;
	}

	return symbol_expr_alloc(int_loc, symtype, NULL, str);
}

static struct expr *json_parse_meta_expr(struct json_ctx *ctx,
					 const char *type, json_t *root)
{
	struct error_record *erec;
	unsigned int key;
	const char *name;

	if (json_unpack_err(ctx, root, "{s:s}", "key", &name))
		return NULL;
	erec = meta_key_parse(int_loc, name, &key);
	if (erec) {
		erec_queue(erec, ctx->msgs);
		return NULL;
	}
	return meta_expr_alloc(int_loc, key);
}

static struct expr *json_parse_tunnel_expr(struct json_ctx *ctx,
					   const char *type, json_t *root)
{
	struct error_record *erec;
	unsigned int key;
	const char *name;

	if (json_unpack_err(ctx, root, "{s:s}", "key", &name))
		return NULL;
	erec = tunnel_key_parse(int_loc, name, &key);
	if (erec) {
		erec_queue(erec, ctx->msgs);
		return NULL;
	}
	return tunnel_expr_alloc(int_loc, key);
}

static struct expr *json_parse_osf_expr(struct json_ctx *ctx,
					const char *type, json_t *root)
{
	const char *key, *ttl;
	uint32_t flagval = 0;
	uint8_t ttlval = 0;

	if (json_unpack_err(ctx, root, "{s:s}", "key", &key))
		return NULL;

	if (!json_unpack(root, "{s:s}", "ttl", &ttl)) {
		if (!strcmp(ttl, "loose")) {
			ttlval = 1;
		} else if (!strcmp(ttl, "skip")) {
			ttlval = 2;
		} else {
			json_error(ctx, "Invalid osf ttl option '%s'.", ttl);
			return NULL;
		}
	}

	if (!strcmp(key, "name")) {
		return osf_expr_alloc(int_loc, ttlval, flagval);
	} else if (!strcmp(key, "version")) {
		flagval |= NFT_OSF_F_VERSION;
		return osf_expr_alloc(int_loc, ttlval, flagval);
	}

	json_error(ctx, "Invalid osf key value.");
	return NULL;
}

static struct expr *json_parse_socket_expr(struct json_ctx *ctx,
					   const char *type, json_t *root)
{
	const char *key;
	int keyval = -1;

	if (json_unpack_err(ctx, root, "{s:s}", "key", &key))
		return NULL;

	if (!strcmp(key, "transparent"))
		keyval = NFT_SOCKET_TRANSPARENT;
	else if (!strcmp(key, "mark"))
		keyval = NFT_SOCKET_MARK;
	else if (!strcmp(key, "wildcard"))
		keyval = NFT_SOCKET_WILDCARD;

	if (keyval == -1) {
		json_error(ctx, "Invalid socket key value.");
		return NULL;
	}

	return socket_expr_alloc(int_loc, keyval, 0);
}

static int json_parse_payload_field(const struct proto_desc *desc,
				    const char *name, int *field)
{
	unsigned int i;

	for (i = 0; i < PROTO_HDRS_MAX; i++) {
		if (desc->templates[i].token &&
		    !strcmp(desc->templates[i].token, name)) {
			if (field)
				*field = i;
			return 0;
		}
	}
	return 1;
}

static int json_parse_tcp_option_type(const char *name, int *val)
{
	unsigned int i;

	for (i = 0; i < array_size(tcpopt_protocols); i++) {
		if (tcpopt_protocols[i] &&
		    !strcmp(tcpopt_protocols[i]->name, name)) {
			if (val)
				*val = i;
			return 0;
		}
	}
	/* special case for sack0 - sack3 */
	if (sscanf(name, "sack%u", &i) == 1 && i < 4) {
		if (val && i == 0)
			*val = TCPOPT_KIND_SACK;
		else if (val && i > 0)
			*val = TCPOPT_KIND_SACK1 + i - 1;
		return 0;
	}
	return 1;
}

static int json_parse_tcp_option_field(int type, const char *name, int *val)
{
	const struct exthdr_desc *desc;
	unsigned int block = 0;
	unsigned int i;

	switch (type) {
	case TCPOPT_KIND_SACK1:
		type = TCPOPT_KIND_SACK;
		block = 1;
		break;
	case TCPOPT_KIND_SACK2:
		type = TCPOPT_KIND_SACK;
		block = 2;
		break;
	case TCPOPT_KIND_SACK3:
		type = TCPOPT_KIND_SACK;
		block = 3;
		break;
	}

	if (type < 0 || type >= (int)array_size(tcpopt_protocols))
		return 1;

	desc = tcpopt_protocols[type];
	if (!desc)
		return 1;

	for (i = 0; i < array_size(desc->templates); i++) {
		if (desc->templates[i].token &&
		    !strcmp(desc->templates[i].token, name)) {
			if (block) {
				block--;
				continue;
			}

			if (val)
				*val = i;
			return 0;
		}
	}
	return 1;
}

static const struct proto_desc *proto_lookup_byname(const char *name)
{
	const struct proto_desc *proto_tbl[] = {
		&proto_eth,
		&proto_vlan,
		&proto_arp,
		&proto_ip,
		&proto_icmp,
		&proto_igmp,
		&proto_ip6,
		&proto_icmp6,
		&proto_ah,
		&proto_esp,
		&proto_comp,
		&proto_udp,
		&proto_udplite,
		&proto_tcp,
		&proto_dccp,
		&proto_sctp,
		&proto_th,
		&proto_vxlan,
		&proto_gre,
		&proto_gretap,
		&proto_geneve,
	};
	unsigned int i;

	for (i = 0; i < array_size(proto_tbl); i++) {
		if (!strcmp(proto_tbl[i]->name, name))
			return proto_tbl[i];
	}
	return NULL;
}

static const struct proto_desc *inner_proto_lookup_byname(const char *name)
{
	const struct proto_desc *proto_tbl[] = {
		&proto_geneve,
		&proto_gre,
		&proto_gretap,
		&proto_vxlan,
	};
	unsigned int i;

	for (i = 0; i < array_size(proto_tbl); i++) {
		if (!strcmp(proto_tbl[i]->name, name))
			return proto_tbl[i];
	}
	return NULL;
}

static struct expr *json_parse_payload_expr(struct json_ctx *ctx,
					    const char *type, json_t *root)
{
	const char *tunnel, *protocol, *field, *base;
	int offset, len, val;
	struct expr *expr;

	if (!json_unpack(root, "{s:s, s:i, s:i}",
			 "base", &base, "offset", &offset, "len", &len)) {
		if (!strcmp(base, "ll")) {
			val = PROTO_BASE_LL_HDR;
		} else if (!strcmp(base, "nh")) {
			val = PROTO_BASE_NETWORK_HDR;
		} else if (!strcmp(base, "th")) {
			val = PROTO_BASE_TRANSPORT_HDR;
		} else if (!strcmp(base, "ih")) {
			val = PROTO_BASE_INNER_HDR;
		} else {
			json_error(ctx, "Invalid payload base '%s'.", base);
			return NULL;
		}

		if (len <= 0 || len > (int)NFT_MAX_EXPR_LEN_BITS) {
			json_error(ctx, "Payload length must be between 0 and %lu, got %d",
				   NFT_MAX_EXPR_LEN_BITS, len);
			return NULL;
		}

		expr = payload_expr_alloc(int_loc, NULL, 0);
		payload_init_raw(expr, val, offset, len);
		expr->byteorder		= BYTEORDER_BIG_ENDIAN;
		expr->payload.is_raw	= true;
		return expr;
	} else if (!json_unpack(root, "{s:s, s:s, s:s}",
				"tunnel", &tunnel, "protocol", &protocol, "field", &field)) {
		const struct proto_desc *proto = proto_lookup_byname(protocol);
		const struct proto_desc *inner_proto = inner_proto_lookup_byname(tunnel);

		if (!inner_proto) {
			json_error(ctx, "Unknown payload tunnel protocol '%s'.",
				   tunnel);
			return NULL;
		}
		if (!proto) {
			json_error(ctx, "Unknown payload protocol '%s'.",
				   protocol);
			return NULL;
		}
		if (json_parse_payload_field(proto, field, &val)) {
			json_error(ctx, "Unknown %s field '%s'.",
				   protocol, field);
			return NULL;
		}
		expr = payload_expr_alloc(int_loc, proto, val);
		expr->payload.inner_desc = inner_proto;

		if (proto == &proto_th)
			expr->payload.is_raw = true;

		return expr;
	} else if (!json_unpack(root, "{s:s, s:s}",
				"protocol", &protocol, "field", &field)) {
		const struct proto_desc *proto = proto_lookup_byname(protocol);

		if (!proto) {
			json_error(ctx, "Unknown payload protocol '%s'.",
				   protocol);
			return NULL;
		}
		if (json_parse_payload_field(proto, field, &val)) {
			json_error(ctx, "Unknown %s field '%s'.",
				   protocol, field);
			return NULL;
		}
		expr = payload_expr_alloc(int_loc, proto, val);

		if (proto == &proto_th)
			expr->payload.is_raw = true;

		return expr;
	}
	json_error(ctx, "Invalid payload expression properties.");
	return NULL;
}

static struct expr *json_parse_tcp_option_expr(struct json_ctx *ctx,
					       const char *type, json_t *root)
{
	int fieldval, kind, offset, len;
	const char *desc, *field;
	struct expr *expr;

	if (!json_unpack(root, "{s:i, s:i, s:i}",
			 "base", &kind, "offset", &offset, "len", &len)) {
		uint32_t flag = 0;

		if (kind < 0 || kind > 255)
			return NULL;

		if (len < 0 || len > (int)NFT_MAX_EXPR_LEN_BITS) {
			json_error(ctx, "option length must be between 0 and %lu, got %d",
				   NFT_MAX_EXPR_LEN_BITS, len);
			return NULL;
		}

		expr = tcpopt_expr_alloc(int_loc, kind,
					 TCPOPT_COMMON_KIND);

		if (offset == TCPOPT_COMMON_KIND && len == 8)
			flag = NFT_EXTHDR_F_PRESENT;

		tcpopt_init_raw(expr, kind, offset, len, flag);
		return expr;
	} else if (!json_unpack(root, "{s:s}", "name", &desc)) {
		if (json_parse_tcp_option_type(desc, &kind)) {
			json_error(ctx, "Unknown tcp option name '%s'.", desc);
			return NULL;
		}

		if (json_unpack(root, "{s:s}", "field", &field)) {
			expr = tcpopt_expr_alloc(int_loc, kind,
						 TCPOPT_COMMON_KIND);
			expr->exthdr.flags = NFT_EXTHDR_F_PRESENT;
			return expr;
		}

		if (json_parse_tcp_option_field(kind, field, &fieldval)) {
			json_error(ctx, "Unknown tcp option field '%s'.", field);
			return NULL;
		}

		return tcpopt_expr_alloc(int_loc, kind, fieldval);
	}

	json_error(ctx, "Invalid tcp option expression properties.");
	return NULL;
}

static int json_parse_ip_option_type(const char *name, int *val)
{
	unsigned int i;

	for (i = 0; i < array_size(ipopt_protocols); i++) {
		if (ipopt_protocols[i] &&
		    !strcmp(ipopt_protocols[i]->name, name)) {
			if (val)
				*val = i;
			return 0;
		}
	}
	return 1;
}

static int json_parse_ip_option_field(int type, const char *name, int *val)
{
	unsigned int i;
	const struct exthdr_desc *desc = ipopt_protocols[type];

	for (i = 0; i < array_size(desc->templates); i++) {
		if (desc->templates[i].token &&
		    !strcmp(desc->templates[i].token, name)) {
			if (val)
				*val = i;
			return 0;
		}
	}
	return 1;
}

static struct expr *json_parse_ip_option_expr(struct json_ctx *ctx,
					      const char *type, json_t *root)
{
	const char *desc, *field;
	int descval, fieldval;
	struct expr *expr;

	if (json_unpack_err(ctx, root, "{s:s}", "name", &desc))
		return NULL;

	if (json_parse_ip_option_type(desc, &descval)) {
		json_error(ctx, "Unknown ip option name '%s'.", desc);
		return NULL;
	}

	if (json_unpack(root, "{s:s}", "field", &field)) {
		expr = ipopt_expr_alloc(int_loc, descval,
					IPOPT_FIELD_TYPE);
		expr->exthdr.flags = NFT_EXTHDR_F_PRESENT;

		return expr;
	}
	if (json_parse_ip_option_field(descval, field, &fieldval)) {
		json_error(ctx, "Unknown ip option field '%s'.", field);
		return NULL;
	}
	return ipopt_expr_alloc(int_loc, descval, fieldval);
}

static int json_parse_sctp_chunk_field(const struct exthdr_desc *desc,
				       const char *name, int *val)
{
	unsigned int i;

	for (i = 0; i < array_size(desc->templates); i++) {
		if (desc->templates[i].token &&
		    !strcmp(desc->templates[i].token, name)) {
			if (val)
				*val = i;
			return 0;
		}
	}
	return 1;
}

static struct expr *json_parse_sctp_chunk_expr(struct json_ctx *ctx,
					       const char *type, json_t *root)
{
	const struct exthdr_desc *desc;
	const char *name, *field;
	struct expr *expr;
	int fieldval;

	if (json_unpack_err(ctx, root, "{s:s}", "name", &name))
		return NULL;

	desc = sctp_chunk_protocol_find(name);
	if (!desc) {
		json_error(ctx, "Unknown sctp chunk name '%s'.", name);
		return NULL;
	}

	if (json_unpack(root, "{s:s}", "field", &field)) {
		expr = sctp_chunk_expr_alloc(int_loc, desc->type,
					     SCTP_CHUNK_COMMON_TYPE);
		expr->exthdr.flags = NFT_EXTHDR_F_PRESENT;

		return expr;
	}
	if (json_parse_sctp_chunk_field(desc, field, &fieldval)) {
		json_error(ctx, "Unknown sctp chunk field '%s'.", field);
		return NULL;
	}
	return sctp_chunk_expr_alloc(int_loc, desc->type, fieldval);
}

static struct expr *json_parse_dccp_option_expr(struct json_ctx *ctx,
						const char *type, json_t *root)
{
	int opt_type;

	if (json_unpack_err(ctx, root, "{s:i}", "type", &opt_type))
		return NULL;

	if (opt_type < DCCPOPT_TYPE_MIN || opt_type > DCCPOPT_TYPE_MAX) {
		json_error(ctx, "Unknown dccp option type '%d'.", opt_type);
		return NULL;
	}

	return dccpopt_expr_alloc(int_loc, opt_type);
}

static const struct exthdr_desc *exthdr_lookup_byname(const char *name)
{
	const struct exthdr_desc *exthdr_tbl[] = {
		&exthdr_hbh,
		&exthdr_rt,
		&exthdr_rt0,
		&exthdr_rt2,
		&exthdr_rt4,
		&exthdr_frag,
		&exthdr_dst,
		&exthdr_mh,
	};
	unsigned int i;

	for (i = 0; i < array_size(exthdr_tbl); i++) {
		if (!strcmp(exthdr_tbl[i]->name, name))
			return exthdr_tbl[i];
	}
	return NULL;
}

static int json_parse_exthdr_field(const struct exthdr_desc *desc,
				   const char *name, int *field)
{
	unsigned int i;

	for (i = 0; i < array_size(desc->templates); i++) {
		if (desc->templates[i].token &&
		    !strcmp(desc->templates[i].token, name)) {
			if (field)
				*field = i;
			return 0;
		}
	}
	return 1;
}

static struct expr *json_parse_exthdr_expr(struct json_ctx *ctx,
					   const char *type, json_t *root)
{
	const char *name, *field = NULL;
	struct expr *expr;
	int offset = 0, fieldval;
	const struct exthdr_desc *desc;

	if (json_unpack_err(ctx, root, "{s:s}", "name", &name))
		return NULL;

	desc = exthdr_lookup_byname(name);
	if (!desc) {
		json_error(ctx, "Invalid exthdr protocol '%s'.", name);
		return NULL;
	}

	if (json_unpack(root, "{s:s}", "field", &field)) {
		expr = exthdr_expr_alloc(int_loc, desc, 1);
		expr->exthdr.flags = NFT_EXTHDR_F_PRESENT;
		return expr;
	}

	if (json_parse_exthdr_field(desc, field, &fieldval)) {
		json_error(ctx, "Unknown %s field %s.", desc->name, field);
		return NULL;
	}

	/* special treatment for rt0 */
	if (desc == &exthdr_rt0 &&
	    json_unpack_err(ctx, root, "{s:i}", "offset", &offset))
		return NULL;

	return exthdr_expr_alloc(int_loc, desc, fieldval + offset);
}

static struct expr *json_parse_rt_expr(struct json_ctx *ctx,
				       const char *type, json_t *root)
{
	const struct {
		const char *name;
		int val;
	} rt_key_tbl[] = {
		{ "classid", NFT_RT_CLASSID },
		{ "nexthop", NFT_RT_NEXTHOP4 },
		{ "mtu", NFT_RT_TCPMSS },
		{ "ipsec", NFT_RT_XFRM },
	};
	const char *key;
	unsigned int i;
	int familyval;

	if (json_unpack_err(ctx, root, "{s:s}", "key", &key))
		return NULL;
	familyval = json_parse_family(ctx, root);
	if (familyval < 0)
		return NULL;

	for (i = 0; i < array_size(rt_key_tbl); i++) {
		int val = rt_key_tbl[i].val;
		bool invalid = true;

		if (strcmp(key, rt_key_tbl[i].name))
			continue;

		if (familyval) {
			if (familyval == NFPROTO_IPV6 &&
			    val == NFT_RT_NEXTHOP4)
				val = NFT_RT_NEXTHOP6;
			invalid = false;
		}
		return rt_expr_alloc(int_loc, val, invalid);
	}
	json_error(ctx, "Unknown rt key '%s'.", key);
	return NULL;
}

static bool ct_key_is_dir(enum nft_ct_keys key)
{
	const enum nft_ct_keys ct_dir_keys[] = {
		NFT_CT_L3PROTOCOL,
		NFT_CT_SRC,
		NFT_CT_DST,
		NFT_CT_PROTOCOL,
		NFT_CT_PROTO_SRC,
		NFT_CT_PROTO_DST,
		NFT_CT_PKTS,
		NFT_CT_BYTES,
		NFT_CT_AVGPKT,
		NFT_CT_ZONE,
		NFT_CT_SRC_IP,
		NFT_CT_DST_IP,
		NFT_CT_SRC_IP6,
		NFT_CT_DST_IP6,
	};
	unsigned int i;

	for (i = 0; i < array_size(ct_dir_keys); i++) {
		if (key == ct_dir_keys[i])
			return true;
	}
	return false;
}

static struct expr *json_parse_ct_expr(struct json_ctx *ctx,
				       const char *type, json_t *root)
{
	int dirval = -1, keyval = -1;
	const char *key, *dir;
	unsigned int i;

	if (json_unpack_err(ctx, root, "{s:s}", "key", &key))
		return NULL;

	for (i = 0; i < array_size(ct_templates); i++) {
		if (ct_templates[i].token &&
		    !strcmp(key, ct_templates[i].token)) {
			keyval = i;
			break;
		}
	}
	if (keyval == -1) {
		json_error(ctx, "Unknown ct key '%s'.", key);
		return NULL;
	}

	if (!json_unpack(root, "{s:s}", "dir", &dir)) {
		if (!strcmp(dir, "original")) {
			dirval = IP_CT_DIR_ORIGINAL;
		} else if (!strcmp(dir, "reply")) {
			dirval = IP_CT_DIR_REPLY;
		} else {
			json_error(ctx, "Invalid direction '%s'.", dir);
			return NULL;
		}

		if (!ct_key_is_dir(keyval)) {
			json_error(ctx, "Direction not supported by CT key '%s'.", key);
			return NULL;
		}
	}

	return ct_expr_alloc(int_loc, keyval, dirval);
}

static struct expr *json_parse_numgen_expr(struct json_ctx *ctx,
					   const char *type, json_t *root)
{
	int modeval, mod, offset = 0;
	const char *mode;

	if (json_unpack_err(ctx, root, "{s:s, s:i}",
			    "mode", &mode, "mod", &mod))
		return NULL;
	json_unpack(root, "{s:i}", "offset", &offset);

	if (!strcmp(mode, "inc")) {
		modeval = NFT_NG_INCREMENTAL;
	} else if (!strcmp(mode, "random")) {
		modeval = NFT_NG_RANDOM;
	} else {
		json_error(ctx, "Unknown numgen mode '%s'.", mode);
		return NULL;
	}

	return numgen_expr_alloc(int_loc, modeval, mod, offset);
}

static struct expr *json_parse_hash_expr(struct json_ctx *ctx,
					 const char *type, json_t *root)
{
	int mod, offset = 0, seed = 0;
	struct expr *expr, *hash_expr;
	bool have_seed;
	json_t *jexpr;


	if (json_unpack_err(ctx, root, "{s:i}", "mod", &mod))
		return NULL;
	json_unpack(root, "{s:i}", "offset", &offset);

	if (!strcmp(type, "symhash")) {
		return hash_expr_alloc(int_loc, mod, false, 0,
				       offset, NFT_HASH_SYM);
	} else if (strcmp(type, "jhash")) {
		json_error(ctx, "Unknown hash type '%s'.", type);
		return NULL;
	}

	if (json_unpack_err(ctx, root, "{s:o}", "expr", &jexpr))
		return NULL;
	expr = json_parse_expr(ctx, jexpr);
	if (!expr) {
		json_error(ctx, "Invalid jhash expression.");
		return NULL;
	}
	have_seed = !json_unpack(root, "{s:i}", "seed", &seed);

	hash_expr = hash_expr_alloc(int_loc, mod, have_seed,
				    seed, offset, NFT_HASH_JENKINS);
	hash_expr->hash.expr = expr;
	return hash_expr;
}

static unsigned int fib_flag_parse(const char *name)
{
	const char *fib_flags[] = {
		"saddr",
		"daddr",
		"mark",
		"iif",
		"oif",
	};
	unsigned int i;

	for (i = 0; i < array_size(fib_flags); i++) {
		if (!strcmp(name, fib_flags[i]))
			return 1 << i;
	}
	return 0;
}

static struct expr *json_parse_fib_expr(struct json_ctx *ctx,
					const char *type, json_t *root)
{
	const char *fib_result_tbl[] = {
		[NFT_FIB_RESULT_UNSPEC] = NULL,
		[NFT_FIB_RESULT_OIF] = "oif",
		[NFT_FIB_RESULT_OIFNAME] = "oifname",
		[NFT_FIB_RESULT_ADDRTYPE] = "type",
		[__NFT_FIB_RESULT_MAX] = "check",	/* Actually, NFT_FIB_F_PRESENT. */
	};
	enum nft_fib_result resultval = NFT_FIB_RESULT_UNSPEC;
	const char *result;
	int flagval = 0;
	unsigned int i;

	if (json_unpack_err(ctx, root, "{s:s}", "result", &result))
		return NULL;

	for (i = 1; i < array_size(fib_result_tbl); i++) {
		if (!strcmp(result, fib_result_tbl[i])) {
			resultval = i;
			break;
		}
	}
	switch (resultval) {
	case NFT_FIB_RESULT_UNSPEC:
		json_error(ctx, "Invalid fib result '%s'.", result);
		return NULL;
	case __NFT_FIB_RESULT_MAX:
		resultval = NFT_FIB_RESULT_OIF;
		flagval = NFTA_FIB_F_PRESENT;
		break;
	default:
		break;
	}

	flagval |= parse_flags_array(ctx, root, "flags", fib_flag_parse);
	if (flagval < 0)
		return NULL;

	/* sanity checks from fib_expr in parser_bison.y */

	if ((flagval & (NFTA_FIB_F_SADDR|NFTA_FIB_F_DADDR)) == 0) {
		json_error(ctx, "fib: need either saddr or daddr");
		return NULL;
	}

	if ((flagval & (NFTA_FIB_F_SADDR|NFTA_FIB_F_DADDR)) ==
	    (NFTA_FIB_F_SADDR|NFTA_FIB_F_DADDR)) {
		json_error(ctx, "fib: saddr and daddr are mutually exclusive");
		return NULL;
	}

	if ((flagval & (NFTA_FIB_F_IIF|NFTA_FIB_F_OIF)) ==
	    (NFTA_FIB_F_IIF|NFTA_FIB_F_OIF)) {
		json_error(ctx, "fib: iif and oif are mutually exclusive");
		return NULL;
	}

	return fib_expr_alloc(int_loc, flagval, resultval);
}

static struct expr *json_parse_binop_expr(struct json_ctx *ctx,
					  const char *type, json_t *root)
{
	const struct {
		const char *type;
		enum ops op;
	} op_tbl[] = {
		{ "|", OP_OR },
		{ "^", OP_XOR },
		{ "&", OP_AND },
		{ ">>", OP_RSHIFT },
		{ "<<", OP_LSHIFT },
	};
	enum ops thisop = OP_INVALID;
	struct expr *left, *right;
	json_t *jleft, *jright;
	unsigned int i;

	for (i = 0; i < array_size(op_tbl); i++) {
		if (strcmp(type, op_tbl[i].type))
			continue;

		thisop = op_tbl[i].op;
		break;
	}
	if (thisop == OP_INVALID) {
		json_error(ctx, "Invalid binop type '%s'.", type);
		return NULL;
	}

	if (json_array_size(root) > 2) {
		left = json_parse_primary_expr(ctx, json_array_get(root, 0));
		if (!left) {
			json_error(ctx, "Failed to parse LHS of binop expression.");
			return NULL;
		}
		right = json_parse_primary_expr(ctx, json_array_get(root, 1));
		if (!right) {
			json_error(ctx, "Failed to parse RHS of binop expression.");
			expr_free(left);
			return NULL;
		}
		left = binop_expr_alloc(int_loc, thisop, left, right);
		for (i = 2; i < json_array_size(root); i++) {
			jright = json_array_get(root, i);
			right = json_parse_primary_expr(ctx, jright);
			if (!right) {
				json_error(ctx, "Failed to parse RHS of binop expression.");
				expr_free(left);
				return NULL;
			}
			left = binop_expr_alloc(int_loc, thisop, left, right);
		}
		return left;
	}

	if (json_unpack_err(ctx, root, "[o, o!]", &jleft, &jright))
		return NULL;

	left = json_parse_primary_expr(ctx, jleft);
	if (!left) {
		json_error(ctx, "Failed to parse LHS of binop expression.");
		return NULL;
	}
	right = json_parse_rhs_expr(ctx, jright);
	if (!right) {
		json_error(ctx, "Failed to parse RHS of binop expression.");
		expr_free(left);
		return NULL;
	}
	return binop_expr_alloc(int_loc, thisop, left, right);
}

static struct expr *json_check_concat_expr(struct json_ctx *ctx, struct expr *e)
{
	if (e->etype != EXPR_CONCAT) {
		json_error(ctx, "Expected concatenation, got %s", expr_name(e));
		goto err_free;
	}

	if (expr_concat(e)->size >= 2)
		return e;

	json_error(ctx, "Concatenation with %d elements is illegal",
		   expr_concat(e)->size);

err_free:
	expr_free(e);
	return NULL;
}

static struct expr *json_parse_concat_expr(struct json_ctx *ctx,
					   const char *type, json_t *root)
{
	struct expr *expr = NULL, *tmp;
	json_t *value;
	size_t index;

	if (!json_is_array(root)) {
		json_error(ctx, "Unexpected concat object type %s.",
			   json_typename(root));
		return NULL;
	}

	json_array_foreach(root, index, value) {
		tmp = json_parse_concat_elem_expr(ctx, value);
		if (!tmp) {
			json_error(ctx, "Parsing expr at index %zd failed.", index);
			expr_free(expr);
			return NULL;
		}
		if (!expr) {
			expr = tmp;
			continue;
		}
		if (expr->etype != EXPR_CONCAT) {
			struct expr *concat;

			concat = concat_expr_alloc(int_loc);
			concat_expr_add(concat, expr);
			expr = concat;
		}
		concat_expr_add(expr, tmp);
	}
	return expr ? json_check_concat_expr(ctx, expr) : NULL;
}

static struct expr *json_parse_prefix_expr(struct json_ctx *ctx,
					   const char *type, json_t *root)
{
	struct expr *expr;
	json_t *addr;
	int len;

	if (json_unpack_err(ctx, root, "{s:o, s:i}",
			    "addr", &addr, "len", &len))
		return NULL;

	expr = json_parse_primary_expr(ctx, addr);
	if (!expr) {
		json_error(ctx, "Invalid address in prefix expr.");
		return NULL;
	}
	return prefix_expr_alloc(int_loc, expr, len);
}

static struct expr *json_parse_range_expr(struct json_ctx *ctx,
					  const char *type, json_t *root)
{
	struct expr *expr_low, *expr_high, *tmp;
	json_t *low, *high;

	if (json_unpack_err(ctx, root, "[o, o!]", &low, &high))
		return NULL;

	expr_low = json_parse_primary_expr(ctx, low);
	if (!expr_low) {
		json_error(ctx, "Invalid low value in range expression.");
		return NULL;
	}
	expr_high = json_parse_primary_expr(ctx, high);
	if (!expr_high) {
		json_error(ctx, "Invalid high value in range expression.");
		expr_free(expr_low);
		return NULL;
	}
	if (is_symbol_value_expr(expr_low) && is_symbol_value_expr(expr_high)) {
		tmp = symbol_range_expr_alloc(int_loc,
					      SYMBOL_VALUE,
					      expr_low->scope,
					      expr_low->identifier,
					      expr_high->identifier);
		expr_free(expr_low);
		expr_free(expr_high);
		return tmp;
	}
	return range_expr_alloc(int_loc, expr_low, expr_high);
}

static struct expr *json_alloc_chain_expr(const char *chain)
{
	if (!chain)
		return NULL;

	return constant_expr_alloc(int_loc, &string_type, BYTEORDER_HOST_ENDIAN,
				   strlen(chain) * BITS_PER_BYTE, chain);
}

static struct expr *json_parse_verdict_expr(struct json_ctx *ctx,
					    const char *type, json_t *root)
{
	const struct {
		int verdict;
		const char *name;
		bool need_chain;
	} verdict_tbl[] = {
		{ NFT_CONTINUE, "continue", false },
		{ NFT_JUMP, "jump", true },
		{ NFT_GOTO, "goto", true },
		{ NFT_RETURN, "return", false },
		{ NF_ACCEPT, "accept", false },
		{ NF_DROP, "drop", false },
	};
	const char *chain = NULL;
	unsigned int i;

	for (i = 0; i < array_size(verdict_tbl); i++) {
		if (strcmp(type, verdict_tbl[i].name))
			continue;

		if (verdict_tbl[i].need_chain) {
			if (json_unpack_err(ctx, root, "{s:s}", "target", &chain))
				return NULL;

			if (!chain || chain[0] == '\0')
				return NULL;
		}

		return verdict_expr_alloc(int_loc, verdict_tbl[i].verdict,
					  json_alloc_chain_expr(chain));
	}
	json_error(ctx, "Unknown verdict '%s'.", type);
	return NULL;
}

static struct expr *json_parse_set_expr(struct json_ctx *ctx,
					const char *type, json_t *root)
{
	struct expr *expr, *set_expr = NULL;
	json_t *value;
	size_t index;

	if (!json_is_array(root)) {
		expr = json_parse_immediate(ctx, root);
		if (!expr)
			return NULL;

		if (expr->etype == EXPR_SYMBOL &&
		    expr->symtype == SYMBOL_SET)
			return expr;

		expr = set_elem_expr_alloc(int_loc, expr);
		set_expr = set_expr_alloc(int_loc, NULL);
		set_expr_add(set_expr, expr);
		return set_expr;
	}

	json_array_foreach(root, index, value) {
		struct expr *expr;
		json_t *jleft, *jright;

		if (!json_unpack(value, "[o, o!]", &jleft, &jright)) {
			struct expr *expr2;

			expr = json_parse_rhs_expr(ctx, jleft);
			if (!expr) {
				json_error(ctx, "Invalid set elem at index %zu.", index);
				expr_free(set_expr);
				return NULL;
			}
			if (expr->etype != EXPR_SET_ELEM)
				expr = set_elem_expr_alloc(int_loc, expr);

			expr2 = json_parse_set_rhs_expr(ctx, jright);
			if (!expr2) {
				json_error(ctx, "Invalid set elem at index %zu.", index);
				expr_free(expr);
				expr_free(set_expr);
				return NULL;
			}
			expr2 = mapping_expr_alloc(int_loc, expr, expr2);
			expr = expr2;
		} else {
			expr = json_parse_rhs_expr(ctx, value);

			if (!expr) {
				json_error(ctx, "Invalid set elem at index %zu.", index);
				expr_free(set_expr);
				return NULL;
			}

			if (expr->etype != EXPR_SET_ELEM)
				expr = set_elem_expr_alloc(int_loc, expr);
		}

		if (!set_expr)
			set_expr = set_expr_alloc(int_loc, NULL);
		set_expr_add(set_expr, expr);
	}
	return set_expr;
}

static struct expr *json_parse_map_expr(struct json_ctx *ctx,
					const char *type, json_t *root)
{
	json_t *jkey, *jdata;
	struct expr *key, *data;

	if (json_unpack_err(ctx, root, "{s:o, s:o}",
			    "key", &jkey, "data", &jdata))
		return NULL;

	key = json_parse_map_lhs_expr(ctx, jkey);
	if (!key) {
		json_error(ctx, "Illegal map expression key.");
		return NULL;
	}

	data = json_parse_rhs_expr(ctx, jdata);
	if (!data) {
		json_error(ctx, "Illegal map expression data.");
		expr_free(key);
		return NULL;
	}

	return map_expr_alloc(int_loc, key, data);
}

static struct expr *json_parse_set_elem_expr(struct json_ctx *ctx,
					     const char *type, json_t *root)
{
	struct expr *expr;
	json_t *tmp;
	uint64_t i;

	if (json_unpack_err(ctx, root, "{s:o}", "val", &tmp))
		return NULL;

	expr = json_parse_expr(ctx, tmp);
	if (!expr)
		return NULL;

	expr = set_elem_expr_alloc(int_loc, expr);

	if (!json_unpack(root, "{s:I}", "timeout", &i))
		expr->timeout = i * 1000;
	if (!json_unpack(root, "{s:I}", "expires", &i))
		expr->expiration = i * 1000;
	if (!json_unpack(root, "{s:s}", "comment", &expr->comment))
		expr->comment = xstrdup(expr->comment);

	return expr;
}

static struct expr *json_parse_xfrm_expr(struct json_ctx *ctx,
					 const char *type, json_t *root)
{
	const char *key, *dir;
	unsigned int i, spnum;
	int dirval = -1, familyval, keyval = -1;

	if (json_unpack_err(ctx, root, "{s:s}", "key", &key))
		return NULL;

	for (i = 1; i < array_size(xfrm_templates); i++) {
		if (strcmp(key, xfrm_templates[i].token))
			continue;
		keyval = i;
		break;
	}

	if (keyval == -1) {
		json_error(ctx, "Unknown xfrm key '%s'.", key);
		return NULL;
	}

	familyval = json_parse_family(ctx, root);
	if (familyval < 0)
		return NULL;

	if (!json_unpack(root, "{s:s}", "dir", &dir)) {
		if (!strcmp(dir, "in")) {
			dirval = XFRM_POLICY_IN;
		} else if (!strcmp(dir, "out")) {
			dirval = XFRM_POLICY_OUT;
		} else {
			json_error(ctx, "Invalid direction '%s'.", dir);
			return NULL;
		}
	}

	spnum = 0;
	if (!json_unpack(root, "{s:i}", "spnum", &spnum)) {
		if (spnum > 255) {
			json_error(ctx, "Invalid spnum'%d'.", spnum);
			return NULL;
		}
	}

	switch (keyval) {
	case NFT_XFRM_KEY_SADDR_IP4:
		if (familyval == NFPROTO_IPV6)
			keyval = NFT_XFRM_KEY_SADDR_IP6;
		break;
	case NFT_XFRM_KEY_DADDR_IP4:
		if (familyval == NFPROTO_IPV6)
			keyval = NFT_XFRM_KEY_DADDR_IP6;
		break;
	default:
		break;
	}

	return xfrm_expr_alloc(int_loc, dirval, spnum, keyval);
}

static struct expr *json_parse_expr(struct json_ctx *ctx, json_t *root)
{
	const struct {
		const char *name;
		struct expr *(*cb)(struct json_ctx *, const char *, json_t *);
		uint32_t flags;
	} cb_tbl[] = {
		{ "concat", json_parse_concat_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_DTYPE | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP },
		{ "set", json_parse_set_expr, CTX_F_RHS | CTX_F_STMT }, /* allow this as stmt expr because that allows set references */
		{ "map", json_parse_map_expr, CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS },
		/* below three are multiton_rhs_expr */
		{ "prefix", json_parse_prefix_expr, CTX_F_RHS | CTX_F_SET_RHS | CTX_F_STMT | CTX_F_CONCAT },
		{ "range", json_parse_range_expr, CTX_F_RHS | CTX_F_SET_RHS | CTX_F_STMT | CTX_F_CONCAT },
		{ "payload", json_parse_payload_expr, CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_MANGLE | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "exthdr", json_parse_exthdr_expr, CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "tcp option", json_parse_tcp_option_expr, CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_MANGLE | CTX_F_SES | CTX_F_CONCAT },
		{ "ip option", json_parse_ip_option_expr, CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_MANGLE | CTX_F_SES | CTX_F_CONCAT },
		{ "sctp chunk", json_parse_sctp_chunk_expr, CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_MANGLE | CTX_F_SES | CTX_F_CONCAT },
		{ "dccp option", json_parse_dccp_option_expr, CTX_F_PRIMARY },
		{ "meta", json_parse_meta_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_MANGLE | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "osf", json_parse_osf_expr, CTX_F_STMT | CTX_F_PRIMARY | CTX_F_MAP | CTX_F_CONCAT },
		{ "ipsec", json_parse_xfrm_expr, CTX_F_PRIMARY | CTX_F_MAP | CTX_F_CONCAT },
		{ "socket", json_parse_socket_expr, CTX_F_PRIMARY | CTX_F_CONCAT },
		{ "rt", json_parse_rt_expr, CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "ct", json_parse_ct_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_MANGLE | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "numgen", json_parse_numgen_expr, CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "tunnel", json_parse_tunnel_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SES | CTX_F_MAP },
		/* below two are hash expr */
		{ "jhash", json_parse_hash_expr, CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "symhash", json_parse_hash_expr, CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "fib", json_parse_fib_expr, CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "|", json_parse_binop_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "^", json_parse_binop_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "&", json_parse_binop_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ ">>", json_parse_binop_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "<<", json_parse_binop_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SET_RHS | CTX_F_SES | CTX_F_MAP | CTX_F_CONCAT },
		{ "accept", json_parse_verdict_expr, CTX_F_RHS | CTX_F_SET_RHS },
		{ "drop", json_parse_verdict_expr, CTX_F_RHS | CTX_F_SET_RHS },
		{ "continue", json_parse_verdict_expr, CTX_F_RHS | CTX_F_SET_RHS },
		{ "jump", json_parse_verdict_expr, CTX_F_RHS | CTX_F_SET_RHS },
		{ "goto", json_parse_verdict_expr, CTX_F_RHS | CTX_F_SET_RHS },
		{ "return", json_parse_verdict_expr, CTX_F_RHS | CTX_F_SET_RHS },
		{ "elem", json_parse_set_elem_expr, CTX_F_RHS | CTX_F_STMT | CTX_F_PRIMARY | CTX_F_SES },
	};
	struct expr *list;
	const char *type;
	unsigned int i;
	json_t *value;
	size_t index;


	switch (json_typeof(root)) {
	case JSON_ARRAY:
		if (!(ctx->flags & (CTX_F_RHS | CTX_F_STMT))) {
			json_error(ctx, "List expression only allowed on RHS or in statement expression.");
			return NULL;
		}

		if (is_PRIMARY(ctx)) {
			json_error(ctx, "List expression not allowed as primary expression.");
			return NULL;
		}

		list = list_expr_alloc(int_loc);
		json_array_foreach(root, index, value) {
			struct expr *expr = json_parse_expr(ctx, value);
			if (!expr) {
				json_error(ctx, "Parsing list expression item at index %zu failed.", index);
				expr_free(list);
				return NULL;
			}
			list_expr_add(list, expr);
		}
		return list;
	case JSON_TRUE:
	case JSON_FALSE:
		if (!is_RHS(ctx) && !is_PRIMARY(ctx)) {
			json_error(ctx, "Boolean values not allowed in this context.");
			return NULL;
		}
		/* fall through */
	case JSON_STRING:
	case JSON_INTEGER:
		return json_parse_immediate(ctx, root);
	default:
		break;
	}

	if (json_unpack_stmt(ctx, root, &type, &value))
		return NULL;

	for (i = 0; i < array_size(cb_tbl); i++) {
		if (strcmp(type, cb_tbl[i].name))
			continue;

		if ((cb_tbl[i].flags & ctx->flags) != ctx->flags) {
			json_error(ctx, "Expression type %s not allowed in context (%s).",
				   type, ctx_flags_to_string(ctx));
			return NULL;
		}

		return cb_tbl[i].cb(ctx, type, value);
	}
	json_error(ctx, "Unknown expression type '%s'.", type);
	return NULL;
}

static struct expr *json_parse_flagged_expr(struct json_ctx *ctx,
					    uint32_t flags, json_t *root)
{
	uint32_t old_flags = ctx->flags;
	struct expr *expr;

	ctx->flags |= flags;
	expr = json_parse_expr(ctx, root);
	ctx->flags = old_flags;

	return expr;
}

static struct expr *json_parse_rhs_expr(struct json_ctx *ctx, json_t *root)
{
	return json_parse_flagged_expr(ctx, CTX_F_RHS, root);
}

static struct expr *json_parse_stmt_expr(struct json_ctx *ctx, json_t *root)
{
	return json_parse_flagged_expr(ctx, CTX_F_STMT, root);
}

static struct expr *json_parse_primary_expr(struct json_ctx *ctx, json_t *root)
{
	return json_parse_flagged_expr(ctx, CTX_F_PRIMARY, root);
}

static struct expr *json_parse_set_rhs_expr(struct json_ctx *ctx, json_t *root)
{
	return json_parse_flagged_expr(ctx, CTX_F_SET_RHS, root);
}

static struct expr *json_parse_mangle_lhs_expr(struct json_ctx *ctx, json_t *root)
{
	return json_parse_flagged_expr(ctx, CTX_F_MANGLE, root);
}

static struct expr *json_parse_set_elem_expr_stmt(struct json_ctx *ctx, json_t *root)
{
	struct expr *expr = json_parse_flagged_expr(ctx, CTX_F_SES, root);

	if (expr && expr->etype != EXPR_SET_ELEM)
		expr = set_elem_expr_alloc(int_loc, expr);

	return expr;
}

static struct expr *json_parse_map_lhs_expr(struct json_ctx *ctx, json_t *root)
{
	return json_parse_flagged_expr(ctx, CTX_F_MAP, root);
}

static struct expr *json_parse_concat_elem_expr(struct json_ctx *ctx, json_t *root)
{
	return json_parse_flagged_expr(ctx, CTX_F_CONCAT, root);
}

static struct expr *json_parse_dtype_expr(struct json_ctx *ctx, json_t *root)
{
	if (json_is_string(root)) {
		const struct datatype *dtype;

		dtype = datatype_lookup_byname(json_string_value(root));
		if (!dtype) {
			json_error(ctx, "Invalid datatype '%s'.",
				   json_string_value(root));
			return NULL;
		}
		return constant_expr_alloc(int_loc, dtype,
					   dtype->byteorder, dtype->size, NULL);
	} else if (json_is_array(root)) {
		json_t *value;
		size_t index;
		struct expr *expr = concat_expr_alloc(int_loc);

		json_array_foreach(root, index, value) {
			struct expr *i = json_parse_dtype_expr(ctx, value);

			if (!i) {
				json_error(ctx, "Invalid datatype at index %zu.", index);
				expr_free(expr);
				return NULL;
			}
			concat_expr_add(expr, i);
		}

		return json_check_concat_expr(ctx, expr);
	} else if (json_is_object(root)) {
		const char *key;
		json_t *val;

		if (!json_unpack_stmt(ctx, root, &key, &val) &&
		    !strcmp(key, "typeof")) {
			return json_parse_expr(ctx, val);
		}
	}

	json_error(ctx, "Invalid set datatype.");
	return NULL;
}

static struct stmt *json_parse_match_stmt(struct json_ctx *ctx,
					  const char *key, json_t *value)
{
	struct expr *left, *right, *rel_expr;
	json_t *jleft, *jright;
	const char *opstr = NULL;
	enum ops op;

	if (json_unpack_err(ctx, value, "{s:o, s:o, s:s}",
			    "left", &jleft,
			    "right", &jright,
			    "op", &opstr))
		return NULL;

	for (op = OP_INVALID; op < __OP_MAX; op++) {
		if (expr_op_symbols[op] &&
		    !strcmp(opstr, expr_op_symbols[op]))
			break;
	}
	switch (op) {
	case OP_EQ ... OP_NEG:
		break;
	case __OP_MAX:
		if (!strcmp(opstr, "in")) {
			op = OP_IMPLICIT;
			break;
		}
		/* fall through */
	default:
		json_error(ctx, "Invalid relational op '%s'.", opstr);
		return NULL;
	}

	left = json_parse_expr(ctx, jleft);
	if (!left) {
		json_error(ctx, "Invalid LHS of relational.");
		return NULL;
	}
	right = json_parse_rhs_expr(ctx, jright);
	if (!right) {
		expr_free(left);
		json_error(ctx, "Invalid RHS of relational.");
		return NULL;
	}

	rel_expr = relational_expr_alloc(int_loc, op, left, right);
	return expr_stmt_alloc(int_loc, rel_expr);
}

static struct stmt *json_parse_counter_stmt(struct json_ctx *ctx,
					    const char *key, json_t *value)
{
	uint64_t packets, bytes;
	struct stmt *stmt;

	if (json_is_null(value))
		return counter_stmt_alloc(int_loc);

	if (!json_unpack(value, "{s:I, s:I}",
			 "packets", &packets,
			 "bytes", &bytes)) {
		stmt = counter_stmt_alloc(int_loc);
		stmt->counter.packets = packets;
		stmt->counter.bytes = bytes;
		return stmt;
	}

	stmt = objref_stmt_alloc(int_loc);
	stmt->objref.type = NFT_OBJECT_COUNTER;
	stmt->objref.expr = json_parse_stmt_expr(ctx, value);
	if (!stmt->objref.expr) {
		json_error(ctx, "Invalid counter reference.");
		stmt_free(stmt);
		return NULL;
	}
	return stmt;
}

static struct stmt *json_parse_last_stmt(struct json_ctx *ctx,
					 const char *key, json_t *value)
{
	struct stmt *stmt;
	int64_t used;

	if (json_is_null(value))
		return last_stmt_alloc(int_loc);

	if (!json_unpack(value, "{s:I}", "used", &used)) {
		stmt = last_stmt_alloc(int_loc);
		if (used != -1) {
			stmt->last.used = used;
			stmt->last.set = 1;
		}
		return stmt;
	}

	return NULL;
}

static struct stmt *json_parse_verdict_stmt(struct json_ctx *ctx,
					    const char *key, json_t *value)
{
	struct expr *expr;

	expr = json_parse_verdict_expr(ctx, key, value);
	if (expr)
		return verdict_stmt_alloc(int_loc, expr);

	return NULL;
}

static struct stmt *json_parse_mangle_stmt(struct json_ctx *ctx,
					   const char *type, json_t *root)
{
	json_t *jkey, *jvalue;
	struct expr *key, *value;
	struct stmt *stmt;

	if (json_unpack_err(ctx, root, "{s:o, s:o}",
			    "key", &jkey, "value", &jvalue))
		return NULL;

	key = json_parse_mangle_lhs_expr(ctx, jkey);
	if (!key) {
		json_error(ctx, "Invalid mangle statement key");
		return NULL;
	}
	value = json_parse_stmt_expr(ctx, jvalue);
	if (!value) {
		json_error(ctx, "Invalid mangle statement value");
		expr_free(key);
		return NULL;
	}

	switch (key->etype) {
	case EXPR_EXTHDR:
		return exthdr_stmt_alloc(int_loc, key, value);
	case EXPR_PAYLOAD:
		return payload_stmt_alloc(int_loc, key, value);
	case EXPR_META:
		stmt = meta_stmt_alloc(int_loc, key->meta.key, value);
		expr_free(key);
		return stmt;
	case EXPR_CT:
		if (key->ct.key == NFT_CT_HELPER) {
			stmt = objref_stmt_alloc(int_loc);
			stmt->objref.type = NFT_OBJECT_CT_HELPER;
			stmt->objref.expr = value;
		} else {
			stmt = ct_stmt_alloc(int_loc, key->ct.key,
					     key->ct.direction, value);
		}
		expr_free(key);
		return stmt;
	default:
		json_error(ctx, "Invalid mangle statement key expression type.");
		expr_free(key);
		expr_free(value);
		return NULL;
	}
}

static uint64_t rate_to_bytes(uint64_t val, const char *unit)
{
	uint64_t bytes = val;

	if (!strcmp(unit, "kbytes"))
		return bytes * 1024;
	if (!strcmp(unit, "mbytes"))
		return bytes * 1024 * 1024;
	return bytes;
}

static struct stmt *json_parse_quota_stmt(struct json_ctx *ctx,
					  const char *key, json_t *value)
{
	struct stmt *stmt;
	int inv = 0;
	const char *val_unit = "bytes", *used_unit = "bytes";
	uint64_t val, used = 0;

	if (!json_unpack(value, "{s:I}", "val", &val)) {
		json_unpack(value, "{s:b}", "inv", &inv);
		json_unpack(value, "{s:s}", "val_unit", &val_unit);
		json_unpack(value, "{s:I}", "used", &used);
		json_unpack(value, "{s:s}", "used_unit", &used_unit);
		stmt = quota_stmt_alloc(int_loc);
		stmt->quota.bytes = rate_to_bytes(val, val_unit);
		if (used)
			stmt->quota.used = rate_to_bytes(used, used_unit);
		stmt->quota.flags = (inv ? NFT_QUOTA_F_INV : 0);
		return stmt;
	}
	stmt = objref_stmt_alloc(int_loc);
	stmt->objref.type = NFT_OBJECT_QUOTA;
	stmt->objref.expr = json_parse_stmt_expr(ctx, value);
	if (!stmt->objref.expr) {
		json_error(ctx, "Invalid quota reference.");
		stmt_free(stmt);
		return NULL;
	}
	return stmt;
}

static uint64_t seconds_from_unit(const char *unit)
{
	if (!strcmp(unit, "week"))
		return 60 * 60 * 24 * 7;
	if (!strcmp(unit, "day"))
		return 60 * 60 * 24;
	if (!strcmp(unit, "hour"))
		return 60 * 60;
	if (!strcmp(unit, "minute"))
		return 60;
	return 1;
}

static struct stmt *json_parse_limit_stmt(struct json_ctx *ctx,
					  const char *key, json_t *value)
{
	struct stmt *stmt;
	uint64_t rate, burst = 0;
	const char *rate_unit = "packets", *time, *burst_unit = "bytes";
	int inv = 0;

	if (!json_unpack(value, "{s:I, s:s}",
			 "rate", &rate, "per", &time)) {
		json_unpack(value, "{s:s}", "rate_unit", &rate_unit);
		json_unpack(value, "{s:b}", "inv", &inv);
		json_unpack(value, "{s:I}", "burst", &burst);
		json_unpack(value, "{s:s}", "burst_unit", &burst_unit);

		stmt = limit_stmt_alloc(int_loc);

		if (!strcmp(rate_unit, "packets")) {
			if (burst == 0)
				burst = 5;

			stmt->limit.type = NFT_LIMIT_PKTS;
			stmt->limit.rate = rate;
			stmt->limit.burst = burst;
		} else {
			stmt->limit.type = NFT_LIMIT_PKT_BYTES;
			stmt->limit.rate = rate_to_bytes(rate, rate_unit);
			stmt->limit.burst = rate_to_bytes(burst, burst_unit);
		}
		stmt->limit.unit = seconds_from_unit(time);
		stmt->limit.flags = inv ? NFT_LIMIT_F_INV : 0;
		return stmt;
	}

	stmt = objref_stmt_alloc(int_loc);
	stmt->objref.type = NFT_OBJECT_LIMIT;
	stmt->objref.expr = json_parse_stmt_expr(ctx, value);
	if (!stmt->objref.expr) {
		json_error(ctx, "Invalid limit reference.");
		stmt_free(stmt);
		return NULL;
	}
	return stmt;
}

static struct stmt *json_parse_fwd_stmt(struct json_ctx *ctx,
					const char *key, json_t *value)
{
	json_t *jaddr, *jdev;
	struct stmt *stmt;
	int familyval;

	if (json_unpack_err(ctx, value, "{s:o}", "dev", &jdev))
		return NULL;

	stmt = fwd_stmt_alloc(int_loc);

	stmt->fwd.dev = json_parse_stmt_expr(ctx, jdev);
	if (!stmt->fwd.dev) {
		json_error(ctx, "Invalid fwd dev value.");
		goto out_err;
	}

	familyval = json_parse_family(ctx, value);
	if (familyval < 0)
		goto out_err;

	if (familyval == NFPROTO_UNSPEC ||
	    json_unpack(value, "{s:o}", "addr", &jaddr))
		return stmt;

	stmt->fwd.family = familyval;
	stmt->fwd.addr = json_parse_stmt_expr(ctx, jaddr);
	if (!stmt->fwd.addr) {
		json_error(ctx, "Invalid fwd addr value.");
		goto out_err;
	}

	return stmt;
out_err:
	stmt_free(stmt);
	return NULL;
}

static struct stmt *json_parse_flow_offload_stmt(struct json_ctx *ctx,
						 const char *key, json_t *value)
{
	const char *opstr, *flowtable;

	if (json_unpack_err(ctx, value, "{s:s, s:s}",
			    "op", &opstr, "flowtable", &flowtable))
		return NULL;

	if (strcmp(opstr, "add")) {
		json_error(ctx, "Unknown flow offload statement op '%s'.", opstr);
		return NULL;
	}

	if (flowtable[0] != '@') {
		json_error(ctx, "Illegal flowtable reference in flow offload statement.");
		return NULL;
	}

	return flow_offload_stmt_alloc(int_loc, xstrdup(flowtable + 1));
}

static struct stmt *json_parse_notrack_stmt(struct json_ctx *ctx,
					    const char *key, json_t *value)
{
	return notrack_stmt_alloc(int_loc);
}

static struct stmt *json_parse_dup_stmt(struct json_ctx *ctx,
					const char *key, json_t *value)
{
	struct stmt *stmt;
	struct expr *expr;
	json_t *tmp;

	if (json_unpack_err(ctx, value, "{s:o}", "addr", &tmp))
		return NULL;

	expr = json_parse_stmt_expr(ctx, tmp);
	if (!expr) {
		json_error(ctx, "Illegal dup addr arg.");
		return NULL;
	}

	stmt = dup_stmt_alloc(int_loc);
	stmt->dup.to = expr;

	if (json_unpack(value, "{s:o}", "dev", &tmp))
		return stmt;

	expr = json_parse_stmt_expr(ctx, tmp);
	if (!expr) {
		json_error(ctx, "Illegal dup dev.");
		stmt_free(stmt);
		return NULL;
	}
	stmt->dup.dev = expr;
	return stmt;
}

static struct stmt *json_parse_secmark_stmt(struct json_ctx *ctx,
					    const char *key, json_t *value)
{
	struct stmt *stmt;

	stmt = objref_stmt_alloc(int_loc);
	stmt->objref.type = NFT_OBJECT_SECMARK;
	stmt->objref.expr = json_parse_stmt_expr(ctx, value);
	if (!stmt->objref.expr) {
		json_error(ctx, "Invalid secmark reference.");
		stmt_free(stmt);
		return NULL;
	}

	return stmt;
}

static struct stmt *json_parse_tunnel_stmt(struct json_ctx *ctx,
					   const char *key, json_t *value)
{
	struct stmt *stmt;

	stmt = objref_stmt_alloc(int_loc);
	stmt->objref.type = NFT_OBJECT_TUNNEL;
	stmt->objref.expr = json_parse_stmt_expr(ctx, value);
	if (!stmt->objref.expr) {
		json_error(ctx, "Invalid tunnel reference.");
		stmt_free(stmt);
		return NULL;
	}

	return stmt;
}

static unsigned int json_parse_nat_flag(const char *flag)
{
	const struct {
		const char *flag;
		int val;
	} flag_tbl[] = {
		{ "random", NF_NAT_RANGE_PROTO_RANDOM },
		{ "fully-random", NF_NAT_RANGE_PROTO_RANDOM_FULLY },
		{ "persistent", NF_NAT_RANGE_PERSISTENT },
		{ "netmap", NF_NAT_RANGE_NETMAP },
	};
	unsigned int i;

	for (i = 0; i < array_size(flag_tbl); i++) {
		if (!strcmp(flag, flag_tbl[i].flag))
			return flag_tbl[i].val;
	}
	return 0;
}

static unsigned int json_parse_nat_type_flag(const char *flag)
{
	const struct {
		const char *flag;
		int val;
	} flag_tbl[] = {
		{ "interval", STMT_NAT_F_INTERVAL },
		{ "prefix", STMT_NAT_F_PREFIX },
		{ "concat", STMT_NAT_F_CONCAT },
	};
	unsigned int i;

	for (i = 0; i < array_size(flag_tbl); i++) {
		if (!strcmp(flag, flag_tbl[i].flag))
			return flag_tbl[i].val;
	}
	return 0;
}

static int nat_type_parse(const char *type)
{
	const char * const nat_etypes[] = {
		[NFT_NAT_SNAT]	= "snat",
		[NFT_NAT_DNAT]	= "dnat",
		[NFT_NAT_MASQ]	= "masquerade",
		[NFT_NAT_REDIR]	= "redirect",
	};
	size_t i;

	for (i = 0; i < array_size(nat_etypes); i++) {
		if (!strcmp(type, nat_etypes[i]))
			return i;
	}
	return -1;
}

static struct stmt *json_parse_nat_stmt(struct json_ctx *ctx,
					const char *key, json_t *value)
{
	int type, familyval, flags;
	struct stmt *stmt;
	json_t *tmp;

	type = nat_type_parse(key);
	if (type < 0) {
		json_error(ctx, "Unknown nat type '%s'.", key);
		return NULL;
	}

	familyval = json_parse_family(ctx, value);
	if (familyval < 0)
		return NULL;

	stmt = nat_stmt_alloc(int_loc, type);
	stmt->nat.family = familyval;

	if (!json_unpack(value, "{s:o}", "addr", &tmp)) {
		stmt->nat.addr = json_parse_stmt_expr(ctx, tmp);
		if (!stmt->nat.addr) {
			json_error(ctx, "Invalid nat addr.");
			stmt_free(stmt);
			return NULL;
		}
	}
	if (!json_unpack(value, "{s:o}", "port", &tmp)) {
		stmt->nat.proto = json_parse_stmt_expr(ctx, tmp);
		if (!stmt->nat.proto) {
			json_error(ctx, "Invalid nat port.");
			stmt_free(stmt);
			return NULL;
		}
	}
	flags = parse_flags_array(ctx, value, "flags", json_parse_nat_flag);
	if (flags < 0) {
		stmt_free(stmt);
		return NULL;
	}
	stmt->nat.flags = flags;

	flags = parse_flags_array(ctx, value, "type_flags",
				  json_parse_nat_type_flag);
	if (flags < 0) {
		stmt_free(stmt);
		return NULL;
	}
	stmt->nat.type_flags = flags;

	return stmt;
}

static struct stmt *json_parse_tproxy_stmt(struct json_ctx *ctx,
					   const char *key, json_t *value)
{
	json_t *jaddr, *tmp;
	struct stmt *stmt;
	int familyval;

	stmt = tproxy_stmt_alloc(int_loc);

	familyval = json_parse_family(ctx, value);
	if (familyval < 0)
		goto out_free;

	stmt->tproxy.family = familyval;

	if (!json_unpack(value, "{s:o}", "addr", &jaddr)) {
		stmt->tproxy.addr = json_parse_stmt_expr(ctx, jaddr);
		if (!stmt->tproxy.addr) {
			json_error(ctx, "Invalid addr.");
			goto out_free;
		}
	}
	if (!json_unpack(value, "{s:o}", "port", &tmp)) {
		stmt->tproxy.port = json_parse_stmt_expr(ctx, tmp);
		if (!stmt->tproxy.port) {
			json_error(ctx, "Invalid port.");
			goto out_free;
		}
	}
	return stmt;

out_free:
	stmt_free(stmt);
	return NULL;
}

static struct stmt *json_parse_reject_stmt(struct json_ctx *ctx,
					   const char *key, json_t *value)
{
	struct stmt *stmt = reject_stmt_alloc(int_loc);
	const struct datatype *dtype = NULL;
	const char *type;
	json_t *tmp;

	stmt->reject.type = -1;
	stmt->reject.icmp_code = -1;

	if (!json_unpack(value, "{s:s}", "type", &type)) {
		if (!strcmp(type, "tcp reset")) {
			stmt->reject.type = NFT_REJECT_TCP_RST;
			stmt->reject.icmp_code = 0;
		} else if (!strcmp(type, "icmpx")) {
			stmt->reject.type = NFT_REJECT_ICMPX_UNREACH;
			dtype = &reject_icmpx_code_type;
			stmt->reject.icmp_code = 0;
		} else if (!strcmp(type, "icmp")) {
			stmt->reject.type = NFT_REJECT_ICMP_UNREACH;
			stmt->reject.family = NFPROTO_IPV4;
			dtype = &reject_icmp_code_type;
			stmt->reject.icmp_code = 0;
		} else if (!strcmp(type, "icmpv6")) {
			stmt->reject.type = NFT_REJECT_ICMP_UNREACH;
			stmt->reject.family = NFPROTO_IPV6;
			dtype = &reject_icmpv6_code_type;
			stmt->reject.icmp_code = 0;
		}
	}
	if (!json_unpack(value, "{s:o}", "expr", &tmp)) {
		stmt->reject.expr = json_parse_immediate(ctx, tmp);
		if (!stmt->reject.expr) {
			json_error(ctx, "Illegal reject expr.");
			stmt_free(stmt);
			return NULL;
		}
		datatype_set(stmt->reject.expr, dtype);
	}
	return stmt;
}

static int json_parse_set_stmt_list(struct json_ctx *ctx,
				    struct list_head *stmt_list,
				    json_t *stmt_json)
{
	struct list_head *head;
	struct stmt *stmt;
	json_t *value;
	size_t index;

	if (!stmt_json)
		return 0;

	if (!json_is_array(stmt_json)) {
		json_error(ctx, "Unexpected object type in stmt");
		return -1;
	}

	head = stmt_list;
	json_array_foreach(stmt_json, index, value) {
		stmt = json_parse_stmt(ctx, value);
		if (!stmt) {
			json_error(ctx, "Parsing set statements array at index %zd failed.", index);
			stmt_list_free(stmt_list);
			return -1;
		}
		if (!(stmt->flags & STMT_F_STATEFUL)) {
			stmt_free(stmt);
			json_error(ctx, "Unsupported set statements array at index %zd failed.", index);
			stmt_list_free(stmt_list);
			return -1;
		}
		list_add(&stmt->list, head);
		head = &stmt->list;
	}

	return 0;
}

static struct stmt *json_parse_set_stmt(struct json_ctx *ctx,
					const char *key, json_t *value)
{
	const char *opstr, *set;
	struct expr *expr, *expr2;
	json_t *elem, *stmt_json;
	struct stmt *stmt;
	int op;

	if (json_unpack_err(ctx, value, "{s:s, s:o, s:s}",
			    "op", &opstr, "elem", &elem, "set", &set))
		return NULL;

	if (!strcmp(opstr, "add")) {
		op = NFT_DYNSET_OP_ADD;
	} else if (!strcmp(opstr, "update")) {
		op = NFT_DYNSET_OP_UPDATE;
	} else if (!strcmp(opstr, "delete")) {
		op = NFT_DYNSET_OP_DELETE;
	} else {
		json_error(ctx, "Unknown set statement op '%s'.", opstr);
		return NULL;
	}

	expr = json_parse_set_elem_expr_stmt(ctx, elem);
	if (!expr) {
		json_error(ctx, "Illegal set statement element.");
		return NULL;
	}

	if (set[0] != '@') {
		json_error(ctx, "Illegal set reference in set statement.");
		expr_free(expr);
		return NULL;
	}
	expr2 = symbol_expr_alloc(int_loc, SYMBOL_SET, NULL, set + 1);

	stmt = set_stmt_alloc(int_loc);
	stmt->set.op = op;
	stmt->set.key = expr;
	stmt->set.set = expr2;

	if (!json_unpack(value, "{s:o}", "stmt", &stmt_json) &&
	    json_parse_set_stmt_list(ctx, &stmt->set.stmt_list, stmt_json) < 0) {
		stmt_free(stmt);
		return NULL;
	}

	return stmt;
}

static struct stmt *json_parse_map_stmt(struct json_ctx *ctx,
					const char *key, json_t *value)
{
	struct expr *expr, *expr2, *expr_data;
	json_t *elem, *data, *stmt_json;
	const char *opstr, *set;
	struct stmt *stmt;
	int op;

	if (json_unpack_err(ctx, value, "{s:s, s:o, s:o, s:s}",
			    "op", &opstr, "elem", &elem, "data", &data, "map", &set))
		return NULL;

	if (!strcmp(opstr, "add")) {
		op = NFT_DYNSET_OP_ADD;
	} else if (!strcmp(opstr, "update")) {
		op = NFT_DYNSET_OP_UPDATE;
	} else if (!strcmp(opstr, "delete")) {
		op = NFT_DYNSET_OP_DELETE;
	} else {
		json_error(ctx, "Unknown map statement op '%s'.", opstr);
		return NULL;
	}

	expr = json_parse_set_elem_expr_stmt(ctx, elem);
	if (!expr) {
		json_error(ctx, "Illegal map statement element.");
		return NULL;
	}

	expr_data = json_parse_set_elem_expr_stmt(ctx, data);
	if (!expr_data) {
		json_error(ctx, "Illegal map expression data.");
		expr_free(expr);
		return NULL;
	}

	if (set[0] != '@') {
		json_error(ctx, "Illegal map reference in map statement.");
		expr_free(expr);
		expr_free(expr_data);
		return NULL;
	}
	expr2 = symbol_expr_alloc(int_loc, SYMBOL_SET, NULL, set + 1);

	stmt = map_stmt_alloc(int_loc);
	stmt->map.op = op;
	stmt->map.key = expr;
	stmt->map.data = expr_data;
	stmt->map.set = expr2;

	if (!json_unpack(value, "{s:o}", "stmt", &stmt_json) &&
	    json_parse_set_stmt_list(ctx, &stmt->set.stmt_list, stmt_json) < 0) {
		stmt_free(stmt);
		return NULL;
	}

	return stmt;
}

static unsigned int json_parse_log_flag(const char *flag)
{
	const struct {
		const char *flag;
		int val;
	} flag_tbl[] = {
		{ "tcp sequence", NF_LOG_TCPSEQ },
		{ "tcp options", NF_LOG_TCPOPT },
		{ "ip options", NF_LOG_IPOPT },
		{ "skuid", NF_LOG_UID },
		{ "ether", NF_LOG_MACDECODE },
		{ "all", NF_LOG_MASK },
	};
	unsigned int i;

	for (i = 0; i < array_size(flag_tbl); i++) {
		if (!strcmp(flag, flag_tbl[i].flag))
			return flag_tbl[i].val;
	}
	return 0;
}

static struct stmt *json_parse_log_stmt(struct json_ctx *ctx,
					const char *key, json_t *value)
{
	const char *tmpstr;
	struct stmt *stmt;
	int tmp, flags;

	stmt = log_stmt_alloc(int_loc);

	if (!json_unpack(value, "{s:s}", "prefix", &tmpstr)) {
		stmt->log.prefix = xstrdup(tmpstr);
		stmt->log.flags |= STMT_LOG_PREFIX;
	}
	if (!json_unpack(value, "{s:i}", "group", &tmp)) {
		stmt->log.group = tmp;
		stmt->log.flags |= STMT_LOG_GROUP;
	}
	if (!json_unpack(value, "{s:i}", "snaplen", &tmp)) {
		stmt->log.snaplen = tmp;
		stmt->log.flags |= STMT_LOG_SNAPLEN;
	}
	if (!json_unpack(value, "{s:i}", "queue-threshold", &tmp)) {
		stmt->log.qthreshold = tmp;
		stmt->log.flags |= STMT_LOG_QTHRESHOLD;
	}
	if (!json_unpack(value, "{s:s}", "level", &tmpstr)) {
		int level = log_level_parse(tmpstr);

		if (level < 0) {
			json_error(ctx, "Invalid log level '%s'.", tmpstr);
			stmt_free(stmt);
			return NULL;
		}
		stmt->log.level = level;
		stmt->log.flags |= STMT_LOG_LEVEL;
	}
	flags = parse_flags_array(ctx, value, "flags", json_parse_log_flag);
	if (flags < 0) {
		stmt_free(stmt);
		return NULL;
	}
	stmt->log.logflags = flags;

	return stmt;
}

static unsigned int json_parse_synproxy_flag(const char *flag)
{
	const struct {
		const char *flag;
		int val;
	} flag_tbl[] = {
		{ "timestamp", NF_SYNPROXY_OPT_TIMESTAMP },
		{ "sack-perm", NF_SYNPROXY_OPT_SACK_PERM },
	};
	unsigned int i;

	for (i = 0; i < array_size(flag_tbl); i++) {
		if (!strcmp(flag, flag_tbl[i].flag))
			return flag_tbl[i].val;
	}
	return 0;
}

static struct stmt *json_parse_synproxy_stmt(struct json_ctx *ctx,
					     const char *key, json_t *value)
{
	struct stmt *stmt = NULL;
	int tmp, flags;

	if (json_typeof(value) == JSON_NULL) {
		stmt = synproxy_stmt_alloc(int_loc);
		return stmt;
	}

	if (!json_unpack(value, "{s:i}", "mss", &tmp)) {
		if (!stmt)
			stmt = synproxy_stmt_alloc(int_loc);
		if (tmp < 0) {
			json_error(ctx, "Invalid synproxy mss value '%d'", tmp);
			stmt_free(stmt);
			return NULL;
		}
		stmt->synproxy.mss = tmp;
		stmt->synproxy.flags |= NF_SYNPROXY_OPT_MSS;
	}
	if (!json_unpack(value, "{s:i}", "wscale", &tmp)) {
		if (!stmt)
			stmt = synproxy_stmt_alloc(int_loc);
		if (tmp < 0) {
			json_error(ctx, "Invalid synproxy wscale value '%d'", tmp);
			stmt_free(stmt);
			return NULL;
		}
		stmt->synproxy.wscale = tmp;
		stmt->synproxy.flags |= NF_SYNPROXY_OPT_WSCALE;
	}

	flags = parse_flags_array(ctx, value, "flags",
				  json_parse_synproxy_flag);
	if (flags < 0) {
		stmt_free(stmt);
		return NULL;
	}
	if (flags) {
		if (!stmt)
			stmt = synproxy_stmt_alloc(int_loc);
		stmt->synproxy.flags |= flags;
	}

	if (!stmt) {
		stmt = objref_stmt_alloc(int_loc);
		stmt->objref.type = NFT_OBJECT_SYNPROXY;
		stmt->objref.expr = json_parse_stmt_expr(ctx, value);
		if (!stmt->objref.expr) {
			json_error(ctx, "Invalid synproxy reference");
			stmt_free(stmt);
			return NULL;
		}
	}
	return stmt;
}

static struct stmt *json_parse_cthelper_stmt(struct json_ctx *ctx,
					     const char *key, json_t *value)
{
	struct stmt *stmt = objref_stmt_alloc(int_loc);

	stmt->objref.type = NFT_OBJECT_CT_HELPER;
	stmt->objref.expr = json_parse_stmt_expr(ctx, value);
	if (!stmt->objref.expr) {
		json_error(ctx, "Invalid ct helper reference.");
		stmt_free(stmt);
		return NULL;
	}
	return stmt;
}

static struct stmt *json_parse_cttimeout_stmt(struct json_ctx *ctx,
					      const char *key, json_t *value)
{
	struct stmt *stmt = objref_stmt_alloc(int_loc);

	stmt->objref.type = NFT_OBJECT_CT_TIMEOUT;
	stmt->objref.expr = json_parse_stmt_expr(ctx, value);
	if (!stmt->objref.expr) {
		json_error(ctx, "Invalid ct timeout reference.");
		stmt_free(stmt);
		return NULL;
	}
	return stmt;
}

static struct stmt *json_parse_ctexpect_stmt(struct json_ctx *ctx,
					     const char *key, json_t *value)
{
	struct stmt *stmt = objref_stmt_alloc(int_loc);

	stmt->objref.type = NFT_OBJECT_CT_EXPECT;
	stmt->objref.expr = json_parse_stmt_expr(ctx, value);
	if (!stmt->objref.expr) {
		json_error(ctx, "Invalid ct expectation reference.");
		stmt_free(stmt);
		return NULL;
	}
	return stmt;
}

static struct stmt *json_parse_meter_stmt(struct json_ctx *ctx,
					  const char *key, json_t *value)
{
	json_t *jkey, *jstmt;
	struct stmt *stmt;
	const char *name;
	uint32_t size = 0;

	if (json_unpack_err(ctx, value, "{s:s, s:o, s:o}",
			    "name", &name, "key", &jkey, "stmt", &jstmt))
		return NULL;
	json_unpack(value, "{s:i}", "size", &size);

	stmt = meter_stmt_alloc(int_loc);
	stmt->meter.name = xstrdup(name);
	stmt->meter.size = size;

	stmt->meter.key = json_parse_set_elem_expr_stmt(ctx, jkey);
	if (!stmt->meter.key) {
		json_error(ctx, "Invalid meter key.");
		stmt_free(stmt);
		return NULL;
	}

	stmt->meter.stmt = json_parse_stmt(ctx, jstmt);
	if (!stmt->meter.stmt) {
		json_error(ctx, "Invalid meter statement.");
		stmt_free(stmt);
		return NULL;
	}
	return stmt;
}

static unsigned int queue_flag_parse(const char *name)
{
	if (!strcmp(name, "bypass"))
		return NFT_QUEUE_FLAG_BYPASS;
	else if (!strcmp(name, "fanout"))
		return NFT_QUEUE_FLAG_CPU_FANOUT;
	return 0;
}

static struct stmt *json_parse_queue_stmt(struct json_ctx *ctx,
					  const char *key, json_t *value)
{
	struct expr *qexpr = NULL;
	json_t *tmp;
	int flags;

	if (!json_unpack(value, "{s:o}", "num", &tmp)) {
		qexpr = json_parse_stmt_expr(ctx, tmp);
		if (!qexpr) {
			json_error(ctx, "Invalid queue num.");
			return NULL;
		}
	}

	flags = parse_flags_array(ctx, value, "flags", queue_flag_parse);
	if (flags < 0) {
		expr_free(qexpr);
		return NULL;
	}

	return queue_stmt_alloc(int_loc, qexpr, flags);
}

static struct stmt *json_parse_connlimit_stmt(struct json_ctx *ctx,
					      const char *key, json_t *value)
{
	struct stmt *stmt = connlimit_stmt_alloc(int_loc);

	if (json_unpack_err(ctx, value, "{s:i}",
			    "val", &stmt->connlimit.count)) {
		stmt_free(stmt);
		return NULL;
	}

	json_unpack(value, "{s:b}", "inv", &stmt->connlimit.flags);
	if (stmt->connlimit.flags)
		stmt->connlimit.flags = NFT_CONNLIMIT_F_INV;

	return stmt;
}

static struct stmt *json_parse_optstrip_stmt(struct json_ctx *ctx,
					     const char *key, json_t *value)
{
	struct expr *expr = json_parse_expr(ctx, value);

	if (!expr ||
	    expr->etype != EXPR_EXTHDR ||
	    expr->exthdr.op != NFT_EXTHDR_OP_TCPOPT) {
		json_error(ctx, "Illegal TCP optstrip argument");
		expr_free(expr);
		return NULL;
	}

	return optstrip_stmt_alloc(int_loc, expr);
}

static struct stmt *json_parse_stmt(struct json_ctx *ctx, json_t *root)
{
	struct {
		const char *key;
		struct stmt *(*cb)(struct json_ctx *, const char *, json_t *);
	} stmt_parser_tbl[] = {
		{ "accept", json_parse_verdict_stmt },
		{ "drop", json_parse_verdict_stmt },
		{ "continue", json_parse_verdict_stmt },
		{ "jump", json_parse_verdict_stmt },
		{ "goto", json_parse_verdict_stmt },
		{ "return", json_parse_verdict_stmt },
		{ "match", json_parse_match_stmt },
		{ "counter", json_parse_counter_stmt },
		{ "mangle", json_parse_mangle_stmt },
		{ "quota", json_parse_quota_stmt },
		{ "last", json_parse_last_stmt },
		{ "limit", json_parse_limit_stmt },
		{ "flow", json_parse_flow_offload_stmt },
		{ "fwd", json_parse_fwd_stmt },
		{ "notrack", json_parse_notrack_stmt },
		{ "dup", json_parse_dup_stmt },
		{ "snat", json_parse_nat_stmt },
		{ "dnat", json_parse_nat_stmt },
		{ "masquerade", json_parse_nat_stmt },
		{ "redirect", json_parse_nat_stmt },
		{ "reject", json_parse_reject_stmt },
		{ "set", json_parse_set_stmt },
		{ "map", json_parse_map_stmt },
		{ "log", json_parse_log_stmt },
		{ "ct helper", json_parse_cthelper_stmt },
		{ "ct timeout", json_parse_cttimeout_stmt },
		{ "ct expectation", json_parse_ctexpect_stmt },
		{ "meter", json_parse_meter_stmt },
		{ "queue", json_parse_queue_stmt },
		{ "ct count", json_parse_connlimit_stmt },
		{ "tproxy", json_parse_tproxy_stmt },
		{ "synproxy", json_parse_synproxy_stmt },
		{ "reset", json_parse_optstrip_stmt },
		{ "secmark", json_parse_secmark_stmt },
		{ "tunnel", json_parse_tunnel_stmt },
	};
	const char *type;
	unsigned int i;
	json_t *tmp;

	if (json_unpack_stmt(ctx, root, &type, &tmp))
		return NULL;

	/* Yes, verdict_map_stmt is actually an expression */
	if (!strcmp(type, "vmap")) {
		struct expr *expr = json_parse_map_expr(ctx, type, tmp);

		if (!expr) {
			json_error(ctx, "Illegal vmap statement.");
			return NULL;
		}
		return verdict_stmt_alloc(int_loc, expr);
	}

	if (!strcmp(type, "xt")) {
		json_error(ctx, "unsupported xtables compat expression, use iptables-nft with this ruleset");
		return NULL;
	}

	for (i = 0; i < array_size(stmt_parser_tbl); i++) {
		if (!strcmp(type, stmt_parser_tbl[i].key))
			return stmt_parser_tbl[i].cb(ctx, stmt_parser_tbl[i].key, tmp);
	}

	json_error(ctx, "Unknown statement object '%s'.", type);
	return NULL;
}

static struct cmd *json_parse_cmd_add_table(struct json_ctx *ctx, json_t *root,
					    enum cmd_ops op, enum cmd_obj obj)
{
	const char *family = "", *comment = NULL;
	struct handle h = {
		.table.location = *int_loc,
	};
	struct table *table = NULL;
	int flags = 0;

	if (json_unpack_err(ctx, root, "{s:s}",
			    "family", &family))
		return NULL;

	if (op != CMD_DELETE) {
		if (json_unpack_err(ctx, root, "{s:s}", "name", &h.table.name))
			return NULL;

		json_unpack(root, "{s:s}", "comment", &comment);

		flags = parse_flags_array(ctx, root, "flags", parse_table_flag);
		if (flags < 0)
			return NULL;
	} else if (op == CMD_DELETE &&
		   json_unpack(root, "{s:s}", "name", &h.table.name) &&
		   json_unpack(root, "{s:I}", "handle", &h.handle.id)) {
		json_error(ctx, "Either name or handle required to delete a table.");
		return NULL;
	}
	if (parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}
	if (h.table.name)
		h.table.name = xstrdup(h.table.name);

	if (comment || flags) {
		table = table_alloc();
		handle_merge(&table->handle, &h);
		if (comment)
			table->comment = xstrdup(comment);
		table->flags = flags;
	}

	if (op == CMD_ADD)
		json_object_del(root, "handle");

	return cmd_alloc(op, obj, &h, int_loc, table);
}

static struct expr *parse_policy(const char *policy)
{
	int policy_num;

	if (!strcmp(policy, "accept"))
		policy_num = NF_ACCEPT;
	else if (!strcmp(policy, "drop"))
		policy_num = NF_DROP;
	else
		return NULL;

	return constant_expr_alloc(int_loc, &integer_type,
				   BYTEORDER_HOST_ENDIAN,
				   sizeof(int) * BITS_PER_BYTE, &policy_num);
}

static struct expr *ifname_expr_alloc(struct json_ctx *ctx,
				      const char *name)
{
	size_t length = strlen(name);

	if (length == 0) {
		json_error(ctx, "empty interface name");
		return NULL;
	}

	if (length >= IFNAMSIZ) {
		json_error(ctx, "Device name %s too long", name);
		return NULL;
	}

	return constant_expr_alloc(int_loc, &ifname_type, BYTEORDER_HOST_ENDIAN,
				   length * BITS_PER_BYTE, name);
}

static struct expr *json_parse_devs(struct json_ctx *ctx, json_t *root)
{
	struct expr *tmp, *expr = list_expr_alloc(int_loc);
	const char *dev;
	json_t *value;
	size_t index;

	if (!json_unpack(root, "s", &dev)) {
		tmp = ifname_expr_alloc(ctx, dev);
		if (!tmp) {
			expr_free(expr);
			return NULL;
		}

		list_expr_add(expr, tmp);
		return expr;
	}
	if (!json_is_array(root)) {
		expr_free(expr);
		return NULL;
	}

	json_array_foreach(root, index, value) {
		if (json_unpack(value, "s", &dev)) {
			json_error(ctx, "Invalid device at index %zu.",
				   index);
			expr_free(expr);
			return NULL;
		}

		tmp = ifname_expr_alloc(ctx, dev);
		if (!tmp) {
			expr_free(expr);
			return NULL;
		}
		list_expr_add(expr, tmp);
	}
	return expr;
}

static struct cmd *json_parse_cmd_add_chain(struct json_ctx *ctx, json_t *root,
					    enum cmd_ops op, enum cmd_obj obj)
{
	struct handle h = {
		.table.location = *int_loc,
	};
	const char *family = "", *policy = "", *type, *hookstr, *comment = NULL;
	struct chain *chain = NULL;
	json_t *devs = NULL;
	int prio;

	if (json_unpack_err(ctx, root, "{s:s, s:s}",
			    "family", &family,
			    "table", &h.table.name))
		return NULL;
	if (op != CMD_DELETE) {
		if (json_unpack_err(ctx, root, "{s:s}", "name", &h.chain.name))
			return NULL;

		json_unpack(root, "{s:s}", "comment", &comment);
	} else if (op == CMD_DELETE &&
		   json_unpack(root, "{s:s}", "name", &h.chain.name) &&
		   json_unpack(root, "{s:I}", "handle", &h.handle.id)) {
		json_error(ctx, "Either name or handle required to delete a chain.");
		return NULL;
	}
	if (parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}
	h.table.name = xstrdup(h.table.name);
	if (h.chain.name)
		h.chain.name = xstrdup(h.chain.name);

	if (comment) {
		chain = chain_alloc();
		handle_merge(&chain->handle, &h);
		chain->comment = xstrdup(comment);
	}

	if (op == CMD_DELETE ||
	    op == CMD_LIST ||
	    op == CMD_FLUSH ||
	    json_unpack(root, "{s:s, s:s, s:i}",
			"type", &type, "hook", &hookstr, "prio", &prio))
		return cmd_alloc(op, obj, &h, int_loc, chain);

	if (!chain)
		chain = chain_alloc();

	chain->flags |= CHAIN_F_BASECHAIN;
	chain->type.str = xstrdup(type);
	chain->priority.expr = constant_expr_alloc(int_loc, &integer_type,
						   BYTEORDER_HOST_ENDIAN,
						   sizeof(int) * BITS_PER_BYTE,
						   &prio);
	chain->hook.name = chain_hookname_lookup(hookstr);
	if (!chain->hook.name) {
		json_error(ctx, "Invalid chain hook '%s'.", hookstr);
		goto err_free_chain;
	}

	json_unpack(root, "{s:o}", "dev", &devs);

	if (devs) {
		chain->dev_expr = json_parse_devs(ctx, devs);
		if (!chain->dev_expr) {
			json_error(ctx, "Invalid chain dev.");
			goto err_free_chain;
		}
	}

	if (!json_unpack(root, "{s:s}", "policy", &policy)) {
		chain->policy = parse_policy(policy);
		if (!chain->policy) {
			json_error(ctx, "Unknown policy '%s'.", policy);
			goto err_free_chain;
		}
	}

	if (op == CMD_ADD)
		json_object_del(root, "handle");

	handle_merge(&chain->handle, &h);
	return cmd_alloc(op, obj, &h, int_loc, chain);

err_free_chain:
	chain_free(chain);
	handle_free(&h);
	return NULL;
}

static struct cmd *json_parse_cmd_add_rule(struct json_ctx *ctx, json_t *root,
					   enum cmd_ops op, enum cmd_obj obj)
{
	struct handle h = {
		.table.location = *int_loc,
		.chain.location = *int_loc,
		.index.location = *int_loc,
	};
	const char *family = "", *comment = NULL;
	struct rule *rule;
	size_t index;
	json_t *tmp, *value;

	if (json_unpack_err(ctx, root, "{s:s, s:s, s:s}",
			    "family", &family,
			    "table", &h.table.name,
			    "chain", &h.chain.name))
		return NULL;
	if (op != CMD_DELETE &&
	    json_unpack_err(ctx, root, "{s:o}", "expr", &tmp))
		return NULL;
	else if ((op == CMD_DELETE || op == CMD_DESTROY) &&
		 json_unpack_err(ctx, root, "{s:I}", "handle", &h.handle.id))
		return NULL;

	if (parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}
	h.table.name = xstrdup(h.table.name);
	h.chain.name = xstrdup(h.chain.name);

	if (op == CMD_DELETE || op == CMD_DESTROY)
		return cmd_alloc(op, obj, &h, int_loc, NULL);

	if (!json_is_array(tmp)) {
		json_error(ctx, "Value of property \"expr\" must be an array.");
		handle_free(&h);
		return NULL;
	}

	if (!json_unpack(root, "{s:I}", "index", &h.index.id)) {
		h.index.id++;
	}

	rule = rule_alloc(int_loc, NULL);

	json_unpack(root, "{s:s}", "comment", &comment);
	if (comment)
		rule->comment = xstrdup(comment);

	json_array_foreach(tmp, index, value) {
		struct stmt *stmt;

		if (!json_is_object(value)) {
			json_error(ctx, "Unexpected expr array element of type %s, expected object.",
				   json_typename(value));
			goto err_free_rule;
		}

		stmt = json_parse_stmt(ctx, value);

		if (!stmt) {
			json_error(ctx, "Parsing expr array at index %zd failed.", index);
			goto err_free_rule;
		}

		rule_stmt_append(rule, stmt);
	}

	if (op == CMD_ADD)
		json_object_del(root, "handle");

	return cmd_alloc(op, obj, &h, int_loc, rule);

err_free_rule:
	rule_free(rule);
	handle_free(&h);
	return NULL;
}

static int string_to_nft_object(const char *str)
{
	const char *obj_tbl[__NFT_OBJECT_MAX] = {
		[NFT_OBJECT_COUNTER]	= "counter",
		[NFT_OBJECT_QUOTA]	= "quota",
		[NFT_OBJECT_CT_HELPER]	= "ct helper",
		[NFT_OBJECT_LIMIT]	= "limit",
		[NFT_OBJECT_CT_TIMEOUT]	= "ct timeout",
		[NFT_OBJECT_SECMARK]	= "secmark",
		[NFT_OBJECT_CT_EXPECT]	= "ct expectation",
		[NFT_OBJECT_SYNPROXY]	= "synproxy",
		[NFT_OBJECT_TUNNEL]	= "tunnel",
	};
	unsigned int i;

	for (i = 0; i <= NFT_OBJECT_MAX; i++) {
		if (obj_tbl[i] && !strcmp(str, obj_tbl[i]))
			return i;
	}
	return 0;
}

static unsigned int string_to_set_flag(const char *str)
{
	const struct {
		enum nft_set_flags val;
		const char *name;
	} flag_tbl[] = {
		{ NFT_SET_CONSTANT, "constant" },
		{ NFT_SET_INTERVAL, "interval" },
		{ NFT_SET_TIMEOUT, "timeout" },
		{ NFT_SET_EVAL,	"dynamic" },
	};
	unsigned int i;

	for (i = 0; i < array_size(flag_tbl); i++) {
		if (!strcmp(str, flag_tbl[i].name))
			return flag_tbl[i].val;
	}
	return 0;
}

static struct cmd *json_parse_cmd_add_set(struct json_ctx *ctx, json_t *root,
					  enum cmd_ops op, enum cmd_obj obj)
{
	struct handle h = { 0 };
	const char *family = "", *policy;
	json_t *tmp, *stmt_json;
	struct set *set;
	int flags;

	if (json_unpack_err(ctx, root, "{s:s, s:s}",
			    "family", &family,
			    "table", &h.table.name))
		return NULL;
	if (op != CMD_DELETE &&
	    json_unpack_err(ctx, root, "{s:s}", "name", &h.set.name)) {
		return NULL;
	} else if ((op == CMD_DELETE || op == CMD_DESTROY) &&
		   json_unpack(root, "{s:s}", "name", &h.set.name) &&
		   json_unpack(root, "{s:I}", "handle", &h.handle.id)) {
		json_error(ctx, "Either name or handle required to delete a set.");
		return NULL;
	}

	if (parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}
	h.table.name = xstrdup(h.table.name);
	if (h.set.name)
		h.set.name = xstrdup(h.set.name);

	switch (op) {
	case CMD_DELETE:
	case CMD_DESTROY:
	case CMD_LIST:
	case CMD_FLUSH:
	case CMD_RESET:
		return cmd_alloc(op, obj, &h, int_loc, NULL);
	default:
		break;
	}

	set = set_alloc(&internal_location);

	if (json_unpack(root, "{s:o}", "type", &tmp)) {
		json_error(ctx, "Invalid set type.");
		set_free(set);
		handle_free(&h);
		return NULL;
	}
	set->key = json_parse_dtype_expr(ctx, tmp);
	if (!set->key) {
		json_error(ctx, "Invalid set type.");
		set_free(set);
		handle_free(&h);
		return NULL;
	}

	if (!json_unpack(root, "{s:o}", "map", &tmp)) {
		if (json_is_string(tmp)) {
			const char *s = json_string_value(tmp);

			set->objtype = string_to_nft_object(s);
		}
		if (set->objtype) {
			set->flags |= NFT_SET_OBJECT;
		} else if ((set->data = json_parse_dtype_expr(ctx, tmp))) {
			set->flags |= NFT_SET_MAP;
		} else {
			const char *dump = json_dumps(tmp, 0);

			json_error(ctx, "Invalid map type '%s'.", dump);
			free_const(dump);
			set_free(set);
			handle_free(&h);
			return NULL;
		}
	}
	if (!json_unpack(root, "{s:s}", "policy", &policy)) {
		if (!strcmp(policy, "performance"))
			set->policy = NFT_SET_POL_PERFORMANCE;
		else if (!strcmp(policy, "memory")) {
			set->policy = NFT_SET_POL_MEMORY;
		} else {
			json_error(ctx, "Unknown set policy '%s'.", policy);
			set_free(set);
			handle_free(&h);
			return NULL;
		}
	}

	flags = parse_flags_array(ctx, root, "flags", string_to_set_flag);
	if (flags < 0) {
		json_error(ctx, "Invalid set flags in set '%s'.", h.set.name);
		set_free(set);
		handle_free(&h);
		return NULL;
	}
	set->flags |= flags;

	if (!json_unpack(root, "{s:o}", "elem", &tmp)) {
		set->init = json_parse_set_expr(ctx, "elem", tmp);
		if (!set->init) {
			json_error(ctx, "Invalid set elem expression.");
			set_free(set);
			handle_free(&h);
			return NULL;
		}
	}
	if (!json_unpack(root, "{s:I}", "timeout", &set->timeout))
		set->timeout *= 1000;
	if (!json_unpack(root, "{s:i}", "gc-interval", &set->gc_int))
		set->gc_int *= 1000;
	json_unpack(root, "{s:i}", "size", &set->desc.size);
	json_unpack(root, "{s:b}", "auto-merge", &set->automerge);

	if (!json_unpack(root, "{s:o}", "stmt", &stmt_json) &&
	    json_parse_set_stmt_list(ctx, &set->stmt_list, stmt_json) < 0) {
		set_free(set);
		handle_free(&h);
		return NULL;
	}

	handle_merge(&set->handle, &h);

	if (op == CMD_ADD)
		json_object_del(root, "handle");

	return cmd_alloc(op, obj, &h, int_loc, set);
}

static struct cmd *json_parse_cmd_add_element(struct json_ctx *ctx,
					      json_t *root, enum cmd_ops op,
					      enum cmd_obj cmd_obj)
{
	struct handle h = { 0 };
	const char *family;
	struct expr *expr;
	json_t *tmp;

	if (json_unpack_err(ctx, root, "{s:s, s:s, s:s, s:o}",
			    "family", &family,
			    "table", &h.table.name,
			    "name", &h.set.name,
			    "elem", &tmp))
		return NULL;

	if (parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}
	h.table.name = xstrdup(h.table.name);
	h.set.name = xstrdup(h.set.name);

	expr = json_parse_set_expr(ctx, "elem", tmp);
	if (!expr) {
		json_error(ctx, "Invalid set.");
		handle_free(&h);
		return NULL;
	}

	if ((op == CMD_CREATE || op == CMD_ADD) &&
	    nft_cmd_collapse_elems(op, ctx->cmds, &h, expr)) {
		handle_free(&h);
		expr_free(expr);
		ctx->flags |= CTX_F_COLLAPSED;
		return NULL;
	}

	return cmd_alloc(op, cmd_obj, &h, int_loc, expr);
}

static struct cmd *json_parse_cmd_add_flowtable(struct json_ctx *ctx,
						json_t *root, enum cmd_ops op,
						enum cmd_obj cmd_obj)
{
	const char *family, *hook, *hookstr;
	struct flowtable *flowtable;
	struct handle h = { 0 };
	json_t *devs = NULL;
	int prio;

	if (json_unpack_err(ctx, root, "{s:s, s:s}",
			    "family", &family,
			    "table", &h.table.name))
		return NULL;

	if (op != CMD_DELETE &&
	    json_unpack_err(ctx, root, "{s:s}", "name", &h.flowtable.name)) {
		return NULL;
	} else if ((op == CMD_DELETE || op == CMD_DESTROY) &&
		   json_unpack(root, "{s:s}", "name", &h.flowtable.name) &&
		   json_unpack(root, "{s:I}", "handle", &h.handle.id)) {
		json_error(ctx, "Either name or handle required to delete a flowtable.");
		return NULL;
	}

	if (parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}
	h.table.name = xstrdup(h.table.name);
	if (h.flowtable.name)
		h.flowtable.name = xstrdup(h.flowtable.name);

	if (op == CMD_DELETE || op == CMD_LIST || op == CMD_DESTROY)
		return cmd_alloc(op, cmd_obj, &h, int_loc, NULL);

	if (json_unpack_err(ctx, root, "{s:s, s:i}",
			    "hook", &hook,
			    "prio", &prio)) {
		handle_free(&h);
		return NULL;
	}

	json_unpack(root, "{s:o}", "dev", &devs);

	hookstr = chain_hookname_lookup(hook);
	if (!hookstr) {
		json_error(ctx, "Invalid flowtable hook '%s'.", hook);
		handle_free(&h);
		return NULL;
	}

	flowtable = flowtable_alloc(int_loc);
	flowtable->hook.name = hookstr;
	flowtable->priority.expr =
		constant_expr_alloc(int_loc, &integer_type,
				    BYTEORDER_HOST_ENDIAN,
				    sizeof(int) * BITS_PER_BYTE, &prio);

	if (devs) {
		flowtable->dev_expr = json_parse_devs(ctx, devs);
		if (!flowtable->dev_expr) {
			json_error(ctx, "Invalid flowtable dev.");
			flowtable_free(flowtable);
			handle_free(&h);
			return NULL;
		}
	}
	return cmd_alloc(op, cmd_obj, &h, int_loc, flowtable);
}

static int json_parse_ct_timeout_policy(struct json_ctx *ctx,
					json_t *root, struct obj *obj)
{
	json_t *tmp, *val;
	const char *key;

	if (json_unpack(root, "{s:o}", "policy", &tmp))
		return 0;

	if (!json_is_object(tmp)) {
		json_error(ctx, "Invalid ct timeout policy.");
		return 1;
	}

	json_object_foreach(tmp, key, val) {
		struct timeout_state *ts;

		if (!json_is_integer(val)) {
			json_error(ctx, "Invalid ct timeout policy value for '%s'.", key);
			return 1;
		}

		ts = xzalloc(sizeof(*ts));
		ts->timeout_str = xstrdup(key);
		ts->timeout_value = json_integer_value(val);
		ts->location = *int_loc;
		init_list_head(&ts->head);
		list_add_tail(&ts->head, &obj->ct_timeout.timeout_list);
	}
	return 0;
}

static int json_parse_tunnel_erspan(struct json_ctx *ctx,
				    json_t *root, struct obj *obj)
{
	const char *dir;
	json_t *tmp;
	int i;

	if (json_unpack_err(ctx, root, "{s:o}", "tunnel", &tmp))
		return 1;

	if (json_unpack_err(ctx, tmp, "{s:i}", "version", &obj->tunnel.erspan.version))
		return 1;

	switch (obj->tunnel.erspan.version) {
	case 1:
		if (json_unpack_err(ctx, tmp, "{s:i}",
				    "index", &obj->tunnel.erspan.v1.index))
			return 1;
		break;
	case 2:
		if (json_unpack_err(ctx, tmp, "{s:s, s:i}",
				   "dir", &dir,
				   "hwid", &i))
			return 1;
		obj->tunnel.erspan.v2.hwid = i;

		if (!strcmp(dir, "ingress")) {
			obj->tunnel.erspan.v2.direction = 0;
		} else if (!strcmp(dir, "egress")) {
			obj->tunnel.erspan.v2.direction = 1;
		} else {
			json_error(ctx, "Invalid direction '%s'.", dir);
			return 1;
		}
		break;
	default:
		json_error(ctx, "Invalid erspan version %u" , obj->tunnel.erspan.version);
		return 1;
	}

	return 0;
}

static enum tunnel_type json_parse_tunnel_type(struct json_ctx *ctx,
					       const char *type)
{
	const struct {
		const char *type;
		int val;
	} type_tbl[] = {
		{ "erspan", TUNNEL_ERSPAN },
		{ "vxlan", TUNNEL_VXLAN },
		{ "geneve", TUNNEL_GENEVE },
	};
	unsigned int i;

	if (!type)
		return TUNNEL_UNSPEC;

	for (i = 0; i < array_size(type_tbl); i++) {
		if (!strcmp(type, type_tbl[i].type))
			return type_tbl[i].val;
	}

	return TUNNEL_UNSPEC;
}

static int json_parse_tunnel_src_and_dst(struct json_ctx *ctx,
					 json_t *root,
					 struct obj *obj)
{
	bool is_ipv4 = false, src_set = false, dst_set = false;
	struct expr *expr;
	json_t *tmp;

	if (!json_unpack(root, "{s:o}", "src-ipv4", &tmp)) {
		is_ipv4 = true;
		src_set = true;
		expr = json_parse_expr(ctx, tmp);
		if (!expr)
			return -1;
		datatype_set(expr, &ipaddr_type);
		obj->tunnel.src = expr;
	}

	if (!json_unpack(root, "{s:o}", "src-ipv6", &tmp)) {
		if (is_ipv4 || src_set)
			return -1;
		src_set = true;
		expr = json_parse_expr(ctx, tmp);
		if (!expr)
			return -1;
		datatype_set(expr, &ip6addr_type);
		obj->tunnel.src = expr;
	}

	if (!json_unpack(root, "{s:o}", "dst-ipv4", &tmp)) {
		dst_set = true;
		if (!is_ipv4)
			return -1;
		expr = json_parse_expr(ctx, tmp);
		if (!expr)
			return -1;
		datatype_set(expr, &ipaddr_type);
		obj->tunnel.dst = expr;
	}

	if (!json_unpack(root, "{s:o}", "dst-ipv6", &tmp)) {
		if (is_ipv4 || dst_set)
			return -1;
		dst_set = true;
		expr = json_parse_expr(ctx, tmp);
		if (!expr)
			return -1;
		datatype_set(expr, &ip6addr_type);
		obj->tunnel.dst = expr;
	}

	if (!dst_set || !src_set)
		return -1;

	return 0;
}

static struct cmd *json_parse_cmd_add_object(struct json_ctx *ctx,
					     json_t *root, enum cmd_ops op,
					     enum cmd_obj cmd_obj)
{
	const char *family, *tmp = NULL, *rate_unit = "packets", *burst_unit = "bytes";
	uint32_t l3proto = NFPROTO_UNSPEC;
	int inv = 0, flags = 0, i, j;
	struct handle h = { 0 };
	json_t *tmp_json;
	struct obj *obj;

	if (json_unpack_err(ctx, root, "{s:s, s:s}",
			    "family", &family,
			    "table", &h.table.name))
		return NULL;
	if ((op != CMD_DELETE ||
	     cmd_obj == NFT_OBJECT_CT_HELPER) &&
	    json_unpack_err(ctx, root, "{s:s}", "name", &h.obj.name)) {
		return NULL;
	} else if ((op == CMD_DELETE || op == CMD_DESTROY) &&
		   cmd_obj != NFT_OBJECT_CT_HELPER &&
		   json_unpack(root, "{s:s}", "name", &h.obj.name) &&
		   json_unpack(root, "{s:I}", "handle", &h.handle.id)) {
		json_error(ctx, "Either name or handle required to delete an object.");
		return NULL;
	}

	if (parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}
	h.table.name = xstrdup(h.table.name);
	if (h.obj.name)
		h.obj.name = xstrdup(h.obj.name);

	if (op == CMD_DELETE || op == CMD_LIST || op == CMD_DESTROY) {
		if (cmd_obj == NFT_OBJECT_CT_HELPER)
			return cmd_alloc_obj_ct(op, NFT_OBJECT_CT_HELPER,
						&h, int_loc, obj_alloc(int_loc));
		return cmd_alloc(op, cmd_obj, &h, int_loc, NULL);
	}

	obj = obj_alloc(int_loc);

	if (!json_unpack(root, "{s:s}", "comment", &obj->comment))
		obj->comment = xstrdup(obj->comment);

	switch (cmd_obj) {
	case CMD_OBJ_COUNTER:
		obj->type = NFT_OBJECT_COUNTER;
		json_unpack(root, "{s:I}", "packets", &obj->counter.packets);
		json_unpack(root, "{s:I}", "bytes", &obj->counter.bytes);
		break;
	case CMD_OBJ_QUOTA:
		obj->type = NFT_OBJECT_QUOTA;
		json_unpack(root, "{s:I}", "bytes", &obj->quota.bytes);
		json_unpack(root, "{s:I}", "used", &obj->quota.used);
		json_unpack(root, "{s:b}", "inv", &obj->quota.flags);
		if (obj->quota.flags)
			obj->quota.flags = NFT_QUOTA_F_INV;
		break;
	case CMD_OBJ_SECMARK:
		obj->type = NFT_OBJECT_SECMARK;
		if (!json_unpack(root, "{s:s}", "context", &tmp)) {
			int ret;
			ret = snprintf(obj->secmark.ctx, sizeof(obj->secmark.ctx), "%s", tmp);
			if (ret < 0 || ret >= (int)sizeof(obj->secmark.ctx)) {
				json_error(ctx, "Invalid secmark context '%s', max length is %zu.",
					   tmp, sizeof(obj->secmark.ctx));
				goto err_free_obj;
			}
		}
		break;
	case NFT_OBJECT_CT_HELPER:
		cmd_obj = CMD_OBJ_CT_HELPER;
		obj->type = NFT_OBJECT_CT_HELPER;
		if (!json_unpack(root, "{s:s}", "type", &tmp)) {
			int ret;

			ret = snprintf(obj->ct_helper.name,
				       sizeof(obj->ct_helper.name), "%s", tmp);
			if (ret < 0 ||
			    ret >= (int)sizeof(obj->ct_helper.name)) {
				json_error(ctx, "Invalid CT helper type '%s', max length is %zu.",
					   tmp, sizeof(obj->ct_helper.name));
				goto err_free_obj;
			}
		}
		if (!json_unpack(root, "{s:s}", "protocol", &tmp)) {
			if (!strcmp(tmp, "tcp")) {
				obj->ct_helper.l4proto = IPPROTO_TCP;
			} else if (!strcmp(tmp, "udp")) {
				obj->ct_helper.l4proto = IPPROTO_UDP;
			} else {
				json_error(ctx, "Invalid ct helper protocol '%s'.", tmp);
				goto err_free_obj;
			}
		}
		if (!json_unpack(root, "{s:s}", "l3proto", &tmp) &&
		    parse_family(tmp, &l3proto)) {
			json_error(ctx, "Invalid ct helper l3proto '%s'.", tmp);
			goto err_free_obj;
		}
		obj->ct_helper.l3proto = l3proto;
		break;
	case NFT_OBJECT_CT_TIMEOUT:
		cmd_obj = CMD_OBJ_CT_TIMEOUT;
		init_list_head(&obj->ct_timeout.timeout_list);
		obj->type = NFT_OBJECT_CT_TIMEOUT;
		if (!json_unpack(root, "{s:s}", "protocol", &tmp)) {
			if (!strcmp(tmp, "tcp")) {
				obj->ct_timeout.l4proto = IPPROTO_TCP;
			} else if (!strcmp(tmp, "udp")) {
				obj->ct_timeout.l4proto = IPPROTO_UDP;
			} else {
				json_error(ctx, "Invalid ct timeout protocol '%s'.", tmp);
				goto err_free_obj;
			}
		}
		if (!json_unpack(root, "{s:s}", "l3proto", &tmp) &&
		    parse_family(tmp, &l3proto)) {
			json_error(ctx, "Invalid ct timeout l3proto '%s'.", tmp);
			goto err_free_obj;
		}
		obj->ct_timeout.l3proto = l3proto;

		if (json_parse_ct_timeout_policy(ctx, root, obj))
			goto err_free_obj;
		break;
	case NFT_OBJECT_CT_EXPECT:
		cmd_obj = CMD_OBJ_CT_EXPECT;
		obj->type = NFT_OBJECT_CT_EXPECT;
		if (!json_unpack(root, "{s:s}", "l3proto", &tmp) &&
		    parse_family(tmp, &l3proto)) {
			json_error(ctx, "Invalid ct expectation l3proto '%s'.", tmp);
			goto err_free_obj;
		}
		obj->ct_expect.l3proto = l3proto;
		if (!json_unpack(root, "{s:s}", "protocol", &tmp)) {
			if (!strcmp(tmp, "tcp")) {
				obj->ct_expect.l4proto = IPPROTO_TCP;
			} else if (!strcmp(tmp, "udp")) {
				obj->ct_expect.l4proto = IPPROTO_UDP;
			} else {
				json_error(ctx, "Invalid ct expectation protocol '%s'.", tmp);
				goto err_free_obj;
			}
		}
		if (!json_unpack(root, "{s:i}", "dport", &i))
			obj->ct_expect.dport = i;
		if (!json_unpack(root, "{s:i}", "timeout", &i))
			obj->ct_expect.timeout = i;
		if (!json_unpack(root, "{s:i}", "size", &i))
			obj->ct_expect.size = i;
		break;
	case CMD_OBJ_LIMIT:
		obj->type = NFT_OBJECT_LIMIT;
		if (json_unpack_err(ctx, root, "{s:I, s:s}",
				    "rate", &obj->limit.rate,
				    "per", &tmp))
			goto err_free_obj;

		json_unpack(root, "{s:s}", "rate_unit", &rate_unit);
		json_unpack(root, "{s:b}", "inv", &inv);
		json_unpack(root, "{s:i}", "burst", &obj->limit.burst);
		json_unpack(root, "{s:s}", "burst_unit", &burst_unit);

		if (!strcmp(rate_unit, "packets")) {
			obj->limit.type = NFT_LIMIT_PKTS;
		} else {
			obj->limit.type = NFT_LIMIT_PKT_BYTES;
			obj->limit.rate = rate_to_bytes(obj->limit.rate,
							rate_unit);
			obj->limit.burst = rate_to_bytes(obj->limit.burst,
							 burst_unit);
		}
		obj->limit.unit = seconds_from_unit(tmp);
		obj->limit.flags = inv ? NFT_LIMIT_F_INV : 0;
		break;
	case CMD_OBJ_SYNPROXY:
		obj->type = NFT_OBJECT_SYNPROXY;
		if (json_unpack_err(ctx, root, "{s:i, s:i}",
				    "mss", &i, "wscale", &j))
			goto err_free_obj;

		obj->synproxy.mss = i;
		obj->synproxy.wscale = j;
		obj->synproxy.flags |= NF_SYNPROXY_OPT_MSS;
		obj->synproxy.flags |= NF_SYNPROXY_OPT_WSCALE;
		flags = parse_flags_array(ctx, root, "flags",
					  json_parse_synproxy_flag);
		if (flags < 0)
			goto err_free_obj;

		obj->synproxy.flags |= flags;
		break;
	case NFT_OBJECT_TUNNEL:
		cmd_obj = CMD_OBJ_TUNNEL;
		obj->type = NFT_OBJECT_TUNNEL;

		if (json_parse_tunnel_src_and_dst(ctx, root, obj))
			goto err_free_obj;

		json_unpack(root, "{s:i}", "id", &obj->tunnel.id);
		json_unpack(root, "{s:i}", "sport", &i);
		obj->tunnel.sport = i;
		json_unpack(root, "{s:i}", "dport", &i);
		obj->tunnel.sport = i;
		json_unpack(root, "{s:i}", "ttl", &i);
		obj->tunnel.ttl = i;
		json_unpack(root, "{s:i}", "tos", &i);
		obj->tunnel.tos = i;
		json_unpack(root, "{s:s}", "type", &tmp);

		obj->tunnel.type = json_parse_tunnel_type(ctx, tmp);
		switch (obj->tunnel.type) {
		case TUNNEL_UNSPEC:
			break;
		case TUNNEL_ERSPAN:
			if (json_parse_tunnel_erspan(ctx, root, obj))
				goto err_free_obj;
			break;
		case TUNNEL_VXLAN:
			if (json_unpack_err(ctx, root,
					    "{s:o}", "tunnel", &tmp_json))
				goto err_free_obj;

			json_unpack(tmp_json, "{s:i}",
				    "gbp", &obj->tunnel.vxlan.gbp);
			break;
		case TUNNEL_GENEVE:
			json_t *value;
			size_t index;

			if (json_unpack_err(ctx, root,
					    "{s:o}", "tunnel", &tmp_json))
				goto err_free_obj;

			json_array_foreach(tmp_json, index, value) {
				struct tunnel_geneve *geneve = xmalloc(sizeof(struct tunnel_geneve));
				if (!geneve)
					memory_allocation_error();

				if (json_unpack_err(ctx, value, "{s:i, s:i, s:s}",
						    "class", &i,
						    "opt-type", &j,
						    "data", &tmp)) {
					free(geneve);
					goto err_free_obj;
				}
				geneve->geneve_class = i;
				geneve->type = j;

				if (tunnel_geneve_data_str2array(tmp,
								 geneve->data,
								 &geneve->data_len)) {
					free(geneve);
					goto err_free_obj;
				}

				if (index == 0)
					init_list_head(&obj->tunnel.geneve_opts);

				list_add_tail(&geneve->list, &obj->tunnel.geneve_opts);
			}
			break;
		}
		break;
	default:
		BUG("Invalid CMD '%d'", cmd_obj);
	}

	if (op == CMD_ADD)
		json_object_del(root, "handle");

	return cmd_alloc(op, cmd_obj, &h, int_loc, obj);

err_free_obj:
	obj_free(obj);
	handle_free(&h);
	return NULL;
}

static struct cmd *json_parse_cmd_add(struct json_ctx *ctx,
				      json_t *root, enum cmd_ops op)
{
	struct {
		const char *key;
		enum cmd_obj obj;
		struct cmd *(*cb)(struct json_ctx *, json_t *,
				  enum cmd_ops, enum cmd_obj);
	} cmd_obj_table[] = {
		{ "table", CMD_OBJ_TABLE, json_parse_cmd_add_table },
		{ "chain", CMD_OBJ_CHAIN, json_parse_cmd_add_chain },
		{ "rule", CMD_OBJ_RULE, json_parse_cmd_add_rule },
		{ "set", CMD_OBJ_SET, json_parse_cmd_add_set },
		{ "map", CMD_OBJ_SET, json_parse_cmd_add_set },
		{ "element", CMD_OBJ_ELEMENTS, json_parse_cmd_add_element },
		{ "flowtable", CMD_OBJ_FLOWTABLE, json_parse_cmd_add_flowtable },
		{ "counter", CMD_OBJ_COUNTER, json_parse_cmd_add_object },
		{ "quota", CMD_OBJ_QUOTA, json_parse_cmd_add_object },
		{ "ct helper", NFT_OBJECT_CT_HELPER, json_parse_cmd_add_object },
		{ "ct timeout", NFT_OBJECT_CT_TIMEOUT, json_parse_cmd_add_object },
		{ "ct expectation", NFT_OBJECT_CT_EXPECT, json_parse_cmd_add_object },
		{ "tunnel", NFT_OBJECT_TUNNEL, json_parse_cmd_add_object },
		{ "limit", CMD_OBJ_LIMIT, json_parse_cmd_add_object },
		{ "secmark", CMD_OBJ_SECMARK, json_parse_cmd_add_object },
		{ "synproxy", CMD_OBJ_SYNPROXY, json_parse_cmd_add_object }
	};
	unsigned int i;
	json_t *tmp;

	if (!json_is_object(root)) {
		json_error(ctx, "Value of add command must be object (got %s instead).",
			   json_typename(root));
		return NULL;
	}

	for (i = 0; i < array_size(cmd_obj_table); i++) {
		tmp = json_object_get(root, cmd_obj_table[i].key);
		if (!tmp)
			continue;

		if (op == CMD_CREATE && cmd_obj_table[i].obj == CMD_OBJ_RULE) {
			json_error(ctx, "Create command not available for rules.");
			return NULL;
		}

		return cmd_obj_table[i].cb(ctx, tmp, op, cmd_obj_table[i].obj);
	}
	json_error(ctx, "Unknown object passed to add command.");
	return NULL;
}

static struct cmd *json_parse_cmd_replace(struct json_ctx *ctx,
					  json_t *root, enum cmd_ops op)
{
	struct handle h = {
		.table.location = *int_loc,
		.chain.location = *int_loc,
		.index.location = *int_loc,
	};
	json_t *tmp, *value;
	const char *family;
	struct rule *rule;
	size_t index;

	if (json_unpack_err(ctx, root, "{s:o}", "rule", &tmp))
		return NULL;
	root = tmp;

	if (json_unpack_err(ctx, root, "{s:s, s:s, s:s, s:o}",
			    "family", &family,
			    "table", &h.table.name,
			    "chain", &h.chain.name,
			    "expr", &tmp))
		return NULL;
	json_unpack(root, "{s:I}", "handle", &h.handle.id);
	if (!json_unpack(root, "{s:I}", "index", &h.index.id)) {
		h.index.id++;
	}

	if (op == CMD_REPLACE && !h.handle.id) {
		json_error(ctx, "Handle is required when replacing a rule.");
		return NULL;
	}

	if ((op == CMD_INSERT || op == CMD_ADD) && h.handle.id) {
		h.position.id = h.handle.id;
		h.handle.id = 0;
	}

	if (parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}

	if (!json_is_array(tmp)) {
		json_error(ctx, "Value of property \"expr\" must be an array.");
		return NULL;
	}

	h.table.name = xstrdup(h.table.name);
	h.chain.name = xstrdup(h.chain.name);

	rule = rule_alloc(int_loc, NULL);

	if (!json_unpack(root, "{s:s}", "comment", &rule->comment))
		rule->comment = xstrdup(rule->comment);

	json_array_foreach(tmp, index, value) {
		struct stmt *stmt;

		if (!json_is_object(value)) {
			json_error(ctx, "Unexpected expr array element of type %s, expected object.",
				   json_typename(value));
			goto err_free_replace;
		}

		stmt = json_parse_stmt(ctx, value);

		if (!stmt) {
			json_error(ctx, "Parsing expr array at index %zd failed.",
				   index);
			goto err_free_replace;
		}

		rule_stmt_append(rule, stmt);
	}

	if (op == CMD_REPLACE)
		json_object_del(root, "handle");

	return cmd_alloc(op, CMD_OBJ_RULE, &h, int_loc, rule);

err_free_replace:
	rule_free(rule);
	handle_free(&h);
	return NULL;
}

static struct cmd *json_parse_cmd_list_multiple(struct json_ctx *ctx,
						json_t *root, enum cmd_ops op,
						enum cmd_obj obj)
{
	struct handle h = {
		.family = NFPROTO_UNSPEC,
	};
	const char *tmp;

	if (!json_unpack(root, "{s:s}", "family", &tmp)) {
		if (parse_family(tmp, &h.family)) {
			json_error(ctx, "Unknown family '%s'.", tmp);
			return NULL;
		}
	}
	switch (obj) {
	case CMD_OBJ_SETS:
	case CMD_OBJ_COUNTERS:
	case CMD_OBJ_CT_HELPERS:
	case CMD_OBJ_TUNNELS:
		if (!json_unpack(root, "{s:s}", "table", &tmp))
			h.table.name = xstrdup(tmp);
		break;
	default:
		break;
	}
	if (obj == CMD_OBJ_CT_HELPERS && !h.table.name) {
		json_error(ctx, "Listing ct helpers requires table reference.");
		return NULL;
	}
	return cmd_alloc(op, obj, &h, int_loc, NULL);
}

static struct cmd *json_parse_cmd_list(struct json_ctx *ctx,
				       json_t *root, enum cmd_ops op)
{
	struct {
		const char *key;
		enum cmd_obj obj;
		struct cmd *(*cb)(struct json_ctx *, json_t *,
				  enum cmd_ops, enum cmd_obj);
	} cmd_obj_table[] = {
		{ "table", CMD_OBJ_TABLE, json_parse_cmd_add_table },
		{ "tables", CMD_OBJ_TABLE, json_parse_cmd_list_multiple },
		{ "chain", CMD_OBJ_CHAIN, json_parse_cmd_add_chain },
		{ "chains", CMD_OBJ_CHAINS, json_parse_cmd_list_multiple },
		{ "set", CMD_OBJ_SET, json_parse_cmd_add_set },
		{ "sets", CMD_OBJ_SETS, json_parse_cmd_list_multiple },
		{ "map", CMD_OBJ_MAP, json_parse_cmd_add_set },
		{ "maps", CMD_OBJ_MAPS, json_parse_cmd_list_multiple },
		{ "counter", CMD_OBJ_COUNTER, json_parse_cmd_add_object },
		{ "counters", CMD_OBJ_COUNTERS, json_parse_cmd_list_multiple },
		{ "quota", CMD_OBJ_QUOTA, json_parse_cmd_add_object },
		{ "quotas", CMD_OBJ_QUOTAS, json_parse_cmd_list_multiple },
		{ "ct helper", NFT_OBJECT_CT_HELPER, json_parse_cmd_add_object },
		{ "ct helpers", CMD_OBJ_CT_HELPERS, json_parse_cmd_list_multiple },
		{ "ct timeout", NFT_OBJECT_CT_TIMEOUT, json_parse_cmd_add_object },
		{ "ct expectation", NFT_OBJECT_CT_EXPECT, json_parse_cmd_add_object },
		{ "tunnel", NFT_OBJECT_TUNNEL, json_parse_cmd_add_object },
		{ "tunnels", CMD_OBJ_TUNNELS, json_parse_cmd_list_multiple },
		{ "limit", CMD_OBJ_LIMIT, json_parse_cmd_add_object },
		{ "limits", CMD_OBJ_LIMIT, json_parse_cmd_list_multiple },
		{ "ruleset", CMD_OBJ_RULESET, json_parse_cmd_list_multiple },
		{ "meter", CMD_OBJ_METER, json_parse_cmd_add_set },
		{ "meters", CMD_OBJ_METERS, json_parse_cmd_list_multiple },
		{ "flowtables", CMD_OBJ_FLOWTABLES, json_parse_cmd_list_multiple },
		{ "secmark", CMD_OBJ_SECMARK, json_parse_cmd_add_object },
		{ "secmarks", CMD_OBJ_SECMARKS, json_parse_cmd_list_multiple },
	};
	unsigned int i;
	json_t *tmp;

	if (!json_is_object(root)) {
		json_error(ctx, "Value of list command must be object (got %s instead).",
			   json_typename(root));
		return NULL;
	}

	for (i = 0; i < array_size(cmd_obj_table); i++) {
		tmp = json_object_get(root, cmd_obj_table[i].key);
		if (!tmp)
			continue;

		return cmd_obj_table[i].cb(ctx, tmp, op, cmd_obj_table[i].obj);
	}
	json_error(ctx, "Unknown object passed to list command.");
	return NULL;
}

static struct cmd *json_parse_cmd_reset_rule(struct json_ctx *ctx,
					     json_t *root, enum cmd_ops op,
					     enum cmd_obj obj)
{
	struct handle h = {
		.family = NFPROTO_UNSPEC,
	};
	const char *family = NULL, *table = NULL, *chain = NULL;


	if (obj == CMD_OBJ_RULE &&
	    json_unpack_err(ctx, root, "{s:s, s:s, s:s, s:I}",
			    "family", &family, "table", &table,
			    "chain", &chain, "handle", &h.handle.id))
		return NULL;
	else if (obj == CMD_OBJ_RULES) {
		json_unpack(root, "{s:s}", "family", &family);
		json_unpack(root, "{s:s}", "table", &table);
		json_unpack(root, "{s:s}", "chain", &chain);
	}

	if (family && parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}
	if (table) {
		h.table.name = xstrdup(table);
		if (chain)
			h.chain.name = xstrdup(chain);
	}
	return cmd_alloc(op, obj, &h, int_loc, NULL);
}

static struct cmd *json_parse_cmd_reset(struct json_ctx *ctx,
				        json_t *root, enum cmd_ops op)
{
	struct {
		const char *key;
		enum cmd_obj obj;
		struct cmd *(*cb)(struct json_ctx *, json_t *,
				  enum cmd_ops, enum cmd_obj);
	} cmd_obj_table[] = {
		{ "counter", CMD_OBJ_COUNTER, json_parse_cmd_add_object },
		{ "counters", CMD_OBJ_COUNTERS, json_parse_cmd_list_multiple },
		{ "quota", CMD_OBJ_QUOTA, json_parse_cmd_add_object },
		{ "quotas", CMD_OBJ_QUOTAS, json_parse_cmd_list_multiple },
		{ "rule", CMD_OBJ_RULE, json_parse_cmd_reset_rule },
		{ "rules", CMD_OBJ_RULES, json_parse_cmd_reset_rule },
		{ "element", CMD_OBJ_ELEMENTS, json_parse_cmd_add_element },
		{ "set", CMD_OBJ_SET, json_parse_cmd_add_set },
		{ "map", CMD_OBJ_MAP, json_parse_cmd_add_set },
	};
	unsigned int i;
	json_t *tmp;

	if (!json_is_object(root)) {
		json_error(ctx, "Value of reset command must be object (got %s instead).",
			   json_typename(root));
		return NULL;
	}

	for (i = 0; i < array_size(cmd_obj_table); i++) {
		tmp = json_object_get(root, cmd_obj_table[i].key);
		if (!tmp)
			continue;

		return cmd_obj_table[i].cb(ctx, tmp, op, cmd_obj_table[i].obj);
	}
	json_error(ctx, "Unknown object passed to reset command.");
	return NULL;
}

static struct cmd *json_parse_cmd_flush(struct json_ctx *ctx,
				        json_t *root, enum cmd_ops op)
{
	struct {
		const char *key;
		enum cmd_obj obj;
		struct cmd *(*cb)(struct json_ctx *, json_t *,
				  enum cmd_ops, enum cmd_obj);
	} cmd_obj_table[] = {
		{ "table", CMD_OBJ_TABLE, json_parse_cmd_add_table },
		{ "chain", CMD_OBJ_CHAIN, json_parse_cmd_add_chain },
		{ "set", CMD_OBJ_SET, json_parse_cmd_add_set },
		{ "map", CMD_OBJ_MAP, json_parse_cmd_add_set },
		{ "meter", CMD_OBJ_METER, json_parse_cmd_add_set },
		{ "ruleset", CMD_OBJ_RULESET, json_parse_cmd_list_multiple },
	};
	unsigned int i;
	json_t *tmp;

	if (!json_is_object(root)) {
		json_error(ctx, "Value of flush command must be object (got %s instead).",
			   json_typename(root));
		return NULL;
	}

	for (i = 0; i < array_size(cmd_obj_table); i++) {
		tmp = json_object_get(root, cmd_obj_table[i].key);
		if (!tmp)
			continue;

		return cmd_obj_table[i].cb(ctx, tmp, op, cmd_obj_table[i].obj);
	}
	json_error(ctx, "Unknown object passed to flush command.");
	return NULL;
}

static struct cmd *json_parse_cmd_rename(struct json_ctx *ctx,
				         json_t *root, enum cmd_ops op)
{
	const char *family, *newname;
	struct handle h = { 0 };
	struct cmd *cmd;

	if (json_unpack_err(ctx, root, "{s:{s:s, s:s, s:s, s:s}}", "chain",
			    "family", &family,
			    "table", &h.table.name,
			    "name", &h.chain.name,
			    "newname", &newname))
		return NULL;
	if (parse_family(family, &h.family)) {
		json_error(ctx, "Unknown family '%s'.", family);
		return NULL;
	}
	h.table.name = xstrdup(h.table.name);
	h.chain.name = xstrdup(h.chain.name);

	cmd = cmd_alloc(op, CMD_OBJ_CHAIN, &h, int_loc, NULL);
	cmd->arg = xstrdup(newname);
	return cmd;
}

static struct cmd *json_parse_cmd(struct json_ctx *ctx, json_t *root)
{
	struct {
		const char *key;
		enum cmd_ops op;
		struct cmd *(*cb)(struct json_ctx *ctx, json_t *, enum cmd_ops);
	} parse_cb_table[] = {
		{ "add", CMD_ADD, json_parse_cmd_add },
		{ "replace", CMD_REPLACE, json_parse_cmd_replace },
		{ "create", CMD_CREATE, json_parse_cmd_add },
		{ "insert", CMD_INSERT, json_parse_cmd_replace },
		{ "delete", CMD_DELETE, json_parse_cmd_add },
		{ "list", CMD_LIST, json_parse_cmd_list },
		{ "reset", CMD_RESET, json_parse_cmd_reset },
		{ "flush", CMD_FLUSH, json_parse_cmd_flush },
		{ "rename", CMD_RENAME, json_parse_cmd_rename },
		{ "destroy", CMD_DESTROY, json_parse_cmd_add },
		//{ "export", CMD_EXPORT, json_parse_cmd_export },
		//{ "monitor", CMD_MONITOR, json_parse_cmd_monitor },
		//{ "describe", CMD_DESCRIBE, json_parse_cmd_describe }
	};
	unsigned int i;
	json_t *tmp;

	for (i = 0; i < array_size(parse_cb_table); i++) {
		tmp = json_object_get(root, parse_cb_table[i].key);
		if (!tmp)
			continue;

		return parse_cb_table[i].cb(ctx, tmp, parse_cb_table[i].op);
	}
	/* to accept 'list ruleset' output 1:1, try add command */
	return json_parse_cmd_add(ctx, root, CMD_ADD);
}

static int json_verify_metainfo(struct json_ctx *ctx, json_t *root)
{
	int schema_version;

	if (!json_unpack(root, "{s:i}", "json_schema_version", &schema_version)) {
		if (schema_version > JSON_SCHEMA_VERSION) {
			json_error(ctx,
				   "Schema version %d not supported, maximum"
			           " supported version is %d\n",
				   schema_version, JSON_SCHEMA_VERSION);
			return 1;
		}
	}

	return 0;
}

struct json_cmd_assoc {
	struct json_cmd_assoc *next;
	struct hlist_node hnode;
	const struct cmd *cmd;
	json_t *json;
};

#define CMD_ASSOC_HSIZE		512
static struct hlist_head json_cmd_assoc_hash[CMD_ASSOC_HSIZE];
static struct json_cmd_assoc *json_cmd_assoc_list;

static void json_cmd_assoc_free(void)
{
	struct json_cmd_assoc *cur;
	struct hlist_node *pos, *n;
	int i;

	while (json_cmd_assoc_list) {
		cur = json_cmd_assoc_list->next;
		free(json_cmd_assoc_list);
		json_cmd_assoc_list = cur;
	}

	for (i = 0; i < CMD_ASSOC_HSIZE; i++) {
		hlist_for_each_entry_safe(cur, pos, n,
					  &json_cmd_assoc_hash[i], hnode) {
			hlist_del(&cur->hnode);
			free(cur);
		}
	}
}

static void json_cmd_assoc_add(json_t *json, const struct cmd *cmd)
{
	struct json_cmd_assoc *new = xzalloc(sizeof *new);

	new->json	= json;
	new->cmd	= cmd;
	new->next	= json_cmd_assoc_list;

	json_cmd_assoc_list = new;
}

static json_t *seqnum_to_json(const uint32_t seqnum)
{
	struct json_cmd_assoc *cur;
	struct hlist_node *n;
	int key;

	while (json_cmd_assoc_list) {
		cur = json_cmd_assoc_list;
		json_cmd_assoc_list = cur->next;

		key = cur->cmd->seqnum_from % CMD_ASSOC_HSIZE;
		hlist_add_head(&cur->hnode, &json_cmd_assoc_hash[key]);
	}

	key = seqnum % CMD_ASSOC_HSIZE;
	hlist_for_each_entry(cur, n, &json_cmd_assoc_hash[key], hnode) {
		if (cur->cmd->seqnum_from == seqnum)
			return cur->json;
	}

	return NULL;
}

static int __json_parse(struct json_ctx *ctx)
{
	json_t *tmp, *value;
	size_t index;

	if (json_unpack_err(ctx, ctx->nft->json_root,
			    "{s:o}", "nftables", &tmp))
		return -1;

	if (!json_is_array(tmp)) {
		json_error(ctx, "Value of property \"nftables\" must be an array.");
		return -1;
	}

	json_array_foreach(tmp, index, value) {
		/* this is more or less from parser_bison.y:716 */
		LIST_HEAD(list);
		struct cmd *cmd;
		json_t *tmp2;

		if (!json_is_object(value)) {
			json_error(ctx, "Unexpected command array element of type %s, expected object.", json_typename(value));
			return -1;
		}

		tmp2 = json_object_get(value, "metainfo");
		if (tmp2) {
			if (json_verify_metainfo(ctx, tmp2)) {
				json_error(ctx, "Metainfo verification failed.");
				return -1;
			}
			continue;
		}

		cmd = json_parse_cmd(ctx, value);

		if (!cmd) {
			if (ctx->flags & CTX_F_COLLAPSED) {
				ctx->flags &= ~CTX_F_COLLAPSED;
				continue;
			}

			json_error(ctx, "Parsing command array at index %zd failed.", index);
			return -1;
		}

		list_add_tail(&cmd->list, &list);

		list_splice_tail(&list, ctx->cmds);

		if (nft_output_echo(&ctx->nft->output))
			json_cmd_assoc_add(value, cmd);
	}

	return 0;
}

int nft_parse_json_buffer(struct nft_ctx *nft, const char *buf,
			  struct list_head *msgs, struct list_head *cmds)
{
	struct json_ctx ctx = {
		.nft = nft,
		.msgs = msgs,
		.cmds = cmds,
	};
	int ret;

	json_indesc.type = INDESC_BUFFER;
	json_indesc.data = buf;

	parser_init(nft, nft->state, msgs, cmds, nft->top_scope);
	nft->json_root = json_loads(buf, 0, NULL);
	if (!nft->json_root)
		return -EINVAL;

	ret = __json_parse(&ctx);

	if (!nft_output_echo(&nft->output)) {
		json_decref(nft->json_root);
		nft->json_root = NULL;
	}
	return ret;
}

int nft_parse_json_filename(struct nft_ctx *nft, const char *filename,
			    struct list_head *msgs, struct list_head *cmds)
{
	struct json_ctx ctx = {
		.nft = nft,
		.msgs = msgs,
		.cmds = cmds,
	};
	json_error_t err;
	int ret;

	if (nft->stdin_buf) {
		json_indesc.type = INDESC_STDIN;
		json_indesc.name = "/dev/stdin";

		return nft_parse_json_buffer(nft, nft->stdin_buf, msgs, cmds);
	}

	json_indesc.type = INDESC_FILE;
	json_indesc.name = filename;

	parser_init(nft, nft->state, msgs, cmds, nft->top_scope);
	nft->json_root = json_load_file(filename, 0, &err);
	if (!nft->json_root)
		return -EINVAL;

	ret = __json_parse(&ctx);

	if (!nft_output_echo(&nft->output)) {
		json_decref(nft->json_root);
		nft->json_root = NULL;
	}
	return ret;
}

__attribute__((format(printf, 2, 3)))
static int json_echo_error(struct netlink_mon_handler *monh,
			   const char *fmt, ...)
{
	struct error_record *erec;
	va_list ap;

	va_start(ap, fmt);
	erec = erec_vcreate(EREC_ERROR, int_loc, fmt, ap);
	va_end(ap);
	erec_queue(erec, monh->ctx->msgs);

	return MNL_CB_ERROR;
}

static uint64_t handle_from_nlmsg(const struct nlmsghdr *nlh)
{
	struct nftnl_flowtable *nlf;
	struct nftnl_table *nlt;
	struct nftnl_chain *nlc;
	struct nftnl_rule *nlr;
	struct nftnl_set *nls;
	struct nftnl_obj *nlo;
	uint64_t handle = 0;
	uint32_t flags;

	switch (NFNL_MSG_TYPE(nlh->nlmsg_type)) {
	case NFT_MSG_NEWTABLE:
		nlt = netlink_table_alloc(nlh);
		handle = nftnl_table_get_u64(nlt, NFTNL_TABLE_HANDLE);
		nftnl_table_free(nlt);
		break;
	case NFT_MSG_NEWCHAIN:
		nlc = netlink_chain_alloc(nlh);
		handle = nftnl_chain_get_u64(nlc, NFTNL_CHAIN_HANDLE);
		nftnl_chain_free(nlc);
		break;
	case NFT_MSG_NEWRULE:
		nlr = netlink_rule_alloc(nlh);
		handle = nftnl_rule_get_u64(nlr, NFTNL_RULE_HANDLE);
		nftnl_rule_free(nlr);
		break;
	case NFT_MSG_NEWSET:
		nls = netlink_set_alloc(nlh);
		flags = nftnl_set_get_u32(nls, NFTNL_SET_FLAGS);
		if (!set_is_anonymous(flags))
			handle = nftnl_set_get_u64(nls, NFTNL_SET_HANDLE);
		nftnl_set_free(nls);
		break;
	case NFT_MSG_NEWOBJ:
		nlo = netlink_obj_alloc(nlh);
		handle = nftnl_obj_get_u64(nlo, NFTNL_OBJ_HANDLE);
		nftnl_obj_free(nlo);
		break;
	case NFT_MSG_NEWFLOWTABLE:
		nlf = netlink_flowtable_alloc(nlh);
		handle = nftnl_flowtable_get_u64(nlf, NFTNL_FLOWTABLE_HANDLE);
		nftnl_flowtable_free(nlf);
		break;
	}
	return handle;
}
int json_events_cb(const struct nlmsghdr *nlh, struct netlink_mon_handler *monh)
{
	uint64_t handle = handle_from_nlmsg(nlh);
	json_t *tmp, *json;
	void *iter;

	if (!handle)
		return MNL_CB_OK;

	json = seqnum_to_json(nlh->nlmsg_seq);
	if (!json) {
		json_echo_error(monh, "No JSON command found with seqnum %u\n",
				nlh->nlmsg_seq);
		return MNL_CB_OK;
	}

	tmp = json_object_get(json, "add");
	if (!tmp)
		tmp = json_object_get(json, "insert");
	if (!tmp)
		/* assume loading JSON dump */
		tmp = json;

	iter = json_object_iter(tmp);
	if (!iter) {
		json_echo_error(monh, "Empty JSON object in cmd list\n");
		return MNL_CB_OK;
	}
	json = json_object_iter_value(iter);
	if (!json_is_object(json) || json_object_iter_next(tmp, iter)) {
		json_echo_error(monh, "Malformed JSON object in cmd list\n");
		return MNL_CB_OK;
	}

	json_object_set_new(json, "handle", json_integer(handle));
	return MNL_CB_OK;
}

void json_print_echo(struct nft_ctx *ctx)
{
	if (!ctx->json_root) {
		if (!ctx->json_echo)
			return;

		ctx->json_echo = json_pack("{s:o}", "nftables", ctx->json_echo);
		json_dumpf(ctx->json_echo, ctx->output.output_fp, JSON_PRESERVE_ORDER);
		json_decref(ctx->json_echo);
		ctx->json_echo = NULL;
		fprintf(ctx->output.output_fp, "\n");
		fflush(ctx->output.output_fp);
	} else {
		json_dumpf(ctx->json_root, ctx->output.output_fp, JSON_PRESERVE_ORDER);
		json_cmd_assoc_free();
		json_decref(ctx->json_root);
		ctx->json_root = NULL;
	}
}
