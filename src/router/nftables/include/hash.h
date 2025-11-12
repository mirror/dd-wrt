#ifndef NFTABLES_HASH_H
#define NFTABLES_HASH_H

extern struct expr *hash_expr_alloc(const struct location *loc,
				    uint32_t modulus,
				    bool seed_set, uint32_t seed,
				    uint32_t offset,
				    enum nft_hash_types type);

#endif /* NFTABLES_HASH_H */
