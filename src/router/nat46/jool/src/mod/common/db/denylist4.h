#ifndef SRC_MOD_SIIT_DENYLIST4_H_
#define SRC_MOD_SIIT_DENYLIST4_H_

/**
 * @file
 * Pool of banned IPv4 addresses; Jool will refuse to translate these addresses.
 */

#include <net/net_namespace.h>
#include "mod/common/address.h"

struct addr4_pool *denylist4_alloc(void);
void denylist4_get(struct addr4_pool *pool);
void denylist4_put(struct addr4_pool *pool);

int denylist4_add(struct addr4_pool *pool, struct ipv4_prefix *prefix,
		bool force);
int denylist4_rm(struct addr4_pool *pool, struct ipv4_prefix *prefix);
int denylist4_flush(struct addr4_pool *pool);

bool interface_contains(struct net *ns, struct in_addr *addr);
bool denylist4_contains(struct addr4_pool *pool, struct in_addr *addr);

int denylist4_foreach(struct addr4_pool *pool,
		int (*func)(struct ipv4_prefix *, void *), void *arg,
		struct ipv4_prefix *offset);
bool denylist4_is_empty(struct addr4_pool *pool);

#endif /* SRC_MOD_SIIT_DENYLIST4_H_ */
