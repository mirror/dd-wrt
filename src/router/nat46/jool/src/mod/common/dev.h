#ifndef SRC_MOD_COMMON_DEV_H_
#define SRC_MOD_COMMON_DEV_H_

#include <linux/inetdevice.h>

int foreach_ifa(struct net *ns, int (*cb)(struct in_ifaddr *, void const *),
		void const *args);

#endif /* SRC_MOD_COMMON_DEV_H_ */
