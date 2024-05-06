#ifndef SRC_MOD_COMMON_ROUTE_H_
#define SRC_MOD_COMMON_ROUTE_H_

#include <linux/bug.h> /* Needed by flow.h in some old kernels (~4.9) */
#include <net/flow.h>
#include "mod/common/xlator.h"

/* Wrappers for the kernel's routing functions. */
struct dst_entry *route4(struct xlator *jool, struct flowi4 *flow);
struct dst_entry *route6(struct xlator *jool, struct flowi6 *flow);

#endif /* SRC_MOD_COMMON_ROUTE_H_ */
