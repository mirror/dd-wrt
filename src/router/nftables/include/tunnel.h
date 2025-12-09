#ifndef NFTABLES_TUNNEL_H
#define NFTABLES_TUNNEL_H

/**
 * struct tunnel_template - template for tunnel expressions
 *
 * @token:	parser token for the expression
 * @dtype:	data type of the expression
 * @len:	length of the expression
 * @byteorder:	byteorder
 */
struct tunnel_template {
	const char		*token;
	const struct datatype	*dtype;
	enum byteorder		byteorder;
	unsigned int		len;
};

extern const struct tunnel_template tunnel_templates[];

#define TUNNEL_TEMPLATE(__token, __dtype, __len, __byteorder) {	\
	.token		= (__token),				\
	.dtype		= (__dtype),				\
	.len		= (__len),				\
	.byteorder	= (__byteorder),			\
}

struct error_record *tunnel_key_parse(const struct location *loc,
			       const char *str,
			       unsigned int *value);

extern struct expr *tunnel_expr_alloc(const struct location *loc,
				      enum nft_tunnel_keys key);

extern const struct expr_ops tunnel_expr_ops;

#endif /* NFTABLES_TUNNEL_H */
