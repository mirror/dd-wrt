#ifndef NFTABLES_RULE_H
#define NFTABLES_RULE_H

#include <nftables.h>
#include <list.h>
#include <netinet/in.h>
#include <libnftnl/object.h>	/* For NFTNL_CTTIMEOUT_ARRAY_MAX. */
#include <linux/netfilter/nf_tables.h>
#include <cache.h>

/**
 * struct handle_spec - handle ID
 *
 * @location:	location this handle was defined at
 * @id:		handle ID value
 */
struct handle_spec {
	struct location		location;
	uint64_t		id;
};

/**
 * struct position_spec - position ID
 *
 * @location:	location this position was defined at
 * @id:		position ID value
 */
struct position_spec {
	struct location		location;
	uint64_t		id;
};

struct table_spec {
	struct location		location;
	const char		*name;
};

struct chain_spec {
	struct location		location;
	const char		*name;
};

struct set_spec {
	struct location		location;
	const char		*name;
};

struct flowtable_spec {
	struct location		location;
	const char		*name;
};

struct obj_spec {
	struct location		location;
	const char		*name;
};

/**
 * struct handle - handle for tables, chains, rules and sets
 *
 * @family:	protocol family
 * @table:	table name
 * @chain:	chain name (chains and rules only)
 * @set:	set name (sets only)
 * @obj:	stateful object name (stateful object only)
 * @flowtable:	flow table name (flow table only)
 * @handle:	rule handle (rules only)
 * @position:	rule position (rules only)
 * @set_id:	set ID (sets only)
 */
struct handle {
	uint32_t		family;
	struct table_spec	table;
	struct chain_spec	chain;
	struct set_spec		set;
	struct obj_spec		obj;
	struct flowtable_spec	flowtable;
	struct handle_spec	handle;
	struct position_spec	position;
	struct position_spec	index;
	uint32_t		set_id;
	uint32_t		chain_id;
	uint32_t		rule_id;
	uint32_t		position_id;
};

extern void handle_merge(struct handle *dst, const struct handle *src);
extern void handle_free(struct handle *h);

/**
 * struct scope
 *
 * @parent:	pointer to parent scope
 * @symbols:	symbols bound in the scope
 */
struct scope {
	const struct scope	*parent;
	struct list_head	symbols;
};

extern struct scope *scope_alloc(void);
extern struct scope *scope_init(struct scope *scope, const struct scope *parent);
extern void scope_release(const struct scope *scope);
extern void scope_free(struct scope *scope);

/**
 * struct symbol
 *
 * @list:	scope symbol list node
 * @identifier:	identifier
 * @expr:	initializer
 * @refcnt:	reference counter
 */
struct symbol {
	struct list_head	list;
	const char		*identifier;
	struct expr		*expr;
	unsigned int		refcnt;
};

extern void symbol_bind(struct scope *scope, const char *identifier,
			struct expr *expr);
extern int symbol_unbind(const struct scope *scope, const char *identifier);
extern struct symbol *symbol_lookup(const struct scope *scope,
				    const char *identifier);
struct symbol *symbol_lookup_fuzzy(const struct scope *scope,
				   const char *identifier);
struct symbol *symbol_get(const struct scope *scope, const char *identifier);

enum table_flags {
	TABLE_F_DORMANT		= (1 << 0),
	TABLE_F_OWNER		= (1 << 1),
	TABLE_F_PERSIST		= (1 << 2),
};
#define TABLE_FLAGS_MAX		3

const char *table_flag_name(uint32_t flag);
unsigned int parse_table_flag(const char *name);

/**
 * struct table - nftables table
 *
 * @list:	list node
 * @handle:	table handle
 * @location:	location the table was defined at
 * @chains:	chains contained in the table
 * @sets:	sets contained in the table
 * @objs:	stateful objects contained in the table
 * @flowtables:	flow tables contained in the table
 * @flags:	table flags
 * @refcnt:	table reference counter
 */
struct table {
	struct list_head	list;
	struct cache_item	cache;
	struct handle		handle;
	struct location		location;
	struct scope		scope;
	struct cache		chain_cache;
	struct cache		set_cache;
	struct cache		obj_cache;
	struct cache		ft_cache;
	struct list_head	chains;
	struct list_head	sets;
	struct list_head	objs;
	struct list_head	flowtables;
	struct list_head	chain_bindings;
	enum table_flags 	flags;
	unsigned int		refcnt;
	uint32_t		owner;
	const char		*comment;
	bool			has_xt_stmts;
	bool			is_from_future;
};

