#ifndef NFTABLES_STATEMENT_H
#define NFTABLES_STATEMENT_H

#include <list.h>
#include <expression.h>
#include <json.h>

extern struct stmt *expr_stmt_alloc(const struct location *loc,
				    struct expr *expr);

extern struct stmt *verdict_stmt_alloc(const struct location *loc,
				       struct expr *expr);

struct chain_stmt {
	struct chain		*chain;
	struct expr		*expr;
};

struct stmt *chain_stmt_alloc(const struct location *loc, struct chain *chain,
			      enum nft_verdicts verdict);

struct flow_stmt {
	const char		*table_name;
};

struct stmt *flow_stmt_alloc(const struct location *loc, const char *name);

struct objref_stmt {
	uint32_t		type;
	struct expr		*expr;
};

const char *objref_type_name(uint32_t type);
struct stmt *objref_stmt_alloc(const struct location *loc);

struct connlimit_stmt {
	uint32_t		count;
	uint32_t		flags;
};

extern struct stmt *connlimit_stmt_alloc(const struct location *loc);

struct counter_stmt {
	uint64_t		packets;
	uint64_t		bytes;
};

extern struct stmt *counter_stmt_alloc(const struct location *loc);

struct last_stmt {
	uint64_t		used;
	uint32_t		set;
};

extern struct stmt *last_stmt_alloc(const struct location *loc);

struct exthdr_stmt {
	struct expr			*expr;
	struct expr			*val;
};

extern struct stmt *exthdr_stmt_alloc(const struct location *loc,
				      struct expr *payload, struct expr *expr);

struct payload_stmt {
	struct expr			*expr;
	struct expr			*val;
};

extern struct stmt *payload_stmt_alloc(const struct location *loc,
				       struct expr *payload, struct expr *expr);

#include <meta.h>
struct meta_stmt {
	enum nft_meta_keys		key;
	const struct meta_template	*tmpl;
	struct expr			*expr;
};

extern struct stmt *meta_stmt_alloc(const struct location *loc,
				    enum nft_meta_keys key,
				    struct expr *expr);

enum {
	STMT_LOG_PREFIX		= (1 << 0),
	STMT_LOG_SNAPLEN	= (1 << 1),
	STMT_LOG_GROUP		= (1 << 2),
	STMT_LOG_QTHRESHOLD	= (1 << 3),
	STMT_LOG_LEVEL		= (1 << 4),
};

struct log_stmt {
	const char		*prefix;
	unsigned int		snaplen;
	uint16_t		group;
	uint16_t		qthreshold;
	uint32_t		level;
	uint32_t		logflags;
	uint32_t		flags;
};

extern const char *log_level(uint32_t level);
extern int log_level_parse(const char *level);
extern struct stmt *log_stmt_alloc(const struct location *loc);


struct limit_stmt {
	uint64_t		rate;
	uint64_t		unit;
	enum nft_limit_type	type;
	uint32_t		burst;
	uint32_t		flags;
};

extern struct stmt *limit_stmt_alloc(const struct location *loc);
extern void __limit_stmt_print(const struct limit_stmt *limit);

struct reject_stmt {
	struct expr		*expr;
	enum nft_reject_types	type:8;
	int8_t			icmp_code;
	uint8_t			verbose_print:1;
	unsigned int		family;
};

extern struct stmt *reject_stmt_alloc(const struct location *loc);

enum nft_nat_etypes {
	__NFT_NAT_SNAT = NFT_NAT_SNAT,
	__NFT_NAT_DNAT = NFT_NAT_DNAT,
	NFT_NAT_MASQ,
	NFT_NAT_REDIR,
};

extern const char *nat_etype2str(enum nft_nat_etypes type);

enum {
	STMT_NAT_F_INTERVAL	= (1 << 0),
	STMT_NAT_F_PREFIX	= (1 << 1),
	STMT_NAT_F_CONCAT	= (1 << 2),
};

struct nat_stmt {
	enum nft_nat_etypes	type;
	struct expr		*addr;
	struct expr		*proto;
	uint32_t		flags;
	uint8_t			family;
	uint32_t		type_flags;
};

