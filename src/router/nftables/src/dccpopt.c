#include <nft.h>

#include <stddef.h>

#include <datatype.h>
#include <dccpopt.h>
#include <expression.h>
#include <nftables.h>
#include <utils.h>

#define PHT(__token, __offset, __len)					 \
	PROTO_HDR_TEMPLATE(__token, &integer_type, BYTEORDER_BIG_ENDIAN, \
			   __offset, __len)

static const struct proto_hdr_template dccpopt_unknown_template =
	PROTO_HDR_TEMPLATE("unknown", &invalid_type, BYTEORDER_INVALID, 0, 0);

/*
 *             Option                           DCCP-  Section
 *     Type    Length     Meaning               Data?  Reference
 *     ----    ------     -------               -----  ---------
 *       0        1       Padding                 Y      5.8.1
 *       1        1       Mandatory               N      5.8.2
 *       2        1       Slow Receiver           Y      11.6
 *     3-31       1       Reserved
 *      32     variable   Change L                N      6.1
 *      33     variable   Confirm L               N      6.2
 *      34     variable   Change R                N      6.1
 *      35     variable   Confirm R               N      6.2
 *      36     variable   Init Cookie             N      8.1.4
 *      37       3-8      NDP Count               Y      7.7
 *      38     variable   Ack Vector [Nonce 0]    N      11.4
 *      39     variable   Ack Vector [Nonce 1]    N      11.4
 *      40     variable   Data Dropped            N      11.7
 *      41        6       Timestamp               Y      13.1
 *      42      6/8/10    Timestamp Echo          Y      13.3
 *      43       4/6      Elapsed Time            N      13.2
 *      44        6       Data Checksum           Y      9.3
 *     45-127  variable   Reserved
 *    128-255  variable   CCID-specific options   -      10.3
 */

static const struct exthdr_desc dccpopt_padding = {
	.name		= "padding",
	.type           = DCCPOPT_PADDING,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",	0,	8),
	},
};

static const struct exthdr_desc dccpopt_mandatory = {
	.name		= "mandatory",
	.type           = DCCPOPT_MANDATORY,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",	0,	8),
	},
};

static const struct exthdr_desc dccpopt_slow_receiver = {
	.name		= "slow_receiver",
	.type           = DCCPOPT_SLOW_RECEIVER,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",	0,	8),
	},
};

static const struct exthdr_desc dccpopt_reserved_short = {
	.name		= "reserved_short",
	.type           = DCCPOPT_RESERVED_SHORT,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",	0,	8),
	},
};

static const struct exthdr_desc dccpopt_change_l = {
	.name		= "change_l",
	.type           = DCCPOPT_CHANGE_L,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8)
	},
};

