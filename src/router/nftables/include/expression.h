#ifndef NFTABLES_EXPRESSION_H
#define NFTABLES_EXPRESSION_H

#include <gmputil.h>
#include <linux/netfilter/nf_tables.h>

#include <nftables.h>
#include <datatype.h>
#include <utils.h>
#include <list.h>
#include <json.h>
#include <libnftnl/udata.h>

#define NFT_MAX_EXPR_LEN_BYTES (NFT_REG32_COUNT * sizeof(uint32_t))
#define NFT_MAX_EXPR_LEN_BITS  (NFT_MAX_EXPR_LEN_BYTES * BITS_PER_BYTE)
#define NFT_MAX_EXPR_RECURSION 16

/**
 * enum expr_types
 *
 * @EXPR_INVALID:	uninitialized type, should not happen
 * @EXPR_VERDICT:	nftables verdict expression
 * @EXPR_SYMBOL:	unparsed symbol
 * @EXPR_VARIABLE:	variable
 * @EXPR_VALUE:		literal numeric or string expression
 * @EXPR_PREFIX:	prefixed expression
 * @EXPR_RANGE:		literal range
 * @EXPR_PAYLOAD:	payload expression
 * @EXPR_EXTHDR:	exthdr expression
 * @EXPR_META:		meta expression
 * @EXPR_SOCKET:	socket expression
 * @EXPR_OSF:		osf expression
 * @EXPR_CT:		conntrack expression
 * @EXPR_CONCAT:	concatenation
 * @EXPR_LIST:		list of expressions
 * @EXPR_SET:		literal set
 * @EXPR_SET_REF:	set reference
 * @EXPR_SET_ELEM:	set element
 * @EXPR_MAPPING:	a single mapping (key : value)
 * @EXPR_MAP:		map operation (expr map { EXPR_MAPPING, ... })
 * @EXPR_UNARY:		byteorder conversion, generated during evaluation
 * @EXPR_BINOP:		binary operations (bitwise, shifts)
 * @EXPR_RELATIONAL:	equality and relational expressions
 * @EXPR_NUMGEN:	number generation expression
 * @EXPR_HASH:		hash expression
 * @EXPR_RT:		routing expression
 * @EXPR_FIB		forward information base expression
 * @EXPR_XFRM		XFRM (ipsec) expression
 * @EXPR_SET_ELEM_CATCHALL catchall element expression
 * @EXPR_RANGE_VALUE	constant range expression
 * @EXPR_RANGE_SYMBOL	unparse symbol range expression
 */
enum expr_types {
	EXPR_INVALID,
	EXPR_VERDICT,
	EXPR_SYMBOL,
	EXPR_VARIABLE,
	EXPR_VALUE,
	EXPR_PREFIX,
	EXPR_RANGE,
	EXPR_PAYLOAD,
	EXPR_EXTHDR,
	EXPR_META,
	EXPR_SOCKET,
	EXPR_OSF,
	EXPR_CT,
	EXPR_CONCAT,
	EXPR_LIST,
	EXPR_SET,
	EXPR_SET_REF,
	EXPR_SET_ELEM,
	EXPR_MAPPING,
	EXPR_MAP,
	EXPR_UNARY,
	EXPR_BINOP,
	EXPR_RELATIONAL,
	EXPR_NUMGEN,
	EXPR_HASH,
	EXPR_RT,
	EXPR_TUNNEL,
	EXPR_FIB,
	EXPR_XFRM,
	EXPR_SET_ELEM_CATCHALL,
	EXPR_RANGE_VALUE,
	EXPR_RANGE_SYMBOL,
	__EXPR_MAX
};
#define EXPR_MAX	(__EXPR_MAX - 1)

