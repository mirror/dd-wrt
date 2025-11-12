#ifndef NFTABLES_TCPOPT_H
#define NFTABLES_TCPOPT_H

#include <proto.h>
#include <exthdr.h>
#include <statement.h>

extern struct expr *tcpopt_expr_alloc(const struct location *loc,
				      unsigned int kind, unsigned int field);

extern void tcpopt_init_raw(struct expr *expr, uint8_t type,
			    unsigned int offset, unsigned int len,
			    uint32_t flags);

extern bool tcpopt_find_template(struct expr *expr, unsigned int offset,
				 unsigned int len);

/* TCP option numbers used on wire */
enum tcpopt_kind {
	TCPOPT_KIND_EOL = 0,
	TCPOPT_KIND_NOP = 1,
	TCPOPT_KIND_MAXSEG = 2,
	TCPOPT_KIND_WINDOW = 3,
	TCPOPT_KIND_SACK_PERMITTED = 4,
	TCPOPT_KIND_SACK = 5,
	TCPOPT_KIND_TIMESTAMP = 8,
	TCPOPT_KIND_ECHO = 8,
	TCPOPT_KIND_MD5SIG = 19,
	TCPOPT_KIND_MPTCP = 30,
	TCPOPT_KIND_FASTOPEN = 34,
	__TCPOPT_KIND_MAX,

	/* extra oob info, internal to nft */
	TCPOPT_KIND_SACK1 = 256,
	TCPOPT_KIND_SACK2 = 257,
	TCPOPT_KIND_SACK3 = 258,
};

/* Internal identifiers */
enum tcpopt_common {
	TCPOPT_COMMON_KIND,
	TCPOPT_COMMON_LENGTH,
};

enum tcpopt_maxseg {
	TCPOPT_MAXSEG_KIND,
	TCPOPT_MAXSEG_LENGTH,
	TCPOPT_MAXSEG_SIZE,
};

enum tcpopt_timestamp {
	TCPOPT_TS_KIND,
	TCPOPT_TS_LENGTH,
	TCPOPT_TS_TSVAL,
	TCPOPT_TS_TSECR,
};

enum tcpopt_windowscale {
	TCPOPT_WINDOW_KIND,
	TCPOPT_WINDOW_LENGTH,
	TCPOPT_WINDOW_COUNT,
};

enum tcpopt_hdr_field_sack {
	TCPOPT_SACK_KIND,
	TCPOPT_SACK_LENGTH,
	TCPOPT_SACK_LEFT,
	TCPOPT_SACK_RIGHT,
	TCPOPT_SACK_LEFT1,
	TCPOPT_SACK_RIGHT1,
	TCPOPT_SACK_LEFT2,
	TCPOPT_SACK_RIGHT2,
	TCPOPT_SACK_LEFT3,
	TCPOPT_SACK_RIGHT3,
};

enum tcpopt_hdr_mptcp_common {
	TCPOPT_MPTCP_KIND,
	TCPOPT_MPTCP_LENGTH,
	TCPOPT_MPTCP_SUBTYPE,
};

extern const struct exthdr_desc *tcpopt_protocols[__TCPOPT_KIND_MAX];

#endif /* NFTABLES_TCPOPT_H */
