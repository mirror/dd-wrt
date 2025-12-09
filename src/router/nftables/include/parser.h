#ifndef NFTABLES_PARSER_H
#define NFTABLES_PARSER_H

#include <list.h>
#include <rule.h> // FIXME
#include <nftables.h>

#define TABSIZE				8

#define YYLTYPE				struct location
#define YYLTYPE_IS_TRIVIAL		0
#define YYENABLE_NLS			0

#define SCOPE_NEST_MAX			4

struct parser_state {
	struct input_descriptor		*indesc;
	struct list_head		indesc_list;

	struct list_head		*msgs;
	unsigned int			nerrs;

	struct scope			*scopes[SCOPE_NEST_MAX];
	unsigned int			scope;
	bool				scope_err;

	unsigned int			flex_state_pop;
	unsigned int			startcond_type;
	struct list_head		*cmds;
	unsigned int			*startcond_active;
};

enum startcond_type {
	PARSER_SC_BEGIN,
	PARSER_SC_ARP,
	PARSER_SC_AT,
	PARSER_SC_CT,
	PARSER_SC_COUNTER,
	PARSER_SC_ETH,
	PARSER_SC_GRE,
	PARSER_SC_ICMP,
	PARSER_SC_IGMP,
	PARSER_SC_IP,
	PARSER_SC_IP6,
	PARSER_SC_LAST,
	PARSER_SC_LIMIT,
	PARSER_SC_META,
	PARSER_SC_POLICY,
	PARSER_SC_QUOTA,
	PARSER_SC_SCTP,
	PARSER_SC_SECMARK,
	PARSER_SC_TCP,
	PARSER_SC_TYPE,
	PARSER_SC_TUNNEL,
	PARSER_SC_VLAN,
	PARSER_SC_XT,
	PARSER_SC_CMD_DESTROY,
	PARSER_SC_CMD_EXPORT,
	PARSER_SC_CMD_IMPORT,
	PARSER_SC_CMD_LIST,
	PARSER_SC_CMD_MONITOR,
	PARSER_SC_CMD_RESET,
	PARSER_SC_EXPR_AH,
	PARSER_SC_EXPR_COMP,
	PARSER_SC_EXPR_DCCP,
	PARSER_SC_EXPR_DST,
	PARSER_SC_EXPR_ESP,
	PARSER_SC_EXPR_FIB,
	PARSER_SC_EXPR_FRAG,
	PARSER_SC_EXPR_HASH,
	PARSER_SC_EXPR_HBH,
	PARSER_SC_EXPR_IPSEC,
	PARSER_SC_EXPR_MH,
	PARSER_SC_EXPR_NUMGEN,
	PARSER_SC_EXPR_OSF,
	PARSER_SC_EXPR_QUEUE,
	PARSER_SC_EXPR_RT,
	PARSER_SC_EXPR_SCTP_CHUNK,
	PARSER_SC_EXPR_SOCKET,
	PARSER_SC_EXPR_TH,
	PARSER_SC_EXPR_UDP,
	PARSER_SC_EXPR_UDPLITE,

	PARSER_SC_STMT_DUP,
	PARSER_SC_STMT_FWD,
	PARSER_SC_STMT_LOG,
	PARSER_SC_STMT_NAT,
	PARSER_SC_STMT_REJECT,
	PARSER_SC_STMT_SYNPROXY,
	PARSER_SC_STMT_TPROXY,

	__SC_MAX
};

struct mnl_socket;

extern void parser_init(struct nft_ctx *nft, struct parser_state *state,
			struct list_head *msgs, struct list_head *cmds,
			struct scope *top_scope);
extern int nft_parse(struct nft_ctx *ctx, void *, struct parser_state *state);

extern void *scanner_init(struct parser_state *state);
extern void scanner_destroy(struct nft_ctx *nft);

extern int scanner_read_file(struct nft_ctx *nft, const char *filename,
			     const struct location *loc);
extern int scanner_include_file(struct nft_ctx *ctx, void *scanner,
				const char *filename,
				const struct location *loc);
extern void scanner_push_buffer(void *scanner,
				const struct input_descriptor *indesc,
				const char *buffer);

extern void scanner_pop_start_cond(void *scanner, enum startcond_type sc);

const char *str_preprocess(struct parser_state *state, struct location *loc,
			   struct scope *scope, const char *x,
			   struct error_record **rec);

#endif /* NFTABLES_PARSER_H */
