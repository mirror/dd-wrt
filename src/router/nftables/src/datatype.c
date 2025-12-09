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

#include <inttypes.h>
#include <ctype.h> /* isdigit */
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/icmpv6.h>
#include <dirent.h>
#include <sys/stat.h>

#include <nftables.h>
#include <datatype.h>
#include <expression.h>
#include <gmputil.h>
#include <erec.h>
#include <netlink.h>
#include <json.h>
#include <misspell.h>
#include "nftutils.h"

#include <netinet/ip_icmp.h>

static const struct datatype *datatypes[TYPE_MAX + 1] = {
	[TYPE_INVALID]		= &invalid_type,
	[TYPE_VERDICT]		= &verdict_type,
	[TYPE_NFPROTO]		= &nfproto_type,
	[TYPE_BITMASK]		= &bitmask_type,
	[TYPE_INTEGER]		= &integer_type,
	[TYPE_STRING]		= &string_type,
	[TYPE_LLADDR]		= &lladdr_type,
	[TYPE_IPADDR]		= &ipaddr_type,
	[TYPE_IP6ADDR]		= &ip6addr_type,
	[TYPE_ETHERADDR]	= &etheraddr_type,
	[TYPE_ETHERTYPE]	= &ethertype_type,
	[TYPE_ARPOP]		= &arpop_type,
	[TYPE_INET_PROTOCOL]	= &inet_protocol_type,
	[TYPE_INET_SERVICE]	= &inet_service_type,
	[TYPE_ICMP_TYPE]	= &icmp_type_type,
	[TYPE_TCP_FLAG]		= &tcp_flag_type,
	[TYPE_DCCP_PKTTYPE]	= &dccp_pkttype_type,
	[TYPE_MH_TYPE]		= &mh_type_type,
	[TYPE_TIME]		= &time_type,
	[TYPE_MARK]		= &mark_type,
	[TYPE_IFINDEX]		= &ifindex_type,
	[TYPE_ARPHRD]		= &arphrd_type,
	[TYPE_REALM]		= &realm_type,
	[TYPE_CLASSID]		= &tchandle_type,
	[TYPE_UID]		= &uid_type,
	[TYPE_GID]		= &gid_type,
	[TYPE_CT_STATE]		= &ct_state_type,
	[TYPE_CT_DIR]		= &ct_dir_type,
	[TYPE_CT_STATUS]	= &ct_status_type,
	[TYPE_ICMP6_TYPE]	= &icmp6_type_type,
	[TYPE_CT_LABEL]		= &ct_label_type,
	[TYPE_PKTTYPE]		= &pkttype_type,
	[TYPE_ICMP_CODE]	= &icmp_code_type,
	[TYPE_ICMPV6_CODE]	= &icmpv6_code_type,
	[TYPE_ICMPX_CODE]	= &icmpx_code_type,
	[TYPE_DEVGROUP]		= &devgroup_type,
	[TYPE_DSCP]		= &dscp_type,
	[TYPE_ECN]		= &ecn_type,
	[TYPE_FIB_ADDR]         = &fib_addr_type,
	[TYPE_BOOLEAN]		= &boolean_type,
	[TYPE_CT_EVENTBIT]	= &ct_event_type,
	[TYPE_IFNAME]		= &ifname_type,
	[TYPE_IGMP_TYPE]	= &igmp_type_type,
	[TYPE_TIME_DATE]	= &date_type,
	[TYPE_TIME_HOUR]	= &hour_type,
	[TYPE_TIME_DAY]		= &day_type,
	[TYPE_CGROUPV2]		= &cgroupv2_type,
};

const struct datatype *datatype_lookup(enum datatypes type)
{
	BUILD_BUG_ON(TYPE_MAX & ~TYPE_MASK);

	if (type > TYPE_MAX)
		return NULL;
	return datatypes[type];
}

const struct datatype *datatype_lookup_byname(const char *name)
{
	const struct datatype *dtype;
	enum datatypes type;

	for (type = TYPE_INVALID; type <= TYPE_MAX; type++) {
		dtype = datatypes[type];
		if (dtype == NULL)
			continue;
		if (!strcmp(dtype->name, name))
			return dtype;
	}
	return NULL;
}

void datatype_print(const struct expr *expr, struct output_ctx *octx)
{
	const struct datatype *dtype = expr->dtype;

	do {
		if (dtype->print != NULL)
			return dtype->print(expr, octx);
		if (dtype->sym_tbl != NULL)
			return symbolic_constant_print(dtype->sym_tbl, expr,
						       false, octx);
	} while ((dtype = dtype->basetype));

	BUG("datatype %s has no print method or symbol table",
	    expr->dtype->name);
}

bool datatype_prefix_notation(const struct datatype *dtype)
{
	return dtype->type == TYPE_IPADDR || dtype->type == TYPE_IP6ADDR;
}

struct error_record *symbol_parse(struct parse_ctx *ctx, const struct expr *sym,
				  struct expr **res)
{
	const struct datatype *dtype = sym->dtype;
	struct error_record *erec;

	assert(sym->etype == EXPR_SYMBOL);

	if (dtype == NULL)
		return error(&sym->location, "No symbol type information");
	do {
		if (dtype->parse != NULL)
			return dtype->parse(ctx, sym, res);
		if (dtype->sym_tbl != NULL)
			return symbolic_constant_parse(ctx, sym, dtype->sym_tbl,
						       res);
	} while ((dtype = dtype->basetype));

	dtype = sym->dtype;
	if (dtype->err) {
		erec = dtype->err(sym);
		if (erec)
			return erec;
	}

	return error(&sym->location, "Could not parse symbolic %s expression",
		     sym->dtype->desc);
}

static struct error_record *__symbol_parse_fuzzy(const struct expr *sym,
						 const struct symbol_table *tbl)
{
	const struct symbolic_constant *s;
	struct string_misspell_state st;

	string_misspell_init(&st);

	for (s = tbl->symbols; s->identifier != NULL; s++) {
		string_misspell_update(sym->identifier, s->identifier,
				       (void *)s->identifier, &st);
	}

	if (st.obj) {
		return error(&sym->location,
			     "Could not parse %s expression; did you you mean `%s`?",
			     sym->dtype->desc, (const char *)st.obj);
	}

	return NULL;
}

static struct error_record *symbol_parse_fuzzy(const struct expr *sym,
					       const struct symbol_table *tbl)
{
	struct error_record *erec;

	if (!tbl)
		return NULL;

	erec = __symbol_parse_fuzzy(sym, tbl);
	if (erec)
		return erec;

	return NULL;
}