static const struct exthdr_desc dccpopt_confirm_l = {
	.name		= "confirm_l",
	.type           = DCCPOPT_CONFIRM_L,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_change_r = {
	.name		= "change_r",
	.type           = DCCPOPT_CHANGE_R,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_confirm_r = {
	.name		= "confirm_r",
	.type           = DCCPOPT_CONFIRM_R,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_init_cookie = {
	.name		= "init_cookie",
	.type           = DCCPOPT_INIT_COOKIE,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_ndp_count = {
	.name		= "ndp_count",
	.type           = DCCPOPT_NDP_COUNT,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_ack_vector_nonce_0 = {
	.name		= "ack_vector_nonce_0",
	.type           = DCCPOPT_ACK_VECTOR_NONCE_0,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_ack_vector_nonce_1 = {
	.name		= "ack_vector_nonce_1",
	.type           = DCCPOPT_ACK_VECTOR_NONCE_1,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_data_dropped = {
	.name		= "data_dropped",
	.type           = DCCPOPT_DATA_DROPPED,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_timestamp = {
	.name		= "timestamp",
	.type           = DCCPOPT_TIMESTAMP,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_timestamp_echo = {
	.name		= "timestamp_echo",
	.type           = DCCPOPT_TIMESTAMP_ECHO,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_elapsed_time = {
	.name		= "elapsed_time",
	.type           = DCCPOPT_ELAPSED_TIME,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_data_checksum = {
	.name		= "data_checksum",
	.type           = DCCPOPT_DATA_CHECKSUM,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_reserved_long = {
	.name		= "reserved_long",
	.type           = DCCPOPT_RESERVED_LONG,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

static const struct exthdr_desc dccpopt_ccid_specific = {
	.name		= "ccid_specific",
	.type           = DCCPOPT_CCID_SPECIFIC,
	.templates	= {
		[DCCPOPT_FIELD_TYPE]	= PHT("type",    0,    8),
	},
};

const struct exthdr_desc *dccpopt_protocols[1 + UINT8_MAX] = {
	[DCCPOPT_PADDING]		= &dccpopt_padding,
	[DCCPOPT_MANDATORY]		= &dccpopt_mandatory,
	[DCCPOPT_SLOW_RECEIVER]		= &dccpopt_slow_receiver,
	[DCCPOPT_RESERVED_SHORT]	= &dccpopt_reserved_short,
	[DCCPOPT_CHANGE_L]		= &dccpopt_change_l,
	[DCCPOPT_CONFIRM_L]		= &dccpopt_confirm_l,
	[DCCPOPT_CHANGE_R]		= &dccpopt_change_r,
	[DCCPOPT_CONFIRM_R]		= &dccpopt_confirm_r,
	[DCCPOPT_INIT_COOKIE]		= &dccpopt_init_cookie,
	[DCCPOPT_NDP_COUNT]		= &dccpopt_ndp_count,
	[DCCPOPT_ACK_VECTOR_NONCE_0]	= &dccpopt_ack_vector_nonce_0,
	[DCCPOPT_ACK_VECTOR_NONCE_1]	= &dccpopt_ack_vector_nonce_1,
	[DCCPOPT_DATA_DROPPED]		= &dccpopt_data_dropped,
	[DCCPOPT_TIMESTAMP]		= &dccpopt_timestamp,
	[DCCPOPT_TIMESTAMP_ECHO]	= &dccpopt_timestamp_echo,
	[DCCPOPT_ELAPSED_TIME]		= &dccpopt_elapsed_time,
	[DCCPOPT_DATA_CHECKSUM]		= &dccpopt_data_checksum,
	[DCCPOPT_RESERVED_LONG]		= &dccpopt_reserved_long,
	[DCCPOPT_CCID_SPECIFIC]		= &dccpopt_ccid_specific,
};

const struct exthdr_desc *
dccpopt_find_desc(uint8_t type)
{
	enum dccpopt_types proto_idx =
		  3 <= type && type <=  31 ? DCCPOPT_RESERVED_SHORT :
		 45 <= type && type <= 127 ? DCCPOPT_RESERVED_LONG  :
		128 <= type                ? DCCPOPT_CCID_SPECIFIC  : type;

	return dccpopt_protocols[proto_idx];
}

struct expr *
dccpopt_expr_alloc(const struct location *loc, uint8_t type)
{
	const struct proto_hdr_template *tmpl;
	const struct exthdr_desc *desc;
	struct expr *expr;

	desc = dccpopt_find_desc(type);
	tmpl = &desc->templates[DCCPOPT_FIELD_TYPE];

	expr = expr_alloc(loc, EXPR_EXTHDR, tmpl->dtype,
			  BYTEORDER_BIG_ENDIAN, BITS_PER_BYTE);
	expr->exthdr.desc     = desc;
	expr->exthdr.tmpl     = tmpl;
	expr->exthdr.offset   = tmpl->offset;
	expr->exthdr.raw_type = type;
	expr->exthdr.flags    = NFT_EXTHDR_F_PRESENT;
	expr->exthdr.op       = NFT_EXTHDR_OP_DCCP;

	return expr;
}

void
dccpopt_init_raw(struct expr *expr, uint8_t type, unsigned int offset,
		 unsigned int len)
{
	const struct proto_hdr_template *tmpl;
	const struct exthdr_desc *desc;

	assert(expr->etype == EXPR_EXTHDR);

	desc = dccpopt_find_desc(type);
	tmpl = &desc->templates[DCCPOPT_FIELD_TYPE];

	expr->len = len;
	datatype_set(expr, &boolean_type);

	expr->exthdr.offset = offset;
	expr->exthdr.desc   = desc;
	expr->exthdr.flags  = NFT_EXTHDR_F_PRESENT;
	expr->exthdr.op     = NFT_EXTHDR_OP_DCCP;

	/* Make sure that it's the right template based on offset and
	 * len
	 */
	if (tmpl->offset != offset || tmpl->len != len)
		expr->exthdr.tmpl = &dccpopt_unknown_template;
	else
		expr->exthdr.tmpl = tmpl;
}
