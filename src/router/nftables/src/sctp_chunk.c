/*
 * Copyright Red Hat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <exthdr.h>
#include <sctp_chunk.h>


#define PHT(__token, __offset, __len) \
	PROTO_HDR_TEMPLATE(__token, &integer_type, BYTEORDER_BIG_ENDIAN, \
			   __offset, __len)

static const struct exthdr_desc sctp_chunk_data = {
	.name	= "data",
	.type	= SCTP_CHUNK_TYPE_DATA,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_DATA_TSN]		= PHT("tsn", 32, 32),
		[SCTP_CHUNK_DATA_STREAM]	= PHT("stream", 64, 16),
		[SCTP_CHUNK_DATA_SSN]		= PHT("ssn", 80, 16),
		[SCTP_CHUNK_DATA_PPID]		= PHT("ppid", 96, 32),
	},
};

static const struct exthdr_desc sctp_chunk_init = {
	.name	= "init",
	.type	= SCTP_CHUNK_TYPE_INIT,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_INIT_TAG]		= PHT("init-tag", 32, 32),
		[SCTP_CHUNK_INIT_RWND]		= PHT("a-rwnd", 64, 32),
		[SCTP_CHUNK_INIT_OSTREAMS]	= PHT("num-outbound-streams", 96, 16),
		[SCTP_CHUNK_INIT_ISTREAMS]	= PHT("num-inbound-streams", 112, 16),
		[SCTP_CHUNK_INIT_TSN]		= PHT("initial-tsn", 128, 32),
	},
};

static const struct exthdr_desc sctp_chunk_init_ack = {
	.name	= "init-ack",
	.type	= SCTP_CHUNK_TYPE_INIT_ACK,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_INIT_TAG]		= PHT("init-tag", 32, 32),
		[SCTP_CHUNK_INIT_RWND]		= PHT("a-rwnd", 64, 32),
		[SCTP_CHUNK_INIT_OSTREAMS]	= PHT("num-outbound-streams", 96, 16),
		[SCTP_CHUNK_INIT_ISTREAMS]	= PHT("num-inbound-streams", 112, 16),
		[SCTP_CHUNK_INIT_TSN]		= PHT("initial-tsn", 128, 32),
	},
};

static const struct exthdr_desc sctp_chunk_sack = {
	.name	= "sack",
	.type	= SCTP_CHUNK_TYPE_SACK,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_SACK_CTSN_ACK]	= PHT("cum-tsn-ack", 32, 32),
		[SCTP_CHUNK_SACK_RWND]		= PHT("a-rwnd", 64, 32),
		[SCTP_CHUNK_SACK_GACK_BLOCKS]	= PHT("num-gap-ack-blocks", 96, 16),
		[SCTP_CHUNK_SACK_DUP_TSNS]	= PHT("num-dup-tsns", 112, 16),
	},
};

static const struct exthdr_desc sctp_chunk_shutdown = {
	.name	= "shutdown",
	.type	= SCTP_CHUNK_TYPE_SHUTDOWN,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_SHUTDOWN_CTSN_ACK]	= PHT("cum-tsn-ack", 32, 32),
	},
};

static const struct exthdr_desc sctp_chunk_ecne = {
	.name	= "ecne",
	.type	= SCTP_CHUNK_TYPE_ECNE,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_ECNE_CWR_MIN_TSN]	= PHT("lowest-tsn", 32, 32),
	},
};

static const struct exthdr_desc sctp_chunk_cwr = {
	.name	= "cwr",
	.type	= SCTP_CHUNK_TYPE_CWR,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_ECNE_CWR_MIN_TSN]	= PHT("lowest-tsn", 32, 32),
	},
};

static const struct exthdr_desc sctp_chunk_asconf_ack = {
	.name	= "asconf-ack",
	.type	= SCTP_CHUNK_TYPE_ASCONF_ACK,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_ASCONF_SEQNO]	= PHT("seqno", 32, 32),
	},
};

static const struct exthdr_desc sctp_chunk_forward_tsn = {
	.name	= "forward-tsn",
	.type	= SCTP_CHUNK_TYPE_FORWARD_TSN,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_FORWARD_TSN_NCTSN]	= PHT("new-cum-tsn", 32, 32),
	},
};

static const struct exthdr_desc sctp_chunk_asconf = {
	.name	= "asconf",
	.type	= SCTP_CHUNK_TYPE_ASCONF,
	.templates = {
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),
		[SCTP_CHUNK_ASCONF_SEQNO]	= PHT("seqno", 32, 32),
	},
};

#define SCTP_CHUNK_DESC_GENERATOR(descname, hname, desctype)		\
static const struct exthdr_desc sctp_chunk_##descname = {		\
	.name	= #hname,						\
	.type	= SCTP_CHUNK_TYPE_##desctype,				\
	.templates = {							\
		[SCTP_CHUNK_COMMON_TYPE]	= PHT("type", 0, 8),	\
		[SCTP_CHUNK_COMMON_FLAGS]	= PHT("flags", 8, 8),	\
		[SCTP_CHUNK_COMMON_LENGTH]	= PHT("length", 16, 16),\
	},								\
};

SCTP_CHUNK_DESC_GENERATOR(heartbeat, heartbeat, HEARTBEAT)
SCTP_CHUNK_DESC_GENERATOR(heartbeat_ack, heartbeat-ack, HEARTBEAT_ACK)
SCTP_CHUNK_DESC_GENERATOR(abort, abort, ABORT)
SCTP_CHUNK_DESC_GENERATOR(shutdown_ack, shutdown-ack, SHUTDOWN_ACK)
SCTP_CHUNK_DESC_GENERATOR(error, error, ERROR)
SCTP_CHUNK_DESC_GENERATOR(cookie_echo, cookie-echo, COOKIE_ECHO)
SCTP_CHUNK_DESC_GENERATOR(cookie_ack, cookie-ack, COOKIE_ACK)
SCTP_CHUNK_DESC_GENERATOR(shutdown_complete, shutdown-complete, SHUTDOWN_COMPLETE)

#undef SCTP_CHUNK_DESC_GENERATOR

static const struct exthdr_desc *sctp_chunk_protocols[] = {
	[SCTP_CHUNK_TYPE_DATA]			= &sctp_chunk_data,
	[SCTP_CHUNK_TYPE_INIT]			= &sctp_chunk_init,
	[SCTP_CHUNK_TYPE_INIT_ACK]		= &sctp_chunk_init_ack,
	[SCTP_CHUNK_TYPE_SACK]			= &sctp_chunk_sack,
	[SCTP_CHUNK_TYPE_HEARTBEAT]		= &sctp_chunk_heartbeat,
	[SCTP_CHUNK_TYPE_HEARTBEAT_ACK]		= &sctp_chunk_heartbeat_ack,
	[SCTP_CHUNK_TYPE_ABORT]			= &sctp_chunk_abort,
	[SCTP_CHUNK_TYPE_SHUTDOWN]		= &sctp_chunk_shutdown,
	[SCTP_CHUNK_TYPE_SHUTDOWN_ACK]		= &sctp_chunk_shutdown_ack,
	[SCTP_CHUNK_TYPE_ERROR]			= &sctp_chunk_error,
	[SCTP_CHUNK_TYPE_COOKIE_ECHO]		= &sctp_chunk_cookie_echo,
	[SCTP_CHUNK_TYPE_COOKIE_ACK]		= &sctp_chunk_cookie_ack,
	[SCTP_CHUNK_TYPE_ECNE]			= &sctp_chunk_ecne,
	[SCTP_CHUNK_TYPE_CWR]			= &sctp_chunk_cwr,
	[SCTP_CHUNK_TYPE_SHUTDOWN_COMPLETE]	= &sctp_chunk_shutdown_complete,
	[SCTP_CHUNK_TYPE_ASCONF_ACK]		= &sctp_chunk_asconf_ack,
	[SCTP_CHUNK_TYPE_FORWARD_TSN]		= &sctp_chunk_forward_tsn,
	[SCTP_CHUNK_TYPE_ASCONF]		= &sctp_chunk_asconf,
};

const struct exthdr_desc *sctp_chunk_protocol_find(const char *name)
{
	unsigned int i;

	for (i = 0; i < array_size(sctp_chunk_protocols); i++) {
		if (sctp_chunk_protocols[i] &&
		    !strcmp(sctp_chunk_protocols[i]->name, name))
			return sctp_chunk_protocols[i];
	}
	return NULL;
}

struct expr *sctp_chunk_expr_alloc(const struct location *loc,
				   unsigned int type, unsigned int field)
{
	const struct proto_hdr_template *tmpl;
	const struct exthdr_desc *desc = NULL;
	struct expr *expr;

	if (type < array_size(sctp_chunk_protocols))
		desc = sctp_chunk_protocols[type];

	if (!desc)
		return NULL;

	tmpl = &desc->templates[field];
	if (!tmpl)
		return NULL;

	expr = expr_alloc(loc, EXPR_EXTHDR, tmpl->dtype,
			  BYTEORDER_BIG_ENDIAN, tmpl->len);
	expr->exthdr.desc	= desc;
	expr->exthdr.tmpl	= tmpl;
	expr->exthdr.op		= NFT_EXTHDR_OP_SCTP;
	expr->exthdr.raw_type	= desc->type;
	expr->exthdr.offset	= tmpl->offset;

	return expr;
}

void sctp_chunk_init_raw(struct expr *expr, uint8_t type, unsigned int off,
			 unsigned int len, uint32_t flags)
{
	const struct proto_hdr_template *tmpl;
	unsigned int i;

	assert(expr->etype == EXPR_EXTHDR);

	expr->len = len;
	expr->exthdr.flags = flags;
	expr->exthdr.offset = off;
	expr->exthdr.op = NFT_EXTHDR_OP_SCTP;

	if (flags & NFT_EXTHDR_F_PRESENT)
		datatype_set(expr, &boolean_type);
	else
		datatype_set(expr, &integer_type);

	if (type >= array_size(sctp_chunk_protocols))
		return;

	expr->exthdr.desc = sctp_chunk_protocols[type];
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