extern struct table *table_alloc(void);
extern struct table *table_get(struct table *table);
extern void table_free(struct table *table);
extern struct table *table_lookup_fuzzy(const struct handle *h,
					const struct nft_cache *cache);

/**
 * enum chain_flags - chain flags
 *
 * @CHAIN_F_BASECHAIN:	chain is a base chain
 */
enum chain_flags {
	CHAIN_F_BASECHAIN	= 0x1,
	CHAIN_F_HW_OFFLOAD	= 0x2,
	CHAIN_F_BINDING		= 0x4,
};

/**
 * enum flowtable_flags - flowtable flags
 *
 */
enum flowtable_flags {
	FLOWTABLE_F_HW_OFFLOAD	= 0x1, /* NF_FLOWTABLE_HW_OFFLOAD in linux nf_flow_table.h */
};

/**
 * struct prio_spec - extendend priority specification for mixed
 *                    textual/numerical parsing.
 *
 * @expr:  expr of the standard priority value
 */
struct prio_spec {
	struct location loc;
	struct expr	*expr;
};

struct hook_spec {
	struct location	loc;
	const char	*name;
	unsigned int	num;
};

struct chain_type_spec {
	struct location	loc;
	const char	*str;
};

/**
 * struct chain - nftables chain
 *
 * @list:	list node in table list
 * @handle:	chain handle
 * @location:	location the chain was defined at
 * @refcnt:	reference counter
 * @flags:	chain flags
 * @hookstr:	unified and human readable hook name (base chains)
 * @hooknum:	hook number (base chains)
 * @priority:	hook priority (base chains)
 * @policy:	default chain policy (base chains)
 * @type:	chain type
 * @dev:	device (if any)
 * @rules:	rules contained in the chain
 */
struct chain {
	struct list_head	list;
	struct cache_item	cache;
	struct handle		handle;
	struct location		location;
	unsigned int		refcnt;
	uint32_t		flags;
	const char		*comment;
	struct {
		struct location		loc;
		struct prio_spec	priority;
		struct hook_spec	hook;
		struct expr		*policy;
		struct chain_type_spec	type;
		const char		**dev_array;
		struct expr		*dev_expr;
		int			dev_array_len;
	};
	struct scope		scope;
	struct list_head	rules;
};

#define STD_PRIO_BUFSIZE 100
extern int std_prio_lookup(const char *std_prio_name, int family, int hook);
extern const char *chain_type_name_lookup(const char *name);
extern const char *chain_hookname_lookup(const char *name);
extern struct chain *chain_alloc(void);
extern struct chain *chain_get(struct chain *chain);
extern void chain_free(struct chain *chain);
extern struct chain *chain_lookup_fuzzy(const struct handle *h,
					const struct nft_cache *cache,
					const struct table **table);
extern struct chain *chain_binding_lookup(const struct table *table,
					  const char *chain_name);

extern const char *family2str(unsigned int family);
#define __NF_ARP_INGRESS	255
extern const char *hooknum2str(unsigned int family, unsigned int hooknum);
extern const char *chain_policy2str(uint32_t policy);
extern void chain_print_plain(const struct chain *chain,
			      struct output_ctx *octx);
extern void chain_rules_print(const struct chain *chain,
			      struct output_ctx *octx, const char *indent);

/**
 * struct rule - nftables rule
 *
 * @list:	list node in chain list
 * @handle:	rule handle
 * @location:	location the rule was defined at
 * @stmt:	list of statements
 * @num_stmts:	number of statements in stmts list
 * @comment:	comment
 * @refcnt:	rule reference counter
 */
struct rule {
	struct list_head	list;
	struct handle		handle;
	struct location		location;
	struct list_head	stmts;
	unsigned int		num_stmts;
	const char		*comment;
	unsigned int		refcnt;
};

extern struct rule *rule_alloc(const struct location *loc,
			       const struct handle *h);
extern struct rule *rule_get(struct rule *rule);
extern void rule_free(struct rule *rule);
extern void rule_print(const struct rule *rule, struct output_ctx *octx);
extern struct rule *rule_lookup(const struct chain *chain, uint64_t handle);
extern struct rule *rule_lookup_by_index(const struct chain *chain,
					 uint64_t index);