struct error_record *symbolic_constant_parse(struct parse_ctx *ctx,
					     const struct expr *sym,
					     const struct symbol_table *tbl,
					     struct expr **res)
{
	const struct symbolic_constant *s;
	const struct datatype *dtype;
	struct error_record *erec;

	for (s = tbl->symbols; s->identifier != NULL; s++) {
		if (!strcmp(sym->identifier, s->identifier))
			break;
	}

	if (s->identifier != NULL)
		goto out;

	dtype = sym->dtype;
	*res = NULL;
	do {
		if (dtype->basetype->parse) {
			erec = dtype->basetype->parse(ctx, sym, res);
			if (erec != NULL) {
				struct error_record *fuzzy_erec;

				fuzzy_erec = symbol_parse_fuzzy(sym, tbl);
				if (!fuzzy_erec)
					return erec;

				erec_destroy(erec);
				return fuzzy_erec;
			}
			if (*res)
				return NULL;
			goto out;
		}
	} while ((dtype = dtype->basetype));

	return error(&sym->location, "Could not parse %s", sym->dtype->desc);
out:
	*res = constant_expr_alloc(&sym->location, sym->dtype,
				   sym->dtype->byteorder, sym->dtype->size,
				   constant_data_ptr(s->value,
				   sym->dtype->size));
	return NULL;
}

void symbolic_constant_print(const struct symbol_table *tbl,
			     const struct expr *expr, bool quotes,
			     struct output_ctx *octx)
{
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE);
	const struct symbolic_constant *s;
	uint64_t val = 0;

	/* Export the data in the correct byteorder for comparison */
	assert(expr->len / BITS_PER_BYTE <= sizeof(val));
	mpz_export_data(constant_data_ptr(val, expr->len), expr->value,
			expr->byteorder, len);

	if (nft_output_numeric_symbol(octx) || !tbl)
		goto basetype_print;

	for (s = tbl->symbols; s->identifier != NULL; s++) {
		if (val == s->value)
			break;
	}
	if (s->identifier) {
		nft_print(octx, quotes ? "\"%s\"" : "%s", s->identifier);
		return;
	}
basetype_print:
	expr_basetype(expr)->print(expr, octx);
}

static void switch_byteorder(void *data, unsigned int len)
{
	mpz_t op;

	mpz_init(op);
	mpz_import_data(op, data, BYTEORDER_BIG_ENDIAN, len);
	mpz_export_data(data, op, BYTEORDER_HOST_ENDIAN, len);
	mpz_clear(op);
}

void symbol_table_print(const struct symbol_table *tbl,
			const struct datatype *dtype,
			enum byteorder byteorder,
			struct output_ctx *octx)
{
	unsigned int len = div_round_up(dtype->size, BITS_PER_BYTE);
	const struct symbolic_constant *s;
	uint64_t value;

	for (s = tbl->symbols; s->identifier != NULL; s++) {
		value = s->value;

		if (byteorder == BYTEORDER_BIG_ENDIAN)
			switch_byteorder(&value, len);

		if (tbl->base == BASE_DECIMAL)
			nft_print(octx, "\t%-30s\t%20" PRIu64 "\n",
				  s->identifier, value);
		else
			nft_print(octx, "\t%-30s\t0x%.*" PRIx64 "\n",
				  s->identifier, 2 * len, value);
	}
}

static void invalid_type_print(const struct expr *expr, struct output_ctx *octx)
{
	nft_gmp_print(octx, "0x%Zx [invalid type]", expr->value);
}

const struct datatype invalid_type = {
	.type		= TYPE_INVALID,
	.name		= "invalid",
	.desc		= "invalid",
	.print		= invalid_type_print,
};

void expr_chain_export(const struct expr *e, char *chain_name)
{
	unsigned int len;

	len = e->len / BITS_PER_BYTE;
	if (len >= NFT_CHAIN_MAXNAMELEN)
		BUG("verdict expression length %u is too large (%u bits max)",
		    e->len, NFT_CHAIN_MAXNAMELEN * BITS_PER_BYTE);

	mpz_export_data(chain_name, e->value, BYTEORDER_HOST_ENDIAN, len);
}

static void verdict_jump_chain_print(const char *what, const struct expr *e,
				     struct output_ctx *octx)
{
	char chain[NFT_CHAIN_MAXNAMELEN];

	memset(chain, 0, sizeof(chain));
	expr_chain_export(e, chain);
	nft_print(octx, "%s %s", what, chain);
}

static void verdict_type_print(const struct expr *expr, struct output_ctx *octx)
{
	switch (expr->verdict) {
	case NFT_CONTINUE:
		nft_print(octx, "continue");
		break;
	case NFT_BREAK:
		nft_print(octx, "break");
		break;
	case NFT_JUMP:
		if (expr->chain->etype == EXPR_VALUE) {
			verdict_jump_chain_print("jump", expr->chain, octx);
		} else {
			nft_print(octx, "jump ");
			expr_print(expr->chain, octx);
		}
		break;
	case NFT_GOTO:
		if (expr->chain->etype == EXPR_VALUE) {
			verdict_jump_chain_print("goto", expr->chain, octx);
		} else {
			nft_print(octx, "goto ");
			expr_print(expr->chain, octx);
		}
		break;
	case NFT_RETURN:
		nft_print(octx, "return");
		break;
	default:
		switch (expr->verdict & NF_VERDICT_MASK) {
		case NF_ACCEPT:
			nft_print(octx, "accept");
			break;
		case NF_DROP:
			nft_print(octx, "drop");
			break;
		case NF_QUEUE:
			nft_print(octx, "queue");
			break;
		case NF_STOLEN:
			nft_print(octx, "stolen");
			break;
		default:
			nft_print(octx, "unknown verdict value %u", expr->verdict);
			break;
		}
	}
}

static struct error_record *verdict_type_error(const struct expr *sym)
{
	/* Skip jump and goto from fuzzy match to provide better error
	 * reporting, fall back to `jump chain' if no clue.
	 */
	static const char *verdict_array[] = {
		"continue", "break", "return", "accept", "drop", "queue",
		"stolen", NULL,
	};
	struct string_misspell_state st;
	int i;

	string_misspell_init(&st);

	for (i = 0; verdict_array[i] != NULL; i++) {
		string_misspell_update(sym->identifier, verdict_array[i],
				       (void *)verdict_array[i], &st);
	}

	if (st.obj) {
		return error(&sym->location, "Could not parse %s; did you mean `%s'?",
			     sym->dtype->desc, (const char *)st.obj);
	}

	/* assume user would like to jump to chain as a hint. */
	return error(&sym->location, "Could not parse %s; did you mean `jump %s'?",
		     sym->dtype->desc, sym->identifier);
}

