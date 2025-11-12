#ifndef NFTABLES_RT_H
#define NFTABLES_RT_H

/**
 * struct rt_template - template for routing expressions
 *
 * @token:	parser token for the expression
 * @dtype:	data type of the expression
 * @len:	length of the expression
 * @byteorder:	byteorder
 * @invalid:	invalidate datatype on allocation from parser
 */
struct rt_template {
	const char		*token;
	const struct datatype	*dtype;
	unsigned int		len;
	enum byteorder		byteorder;
	bool			invalid;
};

extern const struct rt_template rt_templates[];

#define RT_TEMPLATE(__token, __dtype, __len, __byteorder, __invalid) {	\
	.token		= (__token),					\
	.dtype		= (__dtype),					\
	.len		= (__len),					\
	.byteorder	= (__byteorder),				\
	.invalid	= (__invalid),					\
}

extern struct expr *rt_expr_alloc(const struct location *loc,
				  enum nft_rt_keys key, bool invalid);
extern void rt_expr_update_type(struct proto_ctx *ctx, struct expr *expr);

extern const struct datatype realm_type;

#endif /* NFTABLES_RT_H */
