#ifndef _NFT_CMD_H_
#define _NFT_CMD_H_

void cmd_add_loc(struct cmd *cmd, const struct nlmsghdr *nlh, const struct location *loc);
struct mnl_err;
void nft_cmd_error(struct netlink_ctx *ctx, struct cmd *cmd,
		   struct mnl_err *err);

bool nft_cmd_collapse_elems(enum cmd_ops op, struct list_head *cmds,
			    struct handle *handle, struct expr *init);

void nft_cmd_expand(struct cmd *cmd);

#endif
