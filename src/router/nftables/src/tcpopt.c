#include <nft.h>

#include <stddef.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>

#include <utils.h>
#include <headers.h>
#include <expression.h>
#include <tcpopt.h>

static const struct proto_hdr_template tcpopt_unknown_template =
	PROTO_HDR_TEMPLATE("unknown", &invalid_type, BYTEORDER_INVALID, 0, 0);

#define PHT(__token, __offset, __len) \
	PROTO_HDR_TEMPLATE(__token, &integer_type, BYTEORDER_BIG_ENDIAN, \
			   __offset, __len)
static const struct exthdr_desc tcpopt_eol = {
	.name		= "eol",
	.type		= TCPOPT_KIND_EOL,
	.templates	= {
		[TCPOPT_COMMON_KIND]		= PHT("kind",  0,    8),
	},
};

static const struct exthdr_desc tcpopt_nop = {
	.name		= "nop",
	.type		= TCPOPT_KIND_NOP,
	.templates	= {
		[TCPOPT_COMMON_KIND]		= PHT("kind",  0,    8),
	},
};

static const struct exthdr_desc tcptopt_maxseg = {
	.name		= "maxseg",
	.type		= TCPOPT_KIND_MAXSEG,
	.templates	= {
		[TCPOPT_MAXSEG_KIND]	= PHT("kind",   0,  8),
		[TCPOPT_MAXSEG_LENGTH]	= PHT("length", 8,  8),
		[TCPOPT_MAXSEG_SIZE]	= PHT("size",  16, 16),
	},
};

static const struct exthdr_desc tcpopt_window = {
	.name		= "window",
	.type		= TCPOPT_KIND_WINDOW,
	.templates	= {
		[TCPOPT_WINDOW_KIND]	= PHT("kind",   0,  8),
		[TCPOPT_WINDOW_LENGTH]	= PHT("length", 8,  8),
		[TCPOPT_WINDOW_COUNT]	= PHT("count", 16,  8),
	},
};

static const struct exthdr_desc tcpopt_sack_permitted = {
	.name		= "sack-perm",
	.type		= TCPOPT_KIND_SACK_PERMITTED,
	.templates	= {
		[TCPOPT_COMMON_KIND]	= PHT("kind",   0, 8),
		[TCPOPT_COMMON_LENGTH]	= PHT("length", 8, 8),
	},
};

static const struct exthdr_desc tcpopt_sack = {
	.name		= "sack",
	.type		= TCPOPT_KIND_SACK,
	.templates	= {
		[TCPOPT_SACK_KIND]	= PHT("kind",   0,   8),
		[TCPOPT_SACK_LENGTH]	= PHT("length", 8,   8),
		[TCPOPT_SACK_LEFT]	= PHT("left",  16,  32),
		[TCPOPT_SACK_RIGHT]	= PHT("right", 48,  32),
		[TCPOPT_SACK_LEFT1]	= PHT("left",  80,  32),
		[TCPOPT_SACK_RIGHT1]	= PHT("right", 112,  32),
		[TCPOPT_SACK_LEFT2]	= PHT("left",  144,  32),
		[TCPOPT_SACK_RIGHT2]	= PHT("right", 176,  32),
		[TCPOPT_SACK_LEFT3]	= PHT("left",  208,  32),
		[TCPOPT_SACK_RIGHT3]	= PHT("right", 240,  32),
	},
};

static const struct exthdr_desc tcpopt_timestamp = {
	.name		= "timestamp",
	.type		= TCPOPT_KIND_TIMESTAMP,
	.templates	= {
		[TCPOPT_TS_KIND]	= PHT("kind",   0,  8),
		[TCPOPT_TS_LENGTH]	= PHT("length", 8,  8),
		[TCPOPT_TS_TSVAL]	= PHT("tsval",  16, 32),
		[TCPOPT_TS_TSECR]	= PHT("tsecr",  48, 32),
	},
};

static const struct exthdr_desc tcpopt_fastopen = {
	.name		= "fastopen",
	.type		= TCPOPT_KIND_FASTOPEN,
	.templates	= {
		[TCPOPT_COMMON_KIND]	= PHT("kind",   0, 8),
		[TCPOPT_COMMON_LENGTH]	= PHT("length", 8, 8),
	},
};