void rule_stmt_append(struct rule *rule, struct stmt *stmt);
void rule_stmt_insert_at(struct rule *rule, struct stmt *nstmt,
			 struct stmt *stmt);

/**
 * struct set - nftables set
 *
 * @list:	table set list node
 * @handle:	set handle
 * @location:	location the set was defined/declared at
 * @refcnt:	reference count
 * @flags:	bitmask of set flags
 * @gc_int:	garbage collection interval
 * @count:	count of kernel-allocated elements
 * @timeout:	default timeout value
 * @key:	key expression (data type, length))
 * @data:	mapping data expression
 * @objtype:	mapping object type
 * @existing_set: reference to existing set in the kernel
 * @init:	initializer
 * @rg_cache:	cached range element (left)
 * @policy:	set mechanism policy
 * @automerge:	merge adjacents and overlapping elements, if possible
 * @comment:	comment
 * @errors:	expr evaluation errors seen
 * @elem_has_comment: element with comment seen (for printing)
 * @desc.size:		count of set elements
 * @desc.field_len:	length of single concatenated fields, bytes
 * @desc.field_count:	count of concatenated fields
 */
struct set {
	struct list_head	list;
	struct cache_item	cache;
	struct handle		handle;
	struct location		location;
	unsigned int		refcnt;
	uint32_t		flags;
	uint32_t		gc_int;
	uint32_t		count;
	uint64_t		timeout;
	struct expr		*key;
	struct expr		*data;
	uint32_t		objtype;
	struct set		*existing_set;
	struct expr		*init;
	struct expr		*rg_cache;
	uint32_t		policy;
	struct list_head	stmt_list;
	bool			root;
	bool			automerge;
	bool			key_typeof_valid;
	bool			errors;
	bool			elem_has_comment;
	const char		*comment;
	struct {
		uint32_t	size;
		uint8_t		field_len[NFT_REG32_COUNT];
		uint8_t		field_count;
	} desc;
};

extern struct set *set_alloc(const struct location *loc);
extern struct set *set_get(struct set *set);
extern void set_free(struct set *set);
extern struct set *set_clone(const struct set *set);
extern struct set *set_lookup_global(uint32_t family, const char *table,
				     const char *name, struct nft_cache *cache);
extern struct set *set_lookup_fuzzy(const char *set_name,
				    const struct nft_cache *cache,
				    const struct table **table);
extern const char *set_policy2str(uint32_t policy);
extern void set_print(const struct set *set, struct output_ctx *octx);
extern void set_print_plain(const struct set *s, struct output_ctx *octx);

static inline bool set_is_datamap(uint32_t set_flags)
{
	return set_flags & NFT_SET_MAP;
}

static inline bool set_is_objmap(uint32_t set_flags)
{
	return set_flags & NFT_SET_OBJECT;
}

static inline bool set_is_map(uint32_t set_flags)
{
	return set_is_datamap(set_flags) || set_is_objmap(set_flags);
}

static inline bool set_is_anonymous(uint32_t set_flags)
{
	return set_flags & NFT_SET_ANONYMOUS;
}

static inline bool set_is_literal(uint32_t set_flags)
{
	return !(set_is_anonymous(set_flags) || set_is_map(set_flags));
}

static inline bool map_is_literal(uint32_t set_flags)
{
	return !(set_is_anonymous(set_flags) || !set_is_map(set_flags));
}

static inline bool set_is_meter(uint32_t set_flags)
{
	return set_is_anonymous(set_flags) && (set_flags & NFT_SET_EVAL);
}

static inline bool set_is_meter_compat(uint32_t set_flags)
{
	return set_flags & NFT_SET_EVAL;
}

static inline bool set_is_interval(uint32_t set_flags)
{
	return set_flags & NFT_SET_INTERVAL;
}

static inline bool set_is_non_concat_range(const struct set *s)
{
	return (s->flags & NFT_SET_INTERVAL) && s->desc.field_count <= 1;
}

#include <statement.h>

struct counter {
	uint64_t	packets;
	uint64_t	bytes;
};

struct quota {
	uint64_t	bytes;
	uint64_t	used;
	uint32_t	flags;
};

struct ct_helper {
	char name[16];
	uint16_t l3proto;
	uint8_t l4proto;
};

struct timeout_state {
	struct list_head head;
	struct location location;
	uint8_t timeout_index;
	const char *timeout_str;
	unsigned int timeout_value;
};

