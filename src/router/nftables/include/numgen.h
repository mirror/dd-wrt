#ifndef NFTABLES_NUMGEN_H
#define NFTABLES_NUMGEN_H

extern struct expr *numgen_expr_alloc(const struct location *loc,
				      enum nft_ng_types type, uint32_t until,
				      uint32_t offset);

#endif /* NFTABLES_NUMGEN_H */