const struct datatype verdict_type = {
	.type		= TYPE_VERDICT,
	.name		= "verdict",
	.desc		= "netfilter verdict",
	.print		= verdict_type_print,
	.err		= verdict_type_error,
};

static const struct symbol_table nfproto_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("ipv4",		NFPROTO_IPV4),
		SYMBOL("ipv6",		NFPROTO_IPV6),
		SYMBOL_LIST_END
	},
};

const struct datatype nfproto_type = {
	.type		= TYPE_NFPROTO,
	.name		= "nf_proto",
	.desc		= "netfilter protocol",
	.size		= 1 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.sym_tbl	= &nfproto_tbl,
};

const struct datatype bitmask_type = {
	.type		= TYPE_BITMASK,
	.name		= "bitmask",
	.desc		= "bitmask",
	.basefmt	= "0x%Zx",
	.basetype	= &integer_type,
};

static void integer_type_print(const struct expr *expr, struct output_ctx *octx)
{
	const struct datatype *dtype = expr->dtype;
	const char *fmt = "%Zu";

	do {
		if (dtype->basefmt != NULL) {
			fmt = dtype->basefmt;
			break;
		}
	} while ((dtype = dtype->basetype));

	nft_gmp_print(octx, fmt, expr->value);
}

static struct error_record *integer_type_parse(struct parse_ctx *ctx,
					       const struct expr *sym,
					       struct expr **res)
{
	mpz_t v;

	mpz_init(v);
	if (mpz_set_str(v, sym->identifier, 0)) {
		mpz_clear(v);
		return error(&sym->location, "Could not parse %s",
			     sym->dtype->desc);
	}

	*res = constant_expr_alloc(&sym->location, sym->dtype,
				   BYTEORDER_HOST_ENDIAN, 1, NULL);
	mpz_set((*res)->value, v);
	mpz_clear(v);
	return NULL;
}

const struct datatype integer_type = {
	.type		= TYPE_INTEGER,
	.name		= "integer",
	.desc		= "integer",
	.print		= integer_type_print,
	.json		= integer_type_json,
	.parse		= integer_type_parse,
};

static void xinteger_type_print(const struct expr *expr, struct output_ctx *octx)
{
	nft_gmp_print(octx, "0x%Zx", expr->value);
}

/* Alias of integer_type to print raw payload expressions in hexadecimal. */
const struct datatype xinteger_type = {
	.type		= TYPE_INTEGER,
	.name		= "integer",
	.desc		= "integer",
	.basetype	= &integer_type,
	.print		= xinteger_type_print,
	.json		= integer_type_json,
	.parse		= integer_type_parse,
};

static void queue_type_print(const struct expr *expr, struct output_ctx *octx)
{
	nft_gmp_print(octx, "queue");
}

/* Dummy queue_type for set declaration with typeof, see
 * constant_expr_build_udata and constant_expr_parse_udata,
 * basetype is used for elements.
*/
const struct datatype queue_type = {
	.type		= TYPE_INTEGER,
	.is_typeof	= 1,
	.name		= "queue",
	.desc		= "queue",
	.basetype	= &integer_type,
	.print		= queue_type_print,
};

static void string_type_print(const struct expr *expr, struct output_ctx *octx)
{
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE);
	char data[len+1];

	mpz_export_data(data, expr->value, BYTEORDER_HOST_ENDIAN, len);
	data[len] = '\0';
	nft_print(octx, "\"%s\"", data);
}

static struct error_record *string_type_parse(struct parse_ctx *ctx,
					      const struct expr *sym,
	      				      struct expr **res)
{
	*res = constant_expr_alloc(&sym->location, &string_type,
				   BYTEORDER_HOST_ENDIAN,
				   (strlen(sym->identifier) + 1) * BITS_PER_BYTE,
				   sym->identifier);
	return NULL;
}

const struct datatype string_type = {
	.type		= TYPE_STRING,
	.name		= "string",
	.desc		= "string",
	.byteorder	= BYTEORDER_HOST_ENDIAN,
	.print		= string_type_print,
	.json		= string_type_json,
	.parse		= string_type_parse,
};

static void lladdr_type_print(const struct expr *expr, struct output_ctx *octx)
{
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE);
	const char *delim = "";
	uint8_t data[len];
	unsigned int i;

	mpz_export_data(data, expr->value, BYTEORDER_BIG_ENDIAN, len);

	for (i = 0; i < len; i++) {
		nft_print(octx, "%s%.2x", delim, data[i]);
		delim = ":";
	}
}

static struct error_record *lladdr_type_parse(struct parse_ctx *ctx,
					      const struct expr *sym,
					      struct expr **res)
{
	char buf[strlen(sym->identifier) + 1], *p;
	const char *s = sym->identifier;
	unsigned int len, n;

	for (len = 0;;) {
		n = strtoul(s, &p, 16);
		if (s == p || n > 0xff)
			return erec_create(EREC_ERROR, &sym->location,
					   "Invalid LL address");
		buf[len++] = n;
		if (*p == '\0')
			break;
		s = ++p;
	}

	*res = constant_expr_alloc(&sym->location, sym->dtype,
				   BYTEORDER_BIG_ENDIAN, len * BITS_PER_BYTE,
				   buf);
	return NULL;
}

const struct datatype lladdr_type = {
	.type		= TYPE_LLADDR,
	.name		= "ll_addr",
	.desc		= "link layer address",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.basetype	= &integer_type,
	.print		= lladdr_type_print,
	.parse		= lladdr_type_parse,
};

static void ipaddr_type_print(const struct expr *expr, struct output_ctx *octx)
{
	struct sockaddr_in sin = { .sin_family = AF_INET, };
	char buf[NI_MAXHOST];
	int err;

	sin.sin_addr.s_addr = mpz_get_be32(expr->value);
	err = getnameinfo((struct sockaddr *)&sin, sizeof(sin), buf,
			  sizeof(buf), NULL, 0,
			  nft_output_reversedns(octx) ? 0 : NI_NUMERICHOST);
	if (err != 0) {
		getnameinfo((struct sockaddr *)&sin, sizeof(sin), buf,
			    sizeof(buf), NULL, 0, NI_NUMERICHOST);
	}
	nft_print(octx, "%s", buf);
}

static struct error_record *ipaddr_type_parse(struct parse_ctx *ctx,
					      const struct expr *sym,
					      struct expr **res)
{
	struct in_addr addr;

