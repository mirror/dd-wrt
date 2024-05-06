#ifndef SRC_MOD_SIIT_EAM_H_
#define SRC_MOD_SIIT_EAM_H_

#include <linux/kref.h>
#include <linux/rbtree.h>
#include "common/config.h"
#include "common/types.h"
#include "mod/common/rtrie.h"

struct eam_table;

struct eam_table *eamt_alloc(void);
void eamt_get(struct eam_table *eamt);
void eamt_put(struct eam_table *eamt);

/* Safe-to-use-during-packet-translation functions */

int eamt_xlat_4to6(struct eam_table *eamt, struct in_addr *addr4,
		struct result_addrxlat46 *result);
int eamt_xlat_6to4(struct eam_table *eamt, struct in6_addr *addr6,
		struct result_addrxlat64 *result);

bool eamt_contains6(struct eam_table *eamt, struct in6_addr *addr);
bool eamt_contains4(struct eam_table *eamt, __be32 addr);

bool eamt_is_empty(struct eam_table *eamt);

/* Do-not-use-when-you-can't-sleep-functions */

/* See rtrie.h for info on the "synchronize" flag */
int eamt_add(struct eam_table *jool, struct eamt_entry *new, bool force,
		bool synchronize);
int eamt_rm(struct eam_table *eamt, struct ipv6_prefix *prefix6,
		struct ipv4_prefix *prefix4);
void eamt_flush(struct eam_table *eamt);

typedef int (*eamt_foreach_cb)(struct eamt_entry const *, void *);
int eamt_foreach(struct eam_table *eamt,
		eamt_foreach_cb cb, void *arg,
		struct ipv4_prefix *offset);

void eamt_print_refcount(struct eam_table *eamt);

#endif /* SRC_MOD_SIIT_EAM_H_ */