extern struct stmt *nat_stmt_alloc(const struct location *loc,
				   enum nft_nat_etypes type);

struct optstrip_stmt {
	struct expr	*expr;
};

extern struct stmt *optstrip_stmt_alloc(const struct location *loc, struct expr *e);

struct tproxy_stmt {
	struct expr	*addr;
	struct expr	*port;
	uint8_t		family;
	uint8_t		table_family; /* only used for printing the rule */
};

extern struct stmt *tproxy_stmt_alloc(const struct location *loc);

struct queue_stmt {
	struct expr		*queue;
	uint16_t		flags;
};

extern struct stmt *queue_stmt_alloc(const struct location *loc,
				     struct expr *e, uint16_t flags);

struct quota_stmt {
	uint64_t		bytes;
	uint64_t		used;
	uint32_t		flags;
};

struct stmt *quota_stmt_alloc(const struct location *loc);

#include <ct.h>
struct ct_stmt {
	enum nft_ct_keys		key;
	const struct ct_template	*tmpl;
	struct expr			*expr;
	int8_t				direction;
};

extern struct stmt *ct_stmt_alloc(const struct location *loc,
				  enum nft_ct_keys key,
				  int8_t direction,
				  struct expr *expr);
struct dup_stmt {
	struct expr		*to;
	struct expr		*dev;
};

struct stmt *dup_stmt_alloc(const struct location *loc);
uint32_t dup_stmt_type(const char *type);

struct fwd_stmt {
	uint8_t			family;
	struct expr		*addr;
	struct expr		*dev;
};

struct stmt *fwd_stmt_alloc(const struct location *loc);
uint32_t fwd_stmt_type(const char *type);

struct set_stmt {
	struct expr		*set;
	struct expr		*key;
	struct list_head	stmt_list;
	enum nft_dynset_ops	op;
};

extern const char * const set_stmt_op_names[];

extern struct stmt *set_stmt_alloc(const struct location *loc);

struct map_stmt {
	struct expr		*set;
	struct expr		*key;
	struct expr		*data;
	struct list_head	stmt_list;
	enum nft_dynset_ops	op;
};

extern struct stmt *map_stmt_alloc(const struct location *loc);

struct synproxy_stmt {
	uint16_t	mss;
	uint8_t		wscale;
	uint32_t	flags;
};

extern struct stmt *synproxy_stmt_alloc(const struct location *loc);

struct meter_stmt {
	struct expr		*set;
	struct expr		*key;
	struct stmt		*stmt;
	const char		*name;
	uint32_t		size;
};

extern struct stmt *meter_stmt_alloc(const struct location *loc);

/**
 * enum nft_xt_type - xtables statement types
 *
 * @NFT_XT_MATCH:	match
 * @NFT_XT_TARGET:	target
 * @NFT_XT_WATCHER:	watcher (only for the bridge family)
 */
enum nft_xt_type {
	NFT_XT_MATCH = 0,
	NFT_XT_TARGET,
	NFT_XT_WATCHER,
};
#define NFT_XT_MAX	(NFT_XT_WATCHER + 1)

struct xtables_match;
struct xtables_target;

struct xt_stmt {
	const char			*name;
	enum nft_xt_type		type;
	uint32_t			rev;
	uint32_t			family;
	size_t				infolen;
	void				*info;
	uint32_t			proto;
};

extern struct stmt *xt_stmt_alloc(const struct location *loc);

/**
 * enum stmt_types - statement types
 *
 * @STMT_INVALID:	uninitialised
 * @STMT_EXPRESSION:	expression statement (relational)
 * @STMT_VERDICT:	verdict statement
 * @STMT_METER:		meter statement
 * @STMT_COUNTER:	counters
 * @STMT_PAYLOAD:	payload statement
 * @STMT_META:		meta statement
 * @STMT_LIMIT:		limit statement
 * @STMT_LOG:		log statement
 * @STMT_REJECT:	REJECT statement
 * @STMT_NAT:		NAT statement
 * @STMT_QUEUE:		QUEUE statement
 * @STMT_CT:		conntrack statement
 * @STMT_SET:		set statement
 * @STMT_DUP:		dup statement
 * @STMT_FWD:		forward statement
 * @STMT_XT:		XT statement
 * @STMT_QUOTA:		quota statement
 * @STMT_NOTRACK:	notrack statement
 * @STMT_OBJREF:	stateful object reference statement
 * @STMT_EXTHDR:	extension header statement
 * @STMT_FLOW_OFFLOAD:	flow offload statement
 * @STMT_CONNLIMIT:	connection limit statement
 * @STMT_MAP:		map statement
 * @STMT_SYNPROXY:	synproxy statement
 * @STMT_CHAIN:		chain statement
 * @STMT_OPTSTRIP:	optstrip statement
 * @STMT_LAST:		last statement
 */