	if (nft_input_no_dns(ctx->input)) {
		if (inet_pton(AF_INET, sym->identifier, &addr) != 1)
			return error(&sym->location, "Invalid IPv4 address");
	} else {
		struct addrinfo *ai, hints = { .ai_family = AF_INET,
					       .ai_socktype = SOCK_DGRAM};
		int err;

		err = getaddrinfo(sym->identifier, NULL, &hints, &ai);
		if (err != 0)
			return error(&sym->location, "Could not resolve hostname: %s",
				     gai_strerror(err));

		if (ai->ai_next != NULL) {
			freeaddrinfo(ai);
			return error(&sym->location,
				     "Hostname resolves to multiple addresses");
		}
		assert(ai->ai_addr->sa_family == AF_INET);
		addr = ((struct sockaddr_in *) (void *) ai->ai_addr)->sin_addr;
		freeaddrinfo(ai);
	}

	*res = constant_expr_alloc(&sym->location, &ipaddr_type,
				   BYTEORDER_BIG_ENDIAN,
				   sizeof(addr) * BITS_PER_BYTE, &addr);
	return NULL;
}

const struct datatype ipaddr_type = {
	.type		= TYPE_IPADDR,
	.name		= "ipv4_addr",
	.desc		= "IPv4 address",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= 4 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.print		= ipaddr_type_print,
	.parse		= ipaddr_type_parse,
};

static void ip6addr_type_print(const struct expr *expr, struct output_ctx *octx)
{
	struct sockaddr_in6 sin6 = { .sin6_family = AF_INET6 };
	char buf[NI_MAXHOST];
	int err;

	mpz_export_data(&sin6.sin6_addr, expr->value, BYTEORDER_BIG_ENDIAN,
			sizeof(sin6.sin6_addr));

	err = getnameinfo((struct sockaddr *)&sin6, sizeof(sin6), buf,
			  sizeof(buf), NULL, 0,
			  nft_output_reversedns(octx) ? 0 : NI_NUMERICHOST);
	if (err != 0) {
		getnameinfo((struct sockaddr *)&sin6, sizeof(sin6), buf,
			    sizeof(buf), NULL, 0, NI_NUMERICHOST);
	}
	nft_print(octx, "%s", buf);
}

static struct error_record *ip6addr_type_parse(struct parse_ctx *ctx,
					       const struct expr *sym,
					       struct expr **res)
{
	struct in6_addr addr;

	if (nft_input_no_dns(ctx->input)) {
		if (inet_pton(AF_INET6, sym->identifier, &addr) != 1)
			return error(&sym->location, "Invalid IPv6 address");
	} else {
		struct addrinfo *ai, hints = { .ai_family = AF_INET6,
					       .ai_socktype = SOCK_DGRAM};
		int err;

		err = getaddrinfo(sym->identifier, NULL, &hints, &ai);
		if (err != 0)
			return error(&sym->location, "Could not resolve hostname: %s",
				     gai_strerror(err));

		if (ai->ai_next != NULL) {
			freeaddrinfo(ai);
			return error(&sym->location,
				     "Hostname resolves to multiple addresses");
		}

		assert(ai->ai_addr->sa_family == AF_INET6);
		addr = ((struct sockaddr_in6 *)(void *)ai->ai_addr)->sin6_addr;
		freeaddrinfo(ai);
	}

	*res = constant_expr_alloc(&sym->location, &ip6addr_type,
				   BYTEORDER_BIG_ENDIAN,
				   sizeof(addr) * BITS_PER_BYTE, &addr);
	return NULL;
}

const struct datatype ip6addr_type = {
	.type		= TYPE_IP6ADDR,
	.name		= "ipv6_addr",
	.desc		= "IPv6 address",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= 16 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.print		= ip6addr_type_print,
	.parse		= ip6addr_type_parse,
};

static void inet_protocol_type_print(const struct expr *expr,
				      struct output_ctx *octx)
{
	if (!nft_output_numeric_proto(octx) &&
	    mpz_cmp_ui(expr->value, UINT8_MAX) <= 0) {
		char name[NFT_PROTONAME_MAXSIZE];

		if (nft_getprotobynumber(mpz_get_uint8(expr->value), name, sizeof(name))) {
			nft_print(octx, "%s", name);
			return;
		}
	}
	integer_type_print(expr, octx);
}

static void inet_protocol_type_describe(struct output_ctx *octx)
{
	uint8_t protonum;

	for (protonum = 0; protonum < UINT8_MAX; protonum++) {
		char name[NFT_PROTONAME_MAXSIZE];

		if (!nft_getprotobynumber(protonum, name, sizeof(name)))
			continue;

		nft_print(octx, "\t%-30s\t%u\n", name, protonum);
	}
}

static struct error_record *inet_protocol_type_parse(struct parse_ctx *ctx,
						     const struct expr *sym,
						     struct expr **res)
{
	uint8_t proto;
	uintmax_t i;
	char *end;

	errno = 0;
	i = strtoumax(sym->identifier, &end, 0);
	if (sym->identifier != end && *end == '\0') {
		if (errno == ERANGE || i > UINT8_MAX)
			return error(&sym->location, "Protocol out of range");

		proto = i;
	} else {
		int r;

		r = nft_getprotobyname(sym->identifier);
		if (r < 0)
			return error(&sym->location, "Could not resolve protocol name");

		proto = r;
	}

	*res = constant_expr_alloc(&sym->location, &inet_protocol_type,
				   BYTEORDER_HOST_ENDIAN, BITS_PER_BYTE,
				   &proto);
	return NULL;
}

const struct datatype inet_protocol_type = {
	.type		= TYPE_INET_PROTOCOL,
	.name		= "inet_proto",
	.desc		= "Internet protocol",
	.size		= BITS_PER_BYTE,
	.basetype	= &integer_type,
	.print		= inet_protocol_type_print,
	.json		= inet_protocol_type_json,
	.parse		= inet_protocol_type_parse,
	.describe	= inet_protocol_type_describe,
};

static void inet_service_print(const struct expr *expr, struct output_ctx *octx)
{
	uint16_t port = mpz_get_be16(expr->value);
	char name[NFT_SERVNAME_MAXSIZE];

	if (!nft_getservbyport(port, NULL, name, sizeof(name)))
		nft_print(octx, "%hu", ntohs(port));
	else
		nft_print(octx, "\"%s\"", name);
}

void inet_service_type_print(const struct expr *expr, struct output_ctx *octx)
{
	if (nft_output_service(octx) &&
	    mpz_cmp_ui(expr->value, UINT16_MAX) <= 0) {
		inet_service_print(expr, octx);
		return;
	}
	integer_type_print(expr, octx);
}

static struct error_record *inet_service_type_parse(struct parse_ctx *ctx,
						    const struct expr *sym,
						    struct expr **res)
{
	struct addrinfo *ai;
	uint16_t port;
	uintmax_t i;
	char *end;
	int err;