struct ct_timeout {
	uint16_t l3proto;
	uint8_t l4proto;
	uint32_t timeout[NFTNL_CTTIMEOUT_ARRAY_MAX];
	struct list_head timeout_list;
};

struct ct_expect {
	uint16_t l3proto;
	uint8_t l4proto;
	uint16_t dport;
	uint32_t timeout;
	uint8_t size;
};

struct limit {
	uint64_t	rate;
	uint64_t	unit;
	uint32_t	burst;
	uint32_t	type;
	uint32_t	flags;
};

struct synproxy {
	uint16_t	mss;
	uint8_t		wscale;
	uint32_t	flags;
};

struct secmark {
	char		ctx[NFT_SECMARK_CTX_MAXLEN];
};

enum tunnel_type {
	TUNNEL_UNSPEC = 0,
	TUNNEL_ERSPAN,
	TUNNEL_VXLAN,
	TUNNEL_GENEVE,
};

struct tunnel_geneve {
	struct list_head	list;
	uint16_t		geneve_class;
	uint8_t			type;
	uint8_t			data[NFTNL_TUNNEL_GENEVE_DATA_MAXLEN];
	uint32_t		data_len;
};

struct tunnel {
	uint32_t	id;
	struct expr	*src;
	struct expr	*dst;
	uint16_t	sport;
	uint16_t	dport;
	uint8_t		tos;
	uint8_t		ttl;
	enum tunnel_type type;
	union {
		struct {
			uint32_t	version;
			struct {
				uint32_t	index;
			} v1;
			struct {
				uint8_t		direction;
				uint8_t		hwid;
			} v2;
		} erspan;
		struct {
			uint32_t	gbp;
		} vxlan;
		struct list_head	geneve_opts;
	};
};

int tunnel_geneve_data_str2array(const char *hexstr,
				 uint8_t *out_data,
				 uint32_t *out_len);

/**
 * struct obj - nftables stateful object statement
 *
 * @list:	table set list node
 * @location:	location the stateful object was defined/declared at
 * @handle:	counter handle
 * @type:	type of stateful object
 * @refcnt:	object reference counter
 */
struct obj {
	struct list_head		list;
	struct cache_item		cache;
	struct location			location;
	struct handle			handle;
	uint32_t			type;
	unsigned int			refcnt;
	const char			*comment;
	union {
		struct counter		counter;
		struct quota		quota;
		struct ct_helper	ct_helper;
		struct limit		limit;
		struct ct_timeout	ct_timeout;
		struct secmark		secmark;
		struct ct_expect	ct_expect;
		struct synproxy		synproxy;
		struct tunnel		tunnel;
	};
};

struct obj *obj_alloc(const struct location *loc);
extern struct obj *obj_get(struct obj *obj);
void obj_free(struct obj *obj);
struct obj *obj_lookup_fuzzy(const char *obj_name,
			     const struct nft_cache *cache,
			     const struct table **t);
void obj_print(const struct obj *n, struct output_ctx *octx);
void obj_print_plain(const struct obj *obj, struct output_ctx *octx);
const char *obj_type_name(uint32_t type);
enum cmd_obj obj_type_to_cmd(uint32_t type);

struct flowtable {
	struct list_head	list;
	struct cache_item	cache;
	struct handle		handle;
	struct scope		scope;
	struct location		location;
	struct hook_spec	hook;
	struct prio_spec	priority;
	const char		**dev_array;
	struct expr		*dev_expr;
	int			dev_array_len;
	uint32_t		flags;
	unsigned int		refcnt;
};

extern struct flowtable *flowtable_alloc(const struct location *loc);
extern struct flowtable *flowtable_get(struct flowtable *flowtable);
extern void flowtable_free(struct flowtable *flowtable);
extern struct flowtable *flowtable_lookup_fuzzy(const char *ft_name,
						const struct nft_cache *cache,
						const struct table **table);

void flowtable_print(const struct flowtable *n, struct output_ctx *octx);
void flowtable_print_plain(const struct flowtable *ft, struct output_ctx *octx);

