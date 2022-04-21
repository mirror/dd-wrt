#ifndef _BP_OF_NET_H
#define _BP_OF_NET_H
#include_next <linux/of_net.h>
#include <linux/version.h>
#include <linux/err.h>

#ifndef CONFIG_OF
#if LINUX_VERSION_IS_LESS(3,10,0)
static inline const void *of_get_mac_address(struct device_node *np)
{
	return NULL;
}
#endif
#endif

/* The behavior of of_get_mac_address() changed in kernel 5.2, it now
 * returns an error code and not NULL in case of an error.
 */
#if LINUX_VERSION_IS_LESS(5,2,0)
static inline int backport_of_get_mac_address(struct device_node *np, u8 *mac)
{
	const void *mactarget = of_get_mac_address(np);

	if (!mactarget)
		return -ENODEV;
	memcpy(mac, mactarget, 6);
	return 0;
}
#define of_get_mac_address LINUX_BACKPORT(of_get_mac_address)
#endif /* < 5.2 */

#endif /* _BP_OF_NET_H */