enum ops {
	OP_INVALID,
	OP_IMPLICIT,
	/* Unary operations */
	OP_HTON,
	OP_NTOH,
	/* Binary operations */
	OP_LSHIFT,
	OP_RSHIFT,
	OP_AND,
	OP_XOR,
	OP_OR,
	/* Relational operations */
	OP_EQ,
	OP_NEQ,
	OP_LT,
	OP_GT,
	OP_LTE,
	OP_GTE,
	OP_NEG,
	__OP_MAX
};
#define OP_MAX		(__OP_MAX - 1)

extern const char *expr_op_symbols[];

enum symbol_types {
	SYMBOL_VALUE,
	SYMBOL_SET,
};

/**
 * struct expr_ctx - type context for symbol parsing during evaluation
 *
 * @dtype:	expected datatype
 * @byteorder:	expected byteorder
 * @len:	expected len
 * @maxval:	expected maximum value
 */
struct expr_ctx {
	/* expr_ctx does not own the reference to dtype. The caller must ensure
	 * the valid lifetime.
	 */
	const struct datatype	*dtype;

	enum byteorder		byteorder;
	unsigned int		len;
	unsigned int		maxval;
	const struct expr	*key;
};

static inline void __expr_set_context(struct expr_ctx *ctx,
				      const struct datatype *dtype,
				      enum byteorder byteorder,
				      unsigned int len, unsigned int maxval)
{
	ctx->dtype	= dtype;
	ctx->byteorder	= byteorder;
	ctx->len	= len;
	ctx->maxval	= maxval;
	ctx->key	= NULL;
}

static inline void expr_set_context(struct expr_ctx *ctx,
				    const struct datatype *dtype,
				    unsigned int len)
{
	__expr_set_context(ctx, dtype,
			   dtype ? dtype->byteorder : BYTEORDER_INVALID,
			   len, 0);
}

/**
 * struct expr_ops
 *
 * @type:	expression type
 * @name:	expression name for diagnostics
 * @clone:	function to clone type specific data
 * @destroy:	destructor, must release inner expressions
 * @set_type:	function to promote type and byteorder of inner types
 * @print:	function to print the expression
 * @cmp:	function to compare two expressions of the same types
 * @pctx_update:update protocol context
 */
struct proto_ctx;
struct expr_ops {
	enum expr_types		type;
	const char		*name;
	void			(*clone)(struct expr *new, const struct expr *expr);
	void			(*destroy)(struct expr *expr);
	void			(*set_type)(const struct expr *expr,
					    const struct datatype *dtype,
					    enum byteorder byteorder);
	void			(*print)(const struct expr *expr,
					 struct output_ctx *octx);
	json_t			*(*json)(const struct expr *expr,
					 struct output_ctx *octx);
	bool			(*cmp)(const struct expr *e1,
				       const struct expr *e2);
	void			(*pctx_update)(struct proto_ctx *ctx,
					       const struct location *loc,
					       const struct expr *left,
					       const struct expr *right);
	int			(*build_udata)(struct nftnl_udata_buf *udbuf,
					       const struct expr *expr);
	struct expr *		(*parse_udata)(const struct nftnl_udata *ud);
};

const struct expr_ops *expr_ops(const struct expr *e);
const struct expr_ops *expr_ops_by_type_u32(uint32_t value);

/**
 * enum expr_flags
 *
 * @EXPR_F_CONSTANT:		constant expression
 * @EXPR_F_SINGLETON:		singleton (implies primary and constant)
 * @EXPR_F_PROTOCOL:		expressions describes upper layer protocol
 * @EXPR_F_INTERVAL_END:	set member ends an open interval
 * @EXPR_F_BOOLEAN:		expression is boolean (set by relational expr on LHS)
 * @EXPR_F_INTERVAL:		expression describes a interval
 * @EXPR_F_KERNEL:		expression resides in the kernel
 */
