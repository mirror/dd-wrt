#ifndef NFTABLES_DCCPOPT_H
#define NFTABLES_DCCPOPT_H

#include <nftables.h>

#define DCCPOPT_TYPE_MIN 0
#define DCCPOPT_TYPE_MAX UINT8_MAX

enum dccpopt_fields {
	DCCPOPT_FIELD_INVALID,
	DCCPOPT_FIELD_TYPE,
};

enum dccpopt_types {
	DCCPOPT_PADDING			=   0,
	DCCPOPT_MANDATORY		=   1,
	DCCPOPT_SLOW_RECEIVER		=   2,
	DCCPOPT_RESERVED_SHORT		=   3,
	DCCPOPT_CHANGE_L		=  32,
	DCCPOPT_CONFIRM_L		=  33,
	DCCPOPT_CHANGE_R		=  34,
	DCCPOPT_CONFIRM_R		=  35,
	DCCPOPT_INIT_COOKIE		=  36,
	DCCPOPT_NDP_COUNT		=  37,
	DCCPOPT_ACK_VECTOR_NONCE_0	=  38,
	DCCPOPT_ACK_VECTOR_NONCE_1	=  39,
	DCCPOPT_DATA_DROPPED		=  40,
	DCCPOPT_TIMESTAMP		=  41,
	DCCPOPT_TIMESTAMP_ECHO		=  42,
	DCCPOPT_ELAPSED_TIME		=  43,
	DCCPOPT_DATA_CHECKSUM		=  44,
	DCCPOPT_RESERVED_LONG		=  45,
	DCCPOPT_CCID_SPECIFIC           = 128,
};

const struct exthdr_desc *dccpopt_find_desc(uint8_t type);
struct expr *dccpopt_expr_alloc(const struct location *loc, uint8_t type);
void dccpopt_init_raw(struct expr *expr, uint8_t type, unsigned int offset,
		      unsigned int len);

#endif /* NFTABLES_DCCPOPT_H */