enum stmt_types {
	STMT_INVALID,
	STMT_EXPRESSION,
	STMT_VERDICT,
	STMT_METER,
	STMT_COUNTER,
	STMT_PAYLOAD,
	STMT_META,
	STMT_LIMIT,
	STMT_LOG,
	STMT_REJECT,
	STMT_NAT,
	STMT_TPROXY,
	STMT_QUEUE,
	STMT_CT,
	STMT_SET,
	STMT_DUP,
	STMT_FWD,
	STMT_XT,
	STMT_QUOTA,
	STMT_NOTRACK,
	STMT_OBJREF,
	STMT_EXTHDR,
	STMT_FLOW_OFFLOAD,
	STMT_CONNLIMIT,
	STMT_MAP,
	STMT_SYNPROXY,
	STMT_CHAIN,
	STMT_OPTSTRIP,
	STMT_LAST,
};

/**
 * struct stmt_ops
 *
 * @type:	statement type
 * @name:	name
 * @destroy:	destructor
 * @print:	function to print statement
 */
struct stmt;
struct stmt_ops {
	enum stmt_types		type;
	const char		*name;
	void			(*destroy)(struct stmt *stmt);
	void			(*print)(const struct stmt *stmt,
					 struct output_ctx *octx);
	json_t			*(*json)(const struct stmt *stmt,
					 struct output_ctx *octx);
};

enum stmt_flags {
	STMT_F_TERMINAL		= 0x1,
	STMT_F_STATEFUL		= 0x2,
};

/**
 * struct stmt
 *
 * @list:	rule list node
 * @location:	location where the statement was defined
 * @flags:	statement flags
 * @type:	statement type
 * @union:	type specific data
 */
struct stmt {
	struct list_head		list;
	struct location			location;
	enum stmt_flags			flags;
	enum stmt_types			type:8;

	union {
		struct expr		*expr;
		struct exthdr_stmt	exthdr;
		struct meter_stmt	meter;
		struct connlimit_stmt	connlimit;
		struct counter_stmt	counter;
		struct payload_stmt	payload;
		struct meta_stmt	meta;
		struct last_stmt	last;
		struct log_stmt		log;
		struct limit_stmt	limit;
		struct reject_stmt	reject;
		struct nat_stmt		nat;
		struct tproxy_stmt	tproxy;
		struct optstrip_stmt	optstrip;
		struct queue_stmt	queue;
		struct quota_stmt	quota;
		struct ct_stmt		ct;
		struct set_stmt		set;
		struct dup_stmt		dup;
		struct fwd_stmt		fwd;
		struct xt_stmt		xt;
		struct objref_stmt	objref;
		struct flow_stmt	flow;
		struct map_stmt		map;
		struct synproxy_stmt	synproxy;
		struct chain_stmt	chain;
	};
};

extern struct stmt *stmt_alloc(const struct location *loc,
			       const struct stmt_ops *ops);
int stmt_evaluate(struct eval_ctx *ctx, struct stmt *stmt);
int stmt_dependency_evaluate(struct eval_ctx *ctx, struct stmt *stmt);
extern void stmt_free(struct stmt *stmt);
extern void stmt_list_free(struct list_head *list);
extern void stmt_print(const struct stmt *stmt, struct output_ctx *octx);
const char *stmt_name(const struct stmt *stmt);
const struct stmt_ops *stmt_ops(const struct stmt *stmt);

const char *get_rate(uint64_t byte_rate, uint64_t *rate);
const char *get_unit(uint64_t u);

#endif /* NFTABLES_STATEMENT_H */