enum expr_flags {
	EXPR_F_CONSTANT		= 0x1,
	EXPR_F_SINGLETON	= 0x2,
	EXPR_F_PROTOCOL		= 0x4,
	EXPR_F_INTERVAL_END	= 0x8,
	EXPR_F_BOOLEAN		= 0x10,
	EXPR_F_INTERVAL		= 0x20,
	EXPR_F_KERNEL		= 0x40,
	EXPR_F_REMOVE		= 0x80,
	EXPR_F_INTERVAL_OPEN	= 0x100,
};

#include <payload.h>
#include <exthdr.h>
#include <fib.h>
#include <numgen.h>
#include <meta.h>
#include <rt.h>
#include <hash.h>
#include <ct.h>
#include <socket.h>
#include <tunnel.h>
#include <osf.h>
#include <xfrm.h>

/**
 * struct expr
 *
 * @list:	list node
 * @location:	location from parser
 * @refcnt:	reference count
 * @flags:	mask of enum expr_flags
 * @dtype:	data type of expression
 * @byteorder:	byteorder of expression
 * @etype:	expression type
 * @op:		operation for unary, binary and relational expressions
 * @len:	length of expression
 * @union:	type specific data
 */
struct expr {
	struct list_head	list;
	struct location		location;

	unsigned int		refcnt;
	unsigned int		flags;

	const struct datatype	*dtype;
	enum byteorder		byteorder:8;
	enum expr_types		etype:8;
	enum ops		op:8;
	unsigned int		len;

	union {
		struct {
			/* EXPR_SYMBOL, EXPR_RANGE_SYMBOL */
			const struct scope	*scope;
			union {
				const char	*identifier;
				const char	*identifier_range[2];
			};
			enum symbol_types	symtype;
		};
		struct {
			/* EXPR_VARIABLE */
			struct symbol		*sym;
		};
		struct {
			/* EXPR_VERDICT */
			int			verdict;
			struct expr		*chain;
			uint32_t		chain_id;
		};
		struct {
			/* EXPR_VALUE */
			mpz_t			value;
		};
		struct {
			/* EXPR_RANGE_VALUE */
			mpz_t			low;
			mpz_t			high;
		} range;
		struct {
			/* EXPR_PREFIX */
			struct expr		*prefix;
			unsigned int		prefix_len;
		};
		struct expr_concat {
			/* EXPR_CONCAT */
			struct list_head	expressions;
			unsigned int		size;
			uint8_t			field_len[NFT_REG32_COUNT];
			uint8_t			field_count;
		} expr_concat;
		struct expr_set {
			/* EXPR_SET */
			struct list_head	expressions;
			unsigned int		size;
			uint32_t		set_flags;
		} expr_set;
		struct expr_list {
			/* EXPR_LIST */
			struct list_head	expressions;
			unsigned int		size;
		} expr_list;
		struct {
			/* EXPR_SET_REF */
			struct set		*set;
		};
		struct {
			/* EXPR_SET_ELEM */
			struct expr		*key;
			uint64_t		timeout;
			uint64_t		expiration;
			const char		*comment;
			struct list_head	stmt_list;
		};
		struct {
			/* EXPR_UNARY */
			struct expr		*arg;
		};
		struct {
			/* EXPR_RANGE, EXPR_BINOP, EXPR_MAPPING, EXPR_RELATIONAL */
			struct expr		*left;
			struct expr		*right;
		};
		struct {
			/* EXPR_MAP */
			struct expr		*map;
			struct expr		*mappings;
		};

