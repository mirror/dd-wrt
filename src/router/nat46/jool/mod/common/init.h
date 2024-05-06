#ifndef SRC_MOD_COMMON_INIT_H_
#define SRC_MOD_COMMON_INIT_H_

#include <net/net_namespace.h>

int jool_siit_get(void);
void jool_siit_put(void);
int jool_nat64_get(void (*defrag_enable)(struct net *ns));
void jool_nat64_put(void);

bool is_siit_enabled(void);
bool is_nat64_enabled(void);

#endif /* SRC_MOD_COMMON_INIT_H_ */