static const struct exthdr_desc tcpopt_md5sig = {
	.name		= "md5sig",
	.type		= TCPOPT_KIND_MD5SIG,
	.templates	= {
		[TCPOPT_COMMON_KIND]	= PHT("kind",   0, 8),
		[TCPOPT_COMMON_LENGTH]	= PHT("length", 8, 8),
	},
};

static const struct symbol_table mptcp_subtype_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("mp-capable",	0),
		SYMBOL("mp-join",	1),
		SYMBOL("dss",		2),
		SYMBOL("add-addr",	3),
		SYMBOL("remove-addr",	4),
		SYMBOL("mp-prio",	5),
		SYMBOL("mp-fail",	6),
		SYMBOL("mp-fastclose",	7),
		SYMBOL("mp-tcprst",	8),
		SYMBOL_LIST_END
	},
};

/* alias of integer_type to parse mptcp subtypes */
const struct datatype mptcpopt_subtype = {
	.type		= TYPE_INTEGER,
	.name		= "integer",
	.desc		= "mptcp option subtype",
	.size		= 4,
	.basetype	= &integer_type,
	.sym_tbl	= &mptcp_subtype_tbl,
};

static const struct exthdr_desc tcpopt_mptcp = {
	.name		= "mptcp",
	.type		= TCPOPT_KIND_MPTCP,
	.templates	= {
		[TCPOPT_MPTCP_KIND]	= PHT("kind",   0,   8),
		[TCPOPT_MPTCP_LENGTH]	= PHT("length", 8,  8),
		[TCPOPT_MPTCP_SUBTYPE]  = PROTO_HDR_TEMPLATE("subtype",
							     &mptcpopt_subtype,
							     BYTEORDER_BIG_ENDIAN,
							     16, 4),
	},
};

static const struct exthdr_desc tcpopt_fallback = {
	.templates	= {
		[TCPOPT_COMMON_KIND]	= PHT("kind",   0, 8),
		[TCPOPT_COMMON_LENGTH]	= PHT("length", 8, 8),
	},
};
#undef PHT

const struct exthdr_desc *tcpopt_protocols[] = {
	[TCPOPT_KIND_EOL]		= &tcpopt_eol,
	[TCPOPT_KIND_NOP]		= &tcpopt_nop,
	[TCPOPT_KIND_MAXSEG]		= &tcptopt_maxseg,
	[TCPOPT_KIND_WINDOW]		= &tcpopt_window,
	[TCPOPT_KIND_SACK_PERMITTED]	= &tcpopt_sack_permitted,
	[TCPOPT_KIND_SACK]		= &tcpopt_sack,
	[TCPOPT_KIND_TIMESTAMP]		= &tcpopt_timestamp,
	[TCPOPT_KIND_MD5SIG]		= &tcpopt_md5sig,
	[TCPOPT_KIND_MPTCP]		= &tcpopt_mptcp,
	[TCPOPT_KIND_FASTOPEN]		= &tcpopt_fastopen,
};

static void tcpopt_assign_tmpl(struct expr *expr,
			       const struct proto_hdr_template *tmpl,
			       const struct exthdr_desc *desc)
{
	expr->exthdr.op     = NFT_EXTHDR_OP_TCPOPT;

	expr->exthdr.desc   = desc;
	expr->exthdr.tmpl   = tmpl;
	expr->exthdr.offset = tmpl->offset;
}

/**
 * tcpopt_expr_alloc - allocate tcp option extension expression
 *
 * @loc: location from parser
 * @kind: raw tcp option value to find in packet
 * @field: highlevel field to find in the option if @kind is present in packet
 *
 * Allocate a new tcp option expression.
 * @kind is the raw option value to find in the packet.
 * Exception: SACK may use extra OOB data that is mangled here.
 *
 * @field is the optional field to extract from the @type option.
 */