	errno = 0;
	i = strtoumax(sym->identifier, &end, 0);
	if (sym->identifier != end && *end == '\0') {
		if (errno == ERANGE || i > UINT16_MAX)
			return error(&sym->location, "Service out of range");

		port = htons(i);
	} else {
		err = getaddrinfo(NULL, sym->identifier, NULL, &ai);
		if (err != 0)
			return error(&sym->location, "Could not resolve service: %s",
				     gai_strerror(err));

		if (ai->ai_addr->sa_family == AF_INET) {
			port = ((struct sockaddr_in *)(void *)ai->ai_addr)->sin_port;
		} else {
			assert(ai->ai_addr->sa_family == AF_INET6);
			port = ((struct sockaddr_in6 *)(void *)ai->ai_addr)->sin6_port;
		}
		freeaddrinfo(ai);
	}

	*res = constant_expr_alloc(&sym->location, &inet_service_type,
				   BYTEORDER_BIG_ENDIAN,
				   sizeof(port) * BITS_PER_BYTE, &port);
	return NULL;
}

const struct datatype inet_service_type = {
	.type		= TYPE_INET_SERVICE,
	.name		= "inet_service",
	.desc		= "internet network service",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= 2 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.print		= inet_service_type_print,
	.json		= inet_service_type_json,
	.parse		= inet_service_type_parse,
};

#define RT_SYM_TAB_INITIAL_SIZE		16

static FILE *open_iproute2_db(const char *filename, char **path)
{
	FILE *ret;

	if (filename[0] == '/')
		return fopen(filename, "r");

	if (asprintf(path, "/etc/iproute2/%s", filename) == -1)
		goto fail;

	ret = fopen(*path, "r");
	if (ret)
		return ret;

	free(*path);
	if (asprintf(path, "/usr/share/iproute2/%s", filename) == -1)
		goto fail;

	ret = fopen(*path, "r");
	if (ret)
		return ret;

	free(*path);
fail:
	*path = NULL;
	return NULL;
}

struct symbol_table *rt_symbol_table_init(const char *filename)
{
	char buf[512], namebuf[512], *p, *path = NULL;
	struct symbolic_constant s;
	struct symbol_table *tbl;
	unsigned int size, nelems, val;
	FILE *f;

	size = RT_SYM_TAB_INITIAL_SIZE;
	tbl = xmalloc(sizeof(*tbl) + size * sizeof(s));
	tbl->base = BASE_DECIMAL;
	nelems = 0;

	f = open_iproute2_db(filename, &path);
	if (f == NULL)
		goto out;

	while (fgets(buf, sizeof(buf), f)) {
		p = buf;
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == '#' || *p == '\n' || *p == '\0')
			continue;
		if (sscanf(p, "0x%x %511s\n", &val, namebuf) == 2 ||
		    sscanf(p, "0x%x %511s #", &val, namebuf) == 2) {
			tbl->base = BASE_HEXADECIMAL;
		} else if (sscanf(p, "%u %511s\n", &val, namebuf) == 2 ||
			   sscanf(p, "%u %511s #", &val, namebuf) == 2) {
			tbl->base = BASE_DECIMAL;
		} else {
			fprintf(stderr, "iproute database '%s' corrupted\n",
				path ?: filename);
			break;
		}

		/* One element is reserved for list terminator */
		if (nelems == size - 2) {
			size *= 2;
			tbl = xrealloc(tbl, sizeof(*tbl) + size * sizeof(s));
		}

		tbl->symbols[nelems].identifier = xstrdup(namebuf);
		tbl->symbols[nelems].value = val;
		nelems++;
	}

	fclose(f);
out:
	if (path)
		free(path);
	tbl->symbols[nelems] = SYMBOL_LIST_END;
	return tbl;
}

void rt_symbol_table_free(const struct symbol_table *tbl)
{
	const struct symbolic_constant *s;

	for (s = tbl->symbols; s->identifier != NULL; s++)
		free_const(s->identifier);
	free_const(tbl);
}

void rt_symbol_table_describe(struct output_ctx *octx, const char *name,
			      const struct symbol_table *tbl,
			      const struct datatype *type)
{
	char *path = NULL;
	FILE *f;

	if (!tbl || !tbl->symbols[0].identifier)
		return;

	f = open_iproute2_db(name, &path);
	if (f)
		fclose(f);
	if (!path && asprintf(&path, "%s%s",
			      name[0] == '/' ? "" : "unknown location of ",
			      name) < 0)
		return;

	nft_print(octx, "\npre-defined symbolic constants from %s ", path);
	if (tbl->base == BASE_DECIMAL)
		nft_print(octx, "(in decimal):\n");
	else
		nft_print(octx, "(in hexadecimal):\n");
	symbol_table_print(tbl, type, type->byteorder, octx);
	free(path);
}

void mark_table_init(struct nft_ctx *ctx)
{
	ctx->output.tbl.mark = rt_symbol_table_init("rt_marks");
}

void mark_table_exit(struct nft_ctx *ctx)
{
	rt_symbol_table_free(ctx->output.tbl.mark);
}

static void mark_type_print(const struct expr *expr, struct output_ctx *octx)
{
	return symbolic_constant_print(octx->tbl.mark, expr, true, octx);
}

static struct error_record *mark_type_parse(struct parse_ctx *ctx,
					    const struct expr *sym,
					    struct expr **res)
{
	return symbolic_constant_parse(ctx, sym, ctx->tbl->mark, res);
}

static void mark_type_describe(struct output_ctx *octx)
{
	rt_symbol_table_describe(octx, "rt_marks",
				 octx->tbl.mark, &mark_type);
}

const struct datatype mark_type = {
	.type		= TYPE_MARK,
	.name		= "mark",
	.desc		= "packet mark",
	.describe	= mark_type_describe,
	.size		= 4 * BITS_PER_BYTE,
	.byteorder	= BYTEORDER_HOST_ENDIAN,
	.basetype	= &integer_type,
	.basefmt	= "0x%.8Zx",
	.print		= mark_type_print,
	.json		= mark_type_json,
	.parse		= mark_type_parse,
};

/* symbol table for private datatypes for reject statement. */
static const struct symbol_table icmp_code_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("net-unreachable",	ICMP_NET_UNREACH),
		SYMBOL("host-unreachable",	ICMP_HOST_UNREACH),
		SYMBOL("prot-unreachable",	ICMP_PROT_UNREACH),
		SYMBOL("port-unreachable",	ICMP_PORT_UNREACH),
		SYMBOL("net-prohibited",	ICMP_NET_ANO),
		SYMBOL("host-prohibited",	ICMP_HOST_ANO),
		SYMBOL("admin-prohibited",	ICMP_PKT_FILTERED),
		SYMBOL("frag-needed",		ICMP_FRAG_NEEDED),
		SYMBOL_LIST_END
	},
};

