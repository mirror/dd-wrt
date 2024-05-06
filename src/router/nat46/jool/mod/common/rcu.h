#ifndef SRC_MOD_COMMON_RCU_H_
#define SRC_MOD_COMMON_RCU_H_

/**
 * @file
 * RCU primitives which are lacking from the kernel's headers.
 * http://stackoverflow.com/questions/32360052
 */

#include "mod/common/linux_version.h"

#define list_for_each_rcu_bh(pos, head) \
	for (pos = rcu_dereference_bh(list_next_rcu(head));	\
	     pos != head;					\
	     pos = rcu_dereference_bh(list_next_rcu(pos)))

#define hlist_for_each_rcu_bh(pos, head) \
	for (pos = rcu_dereference_bh(hlist_first_rcu(head));	\
	     pos;						\
	     pos = rcu_dereference_bh(hlist_next_rcu(pos)))

/*
 * They removed synchronize_rcu_bh() in kernel 5.1, because synchronize_rcu()
 * can apparently be used instead.
 * https://github.com/torvalds/linux/commit/6ba7d681aca22e53385bdb35b1d7662e61905760
 * https://github.com/torvalds/linux/commit/82fcecfa81855924cc69f3078113cf63dd6c2964
 * https://github.com/torvalds/linux/commit/65cfe3583b612a22e12fba9a7bbd2d37ca5ad941
 * https://patchwork.ozlabs.org/patch/996174/
 * https://www.kernel.org/doc/Documentation/RCU/whatisRCU.txt (Section 7: FULL
 * LIST OF RCU APIs)
 *
 * Which is reaaaaaally weird. What if it turns out they need to add a separate
 * version later again? Hope they know what they're doing, seriously.
 */
#if LINUX_VERSION_AT_LEAST(5, 1, 0, 8, 0)
#define synchronize_rcu_bh synchronize_rcu
#endif

#endif /* SRC_MOD_COMMON_RCU_H_ */