struct expr *tcpopt_expr_alloc(const struct location *loc,
			       unsigned int kind,
			       unsigned int field)
{
	const struct proto_hdr_template *tmpl;
	const struct exthdr_desc *desc = NULL;
	struct expr *expr;

	switch (kind) {
	case TCPOPT_KIND_SACK1:
		kind = TCPOPT_KIND_SACK;
		if (field == TCPOPT_SACK_LEFT)
			field = TCPOPT_SACK_LEFT1;
		else if (field == TCPOPT_SACK_RIGHT)
			field = TCPOPT_SACK_RIGHT1;
		break;
	case TCPOPT_KIND_SACK2:
		kind = TCPOPT_KIND_SACK;
		if (field == TCPOPT_SACK_LEFT)
			field = TCPOPT_SACK_LEFT2;
		else if (field == TCPOPT_SACK_RIGHT)
			field = TCPOPT_SACK_RIGHT2;
		break;
	case TCPOPT_KIND_SACK3:
		kind = TCPOPT_KIND_SACK;
		if (field == TCPOPT_SACK_LEFT)
			field = TCPOPT_SACK_LEFT3;
		else if (field == TCPOPT_SACK_RIGHT)
			field = TCPOPT_SACK_RIGHT3;
		break;
	}

	if (kind < array_size(tcpopt_protocols))
		desc = tcpopt_protocols[kind];

	if (!desc) {
		if (kind > 255)
			return NULL;

		desc = &tcpopt_fallback;

		switch (field) {
		case TCPOPT_COMMON_KIND:
		case TCPOPT_COMMON_LENGTH:
			tmpl = &desc->templates[field];
			break;
		default:
			tmpl = &tcpopt_unknown_template;
			break;
		}

		expr = expr_alloc(loc, EXPR_EXTHDR, &integer_type,
				  BYTEORDER_BIG_ENDIAN, 8);

		expr->exthdr.raw_type = kind;
		tcpopt_assign_tmpl(expr, tmpl, desc);
		return expr;
	}

	tmpl = &desc->templates[field];
	if (!tmpl || !tmpl->dtype)
		return NULL;

	expr = expr_alloc(loc, EXPR_EXTHDR, tmpl->dtype,
			  BYTEORDER_BIG_ENDIAN, tmpl->len);

	expr->exthdr.raw_type = desc->type;
	tcpopt_assign_tmpl(expr, tmpl, desc);

	return expr;
}

void tcpopt_init_raw(struct expr *expr, uint8_t type, unsigned int off,
		     unsigned int len, uint32_t flags)
{
	const struct proto_hdr_template *tmpl;
	unsigned int i;

	assert(expr->etype == EXPR_EXTHDR);

	expr->len = len;
	expr->exthdr.flags = flags;
	expr->exthdr.offset = off;
	expr->exthdr.op = NFT_EXTHDR_OP_TCPOPT;
	expr->exthdr.tmpl = &tcpopt_unknown_template;

	if (flags & NFT_EXTHDR_F_PRESENT)
		datatype_set(expr, &boolean_type);
	else
		datatype_set(expr, &integer_type);

	if (type >= array_size(tcpopt_protocols) ||
	    !tcpopt_protocols[type])
		return;

	expr->exthdr.desc = tcpopt_protocols[type];
	expr->exthdr.flags = flags;
	assert(expr->exthdr.desc != NULL);

	for (i = 0; i < array_size(expr->exthdr.desc->templates); ++i) {
		tmpl = &expr->exthdr.desc->templates[i];
		if (tmpl->offset != off || tmpl->len != len)
			continue;

		if ((flags & NFT_EXTHDR_F_PRESENT) == 0)
			datatype_set(expr, tmpl->dtype);

		expr->exthdr.tmpl = tmpl;
		break;
	}
}

bool tcpopt_find_template(struct expr *expr, unsigned int offset, unsigned int len)
{
	if (expr->exthdr.tmpl != &tcpopt_unknown_template)
		return false;

	tcpopt_init_raw(expr, expr->exthdr.desc->type, offset, len, 0);

	if (expr->exthdr.tmpl == &tcpopt_unknown_template)
		return false;

	return true;
}