/* private datatype for reject statement. */
const struct datatype reject_icmp_code_type = {
	.name		= "icmp_code",
	.desc		= "reject icmp code",
	.size		= BITS_PER_BYTE,
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.basetype	= &integer_type,
	.sym_tbl	= &icmp_code_tbl,
};

/* symbol table for private datatypes for reject statement. */
static const struct symbol_table icmpv6_code_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("no-route",		ICMPV6_NOROUTE),
		SYMBOL("admin-prohibited",	ICMPV6_ADM_PROHIBITED),
		SYMBOL("addr-unreachable",	ICMPV6_ADDR_UNREACH),
		SYMBOL("port-unreachable",	ICMPV6_PORT_UNREACH),
		SYMBOL("policy-fail",		ICMPV6_POLICY_FAIL),
		SYMBOL("reject-route",		ICMPV6_REJECT_ROUTE),
		SYMBOL_LIST_END
	},
};

/* private datatype for reject statement. */
const struct datatype reject_icmpv6_code_type = {
	.name		= "icmpv6_code",
	.desc		= "reject icmpv6 code",
	.size		= BITS_PER_BYTE,
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.basetype	= &integer_type,
	.sym_tbl	= &icmpv6_code_tbl,
};

/* symbol table for private datatypes for reject statement. */
static const struct symbol_table icmpx_code_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("port-unreachable",	NFT_REJECT_ICMPX_PORT_UNREACH),
		SYMBOL("admin-prohibited",	NFT_REJECT_ICMPX_ADMIN_PROHIBITED),
		SYMBOL("no-route",		NFT_REJECT_ICMPX_NO_ROUTE),
		SYMBOL("host-unreachable",	NFT_REJECT_ICMPX_HOST_UNREACH),
		SYMBOL_LIST_END
	},
};

/* private datatype for reject statement. */
const struct datatype reject_icmpx_code_type = {
	.name		= "icmpx_code",
	.desc		= "reject icmpx code",
	.size		= BITS_PER_BYTE,
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.basetype	= &integer_type,
	.sym_tbl	= &icmpx_code_tbl,
};

/* Backward compatible parser for the reject statement. */
static struct error_record *icmp_code_parse(struct parse_ctx *ctx,
					    const struct expr *sym,
					    struct expr **res)
{
	return symbolic_constant_parse(ctx, sym, &icmp_code_tbl, res);
}

const struct datatype icmp_code_type = {
	.type		= TYPE_ICMP_CODE,
	.name		= "icmp_code",
	.desc		= "icmp code",
	.size		= BITS_PER_BYTE,
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.basetype	= &integer_type,
	.parse		= icmp_code_parse,
};

/* Backward compatible parser for the reject statement. */
static struct error_record *icmpv6_code_parse(struct parse_ctx *ctx,
					      const struct expr *sym,
					      struct expr **res)
{
	return symbolic_constant_parse(ctx, sym, &icmpv6_code_tbl, res);
}

const struct datatype icmpv6_code_type = {
	.type		= TYPE_ICMPV6_CODE,
	.name		= "icmpv6_code",
	.desc		= "icmpv6 code",
	.size		= BITS_PER_BYTE,
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.basetype	= &integer_type,
	.parse		= icmpv6_code_parse,
};

/* Backward compatible parser for the reject statement. */
static struct error_record *icmpx_code_parse(struct parse_ctx *ctx,
					     const struct expr *sym,
					     struct expr **res)
{
	return symbolic_constant_parse(ctx, sym, &icmpx_code_tbl, res);
}

const struct datatype icmpx_code_type = {
	.type		= TYPE_ICMPX_CODE,
	.name		= "icmpx_code",
	.desc		= "icmpx code",
	.size		= BITS_PER_BYTE,
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.basetype	= &integer_type,
	.parse		= icmpx_code_parse,
};

void time_print(uint64_t ms, struct output_ctx *octx)
{
	uint64_t days, hours, minutes, seconds;
	bool printed = false;

	if (nft_output_seconds(octx)) {
		nft_print(octx, "%" PRIu64 "s", ms / 1000);
		return;
	}

	days = ms / 86400000;
	ms %= 86400000;

	hours = ms / 3600000;
	ms %= 3600000;

	minutes = ms / 60000;
	ms %= 60000;

	seconds = ms / 1000;
	ms %= 1000;

	if (days > 0) {
		nft_print(octx, "%" PRIu64 "d", days);
		printed = true;
	}
	if (hours > 0) {
		nft_print(octx, "%" PRIu64 "h", hours);
		printed = true;
	}
	if (minutes > 0) {
		nft_print(octx, "%" PRIu64 "m", minutes);
		printed = true;
	}
	if (seconds > 0) {
		nft_print(octx, "%" PRIu64 "s", seconds);
		printed = true;
	}
	if (ms > 0) {
		nft_print(octx, "%" PRIu64 "ms", ms);
		printed = true;
	}

	if (!printed)
		nft_print(octx, "0s");
}

enum {
	DAY	= (1 << 0),
	HOUR	= (1 << 1),
	MIN 	= (1 << 2),
	SECS	= (1 << 3),
	MSECS	= (1 << 4),
};

static uint32_t str2int(const char *str)
{
	int ret, number;

	ret = sscanf(str, "%d", &number);
	return ret == 1 ? number : 0;
}

struct error_record *time_parse(const struct location *loc, const char *str,
				uint64_t *res)
{
	unsigned int max_digits = strlen("12345678");
	int i, len;
	unsigned int k = 0;
	const char *c;
	uint64_t d = 0, h = 0, m = 0, s = 0, ms = 0;
	uint32_t mask = 0;

	c = str;
	len = strlen(c);
	for (i = 0; i < len; i++, c++) {
		switch (*c) {
		case 'd':
			if (mask & DAY)
				return error(loc,
					     "Day has been specified twice");

			d = str2int(c - k);
			k = 0;
			mask |= DAY;
			break;
		case 'h':
			if (mask & HOUR)
				return error(loc,
					     "Hour has been specified twice");

			h = str2int(c - k);
			k = 0;
			mask |= HOUR;
			break;
		case 'm':
			if (strcmp(c, "ms") == 0) {
				if (mask & MSECS)
					return error(loc,
						     "Millisecond has been specified twice");
				ms = str2int(c - k);
				c++;
				i++;
				k = 0;
				mask |= MSECS;
				break;
			}

			if (mask & MIN)
				return error(loc,
					     "Minute has been specified twice");

			m = str2int(c - k);
			k = 0;
			mask |= MIN;
			break;
		case 's':
			if (mask & SECS)
				return error(loc,
					     "Second has been specified twice");

			s = str2int(c - k);
			k = 0;
			mask |= SECS;
			break;
		default:
			if (!isdigit(*c))
				return error(loc, "wrong time format");

			if (k++ >= max_digits)
				return error(loc, "value too large");
			break;
		}
	}

	/* default to seconds if no unit was specified */
	if (!mask)
		ms = atoi(str) * MSEC_PER_SEC;
	else
		ms = 24*60*60*MSEC_PER_SEC * d +
			60*60*MSEC_PER_SEC * h +
			   60*MSEC_PER_SEC * m +
			      MSEC_PER_SEC * s + ms;

	*res = ms;
	return NULL;
}


