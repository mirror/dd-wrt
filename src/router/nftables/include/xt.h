#ifndef _NFT_XT_H_
#define _NFT_XT_H_

struct netlink_linearize_ctx;
struct netlink_parse_ctx;
struct nftnl_expr;
struct rule_pp_ctx;
struct rule;
struct output_ctx;

void xt_stmt_xlate(const struct stmt *stmt, struct output_ctx *octx);
void xt_stmt_destroy(struct stmt *stmt);

void netlink_parse_target(struct netlink_parse_ctx *ctx,
			  const struct location *loc,
			  const struct nftnl_expr *nle);
void netlink_parse_match(struct netlink_parse_ctx *ctx,
			 const struct location *loc,
			 const struct nftnl_expr *nle);
#ifdef HAVE_LIBXTABLES
void stmt_xt_postprocess(struct rule_pp_ctx *rctx, struct stmt *stmt,
			 struct rule *rule);
#else
static inline void stmt_xt_postprocess(struct rule_pp_ctx *rctx,
				       struct stmt *stmt, struct rule *rule) {}

#endif

#endif /* _NFT_XT_H_ */
