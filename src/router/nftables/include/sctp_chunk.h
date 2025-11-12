/*
 * Copyright Red Hat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#ifndef NFTABLES_SCTP_CHUNK_H
#define NFTABLES_SCTP_CHUNK_H

/* SCTP chunk types used on wire */
enum sctp_hdr_chunk_types {
	SCTP_CHUNK_TYPE_DATA			= 0,
	SCTP_CHUNK_TYPE_INIT			= 1,
	SCTP_CHUNK_TYPE_INIT_ACK		= 2,
	SCTP_CHUNK_TYPE_SACK			= 3,
	SCTP_CHUNK_TYPE_HEARTBEAT		= 4,
	SCTP_CHUNK_TYPE_HEARTBEAT_ACK		= 5,
	SCTP_CHUNK_TYPE_ABORT			= 6,
	SCTP_CHUNK_TYPE_SHUTDOWN		= 7,
	SCTP_CHUNK_TYPE_SHUTDOWN_ACK		= 8,
	SCTP_CHUNK_TYPE_ERROR			= 9,
	SCTP_CHUNK_TYPE_COOKIE_ECHO		= 10,
	SCTP_CHUNK_TYPE_COOKIE_ACK		= 11,
	SCTP_CHUNK_TYPE_ECNE			= 12,
	SCTP_CHUNK_TYPE_CWR			= 13,
	SCTP_CHUNK_TYPE_SHUTDOWN_COMPLETE	= 14,
	SCTP_CHUNK_TYPE_ASCONF_ACK		= 128,
	SCTP_CHUNK_TYPE_FORWARD_TSN		= 192,
	SCTP_CHUNK_TYPE_ASCONF			= 193,
};

enum sctp_hdr_chunk_common_fields {
	SCTP_CHUNK_COMMON_TYPE,
	SCTP_CHUNK_COMMON_FLAGS,
	SCTP_CHUNK_COMMON_LENGTH,
	__SCTP_CHUNK_COMMON_MAX,
};

#define SCTP_CHUNK_START_INDEX	__SCTP_CHUNK_COMMON_MAX

enum sctp_hdr_chunk_data_fields {
	SCTP_CHUNK_DATA_TSN = SCTP_CHUNK_START_INDEX,
	SCTP_CHUNK_DATA_STREAM,
	SCTP_CHUNK_DATA_SSN,
	SCTP_CHUNK_DATA_PPID,
};

enum sctp_hdr_chunk_init_fields {
	SCTP_CHUNK_INIT_TAG = SCTP_CHUNK_START_INDEX,
	SCTP_CHUNK_INIT_RWND,
	SCTP_CHUNK_INIT_OSTREAMS,
	SCTP_CHUNK_INIT_ISTREAMS,
	SCTP_CHUNK_INIT_TSN,
};

enum sctp_hdr_chunk_sack_fields {
	SCTP_CHUNK_SACK_CTSN_ACK = SCTP_CHUNK_START_INDEX,
	SCTP_CHUNK_SACK_RWND,
	SCTP_CHUNK_SACK_GACK_BLOCKS,
	SCTP_CHUNK_SACK_DUP_TSNS,
};

enum sctp_hdr_chunk_shutdown_fields {
	SCTP_CHUNK_SHUTDOWN_CTSN_ACK = SCTP_CHUNK_START_INDEX,
};

enum sctp_hdr_chunk_ecne_cwr_fields {
	SCTP_CHUNK_ECNE_CWR_MIN_TSN = SCTP_CHUNK_START_INDEX,
};

enum sctp_hdr_chunk_asconf_fields {
	SCTP_CHUNK_ASCONF_SEQNO = SCTP_CHUNK_START_INDEX,
};

enum sctp_hdr_chunk_fwd_tsn_fields {
	SCTP_CHUNK_FORWARD_TSN_NCTSN = SCTP_CHUNK_START_INDEX,
};

struct expr *sctp_chunk_expr_alloc(const struct location *loc,
				   unsigned int type, unsigned int field);
void sctp_chunk_init_raw(struct expr *expr, uint8_t type, unsigned int off,
			 unsigned int len, uint32_t flags);
const struct exthdr_desc *sctp_chunk_protocol_find(const char *name);

#endif /* NFTABLES_SCTP_CHUNK_H */
