#ifndef SRC_MOD_COMMON_KERNEL_HOOK_H_
#define SRC_MOD_COMMON_KERNEL_HOOK_H_

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include "common/config.h"

#ifndef XTABLES_DISABLED
#include <linux/netfilter/x_tables.h>
#endif

unsigned int hook_ipv6(void *priv, struct sk_buff *skb,
		const struct nf_hook_state *nhs);
unsigned int hook_ipv4(void *priv, struct sk_buff *skb,
		const struct nf_hook_state *nhs);

#ifndef XTABLES_DISABLED

int target_checkentry(const struct xt_tgchk_param *param);
unsigned int target_ipv6(struct sk_buff *skb,
		const struct xt_action_param *param);
unsigned int target_ipv4(struct sk_buff *skb,
		const struct xt_action_param *param);

#endif /* !XTABLES_DISABLED */

#endif /* SRC_MOD_COMMON_KERNEL_HOOK_H_ */