/**
 * enum cmd_ops - command operations
 *
 * @CMD_INVALID:	invalid
 * @CMD_ADD:		add object (non-exclusive)
 * @CMD_REPLACE,	replace object
 * @CMD_CREATE:		create object (exclusive)
 * @CMD_INSERT:		insert object
 * @CMD_DELETE:		delete object
 * @CMD_GET:		get object
 * @CMD_LIST:		list container
 * @CMD_RESET:		reset container
 * @CMD_FLUSH:		flush container
 * @CMD_RENAME:		rename object
 * @CMD_IMPORT:		import a ruleset in a given format
 * @CMD_EXPORT:		export the ruleset in a given format
 * @CMD_MONITOR:	event listener
 * @CMD_DESCRIBE:	describe an expression
 * @CMD_DESTROY:	destroy object
 */
enum cmd_ops {
	CMD_INVALID,
	CMD_ADD,
	CMD_REPLACE,
	CMD_CREATE,
	CMD_INSERT,
	CMD_DELETE,
	CMD_GET,
	CMD_LIST,
	CMD_RESET,
	CMD_FLUSH,
	CMD_RENAME,
	CMD_IMPORT,
	CMD_EXPORT,
	CMD_MONITOR,
	CMD_DESCRIBE,
	CMD_DESTROY,
};

/**
 * enum cmd_obj - command objects
 *
 * @CMD_OBJ_INVALID:	invalid
 * @CMD_OBJ_ELEMENTS:	set element(s)
 * @CMD_OBJ_SET:	set
 * @CMD_OBJ_SETS:	multiple sets
 * @CMD_OBJ_SETELEMS:	set elements
 * @CMD_OBJ_RULE:	rule
 * @CMD_OBJ_CHAIN:	chain
 * @CMD_OBJ_CHAINS:	multiple chains
 * @CMD_OBJ_TABLE:	table
 * @CMD_OBJ_FLOWTABLE:	flowtable
 * @CMD_OBJ_FLOWTABLES:	flowtables
 * @CMD_OBJ_RULESET:	ruleset
 * @CMD_OBJ_EXPR:	expression
 * @CMD_OBJ_MONITOR:	monitor
 * @CMD_OBJ_MARKUP:    import/export
 * @CMD_OBJ_METER:	meter
 * @CMD_OBJ_METERS:	meters
 * @CMD_OBJ_COUNTER:	counter
 * @CMD_OBJ_COUNTERS:	multiple counters
 * @CMD_OBJ_QUOTA:	quota
 * @CMD_OBJ_QUOTAS:	multiple quotas
 * @CMD_OBJ_LIMIT:	limit
 * @CMD_OBJ_LIMITS:	multiple limits
 * @CMD_OBJ_SECMARK:	secmark
 * @CMD_OBJ_SECMARKS:	multiple secmarks
 * @CMD_OBJ_SYNPROXY:	synproxy
 * @CMD_OBJ_SYNPROXYS:	multiple synproxys
 * @CMD_OBJ_TUNNEL:	tunnel
 * @CMD_OBJ_TUNNELS:	multiple tunnels
 * @CMD_OBJ_HOOKS:	hooks, used only for dumping
 */
enum cmd_obj {
	CMD_OBJ_INVALID,
	CMD_OBJ_ELEMENTS,
	CMD_OBJ_SET,
	CMD_OBJ_SETELEMS,
	CMD_OBJ_SETS,
	CMD_OBJ_RULE,
	CMD_OBJ_RULES,
	CMD_OBJ_CHAIN,
	CMD_OBJ_CHAINS,
	CMD_OBJ_TABLE,
	CMD_OBJ_RULESET,
	CMD_OBJ_EXPR,
	CMD_OBJ_MONITOR,
	CMD_OBJ_MARKUP,
	CMD_OBJ_METER,
	CMD_OBJ_METERS,
	CMD_OBJ_MAP,
	CMD_OBJ_MAPS,
	CMD_OBJ_COUNTER,
	CMD_OBJ_COUNTERS,
	CMD_OBJ_QUOTA,
	CMD_OBJ_QUOTAS,
	CMD_OBJ_CT_HELPER,
	CMD_OBJ_CT_HELPERS,
	CMD_OBJ_LIMIT,
	CMD_OBJ_LIMITS,
	CMD_OBJ_FLOWTABLE,
	CMD_OBJ_FLOWTABLES,
	CMD_OBJ_CT_TIMEOUT,
	CMD_OBJ_CT_TIMEOUTS,
	CMD_OBJ_SECMARK,
	CMD_OBJ_SECMARKS,
	CMD_OBJ_CT_EXPECT,
	CMD_OBJ_CT_EXPECTATIONS,
	CMD_OBJ_SYNPROXY,
	CMD_OBJ_SYNPROXYS,
	CMD_OBJ_TUNNEL,
	CMD_OBJ_TUNNELS,
	CMD_OBJ_HOOKS,
};

