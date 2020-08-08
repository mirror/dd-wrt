#ifndef _NFT_SHARED_H_
#define _NFT_SHARED_H_

#include <stdbool.h>

#include <libnftnl/rule.h>
#include <libnftnl/expr.h>
#include <libnftnl/chain.h>

#include <linux/netfilter_arp/arp_tables.h>

#include "xshared.h"

#ifdef DEBUG
#define NLDEBUG
#define DEBUG_DEL
#endif

/*
 * iptables print output emulation
 */

#define FMT_NUMERIC	0x0001
#define FMT_NOCOUNTS	0x0002
#define FMT_KILOMEGAGIGA 0x0004
#define FMT_OPTIONS	0x0008
#define FMT_NOTABLE	0x0010
#define FMT_NOTARGET	0x0020
#define FMT_VIA		0x0040
#define FMT_NONEWLINE	0x0080
#define FMT_LINENUMBERS 0x0100

#define FMT_PRINT_RULE (FMT_NOCOUNTS | FMT_OPTIONS | FMT_VIA \
			| FMT_NUMERIC | FMT_NOTABLE)
#define FMT(tab,notab) ((format) & FMT_NOTABLE ? (notab) : (tab))

struct xtables_args;
struct nft_handle;
struct xt_xlate;

enum {
	NFT_XT_CTX_PAYLOAD	= (1 << 0),
	NFT_XT_CTX_META		= (1 << 1),
	NFT_XT_CTX_BITWISE	= (1 << 2),
	NFT_XT_CTX_IMMEDIATE	= (1 << 3),
	NFT_XT_CTX_PREV_PAYLOAD	= (1 << 4),
};

struct nft_xt_ctx {
	struct iptables_command_state *cs;
	struct nftnl_expr_iter *iter;
	struct nft_handle *h;
	uint32_t flags;
	const char *table;

	uint32_t reg;
	struct {
		uint32_t base;
		uint32_t offset;
		uint32_t len;
	} payload, prev_payload;
	struct {
		uint32_t key;
	} meta;
	struct {
		uint32_t data[4];
		uint32_t len, reg;
	} immediate;
	struct {
		uint32_t mask[4];
		uint32_t xor[4];
	} bitwise;
};

struct nft_family_ops {
	int (*add)(struct nft_handle *h, struct nftnl_rule *r, void *data);
	bool (*is_same)(const void *data_a,
			const void *data_b);
	void (*print_payload)(struct nftnl_expr *e,
			      struct nftnl_expr_iter *iter);
	void (*parse_meta)(struct nft_xt_ctx *ctx, struct nftnl_expr *e,
			   void *data);
	void (*parse_payload)(struct nft_xt_ctx *ctx, struct nftnl_expr *e,
			      void *data);
	void (*parse_bitwise)(struct nft_xt_ctx *ctx, struct nftnl_expr *e,
			      void *data);
	void (*parse_cmp)(struct nft_xt_ctx *ctx, struct nftnl_expr *e,
			  void *data);
	void (*parse_lookup)(struct nft_xt_ctx *ctx, struct nftnl_expr *e,
			     void *data);
	void (*parse_immediate)(const char *jumpto, bool nft_goto, void *data);

	void (*print_table_header)(const char *tablename);
	void (*print_header)(unsigned int format, const char *chain,
			     const char *pol,
			     const struct xt_counters *counters, bool basechain,
			     uint32_t refs, uint32_t entries);
	void (*print_rule)(struct nft_handle *h, struct nftnl_rule *r,
			   unsigned int num, unsigned int format);
	void (*save_rule)(const void *data, unsigned int format);
	void (*save_chain)(const struct nftnl_chain *c, const char *policy);
	void (*proto_parse)(struct iptables_command_state *cs,
			    struct xtables_args *args);
	void (*post_parse)(int command, struct iptables_command_state *cs,
			   struct xtables_args *args);
	void (*parse_match)(struct xtables_match *m, void *data);
	void (*parse_target)(struct xtables_target *t, void *data);
	void (*rule_to_cs)(struct nft_handle *h, const struct nftnl_rule *r,
			   struct iptables_command_state *cs);
	void (*clear_cs)(struct iptables_command_state *cs);
	int (*xlate)(const void *data, struct xt_xlate *xl);
};

void add_meta(struct nftnl_rule *r, uint32_t key);
void add_payload(struct nftnl_rule *r, int offset, int len, uint32_t base);
void add_bitwise(struct nftnl_rule *r, uint8_t *mask, size_t len);
void add_bitwise_u16(struct nftnl_rule *r, uint16_t mask, uint16_t xor);
void add_cmp_ptr(struct nftnl_rule *r, uint32_t op, void *data, size_t len);
void add_cmp_u8(struct nftnl_rule *r, uint8_t val, uint32_t op);
void add_cmp_u16(struct nftnl_rule *r, uint16_t val, uint32_t op);
void add_cmp_u32(struct nftnl_rule *r, uint32_t val, uint32_t op);
void add_iniface(struct nftnl_rule *r, char *iface, uint32_t op);
void add_outiface(struct nftnl_rule *r, char *iface, uint32_t op);
void add_addr(struct nftnl_rule *r, int offset,
	      void *data, void *mask, size_t len, uint32_t op);
