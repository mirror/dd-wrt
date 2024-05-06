#ifndef SRC_MOD_COMMON_NL_BIB_H_
#define SRC_MOD_COMMON_NL_BIB_H_

#include <net/genetlink.h>

int handle_bib_foreach(struct sk_buff *skb, struct genl_info *info);
int handle_bib_add(struct sk_buff *skb, struct genl_info *info);
int handle_bib_rm(struct sk_buff *skb, struct genl_info *info);

#endif /* SRC_MOD_COMMON_NL */
