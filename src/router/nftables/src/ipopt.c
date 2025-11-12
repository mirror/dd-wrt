#include <nft.h>


#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>

#include <utils.h>
#include <headers.h>
#include <expression.h>
#include <ipopt.h>

static const struct proto_hdr_template ipopt_unknown_template =
	PROTO_HDR_TEMPLATE("unknown", &invalid_type, BYTEORDER_INVALID, 0, 0);

#define PHT(__token, __offset, __len) \
	PROTO_HDR_TEMPLATE(__token, &integer_type, BYTEORDER_BIG_ENDIAN, \
			   __offset, __len)
static const struct exthdr_desc ipopt_lsrr = {
	.name		= "lsrr",
	.type		= IPOPT_LSRR,
	.templates	= {
		[IPOPT_FIELD_TYPE]		= PHT("type",    0,  8),
		[IPOPT_FIELD_LENGTH]		= PHT("length",  8,  8),
		[IPOPT_FIELD_PTR]		= PHT("ptr",    16,  8),
		[IPOPT_FIELD_ADDR_0]		= PROTO_HDR_TEMPLATE("addr", &ipaddr_type, BYTEORDER_BIG_ENDIAN, 24, 32),
	},
};

static const struct exthdr_desc ipopt_rr = {
	.name		= "rr",
	.type		= IPOPT_RR,
	.templates	= {
		[IPOPT_FIELD_TYPE]		= PHT("type",   0,   8),
		[IPOPT_FIELD_LENGTH]		= PHT("length",  8,  8),
		[IPOPT_FIELD_PTR]		= PHT("ptr",    16,  8),
		[IPOPT_FIELD_ADDR_0]		= PROTO_HDR_TEMPLATE("addr", &ipaddr_type, BYTEORDER_BIG_ENDIAN, 24, 32),
	},
};

static const struct exthdr_desc ipopt_ssrr = {
	.name		= "ssrr",
	.type		= IPOPT_SSRR,
	.templates	= {
		[IPOPT_FIELD_TYPE]		= PHT("type",   0,   8),
		[IPOPT_FIELD_LENGTH]		= PHT("length",  8,  8),
		[IPOPT_FIELD_PTR]		= PHT("ptr",    16,  8),
		[IPOPT_FIELD_ADDR_0]		= PROTO_HDR_TEMPLATE("addr", &ipaddr_type, BYTEORDER_BIG_ENDIAN, 24, 32),
	},
};

static const struct exthdr_desc ipopt_ra = {
	.name		= "ra",
	.type		= IPOPT_RA,
	.templates	= {
		[IPOPT_FIELD_TYPE]		= PHT("type",   0,   8),
		[IPOPT_FIELD_LENGTH]		= PHT("length", 8,   8),
		[IPOPT_FIELD_VALUE]		= PHT("value",  16, 16),
	},
};

const struct exthdr_desc *ipopt_protocols[UINT8_MAX] = {
	[IPOPT_LSRR]		= &ipopt_lsrr,
	[IPOPT_RR]		= &ipopt_rr,
	[IPOPT_SSRR]		= &ipopt_ssrr,
	[IPOPT_RA]		= &ipopt_ra,
};

struct expr *ipopt_expr_alloc(const struct location *loc, uint8_t type,
			       uint8_t field)
{
	const struct proto_hdr_template *tmpl;
	const struct exthdr_desc *desc;
	struct expr *expr;

	desc = ipopt_protocols[type];
	tmpl = &desc->templates[field];
	if (!tmpl)
		return NULL;

	if (!tmpl->len)
		return NULL;

	expr = expr_alloc(loc, EXPR_EXTHDR, tmpl->dtype,
			  BYTEORDER_BIG_ENDIAN, tmpl->len);
	expr->exthdr.desc   = desc;
	expr->exthdr.tmpl   = tmpl;
	expr->exthdr.op     = NFT_EXTHDR_OP_IPV4;
	expr->exthdr.offset = tmpl->offset;
	expr->exthdr.raw_type = desc->type;

	return expr;
}

void ipopt_init_raw(struct expr *expr, uint8_t type, unsigned int offset,
		     unsigned int len, uint32_t flags, bool set_unknown)
{
	const struct proto_hdr_template *tmpl;
	unsigned int i;

	assert(expr->etype == EXPR_EXTHDR);

	expr->len = len;
	expr->exthdr.flags = flags;
	expr->exthdr.offset = offset;

	assert(type < array_size(ipopt_protocols));
	expr->exthdr.desc = ipopt_protocols[type];
	expr->exthdr.flags = flags;

	for (i = 0; i < array_size(expr->exthdr.desc->templates); ++i) {
		tmpl = &expr->exthdr.desc->templates[i];

		/* Make sure that it's the right template based on offset and len */
		if (tmpl->offset != offset || tmpl->len != len)
			continue;

		if (flags & NFT_EXTHDR_F_PRESENT)
			expr->dtype = &boolean_type;
		else
			expr->dtype = tmpl->dtype;
		expr->exthdr.tmpl = tmpl;
		expr->exthdr.op   = NFT_EXTHDR_OP_IPV4;
		break;
	}
	if (i == array_size(expr->exthdr.desc->templates) && set_unknown) {
		expr->exthdr.tmpl = &ipopt_unknown_template;
		expr->exthdr.op   = NFT_EXTHDR_OP_IPV4;
	}
}

bool ipopt_find_template(struct expr *expr, unsigned int offset,
			  unsigned int len)
{
	if (expr->exthdr.tmpl != &ipopt_unknown_template)
		return false;

	ipopt_init_raw(expr, expr->exthdr.desc->type, offset, len, 0, false);

	if (expr->exthdr.tmpl == &ipopt_unknown_template)
		return false;

	return true;
}