		struct {
			/* EXPR_PAYLOAD */
			const struct proto_desc		*desc;
			const struct proto_hdr_template	*tmpl;
			const struct proto_desc		*inner_desc;
			enum proto_bases		base;
			unsigned int			offset;
			bool				is_raw;
			bool				evaluated;
		} payload;
		struct {
			/* EXPR_EXTHDR */
			const struct exthdr_desc	*desc;
			const struct proto_hdr_template	*tmpl;
			uint16_t			offset;
			uint8_t				raw_type;
			enum nft_exthdr_op		op;
			unsigned int			flags;
		} exthdr;
		struct {
			/* EXPR_META */
			enum nft_meta_keys	key;
			enum proto_bases	base;
			const struct proto_desc	*inner_desc;
		} meta;
		struct {
			/* SOCKET */
			enum nft_socket_keys	key;
			uint32_t		level;
		} socket;
		struct {
			/* EXPR_TUNNEL */
			enum nft_tunnel_keys	key;
		} tunnel;
		struct {
			/* EXPR_RT */
			enum nft_rt_keys	key;
		} rt;
		struct {
			/* EXPR_CT */
			enum nft_ct_keys	key;
			enum proto_bases	base;
			int8_t			direction;
			uint8_t			nfproto;
		} ct;
		struct {
			/* EXPR_NUMGEN */
			enum nft_ng_types	type;
			uint32_t		mod;
			uint32_t		offset;
		} numgen;
		struct {
			/* EXPR_HASH */
			struct expr		*expr;
			uint32_t		mod;
			bool			seed_set;
			uint32_t		seed;
			uint32_t		offset;
			enum nft_hash_types	type;
		} hash;
		struct {
			/* EXPR_FIB */
			uint32_t		flags;
			uint32_t		result;
		} fib;
		struct {
			/* EXPR_XFRM */
			enum nft_xfrm_keys	key;
			uint8_t		direction;
			uint8_t		spnum;
		} xfrm;
		struct {
			/* EXPR_OSF */
			uint8_t			ttl;
			uint32_t		flags;
		} osf;
	};
};

#define expr_set(__expr)	(assert((__expr)->etype == EXPR_SET), &(__expr)->expr_set)
#define expr_concat(__expr)	(assert((__expr)->etype == EXPR_CONCAT), &(__expr)->expr_concat)
#define expr_list(__expr)	(assert((__expr)->etype == EXPR_LIST), &(__expr)->expr_list)

extern struct expr *expr_alloc(const struct location *loc,
			       enum expr_types etype,
			       const struct datatype *dtype,
			       enum byteorder byteorder, unsigned int len);
extern struct expr *expr_clone(const struct expr *expr);
extern struct expr *expr_get(struct expr *expr);
extern void expr_free(struct expr *expr);
extern void expr_print(const struct expr *expr, struct output_ctx *octx);
extern bool expr_cmp(const struct expr *e1, const struct expr *e2);
extern void expr_describe(const struct expr *expr, struct output_ctx *octx);

extern const struct datatype *expr_basetype(const struct expr *expr);
extern void expr_set_type(struct expr *expr, const struct datatype *dtype,
			  enum byteorder byteorder);

struct eval_ctx;
extern int expr_binary_error(struct list_head *msgs,
			     const struct expr *e1, const struct expr *e2,
			     const char *fmt, ...) __fmtstring(4, 5);

