#include "mod/common/dev.h"

#include "mod/common/linux_version.h"

/* "for each interface address" */
int foreach_ifa(struct net *ns, int (*cb)(struct in_ifaddr *, void const *),
		void const *args)
{
	struct net_device *dev;
	struct in_device *in_dev;
#if LINUX_VERSION_AT_LEAST(5, 3, 0, 8, 0)
	struct in_ifaddr *ifa;
#endif
	int result = 0;

	rcu_read_lock();

	for_each_netdev_rcu(ns, dev) {
		in_dev = __in_dev_get_rcu(dev);
		if (!in_dev)
			continue;

#if LINUX_VERSION_AT_LEAST(5, 3, 0, 8, 0)
		in_dev_for_each_ifa_rcu(ifa, in_dev) {
			result = cb(ifa, args);
			if (result)
				goto end;
		}
#else
		for_primary_ifa(in_dev) {
			result = cb(ifa, args);
			if (result)
				goto end;
		} endfor_ifa(in_dev);
#endif
	}

end:
	rcu_read_unlock();
	return result;
}
