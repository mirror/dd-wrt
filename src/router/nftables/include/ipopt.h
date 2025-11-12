#ifndef NFTABLES_IPOPT_H
#define NFTABLES_IPOPT_H

#include <proto.h>
#include <exthdr.h>
#include <statement.h>

extern struct expr *ipopt_expr_alloc(const struct location *loc,
				      uint8_t type, uint8_t field);

extern void ipopt_init_raw(struct expr *expr, uint8_t type,
			    unsigned int offset, unsigned int len,
			    uint32_t flags, bool set_unknown);

extern bool ipopt_find_template(struct expr *expr, unsigned int offset,
			  unsigned int len);

enum ipopt_fields {
	IPOPT_FIELD_INVALID,
	IPOPT_FIELD_TYPE,
	IPOPT_FIELD_LENGTH,
	IPOPT_FIELD_VALUE,
	IPOPT_FIELD_PTR,
	IPOPT_FIELD_ADDR_0,
};

extern const struct exthdr_desc *ipopt_protocols[UINT8_MAX];

#endif /* NFTABLES_IPOPT_H */
