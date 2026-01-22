#ifndef _NFT_COMPAT_H_
#define _NFT_COMPAT_H_

#include <libnftnl/rule.h>

#include <linux/netfilter/x_tables.h>

int nftnl_rule_expr_count(const struct nftnl_rule *r);

enum rule_udata_ext_flags {
	RUE_FLAG_MATCH_TYPE	= (1 << 0),
	RUE_FLAG_TARGET_TYPE	= (1 << 1),
	RUE_FLAG_ZIP		= (1 << 7),
};
#define RUE_FLAG_TYPE_BITS	(RUE_FLAG_MATCH_TYPE | RUE_FLAG_TARGET_TYPE)

struct rule_udata_ext {
	uint8_t start_idx;
	uint8_t end_idx;
	uint8_t flags;
	uint16_t orig_size;
	uint16_t size;
	unsigned char data[];
};

struct nft_handle;

void rule_add_udata_ext(struct nft_handle *h, struct nftnl_rule *r,
			uint16_t start_idx, uint16_t end_idx,
			uint8_t flags, uint16_t size, const void *data);
static inline void
rule_add_udata_match(struct nft_handle *h, struct nftnl_rule *r,
		     uint16_t start_idx, uint16_t end_idx,
		     const struct xt_entry_match *m)
{
	rule_add_udata_ext(h, r, start_idx, end_idx,
			   RUE_FLAG_MATCH_TYPE, m->u.match_size, m);
}

static inline void
rule_add_udata_target(struct nft_handle *h, struct nftnl_rule *r,
		      uint16_t start_idx, uint16_t end_idx,
		      const struct xt_entry_target *t)
{
	rule_add_udata_ext(h, r, start_idx, end_idx,
			   RUE_FLAG_TARGET_TYPE, t->u.target_size, t);
}

struct nft_xt_ctx;

bool rule_has_udata_ext(const struct nftnl_rule *r);
bool rule_parse_udata_ext(struct nft_xt_ctx *ctx, const struct nftnl_rule *r);

#endif /* _NFT_COMPAT_H_ */
