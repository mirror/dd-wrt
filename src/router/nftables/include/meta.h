#ifndef NFTABLES_META_H
#define NFTABLES_META_H

/**
 * struct meta_template - template for meta expressions and statements
 *
 * @token:	parser token for the expression
 * @dtype:	data type of the expression
 * @len:	length of the expression
 * @byteorder:	byteorder
 */
struct meta_template {
	const char		*token;
	const struct datatype	*dtype;
	enum byteorder		byteorder;
	unsigned int		len;
};

extern const struct meta_template meta_templates[];

#define META_TEMPLATE(__token, __dtype, __len, __byteorder) {	\
	.token		= (__token),				\
	.dtype		= (__dtype),				\
	.len		= (__len),				\
	.byteorder	= (__byteorder),			\
}

extern struct expr *meta_expr_alloc(const struct location *loc,
				    enum nft_meta_keys key);

struct stmt *meta_stmt_meta_iiftype(const struct location *loc, uint16_t type);

struct error_record *meta_key_parse(const struct location *loc,
				    const char *name,
				    unsigned int *value);

extern const struct datatype ifindex_type;
extern const struct datatype tchandle_type;
extern const struct datatype gid_type;
extern const struct datatype uid_type;
extern const struct datatype devgroup_type;
extern const struct datatype pkttype_type;
extern const struct datatype ifname_type;
extern const struct datatype date_type;
extern const struct datatype hour_type;
extern const struct datatype day_type;

bool lhs_is_meta_hour(const struct expr *meta);

extern const struct stmt_ops meta_stmt_ops;

#endif /* NFTABLES_META_H */