void add_proto(struct nftnl_rule *r, int offset, size_t len,
	       uint8_t proto, uint32_t op);
void add_l4proto(struct nftnl_rule *r, uint8_t proto, uint32_t op);
void add_compat(struct nftnl_rule *r, uint32_t proto, bool inv);

bool is_same_interfaces(const char *a_iniface, const char *a_outiface,
			unsigned const char *a_iniface_mask,
			unsigned const char *a_outiface_mask,
			const char *b_iniface, const char *b_outiface,
			unsigned const char *b_iniface_mask,
			unsigned const char *b_outiface_mask);

int parse_meta(struct nftnl_expr *e, uint8_t key, char *iniface,
		unsigned char *iniface_mask, char *outiface,
		unsigned char *outiface_mask, uint8_t *invflags);
void print_proto(uint16_t proto, int invert);
void get_cmp_data(struct nftnl_expr *e, void *data, size_t dlen, bool *inv);
void nft_rule_to_iptables_command_state(struct nft_handle *h,
					const struct nftnl_rule *r,
					struct iptables_command_state *cs);
void nft_clear_iptables_command_state(struct iptables_command_state *cs);
void print_header(unsigned int format, const char *chain, const char *pol,
		  const struct xt_counters *counters, bool basechain,
		  uint32_t refs, uint32_t entries);
void print_rule_details(const struct iptables_command_state *cs,
			const char *targname, uint8_t flags,
			uint8_t invflags, uint8_t proto,
			unsigned int num, unsigned int format);
void print_matches_and_target(struct iptables_command_state *cs,
			      unsigned int format);
void save_rule_details(const struct iptables_command_state *cs,
		       uint8_t invflags, uint16_t proto,
		       const char *iniface,
		       unsigned const char *iniface_mask,
		       const char *outiface,
		       unsigned const char *outiface_mask);
void nft_ipv46_save_chain(const struct nftnl_chain *c, const char *policy);
void save_matches_and_target(const struct iptables_command_state *cs,
			     bool goto_flag, const void *fw,
			     unsigned int format);

struct nft_family_ops *nft_family_ops_lookup(int family);

void nft_ipv46_parse_target(struct xtables_target *t, void *data);

bool compare_matches(struct xtables_rule_match *mt1, struct xtables_rule_match *mt2);
bool compare_targets(struct xtables_target *tg1, struct xtables_target *tg2);

struct addr_mask {
	union {
		struct in_addr	*v4;
		struct in6_addr *v6;
	} addr;

	unsigned int naddrs;

	union {
		struct in_addr	*v4;
		struct in6_addr *v6;
	} mask;
};

struct xtables_args {
	int		family;
	uint16_t	proto;
	uint8_t		flags;
	uint8_t		invflags;
	char		iniface[IFNAMSIZ], outiface[IFNAMSIZ];
	unsigned char	iniface_mask[IFNAMSIZ], outiface_mask[IFNAMSIZ];
	bool		goto_set;
	const char	*shostnetworkmask, *dhostnetworkmask;
	const char	*pcnt, *bcnt;
	struct addr_mask s, d;
	unsigned long long pcnt_cnt, bcnt_cnt;
};

struct nft_xt_cmd_parse {
	unsigned int			command;
	unsigned int			rulenum;
	char				*table;
	const char			*chain;
	const char			*newname;
	const char			*policy;
	bool				restore;
	int				verbose;
	bool				xlate;
};

void do_parse(struct nft_handle *h, int argc, char *argv[],
	      struct nft_xt_cmd_parse *p, struct iptables_command_state *cs,
	      struct xtables_args *args);

struct nftnl_chain_list;

struct nft_xt_restore_cb {
	void (*table_new)(struct nft_handle *h, const char *table);
	int (*chain_set)(struct nft_handle *h, const char *table,
			 const char *chain, const char *policy,
			 const struct xt_counters *counters);
	int (*chain_restore)(struct nft_handle *h, const char *chain,
			     const char *table);

	int (*table_flush)(struct nft_handle *h, const char *table);

	int (*do_command)(struct nft_handle *h, int argc, char *argv[],
			  char **table, bool restore);

	int (*commit)(struct nft_handle *h);
	int (*abort)(struct nft_handle *h);
};

struct nft_xt_restore_parse {
	FILE				*in;
	int				testing;
	const char			*tablename;
	bool				commit;
	const struct nft_xt_restore_cb	*cb;
};

void xtables_restore_parse(struct nft_handle *h,
			   const struct nft_xt_restore_parse *p);

void nft_check_xt_legacy(int family, bool is_ipt_save);
#endif