#define expr_error(msgs, expr, fmt, args...) \
	expr_binary_error(msgs, expr, NULL, fmt, ## args)

static inline bool expr_is_constant(const struct expr *expr)
{
	return expr->flags & EXPR_F_CONSTANT ? true : false;
}

static inline bool expr_is_singleton(const struct expr *expr)
{
	return expr->flags & EXPR_F_SINGLETON ? true : false;
}

extern struct expr *unary_expr_alloc(const struct location *loc,
				     enum ops op, struct expr *arg);

extern struct expr *binop_expr_alloc(const struct location *loc, enum ops op,
				     struct expr *left, struct expr *right);

extern bool must_print_eq_op(const struct expr *expr);

extern struct expr *relational_expr_alloc(const struct location *loc, enum ops op,
					  struct expr *left, struct expr *right);

extern void relational_expr_pctx_update(struct proto_ctx *ctx,
					const struct expr *expr);

extern struct expr *verdict_expr_alloc(const struct location *loc,
				       int verdict, struct expr *chain);

extern struct expr *symbol_expr_alloc(const struct location *loc,
				      enum symbol_types type, struct scope *scope,
				      const char *identifier);
#define is_symbol_value_expr(expr) \
	((expr)->etype == EXPR_SYMBOL && (expr)->symtype == SYMBOL_VALUE)

const char *expr_name(const struct expr *e);

static inline void symbol_expr_set_type(struct expr *expr,
					const struct datatype *dtype)
{
	if (expr->etype == EXPR_SYMBOL)
		datatype_set(expr, dtype);
}

struct expr *variable_expr_alloc(const struct location *loc,
				 struct scope *scope, struct symbol *sym);

extern struct expr *constant_expr_alloc(const struct location *loc,
					const struct datatype *dtype,
					enum byteorder byteorder,
					unsigned int len, const void *data);
extern struct expr *constant_expr_join(const struct expr *e1,
				       const struct expr *e2);
extern struct expr *constant_expr_splice(struct expr *expr, unsigned int len);

extern struct expr *constant_range_expr_alloc(const struct location *loc,
					      const struct datatype *dtype,
					      enum byteorder byteorder,
					      unsigned int len,
					      mpz_t low, mpz_t high);

struct expr *symbol_range_expr_alloc(const struct location *loc,
				     enum symbol_types type, const struct scope *scope,
				     const char *identifier_low, const char *identifier_high);

extern struct expr *flag_expr_alloc(const struct location *loc,
				    const struct datatype *dtype,
				    enum byteorder byteorder,
				    unsigned int len, unsigned long n);
extern struct expr *bitmask_expr_to_binops(struct expr *expr);

extern struct expr *prefix_expr_alloc(const struct location *loc,
				      struct expr *expr,
				      unsigned int prefix_len);

extern struct expr *range_expr_alloc(const struct location *loc,
				     struct expr *low, struct expr *high);
struct expr *range_expr_to_prefix(struct expr *range);

extern void list_expr_sort(struct list_head *head);
extern void list_splice_sorted(struct list_head *list, struct list_head *head);

extern struct expr *concat_expr_alloc(const struct location *loc);
void concat_expr_add(struct expr *concat, struct expr *item);
void concat_expr_remove(struct expr *concat, struct expr *expr);

extern struct expr *list_expr_alloc(const struct location *loc);
void list_expr_add(struct expr *expr, struct expr *item);
void list_expr_remove(struct expr *expr, struct expr *item);
struct expr *list_expr_to_binop(struct expr *expr);

extern struct expr *set_expr_alloc(const struct location *loc,
				   const struct set *set);
void __set_expr_add(struct expr *set, struct expr *elem);
void set_expr_add(struct expr *set, struct expr *elem);
void set_expr_remove(struct expr *expr, struct expr *item);

extern void concat_range_aggregate(struct expr *set);
extern void interval_map_decompose(struct expr *set);

extern struct expr *get_set_intervals(const struct set *set,
				      const struct expr *init);
struct table;
extern int get_set_decompose(struct set *cache_set, struct set *set);

extern struct expr *mapping_expr_alloc(const struct location *loc,
				       struct expr *from, struct expr *to);
extern struct expr *map_expr_alloc(const struct location *loc,
				   struct expr *arg, struct expr *list);

extern struct expr *set_ref_expr_alloc(const struct location *loc,
				       struct set *set);

extern struct expr *set_elem_expr_alloc(const struct location *loc,
					struct expr *key);

struct expr *set_elem_catchall_expr_alloc(const struct location *loc);

#define expr_type_catchall(__expr)			\
	((__expr)->etype == EXPR_SET_ELEM_CATCHALL)

extern void range_expr_value_low(mpz_t rop, const struct expr *expr);
extern void range_expr_value_high(mpz_t rop, const struct expr *expr);
void range_expr_swap_values(struct expr *range);

#endif /* NFTABLES_EXPRESSION_H */