static void time_type_print(const struct expr *expr, struct output_ctx *octx)
{
	time_print(mpz_get_uint64(expr->value), octx);
}

static struct error_record *time_type_parse(struct parse_ctx *ctx,
					    const struct expr *sym,
					    struct expr **res)
{
	struct error_record *erec;
	uint32_t s32;
	uint64_t s;

	erec = time_parse(&sym->location, sym->identifier, &s);
	if (erec != NULL)
		return erec;

	if (s > UINT32_MAX)
		return error(&sym->location, "value too large");

	s32 = s;
	*res = constant_expr_alloc(&sym->location, &time_type,
				   BYTEORDER_HOST_ENDIAN,
				   sizeof(uint32_t) * BITS_PER_BYTE, &s32);
	return NULL;
}

const struct datatype time_type = {
	.type		= TYPE_TIME,
	.name		= "time",
	.desc		= "relative time",
	.byteorder	= BYTEORDER_HOST_ENDIAN,
	.size		= 4 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.print		= time_type_print,
	.json		= time_type_json,
	.parse		= time_type_parse,
};

static struct error_record *concat_type_parse(struct parse_ctx *ctx,
					      const struct expr *sym,
					      struct expr **res)
{
	return error(&sym->location, "invalid data type, expected %s",
		     sym->dtype->desc);
}

static struct datatype *datatype_alloc(void)
{
	struct datatype *dtype;

	dtype = xzalloc(sizeof(*dtype));
	dtype->alloc = 1;
	dtype->refcnt = 1;

	return dtype;
}

const struct datatype *datatype_get(const struct datatype *ptr)
{
	struct datatype *dtype = (struct datatype *)ptr;

	if (!dtype)
		return NULL;
	if (!dtype->alloc)
		return dtype;

	dtype->refcnt++;
	return dtype;
}

void __datatype_set(struct expr *expr, const struct datatype *dtype)
{
	const struct datatype *dtype_free;

	dtype_free = expr->dtype;
	expr->dtype = dtype;
	datatype_free(dtype_free);
}

void datatype_set(struct expr *expr, const struct datatype *dtype)
{
	if (dtype != expr->dtype)
		__datatype_set(expr, datatype_get(dtype));
}

struct datatype *datatype_clone(const struct datatype *orig_dtype)
{
	struct datatype *dtype;

	dtype = xmalloc(sizeof(*dtype));
	*dtype = *orig_dtype;
	dtype->name = xstrdup(orig_dtype->name);
	dtype->desc = xstrdup(orig_dtype->desc);
	dtype->alloc = 1;
	dtype->refcnt = 1;

	return dtype;
}

void datatype_free(const struct datatype *ptr)
{
	struct datatype *dtype = (struct datatype *)ptr;

	if (!dtype)
		return;
	if (!dtype->alloc)
		return;

	assert(dtype->refcnt != 0);

	if (--dtype->refcnt > 0)
		return;

	free_const(dtype->name);
	free_const(dtype->desc);
	free(dtype);
}

const struct datatype *concat_type_alloc(uint32_t type)
{
	const struct datatype *i;
	struct datatype *dtype;
	char desc[256] = "concatenation of (";
	char name[256] = "";
	unsigned int size = 0, subtypes = 0, n;

	n = div_round_up(fls(type), TYPE_BITS);
	while (n > 0 && concat_subtype_id(type, --n)) {
		i = concat_subtype_lookup(type, n);
		if (i == NULL)
			return NULL;

		if (subtypes != 0) {
			strncat(desc, ", ", sizeof(desc) - strlen(desc) - 1);
			strncat(name, " . ", sizeof(name) - strlen(name) - 1);
		}
		strncat(desc, i->desc, sizeof(desc) - strlen(desc) - 1);
		strncat(name, i->name, sizeof(name) - strlen(name) - 1);

		size += netlink_padded_len(i->size);
		subtypes++;
	}
	strncat(desc, ")", sizeof(desc) - strlen(desc) - 1);

	dtype		= datatype_alloc();
	dtype->type	= type;
	dtype->size	= size;
	dtype->subtypes = subtypes;
	dtype->name	= xstrdup(name);
	dtype->desc	= xstrdup(desc);
	dtype->parse	= concat_type_parse;

	return dtype;
}

const struct datatype *set_datatype_alloc(const struct datatype *orig_dtype,
					  enum byteorder byteorder)
{
	struct datatype *dtype;

	/* Restrict dynamic datatype allocation to generic integer datatype. */
	if (orig_dtype != &integer_type)
		return datatype_get(orig_dtype);

	dtype = datatype_clone(orig_dtype);
	dtype->byteorder = byteorder;

	return dtype;
}

static struct error_record *time_unit_parse(const struct location *loc,
					    const char *str, uint64_t *unit)
{
	if (strcmp(str, "second") == 0)
		*unit = 1ULL;
	else if (strcmp(str, "minute") == 0)
		*unit = 1ULL * 60;
	else if (strcmp(str, "hour") == 0)
		*unit = 1ULL * 60 * 60;
	else if (strcmp(str, "day") == 0)
		*unit = 1ULL * 60 * 60 * 24;
	else if (strcmp(str, "week") == 0)
		*unit = 1ULL * 60 * 60 * 24 * 7;
	else
		return error(loc, "Wrong time format, expecting second, minute, hour, day or week");

	return NULL;
}

struct error_record *data_unit_parse(const struct location *loc,
				     const char *str, uint64_t *rate)
{
	if (strcmp(str, "bytes") == 0)
		*rate = 1ULL;
	else if (strcmp(str, "kbytes") == 0)
		*rate = 1024;
	else if (strcmp(str, "mbytes") == 0)
		*rate = 1024 * 1024;
	else
		return error(loc, "Wrong unit format, expecting bytes, kbytes or mbytes");

	return NULL;
}

