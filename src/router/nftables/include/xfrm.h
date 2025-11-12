#ifndef NFTABLES_XFRM_H
#define NFTABLES_XFRM_H

struct xfrm_template {
	const char		*token;
	const struct datatype	*dtype;
	unsigned int		len;
	enum byteorder		byteorder;
};

extern const struct xfrm_template xfrm_templates[__NFT_XFRM_KEY_MAX];

extern struct expr *xfrm_expr_alloc(const struct location *loc,
				    uint8_t direction, uint8_t spnum,
				    enum nft_xfrm_keys key);
#endif