struct markup {
	uint32_t	format;
};

struct markup *markup_alloc(uint32_t format);
void markup_free(struct markup *m);

enum {
	CMD_MONITOR_OBJ_ANY,
	CMD_MONITOR_OBJ_TABLES,
	CMD_MONITOR_OBJ_CHAINS,
	CMD_MONITOR_OBJ_RULES,
	CMD_MONITOR_OBJ_SETS,
	CMD_MONITOR_OBJ_ELEMS,
	CMD_MONITOR_OBJ_RULESET,
	CMD_MONITOR_OBJ_TRACE,
	CMD_MONITOR_OBJ_MAX
};

struct monitor {
	struct location	location;
	uint32_t	format;
	uint32_t	flags;
	uint32_t	type;
	const char	*event;
};

struct monitor *monitor_alloc(uint32_t format, uint32_t type, const char *event);
void monitor_free(struct monitor *m);

#define NFT_NLATTR_LOC_MAX 32

struct nlerr_loc {
	uint32_t		seqnum;
	uint32_t		offset;
	const struct location	*location;
};

/**
 * struct cmd - command statement
 *
 * @list:	list node
 * @location:	location of the statement
 * @op:		operation
 * @obj:	object type to perform operation on
 * @handle:	handle for operations working without full objects
 * @seqnum:	sequence number to match netlink errors
 * @union:	object
 * @arg:	argument data
 */
struct cmd {
	struct list_head	list;
	struct location		location;
	enum cmd_ops		op;
	enum cmd_obj		obj;
	struct handle		handle;
	uint32_t		seqnum_from;
	uint32_t		seqnum_to;
	union {
		void		*data;
		struct expr	*expr;
		struct set	*set;
		struct {
			struct expr	*expr;	/* same offset as cmd->expr */
			struct set	*set;
		} elem;
		struct rule	*rule;
		struct chain	*chain;
		struct table	*table;
		struct flowtable *flowtable;
		struct monitor	*monitor;
		struct markup	*markup;
		struct obj	*object;
	};
	struct nlerr_loc	*attr;
	uint32_t		attr_array_len;
	uint32_t		num_attrs;
	const void		*arg;
};

extern struct cmd *cmd_alloc(enum cmd_ops op, enum cmd_obj obj,
			     const struct handle *h, const struct location *loc,
			     void *data);
extern struct cmd *cmd_alloc_obj_ct(enum cmd_ops op, int type,
				    const struct handle *h,
				    const struct location *loc, struct obj *obj);
extern void cmd_free(struct cmd *cmd);

#include <payload.h>
#include <expression.h>

struct eval_recursion {
	uint16_t binop;
	uint16_t list;
};

/**
 * struct eval_ctx - evaluation context
 *
 * @nft:	nftables context
 * @msgs:	message queue
 * @cmd:	current command
 * @table:	current table
 * @rule:	current rule
 * @set:	current set
 * @stmt:	current statement
 * @stmt_len:	current statement template length
 * @recursion:  expr evaluation recursion counters
 * @cache:	cache context
 * @debug_mask: debugging bitmask
 * @ectx:	expression context
 * @_pctx:	payload contexts
 * @inner_desc: inner header description
 */
struct eval_ctx {
	struct nft_ctx		*nft;
	struct list_head	*msgs;
	struct cmd		*cmd;
	struct table		*table;
	struct rule		*rule;
	struct set		*set;
	struct stmt		*stmt;
	uint32_t		stmt_len;
	struct eval_recursion	recursion;
	struct expr_ctx		ectx;
	struct proto_ctx	_pctx[2];
	const struct proto_desc	*inner_desc;
};

extern int cmd_evaluate(struct eval_ctx *ctx, struct cmd *cmd);

extern struct error_record *rule_postprocess(struct rule *rule);

struct netlink_ctx;
extern int do_command(struct netlink_ctx *ctx, struct cmd *cmd);

struct timeout_protocol {
	uint32_t array_size;
	const char *const *state_to_name;
	uint32_t *dflt_timeout;
};

extern struct timeout_protocol timeout_protocol[UINT8_MAX + 1];
extern int timeout_str2num(uint16_t l4proto, struct timeout_state *ts);

#endif /* NFTABLES_RULE_H */
