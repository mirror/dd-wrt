#ifndef NFTABLES_OSF_H
#define NFTABLES_OSF_H

struct expr *osf_expr_alloc(const struct location *loc, const uint8_t ttl,
			    const uint32_t flags);

extern int nfnl_osf_load_fingerprints(struct netlink_ctx *ctx, int del);

#endif /* NFTABLES_OSF_H */
