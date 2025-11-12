#ifndef NFTABLES_SOCKET_H
#define NFTABLES_SOCKET_H

/**
 * struct socket_template - template for routing expressions
 *
 * @token:	parser token for the expression
 * @dtype:	data type of the expression
 * @len:	length of the expression
 * @byteorder:	byteorder
 */
struct socket_template {
	const char		*token;
	const struct datatype	*dtype;
	unsigned int		len;
	enum byteorder		byteorder;
};

extern const struct socket_template socket_templates[];

extern struct expr *socket_expr_alloc(const struct location *loc,
				      enum nft_socket_keys key, uint32_t level);

#endif /* NFTABLES_SOCKET_H */