struct error_record *rate_parse(const struct location *loc, const char *str,
				uint64_t *rate, uint64_t *unit)
{
	const char *slash, *rate_str;
	struct error_record *erec;

	slash = strchr(str, '/');
	if (!slash)
		return error(loc, "wrong rate format, expecting {bytes,kbytes,mbytes}/{second,minute,hour,day,week}");

	rate_str = strndup(str, slash - str);
	if (!rate_str)
		memory_allocation_error();

	erec = data_unit_parse(loc, rate_str, rate);
	free_const(rate_str);

	if (erec != NULL)
		return erec;

	erec = time_unit_parse(loc, slash + 1, unit);
	if (erec != NULL)
		return erec;

	return NULL;
}

static const struct symbol_table boolean_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("exists",	true),
		SYMBOL("missing",	false),
		SYMBOL_LIST_END
	},
};

static struct error_record *boolean_type_parse(struct parse_ctx *ctx,
					       const struct expr *sym,
					       struct expr **res)
{
	struct error_record *erec;
	uint8_t num;

	erec = integer_type_parse(ctx, sym, res);
	if (erec)
		return erec;

	if (mpz_cmp_ui((*res)->value, 0))
		num = 1;
	else
		num = 0;

	expr_free(*res);

	*res = constant_expr_alloc(&sym->location, &boolean_type,
				   BYTEORDER_HOST_ENDIAN, 1, &num);
	return NULL;
}

const struct datatype boolean_type = {
	.type		= TYPE_BOOLEAN,
	.name		= "boolean",
	.desc		= "boolean type",
	.size		= 1,
	.parse		= boolean_type_parse,
	.basetype	= &integer_type,
	.sym_tbl	= &boolean_tbl,
	.json		= boolean_type_json,
};

static struct error_record *priority_type_parse(struct parse_ctx *ctx,
						const struct expr *sym,
						struct expr **res)
{
	struct error_record *erec;
	int num;

	erec = integer_type_parse(ctx, sym, res);
	if (!erec) {
		num = atoi(sym->identifier);
		expr_free(*res);
		*res = constant_expr_alloc(&sym->location, &integer_type,
					   BYTEORDER_HOST_ENDIAN,
					   sizeof(int) * BITS_PER_BYTE, &num);
	} else {
		erec_destroy(erec);
		*res = constant_expr_alloc(&sym->location, &string_type,
					   BYTEORDER_HOST_ENDIAN,
					   strlen(sym->identifier) * BITS_PER_BYTE,
					   sym->identifier);
	}

	return NULL;
}

/* This datatype is not registered via datatype_register()
 * since this datatype should not ever be used from either
 * rules or elements.
 */
const struct datatype priority_type = {
	.type		= TYPE_STRING,
	.name		= "priority",
	.desc		= "priority type",
	.parse		= priority_type_parse,
};

static struct error_record *policy_type_parse(struct parse_ctx *ctx,
					      const struct expr *sym,
					      struct expr **res)
{
	int policy;

	if (!strcmp(sym->identifier, "accept"))
		policy = NF_ACCEPT;
	else if (!strcmp(sym->identifier, "drop"))
		policy = NF_DROP;
	else
		return error(&sym->location, "wrong policy");

	*res = constant_expr_alloc(&sym->location, &integer_type,
				   BYTEORDER_HOST_ENDIAN,
				   sizeof(int) * BITS_PER_BYTE, &policy);
	return NULL;
}

/* This datatype is not registered via datatype_register()
 * since this datatype should not ever be used from either
 * rules or elements.
 */
const struct datatype policy_type = {
	.type		= TYPE_STRING,
	.name		= "policy",
	.desc		= "policy type",
	.parse		= policy_type_parse,
};

#define SYSFS_CGROUPSV2_PATH	"/sys/fs/cgroup"

static char *cgroupv2_get_path(const char *path, uint64_t id)
{
	char dent_name[PATH_MAX + 1];
	char *cgroup_path = NULL;
	struct dirent *dent;
	struct stat st;
	DIR *d;

	d = opendir(path);
	if (!d)
		return NULL;

	while ((dent = readdir(d)) != NULL) {
		if (!strcmp(dent->d_name, ".") ||
		    !strcmp(dent->d_name, ".."))
			continue;

		snprintf(dent_name, sizeof(dent_name), "%s/%s",
			 path, dent->d_name);
		dent_name[sizeof(dent_name) - 1] = '\0';

		if (dent->d_ino == id) {
			cgroup_path = xstrdup(dent_name);
			break;
		}

		if (stat(dent_name, &st) >= 0 && S_ISDIR(st.st_mode)) {
			cgroup_path = cgroupv2_get_path(dent_name, id);
			if (cgroup_path)
				break;
		}
	}
	closedir(d);

	return cgroup_path;
}

static void cgroupv2_type_print(const struct expr *expr,
				struct output_ctx *octx)
{
	uint64_t id = mpz_get_uint64(expr->value);
	char *cgroup_path;

	cgroup_path = cgroupv2_get_path(SYSFS_CGROUPSV2_PATH, id);
	if (cgroup_path)
		nft_print(octx, "\"%s\"",
			  &cgroup_path[strlen(SYSFS_CGROUPSV2_PATH) + 1]);
	else
		nft_print(octx, "%" PRIu64, id);

	free(cgroup_path);
}

static struct error_record *cgroupv2_type_parse(struct parse_ctx *ctx,
						const struct expr *sym,
						struct expr **res)
{
	char cgroupv2_path[PATH_MAX + 1];
	struct stat st;
	uint64_t ino;

	snprintf(cgroupv2_path, sizeof(cgroupv2_path), "%s/%s",
		 SYSFS_CGROUPSV2_PATH, sym->identifier);
	cgroupv2_path[sizeof(cgroupv2_path) - 1] = '\0';

	if (stat(cgroupv2_path, &st) < 0)
		return error(&sym->location, "cgroupv2 path fails: %s",
			     strerror(errno));

	ino = st.st_ino;
	*res = constant_expr_alloc(&sym->location, &cgroupv2_type,
				   BYTEORDER_HOST_ENDIAN,
				   sizeof(ino) * BITS_PER_BYTE, &ino);
	return NULL;
}

const struct datatype cgroupv2_type = {
	.type		= TYPE_CGROUPV2,
	.name		= "cgroupsv2",
	.desc		= "cgroupsv2 path",
	.byteorder	= BYTEORDER_HOST_ENDIAN,
	.size		= 8 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.print		= cgroupv2_type_print,
	.parse		= cgroupv2_type_parse,
};
