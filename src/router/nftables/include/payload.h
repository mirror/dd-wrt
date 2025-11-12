#ifndef NFTABLES_PAYLOAD_H
#define NFTABLES_PAYLOAD_H

#include <nftables.h>
#include <proto.h>

extern struct expr *payload_expr_alloc(const struct location *loc,
				       const struct proto_desc *desc,
				       unsigned int type);
extern void payload_init_raw(struct expr *expr, enum proto_bases base,
			     unsigned int offset, unsigned int len);
extern unsigned int payload_hdr_field(const struct expr *expr);

struct eval_ctx;
struct stmt;
extern int payload_gen_dependency(struct eval_ctx *ctx, const struct expr *expr,
				  struct stmt **res);
int payload_gen_inner_dependency(struct eval_ctx *ctx, const struct expr *expr,
				 struct stmt **res);
extern int payload_gen_icmp_dependency(struct eval_ctx *ctx,
				       const struct expr *expr,
				       struct stmt **res);
extern int exthdr_gen_dependency(struct eval_ctx *ctx, const struct expr *expr,
				 const struct proto_desc *dependency,
				 enum proto_bases pb, struct stmt **res);

/**
 * struct payload_dep_ctx - payload protocol dependency tracking
 *
 * @icmp_type: extra info for icmp(6) decoding
 * @prev: previous statement
 * @pdeps: last dependency match per protocol layer
 */
struct payload_dep_ctx {
	uint8_t		icmp_type;
	struct stmt	*prev;
	struct stmt	*pdeps[PROTO_BASE_MAX + 1];
};

extern bool payload_is_known(const struct expr *expr);
extern bool payload_is_stacked(const struct proto_desc *desc,
			       const struct expr *expr);

void payload_dependency_reset(struct payload_dep_ctx *ctx);
extern void payload_dependency_store(struct payload_dep_ctx *ctx,
				     struct stmt *stmt,
				     enum proto_bases base);
extern bool payload_dependency_exists(const struct payload_dep_ctx *ctx,
				      enum proto_bases base);
extern struct expr *payload_dependency_get(struct payload_dep_ctx *ctx,
					   enum proto_bases base);
extern void payload_dependency_release(struct payload_dep_ctx *ctx,
				       enum proto_bases base);
extern void payload_dependency_kill(struct payload_dep_ctx *ctx,
				    struct expr *expr, unsigned int family);
extern void exthdr_dependency_kill(struct payload_dep_ctx *ctx,
				   struct expr *expr, unsigned int family);

extern bool payload_can_merge(const struct expr *e1, const struct expr *e2);
extern struct expr *payload_expr_join(const struct expr *e1,
				      const struct expr *e2);

bool payload_expr_trim(struct expr *expr, struct expr *mask,
		       const struct proto_ctx *ctx, unsigned int *shift);
bool payload_expr_trim_force(struct expr *expr, struct expr *mask,
			     unsigned int *shift);
bool stmt_payload_expr_trim(struct stmt *stmt, const struct proto_ctx *ctx);
extern void payload_expr_expand(struct list_head *list, struct expr *expr,
				const struct proto_ctx *ctx);
extern void payload_expr_complete(struct expr *expr,
				  const struct proto_ctx *ctx);

bool payload_expr_cmp(const struct expr *e1, const struct expr *e2);

const struct proto_desc *find_proto_desc(const struct nftnl_udata *ud);

extern const struct stmt_ops payload_stmt_ops;

#endif /* NFTABLES_PAYLOAD_H */
