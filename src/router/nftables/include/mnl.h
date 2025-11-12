#ifndef _NFTABLES_MNL_H_
#define _NFTABLES_MNL_H_

#include <list.h>
#include <netlink.h>
#include <rule.h>
#include <libmnl/libmnl.h>

struct mnl_socket *nft_mnl_socket_open(void);

uint32_t mnl_seqnum_inc(uint32_t *seqnum);
uint32_t mnl_genid_get(struct netlink_ctx *ctx);

struct mnl_err {
	struct list_head	head;
	int			err;
	uint32_t		seqnum;
	uint32_t		offset;
};

void mnl_err_list_free(struct mnl_err *err);

struct nftnl_batch *mnl_batch_init(void);
bool mnl_batch_ready(struct nftnl_batch *batch);
void mnl_batch_reset(struct nftnl_batch *batch);
uint32_t mnl_batch_begin(struct nftnl_batch *batch, uint32_t seqnum);
void mnl_batch_end(struct nftnl_batch *batch, uint32_t seqnum);
int mnl_batch_talk(struct netlink_ctx *ctx, struct list_head *err_list,
		   uint32_t num_cmds);

int mnl_nft_rule_add(struct netlink_ctx *ctx, struct cmd *cmd,
		     unsigned int flags);
int mnl_nft_rule_del(struct netlink_ctx *ctx, struct cmd *cmd);
int mnl_nft_rule_replace(struct netlink_ctx *ctx, struct cmd *cmd);

struct nftnl_rule_list *mnl_nft_rule_dump(struct netlink_ctx *ctx, int family,
					  const char *table, const char *chain,
					  uint64_t rule_handle,
					  bool dump, bool reset);

int mnl_nft_chain_add(struct netlink_ctx *ctx, struct cmd *cmd,
		      unsigned int flags);
int mnl_nft_chain_del(struct netlink_ctx *ctx, struct cmd *cmd);
int mnl_nft_chain_rename(struct netlink_ctx *ctx, const struct cmd *cmd,
			 const struct chain *chain);

struct nftnl_chain_list *mnl_nft_chain_dump(struct netlink_ctx *ctx,
					    int family, const char *table,
					    const char *chain);

int mnl_nft_table_add(struct netlink_ctx *ctx, struct cmd *cmd,
		      unsigned int flags);
int mnl_nft_table_del(struct netlink_ctx *ctx, struct cmd *cmd);

struct nftnl_table_list *mnl_nft_table_dump(struct netlink_ctx *ctx,
					    int family, const char *table);

int mnl_nft_set_add(struct netlink_ctx *ctx, struct cmd *cmd,
		    unsigned int flags);
int mnl_nft_set_del(struct netlink_ctx *ctx, struct cmd *cmd);

struct nftnl_set_list *mnl_nft_set_dump(struct netlink_ctx *ctx, int family,
					const char *table, const char *set);

int mnl_nft_setelem_add(struct netlink_ctx *ctx, struct cmd *cmd,
			const struct set *set, const struct expr *expr,
			unsigned int flags);
int mnl_nft_setelem_del(struct netlink_ctx *ctx, struct cmd *cmd,
			const struct handle *h, const struct set *set,
			const struct expr *init);
int mnl_nft_setelem_flush(struct netlink_ctx *ctx, const struct cmd *cmd);
int mnl_nft_setelem_get(struct netlink_ctx *ctx, struct nftnl_set *nls,
			bool reset);
struct nftnl_set *mnl_nft_setelem_get_one(struct netlink_ctx *ctx,
					  struct nftnl_set *nls,
					  bool reset);

struct nftnl_obj_list *mnl_nft_obj_dump(struct netlink_ctx *ctx, int family,
					const char *table,
					const char *name, uint32_t type,
					bool dump, bool reset);
int mnl_nft_obj_add(struct netlink_ctx *ctx, struct cmd *cmd,
		    unsigned int flags);
int mnl_nft_obj_del(struct netlink_ctx *ctx, struct cmd *cmd, int type);

struct nftnl_flowtable_list *
mnl_nft_flowtable_dump(struct netlink_ctx *ctx, int family,
		       const char *table, const char *ft);

int mnl_nft_flowtable_add(struct netlink_ctx *ctx, struct cmd *cmd,
			  unsigned int flags);
int mnl_nft_flowtable_del(struct netlink_ctx *ctx, struct cmd *cmd);

int mnl_nft_dump_nf_hooks(struct netlink_ctx *ctx, int family,
			  const char *devname);

int mnl_nft_event_listener(struct mnl_socket *nf_sock, unsigned int debug_mask,
			   struct output_ctx *octx,
			   int (*cb)(const struct nlmsghdr *nlh, void *data),
			   void *cb_data);

int nft_mnl_talk(struct netlink_ctx *ctx, const void *data, unsigned int len,
		 int (*cb)(const struct nlmsghdr *nlh, void *data),
		 void *cb_data);

#endif /* _NFTABLES_MNL_H_ */
